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
#include <osg/Camera>
#include <osg/Notify>

using namespace osg;

Camera::Camera():
    _view(0),
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

Camera::Camera(const Camera& camera,const CopyOp& copyop):
    Transform(camera,copyop),
    CullSettings(camera),
    _view(camera._view),
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


Camera::~Camera()
{
}

bool Camera::isRenderToTextureCamera() const
{
    return (!_bufferAttachmentMap.empty());
}

void Camera::setRenderTargetImplementation(RenderTargetImplementation impl)
{
    _renderTargetImplementation = impl;
    if (impl<FRAME_BUFFER) _renderTargetFallback = (RenderTargetImplementation)(impl+1);
    else _renderTargetFallback = impl;
}   

void Camera::setRenderTargetImplementation(RenderTargetImplementation impl, RenderTargetImplementation fallback)
{
    if (impl<fallback || (impl==FRAME_BUFFER && fallback==FRAME_BUFFER))
    {
        _renderTargetImplementation = impl;
        _renderTargetFallback = fallback;
    }
    else 
    {
        osg::notify(osg::NOTICE)<<"Warning: Camera::setRenderTargetImplementation(impl,fallback) must have a lower rated fallback than the main target implementation."<<std::endl;
        setRenderTargetImplementation(impl);
    }
}

void Camera::setColorMask(osg::ColorMask* colorMask)
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

void Camera::setColorMask(bool red, bool green, bool blue, bool alpha)
{
    if (!_colorMask) setColorMask(new osg::ColorMask);
    if (_colorMask.valid()) _colorMask->setMask(red,green,blue,alpha);
}

void Camera::setViewport(osg::Viewport* viewport)
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

void Camera::setViewport(int x,int y,int width,int height)
{
    if (!_viewport) setViewport(new osg::Viewport);
    if (_viewport.valid()) _viewport->setViewport(x,y,width,height);
}

Matrixd Camera::getInverseViewMatrix() const
{
    Matrixd inverse;
    inverse.invert(_viewMatrix);
    return inverse;
}
void Camera::setProjectionMatrixAsOrtho(double left, double right,
                                           double bottom, double top,
                                           double zNear, double zFar)
{
    setProjectionMatrix(osg::Matrixd::ortho(left, right,
                                           bottom, top,
                                           zNear, zFar));
}                                           

void Camera::setProjectionMatrixAsOrtho2D(double left, double right,
                                             double bottom, double top)
{
    setProjectionMatrix(osg::Matrixd::ortho2D(left, right,
                                             bottom, top));
}

void Camera::setProjectionMatrixAsFrustum(double left, double right,
                                             double bottom, double top,
                                             double zNear, double zFar)
{
    setProjectionMatrix(osg::Matrixd::frustum(left, right,
                                             bottom, top,
                                             zNear, zFar));
}

void Camera::setProjectionMatrixAsPerspective(double fovy,double aspectRatio,
                                                 double zNear, double zFar)
{
    setProjectionMatrix(osg::Matrixd::perspective(fovy,aspectRatio,
                                                 zNear, zFar));
}                                      

bool Camera::getProjectionMatrixAsOrtho(double& left, double& right,
                                           double& bottom, double& top,
                                           double& zNear, double& zFar)
{
    return _projectionMatrix.getOrtho(left, right,
                                       bottom, top,
                                       zNear, zFar);
}

bool Camera::getProjectionMatrixAsFrustum(double& left, double& right,
                                             double& bottom, double& top,
                                             double& zNear, double& zFar)
{
    return _projectionMatrix.getFrustum(left, right,
                                         bottom, top,
                                         zNear, zFar);
}                                  

bool Camera::getProjectionMatrixAsPerspective(double& fovy,double& aspectRatio,
                                                 double& zNear, double& zFar)
{
    return _projectionMatrix.getPerspective(fovy, aspectRatio, zNear, zFar);
}                                                 

void Camera::setViewMatrixAsLookAt(const Vec3& eye,const Vec3& center,const Vec3& up)
{
    setViewMatrix(osg::Matrixd::lookAt(eye,center,up));
}

void Camera::getViewMatrixAsLookAt(Vec3& eye,Vec3& center,Vec3& up,float lookDistance)
{
    _viewMatrix.getLookAt(eye,center,up,lookDistance);
}


void Camera::attach(BufferComponent buffer, GLenum internalFormat)
{
    _bufferAttachmentMap[buffer]._internalFormat = internalFormat;
}

void Camera::attach(BufferComponent buffer, osg::Texture* texture, unsigned int level, unsigned int face, bool mipMapGeneration)
{
    _bufferAttachmentMap[buffer]._texture = texture;
    _bufferAttachmentMap[buffer]._level = level;
    _bufferAttachmentMap[buffer]._face = face;
    _bufferAttachmentMap[buffer]._mipMapGeneration = mipMapGeneration;
}

void Camera::attach(BufferComponent buffer, osg::Image* image)
{
    _bufferAttachmentMap[buffer]._image = image;
}

void Camera::detach(BufferComponent buffer)
{
    _bufferAttachmentMap.erase(buffer);
}

void Camera::releaseGLObjects(osg::State* state) const
{
    if (state) const_cast<Camera*>(this)->_renderingCache[state->getContextID()] = 0;
    else const_cast<Camera*>(this)->_renderingCache.setAllElementsTo(0);
    
    Transform::releaseGLObjects(state);
}


bool Camera::computeLocalToWorldMatrix(Matrix& matrix,NodeVisitor*) const
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

bool Camera::computeWorldToLocalMatrix(Matrix& matrix,NodeVisitor*) const
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

