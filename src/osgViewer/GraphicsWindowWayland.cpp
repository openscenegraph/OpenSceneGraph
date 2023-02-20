// an OSG GraphicsWindow backed by raw Wayland/EGL

#include <iostream>
#include <sstream>
#include <iomanip>
#include <unistd.h>
#include <osgViewer/GraphicsWindow>
#include "GraphicsWindowWayland.h"
#include <wayland-egl.h>
#include <wayland-cursor.h>
#include <EGL/egl.h>
#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon.h>
#include <sys/mman.h>
#include "xdg-shell.h"
#include "xdg-decoration-unstable-v1.h"

namespace com {
    namespace ashbysoft {

// graphics context state (shared)
#define MAX_OUTPUTS 16
class WLGraphicsWindow;
struct gc_client_state {
    // stuff we reference from the server
    struct wl_display* display;
    struct wl_registry* registry;
    struct wl_shm* shm;
    struct wl_compositor* compositor;
    struct wl_output* output[MAX_OUTPUTS];
    struct xdg_wm_base* xdg_wm_base;
    struct zxdg_decoration_manager_v1* zxdg_decoration_manager_v1;
    struct wl_seat* seat;
    struct wl_keyboard* keyboard;
    struct wl_pointer* pointer;
    struct wl_data_device_manager* data_device_manager;
    EGLDisplay egl_display;
    size_t n_outputs;
    int32_t output_width[MAX_OUTPUTS];
    int32_t output_height[MAX_OUTPUTS];
    // shared cursor theme & lookup map of loaded cursors
    wl_cursor_theme* cursor_theme;
    std::map<osgViewer::GraphicsWindow::MouseCursor, wl_cursor*> cursors;
    // lookup map of windows
    std::map<struct wl_surface*, WLGraphicsWindow*> windows;
    // current seat caps
    uint32_t seat_caps;
    // XKB context, state & current mapping
    struct xkb_context* xkb_content;
    struct xkb_state* xkb_state;
    struct xkb_keymap* xkb_keymap;
    // last pointer position
    float pointer_x;
    float pointer_y;
    // current active surfaces (can be null)
    struct wl_surface* keyboard_surface;
    struct wl_surface* pointer_surface;
    uint32_t keyboard_serial;
    // repeat state
    int32_t keyboard_repeat;
    int32_t keyboard_delay;
    uint64_t keyboard_tick;
    int keyboard_state;
    int keyboard_last;
    #define KEYBOARD_IDLE   0
    #define KEYBOARD_DELAY  1
    #define KEYBOARD_REPEAT 2
};
// graphics window state (instanced)
struct gw_client_state {
    // ref to shared state
    struct gc_client_state* gc;
    // stuff we create
    struct wl_surface* surface;
    struct xdg_surface* xdg_surface;
    struct xdg_toplevel* xdg_toplevel;
    struct zxdg_toplevel_decoration_v1* zxdg_toplevel_decoration_v1;
    struct wl_callback* framecb;
    struct wl_egl_window* egl_window;
    EGLConfig egl_config;
    EGLContext egl_context;
    EGLSurface egl_surface;
    wl_surface* cursor_surface;
    wl_pointer* cursor_pointer;
    uint32_t cursor_serial;
    osgViewer::GraphicsWindow::MouseCursor cursor_last;
    bool floating;
    int width;
    int height;
    bool pending_config;
};

// logging depth and support macro
static thread_local int t_depth;
#define WLGWlog(_e) std::cerr << std::setfill('.') << std::setw(_e>0 ? t_depth++ : _e<0 ? --t_depth: t_depth) << ""

class WLGraphicsWindow : public osgViewer::GraphicsWindow {
private:
    bool _valid = false;
    bool _realized = false;
    struct gw_client_state _gw = {0};
    struct xdg_surface_listener _xdg_surface_listener;
    struct xdg_toplevel_listener _xdg_toplevel_listener;
    std::string _logname;

    static void xdg_surface_configure(void* data, struct xdg_surface* xdg_surface, uint32_t serial) {
        WLGraphicsWindow* obj = (WLGraphicsWindow*)data;
        // tell the compositor we're ok with the config
        xdg_surface_ack_configure(xdg_surface, serial);
        // apply pending config changes (if any)
        if (obj->_gw.pending_config) {
            // size changes
            wl_egl_window_resize(obj->_gw.egl_window, obj->_gw.width, obj->_gw.height, 0, 0);
            obj->resized(0, 0, obj->_gw.width, obj->_gw.height);                // camera(s), also updates _traits
            obj->getEventQueue()->windowResize(obj->_traits->x, obj->_traits->y, obj->_traits->width, obj->_traits->height);     // event co-ords & GUI
            // floating? then decorated
            if (obj->_gw.floating)
                zxdg_toplevel_decoration_v1_set_mode(obj->_gw.zxdg_toplevel_decoration_v1, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
            else
                zxdg_toplevel_decoration_v1_set_mode(obj->_gw.zxdg_toplevel_decoration_v1, ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE);
            obj->_gw.pending_config = false;
            WLGWlog(0) << obj->_logname << "<xdg surface configure: " << obj->_gw.width << 'x' << obj->_gw.height << "," << obj->_gw.floating << ">" << std::endl;
        } else {
            WLGWlog(0) << obj->_logname << "<xdg surface configure (no action)>" << std::endl;
        }
    }
    static void xdg_toplevel_close(void* data, xdg_toplevel* toplevel) {
        WLGraphicsWindow* obj = (WLGraphicsWindow*)data;
        obj->close(true);
        WLGWlog(0) << obj->_logname << "<toplevel closed>" << std::endl;
    }
    static void xdg_toplevel_configure(void* data, xdg_toplevel* toplevel, int width, int height, wl_array* states) {
        WLGraphicsWindow* obj = (WLGraphicsWindow*)data;
        // if width or height are zero, ignore
        if (!width || !height)
            return;
        // determine if we are a floating window
        bool floating=true;
        enum xdg_toplevel_state* pstate = (enum xdg_toplevel_state*)states->data;
        while ((const char*)pstate < (const char*)(states->data)+states->size) {
            if (*pstate==XDG_TOPLEVEL_STATE_TILED_BOTTOM ||
                *pstate==XDG_TOPLEVEL_STATE_TILED_LEFT ||
                *pstate==XDG_TOPLEVEL_STATE_TILED_RIGHT ||
                *pstate==XDG_TOPLEVEL_STATE_TILED_TOP ||
                *pstate==XDG_TOPLEVEL_STATE_FULLSCREEN)
                floating = false;
            pstate++;
        }
        // anything changed?
        if (width!=obj->_traits->width || height!=obj->_traits->height || floating!=obj->_gw.floating) {
            obj->_gw.width = width;
            obj->_gw.height = height;
            obj->_gw.floating = floating;
            // indicate config change is pending
            obj->_gw.pending_config = true;
            WLGWlog(0) << obj->_logname << "<xdg_toplevel_configure: " << width << "," << height << "," << floating << ">" << std::endl;
        }
    }

public:
    WLGraphicsWindow(osg::GraphicsContext::Traits* traits, struct gc_client_state* gc):
        _valid(false),
        _realized(false)
    {
        // generate logging name, use window name if supplied
        if (traits->windowName.empty()) {
            std::ostringstream buf;
            buf << "WLgw(" << this << ')';
            _logname = buf.str();
        } else {
            _logname = "WLgw(" + traits->windowName + ")";
        }
        WLGWlog(1) << _logname << "::<init>[" << std::endl;
        // Fail if we're asked for a pixel buffer
        if (traits->pbuffer) {
            WLGWlog(-1) << _logname << ":pixel buffer surfaces not supported" << std::endl;
            return;
        }
        // save pointer to shared state
        _gw.gc = gc;
        // populate parent object (GraphicsContext) fields: grrrr, should have been constructor args!
        _traits = traits;
        setState(new osg::State);
        getState()->setGraphicsContext(this);
        if (_traits.valid() && _traits->sharedContext.valid()) {
                getState()->setContextID(_traits->sharedContext->getState()->getContextID());
                incrementContextIDUsageCount(getState()->getContextID());
        } else {
                getState()->setContextID(osg::GraphicsContext::createNewContextID());
        }
        // create new surface as top level window
        _gw.surface = wl_compositor_create_surface(_gw.gc->compositor);
        // via an XDG surface wrapper
        _gw.xdg_surface = xdg_wm_base_get_xdg_surface(_gw.gc->xdg_wm_base, _gw.surface);
        _xdg_surface_listener.configure = xdg_surface_configure;
        xdg_surface_add_listener(_gw.xdg_surface, &_xdg_surface_listener, this);
        _gw.xdg_toplevel = xdg_surface_get_toplevel(_gw.xdg_surface);
        _xdg_toplevel_listener.close = xdg_toplevel_close;
        _xdg_toplevel_listener.configure = xdg_toplevel_configure;
        xdg_toplevel_add_listener(_gw.xdg_toplevel, &_xdg_toplevel_listener, this);
        // hint for the window manager on grouping, and allows app-specific rules to be applied
        xdg_toplevel_set_app_id(_gw.xdg_toplevel, "org.flightgear.FlightGear");
        // grab decoration extension for this toplevel
        _gw.zxdg_toplevel_decoration_v1 = zxdg_decoration_manager_v1_get_toplevel_decoration(_gw.gc->zxdg_decoration_manager_v1, _gw.xdg_toplevel);
        // apply traits..
        xdg_toplevel_set_title(_gw.xdg_toplevel, _traits->windowName.c_str());
        WLGWlog(0) << _logname << "name='" << _traits->windowName << "'" << std::endl;
        // bool windowDecoration: used to determine fullscreen or not
        // screen ID used to select output
        if (_traits->windowDecoration)
            xdg_toplevel_unset_fullscreen(_gw.xdg_toplevel);
        else
            xdg_toplevel_set_fullscreen(_gw.xdg_toplevel, NULL);    // allow compositor to select screen
        WLGWlog(0) << _logname << "full=" << !_traits->windowDecoration << "/screen=" << _traits->screenNum << std::endl;
        // bool supportsResize yes, we do, no action.
        // bool pbuffer (see above)
        // bool quadBufferStereo - ignored?
        // doubleBuffer - by default yes.
        // GLenum target, format (texture output)
        // uint level, format, mipMapGen (texture output)
        // bool vsync - by default yes.
        WLGWlog(0) << _logname << "vsync=" << _traits->vsync << std::endl;
        // bool swapGroupEnabled,
        // GLuint swapGroup, swapBarrier - ignored?
        // bool useMultiThreadGL (MacOS only?)
        // bool useCursor - as requested
        _gw.cursor_surface = wl_compositor_create_surface(_gw.gc->compositor);
        setCursor(traits->useCursor ? MouseCursor::LeftArrowCursor : MouseCursor::NoCursor);
        // std::string glContextVersion,
        // uint glContextFlags, glProfileMask - ignored, should check >=
        // bool setInheritedPixelFormat - ignored?
        // DisplaySettings::SwapMethod swapMethod - ignored?
        // OpenThreads::Affinity affinity - ignored?

        // uint red,blue,green,alpha colour bits, depth&stencil buffer depths 
        // uint sampleBuffers, samples
        EGLint ncfg, eglattrs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
            EGL_BLUE_SIZE, (EGLint)_traits->blue,
            EGL_GREEN_SIZE, (EGLint)_traits->green,
            EGL_RED_SIZE, (EGLint)_traits->red,
            EGL_ALPHA_SIZE, (EGLint)_traits->alpha,
            EGL_DEPTH_SIZE, (EGLint)_traits->depth,
            EGL_STENCIL_SIZE, (EGLint)_traits->stencil,
            _traits->sampleBuffers? EGL_SAMPLE_BUFFERS : EGL_NONE, (EGLint)_traits->sampleBuffers,
            _traits->samples? EGL_SAMPLES : EGL_NONE, (EGLint)_traits->samples,
            EGL_NONE };

        // create EGL surface for OpenGL/OSG to draw on
        if (eglChooseConfig(_gw.gc->egl_display, eglattrs, &_gw.egl_config, 1, &ncfg) != EGL_TRUE) {
            WLGWlog(-1) << _logname << "::checkInit: cannot choose EGL config" << std::endl;
            return;
        }
        WLGWlog(0) << _logname << "depths(b/g/r/a/d/s)=(" << _traits->blue << '/' << _traits->green << '/' << _traits->red << '/'
            << _traits->alpha << '/' << _traits->depth << '/' << _traits->stencil << ")" << std::endl;
        if (_traits->sampleBuffers || _traits->samples) {
            WLGWlog(0) << _logname << "msaa(b/s)=(" << _traits->sampleBuffers << '/' << _traits->samples << ")" << std::endl;
        }
        if (!eglBindAPI(EGL_OPENGL_API)) {
            WLGWlog(-1) << _logname << "::checkInit: cannot bind OpenGL API" << std::endl;
            return;
        }
        _gw.egl_context = eglCreateContext(_gw.gc->egl_display, _gw.egl_config, EGL_NO_CONTEXT, NULL);
        if (EGL_NO_CONTEXT == _gw.egl_context) {
            WLGWlog(-1) << _logname << "::checkInit: cannot create EGL context" << std::endl;
            return;
        }
        _gw.egl_window = wl_egl_window_create(_gw.surface, _traits->width, _traits->height);
        if (!_gw.egl_window) {
            WLGWlog(-1) << _logname << ":cannot create EGL window" << std::endl;
            return;
        }
        _gw.egl_surface = eglCreateWindowSurface(_gw.gc->egl_display, _gw.egl_config, _gw.egl_window, nullptr);
        if (EGL_NO_SURFACE == _gw.egl_surface) {
            WLGWlog(-1) << _logname << ":cannot create EGL surface" << std::endl;
            return;
        }
        _valid = true;
        // add ourselves to window map
        gc->windows[_gw.surface] = this;
        // indicate that we're not configured yet
        _gw.width = _traits->width;
        _gw.height = _traits->height;
        _gw.pending_config = true;
        WLGWlog(-1) << "]=valid" << std::endl;
    }
    virtual ~WLGraphicsWindow() {
        WLGWlog(1) << _logname << "WLGraphicsWindow<term>[" << std::endl;
        close(true);
        // remove ourselves from window map
        _gw.gc->windows.erase(_gw.surface);
        WLGWlog(-1) << _logname << "]" << std::endl;
    }
    // accessor for input event system to notify a specific window of the pointer arriving..
    void pointerEnter(wl_pointer* wl_pointer, uint32_t serial) {
        // save state, set our cursor
        _gw.cursor_pointer = wl_pointer;
        _gw.cursor_serial = serial;
        applyCursor(_gw.cursor_last);
        WLGWlog(0) << _logname << "::pointerEnter" << std::endl;
    }

    // GraphicsWindow overrides
    virtual const char* className() const { return "WLGraphicsWindow"; }
    virtual bool setWindowRectangleImplementation(int x, int y, int w, int h) {
        // not without difficulty on Wayland..
        WLGWlog(0) << _logname << "::setWindowRectangleImplementation(" << x << "," << y << "," << w << "," << h << ")=false" << _traits.valid() << std::endl;
        return false;
    }
    virtual bool setWindowDecorationImplementation(bool windowDecoration) {
        WLGWlog(0) << _logname << "::setWindowDecorationImplementation(" << windowDecoration << ")" << std::endl;
        // used to determine fullscreen or not - xdg_toplevel_configure will sort out size/decoration
        if (windowDecoration)
            xdg_toplevel_unset_fullscreen(_gw.xdg_toplevel);
        else
            xdg_toplevel_set_fullscreen(_gw.xdg_toplevel, NULL);    // allow compositor to select screen
        return true;
    }
    virtual void grabFocus() {
        WLGWlog(0) << _logname << "::grabFocus()" << std::endl;
        // UNSUPPORTED in Wayland?
    }
    virtual void raiseWindow() {
        WLGWlog(0) << _logname << "::raiseWindow()" << std::endl;
        // UNSUPPORTED in Wayland?
    }
    virtual void setWindowName(const std::string& name) {
        WLGWlog(0) << _logname << "::setWindowName(" << name << ")" << std::endl;
        _traits->windowName = name;
        _logname = "WLgw(" + name + ")";
        xdg_toplevel_set_title(_gw.xdg_toplevel, _traits->windowName.c_str());
    }
    virtual void setSyncToVBlank(bool on) {
        WLGWlog(0) << _logname << "::setSyncToVBlank(" << on << ")" << std::endl;
        // wait for frame callback..
        _traits->vsync = on;
    }
    virtual void setCursor(MouseCursor mouseCursor) {
        WLGWlog(0) << _logname << "::setCursor(" << mouseCursor << " [" << _gw.cursor_last << "])" << std::endl;
        // InheritCursor => put back previous cursor
        if (MouseCursor::InheritCursor == mouseCursor)
            mouseCursor = _gw.cursor_last;
        applyCursor(mouseCursor);
       // remember what was last set
        _gw.cursor_last = mouseCursor;
    }
    virtual bool valid() const {
        if (!_valid) WLGWlog(0) << _logname << "::valid()=" << _valid << std::endl;
        return _valid;
    }
    virtual bool realizeImplementation() {
        WLGWlog(1) << _logname << "::realizeImplementation()[" << std::endl;
        while (!_realized) {
            wl_surface_commit(_gw.surface);
            wl_display_roundtrip(_gw.gc->display);
            _realized = true;
        }
        WLGWlog(-1) << _logname << "]=" << _realized << std::endl;
        return _realized;
    }
    virtual bool isRealizedImplementation() const {
        WLGWlog(0) << _logname << "::isRealizedImplementation()=" << _realized << std::endl;
        return _realized;
    }
    virtual void closeImplementation() {
        WLGWlog(0) << _logname << "::closeImplementation()" << std::endl;
        if (_valid) {
            if (_gw.egl_surface != EGL_NO_SURFACE) eglDestroySurface(_gw.gc->egl_display, _gw.egl_surface);
            if (_gw.egl_window) wl_egl_window_destroy(_gw.egl_window);
            if (_gw.xdg_toplevel) xdg_toplevel_destroy(_gw.xdg_toplevel);
            if (_gw.xdg_surface) xdg_surface_destroy(_gw.xdg_surface);
            if (_gw.surface) wl_surface_destroy(_gw.surface);
        }
        _valid = false;
    }

    // GraphicsContext overrides
    virtual bool makeCurrentImplementation() {
        EGLContext curContext = eglGetCurrentContext();
        bool rv = (curContext != _gw.egl_context) ? eglMakeCurrent(_gw.gc->egl_display, _gw.egl_surface, _gw.egl_surface, _gw.egl_context) : true;
        //PAA:Noisy in multi-screen! WLGWlog(0) << _logname << "::makeCurrentImplementation()=" << rv << std::endl;
        return rv;
    }
    virtual void swapBuffersImplementation() {
        eglSwapBuffers(_gw.gc->egl_display, _gw.egl_surface);
        // pump any async logic
        checkAsyncWork();
        // pump any Wayland messages
        wl_display_dispatch_pending(_gw.gc->display);
    }

private:
    void applyCursor(MouseCursor mouseCursor) {
        // provided we have a pointer reference and a serial number.. do it!
        if (_gw.cursor_pointer && _gw.cursor_serial) {
            // map requested cursor to a buffer/surface
            auto it = _gw.gc->cursors.find(mouseCursor);
            if (it != _gw.gc->cursors.end() && it->second) {
                wl_surface_destroy(_gw.cursor_surface);
                _gw.cursor_surface = wl_compositor_create_surface(_gw.gc->compositor);
                wl_surface_attach(_gw.cursor_surface, wl_cursor_image_get_buffer(it->second->images[0]), 0, 0);
                wl_surface_commit(_gw.cursor_surface);
                wl_pointer_set_cursor(_gw.cursor_pointer, _gw.cursor_serial, _gw.cursor_surface, 0, 0);
            } else {
                wl_pointer_set_cursor(_gw.cursor_pointer, _gw.cursor_serial, nullptr, 0, 0);
            }
        }
    }
};

class WLWindowingSystemInterface : public osg::GraphicsContext::WindowingSystemInterface {

private:
    bool _wl_init = false;
    struct gc_client_state _gc = {0};
    struct wl_registry_listener _wl_registry_listener;
    struct wl_output_listener _wl_output_listener;
    struct wl_seat_listener _wl_seat_listener;
    struct xdg_wm_base_listener _xdg_wm_base_listener;
    struct wl_keyboard_listener _wl_keyboard_listener;
    struct wl_pointer_listener _wl_pointer_listener;

    // Registry object capture
    static void registry_add(void *data, struct wl_registry *wl_registry, uint32_t name, const char *interface, uint32_t version) {
        WLWindowingSystemInterface* obj = (WLWindowingSystemInterface*) data;
        void** phandle = nullptr;
        const wl_interface* pinterface;
        if (strcmp(interface, wl_shm_interface.name)==0) {
            phandle = (void**)&obj->_gc.shm; pinterface = &wl_shm_interface;
        } else if (strcmp(interface, wl_compositor_interface.name)==0) {
            phandle = (void**)&obj->_gc.compositor; pinterface = &wl_compositor_interface;
        } else if (strcmp(interface, wl_seat_interface.name)==0) {
            phandle = (void**)&obj->_gc.seat; pinterface = &wl_seat_interface;
        } else if (strcmp(interface, wl_output_interface.name)==0) {
            if (obj->_gc.n_outputs < MAX_OUTPUTS) {
                phandle = (void**)&obj->_gc.output[obj->_gc.n_outputs]; pinterface = &wl_output_interface;
                obj->_gc.n_outputs++;
            } else {
                OSG_WARN << "WLwsi: more than " << MAX_OUTPUTS << " outputs available, unable to track them all!" << std::endl;
            }
        } else if (strcmp(interface, xdg_wm_base_interface.name)==0) {
            phandle = (void**)&obj->_gc.xdg_wm_base; pinterface = &xdg_wm_base_interface;
        } else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name)==0) {
            phandle = (void**)&obj->_gc.zxdg_decoration_manager_v1; pinterface = &zxdg_decoration_manager_v1_interface;
        } else if (strcmp(interface, wl_data_device_manager_interface.name)==0) {
            phandle = (void**)&obj->_gc.data_device_manager; pinterface = &wl_data_device_manager_interface;
        }
        if (phandle) {
            uint32_t minversion = (int)version < pinterface->version ? version : (uint32_t)pinterface->version;
            *phandle = wl_registry_bind(wl_registry, name, pinterface, minversion);
            WLGWlog(0) << "<registry bind: " << interface << '@' << minversion << ">" << std::endl;
        }
    }
    static void registry_rem(void *data, struct wl_registry *wl_registry, uint32_t name) {}
    // Output info
    int find_output(struct wl_output* wl_output) {
        for (int o=0; o<(int)_gc.n_outputs; o++) {
            if (_gc.output[o]==wl_output)
                return o;
        }
        return -1;
    }
    static void output_geometry(void *data, wl_output *wl_output, int32_t x, int32_t y, int32_t physical_width, int32_t physical_height, int32_t subpixel, const char *make, const char *model, int32_t transform) {
        WLWindowingSystemInterface* obj = (WLWindowingSystemInterface*) data;
        int o = obj->find_output(wl_output);
        WLGWlog(0) << "<output(" << o << ") geom(" << make << '/' << model << "): " << physical_width << 'x' << physical_height << '@' << x << ',' << y << ">" << std::endl;
    }
    static void output_mode(void *data, wl_output *wl_output, uint32_t flags, int32_t width, int32_t height, int32_t refresh) {
        // retain current mode
        WLWindowingSystemInterface* obj = (WLWindowingSystemInterface*) data;
        int o = obj->find_output(wl_output);
        if ((WL_OUTPUT_MODE_CURRENT & flags) && o>=0) {
            obj->_gc.output_width[o] = width;
            obj->_gc.output_height[o] = height;
        }
        WLGWlog(0) << "<output(" << o << ") mode: " << width << 'x' << height << ">" << std::endl;
    }
    static void output_scale(void *data, wl_output *wl_output, int32_t factor) {
        WLWindowingSystemInterface* obj = (WLWindowingSystemInterface*) data;
        int o = obj->find_output(wl_output);
        WLGWlog(0) << "<output(" << o << ") scale: " << factor << ">" << std::endl;
    }
    static void output_done(void *data, wl_output *wl_output) {}
    // Input handling
    WLGraphicsWindow* get_window(struct wl_surface* surface) {
        if (surface) {
            auto winit = _gc.windows.find(surface);
            if (winit != _gc.windows.end()) {
                return winit->second;
            }
        }
        return nullptr;
    }
    static void keyboard_enter(void* data, wl_keyboard* wl_keyboard, uint32_t serial, wl_surface* surface, wl_array* keys) {
        WLWindowingSystemInterface* obj = (WLWindowingSystemInterface*) data;
        obj->_gc.keyboard_surface = surface;
        obj->_gc.keyboard_serial = serial;
        WLGWlog(1) << "<keyboard enter: " << surface << ">" << std::endl;
        // dump pressed keys
        uint32_t* pkey = (uint32_t*)keys->data;
        while ((char*)pkey < ((char*)keys->data+keys->size)) {
            keyboard_key(data, wl_keyboard, serial, 0, *pkey, 1);
            pkey++;
        }
        WLGWlog(-1) << "<keyboard enter: done>" << std::endl;
    }
    static void keyboard_leave(void* data, wl_keyboard* wl_keyboard, uint32_t serial, wl_surface* surface) {
        WLWindowingSystemInterface* obj = (WLWindowingSystemInterface*) data;
        obj->_gc.keyboard_surface = nullptr;
        obj->_gc.keyboard_state = KEYBOARD_IDLE;
        WLGWlog(0) << "<keyboard leave: " << surface << ">" << std::endl;
    }
    static void keyboard_map(void* data, wl_keyboard* wl_keyboard, uint32_t format, int32_t fd, uint32_t size) {
        WLWindowingSystemInterface* obj = (WLWindowingSystemInterface*) data;
        // we only support XKBv1
        if (format!=WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
            return;
        // create a context if not already done
        if (!obj->_gc.xkb_content)
            obj->_gc.xkb_content = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
        // map to our memory space
        void *xkbtext = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
        // [re-]parse as a keymap
        xkb_keymap_unref(obj->_gc.xkb_keymap);
        obj->_gc.xkb_keymap = xkb_keymap_new_from_string(obj->_gc.xkb_content, (const char *)xkbtext, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
        munmap(xkbtext, size);
        close(fd);
        // [re-]initialise state
        xkb_state_unref(obj->_gc.xkb_state);
        obj->_gc.xkb_state = xkb_state_new(obj->_gc.xkb_keymap);
        WLGWlog(0) << "<keyboard map: format=" << format << ", fd=" << fd << ", size=" << size << ", map=" << obj->_gc.xkb_keymap << ", state=" << obj->_gc.xkb_state << ">" << std::endl;
    }
    static void keyboard_repeat(void* data, wl_keyboard* wl_keyboard, int32_t rate, int32_t delay) {
        WLWindowingSystemInterface* obj = (WLWindowingSystemInterface*) data;
        WLGWlog(0) << "<keyboard repeat: rate=" << rate << ", delay=" << delay << ">" << std::endl;
        obj->_gc.keyboard_repeat = 1000/rate;
        obj->_gc.keyboard_delay = delay;
    }
    static void keyboard_modifiers(void* data, wl_keyboard* wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
        WLWindowingSystemInterface* obj = (WLWindowingSystemInterface*) data;
        // update XKB with modifier state: https://wayland-book.com/seat/keyboard.html
        xkb_state_update_mask(obj->_gc.xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
        if (auto win = obj->get_window(obj->_gc.keyboard_surface)) {
            // adjust currently effective modifiers
            auto es = win->getEventQueue()->getCurrentEventState();
            int emods = es->getModKeyMask();
            if (xkb_state_mod_name_is_active(obj->_gc.xkb_state, XKB_MOD_NAME_SHIFT, XKB_STATE_MODS_EFFECTIVE))
                emods |= osgGA::GUIEventAdapter::ModKeyMask::MODKEY_SHIFT;
            else
                emods &= ~osgGA::GUIEventAdapter::ModKeyMask::MODKEY_SHIFT;
            if (xkb_state_mod_name_is_active(obj->_gc.xkb_state, XKB_MOD_NAME_CTRL, XKB_STATE_MODS_EFFECTIVE))
                emods |= osgGA::GUIEventAdapter::ModKeyMask::MODKEY_CTRL;
            else
                emods &= ~osgGA::GUIEventAdapter::ModKeyMask::MODKEY_CTRL;
            if (xkb_state_mod_name_is_active(obj->_gc.xkb_state, XKB_MOD_NAME_ALT, XKB_STATE_MODS_EFFECTIVE))
                emods |= osgGA::GUIEventAdapter::ModKeyMask::MODKEY_ALT;
            else
                emods &= ~osgGA::GUIEventAdapter::ModKeyMask::MODKEY_ALT;
            es->setModKeyMask(emods);
            // push through a harmless key to update modifier state in event system, otherwise the joystick b0rks.. (arrrrgh!!)
            win->getEventQueue()->keyPress(osgGA::GUIEventAdapter::KeySymbol::KEY_Shift_L);
        }
        WLGWlog(0) << "<keymods: " << mods_depressed << ',' << mods_latched << ',' << mods_locked << ',' << group << ">" << std::endl;
    }
    static void keyboard_key(void* data, wl_keyboard* wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
        WLWindowingSystemInterface* obj = (WLWindowingSystemInterface*) data;
        // NB: from: https://wayland-book.com/seat/keyboard.html
        // "Important: the scancode from this event is the Linux evdev scancode. To translate this to an XKB scancode, you must add 8 to the evdev scancode."
        key += 8;
        // ignore modifier keys..
        if (!xkb_key_repeats(obj->_gc.xkb_keymap, key))
            return;
        // We also rely on the fact that OSG have used the /same UTF32 symbol codes/ as XKB (or so it appears)
        xkb_keysym_t sym = xkb_state_key_get_one_sym(obj->_gc.xkb_state, key);
        // if Ctrl is in play and we have A-Z, synthesize old ASCII values, as 'get_one_sym' above does not translate Ctrl codes..
        if (xkb_state_mod_name_is_active(obj->_gc.xkb_state, XKB_MOD_NAME_CTRL, XKB_STATE_MODS_EFFECTIVE)
            && sym>=XKB_KEY_a && sym<=XKB_KEY_z) {
            sym = 1 + (sym - XKB_KEY_a);
        }
        // find the target window..
        if (auto win = obj->get_window(obj->_gc.keyboard_surface)) {
            if (state)
                win->getEventQueue()->keyPress((int)sym);
            else
                win->getEventQueue()->keyRelease((int)sym);
            WLGWlog(0) << (state?"<keypress: ":"<keyrelease: ") << key << "=>" << sym << ">" << std::endl;
        }
        // any keypress always puts us in DELAY state for repeats, any release and we stop repeating
        obj->_gc.keyboard_state = state ? KEYBOARD_DELAY : KEYBOARD_IDLE;
        obj->_gc.keyboard_last = (int)sym;
    }
    static void pointer_enter(void *data, wl_pointer* wl_pointer, uint32_t serial, wl_surface* surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
        WLGWlog(0) << "<pointer enter: " << surface << ">" << std::endl;
        WLWindowingSystemInterface* obj = (WLWindowingSystemInterface*) data;
        obj->_gc.pointer_surface = surface;
        if (auto win = obj->get_window(surface)) {
            win->pointerEnter(wl_pointer, serial);
        }
    }
    static void pointer_leave(void* data, wl_pointer* wl_pointer, uint32_t serial, wl_surface* surface) {
        WLWindowingSystemInterface* obj = (WLWindowingSystemInterface*) data;
        obj->_gc.pointer_surface = nullptr;
        WLGWlog(0) << "<pointer leave: " << surface << ">" << std::endl;
    }
    static void pointer_axis(void* data, wl_pointer* wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {
        WLWindowingSystemInterface* obj = (WLWindowingSystemInterface*) data;
        int32_t move = (value>>8);
        if (!move)
            return;
        if (auto win = obj->get_window(obj->_gc.pointer_surface)) {
            win->getEventQueue()->mouseScroll(
                move<0 ?
                    WL_POINTER_AXIS_VERTICAL_SCROLL==axis ?
                        osgGA::GUIEventAdapter::ScrollingMotion::SCROLL_UP : osgGA::GUIEventAdapter::ScrollingMotion::SCROLL_LEFT :
                    WL_POINTER_AXIS_VERTICAL_SCROLL==axis ?
                        osgGA::GUIEventAdapter::ScrollingMotion::SCROLL_DOWN : osgGA::GUIEventAdapter::ScrollingMotion::SCROLL_RIGHT
            );
            if (getenv("OSGMOUSEVERB"))
                WLGWlog(0) << "<pointer_axis(" << axis << ',' << (value>>8) << ")>" << std::endl;
        }
    }
    static void pointer_axis_discrete(void* data, wl_pointer* wl_pointer, uint32_t axis, int32_t discrete) {
        // TODO: ignored for now (also deprecated in later protocol versions)
        WLGWlog(0) << "<pointer_axis_discreet(" << discrete << ")>" << std::endl;
    }
    static void pointer_axis_source(void* data, wl_pointer* wl_pointer, uint32_t source) {
        if (getenv("OSGMOUSEVERB"))
            WLGWlog(0) << "<pointer_axis_source(" << source << ")>" << std::endl;
    }
    static void pointer_axis_stop(void* data, wl_pointer* wl_pointer, uint32_t time, uint32_t axis) {
        if (getenv("OSGMOUSEVERB"))
            WLGWlog(0) << "<pointer_axis_stop(" << axis << ")>" << std::endl;
    }
    static void pointer_motion(void* data, wl_pointer* wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
        WLWindowingSystemInterface* obj = (WLWindowingSystemInterface*) data;
        if (auto win = obj->get_window(obj->_gc.pointer_surface)) {
            osgGA::EventQueue* queue = win->getEventQueue();
            if (getenv("OSGMOUSETX")) {
                float minX = queue->getCurrentEventState()->getXmin();
                float maxX = queue->getCurrentEventState()->getXmax();
                float minY = queue->getCurrentEventState()->getYmin();
                float maxY = queue->getCurrentEventState()->getYmax();
                obj->_gc.pointer_x = minX + (maxX-minX)*(float)(surface_x>>8)/(float)(win->getTraits()->width);
                obj->_gc.pointer_y = minY + (maxY-minY)*(float)(surface_y>>8)/(float)(win->getTraits()->height);
            } else {
                obj->_gc.pointer_x = (float)(surface_x>>8);
                obj->_gc.pointer_y = (float)(surface_y>>8);
            }
            queue->mouseMotion(obj->_gc.pointer_x, obj->_gc.pointer_y);
            if (getenv("OSGMOUSEVERB"))
                WLGWlog(0) << "<mouse@" << (obj->_gc.pointer_x) << ',' << (obj->_gc.pointer_y) << ">" << std::endl;
        }
    }
    static void pointer_button(void* data, wl_pointer* wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
        WLWindowingSystemInterface* obj = (WLWindowingSystemInterface*) data;
        int mapped = BTN_LEFT==button ? 1 : BTN_MIDDLE==button ? 2 : BTN_RIGHT==button ? 3 : -1;
        if (mapped<0)
            return;
        if (auto win = obj->get_window(obj->_gc.pointer_surface)) {
            if (state)
                win->getEventQueue()->mouseButtonPress(obj->_gc.pointer_x, obj->_gc.pointer_y, mapped);
            else
                win->getEventQueue()->mouseButtonRelease(obj->_gc.pointer_x, obj->_gc.pointer_y, mapped);
            if (getenv("OSGMOUSEVERB"))
                WLGWlog(0) << "<button: " << button << '@' << (obj->_gc.pointer_x) << ',' << (obj->_gc.pointer_y) << ',' << state << ">" << std::endl;
        }
    }
    static void pointer_frame(void* data, wl_pointer* wl_pointer) {
        if (getenv("OSGMOUSEVERB"))
            WLGWlog(0) << "<pointer frame>" << std::endl;
    }
    static void seat_capabilities(void* data, wl_seat* wl_seat, uint32_t capabilities) {
        WLWindowingSystemInterface* obj = (WLWindowingSystemInterface*) data;
        obj->_gc.seat_caps = capabilities;
        // clean up existing handlers
        if (obj->_gc.keyboard) {
            wl_keyboard_destroy(obj->_gc.keyboard);
            obj->_gc.keyboard = nullptr;
        }
        if (obj->_gc.pointer) {
            wl_pointer_destroy(obj->_gc.pointer);
            obj->_gc.pointer = nullptr;
        }
        // attach new handlers
        if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
            obj->_gc.keyboard = wl_seat_get_keyboard(wl_seat);
            obj->_wl_keyboard_listener.enter = keyboard_enter;
            obj->_wl_keyboard_listener.leave = keyboard_leave;
            obj->_wl_keyboard_listener.keymap = keyboard_map;
            obj->_wl_keyboard_listener.modifiers = keyboard_modifiers;
            obj->_wl_keyboard_listener.key = keyboard_key;
            obj->_wl_keyboard_listener.repeat_info = keyboard_repeat;
            wl_keyboard_add_listener(obj->_gc.keyboard, &obj->_wl_keyboard_listener, obj);
        }
        if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
            obj->_gc.pointer = wl_seat_get_pointer(wl_seat);
            obj->_wl_pointer_listener.enter = pointer_enter;
            obj->_wl_pointer_listener.leave = pointer_leave;
            obj->_wl_pointer_listener.axis = pointer_axis;
            obj->_wl_pointer_listener.axis_discrete = pointer_axis_discrete;
            obj->_wl_pointer_listener.axis_source = pointer_axis_source;
            obj->_wl_pointer_listener.axis_stop = pointer_axis_stop;
            obj->_wl_pointer_listener.motion = pointer_motion;
            obj->_wl_pointer_listener.button = pointer_button;
            obj->_wl_pointer_listener.frame = pointer_frame;
            wl_pointer_add_listener(obj->_gc.pointer, &obj->_wl_pointer_listener, obj);
        }
        WLGWlog(0) << "<seat caps: " << capabilities << ">" << std::endl;
    }
    static void seat_name(void* data, wl_seat* wl_seat, const char *name) {
        WLGWlog(0) << "<seat name: " << name << ">" << std::endl;
    }
    // XDG keepalive responder
    static void xdg_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
        xdg_wm_base_pong(xdg_wm_base, serial);
        WLGWlog(0) << "<xdg_ping>" << std::endl;
    }

    // Lazy init
    bool checkInit() {
        while (!_wl_init) {
            WLGWlog(1) << "WLwsi::checkInit()[" << std::endl;
            _gc.display = wl_display_connect(nullptr);
            if (!_gc.display) {
                WLGWlog(0) << "WLwsi::checkInit: no Wayland display" << std::endl;
                break;
            }
            // enumerate the registry to find shared objects
            _gc.registry = wl_display_get_registry(_gc.display);
            _wl_registry_listener.global = registry_add;
            _wl_registry_listener.global_remove = registry_rem;
            wl_registry_add_listener(_gc.registry, &_wl_registry_listener, this);
            wl_display_roundtrip(_gc.display);
            // ensure we got all required shared objects
            if (!_gc.compositor || !_gc.shm || !_gc.output || !_gc.xdg_wm_base || !_gc.zxdg_decoration_manager_v1) {
                WLGWlog(0) << "WLwsi::checkInit: missing one of compositor/shm/output/xdg_wm_base/zxdg_decoration_manager_v1" << std::endl;
                break;
            }
            // load default cursor theme & cursor images for each OSG cursor
            // eventually deduced by looking at:
            // https://www.opengl.org/resources/libraries/glut/spec3/node28.html - for OSG names and descriptions
            // https://github.com/drizt/xcursor-viewer - to view actual cursors and match against descriptions above
            _gc.cursor_theme = wl_cursor_theme_load(nullptr, 24, _gc.shm);
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::NoCursor] = nullptr;
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::RightArrowCursor] = wl_cursor_theme_get_cursor(_gc.cursor_theme, "right_ptr");
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::LeftArrowCursor] = wl_cursor_theme_get_cursor(_gc.cursor_theme, "left_ptr");
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::InfoCursor] = wl_cursor_theme_get_cursor(_gc.cursor_theme, "hand2");
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::DestroyCursor] = wl_cursor_theme_get_cursor(_gc.cursor_theme, "X_cursor");
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::HelpCursor] = wl_cursor_theme_get_cursor(_gc.cursor_theme, "question_arrow");
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::CycleCursor] = wl_cursor_theme_get_cursor(_gc.cursor_theme, "left_ptr");    // recycle icon: no equivalent
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::SprayCursor] = wl_cursor_theme_get_cursor(_gc.cursor_theme, "pencil");
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::WaitCursor] = wl_cursor_theme_get_cursor(_gc.cursor_theme, "watch");
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::TextCursor] = wl_cursor_theme_get_cursor(_gc.cursor_theme, "xterm");
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::CrosshairCursor] = wl_cursor_theme_get_cursor(_gc.cursor_theme, "cross");
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::HandCursor] = wl_cursor_theme_get_cursor(_gc.cursor_theme, "grabbing");
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::UpDownCursor] = wl_cursor_theme_get_cursor(_gc.cursor_theme, "sb_v_double_arrow");
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::LeftRightCursor] = wl_cursor_theme_get_cursor(_gc.cursor_theme, "sb_h_double_arrow");
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::TopSideCursor] = wl_cursor_theme_get_cursor(_gc.cursor_theme, "top_side");
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::BottomSideCursor] = wl_cursor_theme_get_cursor(_gc.cursor_theme, "bottom_side");
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::LeftSideCursor] = wl_cursor_theme_get_cursor(_gc.cursor_theme, "left_side");
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::RightSideCursor] = wl_cursor_theme_get_cursor(_gc.cursor_theme, "right_side");
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::TopLeftCorner] = wl_cursor_theme_get_cursor(_gc.cursor_theme, "top_left_corner");
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::TopRightCorner] = wl_cursor_theme_get_cursor(_gc.cursor_theme, "top_right_corner");
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::BottomRightCorner] = wl_cursor_theme_get_cursor(_gc.cursor_theme, "bottom_right_corner");
            _gc.cursors[osgViewer::GraphicsWindow::MouseCursor::BottomLeftCorner] = wl_cursor_theme_get_cursor(_gc.cursor_theme, "bottom_left_corner");
            WLGWlog(0) << "WLwsi: loaded cursor_theme: " << _gc.cursor_theme << std::endl;
            // monitor seat capabilities if we have one..
            if (_gc.seat) {
                _wl_seat_listener.capabilities = seat_capabilities;
                _wl_seat_listener.name = seat_name;
                wl_seat_add_listener(_gc.seat, &_wl_seat_listener, this);
            }
            // detect output settings
            _wl_output_listener.geometry = output_geometry;
            _wl_output_listener.mode = output_mode;
            _wl_output_listener.scale = output_scale;
            _wl_output_listener.done = output_done;
            for (size_t o=0; o<_gc.n_outputs; o++)
                wl_output_add_listener(_gc.output[o], &_wl_output_listener, this);
            // attach ping responder
            _xdg_wm_base_listener.ping = xdg_ping;
            xdg_wm_base_add_listener(_gc.xdg_wm_base, &_xdg_wm_base_listener, this);

            // connect EGL
            _gc.egl_display = eglGetDisplay(_gc.display);
            if (EGL_NO_DISPLAY == _gc.egl_display) {
                WLGWlog(0) << "WLwsi::checkInit: no EGL display" << std::endl;
                break;
            }
            EGLint maj, min;
            if (eglInitialize(_gc.egl_display, &maj, &min) != EGL_TRUE) {
                WLGWlog(0) << "WLwsi::checkInit: cannot initialize EGL" << std::endl;
                break;
            }
            WLGWlog(0) << "WLwsi: EGL version: " << maj << "." << min << std::endl;
            // process any outstanding events
            wl_display_roundtrip(_gc.display);
            _wl_init = true;
            WLGWlog(-1) << "]=true" << std::endl;
        }
        if (!_wl_init)
            WLGWlog(-1) << "]=false" << std::endl;
        return _wl_init;
    }

public:
    WLWindowingSystemInterface() : _wl_init(false) {
        WLGWlog(0) << "WLWindowingSystemInterface<init>" << std::endl;
    }
    virtual unsigned int getNumScreens(const osg::GraphicsContext::ScreenIdentifier& si) {
        WLGWlog(1) << "WLwsi:getNumScreens(...)[" << std::endl;
        unsigned int rv = checkInit() ? _gc.n_outputs : 0;
        WLGWlog(-1) << "]=" << rv << std::endl;
        return rv;
    }
    virtual void getScreenSettings(const osg::GraphicsContext::ScreenIdentifier& si, osg::GraphicsContext::ScreenSettings& resolution) {
        WLGWlog(1) << "WLwsi:getScreenSettings(" << si.screenNum << ")[" << std::endl;
        if (!checkInit()) {
            WLGWlog(-1) << "]=failed" << std::endl;
            return;
        }
        // sanity check screenNum :)
        int screen = si.screenNum;
        if (si.screenNum<0 || si.screenNum >= (int)_gc.n_outputs) {
            OSG_WARN << "WLwsi: getScreenSettings: requested screen number (" << si.screenNum << ") out of range (0-" << (_gc.n_outputs-1) << "), using 0" << std::endl;
            screen = 0;
        }
        resolution.width = _gc.output_width[screen];
        resolution.height = _gc.output_height[screen];
        resolution.refreshRate = 0;
        resolution.colorDepth = 0;
        WLGWlog(-1) << "]=" << resolution.width << 'x' << resolution.height << std::endl;
    }
    virtual void enumerateScreenSettings(const osg::GraphicsContext::ScreenIdentifier& si, osg::GraphicsContext::ScreenSettingsList& resList) {
        WLGWlog(1) << "WLwsi:enumerateScreenSettings[" << std::endl;
        if (!checkInit()) {
            OSG_INFO << "]=failed" << std::endl;
            return;
        }
        for (int screen=0; screen<(int)_gc.n_outputs; screen++) {
            osg::GraphicsContext::ScreenIdentifier next(screen);
            osg::GraphicsContext::ScreenSettings s;
            getScreenSettings(si, s);
            resList.push_back(s);
        }
        WLGWlog(-1) << "]=1" << std::endl;
    }
    virtual osg::GraphicsContext* createGraphicsContext(osg::GraphicsContext::Traits* traits) {
        WLGWlog(1) << "WLwsi:createGraphicsContext[" << std::endl;
        if (!checkInit()) {
            WLGWlog(-1) << "]=failed" << std::endl;
            return nullptr;
        }
        osg::ref_ptr<WLGraphicsWindow> toplevel = new WLGraphicsWindow(traits, &_gc);
        WLGraphicsWindow* rv = nullptr;
        if (toplevel->valid()) {
            rv = toplevel.release();
        }
        WLGWlog(-1) << "]=" << rv << std::endl;
        return rv;
    }
    // Accessor for the wl_display
    struct wl_display* getDisplay() {
        WLGWlog(1) << "WLwsi:getDisplay[" << std::endl;
        if (!checkInit()) {
            WLGWlog(-1) << "]=failed" << std::endl;
            return nullptr;
        }
        WLGWlog(-1) << "]=" << _gc.display << std::endl;
        return _gc.display;
    }
    // Accessors for a wl_data_device & wl_data_source, so we can implement a clipboard handler elsewhere.
    struct wl_data_device* getDataDevice() {
        WLGWlog(1) << "WLwsi:getDataDevice[" << std::endl;
        if (!checkInit()) {
            WLGWlog(-1) << "]=failed" << std::endl;
            return nullptr;
        }
        // check we have a data device manager and a seat..
        if (!_gc.seat || !_gc.data_device_manager) {
            WLGWlog(-1) << "]=no seat/data_device_manager" << std::endl;
            return nullptr;
        }
        struct wl_data_device* rv = wl_data_device_manager_get_data_device(_gc.data_device_manager, _gc.seat);
        WLGWlog(-1) << "]=" << rv << std::endl;
        return rv;
    }
    struct wl_data_source* getDataSource() {
        WLGWlog(1) << "WLwsi:getDataSource[" << std::endl;
        if (!checkInit()) {
            WLGWlog(-1) << "]=failed" << std::endl;
            return nullptr;
        }
        // check we have a data device manager..
        if (!_gc.data_device_manager) {
            WLGWlog(-1) << "]=data_device_manager" << std::endl;
            return nullptr;
        }
        struct wl_data_source* rv = wl_data_device_manager_create_data_source(_gc.data_device_manager);
        WLGWlog(-1) << "]=" << rv << std::endl;
        return rv;
    }
    uint32_t getLastKeySerial() {
        return _gc.keyboard_serial;
    }

    // async work pump
    void checkAsyncWork(void) {
        // keyboard repeat?
        if (KEYBOARD_IDLE==_gc.keyboard_state) {
            // while idle, record current tick..
            _gc.keyboard_tick = tickMs();
        } else if (KEYBOARD_DELAY==_gc.keyboard_state) {
            // while in delay, wait for specified time
            uint64_t now = tickMs();
            if (now>_gc.keyboard_tick+(uint64_t)_gc.keyboard_delay) {
                // start repeating from now
                _gc.keyboard_state = KEYBOARD_REPEAT;
                _gc.keyboard_tick = now;
            }
        } else {
            // while in repeat, time to issue another repeat?
            uint64_t now = tickMs();
            if (now>_gc.keyboard_tick+(uint64_t)_gc.keyboard_repeat) {
                // yep - send release then press events
                auto win = get_window(_gc.keyboard_surface);
                win->getEventQueue()->keyRelease(_gc.keyboard_last);
                win->getEventQueue()->keyPress(_gc.keyboard_last);
                _gc.keyboard_tick = now;
                WLGWlog(0) << "<keyboard repeat: " << _gc.keyboard_last << ">" << std::endl;
            }
        }
    }

private:
    uint64_t tickMs(void) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (ts.tv_sec * 1000)+(ts.tv_nsec/1000000);
    }
};

// statically register our new windowing system at run-time
REGISTER_WINDOWINGSYSTEMINTERFACE(Wayland, WLWindowingSystemInterface)

// close namespace
    }
}

// accessor "C" functions for implementing a clipboard..
struct wl_data_device* getWaylandDataDevice() {
    com::ashbysoft::WLWindowingSystemInterface* wsi = com::ashbysoft::s_proxy_WLWindowingSystemInterface._wsi.get();
    return wsi ? wsi->getDataDevice() : nullptr;
}
struct wl_data_source* getWaylandDataSource() {
    com::ashbysoft::WLWindowingSystemInterface* wsi = com::ashbysoft::s_proxy_WLWindowingSystemInterface._wsi.get();
    return wsi ? wsi->getDataSource() : nullptr;
}
uint32_t getWaylandLastKeySerial() {
    com::ashbysoft::WLWindowingSystemInterface* wsi = com::ashbysoft::s_proxy_WLWindowingSystemInterface._wsi.get();
    return wsi ? wsi->getLastKeySerial() : 0;
}
void checkAsyncWork() {
    com::ashbysoft::WLWindowingSystemInterface* wsi = com::ashbysoft::s_proxy_WLWindowingSystemInterface._wsi.get();
    if (wsi) wsi->checkAsyncWork();
}
struct wl_display* getWaylandDisplay() {
    com::ashbysoft::WLWindowingSystemInterface* wsi = com::ashbysoft::s_proxy_WLWindowingSystemInterface._wsi.get();
    return wsi ? wsi->getDisplay() : nullptr;
}
