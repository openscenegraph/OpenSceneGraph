#include <osg/CullStack>

using namespace osg;

CullStack::CullStack()
{

    _cullingMode = ENABLE_ALL_CULLING;
    _LODBias = 1.0f;
    _smallFeatureCullingPixelSize = 3.0f;

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
        float P00 = P(0,0)*W.width()*0.5f;
        float P20 = P(2,0)*W.width()*0.5f + P(2,3)*W.width()*0.5f;
        osg::Vec3 scale(M(0,0)*P00 + M(0,2)*P20,
                        M(1,0)*P00 + M(1,2)*P20,
                        M(2,0)*P00 + M(2,2)*P20);
                        
        float P23 = P(2,3);
        float P33 = P(3,3);
        osg::Vec4 pixelSizeVector2(M(0,2)*P23,
                                  M(1,2)*P23,
                                  M(2,2)*P23,
                                  M(3,2)*P23 + M(3,3)*P33);
                                  
        pixelSizeVector2 /= scale.length();
        
        //cout << "pixelSizeVector = "<<pixelSizeVector<<" pixelSizeVector2="<<pixelSizeVector2<<endl;
        
        _modelviewCullingStack.push_back(osgNew osg::CullingSet(*_projectionCullingStack.back(),*_modelviewStack.back(),pixelSizeVector2));
    }
    
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
    cullingSet->getFrustum().setToUnitFrustumWithoutNearFar();
    cullingSet->getFrustum().transformProvidingInverse(*matrix);
    cullingSet->setSmallFeatureCullingPixelSize(_smallFeatureCullingPixelSize);
    
    _projectionCullingStack.push_back(cullingSet);
    
    //_projectionCullingStack.push_back(osgNew osg::CullingSet(*_clipspaceCullingStack.back(),*matrix));


    pushCullingSet();
}

void CullStack::popProjectionMatrix()
{

    _projectionStack.pop_back();

    _projectionCullingStack.pop_back();

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
