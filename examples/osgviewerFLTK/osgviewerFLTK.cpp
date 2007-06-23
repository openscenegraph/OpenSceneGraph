/* OpenSceneGraph example, osganimate.
*
*  C++ source file - (C) 2003 Robert Osfield.
*  (C) 2005 Mike Weiblen http://mew.cx/
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/


#include <osgViewer/Viewer>
#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>
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
                getCamera()->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(w)/static_cast<double>(h), 1.0f, 10000.0f);
                getCamera()->setGraphicsContext(getGraphicsWindow());
                setThreadingModel(osgViewer::Viewer::SingleThreaded);
            }

    protected:
        virtual void draw() { frame(); }

};

class CompositeViewerFLTK : public osgViewer::CompositeViewer, public AdapterWidget
{
    public:

        CompositeViewerFLTK(int x, int y, int w, int h, const char *label=0):
            AdapterWidget(x,y,w,h,label)
        {
            setThreadingModel(osgViewer::CompositeViewer::SingleThreaded);
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

    osg::ArgumentParser arguments(&argc, argv);

    // load the scene.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    if (!loadedModel)
    {
        std::cout << argv[0] <<": No data loaded." << std::endl;
        return 1;
    }


    if (arguments.read("--CompositeViewer"))
    {
        unsigned int width = 1024;
        unsigned int height = 800;

        CompositeViewerFLTK viewerWindow(100,100,width,height);
        viewerWindow.resizable(&viewerWindow);
        
        {
            osgViewer::View* view1 = new osgViewer::View;
            view1->getCamera()->setGraphicsContext(viewerWindow.getGraphicsWindow());
            view1->getCamera()->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(width)/static_cast<double>(height/2), 1.0, 1000.0);
            view1->getCamera()->setViewport(new osg::Viewport(0,0,width,height/2));
            view1->setCameraManipulator(new osgGA::TrackballManipulator);
            view1->setSceneData(loadedModel.get());
            
            viewerWindow.addView(view1);
        }
        
        {
            osgViewer::View* view2 = new osgViewer::View;
            view2->getCamera()->setGraphicsContext(viewerWindow.getGraphicsWindow());
            view2->getCamera()->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(width)/static_cast<double>(height/2), 1.0, 1000.0);
            view2->getCamera()->setViewport(new osg::Viewport(0,height/2,width,height/2));
            view2->setCameraManipulator(new osgGA::TrackballManipulator);
            view2->setSceneData(loadedModel.get());
            
            viewerWindow.addView(view2);
        }

        viewerWindow.show();

        Fl::set_idle(idle_cb);

        return Fl::run();
    }
    else
    {

        ViewerFLTK viewerWindow(100,100,800,600);
        viewerWindow.resizable(&viewerWindow);

        viewerWindow.setSceneData(loadedModel.get());
        viewerWindow.setCameraManipulator(new osgGA::TrackballManipulator);
        viewerWindow.addEventHandler(new osgViewer::StatsHandler);

        viewerWindow.show();

        Fl::set_idle(idle_cb);

        return Fl::run();
    }
}
