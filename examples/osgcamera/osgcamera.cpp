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

#include <osgDB/ReadFile>

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


struct FrameOperation : public osg::GraphicsThread::Operation
{
    FrameOperation(osg::CameraNode* camera, osg::FrameStamp* frameStamp):
        _camera(camera),
        _frameStamp(frameStamp)
    {
        _sceneView = new osgUtil::SceneView;
        _sceneView->setDefaults();
        _sceneView->setFrameStamp(_frameStamp.get());
            
        if (camera->getNumChildren()>=1)
        {
            _sceneView->setSceneData(camera->getChild(0));
        }
    }
    
    virtual void operator () (osg::GraphicsContext* context)
    {
        std::cout<<"FrameOperation draw begin"<<context<<std::endl;

        _sceneView->setState(context->getState());
        _sceneView->setProjectionMatrix(_camera->getProjectionMatrix());
        _sceneView->setViewMatrix(_camera->getViewMatrix());
        _sceneView->setViewport(_camera->getViewport());
        
        _sceneView->cull();
        _sceneView->draw();

        std::cout<<"FrameOperation draw end"<<context<<std::endl;
    }
    
    osg::ref_ptr<osg::CameraNode>    _camera;
    osg::ref_ptr<osg::FrameStamp>    _frameStamp;
    osg::ref_ptr<osgUtil::SceneView> _sceneView;
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

    
    // set up the frame stamp for current frame to record the current time and frame number so that animtion code can advance correctly
    osg::ref_ptr<osg::FrameStamp> frameStamp = new osg::FrameStamp;

    unsigned int frameNum = 0;

    osgUtil::UpdateVisitor updateVisitor;
    updateVisitor.setFrameStamp(frameStamp.get());


    unsigned int numberCameras = 3;
    unsigned int xpos = 0;
    unsigned int ypos = 400;
    unsigned int width = 400;
    unsigned int height = 400;
    
    typedef std::map< osg::ref_ptr<osg::CameraNode>, osg::ref_ptr<FrameOperation> > CameraMap;
    typedef std::set< osg::GraphicsContext* > GraphicsContextSet;

    CameraMap cameraMap;
    GraphicsContextSet graphicsContextSet;

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

        xpos += width;

        osg::ref_ptr<osg::GraphicsContext> gfxc = osg::GraphicsContext::createGraphicsContext(traits.get());

        if (!gfxc)
        {
            std::cout<<"Unable to create window."<<std::endl;
            return 1;
        }

        // realise the window
        gfxc->realize();

        camera->setGraphicsContext(gfxc.get());

        // initialize the view to look at the center of the scene graph
        const osg::BoundingSphere& bs = loadedModel->getBound();
        osg::Matrix viewMatrix;
        viewMatrix.makeLookAt(bs.center()-osg::Vec3(0.0,2.0f*bs.radius(),0.0),bs.center(),osg::Vec3(0.0f,0.0f,1.0f));

        camera->setViewport(0,0,traits->_width,traits->_height);
        camera->setProjectionMatrixAsPerspective(50.0f,1.4f,1.0f,10000.0f);
        camera->setViewMatrix(viewMatrix);


        gfxc->createGraphicsThread();

        cameraMap[camera] = new FrameOperation(camera.get(), frameStamp.get());
    }


    CameraMap::iterator citr;
    for(citr = cameraMap.begin();
        citr != cameraMap.end();
        ++citr)
    {
        graphicsContextSet.insert(const_cast<osg::GraphicsContext*>(citr->first->getGraphicsContext()));
    }

    osg::ref_ptr<osg::SwapBuffersOperation> swapOp = new osg::SwapBuffersOperation();
    osg::ref_ptr<osg::BarrierOperation> frameEndBarrierOp = new osg::BarrierOperation(graphicsContextSet.size()+1, osg::BarrierOperation::NO_OPERATION);
    osg::ref_ptr<osg::BarrierOperation> preSwapBarrierOp = new osg::BarrierOperation(graphicsContextSet.size(), osg::BarrierOperation::GL_FLUSH);

    std::cout<<"nubmer of gfx."<<graphicsContextSet.size()<<std::endl;


    GraphicsContextSet::iterator gitr;
    for(gitr = graphicsContextSet.begin();
        gitr != graphicsContextSet.end();
        ++gitr)
    {
        std::cout<<"Issue swap."<<std::endl;
        osg::GraphicsContext* context = *gitr;
        context->getGraphicsThread()->add(swapOp.get(), true);
    }
    
    // record the timer tick at the start of rendering.    
    osg::Timer_t start_tick = osg::Timer::instance()->tick();
    
    bool done = false;    

    // main loop (note, window toolkits which take control over the main loop will require a window redraw callback containing the code below.)
    while( !done )
    {
        std::cout<<"Frame "<<frameNum<<std::endl;

        frameStamp->setReferenceTime(osg::Timer::instance()->delta_s(start_tick,osg::Timer::instance()->tick()));
        frameStamp->setFrameNumber(frameNum++);
        
        std::cout<<"Frame rate "<<(double)frameNum / frameStamp->getReferenceTime()<<std::endl;


        loadedModel->accept(updateVisitor);

        // issue the frame for each camera.
        for(citr = cameraMap.begin();
            citr != cameraMap.end();
            ++citr)
        {
            osg::CameraNode* camera = const_cast<osg::CameraNode*>(citr->first.get());
            camera->getGraphicsContext()->getGraphicsThread()->add( citr->second.get(), false); 
        }

        for(gitr = graphicsContextSet.begin();
            gitr != graphicsContextSet.end();
            ++gitr)
        {
            osg::GraphicsContext* context = *gitr;
            context->getGraphicsThread()->add(frameEndBarrierOp.get(), false);
            context->getGraphicsThread()->add(preSwapBarrierOp.get(), false);
        }

        std::cout<<"Join frameEndBarrierOp block "<<std::endl;
        osg::Timer_t before_tick = osg::Timer::instance()->tick();
        frameEndBarrierOp->block();
        osg::Timer_t after_tick = osg::Timer::instance()->tick();
        std::cout<<"Leave frameEndBarrierOp block "<<osg::Timer::instance()->delta_s(before_tick,after_tick)<<std::endl;

        std::cout<<"Join preSwapBarrierOp block "<<std::endl;
        before_tick = osg::Timer::instance()->tick();
//        preSwapBarrierOp->block();
        after_tick = osg::Timer::instance()->tick();
        std::cout<<"Leave preSwapBarrierOp block "<<osg::Timer::instance()->delta_s(before_tick,after_tick)<<std::endl;

        for(gitr = graphicsContextSet.begin();
            gitr != graphicsContextSet.end();
            ++gitr)
        {
            osg::GraphicsContext* context = *gitr;
            context->getGraphicsThread()->add(swapOp.get(), false);
        }

        // check if any of the windows are closed
        for(gitr = graphicsContextSet.begin();
            gitr != graphicsContextSet.end();
            ++gitr)
        {
            osg::GraphicsContext* context = *gitr;
            if (!context->isRealized()) done = true;
        }

    }

    return 0;
}
