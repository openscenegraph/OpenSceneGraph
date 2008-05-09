#include "osggtkdrawingarea.h"

OSGGTKDrawingArea::OSGGTKDrawingArea():
_widget   (gtk_drawing_area_new()),
_glconfig (0),
_context  (0),
_drawable (0),
_state    (0),
_queue    (*getEventQueue()) {
    setCameraManipulator(new osgGA::TrackballManipulator());
}

OSGGTKDrawingArea::~OSGGTKDrawingArea() {
}

bool OSGGTKDrawingArea::createWidget(int width, int height) {
    _glconfig = gdk_gl_config_new_by_mode(static_cast<GdkGLConfigMode>(
        GDK_GL_MODE_RGBA |
        GDK_GL_MODE_DEPTH |
        GDK_GL_MODE_DOUBLE
    ));

    if(not _glconfig) {
        osg::notify(osg::FATAL) << "Fail!" << std::endl;

        return false;
    }

    gtk_widget_set_size_request(_widget, width, height);

    gtk_widget_set_gl_capability(
        _widget,
        _glconfig,
        0,
        true,
        GDK_GL_RGBA_TYPE
    );

    gtk_widget_add_events(
        _widget,
        GDK_BUTTON1_MOTION_MASK |
        GDK_BUTTON2_MOTION_MASK |
        GDK_BUTTON3_MOTION_MASK |
        GDK_POINTER_MOTION_MASK |
        GDK_BUTTON_PRESS_MASK |
        GDK_BUTTON_RELEASE_MASK |
        GDK_KEY_PRESS_MASK |
        GDK_KEY_RELEASE_MASK |
        GDK_VISIBILITY_NOTIFY_MASK
    );

    // We do this so that we don't have to suck up ALL the input to the
    // window, but instead just when the drawing area is focused.
    g_object_set(_widget, "can-focus", true, NULL);

    _connect("realize", G_CALLBACK(&OSGGTKDrawingArea::_srealize));
    _connect("unrealize", G_CALLBACK(&OSGGTKDrawingArea::_sunrealize));
    _connect("expose_event", G_CALLBACK(&OSGGTKDrawingArea::_sexpose_event));
    _connect("configure_event", G_CALLBACK(&OSGGTKDrawingArea::_sconfigure_event));
    _connect("motion_notify_event", G_CALLBACK(&OSGGTKDrawingArea::_smotion_notify_event));
    _connect("button_press_event", G_CALLBACK(&OSGGTKDrawingArea::_sbutton_press_event));
    _connect("button_release_event", G_CALLBACK(&OSGGTKDrawingArea::_sbutton_press_event));
    _connect("key_press_event", G_CALLBACK(&OSGGTKDrawingArea::_skey_press_event));

    _gw = setUpViewerAsEmbeddedInWindow(0, 0, width, height);

    return true;
}

void OSGGTKDrawingArea::_realize(GtkWidget* widget) {
    _context  = gtk_widget_get_gl_context(widget);
    _drawable = gtk_widget_get_gl_drawable(widget);

    gtkRealize();
}

void OSGGTKDrawingArea::_unrealize(GtkWidget* widget) {
    gtkUnrealize();
}

bool OSGGTKDrawingArea::_expose_event(GtkWidget* widget, GdkEventExpose* event) {
    if(not gtkGLBegin()) return false;

    frame();

    gtkGLSwap();
    gtkGLEnd();

    return gtkExpose();
}

bool OSGGTKDrawingArea::_configure_event(GtkWidget* widget, GdkEventConfigure* event) {
    gtkGLBegin();

    _queue.windowResize(0, 0, event->width, event->height);

    _gw->resized(0, 0, event->width, event->height);

    gtkGLEnd();

    return gtkConfigure(event->width, event->height);
}

bool OSGGTKDrawingArea::_motion_notify_event(GtkWidget* widget, GdkEventMotion* event) {
    _state = event->state;

    _queue.mouseMotion(event->x, event->y);

    return gtkMotionNotify(event->x, event->y);
}

bool OSGGTKDrawingArea::_button_press_event(GtkWidget* widget, GdkEventButton* event) {
    _state = event->state;

    if(event->type == GDK_BUTTON_PRESS) {
        if(event->button == 1) gtk_widget_grab_focus(_widget);

        _queue.mouseButtonPress(event->x, event->y, event->button);

        return gtkButtonPress(event->x, event->y, event->button);
    }

    else if(event->type == GDK_BUTTON_RELEASE) {
        _queue.mouseButtonRelease(event->x, event->y, event->button);

        return gtkButtonRelease(event->x, event->y, event->button);
    }

    else return false;
}

bool OSGGTKDrawingArea::_key_press_event(GtkWidget* widget, GdkEventKey* event) {
    _state = event->state;

    if(event->type == GDK_KEY_PRESS) {
        _queue.keyPress(event->keyval);

        return gtkKeyPress(event->keyval);
    }

    else if(event->type == GDK_KEY_RELEASE) {
        _queue.keyRelease(event->keyval);

        return gtkKeyRelease(event->keyval);
    }

    else return false;
}
