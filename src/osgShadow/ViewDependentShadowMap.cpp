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

#include <osgShadow/ViewDependentShadowMap>
#include <osgShadow/ShadowedScene>
#include <osg/CullFace>
#include <osg/io_utils>

using namespace osgShadow;

//////////////////////////////////////////////////////////////////
// fragment shader
//
#if 0
static const char fragmentShaderSource_withBaseTexture[] =
        "uniform sampler2D baseTexture;                                          \n"
        "uniform sampler2DShadow shadowTexture;                                  \n"
        "                                                                        \n"
        "void main(void)                                                         \n"
        "{                                                                       \n"
        "  vec4 colorAmbientEmissive = gl_FrontLightModelProduct.sceneColor;     \n"
        "  vec4 color = texture2D( baseTexture, gl_TexCoord[0].xy );                                            \n"
        "  color *= mix( colorAmbientEmissive, gl_Color, shadow2DProj( shadowTexture, gl_TexCoord[1] ).r );     \n"
        "  gl_FragColor = color;                                                                                \n"
        "} \n";
#else
static const char fragmentShaderSource_withBaseTexture[] =
        "uniform sampler2D baseTexture;                                          \n"
        "uniform int baseTextureUnit;                                          \n"
        "uniform sampler2DShadow shadowTexture;                                  \n"
        "uniform int shadowTextureUnit;                                          \n"
        "                                                                        \n"
        "void main(void)                                                         \n"
        "{                                                                       \n"
        "  vec4 colorAmbientEmissive = gl_FrontLightModelProduct.sceneColor;     \n"
        "  vec4 color = texture2D( baseTexture, gl_TexCoord[baseTextureUnit].xy );                                                            \n"
        "  color *= mix( colorAmbientEmissive, gl_Color, shadow2DProj( shadowTexture, gl_TexCoord[shadowTextureUnit] ).r );     \n"
        "  gl_FragColor = color;                                                                                                \n"
        "} \n";
#endif

template<class T>
class RenderLeafTraverser : public T
{
public:

    RenderLeafTraverser()
    {
    }

    void traverse(const osgUtil::RenderStage* rs)
    {
        traverse(static_cast<const osgUtil::RenderBin*>(rs));
    }

    void traverse(const osgUtil::RenderBin* renderBin)
    {
        const osgUtil::RenderBin::RenderBinList& rbl = renderBin->getRenderBinList();
        for(osgUtil::RenderBin::RenderBinList::const_iterator itr = rbl.begin();
            itr != rbl.end();
            ++itr)
        {
            traverse(itr->second.get());
        }

        const osgUtil::RenderBin::RenderLeafList& rll = renderBin->getRenderLeafList();
        for(osgUtil::RenderBin::RenderLeafList::const_iterator itr = rll.begin();
            itr != rll.end();
            ++itr)
        {
            handle(*itr);
        }

        const osgUtil::RenderBin::StateGraphList& rgl = renderBin->getStateGraphList();
        for(osgUtil::RenderBin::StateGraphList::const_iterator itr = rgl.begin();
            itr != rgl.end();
            ++itr)
        {
            traverse(*itr);
        }

    }

    void traverse(const osgUtil::StateGraph* stateGraph)
    {
        const osgUtil::StateGraph::ChildList& cl = stateGraph->_children;
        for(osgUtil::StateGraph::ChildList::const_iterator itr = cl.begin();
            itr != cl.end();
            ++itr)
        {
            traverse(itr->second.get());
        }

        const osgUtil::StateGraph::LeafList& ll = stateGraph->_leaves;
        for(osgUtil::StateGraph::LeafList::const_iterator itr = ll.begin();
            itr != ll.end();
            ++itr)
        {
            handle(itr->get());
        }
    }

    inline void handle(const osgUtil::RenderLeaf* renderLeaf)
    {
        this->operator()(renderLeaf);
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////
//
// VDSMCameraCullCallback
//
class VDSMCameraCullCallback : public osg::NodeCallback
{
    public:

        VDSMCameraCullCallback(ViewDependentShadowMap* vdsm, osg::Polytope& polytope);

        virtual void operator()(osg::Node*, osg::NodeVisitor* nv);

        osg::RefMatrix* getProjectionMatrix() { return _projectionMatrix.get(); }
        osgUtil::RenderStage* getRenderStage() { return _renderStage.get(); }

    protected:

        ViewDependentShadowMap*                 _vdsm;
        osg::ref_ptr<osg::RefMatrix>            _projectionMatrix;
        osg::ref_ptr<osgUtil::RenderStage>      _renderStage;
        osg::Polytope                           _polytope;
};

VDSMCameraCullCallback::VDSMCameraCullCallback(ViewDependentShadowMap* vdsm, osg::Polytope& polytope):
    _vdsm(vdsm),
    _polytope(polytope)
{
}

void VDSMCameraCullCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
    osg::Camera* camera = dynamic_cast<osg::Camera*>(node);
    OSG_INFO<<"VDSMCameraCullCallback::operator()(osg::Node* "<<camera<<", osg::NodeVisitor* "<<cv<<")"<<std::endl;

    if (!_polytope.empty())
    {
        OSG_INFO<<"Pushing custom Polytope"<<std::endl;
        
        osg::CullingSet& cs = cv->getProjectionCullingStack().back();

        cs.setFrustum(_polytope);

        cv->pushCullingSet();
    }
    
    if (_vdsm->getShadowedScene())
    {
        _vdsm->getShadowedScene()->osg::Group::traverse(*nv);
    }

    if (!_polytope.empty())
    {
        OSG_INFO<<"Popping custom Polytope"<<std::endl;
        cv->popCullingSet();
    }

    _renderStage = cv->getRenderStage();

    if (cv->getComputeNearFarMode() != osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR)        
    {
        // make sure that the near plane is computed correctly.
        cv->computeNearPlane();

        osg::Matrixd projection = *(cv->getProjectionMatrix());

        OSG_INFO<<"RTT Projection matrix "<<projection<<std::endl;

        double left, right, bottom, top, zNear, zFar;
        double epsilon = 1e-6;
        if (fabs(projection(0,3))<epsilon  && fabs(projection(1,3))<epsilon  && fabs(projection(2,3))<epsilon )
        {
            projection.getOrtho(left, right,
                                bottom, top,
                                zNear,  zFar);

            OSG_INFO<<"Ortho zNear="<<zNear<<", zFar="<<zFar<<std::endl;
        }
        else
        {
            projection.getFrustum(left, right,
                                bottom, top,
                                zNear,  zFar);

            OSG_INFO<<"Frustum zNear="<<zNear<<", zFar="<<zFar<<std::endl;
        }

        OSG_INFO<<"Calculated zNear = "<<cv->getCalculatedNearPlane()<<", zFar = "<<cv->getCalculatedFarPlane()<<std::endl;

        zNear = osg::maximum(zNear, cv->getCalculatedNearPlane());
        zFar = osg::minimum(zFar, cv->getCalculatedFarPlane());

        cv->setCalculatedNearPlane(zNear);
        cv->setCalculatedFarPlane(zFar);

        cv->clampProjectionMatrix(projection, zNear, zFar);

        //OSG_INFO<<"RTT zNear = "<<zNear<<", zFar = "<<zFar<<std::endl;
        OSG_INFO<<"RTT Projection matrix after clamping "<<projection<<std::endl;

        camera->setProjectionMatrix(projection);

        _projectionMatrix = cv->getProjectionMatrix();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
// LightData
//
ViewDependentShadowMap::LightData::LightData(ViewDependentShadowMap::ViewDependentData* vdd):
    _viewDependentData(vdd),
    directionalLight(false)
{
}

void ViewDependentShadowMap::LightData::setLightData(osg::RefMatrix* lm, const osg::Light* l, const osg::Matrixd& modelViewMatrix)
{
    lightMatrix = lm;
    light = l;

    lightPos = light->getPosition();
    directionalLight = (light->getPosition().w()== 0.0);
    if (directionalLight)
    {
        lightPos3.set(0.0, 0.0, 0.0); // directional light has no destinct position
        lightDir.set(-lightPos.x(), -lightPos.y(), -lightPos.z());
        lightDir.normalize();
        OSG_INFO<<"   Directional light, lightPos="<<lightPos<<", lightDir="<<lightDir<<std::endl;
        if (lightMatrix.valid())
        {
            OSG_INFO<<"   Light matrix "<<*lightMatrix<<std::endl;
            osg::Matrix lightToLocalMatrix(*lightMatrix * osg::Matrix::inverse(modelViewMatrix) );
            lightDir = osg::Matrix::transform3x3( lightDir, lightToLocalMatrix );
            lightDir.normalize();
            OSG_INFO<<"   new LightDir ="<<lightDir<<std::endl;
        }
    }
    else
    {
        OSG_INFO<<"   Positional light, lightPos="<<lightPos<<std::endl;
        lightDir = light->getDirection();
        lightDir.normalize();
        if (lightMatrix.valid())
        {
            OSG_INFO<<"   Light matrix "<<*lightMatrix<<std::endl;
            osg::Matrix lightToLocalMatrix(*lightMatrix * osg::Matrix::inverse(modelViewMatrix) );
            lightPos = lightPos * lightToLocalMatrix;
            lightDir = osg::Matrix::transform3x3( lightDir, lightToLocalMatrix );
            lightDir.normalize();
            OSG_INFO<<"   new LightPos ="<<lightPos<<std::endl;
            OSG_INFO<<"   new LightDir ="<<lightDir<<std::endl;
        }
        lightPos3.set(lightPos.x()/lightPos.w(), lightPos.y()/lightPos.w(), lightPos.z()/lightPos.w());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
// ShadowData
//
ViewDependentShadowMap::ShadowData::ShadowData(ViewDependentShadowMap::ViewDependentData* vdd):
    _viewDependentData(vdd),
    _textureUnit(0)
{
    bool debug = vdd->getViewDependentShadowMap()->getDebugDraw();

    // set up texgen
    _texgen = new osg::TexGen;

    // set up the texture
    _texture = new osg::Texture2D;

    unsigned int textureSize = debug ? 512 : 2048;
    
    _texture->setTextureSize(textureSize, textureSize);
    if (debug)
    {
        _texture->setInternalFormat(GL_RGB);
    }
    else
    {
        _texture->setInternalFormat(GL_DEPTH_COMPONENT);
        _texture->setShadowComparison(true);
        _texture->setShadowTextureMode(osg::Texture2D::LUMINANCE);
    }
    
    _texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    _texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

    // the shadow comparison should fail if object is outside the texture
    _texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
    _texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
    //_texture->setBorderColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    _texture->setBorderColor(osg::Vec4(0.0f,0.0f,0.0f,0.0f));
    
    // set up the camera
    _camera = new osg::Camera;

    _camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF_INHERIT_VIEWPOINT);

    //_camera->setClearColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    _camera->setClearColor(osg::Vec4(0.0f,0.0f,0.0f,0.0f));

    _camera->setComputeNearFarMode(osg::Camera::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES);

    // set viewport
    _camera->setViewport(0,0,textureSize,textureSize);


    if (debug)
    {
        // clear just the depth buffer
        _camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        // render after the main camera
        _camera->setRenderOrder(osg::Camera::POST_RENDER);

        // attach the texture and use it as the color buffer.
        //_camera->attach(osg::Camera::DEPTH_BUFFER, _texture.get());
        _camera->attach(osg::Camera::COLOR_BUFFER, _texture.get());
    }
    else
    {
        // clear the depth and colour bufferson each clear.
        _camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        
        // set the camera to render before the main camera.
        _camera->setRenderOrder(osg::Camera::PRE_RENDER);
        
        // tell the camera to use OpenGL frame buffer object where supported.
        _camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

        // attach the texture and use it as the color buffer.
        _camera->attach(osg::Camera::DEPTH_BUFFER, _texture.get());
        //_camera->attach(osg::Camera::COLOR_BUFFER, _texture.get());
    }   
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
// Frustum
//
ViewDependentShadowMap::Frustum::Frustum(osgUtil::CullVisitor* cv, double minZNear, double maxZFar):
    corners(8),
    faces(6),
    edges(12)
{
    projectionMatrix = *(cv->getProjectionMatrix());
    modelViewMatrix = *(cv->getModelViewMatrix());

    OSG_INFO<<"Projection matrix "<<projectionMatrix<<std::endl;

    if (cv->getComputeNearFarMode()!=osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR)
    {
        double zNear = osg::maximum(static_cast<double>(cv->getCalculatedNearPlane()),minZNear);
        double zFar = osg::minimum(static_cast<double>(cv->getCalculatedFarPlane()),maxZFar);

        cv->clampProjectionMatrix(projectionMatrix, zNear, zFar);

        OSG_INFO<<"zNear = "<<zNear<<", zFar = "<<zFar<<std::endl;
        OSG_INFO<<"Projection matrix after clamping "<<projectionMatrix<<std::endl;
    }
    
    corners[0].set(-1.0,-1.0,-1.0);
    corners[1].set(1.0,-1.0,-1.0);
    corners[2].set(1.0,-1.0,1.0);
    corners[3].set(-1.0,-1.0,1.0);
    corners[4].set(-1.0,1.0,-1.0);
    corners[5].set(1.0,1.0,-1.0);
    corners[6].set(1.0,1.0,1.0);
    corners[7].set(-1.0,1.0,1.0);

    osg::Matrixd clipToWorld;
    clipToWorld.invert(modelViewMatrix * projectionMatrix);

    // transform frustum corners from clipspace to world coords, and compute center
    for(Vertices::iterator itr = corners.begin();
        itr != corners.end();
        ++itr)
    {
        *itr = (*itr) * clipToWorld;

        OSG_INFO<<"   corner "<<*itr<<std::endl;
    }

    // compute eye point
    eye = osg::Vec3d(0.0,0.0,0.0) * osg::Matrix::inverse(modelViewMatrix);

    // compute center and the frustumCenterLine
    centerNearPlane = (corners[0]+corners[1]+corners[5]+corners[4])*0.25;
    centerFarPlane = (corners[3]+corners[2]+corners[6]+corners[7])*0.25;
    center = (centerNearPlane+centerFarPlane)*0.5;
    frustumCenterLine = centerFarPlane-centerNearPlane;
    frustumCenterLine.normalize();

    OSG_INFO<<"   center "<<center<<std::endl;

    faces[0].push_back(0);
    faces[0].push_back(3);
    faces[0].push_back(7);
    faces[0].push_back(4);

    faces[1].push_back(1);
    faces[1].push_back(5);
    faces[1].push_back(6);
    faces[1].push_back(2);

    faces[2].push_back(0);
    faces[2].push_back(1);
    faces[2].push_back(2);
    faces[2].push_back(3);

    faces[3].push_back(4);
    faces[3].push_back(7);
    faces[3].push_back(6);
    faces[3].push_back(5);

    faces[4].push_back(0);
    faces[4].push_back(4);
    faces[4].push_back(5);
    faces[4].push_back(1);

    faces[5].push_back(2);
    faces[5].push_back(6);
    faces[5].push_back(7);
    faces[5].push_back(3);

    edges[0].push_back(0); edges[0].push_back(1); // corner points on edge
    edges[0].push_back(2); edges[0].push_back(4); // faces on edge

    edges[1].push_back(1); edges[1].push_back(2); // corner points on edge
    edges[1].push_back(2); edges[1].push_back(1); // faces on edge

    edges[2].push_back(2); edges[2].push_back(3); // corner points on edge
    edges[2].push_back(2); edges[2].push_back(5); // faces on edge

    edges[3].push_back(3); edges[3].push_back(0); // corner points on edge
    edges[3].push_back(2); edges[3].push_back(0); // faces on edge


    edges[4].push_back(0); edges[4].push_back(4); // corner points on edge
    edges[4].push_back(0); edges[4].push_back(4); // faces on edge

    edges[5].push_back(1); edges[5].push_back(5); // corner points on edge
    edges[5].push_back(4); edges[5].push_back(1); // faces on edge

    edges[6].push_back(2); edges[6].push_back(6); // corner points on edge
    edges[6].push_back(1); edges[6].push_back(5); // faces on edge

    edges[7].push_back(3); edges[7].push_back(7); // corner points on edge
    edges[7].push_back(5); edges[7].push_back(0); // faces on edge


    edges[8].push_back(4); edges[8].push_back(5); // corner points on edge
    edges[8].push_back(3); edges[8].push_back(4); // faces on edge

    edges[9].push_back(5); edges[9].push_back(6); // corner points on edge
    edges[9].push_back(3); edges[9].push_back(1); // faces on edge

    edges[10].push_back(6);edges[10].push_back(7); // corner points on edge
    edges[10].push_back(3);edges[10].push_back(5); // faces on edge

    edges[11].push_back(7); edges[11].push_back(4); // corner points on edge
    edges[11].push_back(3); edges[11].push_back(0); // faces on edge
}


///////////////////////////////////////////////////////////////////////////////////////////////
//
// ViewDependentData
//
ViewDependentShadowMap::ViewDependentData::ViewDependentData(ViewDependentShadowMap* vdsm):
    _viewDependentShadowMap(vdsm)
{
    OSG_NOTICE<<"ViewDependentData::ViewDependentData()"<<this<<std::endl;
    _stateset = new osg::StateSet;
}


///////////////////////////////////////////////////////////////////////////////////////////////
//
// ViewDependentShadowMap
//
ViewDependentShadowMap::ViewDependentShadowMap():
    ShadowTechnique(),
    _baseShadowTextureUnit(1),
    _shadowMapProjectionHint(PERSPECTIVE_SHADOW_MAP),
    _debugDraw(false)
{
    _shadowRecievingPlaceholderStateSet = new osg::StateSet;
}

ViewDependentShadowMap::ViewDependentShadowMap(const ViewDependentShadowMap& vdsm, const osg::CopyOp& copyop):
    ShadowTechnique(vdsm,copyop),
    _baseShadowTextureUnit(vdsm._baseShadowTextureUnit),
    _shadowMapProjectionHint(vdsm._shadowMapProjectionHint),
    _debugDraw(vdsm._debugDraw)
{
    _shadowRecievingPlaceholderStateSet = new osg::StateSet;
}

ViewDependentShadowMap::~ViewDependentShadowMap()
{
}


void ViewDependentShadowMap::init()
{
    if (!_shadowedScene) return;

    OSG_NOTICE<<"ViewDependentShadowMap::init()"<<std::endl;

    createShaders();
    
    _dirty = false;
}

void ViewDependentShadowMap::cleanSceneGraph()
{
    OSG_NOTICE<<"ViewDependentShadowMap::cleanSceneGraph()"<<std::endl;
}

ViewDependentShadowMap::ViewDependentData* ViewDependentShadowMap::createViewDependentData(osgUtil::CullVisitor* cv)
{
    return new ViewDependentData(this);
}

ViewDependentShadowMap::ViewDependentData* ViewDependentShadowMap::getViewDependentData(osgUtil::CullVisitor* cv)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_viewDependentDataMapMutex);
    ViewDependentDataMap::iterator itr = _viewDependentDataMap.find(cv);
    if (itr!=_viewDependentDataMap.end()) return itr->second.get();

    osg::ref_ptr<ViewDependentData> vdd = createViewDependentData(cv);
    _viewDependentDataMap[cv] = vdd;
    return vdd.release();
}

void ViewDependentShadowMap::update(osg::NodeVisitor& nv)
{
    OSG_INFO<<"ViewDependentShadowMap::update(osg::NodeVisitor& "<<&nv<<")"<<std::endl;
    _shadowedScene->osg::Group::traverse(nv);
}

void ViewDependentShadowMap::cull(osgUtil::CullVisitor& cv)
{
    OSG_INFO<<std::endl<<std::endl<<"ViewDependentShadowMap::cull(osg::CullVisitor&"<<&cv<<")"<<std::endl;

    ViewDependentData* vdd = getViewDependentData(&cv);

    if (!vdd)
    {
        OSG_NOTICE<<"Warning, now ViewDependentData created, unable to create shadows."<<std::endl;
        _shadowedScene->osg::Group::traverse(cv);
        return;
    }

    OSG_INFO<<"cv->getProjectionMatrix()="<<*cv.getProjectionMatrix()<<std::endl;

    osg::CullSettings::ComputeNearFarMode cachedNearFarMode = cv.getComputeNearFarMode();

    osg::RefMatrix& viewProjectionMatrix = *cv.getProjectionMatrix();
    
    // check whether this main views projection is perspective or orthographic
    bool orthographicViewFrustum = viewProjectionMatrix(0,3)==0.0 &&
                                   viewProjectionMatrix(1,3)==0.0 &&
                                   viewProjectionMatrix(2,3)==0.0;

    double minZNear = 0.0;
    double maxZFar = DBL_MAX;

    if (cachedNearFarMode==osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR)
    {
        double left, right, top, bottom;
        if (orthographicViewFrustum)
        {
            viewProjectionMatrix.getOrtho(left, right, bottom, top, minZNear, maxZFar);
        }
        else
        {
            viewProjectionMatrix.getFrustum(left, right, bottom, top, minZNear, maxZFar);
        }
        OSG_INFO<<"minZNear="<<minZNear<<", maxZFar="<<maxZFar<<std::endl;
    }

    // set the compute near/far mode to the highest quality setting to ensure we push the near plan out as far as possible
    cv.setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES);

    // 1. Traverse main scene graph
    cv.pushStateSet( _shadowRecievingPlaceholderStateSet.get() );

    osg::ref_ptr<osgUtil::StateGraph> decoratorStateGraph = cv.getCurrentStateGraph();

    cullShadowReceivingScene(&cv);

    cv.popStateSet();

    if (cv.getComputeNearFarMode()!=osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR)
    {
        // make sure that the near plane is computed correctly so that any projection matrix computations
        // are all done correctly.
        cv.computeNearPlane();
    }
    
    Frustum frustum(&cv, minZNear, maxZFar);

    // return compute near far mode back to it's original settings
    cv.setComputeNearFarMode(cachedNearFarMode);


    // 2. select active light sources
    //    create a list of light sources + their matrices to place them
    selectActiveLights(&cv, vdd);

    unsigned int pos_x = 0;
    unsigned int textureUnit = _baseShadowTextureUnit;
    unsigned int numValidShadows = 0;

    ShadowDataList& sdl = vdd->getShadowDataList();
    ShadowDataList previous_sdl;
    previous_sdl.swap(sdl);
    
    LightDataList& pll = vdd->getLightDataList();
    for(LightDataList::iterator itr = pll.begin();
        itr != pll.end();
        ++itr)
    {
        // 3. create per light/per shadow map division of lightspace/frustum
        //    create a list of light/shadow map data structures

        LightData& pl = **itr;

        // 4. For each light/shadow map
        {
            osg::ref_ptr<ShadowData> sd;

            if (previous_sdl.empty())
            {
                OSG_INFO<<"Create new ShadowData"<<std::endl;
                sd = new ShadowData(vdd);
            }
            else
            {
                OSG_INFO<<"Taking ShadowData from from of previous_sdl"<<std::endl;
                sd = previous_sdl.front();
                previous_sdl.erase(previous_sdl.begin());
            }

            osg::ref_ptr<osg::Camera> camera = sd->_camera;

            if (_debugDraw)
            {
                camera->getViewport()->x() = pos_x;
                pos_x += camera->getViewport()->width() + 40;
            }

            osg::ref_ptr<osg::TexGen> texgen = sd->_texgen;

            // 4.1 compute light space polytope
            //

            osg::Polytope polytope = computeLightViewFrustumPolytope(frustum, pl);

            // if polytope is empty then no rendering.
            if (polytope.empty())
            {
                OSG_NOTICE<<"Polytope empty no shadow to render"<<std::endl;
                continue;
            }

            // 4.2 compute RTT camera view+projection matrix settings
            //
            if (!computeShadowCameraSettings(frustum, pl, camera.get()))
            {
                OSG_NOTICE<<"No valid Camera settings, no shadow to render"<<std::endl;
                continue;
            }

            // transform polytope in model coords into light spaces eye coords.
            osg::Matrixd invertModelView;
            invertModelView.invert(camera->getViewMatrix());

            polytope.transformProvidingInverse(invertModelView);

            osg::ref_ptr<VDSMCameraCullCallback> vdsmCallback = new VDSMCameraCullCallback(this, polytope);
            camera->setCullCallback(vdsmCallback.get());

            // 4.3 traverse RTT camera
            //

            cv.pushStateSet(_shadowCastingStateSet.get());

            cullShadowCastingScene(&cv, camera.get());

            cv.popStateSet();

            if (!orthographicViewFrustum && _shadowMapProjectionHint==PERSPECTIVE_SHADOW_MAP)
            {
                adjustPerspectiveShadowMapCameraSettings(vdsmCallback->getRenderStage(), frustum, pl, camera.get());
                if (vdsmCallback->getProjectionMatrix())
                {
                    vdsmCallback->getProjectionMatrix()->set(camera->getProjectionMatrix());
                }
            }

            // 4.4 compute main scene graph TexGen + uniform settings + setup state
            //

            assignTexGenSettings(&cv, camera.get(), textureUnit, texgen.get());


            // mark the light as one that has active shadows and requires shaders
            pl.textureUnits.push_back(textureUnit);

            // pass on shadow data to ShadowDataList
            sd->_textureUnit = textureUnit;
            
            sdl.push_back(sd);
            
            // increment counters.
            ++textureUnit;
            ++numValidShadows ;
        }
    }

    if (numValidShadows>0)
    {
        decoratorStateGraph->setStateSet(selectStateSetForRenderingShadow(*vdd));        
    }
   
    // OSG_NOTICE<<"End of shadow setup Projection matrix "<<*cv.getProjectionMatrix()<<std::endl;
}

bool ViewDependentShadowMap::selectActiveLights(osgUtil::CullVisitor* cv, ViewDependentData* vdd) const
{
    OSG_INFO<<"selectActiveLights"<<std::endl;

    LightDataList& pll = vdd->getLightDataList();
    
    LightDataList previous_ldl;
    previous_ldl.swap(pll);

    //MR testing giving a specific light
    osgUtil::RenderStage * rs = cv->getRenderStage();

    osg::Matrixd modelViewMatrix = *(cv->getModelViewMatrix());
    
    osgUtil::PositionalStateContainer::AttrMatrixList& aml =
        rs->getPositionalStateContainer()->getAttrMatrixList();

    for(osgUtil::PositionalStateContainer::AttrMatrixList::reverse_iterator itr = aml.rbegin();
        itr != aml.rend();
        ++itr)
    {
        const osg::Light* light = dynamic_cast<const osg::Light*>(itr->first.get());
        if (light)
        {
            LightDataList::iterator pll_itr = pll.begin();
            for(; pll_itr != pll.end(); ++pll_itr)
            {
                if ((*pll_itr)->light->getLightNum()==light->getLightNum()) break;
            }

            if (pll_itr==pll.end())
            {
                OSG_INFO<<"Light num "<<light->getLightNum()<<std::endl;
                LightData* ld = new LightData(vdd);
                ld->setLightData(itr->second.get(), light, modelViewMatrix);
                pll.push_back(ld);
            }
            else
            {
                OSG_INFO<<"Light num "<<light->getLightNum()<<" already used, ignore light"<<std::endl;
            }            
        }
    }
    
    return !pll.empty();
}

void ViewDependentShadowMap::createShaders()
{
    OSG_NOTICE<<"ViewDependentShadowMap::createShaders()"<<std::endl;

    unsigned int _baseTextureUnit = 0;

    _shadowCastingStateSet = new osg::StateSet;

    if (!_debugDraw)
    {
        // note soft (attribute only no mode override) setting. When this works ?
        // 1. for objects prepared for backface culling
        //    because they usually also set CullFace and CullMode on in their state
        //    For them we override CullFace but CullMode remains set by them
        // 2. For one faced, trees, and similar objects which cannot use
        //    backface nor front face so they usually use CullMode off set here.
        //    In this case we will draw them in their entirety.

        _shadowCastingStateSet->setAttribute( new osg::CullFace( osg::CullFace::FRONT ),
                osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

        // make sure GL_CULL_FACE is off by default
        // we assume that if object has cull face attribute set to back
        // it will also set cull face mode ON so no need for override
        _shadowCastingStateSet->setMode( GL_CULL_FACE, osg::StateAttribute::OFF );
    }
    
#if 1
    float factor = 1.1;
    float units =  4.0;
#else
    float factor = -1.1;
    float units =  -4.0;
#endif
    _polygonOffset = new osg::PolygonOffset(factor, units);
    _shadowCastingStateSet->setAttribute(_polygonOffset.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    _shadowCastingStateSet->setMode(GL_POLYGON_OFFSET_FILL, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    
    _uniforms.clear();
    osg::ref_ptr<osg::Uniform> baseTextureSampler = new osg::Uniform("baseTexture",(int)_baseTextureUnit);
    _uniforms.push_back(baseTextureSampler.get());

    osg::ref_ptr<osg::Uniform> baseTextureUnit = new osg::Uniform("baseTextureUnit",(int)_baseTextureUnit);
    _uniforms.push_back(baseTextureUnit.get());

    osg::ref_ptr<osg::Uniform> shadowTextureSampler = new osg::Uniform("shadowTexture",(int)_baseShadowTextureUnit);
    _uniforms.push_back(shadowTextureSampler.get());

    osg::ref_ptr<osg::Uniform> shadowTextureUnit = new osg::Uniform("shadowTextureUnit",(int)_baseShadowTextureUnit);
    _uniforms.push_back(shadowTextureUnit.get());

    //osg::ref_ptr<osg::Shader> fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource_noBaseTexture);
    osg::ref_ptr<osg::Shader> fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource_withBaseTexture);

    _program = new osg::Program;
    _program->addShader(fragment_shader.get());
   
}

osg::Polytope ViewDependentShadowMap::computeLightViewFrustumPolytope(Frustum& frustum, LightData& positionedLight)
{
    OSG_INFO<<"computeLightViewFrustumPolytope()"<<std::endl;

    osg::Polytope polytope;
    polytope.setToUnitFrustum();

    polytope.transformProvidingInverse( frustum.projectionMatrix );
    polytope.transformProvidingInverse( frustum.modelViewMatrix );

    osg::Polytope lightVolumePolytope;

    if (positionedLight.directionalLight)
    {
        osg::Polytope::PlaneList& planes = polytope.getPlaneList();
        osg::Polytope::ClippingMask selector_mask = 0x1;
        osg::Polytope::ClippingMask result_mask = 0x0;
        for(unsigned int i=0; i<planes.size(); ++i, selector_mask <<= 1)
        {
            OSG_INFO<<"      plane "<<planes[i]<<"  planes["<<i<<"].dotProductNormal(lightDir)="<<planes[i].dotProductNormal(positionedLight.lightDir);
            if (planes[i].dotProductNormal(positionedLight.lightDir)>=0.0)
            {
                OSG_INFO<<"     Need remove side "<<i<<std::endl;
            }
            else
            {
                OSG_INFO<<std::endl;
                lightVolumePolytope.add(planes[i]);
                result_mask = result_mask | selector_mask;
            }
        }

        OSG_INFO<<"    planes.size() = "<<planes.size()<<std::endl;
        OSG_INFO<<"    planes.getResultMask() = "<<polytope.getResultMask()<<std::endl;
        OSG_INFO<<"    resultMask = "<<result_mask<<std::endl;
        polytope.setResultMask(result_mask);
    }
    else
    {
        const osg::Polytope::PlaneList& planes = polytope.getPlaneList();
        osg::Polytope::ClippingMask selector_mask = 0x1;
        osg::Polytope::ClippingMask result_mask = 0x0;
        for(unsigned int i=0; i<planes.size(); ++i, selector_mask <<= 1)
        {

            double d = planes[i].distance(positionedLight.lightPos3);
            OSG_INFO<<"      plane "<<planes[i]<<"  planes["<<i<<"].distance(lightPos3)="<<d;
            if (d<0.0)
            {
                OSG_INFO<<"     Need remove side "<<i<<std::endl;
            }
            else
            {
                OSG_INFO<<std::endl;
                lightVolumePolytope.add(planes[i]);
                result_mask = result_mask | selector_mask;
            }
        }
        OSG_INFO<<"    planes.size() = "<<planes.size()<<std::endl;
        OSG_INFO<<"    planes.getResultMask() = "<<polytope.getResultMask()<<std::endl;
        OSG_INFO<<"    resultMask = "<<result_mask<<std::endl;
        polytope.setResultMask(result_mask);
    }

    OSG_INFO<<"Which frustum edges are active?"<<std::endl;
    for(unsigned int i=0; i<12; ++i)
    {
        Frustum::Indices& indices = frustum.edges[i];

        unsigned int corner_a = indices[0];
        unsigned int corner_b = indices[1];
        unsigned int face_a = indices[2];
        unsigned int face_b = indices[3];
        bool face_a_active = (polytope.getResultMask()&(0x1<<face_a))!=0;
        bool face_b_active = (polytope.getResultMask()&(0x1<<face_b))!=0;
        unsigned int numActive = 0;
        if (face_a_active) ++numActive;
        if (face_b_active) ++numActive;
        if (numActive==1)
        {

            osg::Plane boundaryPlane;

            if (positionedLight.directionalLight)
            {
                osg::Vec3d normal = (frustum.corners[corner_b]-frustum.corners[corner_a])^positionedLight.lightDir;
                normal.normalize();
                boundaryPlane.set(normal, frustum.corners[corner_a]);
            }
            else
            {
                boundaryPlane.set(positionedLight.lightPos3, frustum.corners[corner_a], frustum.corners[corner_b]);
            }

            OSG_INFO<<"Boundary Edge "<<i<<", corner_a="<<corner_a<<", corner_b="<<corner_b<<", face_a_active="<<face_a_active<<", face_b_active="<<face_b_active;
            if (boundaryPlane.distance(frustum.center)<0.0)
            {
                boundaryPlane.flip();
                OSG_INFO<<", flipped boundary edge "<<boundaryPlane<<std::endl;
            }
            else
            {
                OSG_INFO<<", no need to flip boundary edge "<<boundaryPlane<<std::endl;
            }
            lightVolumePolytope.add(boundaryPlane);
        }
        else OSG_INFO<<"Internal Edge "<<i<<", corner_a="<<corner_a<<", corner_b="<<corner_b<<", face_a_active="<<face_a_active<<", face_b_active="<<face_b_active<<std::endl;
    }


    const osg::Polytope::PlaneList& planes = lightVolumePolytope.getPlaneList();
    for(unsigned int i=0; i<planes.size(); ++i)
    {
        OSG_INFO<<"      plane "<<planes[i]<<"  "<<((lightVolumePolytope.getResultMask() & (0x1<<i))?"on":"off")<<std::endl;
    }

    return lightVolumePolytope;
}

bool ViewDependentShadowMap::computeShadowCameraSettings(Frustum& frustum, LightData& positionedLight, osg::Camera* camera)
{
    OSG_INFO<<"standardShadowMapCameraSettings()"<<std::endl;

#if 0
    // compute the basis vectors for the light coordinate frame
    osg::Vec3d lightSide_y = positionedLight.lightDir ^ osg::Vec3d(0.0,1.0,0.0);
    osg::Vec3d lightSide_z = positionedLight.lightDir ^ osg::Vec3d(0.0,0.0,1.0);
    osg::Vec3d lightSide = (lightSide_y.length() > lightSide_z.length()) ? lightSide_y : lightSide_z;
    lightSide.normalize();
#else
    osg::Vec3d lightSide = positionedLight.lightDir ^ frustum.frustumCenterLine; lightSide.normalize();
#endif
    osg::Vec3d lightUp = lightSide ^ positionedLight.lightDir;

    OSG_INFO<<"positionedLight.lightDir="<<positionedLight.lightDir<<std::endl;
    OSG_INFO<<"lightSide="<<lightSide<<std::endl;
    OSG_INFO<<"lightUp="<<lightUp<<std::endl;

    if (positionedLight.directionalLight)
    {
        double xMin=0.0, xMax=0.0;
        double yMin=0.0, yMax=0.0;
        double zMin=0.0, zMax=0.0;

        for(Frustum::Vertices::iterator itr = frustum.corners.begin();
            itr != frustum.corners.end();
            ++itr)
        {
            osg::Vec3d cornerDelta(*itr - frustum.center);
            osg::Vec3d cornerInLightCoords(cornerDelta*lightSide,
                                           cornerDelta*lightUp,
                                           cornerDelta*positionedLight.lightDir);

            OSG_INFO<<"    corner ="<<*itr<<" in lightcoords "<<cornerInLightCoords<<std::endl;

            xMin = osg::minimum( xMin, cornerInLightCoords.x());
            xMax = osg::maximum( xMax, cornerInLightCoords.x());
            yMin = osg::minimum( yMin, cornerInLightCoords.y());
            yMax = osg::maximum( yMax, cornerInLightCoords.y());
            zMin = osg::minimum( zMin, cornerInLightCoords.z());
            zMax = osg::maximum( zMax, cornerInLightCoords.z());
        }

        OSG_INFO<<"before bs xMin="<<xMin<<", xMax="<<xMax<<", yMin="<<yMin<<", yMax="<<yMax<<", zMin="<<zMin<<", zMax="<<zMax<<std::endl;

        osg::BoundingSphere bs = _shadowedScene->getBound();
        osg::Vec3d modelCenterRelativeFrustumCenter(bs.center()-frustum.center);
        osg::Vec3d modelCenterInLightCoords(modelCenterRelativeFrustumCenter*lightSide,
                                            modelCenterRelativeFrustumCenter*lightUp,
                                            modelCenterRelativeFrustumCenter*positionedLight.lightDir);

        OSG_INFO<<"modelCenterInLight="<<modelCenterInLightCoords<<" radius="<<bs.radius()<<std::endl;
        double radius(bs.radius());

        xMin = osg::maximum(xMin, modelCenterInLightCoords.x()-radius);
        xMax = osg::minimum(xMax, modelCenterInLightCoords.x()+radius);
        yMin = osg::maximum(yMin, modelCenterInLightCoords.y()-radius);
        yMax = osg::minimum(yMax, modelCenterInLightCoords.y()+radius);
        zMin = modelCenterInLightCoords.z()-radius;
        zMax = osg::minimum(zMax, modelCenterInLightCoords.z()+radius);

        OSG_INFO<<"after bs xMin="<<xMin<<", xMax="<<xMax<<", yMin="<<yMin<<", yMax="<<yMax<<", zMin="<<zMin<<", zMax="<<zMax<<std::endl;

        if (xMin>=xMax || yMin>=yMax || zMin>=zMax)
        {
            OSG_INFO<<"Warning nothing available to create shadows"<<zMax<<std::endl;
            return false;
        }
        else
        {
            camera->setProjectionMatrixAsOrtho(xMin,xMax, yMin, yMax,0.0,zMax-zMin);
            camera->setViewMatrixAsLookAt(frustum.center+positionedLight.lightDir*zMin, frustum.center, lightUp);
        }
    }
    else
    {
        double zMax=-DBL_MAX;

        OSG_INFO<<"lightDir = "<<positionedLight.lightDir<<std::endl;
        OSG_INFO<<"lightPos3 = "<<positionedLight.lightPos3<<std::endl;
        for(Frustum::Vertices::iterator itr = frustum.corners.begin();
            itr != frustum.corners.end();
            ++itr)
        {
            osg::Vec3d cornerDelta(*itr - positionedLight.lightPos3);
            osg::Vec3d cornerInLightCoords(cornerDelta*lightSide,
                                           cornerDelta*lightUp,
                                           cornerDelta*positionedLight.lightDir);

            OSG_INFO<<"   cornerInLightCoords= "<<cornerInLightCoords<<std::endl;
            
            zMax = osg::maximum( zMax, cornerInLightCoords.z());
        }

        OSG_INFO<<"zMax = "<<zMax<<std::endl;

        if (zMax<0.0)
        {
            // view frustum entirely behind light
            return false;
        }

        double minRatio = 0.0001;
        double zMin=zMax*minRatio;

        double fov = positionedLight.light->getSpotCutoff() * 2.0;
        if(fov < 180.0)   // spotlight
        {
            camera->setProjectionMatrixAsPerspective(fov, 1.0, zMin, zMax);
            camera->setViewMatrixAsLookAt(positionedLight.lightPos3,
                                          positionedLight.lightPos3+positionedLight.lightDir, lightUp);
        }
        else
        {
            double fovMAX = 160.0f;
            fov = 0.0;

            // calculate the max FOV from the corners of the frustum relative to the light position
            for(Frustum::Vertices::iterator itr = frustum.corners.begin();
                itr != frustum.corners.end();
                ++itr)
            {
                osg::Vec3d cornerDelta(*itr - positionedLight.lightPos3);
                double length = cornerDelta.length();

                if (length==0.0) fov = osg::minimum(fov, 180.0);
                else
                {
                    double dotProduct = cornerDelta*positionedLight.lightDir;
                    double angle = 2.0*osg::RadiansToDegrees( acos(dotProduct/length) );
                    fov = osg::maximum(fov, angle);
                }
            }

            OSG_INFO<<"Computed fov = "<<fov<<std::endl;

            if (fov>fovMAX)
            {
                OSG_INFO<<"Clampping fov = "<<fov<<std::endl;
                fov=fovMAX;
            }

            camera->setProjectionMatrixAsPerspective(fov, 1.0, zMin, zMax);
            camera->setViewMatrixAsLookAt(positionedLight.lightPos3,
                                          positionedLight.lightPos3+positionedLight.lightDir, lightUp);

        }
    }
    return true;
}

struct ConvexHull
{
    typedef std::vector<osg::Vec3d> Vertices;
    typedef std::pair< osg::Vec3d, osg::Vec3d > Edge;
    typedef std::list< Edge > Edges;

    Edges _edges;

    void setToFrustum(ViewDependentShadowMap::Frustum& frustum)
    {
        _edges.push_back( Edge(frustum.corners[0],frustum.corners[1]) );
        _edges.push_back( Edge(frustum.corners[1],frustum.corners[2]) );
        _edges.push_back( Edge(frustum.corners[2],frustum.corners[3]) );
        _edges.push_back( Edge(frustum.corners[3],frustum.corners[0]) );

        _edges.push_back( Edge(frustum.corners[4],frustum.corners[5]) );
        _edges.push_back( Edge(frustum.corners[5],frustum.corners[6]) );
        _edges.push_back( Edge(frustum.corners[6],frustum.corners[7]) );
        _edges.push_back( Edge(frustum.corners[7],frustum.corners[4]) );

        _edges.push_back( Edge(frustum.corners[0],frustum.corners[4]) );
        _edges.push_back( Edge(frustum.corners[1],frustum.corners[5]) );
        _edges.push_back( Edge(frustum.corners[2],frustum.corners[6]) );
        _edges.push_back( Edge(frustum.corners[3],frustum.corners[7]) );
    }
    
    void transform(const osg::Matrixd& m)
    {
        for(Edges::iterator itr = _edges.begin();
            itr != _edges.end();
            ++itr)
        {
            itr->first = itr->first * m;
            itr->second = itr->second * m;
        }
    }
    
    void clip(const osg::Plane& plane)    
    {
        // OSG_NOTICE<<"clip("<<plane<<") edges.size()="<<_edges.size()<<std::endl;
        for(Edges::iterator itr = _edges.begin();
            itr != _edges.end();
            )
        {
            double d0 = plane.distance(itr->first);
            double d1 = plane.distance(itr->second);
            if (d0<0.0 && d1<0.0)
            {
                // OSG_NOTICE<<"  Edge completely outside, removing"<<std::endl;
                Edges::iterator to_delete_itr = itr;
                ++itr;
                _edges.erase(to_delete_itr);
            }
            else if (d0>=0.0 && d1>=0.0)
            {
                // OSG_NOTICE<<"  Edge completely inside"<<std::endl;
                ++itr;
            }
            else
            {
                osg::Vec3d& v0 = itr->first;
                osg::Vec3d& v1 = itr->second;
                osg::Vec3d intersection = v0 - (v1-v0)*(d0/(d1-d0));
                // OSG_NOTICE<<"  Edge across clip plane, v0="<<v0<<", v1="<<v1<<", intersection= "<<intersection<<std::endl;
                if (d0<0.0)
                {
                    // move first vertex on edge
                    v0 = intersection;
                }
                else
                {
                    // move second vertex on edge
                    v1 = intersection;
                }
                
                ++itr;
            }
        }
        // OSG_NOTICE<<"  after clip("<<plane<<") edges.size()="<<_edges.size()<<std::endl;
    }
    
    void clip(const osg::Polytope& polytope)
    {
        const osg::Polytope::PlaneList& planes = polytope.getPlaneList();
        for(osg::Polytope::PlaneList::const_iterator itr = planes.begin();
            itr != planes.end();
            ++itr)
        {
            clip(*itr);
        }
    }

    double min(unsigned int index) const
    {
        double m = DBL_MAX;
        for(Edges::const_iterator itr = _edges.begin();
            itr != _edges.end();
            ++itr)
        {
            const Edge& edge = *itr;
            if (edge.first[index]<m) m = edge.first[index];
            if (edge.second[index]<m) m = edge.second[index];
        }
        return m;
    }
    
    double max(unsigned int index) const
    {
        double m = -DBL_MAX;
        for(Edges::const_iterator itr = _edges.begin();
            itr != _edges.end();
            ++itr)
        {
            const Edge& edge = *itr;
            if (edge.first[index]>m) m = edge.first[index];
            if (edge.second[index]>m) m = edge.second[index];
        }
        return m;
    }

    double minRatio(const osg::Vec3d& eye, unsigned int index) const
    {
        double m = DBL_MAX;
        osg::Vec3d delta;
        double ratio;
        for(Edges::const_iterator itr = _edges.begin();
            itr != _edges.end();
            ++itr)
        {
            const Edge& edge = *itr;

            delta = edge.first-eye;
            ratio = delta[index]/delta[1];
            if (ratio<m) m = ratio;

            delta = edge.second-eye;
            ratio = delta[index]/delta[1];
            if (ratio<m) m = ratio;
        }
        return m;
    }

    double maxRatio(const osg::Vec3d& eye, unsigned int index) const
    {
        double m = -DBL_MAX;
        osg::Vec3d delta;
        double ratio;
        for(Edges::const_iterator itr = _edges.begin();
            itr != _edges.end();
            ++itr)
        {
            const Edge& edge = *itr;

            delta = edge.first-eye;
            ratio = delta[index]/delta[1];
            if (ratio>m) m = ratio;

            delta = edge.second-eye;
            ratio = delta[index]/delta[1];
            if (ratio>m) m = ratio;
        }
        return m;
    }

    void output(std::ostream& out)
    {
        out<<"ConvexHull"<<std::endl;
        for(Edges::const_iterator itr = _edges.begin();
            itr != _edges.end();
            ++itr)
        {
            const Edge& edge = *itr;
            out<<"   edge ("<<edge.first<<") ("<<edge.second<<")"<<std::endl;
        }
    }
};


struct RenderLeafBounds
{
    RenderLeafBounds():
        numRenderLeaf(0),
        n(0.0),
        previous_modelview(0),
        clip_min_x(-1.0), clip_max_x(1.0),
        clip_min_y(-1.0), clip_max_y(1.0),
        clip_min_z(-1.0), clip_max_z(1.0),
        clip_min_x_ratio(-DBL_MAX), clip_max_x_ratio(DBL_MAX),
        clip_min_z_ratio(-DBL_MAX), clip_max_z_ratio(DBL_MAX),
        min_x_ratio(DBL_MAX), max_x_ratio(-DBL_MAX),
        min_z_ratio(DBL_MAX), max_z_ratio(-DBL_MAX),
        min_x(1.0), max_x(-1.0),
        min_y(1.0), max_y(-1.0),
        min_z(1.0), max_z(-1.0)
    {
        //OSG_NOTICE<<std::endl<<"RenderLeafBounds"<<std::endl;
    }

    void set(const osg::Matrixd& vp, osg::Vec3d& e_ls, double nr)
    {
        light_vp = vp;
        eye_ls = e_ls;
        n = nr;
    }
    
    void operator() (const osgUtil::RenderLeaf* renderLeaf)
    {
        ++numRenderLeaf;
        
        if (renderLeaf->_modelview.get()!=previous_modelview)
        {
            previous_modelview = renderLeaf->_modelview.get();
            light_mvp.mult(*renderLeaf->_modelview, light_vp);
            //OSG_NOTICE<<"Computing new light_mvp "<<light_mvp<<std::endl;
        }
        else
        {
            //OSG_NOTICE<<"Reusing light_mvp "<<light_mvp<<std::endl;
        }

        const osg::BoundingBox& bb = renderLeaf->_drawable->getBound();
        if (bb.valid())
        {
            handle(osg::Vec3d(bb.xMin(),bb.yMin(),bb.zMin()));
            handle(osg::Vec3d(bb.xMax(),bb.yMin(),bb.zMin()));
            handle(osg::Vec3d(bb.xMin(),bb.yMax(),bb.zMin()));
            handle(osg::Vec3d(bb.xMax(),bb.yMax(),bb.zMin()));
            handle(osg::Vec3d(bb.xMin(),bb.yMin(),bb.zMax()));
            handle(osg::Vec3d(bb.xMax(),bb.yMin(),bb.zMax()));
            handle(osg::Vec3d(bb.xMin(),bb.yMax(),bb.zMax()));
            handle(osg::Vec3d(bb.xMax(),bb.yMax(),bb.zMax()));
        }
        else
        {
            //OSG_NOTICE<<"bb invalid"<<std::endl;
        }
    }

    void handle(const osg::Vec3d& v)
    {
        osg::Vec3d ls = v * light_mvp;

        osg::Vec3d delta = ls-eye_ls;

        double x_ratio, z_ratio;
        if (delta.y()>n)
        {
            x_ratio = delta.x()/delta.y();
            z_ratio = delta.z()/delta.y();
        }
        else
        {
            x_ratio = delta.x()/n;
            z_ratio = delta.z()/n;
        }

        if (x_ratio<min_x_ratio) min_x_ratio = x_ratio;
        if (x_ratio>max_x_ratio) max_x_ratio = x_ratio;
        if (z_ratio<min_z_ratio) min_z_ratio = z_ratio;
        if (z_ratio>max_z_ratio) max_z_ratio = z_ratio;
        
        // clip to the light space
        if (ls.x()<clip_min_x) ls.x()=clip_min_x;
        if (ls.x()>clip_max_x) ls.x()=clip_max_x;
        if (ls.y()<clip_min_y) ls.y()=clip_min_y;
        if (ls.y()>clip_max_y) ls.y()=clip_max_y;
        if (ls.z()<clip_min_z) ls.z()=clip_min_z;
        if (ls.z()>clip_max_z) ls.z()=clip_max_z;

        // compute the xyz range.
        if (ls.x()<min_x) min_x=ls.x();
        if (ls.x()>max_x) max_x=ls.x();
        if (ls.y()<min_y) min_y=ls.y();
        if (ls.y()>max_y) max_y=ls.y();
        if (ls.z()<min_z) min_z=ls.z();
        if (ls.z()>max_z) max_z=ls.z();
    }

    unsigned int        numRenderLeaf;

    osg::Matrixd        light_vp;
    osg::Vec3d          eye_ls;
    double              n;
    
    osg::Matrixd        light_mvp;
    osg::RefMatrix*     previous_modelview;
    unsigned int        numLeaves;

    double clip_min_x, clip_max_x;
    double clip_min_y, clip_max_y;
    double clip_min_z, clip_max_z;

    double clip_min_x_ratio, clip_max_x_ratio;
    double clip_min_z_ratio, clip_max_z_ratio;
    
    double min_x_ratio, max_x_ratio;
    double min_z_ratio, max_z_ratio;
    double min_x, max_x;
    double min_y, max_y;
    double min_z, max_z;
};

bool ViewDependentShadowMap::adjustPerspectiveShadowMapCameraSettings(osgUtil::RenderStage* renderStage, Frustum& frustum, LightData& positionedLight, osg::Camera* camera)
{

    //frustum.projectionMatrix;
    //frustum.modelViewMatrix;

    osg::Matrixd light_p = camera->getProjectionMatrix();
    osg::Matrixd light_v = camera->getViewMatrix();
    osg::Matrixd light_vp = light_v * light_p;
    osg::Vec3d lightdir(0.0,0.0,-1.0);

    // check whether this light space projection is perspective or orthographic.
    bool orthographicLightSpaceProjection = light_p(0,3)==0.0 && light_p(1,3)==0.0 && light_p(2,3)==0.0;

    if (!orthographicLightSpaceProjection)
    {
        OSG_INFO<<"perspective light space projection not yet supported."<<std::endl;
        return false;
    }


    //OSG_NOTICE<<"light_v="<<light_v<<std::endl;
    //OSG_NOTICE<<"light_p="<<light_p<<std::endl;
    
    ConvexHull convexHull;
    convexHull.setToFrustum(frustum);
    
#if 0
    OSG_NOTICE<<"ws ConvexHull xMin="<<convexHull.min(0)<<", xMax="<<convexHull.max(0)<<std::endl;
    OSG_NOTICE<<"ws ConvexHull yMin="<<convexHull.min(1)<<", yMax="<<convexHull.max(1)<<std::endl;
    OSG_NOTICE<<"ws ConvexHull zMin="<<convexHull.min(2)<<", zMax="<<convexHull.max(2)<<std::endl;
    
    convexHull.output(osg::notify(osg::NOTICE));
#endif
    
    convexHull.transform(light_vp);

#if 0
    convexHull.output(osg::notify(osg::NOTICE));

    OSG_NOTICE<<"ls ConvexHull xMin="<<convexHull.min(0)<<", xMax="<<convexHull.max(0)<<std::endl;
    OSG_NOTICE<<"ls ConvexHull yMin="<<convexHull.min(1)<<", yMax="<<convexHull.max(1)<<std::endl;
    OSG_NOTICE<<"ls ConvexHull zMin="<<convexHull.min(2)<<", zMax="<<convexHull.max(2)<<std::endl;
#endif
    
    convexHull.clip(osg::Plane(0.0,0.0,1,1.0)); // clip by near plane of light space.
    convexHull.clip(osg::Plane(0.0,0.0,-1,1.0));  // clip by far plane of light space.

#if 0
    convexHull.output(osg::notify(osg::NOTICE));

    OSG_NOTICE<<"clipped ls ConvexHull xMin="<<convexHull.min(0)<<", xMax="<<convexHull.max(0)<<std::endl;
    OSG_NOTICE<<"clipped ls ConvexHull yMin="<<convexHull.min(1)<<", yMax="<<convexHull.max(1)<<std::endl;
    OSG_NOTICE<<"clipped ls ConvexHull zMin="<<convexHull.min(2)<<", zMax="<<convexHull.max(2)<<std::endl;
#endif
    
    double xMin = osg::maximum(-1.0,convexHull.min(0));
    double xMax = osg::minimum(1.0,convexHull.max(0));
    double yMin = osg::maximum(-1.0,convexHull.min(1));
    double yMax = osg::minimum(1.0,convexHull.max(1));
    double zMin = osg::maximum(-1.0,convexHull.min(2));
    double zMax = osg::minimum(1.0,convexHull.max(2));

    if (xMin!=-1.0 || yMin!=-1.0 || zMin!=-1.0 ||
        xMax!=1.0 || yMax!=1.0 || zMax!=1.0)
    {
        osg::Matrix m;
        m.makeTranslate(osg::Vec3d(-0.5*(xMax+xMin),
                                    -0.5*(yMax+yMin),
                                    -0.5*(zMax+zMin)));
        
        m.postMultScale(osg::Vec3d(2.0/(xMax-xMin),
                                   2.0/(yMax-yMin),
                                   2.0/(zMax-zMin)));

        convexHull.transform(m);
        light_p.postMult(m);
        light_vp = light_v * light_p;

#if 0        
        OSG_NOTICE<<"Adjusting projection matrix "<<m<<std::endl;
        convexHull.output(osg::notify(osg::NOTICE));
#endif
        camera->setProjectionMatrix(light_p);
        //return true;
    }

    osg::Vec3d eye_v = frustum.eye * light_v;
    osg::Vec3d centerNearPlane_v = frustum.centerNearPlane * light_v;
    osg::Vec3d center_v = frustum.center * light_v;
    osg::Vec3d viewdir_v = center_v-eye_v; viewdir_v.normalize();

    double dotProduct_v = lightdir * viewdir_v;
    double gamma_v = acos(dotProduct_v);
    double standardShadowMapToleranceAngle = 2.0;
    if (gamma_v<osg::DegreesToRadians(standardShadowMapToleranceAngle) || gamma_v>osg::DegreesToRadians(180-standardShadowMapToleranceAngle))
    {
        // OSG_NOTICE<<"Light and view vectors near parrallel - use standard shadow map."<<std::endl;
        return true;
    }

    //OSG_NOTICE<<"gamma="<<osg::RadiansToDegrees(gamma_v)<<std::endl;
    //OSG_NOTICE<<"eye_v="<<eye_v<<std::endl;
    //OSG_NOTICE<<"viewdir_v="<<viewdir_v<<std::endl;
    
    osg::Vec3d eye_ls = frustum.eye * light_vp;
#if 0
    if (eye_ls.y()>-1.0)
    {
        OSG_NOTICE<<"Eye point within light space - use standard shadow map."<<std::endl;
        return true;
    }
#endif
    
    osg::Vec3d centerNearPlane_ls = frustum.centerNearPlane * light_vp;
    osg::Vec3d centerFarPlane_ls = frustum.centerFarPlane * light_vp;
    osg::Vec3d center_ls = frustum.center * light_vp;
    osg::Vec3d viewdir_ls = center_ls-eye_ls; viewdir_ls.normalize();

    osg::Vec3d side = lightdir ^ viewdir_ls; side.normalize();
    osg::Vec3d up = side ^ lightdir;
    
    double d = 2.0;

    double alpha = osg::DegreesToRadians(30.0);
    double n = tan(alpha)*tan(osg::PI_2-gamma_v)*tan(osg::PI_2-gamma_v);
    //double n = tan(alpha)*tan(osg::PI_2-gamma_v);
    double min_n = osg::maximum(-1.0-eye_ls.y(), 0.01);
    if (n<min_n)
    {
        OSG_INFO<<"Clamping n to eye point"<<std::endl;
        n=min_n;
    }

    //n = min_n;

    //n = 0.01;

    //n = z_n;
    
    double f = n+d;

    double a = (f+n)/(f-n);
    double b = -2.0*f*n/(f-n);

    osg::Vec3d virtual_eye(0.0,-1.0-n, eye_ls.z());

    osg::Matrixd lightView;
    lightView.makeLookAt(virtual_eye, virtual_eye+lightdir, up);

#if 0    
    OSG_NOTICE<<"n = "<<n<<", f="<<f<<std::endl;
    OSG_NOTICE<<"eye_ls = "<<eye_ls<<", virtual_eye="<<virtual_eye<<std::endl;
    OSG_NOTICE<<"frustum.eye_ls="<<frustum.eye<<std::endl;
#endif

    double min_x_ratio = 0.0;
    double max_x_ratio = 0.0;
    double min_z_ratio = FLT_MAX;
    double max_z_ratio = -FLT_MAX;

    min_x_ratio = convexHull.minRatio(virtual_eye,0);
    min_z_ratio = convexHull.minRatio(virtual_eye,2);
    max_x_ratio = convexHull.maxRatio(virtual_eye,0);
    max_z_ratio = convexHull.maxRatio(virtual_eye,2);

    if (renderStage)
    {
        osg::ElapsedTime timer;
        RenderLeafTraverser<RenderLeafBounds> rli;
        rli.set(osg::Matrix::inverse(frustum.modelViewMatrix) * light_vp, virtual_eye, n);
        rli.traverse(renderStage);

        if (rli.numRenderLeaf==0)
        {
            return false;
        }
        
#if 0       
        OSG_NOTICE<<"Time for RenderLeafTraverser "<<timer.elapsedTime_m()<<"ms, number of render leaves "<<rli.numRenderLeaf<<std::endl;
        OSG_NOTICE<<"scene bounds min_x="<<rli.min_x<<", max_x="<<rli.max_x<<std::endl;
        OSG_NOTICE<<"scene bounds min_y="<<rli.min_y<<", max_y="<<rli.max_y<<std::endl;
        OSG_NOTICE<<"scene bounds min_z="<<rli.min_z<<", max_z="<<rli.max_z<<std::endl;
        OSG_NOTICE<<"min_x_ratio="<<rli.min_x_ratio<<", max_x_ratio="<<rli.max_x_ratio<<std::endl;
        OSG_NOTICE<<"min_z_ratio="<<rli.min_z_ratio<<", max_z_ratio="<<rli.max_z_ratio<<std::endl;
#endif
        if (rli.max_z_ratio>max_z_ratio) max_z_ratio = rli.max_z_ratio;
        if (rli.min_z_ratio<min_z_ratio) min_z_ratio = rli.min_z_ratio;

        //if (rli.min_y>min_y) min_y = rli.min_y;
        //if (rli.max_y<max_y) max_y = rli.max_y;
    }

    double best_x_ratio = osg::maximum(fabs(min_x_ratio),fabs(max_x_ratio));
    double best_z_ratio = osg::maximum(fabs(min_z_ratio),fabs(max_z_ratio));

#if 0
    OSG_NOTICE<<"min_x_ratio = "<<min_x_ratio<<std::endl;
    OSG_NOTICE<<"max_x_ratio = "<<max_x_ratio<<std::endl;
    OSG_NOTICE<<"best_x_ratio = "<<best_x_ratio<<std::endl;
    OSG_NOTICE<<"min_z_ratio = "<<min_z_ratio<<std::endl;
    OSG_NOTICE<<"max_z_ratio = "<<max_z_ratio<<std::endl;
    OSG_NOTICE<<"best_z_ratio = "<<best_z_ratio<<std::endl;
#endif

    osg::Matrixd lightPerspective( 1.0/best_x_ratio,  0.0, 0.0,  0.0,
                                   0.0,  a,   0.0,  1.0,
                                   0.0,  0.0, 1.0/best_z_ratio,  0.0,
                                   0.0,  b,   0.0,  0.0 );
    osg::Matrixd light_persp = light_p * lightView * lightPerspective;

#if 0
    OSG_NOTICE<<"light_p = "<<light_p<<std::endl;
    OSG_NOTICE<<"lightView = "<<lightView<<std::endl;
    OSG_NOTICE<<"lightPerspective = "<<lightPerspective<<std::endl;
    OSG_NOTICE<<"light_persp result = "<<light_persp<<std::endl;
#endif
    camera->setProjectionMatrix(light_persp);

    return true;
}

bool ViewDependentShadowMap::assignTexGenSettings(osgUtil::CullVisitor* cv, osg::Camera* camera, unsigned int textureUnit, osg::TexGen* texgen)
{
    OSG_INFO<<"assignTexGenSettings()"<<std::endl;

    texgen->setMode(osg::TexGen::EYE_LINEAR);

    // compute the matrix which takes a vertex from local coords into tex coords
    // We actually use two matrices one used to define texgen
    // and second that will be used as modelview when appling to OpenGL
    texgen->setPlanesFromMatrix( camera->getProjectionMatrix() *
                                 osg::Matrix::translate(1.0,1.0,1.0) *
                                 osg::Matrix::scale(0.5f,0.5f,0.5f) );

    // Place texgen with modelview which removes big offsets (making it float friendly)
    osg::ref_ptr<osg::RefMatrix> refMatrix =
        new osg::RefMatrix( camera->getInverseViewMatrix() * (*(cv->getModelViewMatrix())) );

    cv->getRenderStage()->getPositionalStateContainer()->addPositionedTextureAttribute( textureUnit, refMatrix.get(), texgen );


    return true;
}

void ViewDependentShadowMap::cullShadowReceivingScene(osgUtil::CullVisitor* cv) const
{
    OSG_INFO<<"cullShadowReceivingScene()"<<std::endl;

    // record the traversal mask on entry so we can reapply it later.
    unsigned int traversalMask = cv->getTraversalMask();

    cv->setTraversalMask( traversalMask & _shadowedScene->getReceivesShadowTraversalMask() );

    _shadowedScene->osg::Group::traverse(*cv);

    cv->setTraversalMask( traversalMask );

    return;
}

void ViewDependentShadowMap::cullShadowCastingScene(osgUtil::CullVisitor* cv, osg::Camera* camera) const
{
    OSG_INFO<<"cullShadowCastingScene()"<<std::endl;
    
    // record the traversal mask on entry so we can reapply it later.
    unsigned int traversalMask = cv->getTraversalMask();

    cv->setTraversalMask( traversalMask & _shadowedScene->getCastsShadowTraversalMask() );

        if (camera) camera->accept(*cv);

    cv->setTraversalMask( traversalMask );

    return;
}

osg::StateSet* ViewDependentShadowMap::selectStateSetForRenderingShadow(ViewDependentData& vdd) const
{
    OSG_INFO<<"   selectStateSetForRenderingShadow() "<<vdd.getStateSet()<<std::endl;

    osg::ref_ptr<osg::StateSet> stateset = vdd.getStateSet();
    
    vdd.getStateSet()->clear();
    //vdd.getStateSet()->setTextureMode(0, GL_TEXTURE_2D, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

    for(Uniforms::const_iterator itr=_uniforms.begin();
        itr!=_uniforms.end();
        ++itr)
    {
        stateset->addUniform(itr->get());
    }

    stateset->setAttribute(_program.get());

    LightDataList& pll = vdd.getLightDataList();
    for(LightDataList::iterator itr = pll.begin();
        itr != pll.end();
        ++itr)
    {
        // 3. create per light/per shadow map division of lightspace/frustum
        //    create a list of light/shadow map data structures

        LightData& pl = (**itr);

        // if no texture units have been activated for this light then no shadow state required.
        if (pl.textureUnits.empty()) continue;

        for(LightData::ActiveTextureUnits::iterator atu_itr = pl.textureUnits.begin();
            atu_itr != pl.textureUnits.end();
            ++atu_itr)
        {
            OSG_INFO<<"   Need to assign state for "<<*atu_itr<<std::endl;
        }

    }

    ShadowDataList& sdl = vdd.getShadowDataList();
    for(ShadowDataList::iterator itr = sdl.begin();
        itr != sdl.end();
        ++itr)
    {
        // 3. create per light/per shadow map division of lightspace/frustum
        //    create a list of light/shadow map data structures

        ShadowData& sd = (**itr);

        OSG_INFO<<"   ShadowData for "<<sd._textureUnit<<std::endl;

        stateset->setTextureAttributeAndModes(sd._textureUnit, sd._texture.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        stateset->setTextureMode(sd._textureUnit,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
        stateset->setTextureMode(sd._textureUnit,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
        stateset->setTextureMode(sd._textureUnit,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);
        stateset->setTextureMode(sd._textureUnit,GL_TEXTURE_GEN_Q,osg::StateAttribute::ON);       
    }

    return vdd.getStateSet();
}
