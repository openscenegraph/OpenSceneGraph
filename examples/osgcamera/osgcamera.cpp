// C++ source file - (C) 2003 Robert Osfield, released under the OSGPL.
//
// Simple example of use of Producer::RenderSurface to create an OpenGL
// graphics window, and OSG for rendering.

#include <osg/Timer>
#include <osg/GraphicsContext>
#include <osgUtil/SceneView>
#include <osgDB/ReadFile>
#include <iostream>

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
    
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->_windowName = "osgcamera";
    traits->_x = 100;
    traits->_y = 100;
    traits->_width = 800;
    traits->_height = 800;
    traits->_windowDecoration = true;
    traits->_doubleBuffer = true;
    
    osg::ref_ptr<osg::GraphicsContext> gfxc = osg::GraphicsContext::createGraphicsContext(traits.get());
    
    if (!gfxc)
    {
        std::cout<<"Unable to create window."<<std::endl;
        return 1;
    }
    
    // realise the window
    gfxc->realize();

    // create the view of the scene.
    osg::ref_ptr<osgUtil::SceneView> sceneView = new osgUtil::SceneView;
    sceneView->setDefaults();
    sceneView->setSceneData(loadedModel.get());

    // initialize the view to look at the center of the scene graph
    const osg::BoundingSphere& bs = loadedModel->getBound();
    osg::Matrix viewMatrix;
    viewMatrix.makeLookAt(bs.center()-osg::Vec3(0.0,2.0f*bs.radius(),0.0),bs.center(),osg::Vec3(0.0f,0.0f,1.0f));

    // record the timer tick at the start of rendering.    
    osg::Timer_t start_tick = osg::Timer::instance()->tick();
    
    unsigned int frameNum = 0;
    
    // make the graphics context current
    gfxc->makeCurrent();

    // main loop (note, window toolkits which take control over the main loop will require a window redraw callback containing the code below.)
    while( gfxc->isRealized() )
    {
        // set up the frame stamp for current frame to record the current time and frame number so that animtion code can advance correctly
        osg::ref_ptr<osg::FrameStamp> frameStamp = new osg::FrameStamp;
        frameStamp->setReferenceTime(osg::Timer::instance()->delta_s(start_tick,osg::Timer::instance()->tick()));
        frameStamp->setFrameNumber(frameNum++);
        
        // pass frame stamp to the SceneView so that the update, cull and draw traversals all use the same FrameStamp
        sceneView->setFrameStamp(frameStamp.get());
        
        // update the viewport dimensions, incase the window has been resized.
        sceneView->setViewport(0,0,traits->_width,traits->_height);
        
        // set the view
        sceneView->setViewMatrix(viewMatrix);

        // do the update traversal the scene graph - such as updating animations
        sceneView->update();
        
        // do the cull traversal, collect all objects in the view frustum into a sorted set of rendering bins
        sceneView->cull();
        
        // draw the rendering bins.
        sceneView->draw();

    	// Swap Buffers
    	gfxc->swapBuffers();
    }

    return 0;
}
