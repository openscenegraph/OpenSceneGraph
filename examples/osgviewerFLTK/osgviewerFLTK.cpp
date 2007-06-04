// C++ source file - (C) 2003 Robert Osfield, released under the OSGPL.
// (C) 2005 Mike Weiblen http://mew.cx/ released under the OSGPL.
// Simple example using GLUT to create an OpenGL window and OSG for rendering.
// Derived from osgGLUTsimple.cpp and osgkeyboardmouse.cpp

#include <osgViewer/Viewer>
#include <osgViewer/StatsHandler>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>

#include <iostream>

class AdapterWidget : public Fl_Gl_Window
{
public:

    AdapterWidget(int x, int y, int w, int h, const char *label=0);
    virtual ~AdapterWidget() {}

    osgViewer::GraphicsWindow* getGraphicsWindow() { return _gw.get(); }
    const osgViewer::GraphicsWindow* getGraphicsWindow() const { return _gw.get(); }

    virtual void resize(int x, int y, int w, int h);

protected:

    virtual int handle(int event);
    
    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> _gw;
};

AdapterWidget::AdapterWidget(int x, int y, int w, int h, const char *label):
    Fl_Gl_Window(x, y, w, h, label)
{
    _gw = new osgViewer::GraphicsWindowEmbedded(x,y,w,h);
}

void AdapterWidget::resize(int x, int y, int w, int h)
{
    _gw->getEventQueue()->windowResize(x, y, w, h );
    _gw->resized(x,y,w,h);

    Fl_Gl_Window::resize(x,y,w,h);

}

int AdapterWidget::handle(int event)
{
    switch(event){
        case FL_PUSH:
            _gw->getEventQueue()->mouseButtonPress(Fl::event_x(), Fl::event_y(), Fl::event_button());
            return 1;
        case FL_MOVE:
        case FL_DRAG:
            _gw->getEventQueue()->mouseMotion(Fl::event_x(), Fl::event_y());
            return 1;
        case FL_RELEASE:
            _gw->getEventQueue()->mouseButtonRelease(Fl::event_x(), Fl::event_y(), Fl::event_button());
            return 1;
        case FL_KEYDOWN:
            _gw->getEventQueue()->keyPress((osgGA::GUIEventAdapter::KeySymbol)Fl::event_key());
            return 1;
        case FL_KEYUP:
            _gw->getEventQueue()->keyRelease((osgGA::GUIEventAdapter::KeySymbol)Fl::event_key());
            return 1;
        default:
            // pass other events to the base class
            return Fl_Gl_Window::handle(event);
    }
}

void idle_cb()
{
    Fl::redraw();
}


class ViewerFLTK : public osgViewer::Viewer, public AdapterWidget
{
    public:
        ViewerFLTK(int x, int y, int w, int h, const char *label=0):
            AdapterWidget(x,y,w,h,label)
            {
                getCamera()->setViewport(new osg::Viewport(0,0,w,h));
                getCamera()->setGraphicsContext(getGraphicsWindow());
                setThreadingModel(osgViewer::Viewer::SingleThreaded);
            }

    protected:
        virtual void draw() { frame(); }

};

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


    ViewerFLTK viewerWindow(100,100,800,600);
    viewerWindow.resizable(&viewerWindow);

    viewerWindow.setSceneData(loadedModel.get());
    viewerWindow.setCameraManipulator(new osgGA::TrackballManipulator);
    viewerWindow.addEventHandler(new osgViewer::StatsHandler);
    
    viewerWindow.show();
    
    Fl::set_idle(idle_cb);
    
    return Fl::run();
}
