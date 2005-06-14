/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield 
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
#include <osg/CameraNode>

using namespace osg;



CameraNode::CameraNode():
    _clearColor(osg::Vec4(0.0f,0.0f,0.0f,1.0f)),
    _clearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
{
}

CameraNode::CameraNode(const CameraNode& camera,const CopyOp& copyop):
    Transform(camera,copyop),
    _clearColor(camera._clearColor),
    _clearMask(camera._clearMask),
    _viewport(camera._viewport),
    _projectionMatrix(camera._projectionMatrix),
    _viewMatrix(camera._viewMatrix)
{    
}


CameraNode::~CameraNode()
{
}

Matrixd CameraNode::getInverseViewMatrix() const
{
    Matrixd inverse;
    inverse.invert(_viewMatrix);
    return inverse;
}
void CameraNode::setProjectionMatrixAsOrtho(double left, double right,
                                           double bottom, double top,
                                           double zNear, double zFar)
{
    setProjectionMatrix(osg::Matrixd::ortho(left, right,
                                           bottom, top,
                                           zNear, zFar));
}                                           

void CameraNode::setProjectionMatrixAsOrtho2D(double left, double right,
                                             double bottom, double top)
{
    setProjectionMatrix(osg::Matrixd::ortho2D(left, right,
                                             bottom, top));
}

void CameraNode::setProjectionMatrixAsFrustum(double left, double right,
                                             double bottom, double top,
                                             double zNear, double zFar)
{
    setProjectionMatrix(osg::Matrixd::frustum(left, right,
                                             bottom, top,
                                             zNear, zFar));
}

void CameraNode::setProjectionMatrixAsPerspective(double fovy,double aspectRatio,
                                                 double zNear, double zFar)
{
    setProjectionMatrix(osg::Matrixd::perspective(fovy,aspectRatio,
                                                 zNear, zFar));
}                                      

bool CameraNode::getProjectionMatrixAsOrtho(double& left, double& right,
                                           double& bottom, double& top,
                                           double& zNear, double& zFar)
{
    return _projectionMatrix.getOrtho(left, right,
                                       bottom, top,
                                       zNear, zFar);
}

bool CameraNode::getProjectionMatrixAsFrustum(double& left, double& right,
                                             double& bottom, double& top,
                                             double& zNear, double& zFar)
{
    return _projectionMatrix.getFrustum(left, right,
                                         bottom, top,
                                         zNear, zFar);
}                                  

bool CameraNode::getProjectionMatrixAsPerspective(double& fovy,double& aspectRatio,
                                                 double& zNear, double& zFar)
{
    return _projectionMatrix.getPerspective(fovy, aspectRatio, zNear, zFar);
}                                                 

void CameraNode::setViewMatrixAsLookAt(const Vec3& eye,const Vec3& center,const Vec3& up)
{
    setViewMatrix(osg::Matrixd::lookAt(eye,center,up));
}

void CameraNode::getViewMatrixAsLookAt(Vec3& eye,Vec3& center,Vec3& up,float lookDistance)
{
    _viewMatrix.getLookAt(eye,center,up,lookDistance);
}

bool CameraNode::computeLocalToWorldMatrix(Matrix& matrix,NodeVisitor*) const
{
    if (_referenceFrame==RELATIVE_RF)
    {
        matrix.preMult(_viewMatrix);
    }
    else // absolute
    {
        matrix = _viewMatrix;
    }
    return true;
}

bool CameraNode::computeWorldToLocalMatrix(Matrix& matrix,NodeVisitor*) const
{
    const Matrixd& inverse = getInverseViewMatrix();

    if (_referenceFrame==RELATIVE_RF)
    {
        matrix.postMult(inverse);
    }
    else // absolute
    {
        matrix = inverse;
    }
    return true;
}

