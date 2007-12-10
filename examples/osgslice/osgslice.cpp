/* OpenSceneGraph example, osgslice.
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

// Simple example of use of osg::GraphicContext to create an OpenGL
// graphics window, and OSG for rendering.

#include <osg/Timer>
#include <osg/GraphicsContext>
#include <osg/ApplicationUsage>
#include <osgUtil/SceneView>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <sstream>
#include <iostream>

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

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    


    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = 100;
    traits->y = 100;
    traits->width = numberSlices;
    traits->height = numberSlices;
    traits->alpha = 8;
    traits->windowDecoration = true;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
    traits->pbuffer = false;

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    if (!gc || !gc->valid())
    {
        osg::notify(osg::NOTICE)<<"Error: unable to create graphics window"<<std::endl;
        return 1;
    }
    
    gc->realize();
    gc->makeCurrent();

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
    for( unsigned int i = 0 ; i < sp->_sliceNumber && gc->isRealized() ; ++i )
    {
        // set up the frame stamp for current frame to record the current time and frame number so that animtion code can advance correctly
        osg::ref_ptr<osg::FrameStamp> frameStamp = new osg::FrameStamp;
        frameStamp->setReferenceTime(osg::Timer::instance()->delta_s(start_tick,osg::Timer::instance()->tick()));
        frameStamp->setFrameNumber(frameNum++);
        
        // pass frame stamp to the SceneView so that the update, cull and draw traversals all use the same FrameStamp
        sceneView->setFrameStamp(frameStamp.get());
        
        // update the viewport dimensions, incase the window has been resized.
        sceneView->setViewport(0,0,traits->width,traits->height);
        
        
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
        gc->swapBuffers();
        
        std::cout << "before readPixels: _r = " << sp->_image->r() << std::endl;
        
        tmpImage->readPixels(static_cast<int>(sceneView->getViewport()->x()),
                             static_cast<int>(sceneView->getViewport()->y()),
                             static_cast<int>(sceneView->getViewport()->width()),
                             static_cast<int>(sceneView->getViewport()->height()),
                             GL_RGBA,GL_UNSIGNED_BYTE);
        
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

