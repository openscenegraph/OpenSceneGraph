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
#include <osg/CullStack>
#include <osg/Timer>

#include <osg/Notify>
#include <osg/io_utils>

using namespace osg;

CullStack::CullStack()
{
    _frustumVolume=-1.0f;
    _bbCornerNear = 0;
    _bbCornerFar = 7;
    _currentReuseMatrixIndex=0;
    _identity = new RefMatrix();

    _index_modelviewCullingStack = 0;
    _back_modelviewCullingStack = 0;
    
    _referenceViewPoints.push_back(osg::Vec3(0.0f,0.0f,0.0f));
}

CullStack::CullStack(const CullStack& cs):
    CullSettings(cs)
{
    _frustumVolume=-1.0f;
    _bbCornerNear = 0;
    _bbCornerFar = 7;
    _currentReuseMatrixIndex=0;
    _identity = new RefMatrix();

    _index_modelviewCullingStack = 0;
    _back_modelviewCullingStack = 0;
    
    _referenceViewPoints.push_back(osg::Vec3(0.0f,0.0f,0.0f));
}


CullStack::~CullStack()
{
    reset();
}

void CullStack::reset()
{

    //
    // first unref all referenced objects and then empty the containers.
    //
    _projectionStack.clear();
    _modelviewStack.clear();
    _viewportStack.clear();

    _referenceViewPoints.clear();
    _referenceViewPoints.push_back(osg::Vec3(0.0f,0.0f,0.0f));
    
    _eyePointStack.clear();
    _viewPointStack.clear();


    _clipspaceCullingStack.clear();
    _projectionCullingStack.clear();

    //_modelviewCullingStack.clear();
    _index_modelviewCullingStack=0;
    _back_modelviewCullingStack = 0;

    osg::Vec3 lookVector(0.0,0.0,-1.0);
    
    _bbCornerFar = (lookVector.x()>=0?1:0) |
                   (lookVector.y()>=0?2:0) |
                   (lookVector.z()>=0?4:0);

    _bbCornerNear = (~_bbCornerFar)&7;
    
    _currentReuseMatrixIndex=0;
}


void CullStack::pushCullingSet()
{
    _MVPW_Stack.push_back(0L);

    if (_index_modelviewCullingStack==0) 
    {
        if (_modelviewCullingStack.empty())
            _modelviewCullingStack.push_back(CullingSet());

        _modelviewCullingStack[_index_modelviewCullingStack++].set(_projectionCullingStack.back());
    }
    else 
    {
    
        const osg::Viewport& W = *_viewportStack.back();
        const osg::Matrix& P = *_projectionStack.back();
        const osg::Matrix& M = *_modelviewStack.back();

        osg::Vec4 pixelSizeVector = CullingSet::computePixelSizeVector(W,P,M);
        
        if (_index_modelviewCullingStack>=_modelviewCullingStack.size()) 
        {
            _modelviewCullingStack.push_back(CullingSet());
        }
        
        _modelviewCullingStack[_index_modelviewCullingStack++].set(_projectionCullingStack.back(),*_modelviewStack.back(),pixelSizeVector);
        
    }
    
    _back_modelviewCullingStack = &_modelviewCullingStack[_index_modelviewCullingStack-1];

//     const osg::Polytope& polytope = _modelviewCullingStack.back()->getFrustum();
//     const osg::Polytope::PlaneList& pl = polytope.getPlaneList();
//     std::cout <<"new cull stack"<<std::endl;
//     for(osg::Polytope::PlaneList::const_iterator pl_itr=pl.begin();
//         pl_itr!=pl.end();
//         ++pl_itr)
//     {
//         std::cout << "    plane "<<*pl_itr<<std::endl;
//     }


}

void CullStack::popCullingSet()
{
    _MVPW_Stack.pop_back();
    
    --_index_modelviewCullingStack;
    if (_index_modelviewCullingStack>0) _back_modelviewCullingStack = &_modelviewCullingStack[_index_modelviewCullingStack-1];

}

void CullStack::pushViewport(osg::Viewport* viewport)
{
    _viewportStack.push_back(viewport);
    _MVPW_Stack.push_back(0L);
}

void CullStack::popViewport()
{
    _viewportStack.pop_back();
    _MVPW_Stack.pop_back();
}

void CullStack::pushProjectionMatrix(RefMatrix* matrix)
{
    _projectionStack.push_back(matrix);
    
    _projectionCullingStack.push_back(osg::CullingSet());
    osg::CullingSet& cullingSet = _projectionCullingStack.back();
    
    // set up view frustum.
    cullingSet.getFrustum().setToUnitFrustum(((_cullingMode&NEAR_PLANE_CULLING)!=0),((_cullingMode&FAR_PLANE_CULLING)!=0));
    cullingSet.getFrustum().transformProvidingInverse(*matrix);
    
    // set the culling mask ( There should be a more elegant way!)  Nikolaus H.
    cullingSet.setCullingMask(_cullingMode);

    // set the small feature culling.
    cullingSet.setSmallFeatureCullingPixelSize(_smallFeatureCullingPixelSize);
    
    // set up the relevant occluders which a related to this projection.
    for(ShadowVolumeOccluderList::iterator itr=_occluderList.begin();
        itr!=_occluderList.end();
        ++itr)
    {
        //std::cout << " ** testing occluder"<<std::endl;
        if (itr->matchProjectionMatrix(*matrix))
        {
            //std::cout << " ** activating occluder"<<std::endl;
            cullingSet.addOccluder(*itr);
        }
    }
    
    

    // need to recompute frustum volume.
    _frustumVolume = -1.0f;

    pushCullingSet();
}

void CullStack::popProjectionMatrix()
{

    _projectionStack.pop_back();

    _projectionCullingStack.pop_back();

    // need to recompute frustum volume.
    _frustumVolume = -1.0f;

    popCullingSet();
}

void CullStack::pushModelViewMatrix(RefMatrix* matrix, Transform::ReferenceFrame referenceFrame)
{
    osg::RefMatrix* originalModelView = _modelviewStack.empty() ? 0 : _modelviewStack.back().get();

    _modelviewStack.push_back(matrix);
    
    pushCullingSet();
    
    osg::Matrix inv;
    inv.invert(*matrix);

    
    switch(referenceFrame)
    {
        case(Transform::RELATIVE_RF):
            _eyePointStack.push_back(inv.getTrans());
            _referenceViewPoints.push_back(getReferenceViewPoint());
            _viewPointStack.push_back(getReferenceViewPoint() * inv);
            break;
        case(Transform::ABSOLUTE_RF):
            _eyePointStack.push_back(inv.getTrans());
            _referenceViewPoints.push_back(osg::Vec3(0.0,0.0,0.0));
            _viewPointStack.push_back(_eyePointStack.back());
            break;
        case(Transform::ABSOLUTE_RF_INHERIT_VIEWPOINT):
        {
            _eyePointStack.push_back(inv.getTrans());
            
            osg::Vec3 referenceViewPoint = getReferenceViewPoint();
            if (originalModelView)
            {
                osg::Matrix viewPointTransformMatrix;
                viewPointTransformMatrix.invert(*originalModelView);
                viewPointTransformMatrix.postMult(*matrix);
                referenceViewPoint = referenceViewPoint * viewPointTransformMatrix;
            }

            _referenceViewPoints.push_back(referenceViewPoint);
            _viewPointStack.push_back(getReferenceViewPoint() * inv);
            break;
        }
    }


    osg::Vec3 lookVector = getLookVectorLocal();                   
    
    _bbCornerFar = (lookVector.x()>=0?1:0) |
                   (lookVector.y()>=0?2:0) |
                   (lookVector.z()>=0?4:0);

    _bbCornerNear = (~_bbCornerFar)&7;
                                       
}

void CullStack::popModelViewMatrix()
{
    _modelviewStack.pop_back();
    
    _eyePointStack.pop_back();
    _referenceViewPoints.pop_back();
    _viewPointStack.pop_back();

    popCullingSet();


    osg::Vec3 lookVector(0.0f,0.0f,-1.0f);
    if (!_modelviewStack.empty())
    {
        lookVector = getLookVectorLocal();
    }
    _bbCornerFar = (lookVector.x()>=0?1:0) |
                   (lookVector.y()>=0?2:0) |
                   (lookVector.z()>=0?4:0);

    _bbCornerNear = (~_bbCornerFar)&7;
}

void CullStack::computeFrustumVolume()
{
    osg::Matrix invP;
    invP.invert(*getProjectionMatrix());

    osg::Vec3 f1(-1,-1,-1); f1 = f1*invP;
    osg::Vec3 f2(-1, 1,-1); f2 = f2*invP;
    osg::Vec3 f3( 1, 1,-1); f3 = f3*invP;
    osg::Vec3 f4( 1,-1,-1); f4 = f4*invP;

    osg::Vec3 b1(-1,-1,1); b1 = b1*invP;
    osg::Vec3 b2(-1, 1,1); b2 = b2*invP;
    osg::Vec3 b3( 1, 1,1); b3 = b3*invP;
    osg::Vec3 b4( 1,-1,1); b4 = b4*invP;

    _frustumVolume = computeVolume(f1,f2,f3,b1,b2,b3)+
                     computeVolume(f2,f3,f4,b1,b3,b4);
        
}
