// C++ source file - (C) 2003 Robert Osfield, released under the OSGPL.
//
// Simple example of use of Producer::RenderSurface to create an OpenGL
// graphics window, and OSG for rendering.

#include <osg/Timer>
#include <osgUtil/SceneView>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgProducer/Viewer>

#include <sstream>

#define MIN_NEARFAROFFSET 0.1

class SliceProcessor
{
    public:

        SliceProcessor() : _sliceNumber(128), _sliceDelta(0.1), _nearPlane(0.0), _farPlane(MIN_NEARFAROFFSET)
        {
            //
        }
        SliceProcessor( const double& objectRadius, unsigned int numberSlices) : _sliceNumber(numberSlices)
        {
            _sliceDelta = (objectRadius*2) / _sliceNumber;
            _nearPlane = objectRadius;                          // note: distance from viewpoint is going to be set 2x radius
            _farPlane = _nearPlane+MIN_NEARFAROFFSET;
            _image = new osg::Image;
            if( _sliceDelta > MIN_NEARFAROFFSET )
            {
                _nearFarOffset = MIN_NEARFAROFFSET;
            }
            else
            {
                _nearFarOffset = _sliceDelta;
            }
            _image->allocateImage( _sliceNumber, _sliceNumber,_sliceNumber, GL_RGBA, GL_UNSIGNED_BYTE );
            
        }
        // needs 3D-Texture object
        osg::Image* _image;
        unsigned int _sliceNumber;
        double _sliceDelta;
        double _nearPlane;
        double _farPlane;
        double _nearFarOffset;
        
    // needs function to do rendering and slicing
};

int main( int argc, char **argv )
{
        // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use of osg::AnimationPath and UpdateCallbacks for adding animation to your scenes.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("-o <filename>","Object to be loaded");
    
    if( arguments.read( "-h" ) || arguments.read( "--help" ) )
    {
        std::cout << "Argumentlist:" << std::endl;
        std::cout << "\t-o <filename> sets object to be loaded and sliced" << std::endl;
        std::cout << "\t--slices <unsigned int> sets number of slices through the object" << std::endl;
        std::cout << "\t--near <double> sets start for near clipping plane" << std::endl;
        std::cout << "\t--far <double> sets start for far clipping plane" << std::endl;
        
        return 1;
    }
    
    std::string outputName("volume_tex.dds");
    while( arguments.read( "-o", outputName ) ) { }


    unsigned int numberSlices = 128;
    while( arguments.read( "--slices", numberSlices) ) { }

    double nearClip=0.0f;
    double farClip=0.0f;
    while( arguments.read( "--near",nearClip ) ) { }
    while( arguments.read( "--far", farClip) ) { }

    
    // load the scene.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles( arguments );
    if (!loadedModel) 
    {
        std::cout << "No data loaded." << std::endl;
        return 1;
    }

    const osg::BoundingSphere& bs = loadedModel->getBound();
    SliceProcessor* sp = new SliceProcessor( (double)bs.radius(), numberSlices );
    if (nearClip!=0.0) sp->_nearPlane = nearClip;
    if (farClip!=0.0) sp->_farPlane = farClip;

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
    // create the window to draw to.
    osg::ref_ptr<Producer::RenderSurface> renderSurface = new Producer::RenderSurface;
    renderSurface->setWindowName("osgsimple");
    renderSurface->setWindowRectangle(100,100,numberSlices,numberSlices);
    renderSurface->useBorder(true);

    // make sure we have a alpha channel in the colour buffer, to enable 
    // alpha on read back.
    Producer::VisualChooser* rs_vc = renderSurface->getVisualChooser();
    if (!rs_vc)
    {
        rs_vc = new Producer::VisualChooser;
        rs_vc->setSimpleConfiguration();
        renderSurface->setVisualChooser(rs_vc);
    }
    rs_vc->setAlphaSize(8);

    // create the graphics context.
    renderSurface->realize();
    
    // create the view of the scene.
    osg::ref_ptr<osgUtil::SceneView> sceneView = new osgUtil::SceneView;
    sceneView->setDefaults();
    sceneView->setSceneData(loadedModel.get());

    // initialize the view to look at the center of the scene graph
    osg::Matrix viewMatrix;
    // distance from viewport to object's center is set to be 2x bs.radius()
    viewMatrix.makeLookAt(bs.center()-osg::Vec3(0.0,2.0f*bs.radius(),0.0),bs.center(),osg::Vec3(0.0f,0.0f,1.0f));
    
    // turn off autocompution of near and far clipping planes
    sceneView->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR);

    // set the clear color of the background to make sure that the alpha is 0.0.
    sceneView->setClearColor(osg::Vec4(0.0f,0.0f,0.0f,0.0f));

    // record the timer tick at the start of rendering.     
    osg::Timer_t start_tick = osg::Timer::instance()->tick();
    
    std::cout << "radius: " << bs.radius() << std::endl;
    
    unsigned int frameNum = 0;
    double tmpNear, tmpFar;
    std::string baseImageName("shot_");
    std::string tmpImageName;
    
    osg::Image* tmpImage = new osg::Image;
    
    // main loop (note, window toolkits which take control over the main loop will require a window redraw callback containing the code below.)
    for( unsigned int i = 0 ; i < sp->_sliceNumber && renderSurface->isRealized() ; ++i )
    {
        // set up the frame stamp for current frame to record the current time and frame number so that animtion code can advance correctly
        osg::ref_ptr<osg::FrameStamp> frameStamp = new osg::FrameStamp;
        frameStamp->setReferenceTime(osg::Timer::instance()->delta_s(start_tick,osg::Timer::instance()->tick()));
        frameStamp->setFrameNumber(frameNum++);
        
        // pass frame stamp to the SceneView so that the update, cull and draw traversals all use the same FrameStamp
        sceneView->setFrameStamp(frameStamp.get());
        
        // update the viewport dimensions, incase the window has been resized.
        sceneView->setViewport(0,0,renderSurface->getWindowWidth(),renderSurface->getWindowHeight());
        
        
        // set the view
        sceneView->setViewMatrix(viewMatrix);
        
        // set Projection Matrix
        tmpNear = sp->_nearPlane+i*sp->_sliceDelta;
        tmpFar = sp->_farPlane+(i*sp->_sliceDelta)+sp->_nearFarOffset;
        sceneView->setProjectionMatrixAsOrtho(-(bs.radius()+bs.radius()/2), bs.radius()+bs.radius()/2,-bs.radius(), bs.radius(), tmpNear, tmpFar);

        // do the update traversal the scene graph - such as updating animations
        sceneView->update();
        
        // do the cull traversal, collect all objects in the view frustum into a sorted set of rendering bins
        sceneView->cull();
        
        // draw the rendering bins.
        sceneView->draw();
                
        // Swap Buffers
        renderSurface->swapBuffers();
        
        std::cout << "before readPixels: _r = " << sp->_image->r() << std::endl;
        
        tmpImage->readPixels(sceneView->getViewport()->x(),sceneView->getViewport()->y(),sceneView->getViewport()->width(),sceneView->getViewport()->height(),GL_RGBA,GL_UNSIGNED_BYTE);
        
//        std::cout << "vor copySubImage: _r = " << sp->_image->r() << std::endl;
        sp->_image->copySubImage( 0, 0, i, tmpImage );

        /*
        std::ostringstream o;
        o << baseImageName << i << ".rgba";
        tmpImageName = o.str();
        osgDB::writeImageFile( *(sp->_image), tmpImageName );
        std::cout << "Wrote image to file: " << tmpImageName << std::endl;
        */
    }
    osgDB::writeImageFile( *(sp->_image), outputName);

    return 0;
}

