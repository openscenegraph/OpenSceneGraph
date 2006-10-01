// C++ source file - (C) 2003 Robert Osfield, released under the OSGPL.
//
// Simple example of use of Producer::RenderSurface + KeyboardMouseCallback + SimpleViewer

#include <Producer/RenderSurface>
#include <Producer/KeyboardMouse>
#include <Producer/Trackball>

#include <osgDB/ReadFile>

#include <osgGA/SimpleViewer>
#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>

// ----------- Begining of glue classes to adapter Producer's keyboard mouse events to osgGA's abstraction events.
class MyKeyboardMouseCallback : public Producer::KeyboardMouseCallback
{
public:

    MyKeyboardMouseCallback(osgGA::EventQueue* eventQueue) :
        _done(false),
        _eventQueue(eventQueue)
    {
    }

    virtual void shutdown()
    {
        _done = true; 
    }

    virtual void specialKeyPress( Producer::KeyCharacter key )
    {
        if (key==Producer::KeyChar_Escape)
                    shutdown();

        _eventQueue->keyPress( (osgGA::GUIEventAdapter::KeySymbol) key );
    }

    virtual void specialKeyRelease( Producer::KeyCharacter key )
    {
        _eventQueue->keyRelease( (osgGA::GUIEventAdapter::KeySymbol) key );
    }

    virtual void keyPress( Producer::KeyCharacter key)
    {
        _eventQueue->keyPress( (osgGA::GUIEventAdapter::KeySymbol) key );
    }

    virtual void keyRelease( Producer::KeyCharacter key)
    {
        _eventQueue->keyRelease( (osgGA::GUIEventAdapter::KeySymbol) key );
    }

    virtual void mouseMotion( float mx, float my ) 
    {
        _eventQueue->mouseMotion( mx, my );
    }

    virtual void buttonPress( float mx, float my, unsigned int mbutton ) 
    {
        _eventQueue->mouseButtonPress(mx, my, mbutton);
    }
    
    virtual void buttonRelease( float mx, float my, unsigned int mbutton ) 
    {
        _eventQueue->mouseButtonRelease(mx, my, mbutton);
    }

    bool done() { return _done; }

private:

    bool                                _done;
    osg::ref_ptr<osgGA::EventQueue>     _eventQueue;
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
    
    // create the window to draw to.
    osg::ref_ptr<Producer::RenderSurface> renderSurface = new Producer::RenderSurface;
    renderSurface->setWindowName("osgkeyboardmouse");
    renderSurface->setWindowRectangle(100,100,800,600);
    renderSurface->useBorder(true);
    renderSurface->realize();
    

    // create the view of the scene.
    osgGA::SimpleViewer viewer;
    viewer.setSceneData(loadedModel.get());
    
    // set up a KeyboardMouse to manage the events comming in from the RenderSurface
    osg::ref_ptr<Producer::KeyboardMouse>  kbm = new Producer::KeyboardMouse(renderSurface.get());

    // create a KeyboardMouseCallback to handle the mouse events within this applications
    osg::ref_ptr<MyKeyboardMouseCallback> kbmcb = new MyKeyboardMouseCallback(viewer.getEventQueue());

    // create a tracball manipulator to move the camera around in response to keyboard/mouse events
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );
    
    // set the window dimensions 
    viewer.getEventQueue()->getCurrentEventState()->setWindowRectangle(100,100,800,600);

    // set the mouse input range.
    // Producer defaults to using non-dimensional units, so we pass this onto osgGA, most windowing toolkits use pixel coords so use the window size instead.
    viewer.getEventQueue()->getCurrentEventState()->setInputRange(-1.0, -1.0, 1.0, 1.0);

    // Producer has the y axis increase upwards, like OpenGL, and contary to most Windowing toolkits.
    // we need to construct the event queue so that it knows about this convention.
    viewer.getEventQueue()->getCurrentEventState()->setMouseYOrientation(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);

    // main loop (note, window toolkits which take control over the main loop will require a window redraw callback containing the code below.)
    while( renderSurface->isRealized() && !kbmcb->done())
    {
        // update the window dimensions, in case the window has been resized.
         viewer.getEventQueue()->windowResize(0,0,renderSurface->getWindowWidth(),renderSurface->getWindowHeight(), false);

        // pass any keyboard mouse events onto the local keyboard mouse callback.
        kbm->update( *kbmcb );
        
        viewer.frame();

        // Swap Buffers
        renderSurface->swapBuffers();
    }

    return 0;
}

