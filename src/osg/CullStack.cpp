#include <osg/CullStack>

using namespace osg;

CullStack::CullStack()
{
    _cullingMode = ENABLE_ALL_CULLING;
    _LODScale = 1.0f;
    _smallFeatureCullingPixelSize = 2.0f;
    _frustumVolume=-1.0f;
    _bbCornerNear = 0;
    _bbCornerFar = 7;
   _currentReuseMatrixIndex=0;
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
    _eyePointStack.clear();


    _clipspaceCullingStack.clear();
    _projectionCullingStack.clear();
    _modelviewCullingStack.clear();
    
    osg::Vec3 lookVector(0.0,0.0,-1.0);
    
    _bbCornerFar = (lookVector.x()>=0?1:0) |
                   (lookVector.y()>=0?2:0) |
                   (lookVector.z()>=0?4:0);

    _bbCornerNear = (~_bbCornerFar)&7;
}

void CullStack::pushCullingSet()
{
    _MVPW_Stack.push_back(0L);
    
    if (_modelviewStack.empty()) 
    {
        _modelviewCullingStack.push_back(_projectionCullingStack.back());

    }
    else 
    {
    
        const osg::Viewport& W = *_viewportStack.back();
        const osg::Matrix& P = *_projectionStack.back();
        const osg::Matrix& M = *_modelviewStack.back();
        
        // pre adjust P00,P20,P23,P33 by multiplying them by the viewport window matrix.
        // here we do it in short hand with the knowledge of how the window matrix is formed
        // note P23,P33 are multiplied by an implicit 1 which would come from the window matrix.
        // Robert Osfield, June 2002.

        // scaling for horizontal pixels
        float P00 = P(0,0)*W.width()*0.5f;
        float P20_00 = P(2,0)*W.width()*0.5f + P(2,3)*W.width()*0.5f;
        osg::Vec3 scale_00(M(0,0)*P00 + M(0,2)*P20_00,
                           M(1,0)*P00 + M(1,2)*P20_00,
                           M(2,0)*P00 + M(2,2)*P20_00);
                           
        // scaling for vertical pixels
        float P10 = P(1,1)*W.height()*0.5f;
        float P20_10 = P(2,1)*W.height()*0.5f + P(2,3)*W.height()*0.5f;
        osg::Vec3 scale_10(M(0,1)*P10 + M(0,2)*P20_10,
                           M(1,1)*P10 + M(1,2)*P20_10,
                           M(2,1)*P10 + M(2,2)*P20_10);
        
        float P23 = P(2,3);
        float P33 = P(3,3);
        osg::Vec4 pixelSizeVector(M(0,2)*P23,
                                     M(1,2)*P23,
                                     M(2,2)*P23,
                                     M(3,2)*P23 + M(3,3)*P33);
                                  
        float scaleRatio  = 1.0f/sqrtf(scale_00.length2()+scale_10.length2());

        pixelSizeVector *= scaleRatio;
        
        _modelviewCullingStack.push_back(osgNew osg::CullingSet(*_projectionCullingStack.back(),*_modelviewStack.back(),pixelSizeVector));
        
    }
    
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
    
    _modelviewCullingStack.pop_back();
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

void CullStack::pushProjectionMatrix(Matrix* matrix)
{
    _projectionStack.push_back(matrix);
    
    osg::CullingSet* cullingSet = osgNew osg::CullingSet();
    
    // set up view frustum.
    cullingSet->getFrustum().setToUnitFrustum(((_cullingMode&NEAR_PLANE_CULLING)!=0),((_cullingMode&FAR_PLANE_CULLING)!=0));
    cullingSet->getFrustum().transformProvidingInverse(*matrix);
    
    // set the culling mask ( There should be a more elegant way!)  Nikolaus H.
    osg::CullingSet::Mask mask = 0;
    if( _cullingMode&VIEW_FRUSTUM_CULLING) mask |= osg::CullingSet::VIEW_FRUSTUM_CULLING;
    if( _cullingMode&SMALL_FEATURE_CULLING) mask |= osg::CullingSet::SMALL_FEATURE_CULLING;
    if( _cullingMode&SHADOW_OCCLUSION_CULLING) mask |= osg::CullingSet::SHADOW_OCCLUSION_CULLING;
    cullingSet->setCullingMask(mask);

    // set the small feature culling.
    cullingSet->setSmallFeatureCullingPixelSize(_smallFeatureCullingPixelSize);
    
    // set up the relevant occluders which a related to this projection.
    for(ShadowVolumeOccluderList::iterator itr=_occluderList.begin();
        itr!=_occluderList.end();
        ++itr)
    {
        //std::cout << " ** testing occluder"<<std::endl;
        if (itr->matchProjectionMatrix(*matrix))
        {
            //std::cout << " ** activating occluder"<<std::endl;
            cullingSet->addOccluder(*itr);
        }
    }
    
    
    _projectionCullingStack.push_back(cullingSet);

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

void CullStack::pushModelViewMatrix(Matrix* matrix)
{
    _modelviewStack.push_back(matrix);
    
    pushCullingSet();

    // fast method for computing the eye point in local coords which doesn't require the inverse matrix.
    const float x_0 = (*matrix)(0,0);
    const float x_1 = (*matrix)(1,0);
    const float x_2 = (*matrix)(2,0);
    const float x_scale = (*matrix)(3,0) / -(x_0*x_0+x_1*x_1+x_2*x_2);

    const float y_0 = (*matrix)(0,1);
    const float y_1 = (*matrix)(1,1);
    const float y_2 = (*matrix)(2,1);
    const float y_scale = (*matrix)(3,1) / -(y_0*y_0+y_1*y_1+y_2*y_2);

    const float z_0 = (*matrix)(0,2);
    const float z_1 = (*matrix)(1,2);
    const float z_2 = (*matrix)(2,2);
    const float z_scale = (*matrix)(3,2) / -(z_0*z_0+z_1*z_1+z_2*z_2);
    
    _eyePointStack.push_back(osg::Vec3(x_0*x_scale + y_0*y_scale + z_0*z_scale,
                                       x_1*x_scale + y_1*y_scale + z_1*z_scale,
                                       x_2*x_scale + y_2*y_scale + z_2*z_scale));
                                       
                    
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
    invP.invert(getProjectionMatrix());

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
