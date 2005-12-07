// C++ source file - (C) 2003 Robert Osfield, released under the OSGPL.
//
// Simple example of use of Producer::RenderSurface to create an OpenGL
// graphics window, and OSG for rendering.

#include <osg/Timer>
#include <osg/GraphicsContext>
#include <osg/GraphicsThread>

#include <osgUtil/UpdateVisitor>
#include <osgUtil/CullVisitor>
#include <osgUtil/SceneView>
#include <osgUtil/GLObjectsVisitor>

#include <osgDB/ReadFile>
#include <osgDB/DynamicLibrary>
#include <osgDB/Registry>

#include <map>
#include <list>
#include <iostream>



////////////////////////////////////////////////////////////////////////////////
//
//
//  **************** THIS IS AN EXPERIMENTAL IMPLEMENTATION ***************
//  ************************** PLEASE DO NOT COPY  ************************
//
//
///////////////////////////////////////////////////////////////////////////////


// Compile operation, that compile OpenGL objects.
struct CompileOperation : public osg::GraphicsThread::Operation
{
    CompileOperation(osg::Node* scene):
        osg::GraphicsThread::Operation("Compile",false),
        _scene(scene)
    {
    }
    
    virtual void operator () (osg::GraphicsContext* context)
    {
        std::cout<<"Compile"<<std::endl;
    
        osgUtil::GLObjectsVisitor compileVisitor;
        compileVisitor.setState(context->getState());

        // do the compile traversal
        _scene->accept(compileVisitor);
    }
    
    osg::ref_ptr<osg::Node> _scene;
};

// Cull operation, that does a cull on the scene graph.
struct CullOperation : public osg::GraphicsThread::Operation
{
    CullOperation(osgUtil::SceneView* sceneView):
        osg::GraphicsThread::Operation("Cull",true),
        _sceneView(sceneView)
    {
    }
    
    virtual void operator () (osg::GraphicsContext* context)
    {
        _sceneView->setState(context->getState());
        _sceneView->cull();
    }
    
    osg::ref_ptr<osgUtil::SceneView> _sceneView;
};

// Draw operation, that does a draw on the scene graph.
struct DrawOperation : public osg::GraphicsThread::Operation
{
    DrawOperation(osgUtil::SceneView* sceneView):
        osg::GraphicsThread::Operation("Draw",true),
        _sceneView(sceneView)
    {
    }
    
    virtual void operator () (osg::GraphicsContext*)
    {
        _sceneView->draw();
    }
    
    osg::ref_ptr<osgUtil::SceneView> _sceneView;
};

struct CleanUpOperation : public osg::GraphicsThread::Operation
{
    CleanUpOperation(osgUtil::SceneView* sceneView):
        osg::GraphicsThread::Operation("CleanUp",false),
        _sceneView(sceneView)
    {
    }
    
    virtual void operator () (osg::GraphicsContext*)
    {
        _sceneView->releaseAllGLObjects();
        _sceneView->flushAllDeletedGLObjects();
    }
    
    osg::ref_ptr<osgUtil::SceneView> _sceneView;
};

// main does the following steps to create a multi-thread, multiple camera/graphics context view of a scene graph.
//
// 1) load the scene graph
//
// 2) create a list of camera, each with their own graphis context, with a graphics thread for each context.
//
// 3) set up the graphic threads so that the do an initial compile OpenGL objects operation, this is done once, and then this compile op is disgarded
//
// 4) set up the graphics thread so that it has all the graphics ops required for the main loop, these ops are:
// 4.a) frame begin barrair, syncronizes all the waiting graphic threads so they don't run while update is occuring
// 4.b) frame operation - the cull and draw for each camera
// 4.c) frame end barrier, releases the update thread once all graphic threads have dispatched all their OpenGL commands
// 4.d) pre swap barrier, barrier which ensures that all graphics threads have sent their data down to the gfx card.
// 4.e) swap buffers, do the swap buffers on all the graphics contexts.
//
// 5. The main loop:
// 5.a) update
// 5.b) join the frame begin barrrier, releasing all the graphics threads to do their stuff
// 5.c) block on the frame end barrier, waiting till all the graphics threads have done their cull/draws.
// 5.d) check to see if any of the windows has been closed. 
//
int main( int argc, char **argv )
{
    osg::Referenced::setThreadSafeReferenceCounting(true);

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    std::string windowingLibrary("osgProducer");
    while (arguments.read("--windowing",windowingLibrary)) {}

    // load the osgProducer library manually.
    osg::ref_ptr<osgDB::DynamicLibrary> windowingLib =
        osgDB::DynamicLibrary::loadLibrary(osgDB::Registry::instance()->createLibraryNameForNodeKit(windowingLibrary));


    if (!windowingLib)
    {
        std::cout<<"Error: failed to loading windowing library: "<<windowingLibrary<<std::endl;
    }

    unsigned int numberCameras = 3;
    while (arguments.read("--cameras",numberCameras)) {}

    unsigned int xpos = 0;
    unsigned int ypos = 400;
    unsigned int width = 400;
    unsigned int height = 400;

    while (arguments.read("--xpos",xpos)) {}
    while (arguments.read("--ypos",ypos)) {}
    while (arguments.read("--height",height)) {}
    while (arguments.read("--width",width)) {}

    unsigned int maxNumFrames = 1000;
    while (arguments.read("--max-num-frames",maxNumFrames)) {}

    // load the scene.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    if (!loadedModel) 
    {
        std::cout << argv[0] <<": No data loaded." << std::endl;
        return 1;
    }

    
    // set up the frame stamp for current frame to record the current time and frame number so that animtion code can advance correctly
    osg::ref_ptr<osg::FrameStamp> frameStamp = new osg::FrameStamp;

    unsigned int frameNum = 0;

    osgUtil::UpdateVisitor updateVisitor;
    updateVisitor.setFrameStamp(frameStamp.get());
    
    typedef std::list< osg::ref_ptr<osg::CameraNode> > CameraList;
    typedef std::set< osg::GraphicsContext* > GraphicsContextSet;

    CameraList cameraList;
    GraphicsContextSet graphicsContextSet;

    // create the cameras, graphic contexts and graphic threads.
    bool shareContexts = false;
    osg::GraphicsContext* previousContext = 0;
    for(unsigned int i=0; i< numberCameras; ++i)
    {
        osg::ref_ptr<osg::CameraNode> camera = new osg::CameraNode;
        camera->addChild(loadedModel.get());

        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
        traits->_windowName = "osgcamera";
        traits->_x = xpos;
        traits->_y = ypos;
        traits->_width = width;
        traits->_height = height;
        traits->_windowDecoration = true;
        traits->_doubleBuffer = true;
        traits->_sharedContext = shareContexts ? previousContext : 0;
        
        xpos += width;

        osg::ref_ptr<osg::GraphicsContext> gfxc = osg::GraphicsContext::createGraphicsContext(traits.get());

        if (!gfxc)
        {
            std::cout<<"Unable to create window."<<std::endl;
            return 1;
        }

        camera->setGraphicsContext(gfxc.get());

        // initialize the view to look at the center of the scene graph
        const osg::BoundingSphere& bs = loadedModel->getBound();
        osg::Matrix viewMatrix;
        viewMatrix.makeLookAt(bs.center()-osg::Vec3(0.0,2.0f*bs.radius(),0.0),bs.center(),osg::Vec3(0.0f,0.0f,1.0f));

        camera->setViewport(0,0,traits->_width,traits->_height);
        camera->setProjectionMatrixAsPerspective(50.0f,1.4f,1.0f,10000.0f);
        camera->setViewMatrix(viewMatrix);

        // graphics thread will realize the window.
        gfxc->createGraphicsThread();

        cameraList.push_back(camera);

        previousContext = gfxc.get();
    }


    // build the list of unique graphics contexts.
    CameraList::iterator citr;
    for(citr = cameraList.begin();
        citr != cameraList.end();
        ++citr)
    {
        graphicsContextSet.insert(const_cast<osg::GraphicsContext*>((*citr)->getGraphicsContext()));
    }


    std::cout<<"Number of cameras = "<<cameraList.size()<<std::endl;
    std::cout<<"Number of graphics contexts = "<<graphicsContextSet.size()<<std::endl;


    // first the compile of the GL Objects, do it syncronously.
    GraphicsContextSet::iterator gitr;
    osg::ref_ptr<CompileOperation> compileOp = new CompileOperation(loadedModel.get());
    for(gitr = graphicsContextSet.begin();
        gitr != graphicsContextSet.end();
        ++gitr)
    {
        osg::GraphicsContext* context = *gitr;
        context->getGraphicsThread()->add(compileOp.get(), false);
    }


    // second the begin frame barrier to all graphics threads
    osg::ref_ptr<osg::BarrierOperation> frameBeginBarrierOp = new osg::BarrierOperation(graphicsContextSet.size()+1, osg::BarrierOperation::NO_OPERATION);
    for(gitr = graphicsContextSet.begin();
        gitr != graphicsContextSet.end();
        ++gitr)
    {
        osg::GraphicsContext* context = *gitr;
        context->getGraphicsThread()->add(frameBeginBarrierOp.get(), false);
    }

    osg::ref_ptr<osg::BarrierOperation> glFinishBarrierOp = new osg::BarrierOperation(graphicsContextSet.size(), osg::BarrierOperation::GL_FINISH);

    // we can put a finish in to gate rendering throughput, so that each new frame starts with a clean sheet.
    // you should only enable one of these, doFinishBeforeNewDraw will allow for the better parallism of the two finish approaches
    // note, both are disabled right now, as glFinish is spin locking the CPU, not something that we want...
    bool doFinishBeforeNewDraw = false;
    bool doFinishAfterSwap = false;

    // third add the frame for each camera.
    for(citr = cameraList.begin();
        citr != cameraList.end();
        ++citr)
    {
        osg::CameraNode* camera = citr->get();
        osg::GraphicsThread* graphicsThread = camera->getGraphicsContext()->getGraphicsThread();
        
        // create a scene view to do the cull and draw
        osgUtil::SceneView* sceneView = new osgUtil::SceneView;
        sceneView->setDefaults();
        sceneView->setFrameStamp(frameStamp.get());
        sceneView->setCamera(camera);    

        // cull traversal operation
        graphicsThread->add( new CullOperation(sceneView), false); 

        // optionally add glFinish barrier to ensure that all OpenGL commands are completed before we start dispatching a new frame
        if (doFinishBeforeNewDraw) graphicsThread->add( glFinishBarrierOp.get(), false); 

        // draw traversal operation.
        graphicsThread->add( new DrawOperation(sceneView), false); 
    }

    // fourth add the frame end barrier, the pre swap barrier and finally the swap buffers to each graphics thread.
    // The frame end barrier tells the main thead that the draw dispatch/read phase of the scene graph is complete.
    // The pre swap barrier is an optional extra, which does a flush before joining the barrier, using this all graphics threads
    // are held back until they have all dispatched their fifo to the graphics hardware.  
    // The swapOp just issues a swap buffers for each of the graphics contexts.
    osg::ref_ptr<osg::BarrierOperation> frameEndBarrierOp = new osg::BarrierOperation(graphicsContextSet.size()+1, osg::BarrierOperation::NO_OPERATION);
    osg::ref_ptr<osg::BarrierOperation> preSwapBarrierOp = new osg::BarrierOperation(graphicsContextSet.size(), osg::BarrierOperation::GL_FLUSH);
    osg::ref_ptr<osg::SwapBuffersOperation> swapOp = new osg::SwapBuffersOperation();
    for(gitr = graphicsContextSet.begin();
        gitr != graphicsContextSet.end();
        ++gitr)
    {
        osg::GraphicsContext* context = *gitr;

        context->getGraphicsThread()->add(frameEndBarrierOp.get(), false);
        // context->getGraphicsThread()->add(preSwapBarrierOp.get(), false);
        context->getGraphicsThread()->add(swapOp.get(), false);
        
        // optionally add finish barrier to ensure that we don't do any other graphics work till the current OpenGL commands are complete.
        if (doFinishAfterSwap) context->getGraphicsThread()->add(glFinishBarrierOp.get(), false);
    }


    // record the timer tick at the start of rendering.    
    osg::Timer_t start_tick = osg::Timer::instance()->tick();
    osg::Timer_t previous_tick = start_tick;
    
    bool done = false;

    // main loop -  update scene graph, dispatch frame, wait for frame done.
    while( !done && frameNum<maxNumFrames)
    {

        osg::Timer_t current_tick = osg::Timer::instance()->tick();

        frameStamp->setReferenceTime(osg::Timer::instance()->delta_s(start_tick,current_tick));
        frameStamp->setFrameNumber(frameNum++);
        
        //std::cout<<"Frame rate "<<1.0/osg::Timer::instance()->delta_s(previous_tick,current_tick)<<std::endl;
        previous_tick = current_tick;


        // do the update traversal.
        loadedModel->accept(updateVisitor);

        // dispatch the frame.
        frameBeginBarrierOp->block();
        
        // wait till the frame is done.
        frameEndBarrierOp->block();

        // check if any of the windows are closed
        for(gitr = graphicsContextSet.begin();
            gitr != graphicsContextSet.end();
            ++gitr)
        {
            osg::GraphicsContext* context = *gitr;
            if (!context->isRealized()) done = true;
        }

    }

    
    std::cout<<"Exiting application"<<std::endl;

    // start clean up.
    for(gitr = graphicsContextSet.begin();
        gitr != graphicsContextSet.end();
        ++gitr)
    {
        osg::GraphicsContext* context = *gitr;
        osg::GraphicsThread* thread = context->getGraphicsThread();
        if (thread)
        {
            thread->removeAllOperations();
            thread->setDone(true);
        }
    }

    std::cout<<"Removed all operations"<<std::endl;

    return 0;
}
