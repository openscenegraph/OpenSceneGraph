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
#include <osgUtil/SceneView>
#include <osgUtil/UpdateVisitor>
#include <osgUtil/DisplayListVisitor>

#include <osg/Notify>
#include <osg/Texture>
#include <osg/VertexProgram>
#include <osg/AlphaFunc>
#include <osg/TexEnv>
#include <osg/ColorMatrix>
#include <osg/LightModel>
#include <osg/CollectOccludersVisitor>

#include <osg/GLU>

using namespace osg;
using namespace osgUtil;

SceneView::SceneView(DisplaySettings* ds)
{
    _displaySettings = ds;

    _backgroundColor.set(0.2f, 0.2f, 0.4f, 1.0f);

    _computeNearFar = CullVisitor::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES;

    _cullingMode = osg::CullStack::ENABLE_ALL_CULLING;
    _LODScale = 1.0f;
    _smallFeatureCullingPixelSize = 3.0f;

    _fusionDistanceMode = PROPORTIONAL_TO_SCREEN_DISTANCE;
    _fusionDistanceValue = 1.0f;

    _lightingMode=HEADLIGHT;
    
    _prioritizeTextures = false;
    
    _viewport = new Viewport;
    
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
    if (!_projectionMatrix) 
    {
        _projectionMatrix = new RefMatrix();
        _projectionMatrix->makePerspective(50.0f,1.4f,1.0f,10000.0f);
    }
    if (!_modelviewMatrix)
    {
        _modelviewMatrix = new RefMatrix();
    }

    _globalStateSet = new osg::StateSet;

    _lightingMode=HEADLIGHT;
    _light = new osg::Light;
    _light->setLightNum(0);
    _light->setAmbient(Vec4(0.00f,0.0f,0.00f,1.0f));
    _light->setDiffuse(Vec4(0.8f,0.8f,0.8f,1.0f));
    _light->setSpecular(Vec4(1.0f,1.0f,1.0f,1.0f));

    _state = new State;
    
    _rendergraph = new RenderGraph;
    _renderStage = new RenderStage;


    DisplayListVisitor::Mode  dlvMode = DisplayListVisitor::COMPILE_DISPLAY_LISTS|DisplayListVisitor::COMPILE_STATE_ATTRIBUTES;

#ifdef __sgi
    dlvMode = DisplayListVisitor::COMPILE_STATE_ATTRIBUTES;
#endif

    // sgi's IR graphics has a problem with lighting and display lists, as it seems to store 
    // lighting state with the display list, and the display list visitor doesn't currently apply
    // state before creating display lists. So will disable the init visitor default, this won't
    // affect functionality since the display lists will be created as and when needed.
    DisplayListVisitor* dlv = new DisplayListVisitor(dlvMode);
    dlv->setState(_state.get());
    dlv->setNodeMaskOverride(0xffffffff);
    _initVisitor = dlv;

    _updateVisitor = new UpdateVisitor;

    _cullVisitor = new CullVisitor;

    _cullVisitor->setRenderGraph(_rendergraph.get());
    _cullVisitor->setRenderStage(_renderStage.get());

    _globalStateSet->setGlobalDefaults();
    
    // enable lighting by default.
    _globalStateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    _globalStateSet->setAssociatedModes(_light.get(),osg::StateAttribute::ON);
    
    // enable depth testing by default.
    _globalStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

    // set up an alphafunc by default to speed up blending operations.
    osg::AlphaFunc* alphafunc = new osg::AlphaFunc;
    alphafunc->setFunction(osg::AlphaFunc::GREATER,0.0f);
    _globalStateSet->setAttributeAndModes(alphafunc, osg::StateAttribute::ON);

    // set up an texture environment by default to speed up blending operations.
     osg::TexEnv* texenv = new osg::TexEnv;
     texenv->setMode(osg::TexEnv::MODULATE);
     _globalStateSet->setTextureAttributeAndModes(0,texenv, osg::StateAttribute::ON);

    osg::LightModel* lightmodel = new osg::LightModel;
    lightmodel->setAmbientIntensity(osg::Vec4(0.1f,0.1f,0.1f,1.0f));
    _globalStateSet->setAttributeAndModes(lightmodel, osg::StateAttribute::ON);

    _backgroundColor.set(0.2f, 0.2f, 0.4f, 1.0f);

    _cullMask = 0xffffffff;
    _cullMaskLeft = 0xffffffff;
    _cullMaskRight = 0xffffffff;
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

void SceneView::update()
{
    if (!_initCalled) init();


    if (_sceneData.valid() && _updateVisitor.valid())
    { 
        _updateVisitor->reset();

        _updateVisitor->setFrameStamp(_frameStamp.get());

        // use the frame number for the traversal number.
        if (_frameStamp.valid())
        {
             _updateVisitor->setTraversalNumber(_frameStamp->getFrameNumber());
        }
        
        _sceneData->accept(*_updateVisitor.get());
        
        // now force a recompute of the bounding volume while we are still in
        // the read/write app phase, this should prevent the need to recompute
        // the bounding volumes from within the cull traversal which may be
        // multi-threaded.
        _sceneData->getBound();
    }
    
    
}

void SceneView::cull()
{

    if (!_state)
    {
        osg::notify(osg::INFO) << "Warning: no valid osgUtil::SceneView::_state attached, creating a default state automatically."<< std::endl;

        // note the constructor for osg::State will set ContextID to 0 which will be fine to single context graphics
        // applications which is ok for most apps, but not multiple context/pipe applications.
        _state = new osg::State;
    }

    if (!_localStateSet)
    {
        _localStateSet = new osg::StateSet;
    }
    
    // we in theory should be able to be able to bypass reset, but we'll call it just incase.
    _state->reset();
   
    _state->setFrameStamp(_frameStamp.get());
    _state->setDisplaySettings(_displaySettings.get());


    osg::ref_ptr<osg::RefMatrix> projection = _projectionMatrix.get();
    osg::ref_ptr<osg::RefMatrix> modelview = _modelviewMatrix.get();
       
    if (!projection) projection = new osg::RefMatrix();
    if (!modelview)  modelview  = new osg::RefMatrix();

    if (!_cullVisitor)
    {
        osg::notify(osg::INFO) << "Warning: no valid osgUtil::SceneView:: attached, creating a default CullVisitor automatically."<< std::endl;
        _cullVisitor = new CullVisitor;
    }
    if (!_rendergraph)
    {
        osg::notify(osg::INFO) << "Warning: no valid osgUtil::SceneView:: attached, creating a global default RenderGraph automatically."<< std::endl;
        _rendergraph = new RenderGraph;
    }
    if (!_renderStage)
    {
        osg::notify(osg::INFO) << "Warning: no valid osgUtil::SceneView::_renderStage attached, creating a default RenderStage automatically."<< std::endl;
        _renderStage = new RenderStage;
    }

    if (_displaySettings.valid() && _displaySettings->getStereo()) 
    {

        float fusionDistance = _displaySettings->getScreenDistance();
        switch(_fusionDistanceMode)
        {
            case(USE_FUSION_DISTANCE_VALUE):
                fusionDistance = _fusionDistanceValue;
                break;
            case(PROPORTIONAL_TO_SCREEN_DISTANCE):
                fusionDistance *= _fusionDistanceValue;
                break;
        }

        float iod = _displaySettings->getEyeSeparation();
        float sd = _displaySettings->getScreenDistance();
        float es = 0.5f*iod*(fusionDistance/sd);

        if (_displaySettings->getStereoMode()==osg::DisplaySettings::LEFT_EYE)
        {
            // set up the left eye.
            osg::ref_ptr<osg::RefMatrix> projectionLeft = new osg::RefMatrix(osg::Matrix(1.0f,0.0f,0.0f,0.0f,
                                                                                         0.0f,1.0f,0.0f,0.0f,
                                                                                         iod/(2.0f*sd),0.0f,1.0f,0.0f,
                                                                                         0.0f,0.0f,0.0f,1.0f)*
                                                                             (*projection));


            osg::ref_ptr<osg::RefMatrix> modelviewLeft = new osg::RefMatrix( (*modelview) *
                                                                             osg::Matrix(1.0f,0.0f,0.0f,0.0f,
                                                                                         0.0f,1.0f,0.0f,0.0f,
                                                                                         0.0f,0.0f,1.0f,0.0f,
                                                                                         es,0.0f,0.0f,1.0f));

            _cullVisitor->setTraversalMask(_cullMaskLeft);
            cullStage(projectionLeft.get(),modelviewLeft.get(),_cullVisitor.get(),_rendergraph.get(),_renderStage.get());

        }
        else if (_displaySettings->getStereoMode()==osg::DisplaySettings::RIGHT_EYE)
        {
            // set up the right eye.
            osg::ref_ptr<osg::RefMatrix> projectionRight = new osg::RefMatrix(osg::Matrix(1.0f,0.0f,0.0f,0.0f,
                                                                                          0.0f,1.0f,0.0f,0.0f,
                                                                                          -iod/(2.0f*sd),0.0f,1.0f,0.0f,
                                                                                          0.0f,0.0f,0.0f,1.0f)*
                                                                              (*projection));

            osg::ref_ptr<osg::RefMatrix> modelviewRight = new osg::RefMatrix( (*modelview) *
                                                                              osg::Matrix(1.0f,0.0f,0.0f,0.0f,
                                                                                          0.0f,1.0f,0.0f,0.0f,
                                                                                          0.0f,0.0f,1.0f,0.0f,
                                                                                          -es,0.0f,0.0f,1.0f));

            _cullVisitor->setTraversalMask(_cullMaskRight);
            cullStage(projectionRight.get(),modelviewRight.get(),_cullVisitor.get(),_rendergraph.get(),_renderStage.get());


        }
        else
        {

            if (!_cullVisitorLeft.valid()) _cullVisitorLeft = dynamic_cast<CullVisitor*>(_cullVisitor->cloneType());
            if (!_rendergraphLeft.valid()) _rendergraphLeft = dynamic_cast<RenderGraph*>(_rendergraph->cloneType());
            if (!_renderStageLeft.valid()) _renderStageLeft = dynamic_cast<RenderStage*>(_renderStage->clone(osg::CopyOp::DEEP_COPY_ALL));

            if (!_cullVisitorRight.valid()) _cullVisitorRight = dynamic_cast<CullVisitor*>(_cullVisitor->cloneType());
            if (!_rendergraphRight.valid()) _rendergraphRight = dynamic_cast<RenderGraph*>(_rendergraph->cloneType());
            if (!_renderStageRight.valid()) _renderStageRight = dynamic_cast<RenderStage*>(_renderStage->clone(osg::CopyOp::DEEP_COPY_ALL));




            osg::Matrix projection_scale;
            
            
            if (_displaySettings->getSplitStereoAutoAjustAspectRatio())
            {

                switch(_displaySettings->getStereoMode())
                {
                    case(osg::DisplaySettings::HORIZONTAL_SPLIT):
                        projection_scale.makeScale(2.0f,1.0f,1.0f);
                        break;
                    case(osg::DisplaySettings::VERTICAL_SPLIT):
                        projection_scale.makeScale(1.0f,2.0f,1.0f);
                        break;
                    default:
                        break;
                }
            }
            
            // set up the left eye.
            osg::ref_ptr<osg::RefMatrix> projectionLeft = new osg::RefMatrix(osg::Matrix(1.0f,0.0f,0.0f,0.0f,
                                                                                         0.0f,1.0f,0.0f,0.0f,
                                                                                         iod/(2.0f*sd),0.0f,1.0f,0.0f,
                                                                                         0.0f,0.0f,0.0f,1.0f)*
                                                                             projection_scale*
                                                                             (*projection));



            osg::ref_ptr<osg::RefMatrix> modelviewLeft = new osg::RefMatrix( (*modelview) *
                                                             osg::Matrix(1.0f,0.0f,0.0f,0.0f,
                                                                         0.0f,1.0f,0.0f,0.0f,
                                                                         0.0f,0.0f,1.0f,0.0f,
                                                                         es,0.0f,0.0f,1.0f));

            _cullVisitorLeft->setTraversalMask(_cullMaskLeft);
            cullStage(projectionLeft.get(),modelviewLeft.get(),_cullVisitorLeft.get(),_rendergraphLeft.get(),_renderStageLeft.get());


            // set up the right eye.
            osg::ref_ptr<osg::RefMatrix> projectionRight = new osg::RefMatrix(osg::Matrix(1.0f,0.0f,0.0f,0.0f,
                                                                                          0.0f,1.0f,0.0f,0.0f,
                                                                                          -iod/(2.0f*sd),0.0f,1.0f,0.0f,
                                                                                          0.0f,0.0f,0.0f,1.0f)*
                                                                              projection_scale*
                                                                              (*projection));

            osg::ref_ptr<osg::RefMatrix> modelviewRight = new osg::RefMatrix( (*modelview) *
                                                                              osg::Matrix(1.0f,0.0f,0.0f,0.0f,
                                                                                         0.0f,1.0f,0.0f,0.0f,
                                                                                         0.0f,0.0f,1.0f,0.0f,
                                                                                         -es,0.0f,0.0f,1.0f));



            _cullVisitorRight->setTraversalMask(_cullMaskRight);
            cullStage(projectionRight.get(),modelviewRight.get(),_cullVisitorRight.get(),_rendergraphRight.get(),_renderStageRight.get());
           
        }

    }
    else
    {

        _cullVisitor->setTraversalMask(_cullMask);
        cullStage(projection.get(),modelview.get(),_cullVisitor.get(),_rendergraph.get(),_renderStage.get());

    }
    
    
}

void SceneView::cullStage(osg::RefMatrix* projection,osg::RefMatrix* modelview,osgUtil::CullVisitor* cullVisitor, osgUtil::RenderGraph* rendergraph, osgUtil::RenderStage* renderStage)
{

    if (!_sceneData || !_viewport->valid()) return;

    if (!_initCalled) init();



    // collect any occluder in the view frustum.
    if (_sceneData->containsOccluderNodes())
    {
        //std::cout << "Scene graph contains occluder nodes, searching for them"<<std::endl;
        
        osg::CollectOccludersVisitor cov;
        
        cov.setFrameStamp(_frameStamp.get());

        // use the frame number for the traversal number.
        if (_frameStamp.valid())
        {
             cov.setTraversalNumber(_frameStamp->getFrameNumber());
        }

        cov.pushViewport(_viewport.get());
        cov.pushProjectionMatrix(projection);
        cov.pushModelViewMatrix(modelview);

        // traverse the scene graph to search for occluder in there new positions.
        _sceneData->accept(cov);

        cov.popModelViewMatrix();
        cov.popProjectionMatrix();
        cov.popViewport();
        
        // sort the occluder from largest occluder volume to smallest.
        cov.removeOccludedOccluders();
        
        
        //std::cout << "finished searching for occluder - found "<<cov.getCollectedOccluderSet().size()<<std::endl;
           
        cullVisitor->getOccluderList().clear();
        std::copy(cov.getCollectedOccluderSet().begin(),cov.getCollectedOccluderSet().end(), std::back_insert_iterator<CullStack::OccluderList>(cullVisitor->getOccluderList()));
    }
    


    cullVisitor->reset();

    cullVisitor->setFrameStamp(_frameStamp.get());

    // use the frame number for the traversal number.
    if (_frameStamp.valid())
    {
         cullVisitor->setTraversalNumber(_frameStamp->getFrameNumber());
    }

    cullVisitor->setCullingMode(_cullingMode);
    cullVisitor->setComputeNearFarMode(_computeNearFar);
    cullVisitor->setLODScale(_LODScale);
    cullVisitor->setSmallFeatureCullingPixelSize(_smallFeatureCullingPixelSize);

    cullVisitor->setClearNode(NULL); // reset earth sky on each frame.
    
    cullVisitor->setRenderGraph(rendergraph);
    cullVisitor->setRenderStage(renderStage);

    cullVisitor->setState( _state.get() );

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
        if (_light.valid()) renderStage->addPositionedAttribute(NULL,_light.get());
        else osg::notify(osg::WARN)<<"Warning: no osg::Light attached to ogUtil::SceneView to provide head light.*/"<<std::endl;
        break;
    case(SKY_LIGHT):
        if (_light.valid()) renderStage->addPositionedAttribute(modelview,_light.get());
        else osg::notify(osg::WARN)<<"Warning: no osg::Light attached to ogUtil::SceneView to provide sky light.*/"<<std::endl;
        break;
    case(NO_SCENEVIEW_LIGHT):
        break;
    }            

    if (_globalStateSet.valid()) cullVisitor->pushStateSet(_globalStateSet.get());
    if (_localStateSet.valid()) cullVisitor->pushStateSet(_localStateSet.get());


    cullVisitor->pushViewport(_viewport.get());
    cullVisitor->pushProjectionMatrix(projection);
    cullVisitor->pushModelViewMatrix(modelview);
    

    // traverse the scene graph to generate the rendergraph.
    _sceneData->accept(*cullVisitor);

    cullVisitor->popModelViewMatrix();
    cullVisitor->popProjectionMatrix();
    cullVisitor->popViewport();

    if (_localStateSet.valid()) cullVisitor->popStateSet();
    if (_globalStateSet.valid()) cullVisitor->popStateSet();
    

    const osg::ClearNode* clearNode = cullVisitor->getClearNode();
    if (clearNode)
    {
        if (clearNode->getRequiresClear())
        {
            renderStage->setClearColor(clearNode->getClearColor());
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

    renderStage->sort();

    // prune out any empty RenderGraph children.
    // note, this would be not required if the rendergraph had been
    // reset at the start of each frame (see top of this method) but
    // a clean has been used instead to try to minimize the amount of
    // allocation and deleteing of the RenderGraph nodes.
    rendergraph->prune();
}



void SceneView::draw()
{

    // note, to support multi-pipe systems the deletion of OpenGL display list
    // and texture objects is deferred until the OpenGL context is the correct
    // context for when the object were originally created.  Here we know what
    // context we are in so can flush the appropriate caches.
    osg::Drawable::flushDeletedDisplayLists(_state->getContextID());
    osg::Drawable::flushDeletedVertexBufferObjects(_state->getContextID());
    osg::Texture::flushDeletedTextureObjects(_state->getContextID());
    osg::VertexProgram::flushDeletedVertexProgramObjects(_state->getContextID());

    RenderLeaf* previous = NULL;
    if (_displaySettings.valid() && _displaySettings->getStereo()) 
    {
    
        switch(_displaySettings->getStereoMode())
        {
        case(osg::DisplaySettings::QUAD_BUFFER):
            {

                _localStateSet->setAttribute(_viewport.get());

                _renderStageLeft->drawPreRenderStages(*_state,previous);
                _renderStageRight->drawPreRenderStages(*_state,previous);

                glDrawBuffer(GL_BACK_LEFT);
                _renderStageLeft->draw(*_state,previous);

                glDrawBuffer(GL_BACK_RIGHT);
                _renderStageRight->draw(*_state,previous);

            }
            break;
        case(osg::DisplaySettings::ANAGLYPHIC):
            {
                
                _localStateSet->setAttribute(_viewport.get());

                _renderStageLeft->drawPreRenderStages(*_state,previous);
                _renderStageRight->drawPreRenderStages(*_state,previous);

                // draw left eye.
                osg::ref_ptr<osg::ColorMask> red = new osg::ColorMask;
                red->setMask(true,false,false,true);
                _localStateSet->setAttribute(red.get());
                _renderStageLeft->setColorMask(red.get());
                _renderStageLeft->draw(*_state,previous);

                // draw right eye.
                osg::ref_ptr<osg::ColorMask> green = new osg::ColorMask;
                green->setMask(false,true,true,true);
                _localStateSet->setAttribute(green.get());
                _renderStageRight->setColorMask(green.get());
                _renderStageRight->draw(*_state,previous);

            }
            break;
        case(osg::DisplaySettings::HORIZONTAL_SPLIT):
            {
                _renderStageLeft->drawPreRenderStages(*_state,previous);
                _renderStageRight->drawPreRenderStages(*_state,previous);

                int separation = _displaySettings->getSplitStereoHorizontalSeparation();

                int left_half_width = (_viewport->width()-separation)/2;
                int right_half_begin = (_viewport->width()+separation)/2;
                int right_half_width = _viewport->width()-right_half_begin;

                osg::ref_ptr<osg::Viewport> viewportLeft = new osg::Viewport;
                viewportLeft->setViewport(_viewport->x(),_viewport->y(),left_half_width,_viewport->height());

                osg::ref_ptr<osg::Viewport> viewportRight = new osg::Viewport;
                viewportRight->setViewport(_viewport->x()+right_half_begin,_viewport->y(),right_half_width,_viewport->height());


                clearArea(_viewport->x()+left_half_width,_viewport->y(),separation,_viewport->height(),_renderStageLeft->getClearColor());

                if (_displaySettings->getSplitStereoHorizontalEyeMapping()==osg::DisplaySettings::LEFT_EYE_LEFT_VIEWPORT)
                {
                    _localStateSet->setAttribute(viewportLeft.get());
                    _renderStageLeft->setViewport(viewportLeft.get());
                    _renderStageLeft->draw(*_state,previous);

                    _localStateSet->setAttribute(viewportRight.get());
                    _renderStageRight->setViewport(viewportRight.get());
                    _renderStageRight->draw(*_state,previous);
                }
                else
                {
                    _localStateSet->setAttribute(viewportRight.get());
                    _renderStageLeft->setViewport(viewportRight.get());
                    _renderStageLeft->draw(*_state,previous);

                    _localStateSet->setAttribute(viewportLeft.get());
                    _renderStageRight->setViewport(viewportLeft.get());
                    _renderStageRight->draw(*_state,previous);
                }

            }
            break;
        case(osg::DisplaySettings::VERTICAL_SPLIT):
            {

                _renderStageLeft->drawPreRenderStages(*_state,previous);
                _renderStageRight->drawPreRenderStages(*_state,previous);

                int separation = _displaySettings->getSplitStereoVerticalSeparation();

                int bottom_half_height = (_viewport->height()-separation)/2;
                int top_half_begin = (_viewport->height()+separation)/2;
                int top_half_height = _viewport->height()-top_half_begin;

                osg::ref_ptr<osg::Viewport> viewportTop = new osg::Viewport;
                viewportTop->setViewport(_viewport->x(),_viewport->y()+top_half_begin,_viewport->width(),top_half_height);

                osg::ref_ptr<osg::Viewport> viewportBottom = new osg::Viewport;
                viewportBottom->setViewport(_viewport->x(),_viewport->y(),_viewport->width(),bottom_half_height);

                clearArea(_viewport->x(),_viewport->y()+bottom_half_height,_viewport->width(),separation,_renderStageLeft->getClearColor());

                if (_displaySettings->getSplitStereoVerticalEyeMapping()==osg::DisplaySettings::LEFT_EYE_TOP_VIEWPORT)
                {
                    _localStateSet->setAttribute(viewportTop.get());
                    _renderStageLeft->setViewport(viewportTop.get());
                    _renderStageLeft->draw(*_state,previous);

                    _localStateSet->setAttribute(viewportBottom.get());
                    _renderStageRight->setViewport(viewportBottom.get());
                    _renderStageRight->draw(*_state,previous);
                }
                else
                {
                    _localStateSet->setAttribute(viewportBottom.get());
                    _renderStageLeft->setViewport(viewportBottom.get());
                    _renderStageLeft->draw(*_state,previous);

                    _localStateSet->setAttribute(viewportTop.get());
                    _renderStageRight->setViewport(viewportTop.get());
                    _renderStageRight->draw(*_state,previous);
                }
            }
            break;
        case(osg::DisplaySettings::RIGHT_EYE):
        case(osg::DisplaySettings::LEFT_EYE):
            {
                _localStateSet->setAttribute(_viewport.get());
                _renderStage->drawPreRenderStages(*_state,previous);
                _renderStage->draw(*_state,previous);
            }
            break;
        default:
            {
                osg::notify(osg::NOTICE)<<"Warning: stereo mode not implemented yet."<< std::endl;
            }
            break;
        }
    }
    else
    {

	// Need to restore draw buffer when toggling Stereo off.
	glDrawBuffer(GL_BACK);

        _localStateSet->setAttribute(_viewport.get());
        osg::ref_ptr<osg::ColorMask> cmask = new osg::ColorMask;
        cmask->setMask(true,true,true,true);
        _localStateSet->setAttribute(cmask.get());

        // bog standard draw.
        _renderStage->drawPreRenderStages(*_state,previous);
        _renderStage->draw(*_state,previous);
    }

    GLenum errorNo = glGetError();
    if (errorNo!=GL_NO_ERROR)
    {
        osg::notify(WARN)<<"Warning: detected OpenGL error '"<<gluErrorString(errorNo)<<"'"<< std::endl;
        // go into debug mode of OGL errors.
        _state->setReportGLErrors(true);
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
        
    if (_projectionMatrix.valid())
        matrix.postMult(*_projectionMatrix);
        
    if (_viewport.valid())
        matrix.postMult(_viewport->computeWindowMatrix());
    else
        osg::notify(osg::WARN)<<"osg::Matrix SceneView::computeMVPW() - error no viewport attached to SceneView, coords will be computed inccorectly."<<std::endl;

    return matrix;
}

void SceneView::clearArea(int x,int y,int width,int height,const osg::Vec4& color)
{
    osg::ref_ptr<osg::Viewport> viewport = new osg::Viewport;
    viewport->setViewport(x,y,width,height);

    _state->applyAttribute(viewport.get());
    
    glScissor( x, y, width, height );
    glEnable( GL_SCISSOR_TEST );
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    glClearColor( color[0], color[1], color[2], color[3]);
    glClear( GL_COLOR_BUFFER_BIT);
    glDisable( GL_SCISSOR_TEST );
}
