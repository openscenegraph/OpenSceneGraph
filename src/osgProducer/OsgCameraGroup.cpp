/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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

using namespace Producer;
using namespace osgProducer;

OsgCameraGroup::RealizeCallback::~RealizeCallback()
{
}

class RenderSurfaceRealizeCallback : public Producer::RenderSurface::Callback
{
public:

    RenderSurfaceRealizeCallback(OsgCameraGroup* cameraGroup,OsgSceneHandler* sceneHandler):
        _cameraGroup(cameraGroup),
        _sceneHandler(sceneHandler) {}
    
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

    OsgCameraGroup* _cameraGroup;
    OsgSceneHandler* _sceneHandler;

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


void OsgCameraGroup::_init()
{
    _thread_model = ThreadPerCamera;

    _scene_data = NULL;
    _global_stateset = NULL;
    _background_color.set( 0.2f, 0.2f, 0.4f, 1.0f );
    _LODScale = 1.0f;

    _fusionDistanceMode = osgUtil::SceneView::PROPORTIONAL_TO_SCREEN_DISTANCE;
    _fusionDistanceValue = 1.0f;
    
    _realizeSceneViewOptions = osgUtil::SceneView::STANDARD_SETTINGS;

    _initialized = false;

    // set up the time and frame counter.
    _frameNumber = 0;
    _start_tick = _timer.tick();

    if (!_frameStamp) _frameStamp = new osg::FrameStamp;

    // set up the maximum number of graphics contexts, before loading the scene graph
    // to ensure that texture objects and display buffers are configured to the correct size.
    osg::DisplaySettings::instance()->setMaxNumberOfGraphicsContexts( getNumberOfCameras() );
    
    _applicationUsage = osg::ApplicationUsage::instance();

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
        sv->setClearColor( _background_color );
        sv->setLODScale( _LODScale );
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

void OsgCameraGroup::setBackgroundColor( const osg::Vec4& backgroundColor ) 
{
    _background_color = backgroundColor;
    setUpSceneViewsWithData();
}

void OsgCameraGroup::setLODScale( float scale )
{
    // need to set a local variable?
    _LODScale = scale;
    setUpSceneViewsWithData();
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
    _thread_model = thread_model;
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
        if (strcmp(node.libraryName(),"osgParticle")==0) _foundParticles = true;
        
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

    _ds->setMaxNumberOfGraphicsContexts( _cfg->getNumberOfCameras() );
    
    _shvec.clear();
    
    osg::Node* node = getTopMostSceneData();
    if (node)
    {
        // traverse the scene graphs gathering the requirements of the OpenGL buffers.
        osgUtil::DisplayRequirementsVisitor drv;
        drv.setDisplaySettings(_ds.get());
        node->accept(drv);
    }
    
    unsigned int numMultiSamples = 0;

    #ifdef __sgi
    // switch on anti-aliasing by default, just in case we have an Onyx :-)
    numMultiSamples = 4;
    #endif

    // set up each render stage to clear the appropriate buffers.
    GLbitfield clear_mask=0;
    if (_ds->getRGB())              clear_mask |= GL_COLOR_BUFFER_BIT;
    if (_ds->getDepthBuffer())      clear_mask |= GL_DEPTH_BUFFER_BIT;
    if (_ds->getStencilBuffer())    clear_mask |= GL_STENCIL_BUFFER_BIT;

    // make sure any camera's which share the same render surface also share the same osg::State.
    // use a std::map to keep track of what render surfaces are associated with what state.
    typedef std::map<Producer::RenderSurface*,osg::State*> RenderSurfaceStateMap;
    RenderSurfaceStateMap _renderSurfaceStateMap;
    unsigned int contextID = 0;

    for(unsigned int i = 0; i < _cfg->getNumberOfCameras(); i++ )
    {
        Producer::Camera *cam = _cfg->getCamera(i);
        Producer::RenderSurface* rs = cam->getRenderSurface();
        
        // create the scene handler.
        osgProducer::OsgSceneHandler *sh = new osgProducer::OsgSceneHandler(_ds.get());

        osgUtil::SceneView* sv = sh->getSceneView();
        sv->setDefaults(_realizeSceneViewOptions);
        
        if (_renderSurfaceStateMap.count(rs)==0)
        {
            _renderSurfaceStateMap[rs] = sv->getState();
            sv->getState()->setContextID(contextID++);
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
        if (_ds.valid() || numMultiSamples!=0)
        {
        
            Producer::VisualChooser* rs_vc = rs->getVisualChooser();
            if (!rs_vc)
            {
                rs_vc = new Producer::VisualChooser;
                rs_vc->setSimpleConfiguration();
                rs->setVisualChooser(rs_vc);
            }
            if (_ds->getStereo() && _ds->getStereoMode()==osg::DisplaySettings::QUAD_BUFFER) rs_vc->useStereo();
            if (_ds->getStencilBuffer()) rs_vc->setStencilSize(_ds->getMinimumNumStencilBits());
            if (_ds->getAlphaBuffer()) rs_vc->setAlphaSize(_ds->getMinimumNumAlphaBits());

            rs_vc->setDepthSize(24);

            if (numMultiSamples)
            {
                #if defined( GLX_SAMPLES_SGIS )
                    rs_vc->addExtendedAttribute( GLX_SAMPLES_SGIS,  numMultiSamples);
                #endif
                #if defined( GLX_SAMPLES_BUFFER_SGIS )
                    rs_vc->addExtendedAttribute( GLX_SAMPLES_BUFFER_SGIS,  1);
                #endif
            }
        }        
    }

    
    if( _global_stateset == NULL && _shvec.size() > 0 )
    {
        SceneHandlerList::iterator p = _shvec.begin();
        _global_stateset = (*p)->getSceneView()->getGlobalStateSet();
    }

    setUpSceneViewsWithData();
    
    if (getTopMostSceneData())
    {
        SearchForSpecialNodes sfsn;
        getTopMostSceneData()->accept(sfsn);
        if (sfsn._foundParticles)
        {
            osg::notify(osg::INFO)<<"Warning: disabling multi-threading of cull and draw"<<std::endl;
            osg::notify(osg::INFO)<<"         to avoid threading problems in osgParticle."<<std::endl;
            _thread_model = Producer::CameraGroup::SingleThreaded;
        }
        else
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
                _thread_model = Producer::CameraGroup::SingleThreaded;
            }
            
            
        }
        
    }
    

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

void OsgCameraGroup::sync()
{
    CameraGroup::sync();

    // set the frame stamp for the new frame.
    double time_since_start = _timer.delta_s(_start_tick,_timer.tick());
    _frameStamp->setFrameNumber(_frameNumber++);
    _frameStamp->setReferenceTime(time_since_start);
}        

void OsgCameraGroup::frame()
{
    osg::Node* node = getTopMostSceneData();
    if (node) node->getBound();

    CameraGroup::frame();
}

