#include <string.h>

#include <iostream>
#include <string>
#include <osg/Stats>
#include <osgDB/ReadFile>

#include "osggtkdrawingarea.h"

const char* HELP_TEXT =
    "Use CTRL or SHIFT plus right-click to pull up a fake menu.\n"
    "Use the standard TrackballManipulator keys to rotate the loaded\n"
    "model (with caveats; the model won't keep rotating).\n"
    "\n"
    "<b>OpenSceneGraph Project, 2008</b>"
;

// If you want to see how to connect class method to callbacks, take a look at the
// implementation of OSGGTKDrawingArea. It's dirty, but it's the only way I could
// come up with.
bool activate(GtkWidget* widget, gpointer) {
    GtkWidget* label = gtk_bin_get_child(GTK_BIN(widget));

    std::cout << "MENU: " << gtk_label_get_label(GTK_LABEL(label)) << std::endl;

    return true;
}

// Our derived OSGGTKDrawingArea "widget." Redraws occur while the mouse buttons
// are held down and mouse motion is detected.
//
// This is the easiest way to demonstrate the use of OSGGTKDrawingArea. We override
// a few of the event methods to setup our menu and to issue redraws. Note that an
// unmodified OSGGTKDrawingArea never calls queueDraw, so OSG is never asked to render
// itself.
class ExampleOSGGTKDrawingArea : public OSGGTKDrawingArea {
    GtkWidget* _menu;
    
    unsigned int _tid;

    // A helper function to easily setup our menu entries.
    void _menuAdd(const std::string& title) {
        GtkWidget* item = gtk_menu_item_new_with_label(title.c_str());
        
        gtk_menu_shell_append(GTK_MENU_SHELL(_menu), item);

        g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(activate), 0);
    }

    bool _clicked(GtkWidget* widget) {
        const char* text = gtk_label_get_label(
            GTK_LABEL(gtk_bin_get_child(GTK_BIN(widget)))
        );

        if(not strncmp(text, "Close", 5)) gtk_main_quit();
    
        else if(not strncmp(text, "Open File", 9)) {
            GtkWidget* of = gtk_file_chooser_dialog_new(
                "Please select an OSG file...",
                GTK_WINDOW(gtk_widget_get_toplevel(getWidget())),
                GTK_FILE_CHOOSER_ACTION_OPEN,
                GTK_STOCK_CANCEL,
                GTK_RESPONSE_CANCEL,
                GTK_STOCK_OPEN,
                GTK_RESPONSE_ACCEPT,
                NULL
            );

            if(gtk_dialog_run(GTK_DIALOG(of)) == GTK_RESPONSE_ACCEPT) {
                char* file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(of));

                osg::ref_ptr<osg::Node> model = osgDB::readRefNodeFile(file);

                if(model.valid()) {
                    setSceneData(model.get());
                
                    queueDraw();
                }

                g_free(file);
            }
            
            gtk_widget_destroy(of);
        }

        // Assume we're wanting FPS toggling.
        else {
            if(not _tid) {
                _tid = g_timeout_add(
                    15,
                    (GSourceFunc)(ExampleOSGGTKDrawingArea::timeout),
                    this
                );

                gtk_button_set_label(GTK_BUTTON(widget), "Toggle 60 FPS (off)");
            }

            else {
                g_source_remove(_tid);
                gtk_button_set_label(GTK_BUTTON(widget), "Toggle 60 FPS (on)");

                _tid = 0;
            }
        }

        return true;
    }

protected:
    // Check right-click release to see if we need to popup our menu.
    bool gtkButtonRelease(double, double, unsigned int button) {
        if(button == 3 and (stateControl() or stateShift())) gtk_menu_popup(
            GTK_MENU(_menu),
            0,
            0,
            0,
            0,
            button,
            0
        );

        return true;
    }

    // Our "main" drawing pump. Since our app is just a model viewer, we use
    // click+motion as our criteria for issuing OpenGL refreshes.
    bool gtkMotionNotify(double, double) {
        if(stateButton()) queueDraw();

        return true;
    }

public:
    ExampleOSGGTKDrawingArea():
    OSGGTKDrawingArea (),
    _menu             (gtk_menu_new()),
    _tid              (0) {
        _menuAdd("Option");
        _menuAdd("Another Option");
        _menuAdd("Still More Options");

        gtk_widget_show_all(_menu);

        getCamera()->setStats(new osg::Stats("omg"));
    }

    ~ExampleOSGGTKDrawingArea() {}

    // Public so that we can use this as a callback in main().
    static bool clicked(GtkWidget* widget, gpointer self) {
        return static_cast<ExampleOSGGTKDrawingArea*>(self)->_clicked(widget);
    }

    //static gboolean timeout(GtkWidget* widget) {
    static bool timeout(void* self) {
        static_cast<ExampleOSGGTKDrawingArea*>(self)->queueDraw();

        return true;
    }
};

// Our main() function! FINALLY! Most of this code is GTK stuff, so it's mostly boilerplate.
// If we wanted to get real jiggy with it we could use Glade and cut down about 20 lines of
// code or so.
int main(int argc, char** argv) {
    gtk_init(&argc, &argv);
    gtk_gl_init(&argc, &argv);

    ExampleOSGGTKDrawingArea da;

    if(da.createWidget(640, 480)) {
        if(argc >= 2) {
            osg::ref_ptr<osg::Node> model = osgDB::readRefNodeFile(argv[1]);

            if(model.valid()) da.setSceneData(model.get());
        }

        GtkWidget* window    = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        GtkWidget* vbox1     = gtk_vbox_new(false, 3);
        GtkWidget* vbox2     = gtk_vbox_new(false, 3);
        GtkWidget* hbox      = gtk_hbox_new(false, 3);
        GtkWidget* label     = gtk_label_new("");
        GtkWidget* buttons[] = {
            gtk_button_new_with_label("Open File"),
            gtk_button_new_with_label("Toggle 60 FPS (on)"),
            gtk_button_new_with_label("Close")
        };

        gtk_label_set_use_markup(GTK_LABEL(label), true);
        gtk_label_set_label(GTK_LABEL(label), HELP_TEXT);

        for(unsigned int i = 0; i < sizeof(buttons) / sizeof(GtkWidget*); i++) {
            gtk_box_pack_start(
                GTK_BOX(vbox2),
                buttons[i],
                false,
                false,
                0
            );

            g_signal_connect(
                G_OBJECT(buttons[i]),
                "clicked",
                G_CALLBACK(ExampleOSGGTKDrawingArea::clicked),
                &da
            );
        }

        gtk_window_set_title(GTK_WINDOW(window), "osgviewerGTK");

        gtk_box_pack_start(GTK_BOX(hbox), vbox2, true, true, 2);
        gtk_box_pack_start(GTK_BOX(hbox), label, true, true, 2);

        gtk_box_pack_start(GTK_BOX(vbox1), da.getWidget(), true, true, 2);
        gtk_box_pack_start(GTK_BOX(vbox1), hbox, false, false, 2);

        gtk_container_set_reallocate_redraws(GTK_CONTAINER(window), true);
        gtk_container_add(GTK_CONTAINER(window), vbox1);

        g_signal_connect(
            G_OBJECT(window),
            "delete_event",
            G_CALLBACK(gtk_main_quit),
            0
        );

        gtk_widget_show_all(window);
        gtk_main();
    }

    else return 1;

    return 0;
}
