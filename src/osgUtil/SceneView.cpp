#include <osgUtil/SceneView>
#include <osgUtil/AppVisitor>
#include <osgUtil/DisplayListVisitor>

#include <osg/Notify>
#include <osg/Texture>
#include <osg/AlphaFunc>
#include <osg/TexEnv>
#include <osg/ColorMatrix>
#include <osg/LightModel>

#include <osg/GLU>

using namespace osg;
using namespace osgUtil;

SceneView::SceneView(DisplaySettings* ds)
{
    _displaySettings = ds;

    _calc_nearfar = true;

    _backgroundColor.set(0.2f, 0.2f, 0.4f, 1.0f);

    _near_plane = 1.0f;
    _far_plane = 1.0f;

    _lodBias = 1.0f;

    _lightingMode=HEADLIGHT;
    
    _prioritizeTextures = false;
    
    _viewport = osgNew Viewport;
    
    _initCalled = false;

    _cullMask = 0xffffffff;
    _cullMaskLeft = 0xffffffff;
    _cullMaskRight = 0xffffffff;

}


SceneView::~SceneView()
{
}


void SceneView::setDefaults()
{
    _globalState = osgNew osg::StateSet;

    _lightingMode=HEADLIGHT;
    _light = osgNew osg::Light;
    _light->setLightNum(0);
    _light->setAmbient(Vec4(0.00f,0.0f,0.00f,1.0f));
    _light->setDiffuse(Vec4(0.8f,0.8f,0.8f,1.0f));
    _light->setSpecular(Vec4(1.0f,1.0f,1.0f,1.0f));

    _state = osgNew State;
    
    _camera = osgNew Camera(_displaySettings.get());

    _rendergraph = osgNew RenderGraph;
    _renderStage = osgNew RenderStage;


//#ifndef __sgi
    // sgi's IR graphics has a problem with lighting and display lists, as it seems to store 
    // lighting state with the display list, and the display list visitor doesn't currently apply
    // state before creating display lists. So will disable the init visitor default, this won't
    // affect functionality since the display lists will be created as and when needed.
    DisplayListVisitor* dlv = osgNew DisplayListVisitor();
    dlv->setState(_state.get());
    dlv->setNodeMaskOverride(0xffffffff);
    _initVisitor = dlv;
//#endif

    _appVisitor = osgNew AppVisitor;    

    _cullVisitor = osgNew CullVisitor;

    _cullVisitor->setRenderGraph(_rendergraph.get());
    _cullVisitor->setRenderStage(_renderStage.get());

    _globalState->setGlobalDefaults();
    
    // enable lighting by default.
    _globalState->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    _light->setStateSetModes(*_globalState,osg::StateAttribute::ON);
    
    // enable depth testing by default.
    _globalState->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

    // set up an alphafunc by default to speed up blending operations.
    osg::AlphaFunc* alphafunc = osgNew osg::AlphaFunc;
    alphafunc->setFunction(osg::AlphaFunc::GREATER,0.0f);
    _globalState->setAttributeAndModes(alphafunc, osg::StateAttribute::ON);

    // set up an alphafunc by default to speed up blending operations.
    osg::TexEnv* texenv = osgNew osg::TexEnv;
    texenv->setMode(osg::TexEnv::MODULATE);
    _globalState->setAttributeAndModes(texenv, osg::StateAttribute::ON);

    osg::LightModel* lightmodel = osgNew osg::LightModel;
    lightmodel->setAmbientIntensity(osg::Vec4(0.0f,0.0f,0.0f,1.0f));
    _globalState->setAttributeAndModes(lightmodel, osg::StateAttribute::ON);

    _backgroundColor.set(0.2f, 0.2f, 0.4f, 1.0f);

    _cullMask = 0xffffffff;
    _cullMaskLeft = 0xffffffff;
    _cullMaskLeft = 0xffffffff;
}

void SceneView::init()
{
    _initCalled = true;

    if (_sceneData.valid() && _initVisitor.valid())
    {
        _initVisitor->reset();
        _initVisitor->setFrameStamp(_frameStamp.get());
        
        if (_frameStamp.valid())
        {
             _initVisitor->setTraversalNumber(_frameStamp->getFrameNumber());
        }
        
        _sceneData->accept(*_initVisitor.get());
        
    } 
}

void SceneView::app()
{
    if (!_initCalled) init();

    if (_sceneData.valid() && _appVisitor.valid())
    { 
        _appVisitor->reset();

        _appVisitor->setFrameStamp(_frameStamp.get());

        // use the frame number for the traversal number.
        if (_frameStamp.valid())
        {
             _appVisitor->setTraversalNumber(_frameStamp->getFrameNumber());
        }
        
        _sceneData->accept(*_appVisitor.get());
    }
    
    
}

void SceneView::cull()
{

    if (_displaySettings.valid() && _displaySettings->getStereo()) 
    {
    
        _camera->setScreenDistance(_displaySettings->getScreenDistance());

        _cameraLeft = osgNew osg::Camera(*_camera);
        _cameraRight = osgNew osg::Camera(*_camera);

        float iod = _displaySettings->getEyeSeperation();

        _cameraLeft->adjustEyeOffsetForStereo(osg::Vec3(-iod*0.5,0.0f,0.0f));
        _cameraRight->adjustEyeOffsetForStereo(osg::Vec3(iod*0.5,0.0f,0.0f));

        if (!_cullVisitorLeft.valid()) _cullVisitorLeft = dynamic_cast<CullVisitor*>(_cullVisitor->cloneType());
        if (!_rendergraphLeft.valid()) _rendergraphLeft = dynamic_cast<RenderGraph*>(_rendergraph->cloneType());
        if (!_renderStageLeft.valid()) _renderStageLeft = dynamic_cast<RenderStage*>(_renderStage->cloneType());

        if (!_cullVisitorRight.valid()) _cullVisitorRight = dynamic_cast<CullVisitor*>(_cullVisitor->cloneType());
        if (!_rendergraphRight.valid()) _rendergraphRight = dynamic_cast<RenderGraph*>(_rendergraph->cloneType());
        if (!_renderStageRight.valid()) _renderStageRight = dynamic_cast<RenderStage*>(_renderStage->cloneType());

        _cullVisitorLeft->setTraversalMask(_cullMaskLeft);
        cullStage(_cameraLeft.get(),_cullVisitorLeft.get(),_rendergraphLeft.get(),_renderStageLeft.get());

        _cullVisitorRight->setTraversalMask(_cullMaskRight);
        cullStage(_cameraRight.get(),_cullVisitorRight.get(),_rendergraphRight.get(),_renderStageRight.get());

    }
    else
    {
        _cullVisitor->setTraversalMask(_cullMask);
        cullStage(_camera.get(),_cullVisitor.get(),_rendergraph.get(),_renderStage.get());
    }

}

void SceneView::cullStage(osg::Camera* camera, osgUtil::CullVisitor* cullVisitor, osgUtil::RenderGraph* rendergraph, osgUtil::RenderStage* renderStage)
{

    if (!_sceneData || !_viewport->valid()) return;

    if (!_initCalled) init();

    camera->adjustAspectRatio(_viewport->aspectRatio());
    
    

    cullVisitor->reset();

    cullVisitor->setFrameStamp(_frameStamp.get());

    // use the frame number for the traversal number.
    if (_frameStamp.valid())
    {
         cullVisitor->setTraversalNumber(_frameStamp->getFrameNumber());
    }

    // get the camera's modelview
    osg::Matrix* modelview = osgNew osg::Matrix(camera->getModelViewMatrix());


    // take a copy of camera, and init it home
    osg::Camera* local_camera = osgNew Camera(*camera);
    local_camera->home(); 
    local_camera->attachTransform(osg::Camera::NO_ATTACHED_TRANSFORM); 


    cullVisitor->setLODBias(_lodBias);
    cullVisitor->setCamera(*local_camera);
    cullVisitor->setViewport(_viewport.get());
    cullVisitor->setEarthSky(NULL); // reset earth sky on each frame.
    
    cullVisitor->setRenderGraph(rendergraph);
    cullVisitor->setRenderStage(renderStage);

    // SandB
    //now make it compute "clipping directions" needed for detailed culling
    if(cullVisitor->getDetailedCulling()) 
	    cullVisitor->calcClippingDirections();//only once pre frame

    renderStage->reset();

    // comment out reset of rendergraph since clean is more efficient.
    //  rendergraph->reset();

    // use clean of the rendergraph rather than reset, as it is able to
    // reuse the structure on the rendergraph in the next frame. This
    // achieves a certain amount of frame cohereancy of memory allocation.
    rendergraph->clean();

    renderStage->setViewport(_viewport.get());
    renderStage->setCamera(local_camera);
    renderStage->setClearColor(_backgroundColor);


    switch(_lightingMode)
    {
    case(HEADLIGHT):
        renderStage->addLight(_light.get(),NULL);
        break;
    case(SKY_LIGHT):
        renderStage->addLight(_light.get(),modelview);
        break;
    case(NO_SCENEVIEW_LIGHT):
        break;
    }            

    if (_globalState.valid()) cullVisitor->pushStateSet(_globalState.get());


    cullVisitor->pushCullViewState(modelview);
    

    // traverse the scene graph to generate the rendergraph.
    _sceneData->accept(*cullVisitor);

    if (_globalState.valid()) cullVisitor->popStateSet();
    
    cullVisitor->popCullViewState();


    const osg::EarthSky* earthSky = cullVisitor->getEarthSky();
    if (earthSky)
    {
        if (earthSky->getRequiresClear())
        {
            renderStage->setClearColor(earthSky->getClearColor());
            renderStage->setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            // really should set clear mask here, but what to? Need
            // to consider the stencil and accumulation buffers..
            // will defer to later.  Robert Osfield. October 2001.
        }
        else
        {
            // we have an earth sky implementation to do the work for use
            // so we don't need to clear.
            renderStage->setClearMask(0);
        }
    }


    if (_calc_nearfar)
    {
        _near_plane = cullVisitor->getCalculatedNearPlane();
        _far_plane = cullVisitor->getCalculatedFarPlane();

        if (_near_plane<=_far_plane)
        {
            // shift the far plane slight further away from the eye point.
            // and shift the near plane slightly near the eye point, this
            // will give a little space betwenn the near and far planes
            // and the model, crucial when the naer and far planes are
            // coincedent.
            _far_plane  *= 1.05;
            _near_plane *= 0.95;

            // if required clamp the near plane to prevent negative or near zero
            // near planes.
            if(!cullVisitor->getDetailedCulling())
            {
                float min_near_plane = _far_plane*0.0005f;
                if (_near_plane<min_near_plane) _near_plane=min_near_plane;
            }
        }
        else
        {
            _near_plane = 1.0f;
            _far_plane = 1000.0f;
        }

        local_camera->setNearFar(_near_plane,_far_plane);
        camera->setNearFar(_near_plane,_far_plane);
    }

    // prune out any empty RenderGraph children.
    // note, this would be not required if the rendergraph had been
    // reset at the start of each frame (see top of this method) but
    // a clean has been used instead to try to minimize the amount of
    // allocation and deleteing of the RenderGraph nodes.
    rendergraph->prune();
}


void SceneView::draw()
{
    if (_displaySettings.valid() && _displaySettings->getStereo()) 
    {
    
        _camera->setScreenDistance(_displaySettings->getScreenDistance());

        switch(_displaySettings->getStereoMode())
        {
        case(osg::DisplaySettings::QUAD_BUFFER):
            {

                glDrawBuffer(GL_BACK_LEFT);
                drawStage(_renderStageLeft.get());


                glDrawBuffer(GL_BACK_RIGHT);
                drawStage(_renderStageRight.get());

            }
            break;
        case(osg::DisplaySettings::ANAGLYPHIC):
            {
                
                // draw left eye.
                osg::ref_ptr<osg::ColorMask> red = osgNew osg::ColorMask;
                red->setMask(true,false,false,true);
                _globalState->setAttribute(red.get());
                _renderStageLeft->setColorMask(red.get());
                drawStage(_renderStageLeft.get());

                // draw right eye.
                osg::ref_ptr<osg::ColorMask> green = osgNew osg::ColorMask;
                green->setMask(false,true,true,true);
                _globalState->setAttribute(green.get());
                _renderStageRight->setColorMask(green.get());
                drawStage(_renderStageRight.get());

            }
            break;
        case(osg::DisplaySettings::HORIZONTAL_SPLIT):
            {
                int left_half = _viewport->width()/2;
                int right_half = _viewport->width()-left_half;
                

                // draw left eye.
                osg::ref_ptr<osg::Viewport> viewportLeft = osgNew osg::Viewport;
                viewportLeft->setViewport(_viewport->x(),_viewport->y(),left_half,_viewport->height());
                _renderStageLeft->setViewport(viewportLeft.get());
                drawStage(_renderStageLeft.get());

                // draw right eye.
                osg::ref_ptr<osg::Viewport> viewportRight = osgNew osg::Viewport;
                viewportRight->setViewport(_viewport->x()+left_half,_viewport->y(),right_half,_viewport->height());
                _renderStageRight->setViewport(viewportRight.get());
                drawStage(_renderStageRight.get());

            }
            break;
        case(osg::DisplaySettings::VERTICAL_SPLIT):
            {
                int bottom_half = _viewport->height()/2;
                int top_half = _viewport->height()-bottom_half;

                // draw left eye.
                // assume left eye at top, this could be implementation dependant...
                osg::ref_ptr<osg::Viewport> viewportLeft = osgNew osg::Viewport;
                viewportLeft->setViewport(_viewport->x(),_viewport->y()+bottom_half,_viewport->width(),top_half);
                _renderStageLeft->setViewport(viewportLeft.get());
                drawStage(_renderStageLeft.get());

                // draw right eye.
                // assume right eye at top, this could be implementation dependant...
                osg::ref_ptr<osg::Viewport> viewportRight = osgNew osg::Viewport;
                viewportRight->setViewport(_viewport->x(),_viewport->y(),_viewport->width(),bottom_half);
                _renderStageRight->setViewport(viewportRight.get());
                drawStage(_renderStageRight.get());

            }
            break;
        default:
            {
                osg::notify(osg::NOTICE)<<"Warning: stereo camera mode not implemented yet."<< std::endl;
                drawStage(_renderStageLeft.get());
            }
            break;
        }
    }
    else
    {
        // bog standard draw.
        drawStage(_renderStage.get());
    }

}

void SceneView::drawStage(osgUtil::RenderStage* renderStage)
{
    if (!_sceneData || !_viewport->valid()) return;

    if (!_state)
    {
        osg::notify(osg::WARN) << "Warning: no valid osgUtil::SceneView::_state"<< std::endl;
        osg::notify(osg::WARN) << "         creating a state automatically."<< std::endl;

        // note the constructor for osg::State will set ContextID to 0.
        _state = osgNew osg::State;
    }
    // we in theory should be able to 
    _state->reset();
    
    _state->setFrameStamp(_frameStamp.get());
    _state->setDisplaySettings(_displaySettings.get());

    // note, to support multi-pipe systems the deletion of OpenGL display list
    // and texture objects is deferred until the OpenGL context is the correct
    // context for when the object were originally created.  Here we know what
    // context we are in so can flush the appropriate caches.
    osg::Drawable::flushDeletedDisplayLists(_state->getContextID());
    osg::Texture::flushDeletedTextureObjects(_state->getContextID());

    RenderLeaf* previous = NULL;
    
    renderStage->draw(*_state,previous);
        
    GLenum errorNo = glGetError();
    if (errorNo!=GL_NO_ERROR)
    {
        osg::notify(WARN)<<"Warning: detected OpenGL error '"<<gluErrorString(errorNo)<<"'"<< std::endl;
    }
    
}


/** Calculate, via glUnProject, the object coordinates of a window point.
    Note, current implementation requires that SceneView::draw() has been previously called
    for projectWindowIntoObject to produce valid values.  As per OpenGL
    windows coordinates are calculated relative to the bottom left of the window.*/
bool SceneView::projectWindowIntoObject(const osg::Vec3& window,osg::Vec3& object) const
{
    return _camera->unproject(window,*_viewport,object);
}


/** Calculate, via glUnProject, the object coordinates of a window x,y
    when projected onto the near and far planes.
    Note, current implementation requires that SceneView::draw() has been previously called
    for projectWindowIntoObject to produce valid values.  As per OpenGL
    windows coordinates are calculated relative to the bottom left of the window.*/
bool SceneView::projectWindowXYIntoObject(int x,int y,osg::Vec3& near_point,osg::Vec3& far_point) const
{
    bool result_near = _camera->unproject(Vec3(x,y,0.0f),*_viewport,near_point);
    bool result_far =  _camera->unproject(Vec3(x,y,1.0f),*_viewport,far_point);
    return result_near & result_far;
}


/** Calculate, via glProject, the object coordinates of a window.
    Note, current implementation requires that SceneView::draw() has been previously called
    for projectWindowIntoObject to produce valid values.  As per OpenGL
    windows coordinates are calculated relative to the bottom left of the window.*/
bool SceneView::projectObjectIntoWindow(const osg::Vec3& object,osg::Vec3& window) const
{
    return _camera->project(object,*_viewport,window);
}
