#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>

// This is an implementation of SimpleViewer that is designed to be subclassed
// and used as a GtkDrawingArea in a GTK application. Because of the implemention
// of GTK, I was unable to derive from GtkWidget and instead had to "wrap" it.
// Conceptually, however, you can think of an OSGGTKDrawingArea as both an OSG
// Viewer AND GtkDrawingArea.
//
// While it is possible to use this class directly, it won't end up doing anything
// interesting without calls to queueDraw, which ideally are done in the user's
// subclass implementation (see: osgviewerGTK).
class OSGGTKDrawingArea : public osgViewer::Viewer {
    GtkWidget*     _widget;
    GdkGLConfig*   _glconfig;
    GdkGLContext*  _context;
    GdkGLDrawable* _drawable;
    
    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> _gw;

    unsigned int _state;

    osgGA::EventQueue& _queue;

    static OSGGTKDrawingArea* _self(gpointer self) {
        return static_cast<OSGGTKDrawingArea*>(self);
    }

    // A simple helper function to connect us to the various GTK signals.
    void _connect(const char* name, GCallback callback) {
        g_signal_connect(G_OBJECT(_widget), name, callback, this);
    }

    void _realize             (GtkWidget*);
    void _unrealize           (GtkWidget*);
    bool _expose_event        (GtkWidget*, GdkEventExpose*);
    bool _configure_event     (GtkWidget*, GdkEventConfigure*);
    bool _motion_notify_event (GtkWidget*, GdkEventMotion*);
    bool _button_press_event  (GtkWidget*, GdkEventButton*);
    bool _key_press_event     (GtkWidget*, GdkEventKey*);

    // The following functions are static "wrappers" so that we can invoke the
    // bound methods of a class instance by passing the "this" pointer as the
    // self argument and invoking it explicitly.
    static void _srealize(GtkWidget* widget, gpointer self) {
        _self(self)->_realize(widget);
    }

    static void _sunrealize(GtkWidget* widget, gpointer self) {
        _self(self)->_unrealize(widget);
    }

    static bool _sexpose_event(GtkWidget* widget, GdkEventExpose* expose, gpointer self) {
        return _self(self)->_expose_event(widget, expose);
    }

    static bool _sconfigure_event(
        GtkWidget*         widget,
        GdkEventConfigure* event,
        gpointer           self
    ) {
        return _self(self)->_configure_event(widget, event);
    }

    static bool _smotion_notify_event(
        GtkWidget*      widget,
        GdkEventMotion* event,
        gpointer        self
    ) {
        return _self(self)->_motion_notify_event(widget, event);
    }

    static bool _sbutton_press_event(
        GtkWidget*      widget,
        GdkEventButton* event,
        gpointer        self
    ) {
        return _self(self)->_button_press_event(widget, event);
    }

    static bool _skey_press_event(
        GtkWidget*   widget,
        GdkEventKey* event,
        gpointer     self
    ) {
        return _self(self)->_key_press_event(widget, event);
    }

protected:
    // You can override these in your subclass if you'd like. :)
    // Right now they're fairly uninformative, but they could be easily extended.
    // Note that the "state" information isn't passed around to each function
    // but is instead stored and abstracted internally. See below.

    virtual void gtkRealize   () {};
    virtual void gtkUnrealize () {};
    virtual bool gtkExpose    () {
        return true;
    };

    // The new width and height.
    virtual bool gtkConfigure(int, int) {
        return true;
    };

    // The "normalized" coordinates of the mouse.
    virtual bool gtkMotionNotify(double, double) {
        return true;
    };

    // The "normalized" coordinates of the mouse and the mouse button code on down.
    virtual bool gtkButtonPress(double, double, unsigned int) {
        return true;
    };

    // The "normalized" coordinates of the mouse and mouse button code on release.
    virtual bool gtkButtonRelease(double, double, unsigned int) {
        return true;
    }

    // The X key value on down.
    virtual bool gtkKeyPress(unsigned int) {
        return true;
    };

    // The X key value on release.
    virtual bool gtkKeyRelease(unsigned int) {
        return true;
    };

    // These functions wrap state tests of the most recent state in the
    // GtkDrawingArea.

    inline bool stateShift() {
        return _state & GDK_SHIFT_MASK;
    }

    inline bool stateLock() {
        return _state & GDK_LOCK_MASK;
    }

    inline bool stateControl() {
        return _state & GDK_CONTROL_MASK;
    }

    inline bool stateMod() {
        return _state & (
            GDK_MOD1_MASK |
            GDK_MOD2_MASK |
            GDK_MOD3_MASK |
            GDK_MOD4_MASK |
            GDK_MOD5_MASK
        );
    }

    inline bool stateButton() {
        return _state & (
            GDK_BUTTON1_MASK |
            GDK_BUTTON2_MASK |
            GDK_BUTTON3_MASK |
            GDK_BUTTON4_MASK |
            GDK_BUTTON5_MASK
        );
    }

public:
    OSGGTKDrawingArea  ();
    ~OSGGTKDrawingArea ();

    bool createWidget(int, int);

    GtkWidget* getWidget() {
        return _widget;
    }

    bool gtkGLBegin() {
        if(_drawable and _context) return gdk_gl_drawable_gl_begin(_drawable, _context);

        else return false;
    }

    void gtkGLEnd() {
        if(_drawable) gdk_gl_drawable_gl_end(_drawable);
    }

    // Because of GTK's internal double buffering, I'm not sure if we're really
    // taking advantage of OpenGL's internal swapping.
    bool gtkGLSwap() {
        if(_drawable and gdk_gl_drawable_is_double_buffered(_drawable)) {
            gdk_gl_drawable_swap_buffers(_drawable);

            return true;
        }

        else {
            glFlush();

            return false;
        }
    }

    void queueDraw() {
        gtk_widget_queue_draw(_widget);
    }
};
