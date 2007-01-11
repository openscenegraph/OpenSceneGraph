// C++ source file - (C) 2003 Robert Osfield, released under the OSGPL.
//
// Simple example of use of Producer::RenderSurface to create an OpenGL
// graphics window, and OSG for rendering.

#include <osgUtil/SceneView>
#include <osgDB/ReadFile>
#include <osgViewer/SimpleViewer>
#include <osgViewer/GraphicsWindow>
#include <iostream>

class ExitHandler : public osgGA::GUIEventHandler 
{
public: 

    ExitHandler():
        _done(false) {}
        
    bool done() const { return _done; }    

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&)
    {
        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYUP):
            {
                if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Escape)
                {
                    _done = true;
                }
                return false;
            }
            case(osgGA::GUIEventAdapter::CLOSE_WINDOW):
            case(osgGA::GUIEventAdapter::QUIT_APPLICATION):
            {
                _done = true;
            }
            default: break;
        }
        
        return false;
    }
    
    bool _done;
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
    
    osgViewer::SimpleViewer viewer;

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = 200;
    traits->y = 200;
    traits->width = 800;
    traits->height = 600;
    traits->windowDecoration = true;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(gc.get());
    if (!gw)
    {
        osg::notify(osg::NOTICE)<<"Error: unable to create graphics window."<<std::endl;
    }

    viewer.setEventQueue(gw->getEventQueue());    
    viewer.getEventQueue()->windowResize(traits->x,traits->y,traits->width,traits->height);

    gc->realize();
    gc->makeCurrent();
    

    viewer.setSceneData(loadedModel.get());

    // add the exit handler'
    ExitHandler* exitHandler = new ExitHandler;
    viewer.addEventHandler(exitHandler);

    // initialize the view to look at the center of the scene graph
    const osg::BoundingSphere& bs = loadedModel->getBound();
    osg::Matrix viewMatrix;
    viewMatrix.makeLookAt(bs.center()-osg::Vec3(0.0,2.0f*bs.radius(),0.0),bs.center(),osg::Vec3(0.0f,0.0f,1.0f));


    // main loop (note, window toolkits which take control over the main loop will require a window redraw callback containing the code below.)
    while( gw->isRealized() && !exitHandler->done() )
    {
        // check the window evnts
        gw->checkEvents();

        // set the view
        viewer.getCamera()->setViewMatrix(viewMatrix);
        
        // advance the frame, handle events, update the scene and render it. 
        viewer.frame();

        // Swap Buffers
        gw->swapBuffers();
    }

    return 0;
}

