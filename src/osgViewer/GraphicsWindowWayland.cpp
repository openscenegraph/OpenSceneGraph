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
    // shared cursor
    wl_cursor* cursor;
    // lookup map of windows
    std::map<struct wl_surface*, WLGraphicsWindow*> windows;
    // current seat caps
    uint32_t seat_caps;
    // current key modifiers
    uint32_t keyboard_mods;
    // last pointer position
    float pointer_x;
    float pointer_y;
    // current active surfaces (can be null)
    struct wl_surface* keyboard_surface;
    struct wl_surface* pointer_surface;
    uint32_t keyboard_serial;
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
    bool floating;
    int width;
    int height;
    bool pending_config;
};
// keyboard mapping from linux event scan codes to osgGA codes
static std::map<uint32_t, int> s_keymap;

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
            xdg_toplevel_set_fullscreen(_gw.xdg_toplevel, _gw.gc->output[_traits->screenNum]);
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
        // bool useCursor - TODO, cursor support HOW?
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
        // create cursor surface
        _gw.cursor_surface = wl_compositor_create_surface(_gw.gc->compositor);
        wl_surface_attach(_gw.cursor_surface, wl_cursor_image_get_buffer(_gw.gc->cursor->images[0]), 0, 0);
        wl_surface_commit(_gw.cursor_surface);
        _valid = true;
        // add ourselves to window map
        gc->windows[_gw.surface] = this;
        // indicate that we're not configured yet
        _gw.width = _traits->width;
        _gw.height = _traits->height;
        _gw.pending_config = true;
        WLGWlog(-1) << _logname << "]=valid" << std::endl;
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
        // set our cursor
        wl_pointer_set_cursor(wl_pointer, serial, _gw.cursor_surface, 0, 0);
        WLGWlog(0) << _logname << "::pointerEnter" << std::endl;
    }
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
            xdg_toplevel_set_fullscreen(_gw.xdg_toplevel, _gw.gc->output[_traits->screenNum]);
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
        wl_display_dispatch_pending(_gw.gc->display);
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
        WLGWlog(0) << "<keyboard enter: " << surface << ">" << std::endl;
    }
    static void keyboard_leave(void* data, wl_keyboard* wl_keyboard, uint32_t serial, wl_surface* surface) {
        WLWindowingSystemInterface* obj = (WLWindowingSystemInterface*) data;
        obj->_gc.keyboard_surface = nullptr;
        WLGWlog(0) << "<keyboard leave: " << surface << ">" << std::endl;
    }
    static void keyboard_map(void* data, wl_keyboard* wl_keyboard, uint32_t format, int32_t fd, uint32_t size) {}
    static void keyboard_repeat(void* data, wl_keyboard* wl_keyboard, int32_t rate, int32_t delay) {}
    static void keyboard_modifiers(void* data, wl_keyboard* wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
        WLWindowingSystemInterface* obj = (WLWindowingSystemInterface*) data;
        obj->_gc.keyboard_mods = mods_depressed;
        WLGWlog(0) << "<keymods: " << mods_depressed << ',' << mods_latched << ',' << mods_locked << ">" << std::endl;
    }
    static void keyboard_key(void* data, wl_keyboard* wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
        WLWindowingSystemInterface* obj = (WLWindowingSystemInterface*) data;
        int mapped = (int)key;
        auto kit = s_keymap.find(key);
        if (kit != s_keymap.end())
            mapped = kit->second;
        int shifted = mapped;
        // translate 'a-z[]'=>'A-Z{}' if shift is held
        if ((obj->_gc.keyboard_mods & 1) && mapped>='a' && mapped<='z')
            shifted = mapped-'a'+'A';
        if ((obj->_gc.keyboard_mods & 1) && (mapped=='[' || mapped==']'))
            shifted = mapped=='['?'{':'}';
        if (auto win = obj->get_window(obj->_gc.keyboard_surface)) {
            osgGA::GUIEventAdapter* event;
            if (state)
                event = win->getEventQueue()->keyPress(shifted, mapped);
            else
                event = win->getEventQueue()->keyRelease(shifted, mapped);
            WLGWlog(0) << (state?"<keypress: ":"<keyrelease: ") << key << "=>" << shifted << '/' << mapped << ',' << state
                << ",mods=" << event->getModKeyMask() << ">" << std::endl;
        }
    }
    static void pointer_enter(void *data, wl_pointer* wl_pointer, uint32_t serial, wl_surface* surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
        WLWindowingSystemInterface* obj = (WLWindowingSystemInterface*) data;
        obj->_gc.pointer_surface = surface;
        if (auto win = obj->get_window(surface)) {
            win->pointerEnter(wl_pointer, serial);
        }
        WLGWlog(0) << "<pointer enter: " << surface << ">" << std::endl;
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
            // load cursor & monitor seat capabilities if we have one..
            if (_gc.seat) {
                wl_cursor_theme* theme = wl_cursor_theme_load(nullptr, 24, _gc.shm);
                _gc.cursor = wl_cursor_theme_get_cursor(theme, "left_ptr");
                WLGWlog(0) << "WLwsi: loaded cursor: " << _gc.cursor << std::endl;
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
    WLWindowingSystemInterface() {
        WLGWlog(0) << "WLWindowingSystemInterface<init>" << std::endl;
        _wl_init = false;   // lazy init as we may never get called..
        // fill out keymap
        s_keymap[KEY_ESC]  = osgGA::GUIEventAdapter::KEY_Escape;
        // Some of these should be osgGA::GUIEventAdapter::KEY_<n> but that collides
        // with the macros used in input-event-codes.h and really fouls up :(
        s_keymap[KEY_1] = '1';
        s_keymap[KEY_2] = '2';
        s_keymap[KEY_3] = '3';
        s_keymap[KEY_4] = '4';
        s_keymap[KEY_5] = '5';
        s_keymap[KEY_6] = '6';
        s_keymap[KEY_7] = '7';
        s_keymap[KEY_8] = '8';
        s_keymap[KEY_9] = '9';
        s_keymap[KEY_0] = '0';
        s_keymap[KEY_MINUS] = osgGA::GUIEventAdapter::KEY_Minus;
        s_keymap[KEY_EQUAL] = osgGA::GUIEventAdapter::KEY_Equals;
        s_keymap[KEY_BACKSPACE] = osgGA::GUIEventAdapter::KEY_BackSpace;
        s_keymap[KEY_TAB] = osgGA::GUIEventAdapter::KEY_Tab;
        s_keymap[KEY_Q] = 'q';
        s_keymap[KEY_W] = 'w';
        s_keymap[KEY_E] = 'e';
        s_keymap[KEY_R] = 'r';
        s_keymap[KEY_T] = 't';
        s_keymap[KEY_Y] = 'y';
        s_keymap[KEY_U] = 'u';
        s_keymap[KEY_I] = 'i';
        s_keymap[KEY_O] = 'o';
        s_keymap[KEY_P] = 'p';
        s_keymap[KEY_LEFTBRACE] = osgGA::GUIEventAdapter::KEY_Leftbracket;
        s_keymap[KEY_RIGHTBRACE] = osgGA::GUIEventAdapter::KEY_Rightbracket;
        s_keymap[KEY_ENTER] = osgGA::GUIEventAdapter::KEY_Return;
        s_keymap[KEY_LEFTCTRL] = osgGA::GUIEventAdapter::KEY_Control_L;
        s_keymap[KEY_A] = 'a';
        s_keymap[KEY_S] = 's';
        s_keymap[KEY_D] = 'd';
        s_keymap[KEY_F] = 'f';
        s_keymap[KEY_G] = 'g';
        s_keymap[KEY_H] = 'h';
        s_keymap[KEY_J] = 'j';
        s_keymap[KEY_K] = 'k';
        s_keymap[KEY_L] = 'l';
        s_keymap[KEY_SEMICOLON] = osgGA::GUIEventAdapter::KEY_Semicolon;
        s_keymap[KEY_APOSTROPHE] = osgGA::GUIEventAdapter::KEY_At;
        s_keymap[KEY_GRAVE] = osgGA::GUIEventAdapter::KEY_Backquote;
        s_keymap[KEY_LEFTSHIFT] = osgGA::GUIEventAdapter::KEY_Shift_L;
        s_keymap[KEY_BACKSLASH] = osgGA::GUIEventAdapter::KEY_Backslash;
        s_keymap[KEY_Z] = 'z';
        s_keymap[KEY_X] = 'x';
        s_keymap[KEY_C] = 'c';
        s_keymap[KEY_V] = 'v';
        s_keymap[KEY_B] = 'b';
        s_keymap[KEY_N] = 'n';
        s_keymap[KEY_M] = 'm';
        s_keymap[KEY_COMMA] = osgGA::GUIEventAdapter::KEY_Comma;
        s_keymap[KEY_DOT] = osgGA::GUIEventAdapter::KEY_Period;
        s_keymap[KEY_SLASH] = osgGA::GUIEventAdapter::KEY_Slash;
        s_keymap[KEY_RIGHTSHIFT] = osgGA::GUIEventAdapter::KEY_Shift_R;
        s_keymap[KEY_KPASTERISK] = osgGA::GUIEventAdapter::KEY_KP_Multiply;
        s_keymap[KEY_LEFTALT] = osgGA::GUIEventAdapter::KEY_Alt_L;
        s_keymap[KEY_SPACE] = osgGA::GUIEventAdapter::KEY_Space;
        s_keymap[KEY_CAPSLOCK] = osgGA::GUIEventAdapter::KEY_Caps_Lock;
        s_keymap[KEY_F1] = 0xffbe;
        s_keymap[KEY_F2] = 0xffbf;
        s_keymap[KEY_F3] = 0xffc0;
        s_keymap[KEY_F4] = 0xffc1;
        s_keymap[KEY_F5] = 0xffc2;
        s_keymap[KEY_F6] = 0xffc3;
        s_keymap[KEY_F7] = 0xffc4;
        s_keymap[KEY_F8] = 0xffc5;
        s_keymap[KEY_F9] = 0xffc6;
        s_keymap[KEY_F10] = 0xffc7;
        s_keymap[KEY_NUMLOCK] = osgGA::GUIEventAdapter::KEY_Num_Lock;
        s_keymap[KEY_SCROLLLOCK] = osgGA::GUIEventAdapter::KEY_Scroll_Lock;
        s_keymap[KEY_KP7] = osgGA::GUIEventAdapter::KEY_KP_7;
        s_keymap[KEY_KP8] = osgGA::GUIEventAdapter::KEY_KP_8;
        s_keymap[KEY_KP9] = osgGA::GUIEventAdapter::KEY_KP_9;
        s_keymap[KEY_KPMINUS] = osgGA::GUIEventAdapter::KEY_KP_Subtract;
        s_keymap[KEY_KP4] = osgGA::GUIEventAdapter::KEY_KP_4;
        s_keymap[KEY_KP5] = osgGA::GUIEventAdapter::KEY_KP_5;
        s_keymap[KEY_KP6] = osgGA::GUIEventAdapter::KEY_KP_6;
        s_keymap[KEY_KPPLUS] = osgGA::GUIEventAdapter::KEY_KP_Add;
        s_keymap[KEY_KP1] = osgGA::GUIEventAdapter::KEY_KP_1;
        s_keymap[KEY_KP2] = osgGA::GUIEventAdapter::KEY_KP_2;
        s_keymap[KEY_KP3] = osgGA::GUIEventAdapter::KEY_KP_3;
        s_keymap[KEY_KP0] = osgGA::GUIEventAdapter::KEY_KP_0;
        s_keymap[KEY_KPDOT] = osgGA::GUIEventAdapter::KEY_KP_Decimal;
        s_keymap[KEY_F11] = 0xffc8;
        s_keymap[KEY_F12] = 0xffc9;
        s_keymap[KEY_KPENTER] = osgGA::GUIEventAdapter::KEY_KP_Enter;
        s_keymap[KEY_RIGHTCTRL] = osgGA::GUIEventAdapter::KEY_KP_Enter;
        s_keymap[KEY_KPSLASH] = osgGA::GUIEventAdapter::KEY_KP_Divide;
        s_keymap[KEY_RIGHTALT] = osgGA::GUIEventAdapter::KEY_Alt_R;
        s_keymap[KEY_HOME] = osgGA::GUIEventAdapter::KEY_Home;
        s_keymap[KEY_UP] = osgGA::GUIEventAdapter::KEY_Up;
        s_keymap[KEY_PAGEUP] = osgGA::GUIEventAdapter::KEY_Page_Up;
        s_keymap[KEY_LEFT] = osgGA::GUIEventAdapter::KEY_Left;
        s_keymap[KEY_RIGHT] = osgGA::GUIEventAdapter::KEY_Right;
        s_keymap[KEY_END] = osgGA::GUIEventAdapter::KEY_End;
        s_keymap[KEY_DOWN] = osgGA::GUIEventAdapter::KEY_Down;
        s_keymap[KEY_PAGEDOWN] = osgGA::GUIEventAdapter::KEY_Page_Down;
        s_keymap[KEY_INSERT] = osgGA::GUIEventAdapter::KEY_Insert;
        s_keymap[KEY_DELETE] = osgGA::GUIEventAdapter::KEY_Delete;
        s_keymap[KEY_LEFTMETA] = osgGA::GUIEventAdapter::KEY_Meta_L;
        s_keymap[KEY_RIGHTMETA] = osgGA::GUIEventAdapter::KEY_Meta_R;
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
struct wl_display* getWaylandDisplay() {
    com::ashbysoft::WLWindowingSystemInterface* wsi = com::ashbysoft::s_proxy_WLWindowingSystemInterface._wsi.get();
    return wsi ? wsi->getDisplay() : nullptr;
}
