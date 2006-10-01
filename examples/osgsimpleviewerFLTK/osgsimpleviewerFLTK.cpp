// C++ source file - (C) 2003 Robert Osfield, released under the OSGPL.
// (C) 2005 Mike Weiblen http://mew.cx/ released under the OSGPL.
// Simple example using GLUT to create an OpenGL window and OSG for rendering.
// Derived from osgGLUTsimple.cpp and osgkeyboardmouse.cpp

#include <osgGA/SimpleViewer>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>

#include <iostream>

class Fl_OSG_Widget : public Fl_Gl_Window, public osgGA::SimpleViewer
{
public:

    Fl_OSG_Widget(int x, int y, int w, int h, const char *label=0);
    virtual ~Fl_OSG_Widget() {}

    virtual void resize(int x, int y, int w, int h);

protected:

    virtual int handle(int event);
    virtual void draw();

};

Fl_OSG_Widget::Fl_OSG_Widget(int x, int y, int w, int h, const char *label):
    Fl_Gl_Window(x, y, w, h, label)
{
    getEventQueue()->windowResize(x, y, w, h );
}

void Fl_OSG_Widget::resize(int x, int y, int w, int h)
{
    getEventQueue()->windowResize(x, y, w, h );
    Fl_Gl_Window::resize(x,y,w,h);
}

int Fl_OSG_Widget::handle(int event)
{
    switch(event){
        case FL_PUSH:
            getEventQueue()->mouseButtonPress(Fl::event_x(), Fl::event_y(), Fl::event_button());
            return 1;
        case FL_MOVE:
        case FL_DRAG:
            getEventQueue()->mouseMotion(Fl::event_x(), Fl::event_y());
            return 1;
        case FL_RELEASE:
            getEventQueue()->mouseButtonRelease(Fl::event_x(), Fl::event_y(), Fl::event_button());
            return 1;
        case FL_KEYDOWN:
            getEventQueue()->keyPress((osgGA::GUIEventAdapter::KeySymbol)Fl::event_key());
            return 1;
        case FL_KEYUP:
            getEventQueue()->keyRelease((osgGA::GUIEventAdapter::KeySymbol)Fl::event_key());
            return 1;
        default:
            // pass other events to the base class
            return Fl_Gl_Window::handle(event);
    }
}

void Fl_OSG_Widget::draw()
{
    frame();
}

void idle_cb()
{
    Fl::redraw();
}

int main( int argc, char **argv )
{
    
    if (argc<2)
    {
        std::cout << argv[0] <<": requires filename argument." << std::endl;
        return 1;
    }

    // load the scene.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(argv[1]);
    if (!loadedModel)
    {
        std::cout << argv[0] <<": No data loaded." << std::endl;
        return 1;
    }


    Fl_OSG_Widget viewerWindow(100,100,800,600);
    viewerWindow.resizable(&viewerWindow);

    viewerWindow.setSceneData(loadedModel.get());
    viewerWindow.setCameraManipulator(new osgGA::TrackballManipulator);

    viewerWindow.show();
    
    Fl::set_idle(idle_cb);
    
    return Fl::run();
}
