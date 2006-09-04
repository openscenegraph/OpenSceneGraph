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
#include <osg/CameraNode>
#include <osg/Notify>

using namespace osg;

CameraNode::CameraNode():
    _clearColor(osg::Vec4(0.0f,0.0f,0.0f,1.0f)),
    _clearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT),
    _transformOrder(PRE_MULTIPLY),
    _renderOrder(POST_RENDER),
    _renderOrderNum(0),
    _drawBuffer(GL_NONE),
    _readBuffer(GL_NONE),
    _renderTargetImplementation(FRAME_BUFFER),
    _renderTargetFallback(FRAME_BUFFER)
{
    setStateSet(new StateSet);
}

CameraNode::CameraNode(const CameraNode& camera,const CopyOp& copyop):
    Transform(camera,copyop),
    CullSettings(camera),
    _clearColor(camera._clearColor),
    _clearMask(camera._clearMask),
    _colorMask(camera._colorMask),
    _viewport(camera._viewport),
    _transformOrder(camera._transformOrder),
    _projectionMatrix(camera._projectionMatrix),
    _viewMatrix(camera._viewMatrix),
    _renderOrder(camera._renderOrder),
    _renderOrderNum(camera._renderOrderNum),
    _drawBuffer(camera._drawBuffer),
    _readBuffer(camera._readBuffer),
    _renderTargetImplementation(camera._renderTargetImplementation),
    _renderTargetFallback(camera._renderTargetFallback),
    _bufferAttachmentMap(camera._bufferAttachmentMap),
    _postDrawCallback(camera._postDrawCallback)
{
}


CameraNode::~CameraNode()
{
}

bool CameraNode::isRenderToTextureCamera() const
{
    return (!_bufferAttachmentMap.empty());
}

void CameraNode::setRenderTargetImplementation(RenderTargetImplementation impl)
{
    _renderTargetImplementation = impl;
    if (impl<FRAME_BUFFER) _renderTargetFallback = (RenderTargetImplementation)(impl+1);
    else _renderTargetFallback = impl;
}   

void CameraNode::setRenderTargetImplementation(RenderTargetImplementation impl, RenderTargetImplementation fallback)
{
    if (impl<fallback || (impl==FRAME_BUFFER && fallback==FRAME_BUFFER))
    {
        _renderTargetImplementation = impl;
        _renderTargetFallback = fallback;
    }
    else 
    {
        osg::notify(osg::NOTICE)<<"Warning: CameraNode::setRenderTargetImplementation(impl,fallback) must have a lower rated fallback than the main target implementation."<<std::endl;
        setRenderTargetImplementation(impl);
    }
}

void CameraNode::setColorMask(osg::ColorMask* colorMask)
{
    if (_colorMask == colorMask) return;

    osg::StateSet* stateset = getOrCreateStateSet();
    if (_colorMask.valid() && stateset)
    {
        stateset->removeAttribute(_colorMask.get());
    }
    
    _colorMask = colorMask;
    
    if (_colorMask.valid() && stateset)
    {
        stateset->setAttribute(_colorMask.get());
    }
}

void CameraNode::setColorMask(bool red, bool green, bool blue, bool alpha)
{
    if (!_colorMask) setColorMask(new osg::ColorMask);
    if (_colorMask.valid()) _colorMask->setMask(red,green,blue,alpha);
}

void CameraNode::setViewport(osg::Viewport* viewport)
{
    if (_viewport == viewport) return;

    osg::StateSet* stateset = getOrCreateStateSet();
    if (_viewport.valid() && stateset)
    {
        stateset->removeAttribute(_viewport.get());
    }
    
    _viewport = viewport;
    
    if (_viewport.valid() && stateset)
    {
        stateset->setAttribute(_viewport.get());
    }
}

void CameraNode::setViewport(int x,int y,int width,int height)
{
    if (!_viewport) setViewport(new osg::Viewport);
    if (_viewport.valid()) _viewport->setViewport(x,y,width,height);
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


void CameraNode::attach(BufferComponent buffer, GLenum internalFormat)
{
    _bufferAttachmentMap[buffer]._internalFormat = internalFormat;
}

void CameraNode::attach(BufferComponent buffer, osg::Texture* texture, unsigned int level, unsigned int face, bool mipMapGeneration)
{
    _bufferAttachmentMap[buffer]._texture = texture;
    _bufferAttachmentMap[buffer]._level = level;
    _bufferAttachmentMap[buffer]._face = face;
    _bufferAttachmentMap[buffer]._mipMapGeneration = mipMapGeneration;
}

void CameraNode::attach(BufferComponent buffer, osg::Image* image)
{
    _bufferAttachmentMap[buffer]._image = image;
}

void CameraNode::detach(BufferComponent buffer)
{
    _bufferAttachmentMap.erase(buffer);
}

void CameraNode::releaseGLObjects(osg::State* state) const
{
    if (state) const_cast<CameraNode*>(this)->_renderingCache[state->getContextID()] = 0;
    else const_cast<CameraNode*>(this)->_renderingCache.setAllElementsTo(0);
    
    Transform::releaseGLObjects(state);
}


bool CameraNode::computeLocalToWorldMatrix(Matrix& matrix,NodeVisitor*) const
{
    if (_referenceFrame==RELATIVE_RF)
    {
        if (_transformOrder==PRE_MULTIPLY)
        {
            matrix.preMult(_viewMatrix);
        }
        else
        {
            matrix.postMult(_viewMatrix);
        }
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
        if (_transformOrder==PRE_MULTIPLY)
        {
            // note doing inverse so pre becomes post.
            matrix.postMult(inverse);
        }
        else
        {
            // note doing inverse so post becomes pre.
            matrix.preMult(inverse);
        }
    }
    else // absolute
    {
        matrix = inverse;
    }
    return true;
}

