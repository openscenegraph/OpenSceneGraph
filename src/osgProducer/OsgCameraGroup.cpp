/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#include <osg/ApplicationUsage>
#include <osg/Timer>
#include <osg/Notify>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/ImageStream>

#include <osgUtil/DisplayRequirementsVisitor>
#include <osgDB/FileUtils>

#include <osgProducer/OsgCameraGroup>
#include <Producer/CameraConfig>

#ifdef WIN32
#define        WGL_SAMPLE_BUFFERS_ARB        0x2041
#define        WGL_SAMPLES_ARB                0x2042
#endif

using namespace Producer;
using namespace osgProducer;

static osg::ApplicationUsageProxy OsgCameraGroup_e0(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE, "OSG_PROCESSOR_AFFINITY <mode>", "ON | OFF - Where supported, switch on or off the processor affinity." );

OsgCameraGroup::RealizeCallback::~RealizeCallback()
{
}

class RenderSurfaceRealizeCallback : public Producer::RenderSurface::Callback
{
public:

    RenderSurfaceRealizeCallback(OsgCameraGroup* cameraGroup,OsgSceneHandler* sceneHandler):
        _cameraGroup(cameraGroup),
        _sceneHandler(sceneHandler)
    {
        const OsgCameraGroup::SceneHandlerList& shl = _cameraGroup->getSceneHandlerList();
        for(unsigned int i=0; i<shl.size(); ++i)
        {
            if (shl[i]==sceneHandler) _sceneHandlerNumber = i;
        }
    }
    
    virtual void operator()( const Producer::RenderSurface & rs)
    {
        osg::Timer timer;
        osg::Timer_t start_t = timer.tick();
    
        if (_cameraGroup->getRealizeCallback())
        {
            (*(_cameraGroup->getRealizeCallback()))(*_cameraGroup,*_sceneHandler,rs);
        }
        else if (_sceneHandler) _sceneHandler->init();

        osg::Timer_t end_t = timer.tick();
        double time = timer.delta_m(start_t,end_t);
        osg::notify(osg::INFO) << "Time to init = "<<time<<std::endl;

    }

    OsgCameraGroup*     _cameraGroup;
    OsgSceneHandler*    _sceneHandler;

    int                 _numberOfProcessors;
    int                 _sceneHandlerNumber;
    bool                _enableProccessAffinityHint;

};



std::string findCameraConfigFile(const std::string& configFile)
{
    std::string foundFile = osgDB::findDataFile(configFile);
    if (foundFile.empty()) return "";
    else return foundFile;
}

std::string extractCameraConfigFile(osg::ArgumentParser& arguments)
{
    // report the usage options.
    if (arguments.getApplicationUsage())
    {
        arguments.getApplicationUsage()->addCommandLineOption("-c <filename>","Specify camera config file");
    }

    std::string filename;
    if (arguments.read("-c",filename)) return findCameraConfigFile(filename);

    char *ptr;
    if( (ptr = getenv( "PRODUCER_CAMERA_CONFIG_FILE" )) )
    {
        osg::notify(osg::DEBUG_INFO) << "PRODUCER_CAMERA_CONFIG_FILE("<<ptr<<")"<<std::endl;
        return findCameraConfigFile(ptr);
    }

    return "";
}

static osg::ApplicationUsageProxy OsgCameraGroup_e1(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"PRODUCER_CAMERA_CONFIG_FILE <filename>","specify the default producer camera config to use when opening osgProducer based applications.");

static osg::ApplicationUsageProxy OsgCameraGroup_e4(
    osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,
    "PRODUCER_CAMERA_BLOCK_ON_VSYNC", 
    "After the CPU finishes generating the scene for one frame,"
    " block the CPU until the start of the next frame. Same as pressing 'v'"
    " within an osgProducer-based application.");


OsgCameraGroup::OsgCameraGroup() : Producer::CameraGroup() 
{
    _init();
}

OsgCameraGroup::OsgCameraGroup(Producer::CameraConfig *cfg):
    Producer::CameraGroup(cfg) 
{
    _init();
}

OsgCameraGroup::OsgCameraGroup(const std::string& configFile):
    Producer::CameraGroup(findCameraConfigFile(configFile)) 
{
    _init();
}

OsgCameraGroup::OsgCameraGroup(osg::ArgumentParser& arguments):
    Producer::CameraGroup(extractCameraConfigFile(arguments))
{
    _init();
    _applicationUsage = arguments.getApplicationUsage();
    
    while (arguments.read("--affinity"))
    {
        _enableProccessAffinityHint = true;
    }

    // report the usage options.
    if (arguments.getApplicationUsage())
    {
        arguments.getApplicationUsage()->addCommandLineOption("--affinity","Enable processor affinity where supported.");
    }


    for( unsigned int i = 0; i < _cfg->getNumberOfCameras(); i++ )
    {
        Producer::Camera *cam = _cfg->getCamera(i);
        Producer::RenderSurface *rs = cam->getRenderSurface();
        if (rs->getWindowName()== Producer::RenderSurface::defaultWindowName )
        {
            rs->setWindowName(arguments.getApplicationName());
        }
    }    



}


class QuitImageStreamVisitor : public osg::NodeVisitor
{
    public:

        QuitImageStreamVisitor():
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
       

        /** Simply traverse using standard NodeVisitor traverse method.*/
        virtual void apply(osg::Node& node)
        {
            if (node.getStateSet())
                apply(*(node.getStateSet()));

            traverse(node);
        }

        virtual void apply(osg::Geode& node)
        {
            if (node.getStateSet())
                apply(*(node.getStateSet()));

            for(unsigned int i=0;i<node.getNumDrawables();++i)
            {
                osg::Drawable* drawable = node.getDrawable(i);
                if (drawable && drawable->getStateSet())
                    apply(*(drawable->getStateSet()));
            }
        }

        void apply(osg::StateSet& stateset)
        {
            for(unsigned int i=0;i<stateset.getTextureAttributeList().size();++i)
            {
                osg::StateAttribute* texture = stateset.getTextureAttribute(i,osg::StateAttribute::TEXTURE);
                if (texture)
                {
                    osg::TextureRectangle* textureRect = dynamic_cast<osg::TextureRectangle*>(texture);
                    if (textureRect)
                    {
                        osg::ImageStream* imageStream = dynamic_cast<osg::ImageStream*>(textureRect->getImage());
                        if (imageStream)
                        {
                            imageStream->quit();
                        }
                    }

                    osg::Texture2D* texture2D = dynamic_cast<osg::Texture2D*>(texture);
                    if (texture2D)
                    {
                        osg::ImageStream* imageStream = dynamic_cast<osg::ImageStream*>(texture2D->getImage());
                        if (imageStream)
                        {
                            imageStream->quit();
                        }
                    }
                }
            }
        }

};

OsgCameraGroup::~OsgCameraGroup()
{
    // kill the DatabasePager and associated thread if one exists.
    osgDB::Registry::instance()->setDatabasePager(0);

    osg::Node* node = getTopMostSceneData();
    if (node)
    {
        // kill any ImageStream threads
        QuitImageStreamVisitor qisv;
        node->accept(qisv);
    }
}


static osg::ApplicationUsageProxy OsgCameraGroup_e2(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_CAMERA_THREADING <value>","Set the threading model using by osgProducer::Viewer/OsgCameraGroup based apps. <value> can be SingleThreaded or ThreadPerCamera");
static osg::ApplicationUsageProxy OsgCameraGroup_e3(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_SHARE_GRAPHICS_CONTEXTS <value>","Set whether to share graphics contexts. <value> can be ON or OFF");

void OsgCameraGroup::_init()
{
    if (_cfg.valid())
    {
        // By default select ThreadPerCamera when we have multiple cameras.
        _threadModel = (_cfg->getNumberOfCameras()>1) ? ThreadPerCamera : SingleThreaded;
    }

    const char* str = getenv("OSG_CAMERA_THREADING");
    if (str)
    {
        if (strcmp(str,"SingleThreaded")==0) _threadModel = SingleThreaded;
        else if (strcmp(str,"ThreadPerCamera")==0) _threadModel = ThreadPerCamera;
    }
    

    // work out how many graphics context we have    
    std::set<Producer::RenderSurface*> renderSurfaces;
    for(unsigned int i=0; i<getNumberOfCameras(); ++i)
    {
        if (getCamera(i) && getCamera(i)->getRenderSurface())
        {
            renderSurfaces.insert(getCamera(i)->getRenderSurface());
        }
    }
    unsigned int numContexts = renderSurfaces.size();
    
    str = getenv("OSG_SHARE_GRAPHICS_CONTEXTS");
    if (str)
    {
        if (strcmp(str,"ON")==0) 
        {
            Producer::RenderSurface::shareAllGLContexts(true);

            // if we are sharing graphics contexts then we just treat them all as one.
            numContexts = 1;
        }
        else if (strcmp(str,"OFF")==0) 
        {
            Producer::RenderSurface::shareAllGLContexts(false);
        }
        
    }

    osg::DisplaySettings::instance()->setMaxNumberOfGraphicsContexts(numContexts);

    if ((_threadModel==ThreadPerCamera || _threadModel==ThreadPerRenderSurface) && _cfg->getNumberOfCameras()>1)
    {
        // switch on thread safe reference counting by default when running multi-threaded.
        osg::Referenced::setThreadSafeReferenceCounting(true);
    }


    _scene_data = NULL;
    _global_stateset = NULL;
    _clear_color.set( 0.2f, 0.2f, 0.4f, 1.0f );

    _fusionDistanceMode = osgUtil::SceneView::PROPORTIONAL_TO_SCREEN_DISTANCE;
    _fusionDistanceValue = 1.0f;
    
    _realizeSceneViewOptions = osgUtil::SceneView::STANDARD_SETTINGS;

    _initialized = false;

    // set up the time and frame counter.
    _frameNumber = 0;
    _start_tick = _timer.tick();

    if (!_frameStamp) _frameStamp = new osg::FrameStamp;
    
    _applicationUsage = osg::ApplicationUsage::instance();
    
    _enableProccessAffinityHint = false;

    str = getenv("OSG_PROCESSOR_AFFINITY");
    if (str && (strcmp(str,"ON")==0 || strcmp(str,"On")==0 || strcmp(str,"on")==0))
    {
        _enableProccessAffinityHint = true;
    }

}

void OsgCameraGroup::setSceneData( osg::Node *scene ) 
{ 
    if (_scene_data==scene) return;

    if (_scene_decorator.valid() && _scene_data.valid())
    {
        _scene_decorator->removeChild(_scene_data.get());
    }
    
    _scene_data = scene; 
    
    if (_scene_decorator.valid() && _scene_data.valid())
    {
        _scene_decorator->addChild(scene);
    }
        
    updatedSceneData();
}
        
void OsgCameraGroup::setSceneDecorator( osg::Group* decorator)
{
    if (_scene_decorator==decorator) return;

    _scene_decorator = decorator;

    if (_scene_data.valid() && decorator) 
    {
        decorator->addChild(_scene_data.get());
    }
    updatedSceneData();
}

        
void OsgCameraGroup::updatedSceneData()
{
    setUpSceneViewsWithData();
}

void OsgCameraGroup::setUpSceneViewsWithData()
{
    for(SceneHandlerList::iterator  p = _shvec.begin(); p != _shvec.end(); p++ )
    {
        osgUtil::SceneView* sv = (*p)->getSceneView();
    
        if (_scene_decorator.valid())
        {
            sv->setSceneData( _scene_decorator.get() );
        }
        else if (_scene_data.valid())
        {
            sv->setSceneData( _scene_data.get() );
        }
        else
        {
            sv->setSceneData( 0 );
        }
        
        sv->setFrameStamp( _frameStamp.get() );
        sv->setGlobalStateSet( _global_stateset.get() );
        sv->setFusionDistance( _fusionDistanceMode, _fusionDistanceValue );
    }
}


void OsgCameraGroup::setFrameStamp( osg::FrameStamp* fs )
{
    _frameStamp = fs;
    setUpSceneViewsWithData();
}


void OsgCameraGroup::setGlobalStateSet( osg::StateSet *sset ) 
{ 
    _global_stateset = sset; 
    setUpSceneViewsWithData();
}

void OsgCameraGroup::setClearColor( const osg::Vec4& clearColor ) 
{
    _clear_color = clearColor;
    for(unsigned int i=0;i<getNumberOfCameras();++i)
    {
        Producer::Camera *cam = _cfg->getCamera(i);
        cam->setClearColor(_clear_color[0],_clear_color[1],_clear_color[2],_clear_color[3]);
    }
}

const osg::Vec4& OsgCameraGroup::getClearColor() const
{
    return _clear_color;
}

void OsgCameraGroup::setLODScale( float scale )
{
    getCullSettings().setLODScale(scale);
    setUpSceneViewsWithData();
}

float OsgCameraGroup::getLODScale() const
{
    return getCullSettings().getLODScale();
}

void OsgCameraGroup::setFusionDistance( osgUtil::SceneView::FusionDistanceMode mode,float value)
{
    // need to set a local variable?
    _fusionDistanceMode = mode;
    _fusionDistanceValue = value;
    setUpSceneViewsWithData();
}

void OsgCameraGroup::advance()
{
    if( !_initialized ) return;
    CameraGroup::advance();        
}

bool OsgCameraGroup::realize( ThreadingModel thread_model )
{
    if( _realized ) return _realized;
    
    if (_cfg.valid()) _cfg->setThreadModelDirective(thread_model);
    _threadModel = thread_model;
    
    return realize();
}


// small visitor to check for the existance of particle systems,
// which currently arn't thread safe, so we would need to disable
// multithreading of cull and draw.
class SearchForSpecialNodes : public osg::NodeVisitor
{
public:
    SearchForSpecialNodes():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _foundParticles(false),        
        _foundPagedLOD(false)
    {
    }
    
    virtual void apply(osg::Node& node)
    {
        if (strcmp(node.libraryName(),"osgParticle")==0 && strcmp(node.className(),"PrecipitationEffect")!=0) _foundParticles = true;
        
        if (!_foundParticles ||
            !_foundPagedLOD) traverse(node);
    }

    virtual void apply(osg::PagedLOD& node)
    {
        _foundPagedLOD = true;
        apply((osg::Node&)node);
    }    
    
    bool _foundParticles;
    bool _foundPagedLOD;
};

bool OsgCameraGroup::realize()
{
    if( _initialized ) return _realized;

    if (!_ds) _ds = osg::DisplaySettings::instance();

    if (_enableProccessAffinityHint)
    {
        unsigned int numProcessors = OpenThreads::GetNumberOfProcessors();
        if (numProcessors>1)
        {
            for( unsigned int i = 0; i < _cfg->getNumberOfCameras(); i++ )
            {
                Producer::Camera *cam = _cfg->getCamera(i);
                cam->setProcessorAffinity(i % numProcessors);
            }    
        }
    }

    _shvec.clear();
    
    osg::Node* node = getTopMostSceneData();
    if (node)
    {
        // traverse the scene graphs gathering the requirements of the OpenGL buffers.
        osgUtil::DisplayRequirementsVisitor drv;
        drv.setDisplaySettings(_ds.get());
        node->accept(drv);
    }

    // set up each render stage to clear the appropriate buffers.
    GLbitfield clear_mask=0;
    if (_ds->getRGB())              clear_mask |= GL_COLOR_BUFFER_BIT;
    if (_ds->getDepthBuffer())      clear_mask |= GL_DEPTH_BUFFER_BIT;
    if (_ds->getStencilBuffer())    clear_mask |= GL_STENCIL_BUFFER_BIT;

    // make sure any camera's which share the same render surface also share the same osg::State.
    // use a std::map to keep track of what render surfaces are associated with what state.
    typedef std::map<Producer::RenderSurface*,osg::State*> RenderSurfaceStateMap;
    RenderSurfaceStateMap _renderSurfaceStateMap;

    bool needToCreatedNewContextID = true;
    unsigned int contextID = 0;

    for(unsigned int i = 0; i < _cfg->getNumberOfCameras(); i++ )
    {
        Producer::Camera *cam = _cfg->getCamera(i);
        Producer::RenderSurface* rs = cam->getRenderSurface();
        
        cam->setClearColor(_clear_color[0],_clear_color[1],_clear_color[2],_clear_color[3]);
        
        // get or create the scene handler.
        osgProducer::OsgSceneHandler *sh;
        sh = dynamic_cast<osgProducer::OsgSceneHandler*>(cam->getSceneHandler());
        if(sh == NULL)
            sh = new osgProducer::OsgSceneHandler(_ds.get()); 
    
        osgUtil::SceneView* sv = sh->getSceneView();
        sv->setDefaults(_realizeSceneViewOptions);
        
        if (_renderSurfaceStateMap.count(rs)==0)
        {
            _renderSurfaceStateMap[rs] = sv->getState();

            if (needToCreatedNewContextID)
            {
                // create a unique contextID for this graphics context.
                contextID = osg::GraphicsContext::createNewContextID();
                
                // if we are sharing our graphics context then when needn't create any more.
                needToCreatedNewContextID = !Producer::RenderSurface::allGLContextsAreShared();
            }
            else
            {
                osg::GraphicsContext::incrementContextIDUsageCount(contextID);
            }

            sv->getState()->setContextID(contextID);
        }
        else
        {
            sv->setState(_renderSurfaceStateMap[rs]);
        }

        _shvec.push_back( sh );
        cam->setSceneHandler( sh );
        
        // set up the clear mask.
        osgUtil::RenderStage *stage = sv->getRenderStage();
        if (stage) stage->setClearMask(clear_mask);

        // set the realize callback.
        rs->setRealizeCallback( new RenderSurfaceRealizeCallback(this, sh));
        
        // set up the visual chooser.
        if (_ds.valid())
        {
        
            Producer::VisualChooser* rs_vc = rs->getVisualChooser();
            if (!rs_vc)
            {
                rs_vc = new Producer::VisualChooser;
                rs_vc->setSimpleConfiguration();
                rs->setVisualChooser(rs_vc);
            }

            unsigned int numStencilBits = 0;
            if (_ds->getStereo())
            {
                switch(_ds->getStereoMode())
                {
                    case(osg::DisplaySettings::QUAD_BUFFER): 
                        rs_vc->useStereo();
                        break;
                    case(osg::DisplaySettings::HORIZONTAL_INTERLACE):
                    case(osg::DisplaySettings::VERTICAL_INTERLACE):
                        numStencilBits = 8;
                        break;
                    default:
                        break;
                }
            }
            
            // set up stencil buffer if required.            
            numStencilBits = osg::maximum(numStencilBits,_ds->getMinimumNumStencilBits());
            if (numStencilBits > 0)
            {
                rs_vc->setStencilSize(numStencilBits);
            }
            
            if (_ds->getAlphaBuffer()) rs_vc->setAlphaSize(_ds->getMinimumNumAlphaBits());

            rs_vc->setDepthSize(24);

            if (_ds->getAccumBuffer())
            {
                rs_vc->setAccumRedSize(_ds->getMinimumNumAccumRedBits());
                rs_vc->setAccumGreenSize(_ds->getMinimumNumAccumGreenBits());
                rs_vc->setAccumBlueSize(_ds->getMinimumNumAccumBlueBits());
                rs_vc->setAccumAlphaSize(_ds->getMinimumNumAccumAlphaBits());
            }
            
            if (_ds->getMultiSamples())
            {
                rs_vc->addAttribute(Producer::VisualChooser::SampleBuffers, 1);
                rs_vc->addAttribute(Producer::VisualChooser::Samples, _ds->getNumMultiSamples());
            }
        }        
    }

    // now set up GraphicsContext wrappers for each of the render surfaces
    // to all core OSG classes to keep track of the graphics context.
    for(RenderSurfaceStateMap::iterator ritr = _renderSurfaceStateMap.begin();
        ritr != _renderSurfaceStateMap.end();
        ++ritr)
    {
        Producer::RenderSurface* rs = ritr->first;
        osg::State* state = ritr->second;
        GraphicsContextImplementation* gc = new GraphicsContextImplementation(rs);
        gc->setState(state);
        state->setGraphicsContext(gc);
        _gcList.push_back(gc);
    }
    
    if( _global_stateset == NULL && _shvec.size() > 0 )
    {
        SceneHandlerList::iterator p = _shvec.begin();
        _global_stateset = (*p)->getSceneView()->getGlobalStateSet();
    }

    setUpSceneViewsWithData();

    // if we are multi-threaded check to see if particle exists in the scene
    // if so we need to disable multi-threading of cameras.
    if (_threadModel == Producer::CameraGroup::ThreadPerCamera)
    {
        if (getTopMostSceneData())
        {
            SearchForSpecialNodes sfsn;
            getTopMostSceneData()->accept(sfsn);
            if (sfsn._foundParticles)
            {
                osg::notify(osg::INFO)<<"Warning: disabling multi-threading of cull and draw"<<std::endl;
                osg::notify(osg::INFO)<<"         to avoid threading problems in osgParticle."<<std::endl;
                _threadModel = Producer::CameraGroup::SingleThreaded;
            }
        }
        
    }

    // if we are still multi-thread check to make sure that no render surfaces
    // are shared and don't use shared contexts, if they do we need to 
    // disable multi-threading of cameras.
    if (_threadModel == Producer::CameraGroup::ThreadPerCamera)
    {
        std::set<RenderSurface*> renderSurfaceSet;
        for( unsigned int i = 0; i < _cfg->getNumberOfCameras(); i++ )
        {
            Producer::Camera *cam = _cfg->getCamera(i);
            renderSurfaceSet.insert(cam->getRenderSurface());
        }
        if (renderSurfaceSet.size()!=_cfg->getNumberOfCameras())
        {
            // camera's must be sharing a RenderSurface, so we need to ensure that we're
            // running single threaded, to avoid OpenGL threading issues.
            osg::notify(osg::INFO)<<"Warning: disabling multi-threading of cull and draw to avoid"<<std::endl;
            osg::notify(osg::INFO)<<"         threading problems when camera's share a RenderSurface."<<std::endl;
            _threadModel = Producer::CameraGroup::SingleThreaded;
        }
        else if (renderSurfaceSet.size()>1 && RenderSurface::allGLContextsAreShared())
        {
            // we have multiple RenderSurface, but with share contexts, which is dangerous for multi-threaded usage,
            // so need to disable multi-threading to prevent problems.
            osg::notify(osg::INFO)<<"Warning: disabling multi-threading of cull and draw to avoid"<<std::endl;
            osg::notify(osg::INFO)<<"         threading problems when sharing graphics contexts within RenderSurface."<<std::endl;
            _threadModel = Producer::CameraGroup::SingleThreaded;
        }

    }
    
    if (_threadModel==Producer::CameraGroup::SingleThreaded)
    {
        osg::notify(osg::INFO)<<"OsgCameraGroup::realize() _threadModel==Producer::CameraGroup::SingleThreaded"<<std::endl;
    }
    else
    {
        osg::notify(osg::INFO)<<"OsgCameraGroup::realize() _threadModel==Producer::CameraGroup::ThreadPerCamera"<<std::endl;
    }

    if (_cfg.valid()) _cfg->setThreadModelDirective(_threadModel);

    _initialized = CameraGroup::realize();
    
    return _initialized;
}

osg::Node* OsgCameraGroup::getTopMostSceneData()
{
    if (_scene_decorator.valid())
        return _scene_decorator.get();
    else
        return _scene_data.get(); 
}

const osg::Node* OsgCameraGroup::getTopMostSceneData() const
{
    if (_scene_decorator.valid())
        return _scene_decorator.get();
    else
        return _scene_data.get(); 
}

void OsgCameraGroup::setView(const osg::Matrixd& matrix)
{
    Producer::Matrix pm(matrix.ptr());
            
    setViewByMatrix(pm);
}

osg::Matrixd OsgCameraGroup::getViewMatrix() const
{
    osg::Matrixd matrix;
    if (_cfg.valid() && _cfg->getNumberOfCameras()>=1)
    {
        const Producer::Camera *cam = _cfg->getCamera(0);
        matrix.set(cam->getViewMatrix());
    }
    return matrix;
}

static osg::Timer_t _last_frame_tick = 0;
static osg::Timer_t _previous_previous_frame_tick = 0;
static osg::Timer_t _previous_frame_tick = 0;
static bool _useStartOfUpdateForFrameTime = true;

void OsgCameraGroup::sync()
{
    CameraGroup::sync();

    // set the frame stamp for the new frame.
    _frameStamp->setFrameNumber(_frameNumber++);
    
    if (_useStartOfUpdateForFrameTime)
    {
        double time_since_start = _timer.delta_s(_start_tick,_timer.tick());
        _frameStamp->setReferenceTime(time_since_start);
    }
    else
    {   
        osg::Timer_t endOfNewFrameTick = _last_frame_tick + (_last_frame_tick-_previous_previous_frame_tick);
        double estimatedSwapTimeForFrame = _timer.delta_s(_start_tick,endOfNewFrameTick);

        _frameStamp->setReferenceTime(estimatedSwapTimeForFrame);
    }
}        

void OsgCameraGroup::frame()
{
    if (!_useStartOfUpdateForFrameTime)
    {
        _previous_previous_frame_tick = _previous_frame_tick;
        _previous_frame_tick = _last_frame_tick;
        _last_frame_tick = _timer.tick();
    }
    
    osg::Node* node = getTopMostSceneData();
    if (node) node->getBound();


    // pass on the cull settings to the scene views to keep
    // the settings in sync.
    for(SceneHandlerList::iterator itr = _shvec.begin();
        itr != _shvec.end();
        ++itr)
    {
        (*itr)->getSceneView()->inheritCullSettings(_cullSettings);
    }
 
 
     CameraGroup::frame();
}

void OsgCameraGroup::cleanup_frame()
{
    // first relase all GL objects and switch on the flush of deleted objects
    // in the next frame.
    for(SceneHandlerList::iterator itr = _shvec.begin();
        itr != _shvec.end();
        ++itr)
    {
        (*itr)->getSceneView()->releaseAllGLObjects();
        (*itr)->getSceneView()->setRenderStage(0);
        (*itr)->setCleanUpOnNextFrame(true);
    }
    
    // make sure that the registry all flushes all its texture objects.
    osgDB::Registry::instance()->releaseGLObjects();
    
    // then run the frame to do the actuall OpenGL clean up.
    frame();
}


