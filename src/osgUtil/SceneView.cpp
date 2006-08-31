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
#include <osgUtil/SceneView>
#include <osgUtil/UpdateVisitor>
#include <osgUtil/GLObjectsVisitor>

#include <osg/Timer>
#include <osg/Notify>
#include <osg/Texture>
#include <osg/VertexProgram>
#include <osg/FragmentProgram>
#include <osg/AlphaFunc>
#include <osg/TexEnv>
#include <osg/ColorMatrix>
#include <osg/LightModel>
#include <osg/CollectOccludersVisitor>
#include <osg/Shader>

#include <osg/GLU>

using namespace osg;
using namespace osgUtil;

static const GLubyte patternVertEven[] = {
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};

static const GLubyte patternVertOdd[] = {
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};

static const GLubyte patternHorzEven[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};

SceneView::SceneView(DisplaySettings* ds)
{
    _displaySettings = ds;



    _fusionDistanceMode = PROPORTIONAL_TO_SCREEN_DISTANCE;
    _fusionDistanceValue = 1.0f;

    _lightingMode=NO_SCENEVIEW_LIGHT;
    
    _prioritizeTextures = false;
    
    _camera = new CameraNode;
    _camera->setViewport(new Viewport);
    _camera->setClearColor(osg::Vec4(0.2f, 0.2f, 0.4f, 1.0f));
    
    _initCalled = false;

    
    _drawBufferValue = GL_BACK;

    _requiresFlush = true;
    
    _activeUniforms = DEFAULT_UNIFORMS;
    
    _previousFrameTime = 0;
    
    _redrawInterlacedStereoStencilMask = true;
    _interlacedStereoStencilWidth = 0;
    _interlacedStereoStencilHeight = 0;
}


SceneView::~SceneView()
{
}


void SceneView::setDefaults(unsigned int options)
{
    _camera->getProjectionMatrix().makePerspective(50.0f,1.4f,1.0f,10000.0f);
    _camera->getViewMatrix().makeIdentity();

    _globalStateSet = new osg::StateSet;

    if ((options & HEADLIGHT) || (options & SKY_LIGHT))
    {
        _lightingMode=(options&HEADLIGHT) ? HEADLIGHT : SKY_LIGHT;
        _light = new osg::Light;
        _light->setLightNum(0);
        _light->setAmbient(Vec4(0.00f,0.0f,0.00f,1.0f));
        _light->setDiffuse(Vec4(0.8f,0.8f,0.8f,1.0f));
        _light->setSpecular(Vec4(1.0f,1.0f,1.0f,1.0f));

        _globalStateSet->setAssociatedModes(_light.get(),osg::StateAttribute::ON);

        osg::LightModel* lightmodel = new osg::LightModel;
        lightmodel->setAmbientIntensity(osg::Vec4(0.1f,0.1f,0.1f,1.0f));
        _globalStateSet->setAttributeAndModes(lightmodel, osg::StateAttribute::ON);

        // enable lighting by default.
        _globalStateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON);

    }
    else
    {
        _lightingMode = NO_SCENEVIEW_LIGHT;
    }
 
    _state = new State;
    
    _rendergraph = new StateGraph;
    _renderStage = new RenderStage;


    if (options & COMPILE_GLOBJECTS_AT_INIT)
    {
        GLObjectsVisitor::Mode  dlvMode = GLObjectsVisitor::COMPILE_DISPLAY_LISTS |
                                          GLObjectsVisitor::COMPILE_STATE_ATTRIBUTES | 
                                          GLObjectsVisitor::CHECK_BLACK_LISTED_MODES;

    #ifdef __sgi
        dlvMode = GLObjectsVisitor::COMPILE_STATE_ATTRIBUTES;
    #endif

        // sgi's IR graphics has a problem with lighting and display lists, as it seems to store 
        // lighting state with the display list, and the display list visitor doesn't currently apply
        // state before creating display lists. So will disable the init visitor default, this won't
        // affect functionality since the display lists will be created as and when needed.
        GLObjectsVisitor* dlv = new GLObjectsVisitor(dlvMode);
        dlv->setNodeMaskOverride(0xffffffff);
        _initVisitor = dlv;

    }
    
    _updateVisitor = new UpdateVisitor;

    _cullVisitor = new CullVisitor;

    _cullVisitor->setStateGraph(_rendergraph.get());
    _cullVisitor->setRenderStage(_renderStage.get());

    _globalStateSet->setGlobalDefaults();
    
    
    // enable depth testing by default.
    _globalStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

#if 0
    // set up an alphafunc by default to speed up blending operations.
    osg::AlphaFunc* alphafunc = new osg::AlphaFunc;
    alphafunc->setFunction(osg::AlphaFunc::GREATER,1.0f);
    _globalStateSet->setAttributeAndModes(alphafunc, osg::StateAttribute::OFF);
#endif

    // set up an texture environment by default to speed up blending operations.
     osg::TexEnv* texenv = new osg::TexEnv;
     texenv->setMode(osg::TexEnv::MODULATE);
     _globalStateSet->setTextureAttributeAndModes(0,texenv, osg::StateAttribute::ON);

    _camera->setClearColor(osg::Vec4(0.2f, 0.2f, 0.4f, 1.0f));
}

void SceneView::setCamera(osg::CameraNode* camera)
{
    if (camera)
    {
        _camera = camera;
    }
    else
    {
        osg::notify(osg::NOTICE)<<"Warning: attempt to assign a NULL camera to SceneView not permitted."<<std::endl;
    }
}

void SceneView::setSceneData(osg::Node* node)
{
    // take a temporary reference to node to prevent the possibility
    // of it getting deleted when when we do the camera clear of children. 
    osg::ref_ptr<osg::Node> temporaryRefernce = node;
    
    // remove pre existing children
    _camera->removeChildren(0, _camera->getNumChildren());
    
    // add the new one in.
    _camera->addChild(node);
}

void SceneView::init()
{

    _initCalled = true;

    if (_camera.valid() && _initVisitor.valid())
    {
        _initVisitor->reset();
        _initVisitor->setFrameStamp(_frameStamp.get());
        
        GLObjectsVisitor* dlv = dynamic_cast<GLObjectsVisitor*>(_initVisitor.get());
        if (dlv) dlv->setState(_state.get());
        
        if (_frameStamp.valid())
        {
             _initVisitor->setTraversalNumber(_frameStamp->getFrameNumber());
        }
        
        _camera->accept(*_initVisitor.get());
        
    } 
}

void SceneView::update()
{
    if (_camera.valid() && _updateVisitor.valid())
    { 
        _updateVisitor->reset();

        _updateVisitor->setFrameStamp(_frameStamp.get());

        // use the frame number for the traversal number.
        if (_frameStamp.valid())
        {
             _updateVisitor->setTraversalNumber(_frameStamp->getFrameNumber());
        }
        
        _camera->accept(*_updateVisitor.get());
        
        // now force a recompute of the bounding volume while we are still in
        // the read/write app phase, this should prevent the need to recompute
        // the bounding volumes from within the cull traversal which may be
        // multi-threaded.
        _camera->getBound();
    }
}

void SceneView::updateUniforms()
{
    if (!_localStateSet)
    {
        _localStateSet = new osg::StateSet;
    }

    if (!_localStateSet) return;
    
    if ((_activeUniforms & FRAME_NUMBER_UNIFORM) && _frameStamp.valid())
    {
        osg::Uniform* uniform = _localStateSet->getOrCreateUniform("osg_FrameNumber",osg::Uniform::INT);
        uniform->set(_frameStamp->getFrameNumber());        
    }
    
    if ((_activeUniforms & FRAME_TIME_UNIFORM) && _frameStamp.valid())
    {
        osg::Uniform* uniform = _localStateSet->getOrCreateUniform("osg_FrameTime",osg::Uniform::FLOAT);
        uniform->set(static_cast<float>(_frameStamp->getReferenceTime()));
    }
    
    if ((_activeUniforms & DELTA_FRAME_TIME_UNIFORM) && _frameStamp.valid())
    {
        float delta_frame_time = (_previousFrameTime != 0.0) ? static_cast<float>(_frameStamp->getReferenceTime()-_previousFrameTime) : 0.0f;
        _previousFrameTime = _frameStamp->getReferenceTime();
        
        osg::Uniform* uniform = _localStateSet->getOrCreateUniform("osg_DeltaFrameTime",osg::Uniform::FLOAT);
        uniform->set(delta_frame_time);
    }
    
    if (_activeUniforms & VIEW_MATRIX_UNIFORM)
    {
        osg::Uniform* uniform = _localStateSet->getOrCreateUniform("osg_ViewMatrix",osg::Uniform::FLOAT_MAT4);
        uniform->set(getViewMatrix());
    }

    if (_activeUniforms & VIEW_MATRIX_INVERSE_UNIFORM)
    {
        osg::Uniform* uniform = _localStateSet->getOrCreateUniform("osg_ViewMatrixInverse",osg::Uniform::FLOAT_MAT4);
        uniform->set(osg::Matrix::inverse(getViewMatrix()));
    }

}

osg::Matrixd SceneView::computeLeftEyeProjectionImplementation(const osg::Matrixd& projection) const
{
    double iod = _displaySettings->getEyeSeparation();
    double sd = _displaySettings->getScreenDistance();
    double scale_x = 1.0;
    double scale_y = 1.0;

    if (_displaySettings->getSplitStereoAutoAjustAspectRatio())
    {
        switch(_displaySettings->getStereoMode())
        {
            case(osg::DisplaySettings::HORIZONTAL_SPLIT):
                scale_x = 2.0;
                break;
            case(osg::DisplaySettings::VERTICAL_SPLIT):
                scale_y = 2.0;
                break;
            default:
                break;
        }
    }

    if (_displaySettings->getDisplayType()==osg::DisplaySettings::HEAD_MOUNTED_DISPLAY)
    {
        // head mounted display has the same projection matrix for left and right eyes.
        return osg::Matrixd::scale(scale_x,scale_y,1.0) *
               projection;
    }
    else
    {
        // all other display types assume working like a projected power wall
        // need to shjear projection matrix to account for asymetric frustum due to eye offset.
        return osg::Matrixd(1.0,0.0,0.0,0.0,
                           0.0,1.0,0.0,0.0,
                           iod/(2.0*sd),0.0,1.0,0.0,
                           0.0,0.0,0.0,1.0) *
               osg::Matrixd::scale(scale_x,scale_y,1.0) *
               projection;
    }
}

osg::Matrixd SceneView::computeLeftEyeViewImplementation(const osg::Matrixd& view) const
{
    double fusionDistance = _displaySettings->getScreenDistance();
    switch(_fusionDistanceMode)
    {
        case(USE_FUSION_DISTANCE_VALUE):
            fusionDistance = _fusionDistanceValue;
            break;
        case(PROPORTIONAL_TO_SCREEN_DISTANCE):
            fusionDistance *= _fusionDistanceValue;
            break;
    }

    double iod = _displaySettings->getEyeSeparation();
    double sd = _displaySettings->getScreenDistance();
    double es = 0.5f*iod*(fusionDistance/sd);

    return view *
           osg::Matrixd(1.0,0.0,0.0,0.0,
                       0.0,1.0,0.0,0.0,
                       0.0,0.0,1.0,0.0,
                       es,0.0,0.0,1.0);
}

osg::Matrixd SceneView::computeRightEyeProjectionImplementation(const osg::Matrixd& projection) const
{
    double iod = _displaySettings->getEyeSeparation();
    double sd = _displaySettings->getScreenDistance();
    double scale_x = 1.0;
    double scale_y = 1.0;

    if (_displaySettings->getSplitStereoAutoAjustAspectRatio())
    {
        switch(_displaySettings->getStereoMode())
        {
            case(osg::DisplaySettings::HORIZONTAL_SPLIT):
                scale_x = 2.0;
                break;
            case(osg::DisplaySettings::VERTICAL_SPLIT):
                scale_y = 2.0;
                break;
            default:
                break;
        }
    }

    if (_displaySettings->getDisplayType()==osg::DisplaySettings::HEAD_MOUNTED_DISPLAY)
    {
        // head mounted display has the same projection matrix for left and right eyes.
        return osg::Matrixd::scale(scale_x,scale_y,1.0) *
               projection;
    }
    else
    {
        // all other display types assume working like a projected power wall
        // need to shjear projection matrix to account for asymetric frustum due to eye offset.
        return osg::Matrixd(1.0,0.0,0.0,0.0,
                           0.0,1.0,0.0,0.0,
                           -iod/(2.0*sd),0.0,1.0,0.0,
                           0.0,0.0,0.0,1.0) *
               osg::Matrixd::scale(scale_x,scale_y,1.0) *
               projection;
    }
}

osg::Matrixd SceneView::computeRightEyeViewImplementation(const osg::Matrixd& view) const
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

    double iod = _displaySettings->getEyeSeparation();
    double sd = _displaySettings->getScreenDistance();
    double es = 0.5*iod*(fusionDistance/sd);

    return view *
           osg::Matrixd(1.0,0.0,0.0,0.0,
                       0.0,1.0,0.0,0.0,
                       0.0,0.0,1.0,0.0,
                       -es,0.0,0.0,1.0);
}

void SceneView::cull()
{
    // update the active uniforms
    updateUniforms();

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
    //_state->reset();
   
    _state->setFrameStamp(_frameStamp.get());
    _state->setDisplaySettings(_displaySettings.get());


    if (!_cullVisitor)
    {
        osg::notify(osg::INFO) << "Warning: no valid osgUtil::SceneView:: attached, creating a default CullVisitor automatically."<< std::endl;
        _cullVisitor = new CullVisitor;
    }
    if (!_rendergraph)
    {
        osg::notify(osg::INFO) << "Warning: no valid osgUtil::SceneView:: attached, creating a global default StateGraph automatically."<< std::endl;
        _rendergraph = new StateGraph;
    }
    if (!_renderStage)
    {
        osg::notify(osg::INFO) << "Warning: no valid osgUtil::SceneView::_renderStage attached, creating a default RenderStage automatically."<< std::endl;
        _renderStage = new RenderStage;
    }

    bool computeNearFar = (_cullVisitor->getComputeNearFarMode()!=osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR) && getSceneData()!=0;

    if (_displaySettings.valid() && _displaySettings->getStereo()) 
    {

        if (_displaySettings->getStereoMode()==osg::DisplaySettings::LEFT_EYE)
        {
            // set up the left eye.
            _cullVisitor->setTraversalMask(_cullMaskLeft);
            cullStage(computeLeftEyeProjection(getProjectionMatrix()),computeLeftEyeView(getViewMatrix()),_cullVisitor.get(),_rendergraph.get(),_renderStage.get());

            if (computeNearFar)
            {
                CullVisitor::value_type zNear = _cullVisitor->getCalculatedNearPlane();
                CullVisitor::value_type zFar = _cullVisitor->getCalculatedFarPlane();
                _cullVisitor->clampProjectionMatrix(getProjectionMatrix(),zNear,zFar);
            }

        }
        else if (_displaySettings->getStereoMode()==osg::DisplaySettings::RIGHT_EYE)
        {
            // set up the right eye.
            _cullVisitor->setTraversalMask(_cullMaskRight);
            cullStage(computeRightEyeProjection(getProjectionMatrix()),computeRightEyeView(getViewMatrix()),_cullVisitor.get(),_rendergraph.get(),_renderStage.get());

            if (computeNearFar)
            {
                CullVisitor::value_type zNear = _cullVisitor->getCalculatedNearPlane();
                CullVisitor::value_type zFar = _cullVisitor->getCalculatedFarPlane();
                _cullVisitor->clampProjectionMatrix(getProjectionMatrix(),zNear,zFar);
            }

        }
        else
        {

            if (!_cullVisitorLeft.valid()) _cullVisitorLeft = dynamic_cast<CullVisitor*>(_cullVisitor->cloneType());
            if (!_rendergraphLeft.valid()) _rendergraphLeft = dynamic_cast<StateGraph*>(_rendergraph->cloneType());
            if (!_renderStageLeft.valid()) _renderStageLeft = dynamic_cast<RenderStage*>(_renderStage->clone(osg::CopyOp::DEEP_COPY_ALL));

            if (!_cullVisitorRight.valid()) _cullVisitorRight = dynamic_cast<CullVisitor*>(_cullVisitor->cloneType());
            if (!_rendergraphRight.valid()) _rendergraphRight = dynamic_cast<StateGraph*>(_rendergraph->cloneType());
            if (!_renderStageRight.valid()) _renderStageRight = dynamic_cast<RenderStage*>(_renderStage->clone(osg::CopyOp::DEEP_COPY_ALL));
            
            _cullVisitorLeft->setDatabaseRequestHandler(_cullVisitor->getDatabaseRequestHandler());
            _cullVisitorLeft->setClampProjectionMatrixCallback(_cullVisitor->getClampProjectionMatrixCallback());
            _cullVisitorLeft->setTraversalMask(_cullMaskLeft);
            cullStage(computeLeftEyeProjection(getProjectionMatrix()),computeLeftEyeView(getViewMatrix()),_cullVisitorLeft.get(),_rendergraphLeft.get(),_renderStageLeft.get());


            // set up the right eye.
            _cullVisitorRight->setDatabaseRequestHandler(_cullVisitor->getDatabaseRequestHandler());
            _cullVisitorRight->setClampProjectionMatrixCallback(_cullVisitor->getClampProjectionMatrixCallback());
            _cullVisitorRight->setTraversalMask(_cullMaskRight);
            cullStage(computeRightEyeProjection(getProjectionMatrix()),computeRightEyeView(getViewMatrix()),_cullVisitorRight.get(),_rendergraphRight.get(),_renderStageRight.get());
           
            if (computeNearFar)
            {
                CullVisitor::value_type zNear = osg::minimum(_cullVisitorLeft->getCalculatedNearPlane(),_cullVisitorRight->getCalculatedNearPlane());
                CullVisitor::value_type zFar =  osg::maximum(_cullVisitorLeft->getCalculatedFarPlane(),_cullVisitorRight->getCalculatedFarPlane());
                _cullVisitor->clampProjectionMatrix(getProjectionMatrix(),zNear,zFar);
            }

        }

    }
    else
    {

        _cullVisitor->setTraversalMask(_cullMask);
        cullStage(getProjectionMatrix(),getViewMatrix(),_cullVisitor.get(),_rendergraph.get(),_renderStage.get());

        if (computeNearFar)
        {
            CullVisitor::value_type zNear = _cullVisitor->getCalculatedNearPlane();
            CullVisitor::value_type zFar = _cullVisitor->getCalculatedFarPlane();
            _cullVisitor->clampProjectionMatrix(getProjectionMatrix(),zNear,zFar);
        }
    }
    
    
}

void SceneView::cullStage(const osg::Matrixd& projection,const osg::Matrixd& modelview,osgUtil::CullVisitor* cullVisitor, osgUtil::StateGraph* rendergraph, osgUtil::RenderStage* renderStage)
{

    if (!_camera || !getViewport()) return;

    if (!_initCalled) init();


    osg::ref_ptr<RefMatrix> proj = new osg::RefMatrix(projection);
    osg::ref_ptr<RefMatrix> mv = new osg::RefMatrix(modelview);


    // collect any occluder in the view frustum.
    if (_camera->containsOccluderNodes())
    {
        //std::cout << "Scene graph contains occluder nodes, searching for them"<<std::endl;
        
        
        if (!_collectOccludersVisistor) _collectOccludersVisistor = new osg::CollectOccludersVisitor;
        
        _collectOccludersVisistor->inheritCullSettings(*this);
        
        _collectOccludersVisistor->reset();
        
        _collectOccludersVisistor->setFrameStamp(_frameStamp.get());

        // use the frame number for the traversal number.
        if (_frameStamp.valid())
        {
             _collectOccludersVisistor->setTraversalNumber(_frameStamp->getFrameNumber());
        }

        _collectOccludersVisistor->pushViewport(getViewport());
        _collectOccludersVisistor->pushProjectionMatrix(proj.get());
        _collectOccludersVisistor->pushModelViewMatrix(mv.get());

        // traverse the scene graph to search for occluder in there new positions.
        for(unsigned int i=0; i< _camera->getNumChildren(); ++i)
        {
            _camera->getChild(i)->accept(*_collectOccludersVisistor);
        }

        _collectOccludersVisistor->popModelViewMatrix();
        _collectOccludersVisistor->popProjectionMatrix();
        _collectOccludersVisistor->popViewport();
        
        // sort the occluder from largest occluder volume to smallest.
        _collectOccludersVisistor->removeOccludedOccluders();
        
        
        osg::notify(osg::DEBUG_INFO) << "finished searching for occluder - found "<<_collectOccludersVisistor->getCollectedOccluderSet().size()<<std::endl;
           
        cullVisitor->getOccluderList().clear();
        std::copy(_collectOccludersVisistor->getCollectedOccluderSet().begin(),_collectOccludersVisistor->getCollectedOccluderSet().end(), std::back_insert_iterator<CullStack::OccluderList>(cullVisitor->getOccluderList()));
    }
    


    cullVisitor->reset();

    cullVisitor->setFrameStamp(_frameStamp.get());

    // use the frame number for the traversal number.
    if (_frameStamp.valid())
    {
         cullVisitor->setTraversalNumber(_frameStamp->getFrameNumber());
    }

    cullVisitor->inheritCullSettings(*this);

    cullVisitor->setClearNode(NULL); // reset earth sky on each frame.
    
    cullVisitor->setStateGraph(rendergraph);
    cullVisitor->setRenderStage(renderStage);

    cullVisitor->setState( _state.get() );

    renderStage->reset();

    // comment out reset of rendergraph since clean is more efficient.
    //  rendergraph->reset();

    // use clean of the rendergraph rather than reset, as it is able to
    // reuse the structure on the rendergraph in the next frame. This
    // achieves a certain amount of frame cohereancy of memory allocation.
    rendergraph->clean();

    renderStage->setViewport(getViewport());
    renderStage->setClearColor(getClearColor());


    switch(_lightingMode)
    {
    case(HEADLIGHT):
        if (_light.valid()) renderStage->addPositionedAttribute(NULL,_light.get());
        else osg::notify(osg::WARN)<<"Warning: no osg::Light attached to ogUtil::SceneView to provide head light.*/"<<std::endl;
        break;
    case(SKY_LIGHT):
        if (_light.valid()) renderStage->addPositionedAttribute(mv.get(),_light.get());
        else osg::notify(osg::WARN)<<"Warning: no osg::Light attached to ogUtil::SceneView to provide sky light.*/"<<std::endl;
        break;
    default:
        break;
    }            

    if (_globalStateSet.valid()) cullVisitor->pushStateSet(_globalStateSet.get());
    if (_localStateSet.valid()) cullVisitor->pushStateSet(_localStateSet.get());


    cullVisitor->pushViewport(getViewport());
    cullVisitor->pushProjectionMatrix(proj.get());
    cullVisitor->pushModelViewMatrix(mv.get());
    

    // traverse the scene graph to generate the rendergraph.
    for(unsigned int childNo=0;
        childNo<_camera->getNumChildren();
        ++childNo)
    {
        _camera->getChild(childNo)->accept(*cullVisitor);
    }

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
            renderStage->setClearMask(clearNode->getClearMask());
        }
        else
        {
            // we have an earth sky implementation to do the work for use
            // so we don't need to clear.
            renderStage->setClearMask(0);
        }
    }

    renderStage->sort();

    // prune out any empty StateGraph children.
    // note, this would be not required if the rendergraph had been
    // reset at the start of each frame (see top of this method) but
    // a clean has been used instead to try to minimize the amount of
    // allocation and deleteing of the StateGraph nodes.
    rendergraph->prune();
}

void SceneView::releaseAllGLObjects()
{
    if (!_camera) return;
   
    _camera->releaseGLObjects(_state.get());
    
    // we need to reset State as it keeps handles to Program objects.
    if (_state.valid()) _state->reset();
}


void SceneView::flushAllDeletedGLObjects()
{
    _requiresFlush = false;
    
    double availableTime = 100.0f;
    double currentTime = _state->getFrameStamp()?_state->getFrameStamp()->getReferenceTime():0.0;
    
    osg::FrameBufferObject::flushDeletedFrameBufferObjects(_state->getContextID(),currentTime,availableTime);
    osg::RenderBuffer::flushDeletedRenderBuffers(_state->getContextID(),currentTime,availableTime);
    osg::Texture::flushAllDeletedTextureObjects(_state->getContextID());
    osg::Drawable::flushAllDeletedDisplayLists(_state->getContextID());
    osg::Drawable::flushDeletedVertexBufferObjects(_state->getContextID(),currentTime,availableTime);
    osg::VertexProgram::flushDeletedVertexProgramObjects(_state->getContextID(),currentTime,availableTime);
    osg::FragmentProgram::flushDeletedFragmentProgramObjects(_state->getContextID(),currentTime,availableTime);
    osg::Program::flushDeletedGlPrograms(_state->getContextID(),currentTime,availableTime);
    osg::Shader::flushDeletedGlShaders(_state->getContextID(),currentTime,availableTime);
 }

void SceneView::flushDeletedGLObjects(double& availableTime)
{
    _requiresFlush = false;

    double currentTime = _state->getFrameStamp()?_state->getFrameStamp()->getReferenceTime():0.0;

    osg::FrameBufferObject::flushDeletedFrameBufferObjects(_state->getContextID(),currentTime,availableTime);
    osg::RenderBuffer::flushDeletedRenderBuffers(_state->getContextID(),currentTime,availableTime);
    osg::Texture::flushDeletedTextureObjects(_state->getContextID(),currentTime,availableTime);
    osg::Drawable::flushDeletedDisplayLists(_state->getContextID(),availableTime);
    osg::Drawable::flushDeletedVertexBufferObjects(_state->getContextID(),currentTime,availableTime);
    osg::VertexProgram::flushDeletedVertexProgramObjects(_state->getContextID(),currentTime,availableTime);
    osg::FragmentProgram::flushDeletedFragmentProgramObjects(_state->getContextID(),currentTime,availableTime);
    osg::Program::flushDeletedGlPrograms(_state->getContextID(),currentTime,availableTime);
    osg::Shader::flushDeletedGlShaders(_state->getContextID(),currentTime,availableTime);
}

void SceneView::draw()
{

    // note, to support multi-pipe systems the deletion of OpenGL display list
    // and texture objects is deferred until the OpenGL context is the correct
    // context for when the object were originally created.  Here we know what
    // context we are in so can flush the appropriate caches.
    
    if (_requiresFlush)
    {
        double availableTime = 0.005;
        flushDeletedGLObjects(availableTime);
    }

    // assume the the draw which is about to happen could generate GL objects that need flushing in the next frame.
    _requiresFlush = true;

    _state->setInitialViewMatrix(new osg::RefMatrix(getViewMatrix()));

    RenderLeaf* previous = NULL;
    if (_displaySettings.valid() && _displaySettings->getStereo()) 
    {
    
        switch(_displaySettings->getStereoMode())
        {
        case(osg::DisplaySettings::QUAD_BUFFER):
            {

                _localStateSet->setAttribute(getViewport());

                // ensure that all color planes are active.
                osg::ColorMask* cmask = static_cast<osg::ColorMask*>(_localStateSet->getAttribute(osg::StateAttribute::COLORMASK));
                if (cmask)
                {
                    cmask->setMask(true,true,true,true);
                }
                else
                {
                    cmask = new osg::ColorMask(true,true,true,true);
                    _localStateSet->setAttribute(cmask);
                }
                _renderStageLeft->setColorMask(cmask);
                _renderStageRight->setColorMask(cmask);

                _renderStageLeft->setDrawBuffer(GL_BACK_LEFT);
                _renderStageLeft->setReadBuffer(GL_BACK_LEFT);
                _renderStageRight->setDrawBuffer(GL_BACK_RIGHT);
                _renderStageRight->setReadBuffer(GL_BACK_RIGHT);

                _renderStageLeft->drawPreRenderStages(*_state,previous);
                _renderStageRight->drawPreRenderStages(*_state,previous);

                _renderStageLeft->draw(*_state,previous);

                _renderStageRight->draw(*_state,previous);

            }
            break;
        case(osg::DisplaySettings::ANAGLYPHIC):
            {
                if( _drawBufferValue !=  GL_NONE)
                {
                    _renderStageLeft->setDrawBuffer(_drawBufferValue);
                    _renderStageLeft->setReadBuffer(_drawBufferValue);

                    _renderStageRight->setDrawBuffer(_drawBufferValue);
                    _renderStageRight->setReadBuffer(_drawBufferValue);
                }
                
                _localStateSet->setAttribute(getViewport());

                
                _renderStageLeft->drawPreRenderStages(*_state,previous);
                _renderStageRight->drawPreRenderStages(*_state,previous);


                // ensure that left eye color planes are active.
                osg::ColorMask* leftColorMask = _renderStageLeft->getColorMask();
                if (!leftColorMask)
                {
                    leftColorMask = new osg::ColorMask();
                    _renderStageRight->setColorMask(leftColorMask);
                }
                
                // red
                leftColorMask->setMask(true,false,false,true);

                // orange
                // leftColorMask->setMask(true,true,false,true);

                _localStateSet->setAttribute(leftColorMask);

                // draw left eye.
                _renderStageLeft->draw(*_state,previous);
                
                

                // ensure that right eye color planes are active.
                osg::ColorMask* rightColorMask = _renderStageLeft->getColorMask();
                if (!rightColorMask)
                {
                    rightColorMask = new osg::ColorMask();
                    _renderStageRight->setColorMask(rightColorMask);
                }

                // cyan
                rightColorMask->setMask(false,true,true,true);
                
                // blue
                // rightColorMask->setMask(false,false,true,true);

                _localStateSet->setAttribute(rightColorMask);
                _renderStageRight->setColorMask(rightColorMask);

                // draw right eye.
                _renderStageRight->draw(*_state,previous);

            }
            break;
        case(osg::DisplaySettings::HORIZONTAL_SPLIT):
            {
                if( _drawBufferValue !=  GL_NONE)
                {
                    _renderStageLeft->setDrawBuffer(_drawBufferValue);
                    _renderStageLeft->setReadBuffer(_drawBufferValue);

                    _renderStageRight->setDrawBuffer(_drawBufferValue);
                    _renderStageRight->setReadBuffer(_drawBufferValue);
                }

                // ensure that all color planes are active.
                osg::ColorMask* cmask = static_cast<osg::ColorMask*>(_localStateSet->getAttribute(osg::StateAttribute::COLORMASK));
                if (cmask)
                {
                    cmask->setMask(true,true,true,true);
                }
                else
                {
                    cmask = new osg::ColorMask(true,true,true,true);
                    _localStateSet->setAttribute(cmask);
                }
                _renderStageLeft->setColorMask(cmask);
                _renderStageRight->setColorMask(cmask);

                _renderStageLeft->drawPreRenderStages(*_state,previous);
                _renderStageRight->drawPreRenderStages(*_state,previous);

                int separation = _displaySettings->getSplitStereoHorizontalSeparation();

                int left_half_width = (getViewport()->width()-separation)/2;
                int right_half_begin = (getViewport()->width()+separation)/2;
                int right_half_width = getViewport()->width()-right_half_begin;

                osg::ref_ptr<osg::Viewport> viewportLeft = new osg::Viewport;
                viewportLeft->setViewport(getViewport()->x(),getViewport()->y(),left_half_width,getViewport()->height());

                osg::ref_ptr<osg::Viewport> viewportRight = new osg::Viewport;
                viewportRight->setViewport(getViewport()->x()+right_half_begin,getViewport()->y(),right_half_width,getViewport()->height());


                clearArea(getViewport()->x()+left_half_width,getViewport()->y(),separation,getViewport()->height(),_renderStageLeft->getClearColor());

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
                if( _drawBufferValue !=  GL_NONE)
                {
                    _renderStageLeft->setDrawBuffer(_drawBufferValue);
                    _renderStageLeft->setReadBuffer(_drawBufferValue);

                    _renderStageRight->setDrawBuffer(_drawBufferValue);
                    _renderStageRight->setReadBuffer(_drawBufferValue);
                }

                // ensure that all color planes are active.
                osg::ColorMask* cmask = static_cast<osg::ColorMask*>(_localStateSet->getAttribute(osg::StateAttribute::COLORMASK));
                if (cmask)
                {
                    cmask->setMask(true,true,true,true);
                }
                else
                {
                    cmask = new osg::ColorMask(true,true,true,true);
                    _localStateSet->setAttribute(cmask);
                }

                _renderStageLeft->setColorMask(cmask);
                _renderStageRight->setColorMask(cmask);
                _renderStageLeft->drawPreRenderStages(*_state,previous);
                _renderStageRight->drawPreRenderStages(*_state,previous);

                int separation = _displaySettings->getSplitStereoVerticalSeparation();

                int bottom_half_height = (getViewport()->height()-separation)/2;
                int top_half_begin = (getViewport()->height()+separation)/2;
                int top_half_height = getViewport()->height()-top_half_begin;

                osg::ref_ptr<osg::Viewport> viewportTop = new osg::Viewport;
                viewportTop->setViewport(getViewport()->x(),getViewport()->y()+top_half_begin,getViewport()->width(),top_half_height);

                osg::ref_ptr<osg::Viewport> viewportBottom = new osg::Viewport;
                viewportBottom->setViewport(getViewport()->x(),getViewport()->y(),getViewport()->width(),bottom_half_height);

                clearArea(getViewport()->x(),getViewport()->y()+bottom_half_height,getViewport()->width(),separation,_renderStageLeft->getClearColor());

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
                if( _drawBufferValue !=  GL_NONE)
                {
                    _renderStage->setDrawBuffer(_drawBufferValue);
                    _renderStage->setReadBuffer(_drawBufferValue);
                }

                // ensure that all color planes are active.
                osg::ColorMask* cmask = static_cast<osg::ColorMask*>(_localStateSet->getAttribute(osg::StateAttribute::COLORMASK));
                if (cmask)
                {
                    cmask->setMask(true,true,true,true);
                }
                else
                {
                    cmask = new osg::ColorMask(true,true,true,true);
                    _localStateSet->setAttribute(cmask);
                }
                _renderStage->setColorMask(cmask);
                _renderStage->setColorMask(cmask);

                _localStateSet->setAttribute(getViewport());
                _renderStage->drawPreRenderStages(*_state,previous);
                _renderStage->draw(*_state,previous);
            }
            break;
        case(osg::DisplaySettings::VERTICAL_INTERLACE):
            {
                _localStateSet->setAttribute(getViewport());

                // ensure that all color planes are active.
                osg::ColorMask* cmask = static_cast<osg::ColorMask*>(_localStateSet->getAttribute(osg::StateAttribute::COLORMASK));
                if (cmask)
                {
                    cmask->setMask(true,true,true,true);
                }
                else
                {
                    cmask = new osg::ColorMask(true,true,true,true);
                    _localStateSet->setAttribute(cmask);
                }
                _renderStageLeft->setColorMask(cmask);
                _renderStageRight->setColorMask(cmask);

                _renderStageLeft->drawPreRenderStages(*_state,previous);
                _renderStageRight->drawPreRenderStages(*_state,previous);

                glEnable(GL_STENCIL_TEST);

                if(_redrawInterlacedStereoStencilMask ||
                   _interlacedStereoStencilWidth != getViewport()->width() ||
                  _interlacedStereoStencilHeight != getViewport()->height() )
                {
                    getViewport()->apply(*_state);
                    glMatrixMode(GL_PROJECTION);
                    glLoadIdentity();
                    glOrtho(getViewport()->x(), getViewport()->width(), getViewport()->y(), getViewport()->height(), -1.0, 1.0);
                    glMatrixMode(GL_MODELVIEW);
                    glLoadIdentity();    
                    glDisable(GL_LIGHTING);
                    glDisable(GL_DEPTH_TEST);
                    glStencilMask(~0u);
                    glClear(GL_STENCIL_BUFFER_BIT);
                    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
                    glStencilFunc(GL_ALWAYS, 1, ~0u);
                    glPolygonStipple(patternVertEven);
                    glEnable(GL_POLYGON_STIPPLE);
                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                    glRecti(getViewport()->x(), getViewport()->y(), getViewport()->width(), getViewport()->height());
                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                    glDisable(GL_POLYGON_STIPPLE);
                    glEnable(GL_LIGHTING);
                    glEnable(GL_DEPTH_TEST);
                    
                    _redrawInterlacedStereoStencilMask = false;
                    _interlacedStereoStencilWidth = getViewport()->width();
                    _interlacedStereoStencilHeight = getViewport()->height();
                }

                _renderStageLeft->setClearMask(_renderStageLeft->getClearMask() & ~(GL_STENCIL_BUFFER_BIT));
                _renderStageRight->setClearMask(_renderStageRight->getClearMask() & ~(GL_STENCIL_BUFFER_BIT|GL_COLOR_BUFFER_BIT));

                glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
                glStencilFunc(GL_EQUAL, 0, ~0u);    
                _renderStageLeft->draw(*_state,previous);
                
                glStencilFunc(GL_NOTEQUAL, 0, ~0u);
                _renderStageRight->draw(*_state,previous);
                glDisable(GL_STENCIL_TEST);
            }
            break;
        case(osg::DisplaySettings::HORIZONTAL_INTERLACE):
            {
                _localStateSet->setAttribute(getViewport());

                // ensure that all color planes are active.
                osg::ColorMask* cmask = static_cast<osg::ColorMask*>(_localStateSet->getAttribute(osg::StateAttribute::COLORMASK));
                if (cmask)
                {
                    cmask->setMask(true,true,true,true);
                }
                else
                {
                    cmask = new osg::ColorMask(true,true,true,true);
                    _localStateSet->setAttribute(cmask);
                }
                _renderStageLeft->setColorMask(cmask);
                _renderStageRight->setColorMask(cmask);

                _renderStageLeft->drawPreRenderStages(*_state,previous);
                _renderStageRight->drawPreRenderStages(*_state,previous);

                glEnable(GL_STENCIL_TEST);

                if(_redrawInterlacedStereoStencilMask ||
                   _interlacedStereoStencilWidth != getViewport()->width() ||
                  _interlacedStereoStencilHeight != getViewport()->height() )
                {
                    getViewport()->apply(*_state);
                    glMatrixMode(GL_PROJECTION);
                    glLoadIdentity();
                    glOrtho(getViewport()->x(), getViewport()->width(), getViewport()->y(), getViewport()->height(), -1.0, 1.0);
                    glMatrixMode(GL_MODELVIEW);
                    glLoadIdentity();
                    glDisable(GL_LIGHTING);
                    glDisable(GL_DEPTH_TEST);
                    glStencilMask(~0u);
                    glClear(GL_STENCIL_BUFFER_BIT);
                    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
                    glStencilFunc(GL_ALWAYS, 1, ~0u);
                    glPolygonStipple(patternHorzEven);
                    glEnable(GL_POLYGON_STIPPLE);
                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                    glRecti(getViewport()->x(), getViewport()->y(), getViewport()->width(), getViewport()->height());
                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                    glDisable(GL_POLYGON_STIPPLE);
                    glEnable(GL_LIGHTING);
                    glEnable(GL_DEPTH_TEST);
                    
                    _redrawInterlacedStereoStencilMask = false;
                    _interlacedStereoStencilWidth = getViewport()->width();
                    _interlacedStereoStencilHeight = getViewport()->height();
                }

                _renderStageLeft->setClearMask(_renderStageLeft->getClearMask() & ~(GL_STENCIL_BUFFER_BIT));
                _renderStageRight->setClearMask(_renderStageRight->getClearMask() & ~(GL_STENCIL_BUFFER_BIT|GL_COLOR_BUFFER_BIT));

                glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
                glStencilFunc(GL_EQUAL, 0, ~0u);    
                _renderStageLeft->draw(*_state,previous);
                
                glStencilFunc(GL_NOTEQUAL, 0, ~0u);
                _renderStageRight->draw(*_state,previous);
                glDisable(GL_STENCIL_TEST);
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
        if( _drawBufferValue !=  GL_NONE)
        {
            _renderStage->setDrawBuffer(_drawBufferValue);
            _renderStage->setReadBuffer(_drawBufferValue);
        }

        _localStateSet->setAttribute(getViewport());


        // ensure that all color planes are active.
        osg::ColorMask* cmask = static_cast<osg::ColorMask*>(_localStateSet->getAttribute(osg::StateAttribute::COLORMASK));
        if (cmask)
        {
            cmask->setMask(true,true,true,true);
        }
        else
        {
            cmask = new osg::ColorMask(true,true,true,true);
            _localStateSet->setAttribute(cmask);
        }
        _renderStage->setColorMask(cmask);

        // bog standard draw.
        _renderStage->drawPreRenderStages(*_state,previous);
        _renderStage->draw(*_state,previous);
    }
    
    // re apply the defalt OGL state.
    _state->popAllStateSets();
    _state->apply();
    
    if (_camera->getPostDrawCallback())
    {
        (*(_camera->getPostDrawCallback()))(*_camera);
    }

    if (_state->getCheckForGLErrors()!=osg::State::NEVER_CHECK_GL_ERRORS)
    {
        GLenum errorNo = glGetError();
        if (errorNo!=GL_NO_ERROR)
        {
            osg::notify(WARN)<<"Warning: detected OpenGL error '"<<gluErrorString(errorNo)<<"'"<< std::endl;

            // go into debug mode of OGL error in a fine grained way to help
            // track down OpenGL errors.
            _state->setCheckForGLErrors(osg::State::ONCE_PER_ATTRIBUTE);
        }
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
    osg::Matrix matrix( getViewMatrix() * getProjectionMatrix());
        
    if (getViewport())
        matrix.postMult(getViewport()->computeWindowMatrix());
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

void SceneView::setProjectionMatrixAsOrtho(double left, double right,
                                           double bottom, double top,
                                           double zNear, double zFar)
{
    setProjectionMatrix(osg::Matrixd::ortho(left, right,
                                           bottom, top,
                                           zNear, zFar));
}                                           

void SceneView::setProjectionMatrixAsOrtho2D(double left, double right,
                                             double bottom, double top)
{
    setProjectionMatrix(osg::Matrixd::ortho2D(left, right,
                                             bottom, top));
}

void SceneView::setProjectionMatrixAsFrustum(double left, double right,
                                             double bottom, double top,
                                             double zNear, double zFar)
{
    setProjectionMatrix(osg::Matrixd::frustum(left, right,
                                             bottom, top,
                                             zNear, zFar));
}

void SceneView::setProjectionMatrixAsPerspective(double fovy,double aspectRatio,
                                                 double zNear, double zFar)
{
    setProjectionMatrix(osg::Matrixd::perspective(fovy,aspectRatio,
                                                 zNear, zFar));
}                                      

bool SceneView::getProjectionMatrixAsOrtho(double& left, double& right,
                                           double& bottom, double& top,
                                           double& zNear, double& zFar) const
{
    return getProjectionMatrix().getOrtho(left, right,
                                       bottom, top,
                                       zNear, zFar);
}

bool SceneView::getProjectionMatrixAsFrustum(double& left, double& right,
                                             double& bottom, double& top,
                                             double& zNear, double& zFar) const
{
    return getProjectionMatrix().getFrustum(left, right,
                                         bottom, top,
                                         zNear, zFar);
}                                  

bool SceneView::getProjectionMatrixAsPerspective(double& fovy,double& aspectRatio,
                                                 double& zNear, double& zFar) const
{
    return getProjectionMatrix().getPerspective(fovy, aspectRatio, zNear, zFar);
}                                                 

void SceneView::setViewMatrixAsLookAt(const Vec3& eye,const Vec3& center,const Vec3& up)
{
    setViewMatrix(osg::Matrixd::lookAt(eye,center,up));
}

void SceneView::getViewMatrixAsLookAt(Vec3& eye,Vec3& center,Vec3& up,float lookDistance) const
{
    getViewMatrix().getLookAt(eye,center,up,lookDistance);
}

bool SceneView::getStats(Statistics& stats)
{
    if (_displaySettings.valid() && _displaySettings->getStereo()) 
    {
        switch(_displaySettings->getStereoMode())
        {
        case(osg::DisplaySettings::QUAD_BUFFER):
        case(osg::DisplaySettings::ANAGLYPHIC):
        case(osg::DisplaySettings::HORIZONTAL_SPLIT):
        case(osg::DisplaySettings::VERTICAL_SPLIT):
        case(osg::DisplaySettings::VERTICAL_INTERLACE):
        case(osg::DisplaySettings::HORIZONTAL_INTERLACE):
        {
            bool resultLeft = _renderStageLeft->getStats(stats);
            bool resultRight = _renderStageRight->getStats(stats);
            return resultLeft && resultRight;
        }
        case(osg::DisplaySettings::RIGHT_EYE):
        case(osg::DisplaySettings::LEFT_EYE):
        default:
            return _renderStage->getStats(stats);
        }
    }
    else
    {
        return _renderStage->getStats(stats);
    }
}
