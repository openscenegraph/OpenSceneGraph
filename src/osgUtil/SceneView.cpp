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


#ifndef __sgi
    // sgi's IR graphics has a problem with lighting and display lists, as it seems to store 
    // lighting state with the display list, and the display list visitor doesn't currently apply
    // state before creating display lists. So will disable the init visitor default, this won't
    // affect functionality since the display lists will be created as and when needed.
    DisplayListVisitor* dlv = osgNew DisplayListVisitor();
    dlv->setState(_state.get());
    dlv->setNodeMaskOverride(0xffffffff);
    _initVisitor = dlv;
#endif

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

    _state->reset();
   
    osg::ref_ptr<osg::Matrix> projection = _projectionMatrix.get();
    osg::ref_ptr<osg::Matrix> modelview = _modelviewMatrix.get();
    
    if (_camera.valid())
    {
        _camera->adjustAspectRatio(_viewport->aspectRatio());
        
        if (_displaySettings.valid())
            _camera->setScreenDistance(_displaySettings->getScreenDistance());
        
        if (!projection) projection = osgNew osg::Matrix(_camera->getProjectionMatrix());
        if (!modelview)  modelview  = osgNew osg::Matrix(_camera->getModelViewMatrix());
    }
    
    if (!projection) projection = osgNew osg::Matrix();
    if (!modelview)  modelview  = osgNew osg::Matrix();

    if (_displaySettings.valid() && _displaySettings->getStereo()) 
    {

        float fusionDistance = _displaySettings->getScreenDistance();

        if (_camera.valid())
        {
            fusionDistance = _camera->getFusionDistance();
        }
        
        float iod = _displaySettings->getEyeSeperation();
        float sd = _displaySettings->getScreenDistance();
        float es = 0.5f*iod*(fusionDistance/sd);

        if (!_cullVisitorLeft.valid()) _cullVisitorLeft = dynamic_cast<CullVisitor*>(_cullVisitor->cloneType());
        if (!_rendergraphLeft.valid()) _rendergraphLeft = dynamic_cast<RenderGraph*>(_rendergraph->cloneType());
        if (!_renderStageLeft.valid()) _renderStageLeft = dynamic_cast<RenderStage*>(_renderStage->cloneType());

        if (!_cullVisitorRight.valid()) _cullVisitorRight = dynamic_cast<CullVisitor*>(_cullVisitor->cloneType());
        if (!_rendergraphRight.valid()) _rendergraphRight = dynamic_cast<RenderGraph*>(_rendergraph->cloneType());
        if (!_renderStageRight.valid()) _renderStageRight = dynamic_cast<RenderStage*>(_renderStage->cloneType());

        
        // set up the left eye.
        osg::ref_ptr<osg::Matrix> projectionLeft = osgNew osg::Matrix(osg::Matrix(1.0f,0.0f,0.0f,0.0f,
                                                                      0.0f,1.0f,0.0f,0.0f,
                                                                      iod/(2.0f*sd),0.0f,1.0f,0.0f,
                                                                      0.0f,0.0f,0.0f,1.0f)*
                                                          (*projection));
        
        
        osg::ref_ptr<osg::Matrix> modelviewLeft = osgNew osg::Matrix( (*modelview) *
                                                         osg::Matrix(1.0f,0.0f,0.0f,0.0f,
                                                                     0.0f,1.0f,0.0f,0.0f,
                                                                     0.0f,0.0f,1.0f,0.0f,
                                                                     es,0.0f,0.0f,1.0f));
        
        _cullVisitorLeft->setTraversalMask(_cullMaskLeft);
        cullStage(projectionLeft.get(),modelviewLeft.get(),_cullVisitorLeft.get(),_rendergraphLeft.get(),_renderStageLeft.get());


        // set up the right eye.
        osg::ref_ptr<osg::Matrix> projectionRight = osgNew osg::Matrix(osg::Matrix(1.0f,0.0f,0.0f,0.0f,
                                                                      0.0f,1.0f,0.0f,0.0f,
                                                                      -iod/(2.0f*sd),0.0f,1.0f,0.0f,
                                                                      0.0f,0.0f,0.0f,1.0f)*
                                                          (*projection));

        osg::ref_ptr<osg::Matrix> modelviewRight = osgNew osg::Matrix( (*modelview) *
                                                         osg::Matrix(1.0f,0.0f,0.0f,0.0f,
                                                                     0.0f,1.0f,0.0f,0.0f,
                                                                     0.0f,0.0f,1.0f,0.0f,
                                                                     -es,0.0f,0.0f,1.0f));

        _cullVisitorRight->setTraversalMask(_cullMaskRight);
        cullStage(projectionRight.get(),modelviewRight.get(),_cullVisitorRight.get(),_rendergraphRight.get(),_renderStageRight.get());


    }
    else
    {
        _cullVisitor->setTraversalMask(_cullMask);
        cullStage(projection.get(),modelview.get(),_cullVisitor.get(),_rendergraph.get(),_renderStage.get());
    }

    if (_camera.valid() && _calc_nearfar)
    {
        _camera->setNearFar(_near_plane,_far_plane);
    }
    
}

void SceneView::cullStage(osg::Matrix* projection,osg::Matrix* modelview,osgUtil::CullVisitor* cullVisitor, osgUtil::RenderGraph* rendergraph, osgUtil::RenderStage* renderStage)
{

    if (!_sceneData || !_viewport->valid()) return;

    if (!_initCalled) init();

    if (!_state)
    {
        osg::notify(osg::WARN) << "Warning: no valid osgUtil::SceneView::_state"<< std::endl;
        osg::notify(osg::WARN) << "         creating a state automatically."<< std::endl;

        // note the constructor for osg::State will set ContextID to 0.
        _state = osgNew osg::State;
    }

    // we in theory should be able to be able to bypass reset, but we'll call it just incase.
    _state->reset();
    
    _state->setFrameStamp(_frameStamp.get());
    _state->setDisplaySettings(_displaySettings.get());



    cullVisitor->reset();

    cullVisitor->setFrameStamp(_frameStamp.get());

    // use the frame number for the traversal number.
    if (_frameStamp.valid())
    {
         cullVisitor->setTraversalNumber(_frameStamp->getFrameNumber());
    }

    cullVisitor->setLODBias(_lodBias);
    cullVisitor->setEarthSky(NULL); // reset earth sky on each frame.
    
    cullVisitor->setRenderGraph(rendergraph);
    cullVisitor->setRenderStage(renderStage);

//     // SandB
//     //now make it compute "clipping directions" needed for detailed culling
//     if(cullVisitor->getDetailedCulling()) 
// 	    cullVisitor->calcClippingDirections();//only once pre frame

    renderStage->reset();

    // comment out reset of rendergraph since clean is more efficient.
    //  rendergraph->reset();

    // use clean of the rendergraph rather than reset, as it is able to
    // reuse the structure on the rendergraph in the next frame. This
    // achieves a certain amount of frame cohereancy of memory allocation.
    rendergraph->clean();

    renderStage->setViewport(_viewport.get());
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


    cullVisitor->pushViewport(_viewport.get());
    cullVisitor->pushProjectionMatrix(projection);
    cullVisitor->pushModelViewMatrix(modelview);
    

    // traverse the scene graph to generate the rendergraph.
    _sceneData->accept(*cullVisitor);

    if (_globalState.valid()) cullVisitor->popStateSet();
    
    cullVisitor->popModelViewMatrix();
    cullVisitor->popProjectionMatrix();
    cullVisitor->popViewport();


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

//             // if required clamp the near plane to prevent negative or near zero
//             // near planes.
//             if(!cullVisitor->getDetailedCulling())
//             {
                float min_near_plane = _far_plane*0.0005f;
                if (_near_plane<min_near_plane) _near_plane=min_near_plane;
//             }
        }
        else
        {
            _near_plane = 1.0f;
            _far_plane = 1000.0f;
        }

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
                int seperation = _displaySettings->getSplitStereoHorizontalSeperation();

                int left_half_width = (_viewport->width()-seperation)/2;
                int right_half_begin = (_viewport->width()+seperation)/2;
                int right_half_width = _viewport->width()-right_half_begin;

                osg::ref_ptr<osg::Viewport> viewportLeft = osgNew osg::Viewport;
                viewportLeft->setViewport(_viewport->x(),_viewport->y(),left_half_width,_viewport->height());

                osg::ref_ptr<osg::Viewport> viewportRight = osgNew osg::Viewport;
                viewportRight->setViewport(_viewport->x()+right_half_begin,_viewport->y(),right_half_width,_viewport->height());


                clearArea(_viewport->x()+left_half_width,_viewport->y(),seperation,_viewport->height(),_renderStageLeft->getClearColor());

                if (_displaySettings->getSplitStereoHorizontalEyeMapping()==osg::DisplaySettings::LEFT_EYE_LEFT_VIEWPORT)
                {
                    _renderStageLeft->setViewport(viewportLeft.get());
                    drawStage(_renderStageLeft.get());

                    _renderStageRight->setViewport(viewportRight.get());
                    drawStage(_renderStageRight.get());
                }
                else
                {
                    _renderStageLeft->setViewport(viewportRight.get());
                    drawStage(_renderStageLeft.get());

                    _renderStageRight->setViewport(viewportLeft.get());
                    drawStage(_renderStageRight.get());
                }

            }
            break;
        case(osg::DisplaySettings::VERTICAL_SPLIT):
            {

                int seperation = _displaySettings->getSplitStereoVerticalSeperation();

                int bottom_half_height = (_viewport->height()-seperation)/2;
                int top_half_begin = (_viewport->height()+seperation)/2;
                int top_half_height = _viewport->height()-top_half_begin;

                osg::ref_ptr<osg::Viewport> viewportTop = osgNew osg::Viewport;
                viewportTop->setViewport(_viewport->x(),_viewport->y()+top_half_begin,_viewport->width(),top_half_height);

                osg::ref_ptr<osg::Viewport> viewportBottom = osgNew osg::Viewport;
                viewportBottom->setViewport(_viewport->x(),_viewport->y(),_viewport->width(),bottom_half_height);

                clearArea(_viewport->x(),_viewport->y()+bottom_half_height,_viewport->width(),seperation,_renderStageLeft->getClearColor());

                if (_displaySettings->getSplitStereoVerticalEyeMapping()==osg::DisplaySettings::LEFT_EYE_TOP_VIEWPORT)
                {
                    _renderStageLeft->setViewport(viewportTop.get());
                    drawStage(_renderStageLeft.get());

                    _renderStageRight->setViewport(viewportBottom.get());
                    drawStage(_renderStageRight.get());
                }
                else
                {
                    _renderStageLeft->setViewport(viewportBottom.get());
                    drawStage(_renderStageLeft.get());

                    _renderStageRight->setViewport(viewportTop.get());
                    drawStage(_renderStageRight.get());
                }
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
    osg::Matrix inverseMVPW;
    inverseMVPW.invert(computeMVPW());
    
    object = window*inverseMVPW;
    
    return true;
}


/** Calculate, via glUnProject, the object coordinates of a window x,y
    when projected onto the near and far planes.
    Note, current implementation requires that SceneView::draw() has been previously called
    for projectWindowIntoObject to produce valid values.  As per OpenGL
    windows coordinates are calculated relative to the bottom left of the window.*/
bool SceneView::projectWindowXYIntoObject(int x,int y,osg::Vec3& near_point,osg::Vec3& far_point) const
{
    osg::Matrix inverseMVPW;
    inverseMVPW.invert(computeMVPW());
    
    near_point = osg::Vec3(x,y,0.0f)*inverseMVPW;
    far_point = osg::Vec3(x,y,1.0f)*inverseMVPW;
        
    return true;
}


/** Calculate, via glProject, the object coordinates of a window.
    Note, current implementation requires that SceneView::draw() has been previously called
    for projectWindowIntoObject to produce valid values.  As per OpenGL
    windows coordinates are calculated relative to the bottom left of the window.*/
bool SceneView::projectObjectIntoWindow(const osg::Vec3& object,osg::Vec3& window) const
{
    window = object*computeMVPW();
    return true;
}

const osg::Matrix SceneView::computeMVPW() const
{
    osg::Matrix matrix;
    
    if (_modelviewMatrix.valid())
        matrix = (*_modelviewMatrix);
    else if (_camera.valid())
        matrix = _camera->getModelViewMatrix();
        
    if (_projectionMatrix.valid())
        matrix.postMult(*_projectionMatrix);
    else if (_camera.valid())
        matrix.postMult(_camera->getProjectionMatrix());
        
    if (_viewport.valid())
        matrix.postMult(_viewport->computeWindowMatrix());
    else
        osg::notify(osg::WARN)<<"osg::Matrix SceneView::computeMVPW() - error no viewport attached to SceneView, coords will be computed inccorectly."<<std::endl;

    return matrix;
}

void SceneView::clearArea(int x,int y,int width,int height,const osg::Vec4& color)
{
    osg::ref_ptr<osg::Viewport> viewport = osgNew osg::Viewport;
    viewport->setViewport(x,y,width,height);

    viewport->apply(*_state);
    
    glScissor( x, y, width, height );
    glEnable( GL_SCISSOR_TEST );
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    glClearColor( color[0], color[1], color[2], color[3]);
    glClear( GL_COLOR_BUFFER_BIT);
    glDisable( GL_SCISSOR_TEST );
}
