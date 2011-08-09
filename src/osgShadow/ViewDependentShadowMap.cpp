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
#include <osg/io_utils>

using namespace osgShadow;

///////////////////////////////////////////////////////////////////////////////////////////////
//
// VDSMCameraCullCallback
//
class VDSMCameraCullCallback : public osg::NodeCallback
{
    public:

        VDSMCameraCullCallback(ViewDependentShadowMap* vdsm, osg::Polytope& polytope);

        virtual void operator()(osg::Node*, osg::NodeVisitor* nv);

    protected:

        ViewDependentShadowMap* _vdsm;
        osg::Polytope           _polytope;
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

    if (cv->getComputeNearFarMode() != osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR)        
    {
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
        OSG_NOTICE<<"RTT Projection matrix after clamping "<<projection<<std::endl;

        camera->setProjectionMatrix(projection);
    }
    
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
// LightData
//
ViewDependentShadowMap::LightData::LightData():
    active(false),
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
        OSG_NOTICE<<"   Directional light, lightPos="<<lightPos<<", lightDir="<<lightDir<<std::endl;
        if (lightMatrix.valid())
        {
            OSG_NOTICE<<"   Light matrix "<<*lightMatrix<<std::endl;
            osg::Matrix lightToLocalMatrix(*lightMatrix * osg::Matrix::inverse(modelViewMatrix) );
            lightDir = osg::Matrix::transform3x3( lightDir, lightToLocalMatrix );
            lightDir.normalize();
            OSG_NOTICE<<"   new LightDir ="<<lightDir<<std::endl;
        }
    }
    else
    {
        OSG_NOTICE<<"   Positional light, lightPos="<<lightPos<<std::endl;
        lightDir = light->getDirection();
        lightDir.normalize();
        if (lightMatrix.valid())
        {
            OSG_NOTICE<<"   Light matrix "<<*lightMatrix<<std::endl;
            osg::Matrix lightToLocalMatrix(*lightMatrix * osg::Matrix::inverse(modelViewMatrix) );
            lightPos = lightPos * lightToLocalMatrix;
            lightDir = osg::Matrix::transform3x3( lightDir, lightToLocalMatrix );
            lightDir.normalize();
            OSG_NOTICE<<"   new LightPos ="<<lightPos<<std::endl;
            OSG_NOTICE<<"   new LightDir ="<<lightDir<<std::endl;
        }
        lightPos3.set(lightPos.x()/lightPos.w(), lightPos.y()/lightPos.w(), lightPos.z()/lightPos.w());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
// ShadowData
//
ViewDependentShadowMap::ShadowData::ShadowData():
    _active(false),
    _textureUnit(0)
{
    bool debug = false;

    // set up texgen
    _texgen = new osg::TexGen;

    // set up the texture
    _texture = new osg::Texture2D;

    unsigned int textureSize = debug ? 512 : 2048;
    
    _texture->setTextureSize(textureSize, textureSize);
    //_texture->setInternalFormat(GL_DEPTH_COMPONENT);
    _texture->setInternalFormat(GL_RGB);
    //_texture->setShadowComparison(true);
    //_texture->setShadowTextureMode(osg::Texture2D::LUMINANCE);
    _texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    _texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

    // the shadow comparison should fail if object is outside the texture
    _texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
    _texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
    _texture->setBorderColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    
    // set up the camera
    _camera = new osg::Camera;

    _camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF_INHERIT_VIEWPOINT);

    _camera->setClearColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    //camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);

    // set viewport
    _camera->setViewport(0,0,textureSize,textureSize);


    if (debug)
    {
        // clear just the depth buffer
        _camera->setClearMask(GL_DEPTH_BUFFER_BIT);

        // render after the main camera
        _camera->setRenderOrder(osg::Camera::POST_RENDER);
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
        //_camera->attach(osg::Camera::DEPTH_BUFFER, _texture.get());
        _camera->attach(osg::Camera::COLOR_BUFFER, _texture.get());
    }   
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
// Frustum
//
ViewDependentShadowMap::Frustum::Frustum(osgUtil::CullVisitor* cv):
    corners(8),
    faces(6),
    edges(12)
{
    projectionMatrix = *(cv->getProjectionMatrix());
    modelViewMatrix = *(cv->getModelViewMatrix());

    OSG_NOTICE<<"Projection matrix "<<projectionMatrix<<std::endl;

    osgUtil::CullVisitor::value_type zNear = cv->getCalculatedNearPlane();
    osgUtil::CullVisitor::value_type zFar = cv->getCalculatedFarPlane();

    cv->clampProjectionMatrix(projectionMatrix,zNear,zFar);

    OSG_INFO<<"zNear = "<<zNear<<", zFar = "<<zFar<<std::endl;
    OSG_INFO<<"Projection matrix after clamping "<<projectionMatrix<<std::endl;

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

    // compute center and the frustumCenterLine
    centerNearPlane = (corners[0]+corners[1]+corners[5]+corners[4])*0.25;
    centerFarPlane = (corners[3]+corners[2]+corners[6]+corners[7])*0.25;
    center = (centerNearPlane+centerNearPlane)*0.5;
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
ViewDependentShadowMap::ViewDependentData::ViewDependentData()
{
    OSG_NOTICE<<"ViewDependentData::ViewDependentData()"<<this<<std::endl;
    _stateset = new osg::StateSet;
}


///////////////////////////////////////////////////////////////////////////////////////////////
//
// ViewDependentShadowMap
//
ViewDependentShadowMap::ViewDependentShadowMap():
    ShadowTechnique()
{
    _shadowRecievingPlaceholderStateSet = new osg::StateSet;
}

ViewDependentShadowMap::ViewDependentShadowMap(const ViewDependentShadowMap& vdsm, const osg::CopyOp& copyop):
    ShadowTechnique(vdsm,copyop)
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
    return new ViewDependentData();
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
    OSG_NOTICE<<"ViewDependentShadowMap::update(osg::NodeVisitor& "<<&nv<<")"<<std::endl;
    _shadowedScene->osg::Group::traverse(nv);
}

void ViewDependentShadowMap::cull(osgUtil::CullVisitor& cv)
{
    OSG_NOTICE<<std::endl<<std::endl<<"ViewDependentShadowMap::cull(osg::CullVisitor&"<<&cv<<")"<<std::endl;

    ViewDependentData* vdd = getViewDependentData(&cv);

    if (!vdd)
    {
        OSG_NOTICE<<"Warning, now ViewDependentData created, unable to create shadows."<<std::endl;
        _shadowedScene->osg::Group::traverse(cv);
        return;
    }
    

    // 1. Traverse main scene graph
    cv.pushStateSet( _shadowRecievingPlaceholderStateSet.get() );

    osg::ref_ptr<osgUtil::StateGraph> decoratorStateGraph = cv.getCurrentStateGraph();

    cullShadowReceivingScene(&cv);

    cv.popStateSet();

    
    // 2. select active light sources
    //    create a list of light sources + their matrices to place them
    LightDataList& pll = vdd->getLightDataList();
    selectActiveLights(&cv, pll);

    unsigned int pos_x = 0;
    unsigned int baseTextureUnit = 0;
    unsigned int textureUnit = baseTextureUnit;
    unsigned int numValidShadows = 0;

    bool debug = false;

    Frustum frustum(&cv);

    ShadowDataList& sdl = vdd->getShadowDataList();
    ShadowDataList previous_sdl;
    previous_sdl.swap(sdl);
    
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
                OSG_NOTICE<<"Create new ShadowData"<<std::endl;
                sd = new ShadowData;
            }
            else
            {
                OSG_NOTICE<<"Taking ShadowData from from of previous_sdl"<<std::endl;
                sd = previous_sdl.front();
                previous_sdl.erase(previous_sdl.begin());
            }

            osg::ref_ptr<osg::Camera> camera = sd->_camera;

            if (debug)
            {
                camera->setViewport(pos_x,0,400,400);
                pos_x += 440;
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

            camera->setCullCallback(new VDSMCameraCullCallback(this, polytope));

            // 4.3 traverse RTT camera
            //

            cullShadowCastingScene(&cv, camera.get());

            // 4.4 compute main scene graph TexGen + uniform settings + setup state
            //

            assignTexGenSettings(&cv, camera.get(), textureUnit, texgen);


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
        OSG_NOTICE<<"Need to assign "<<numValidShadows<<" shadows"<<std::endl;

        decoratorStateGraph->setStateSet(selectStateSetForRenderingShadow(*vdd));        
    }
   
}

bool ViewDependentShadowMap::selectActiveLights(osgUtil::CullVisitor* cv, LightDataList& pll) const
{
    OSG_NOTICE<<"selectActiveLights"<<std::endl;

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
                OSG_NOTICE<<"Light num "<<light->getLightNum()<<std::endl;
                LightData* ld = new LightData();
                ld->setLightData(itr->second, light, modelViewMatrix);
                pll.push_back(ld);
            }
            else
            {
                OSG_NOTICE<<"Light num "<<light->getLightNum()<<" already used, ignore light"<<std::endl;
            }            
        }
    }
    
    return !pll.empty();
}

void ViewDependentShadowMap::createShaders()
{
    OSG_NOTICE<<"ViewDependentShadowMap::createShaders()"<<std::endl;
}

osg::Polytope ViewDependentShadowMap::computeLightViewFrustumPolytope(Frustum& frustum, LightData& positionedLight)
{
    OSG_NOTICE<<"computeLightViewFrustumPolytope()"<<std::endl;

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
    OSG_NOTICE<<"computeShadowCameraSettings()"<<std::endl;

    // compute the basis vectors for the light coordinate frame
    osg::Vec3d lightSide_y = positionedLight.lightDir ^ osg::Vec3d(0.0,1.0,0.0);
    osg::Vec3d lightSide_z = positionedLight.lightDir ^ osg::Vec3d(0.0,0.0,1.0);
    osg::Vec3d lightSide = (lightSide_y.length() > lightSide_z.length()) ? lightSide_y : lightSide_z;
    lightSide.normalize();

    //osg::Vec3d lightSide = positionedLight.lightDir ^ frustum.frustumCenterLine; lightSide.normalize();
    osg::Vec3d lightUp = lightSide ^ positionedLight.lightDir;

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

bool ViewDependentShadowMap::assignTexGenSettings(osgUtil::CullVisitor* cv, osg::Camera* camera, unsigned int textureUnit, osg::TexGen* texgen)
{
    OSG_NOTICE<<"assignTexGenSettings()"<<std::endl;

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
    OSG_NOTICE<<"cullShadowReceivingScene()"<<std::endl;

    // record the traversal mask on entry so we can reapply it later.
    unsigned int traversalMask = cv->getTraversalMask();

    cv->setTraversalMask( traversalMask & _shadowedScene->getReceivesShadowTraversalMask() );

    _shadowedScene->osg::Group::traverse(*cv);

    cv->setTraversalMask( traversalMask );

    return;
}

void ViewDependentShadowMap::cullShadowCastingScene(osgUtil::CullVisitor* cv, osg::Camera* camera) const
{
    OSG_NOTICE<<"cullShadowCastingScene()"<<std::endl;
    
    // record the traversal mask on entry so we can reapply it later.
    unsigned int traversalMask = cv->getTraversalMask();

    cv->setTraversalMask( traversalMask & _shadowedScene->getCastsShadowTraversalMask() );

        if (camera) camera->accept(*cv);

    cv->setTraversalMask( traversalMask );

    return;
}

osg::StateSet* ViewDependentShadowMap::selectStateSetForRenderingShadow(ViewDependentData& vdd) const
{
    OSG_NOTICE<<"   selectStateSetForRenderingShadow() "<<vdd.getStateSet()<<std::endl;

    osg::ref_ptr<osg::StateSet> stateset = vdd.getStateSet();
    
    vdd.getStateSet()->clear();
    //vdd.getStateSet()->setTextureMode(0, GL_TEXTURE_2D, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

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
            OSG_NOTICE<<"   Need to assign state for "<<*atu_itr<<std::endl;
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

        OSG_NOTICE<<"   ShadowData for "<<sd._textureUnit<<std::endl;

        stateset->setTextureAttributeAndModes(sd._textureUnit, sd._texture.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        stateset->setTextureMode(sd._textureUnit,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
        stateset->setTextureMode(sd._textureUnit,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
        stateset->setTextureMode(sd._textureUnit,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);
        stateset->setTextureMode(sd._textureUnit,GL_TEXTURE_GEN_Q,osg::StateAttribute::ON);       
    }

    return vdd.getStateSet();
}

