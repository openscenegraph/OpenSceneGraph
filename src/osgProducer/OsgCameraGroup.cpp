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

#include <osgUtil/DisplayRequirementsVisitor>
#include <osgDB/FileUtils>

#include <osgProducer/OsgCameraGroup>

using namespace Producer;
using namespace osgProducer;


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
}

void OsgCameraGroup::_init()
{
    _scene_data = NULL;
    _global_stateset = NULL;
    _background_color.set( 0.2f, 0.2f, 0.4f, 1.0f );
    _LODScale = 1.0f;

    _fusionDistanceMode = osgUtil::SceneView::USE_CAMERA_FUSION_DISTANCE;
    _fusionDistanceValue = 1.0f;

    _initialized = false;

    if (!_frameStamp) _frameStamp = new osg::FrameStamp;

    // set up the maximum number of graphics contexts, before loading the scene graph
    // to ensure that texture objects and display buffers are configured to the correct size.
    osg::DisplaySettings::instance()->setMaxNumberOfGraphicsContexts( getNumberOfCameras() );

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
        
    setUpSceneViewsWithData();
}
        
void OsgCameraGroup::setSceneDecorator( osg::Group* decorator)
{
    if (_scene_decorator==decorator) return;

    _scene_decorator = decorator;

    if (_scene_data.valid() && decorator) 
    {
        decorator->addChild(_scene_data.get());
    }
    setUpSceneViewsWithData();
}

        
void OsgCameraGroup::setUpSceneViewsWithData()
{
    for(SceneHandlerList::iterator  p = _shvec.begin(); p != _shvec.end(); p++ )
    {
        if (_scene_decorator.valid())
        {
            (*p)->setSceneData( _scene_decorator.get() );
        }
        else if (_scene_data.valid())
        {
            (*p)->setSceneData( _scene_data.get() );
        }
        else
        {
            (*p)->setSceneData( 0 );
        }
        
        (*p)->setFrameStamp( _frameStamp.get() );
        (*p)->setGlobalStateSet( _global_stateset.get() );
        (*p)->setBackgroundColor( _background_color );
        (*p)->setLODScale( _LODScale );
        (*p)->setFusionDistance( _fusionDistanceMode, _fusionDistanceValue );
        (*p)->getState()->reset();
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

void OsgCameraGroup::realize( ThreadingModel thread_model)
{
    if( _initialized ) return;


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

    for( unsigned int i = 0; i < _cfg->getNumberOfCameras(); i++ )
    {
        Producer::Camera *cam = _cfg->getCamera(i);
        
        // create the scene handler.
        osgProducer::OsgSceneHandler *sh = new osgProducer::OsgSceneHandler(_ds.get());
        sh->setDefaults();
        sh->getState()->setContextID(i);

        _shvec.push_back( sh );
        cam->setSceneHandler( sh );
        
        // set up the clear mask.
        osgUtil::RenderStage *stage = sh->getRenderStage();
        if (stage) stage->setClearMask(clear_mask);

        // set the realize callback.
        Producer::RenderSurface* rs = cam->getRenderSurface();
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
        _global_stateset = (*p)->getGlobalStateSet();
    }

    setUpSceneViewsWithData();

    CameraGroup::realize( thread_model );        
    _initialized = true;
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

void OsgCameraGroup::frame()
{
    osg::Node* node = getTopMostSceneData();
    if (node) node->getBound();

    CameraGroup::frame();
    _frameStamp->setFrameNumber( _frameStamp->getFrameNumber() + 1 );
}

