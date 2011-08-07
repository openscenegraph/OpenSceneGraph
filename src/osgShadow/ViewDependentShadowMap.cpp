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
// ViewDependentShadowMap
//
ViewDependentShadowMap::ViewDependentShadowMap():
    ShadowTechnique()
{
}

ViewDependentShadowMap::ViewDependentShadowMap(const ViewDependentShadowMap& vdsm, const osg::CopyOp& copyop):
    ShadowTechnique(vdsm,copyop)
{
}

ViewDependentShadowMap::~ViewDependentShadowMap()
{
}


void ViewDependentShadowMap::init()
{
    if (!_shadowedScene) return;

    OSG_NOTICE<<"ViewDependentShadowMap::init()"<<std::endl;
    
    _dirty = false;
}

void ViewDependentShadowMap::cleanSceneGraph()
{
    OSG_NOTICE<<"ViewDependentShadowMap::cleanSceneGraph()"<<std::endl;
}

ViewDependentShadowMap::ViewDependentData* ViewDependentShadowMap::createViewDependentData(osgUtil::CullVisitor* cv)
{
    return new ViewDependentData(this, cv);
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
    OSG_NOTICE<<"ViewDependentShadowMap::cull(osg::CullVisitor&"<<&cv<<")"<<std::endl;

    osg::StateSet* stateset = 0;
    
    // 1. Traverse main scene graph
    cullShadowReceivingScene(&cv, stateset);

    // 2. select active light sources
    //    create a list of light sources + their matrices to place them
    PositionedLightList pll;
    selectActiveLights(&cv, pll);

    for(PositionedLightList::iterator itr = pll.begin();
        itr != pll.end();
        ++itr)
    {
        // 3. create per light/per shadow map division of lightspace/frustum
        //    create a list of light/shadow map data structures

        PositionedLight& pl = *itr;

        // 4. For each light/shadow map
        {
            osg::Camera* camera = 0;
            osg::TexGen* texgen = 0;
            unsigned int textureUnit = 0;

            // 4.1 compute light space polytope
            //

            osg::Polytope polytope = computeLightViewFrustumPolytope(&cv, pl);

            // 4.2 compute RTT camera view+projection matrix settings
            //

            computeShadowCameraSettings(pl, polytope, camera);

            // 4.3 traverse RTT camera
            //

            cullShadowCastingScene(&cv, camera);

            // 4.4 compute main scene graph TexGen + uniform settings + setup state
            //

            assignTexGenSettings(&cv, camera, textureUnit, texgen);
        }
    }
   
}

bool ViewDependentShadowMap::selectActiveLights(osgUtil::CullVisitor* cv, PositionedLightList& pll) const
{
    OSG_NOTICE<<"selectActiveLights"<<std::endl;

    //MR testing giving a specific light
    osgUtil::RenderStage * rs = cv->getRenderStage();

    osgUtil::PositionalStateContainer::AttrMatrixList& aml =
        rs->getPositionalStateContainer()->getAttrMatrixList();

    for(osgUtil::PositionalStateContainer::AttrMatrixList::iterator itr = aml.begin();
        itr != aml.end();
        ++itr)
    {
        const osg::Light* light = dynamic_cast<const osg::Light*>(itr->first.get());
        if (light)
        {
            pll.push_back(PositionedLight(itr->second, light));
        }
    }
    
    return !pll.empty();
}

osg::Polytope ViewDependentShadowMap::computeLightViewFrustumPolytope(osgUtil::CullVisitor* cv, PositionedLight& positionedLight)
{
    OSG_NOTICE<<"computeLightViewFrustumPolytope()"<<std::endl;

    const osg::RefMatrix* lightMatrix = positionedLight.first.get();
    const osg::Light* light = positionedLight.second.get();

    osg::Matrix projectionMatrix( *(cv->getProjectionMatrix()) );
    osg::Matrix modelViewMatrix( *(cv->getModelViewMatrix()) );

    OSG_NOTICE<<"Projection matrix "<<projectionMatrix<<std::endl;

    osgUtil::CullVisitor::value_type zNear = cv->getCalculatedNearPlane();
    osgUtil::CullVisitor::value_type zFar = cv->getCalculatedFarPlane();

    cv->clampProjectionMatrix(projectionMatrix,zNear,zFar);

    OSG_NOTICE<<"zNear = "<<zNear<<", zFar = "<<zFar<<std::endl;
    OSG_NOTICE<<"Projection matrix after clamping "<<projectionMatrix<<std::endl;

    typedef std::vector<osg::Vec3d> Vertices;
    Vertices corners(8);

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

    osg::Vec3d center( osg::Vec3d(0.0,0.0,0.0) * clipToWorld );

    for(Vertices::iterator itr = corners.begin();
        itr != corners.end();
        ++itr)
    {
        *itr = (*itr) * clipToWorld;
        OSG_NOTICE<<"   corner "<<*itr<<std::endl;
    }

    typedef std::vector<unsigned int> Indices;
    typedef std::vector<Indices> Faces;
    Faces faces(6);

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

    typedef std::vector<Indices> Edges;
    Edges edges(12);
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


    osg::Polytope polytope;
    polytope.setToUnitFrustum();

    polytope.transformProvidingInverse( projectionMatrix );
    polytope.transformProvidingInverse( modelViewMatrix );

    osg::Polytope lightVolumePolytope;


    osg::Vec4d lightPos = positionedLight.second->getPosition();
    osg::Vec3d lightPos3(lightPos.x(), lightPos.y(), lightPos.z());
    osg::Vec3d lightDir;
    bool directionalLight = (lightPos.w()== 0.0);
    if (directionalLight)
    {
        lightDir.set(-lightPos.x(), -lightPos.y(), -lightPos.z());
        lightDir.normalize();
        OSG_NOTICE<<"   Directional light, lightPos="<<lightPos<<", lightDir="<<lightDir<<std::endl;
        if (lightMatrix)
        {
            OSG_NOTICE<<"   Light matrix "<<*lightMatrix<<std::endl;
            osg::Matrix lightToLocalMatrix(*lightMatrix * osg::Matrix::inverse(modelViewMatrix) );
            lightDir = osg::Matrix::transform3x3( lightDir, lightToLocalMatrix );
            lightDir.normalize();
            OSG_NOTICE<<"   new LightDir ="<<lightDir<<std::endl;
        }

        osg::Polytope::PlaneList& planes = polytope.getPlaneList();
        osg::Polytope::ClippingMask selector_mask = 0x1;
        osg::Polytope::ClippingMask result_mask = 0x0;
        for(unsigned int i=0; i<planes.size(); ++i, selector_mask <<= 1)
        {
            OSG_NOTICE<<"      plane "<<planes[i]<<"  planes["<<i<<"].dotProductNormal(lightDir)="<<planes[i].dotProductNormal(lightDir);
            if (planes[i].dotProductNormal(lightDir)>=0.0)
            {
                OSG_NOTICE<<"     Need remove side "<<i<<std::endl;
            }
            else
            {
                OSG_NOTICE<<std::endl;
                lightVolumePolytope.add(planes[i]);
                result_mask = result_mask | selector_mask;
            }
        }

        OSG_NOTICE<<"    planes.size() = "<<planes.size()<<std::endl;
        OSG_NOTICE<<"    planes.getResultMask() = "<<polytope.getResultMask()<<std::endl;
        OSG_NOTICE<<"    resultMask = "<<result_mask<<std::endl;
        polytope.setResultMask(result_mask);
    }
    else
    {
        OSG_NOTICE<<"   Positional light, lightPos="<<lightPos<<std::endl;
        lightDir = light->getDirection();
        lightDir.normalize();
        if (lightMatrix)
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
        OSG_NOTICE<<"   LightPos3 ="<<lightPos3<<std::endl;
        const osg::Polytope::PlaneList& planes = polytope.getPlaneList();
        osg::Polytope::ClippingMask selector_mask = 0x1;
        osg::Polytope::ClippingMask result_mask = 0x0;
        for(unsigned int i=0; i<planes.size(); ++i, selector_mask <<= 1)
        {

            double d = planes[i].distance(lightPos3);
            OSG_NOTICE<<"      plane "<<planes[i]<<"  planes["<<i<<"].distance(lightPos3)="<<d;
            if (d<0.0)
            {
                OSG_NOTICE<<"     Need remove side "<<i<<std::endl;
            }
            else
            {
                OSG_NOTICE<<std::endl;
                lightVolumePolytope.add(planes[i]);
                result_mask = result_mask | selector_mask;
            }
        }
        OSG_NOTICE<<"    planes.size() = "<<planes.size()<<std::endl;
        OSG_NOTICE<<"    planes.getResultMask() = "<<polytope.getResultMask()<<std::endl;
        OSG_NOTICE<<"    resultMask = "<<result_mask<<std::endl;
        polytope.setResultMask(result_mask);
    }

    OSG_NOTICE<<"Which frustum edges are active?"<<std::endl;
    for(unsigned int i=0; i<12; ++i)
    {
        Indices& indices = edges[i];

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

            if (directionalLight)
            {
                osg::Vec3d normal = (corners[corner_b]-corners[corner_a])^lightDir;
                normal.normalize();
                boundaryPlane.set(normal, corners[corner_a]);
            }
            else
            {
                boundaryPlane.set(lightPos3, corners[corner_a], corners[corner_b]);
            }

            OSG_NOTICE<<"Boundary Edge "<<i<<", corner_a="<<corner_a<<", corner_b="<<corner_b<<", face_a_active="<<face_a_active<<", face_b_active="<<face_b_active;
            if (boundaryPlane.distance(center)<0.0)
            {
                boundaryPlane.flip();
                OSG_NOTICE<<", flipped boundary edge "<<boundaryPlane<<std::endl;
            }
            else
            {
                OSG_NOTICE<<", no need to flip boundary edge "<<boundaryPlane<<std::endl;
            }
            lightVolumePolytope.add(boundaryPlane);
        }
        else OSG_NOTICE<<"Internal Edge "<<i<<", corner_a="<<corner_a<<", corner_b="<<corner_b<<", face_a_active="<<face_a_active<<", face_b_active="<<face_b_active<<std::endl;
    }

    return lightVolumePolytope;   
}

bool ViewDependentShadowMap::computeShadowCameraSettings(PositionedLight& positionedLight, osg::Polytope& polytope, osg::Camera* camera)
{
    OSG_NOTICE<<"computeShadowCameraSettings()"<<std::endl;

    const osg::Polytope::PlaneList& planes = polytope.getPlaneList();
    for(unsigned int i=0; i<planes.size(); ++i)
    {
        OSG_NOTICE<<"      plane "<<planes[i]<<"  "<<((polytope.getResultMask() & (0x1<<i))?"on":"off")<<std::endl;
    }
   

    return false;
}

bool ViewDependentShadowMap::assignTexGenSettings(osgUtil::CullVisitor* cv, osg::Camera* camera, unsigned int textureUnit, osg::TexGen* texgen)
{
    OSG_NOTICE<<"assignTexGenSettings()"<<std::endl;
    return false;
}

void ViewDependentShadowMap::cullShadowReceivingScene(osgUtil::CullVisitor* cv, osg::StateSet* stateset) const
{
    OSG_NOTICE<<"cullShadowReceivingScene()"<<std::endl;

    // record the traversal mask on entry so we can reapply it later.
    unsigned int traversalMask = cv->getTraversalMask();

    cv->setTraversalMask( traversalMask & _shadowedScene->getReceivesShadowTraversalMask() );

    if (stateset) cv->pushStateSet( stateset );

        _shadowedScene->osg::Group::traverse(*cv);

    if (stateset) cv->popStateSet();

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


///////////////////////////////////////////////////////////////////////////////////////////////
//
// ViewDependentData
//
ViewDependentShadowMap::ViewDependentData::ViewDependentData(ViewDependentShadowMap* vdsm, osgUtil::CullVisitor* cv)
{
    OSG_NOTICE<<"ViewDependentData::ViewDependentData("<<vdsm<<", "<<cv<<") "<<this<<std::endl;
}
