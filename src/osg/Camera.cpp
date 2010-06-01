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
#include <osg/RenderInfo>
#include <osg/Notify>

using namespace osg;

const unsigned int Camera::FACE_CONTROLLED_BY_GEOMETRY_SHADER = 0xffffffff;

Camera::Camera():
    _view(0),
    _allowEventFocus(true),
    _clearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT),
    _clearColor(osg::Vec4(0.0f,0.0f,0.0f,1.0f)),
    _clearAccum(osg::Vec4(0.0f,0.0f,0.0f,1.0f)),
    _clearDepth(1.0),
    _clearStencil(0),
    _transformOrder(PRE_MULTIPLY),
    _projectionResizePolicy(HORIZONTAL),
    _renderOrder(POST_RENDER),
    _renderOrderNum(0),
    _drawBuffer(GL_NONE),
    _readBuffer(GL_NONE),
    _renderTargetImplementation(FRAME_BUFFER),
    _renderTargetFallback(FRAME_BUFFER),
    _implicitBufferAttachmentRenderMask( USE_DISPLAY_SETTINGS_MASK ),
    _implicitBufferAttachmentResolveMask( USE_DISPLAY_SETTINGS_MASK )
{
    setStateSet(new StateSet);
}

Camera::Camera(const Camera& camera,const CopyOp& copyop):
    Transform(camera,copyop),
    CullSettings(camera),
    _view(camera._view),
    _allowEventFocus(camera._allowEventFocus),
    _displaySettings(camera._displaySettings),
    _clearMask(camera._clearMask),
    _clearColor(camera._clearColor),
    _clearAccum(camera._clearAccum),
    _clearDepth(camera._clearDepth),
    _clearStencil(camera._clearStencil),
    _colorMask(camera._colorMask),
    _viewport(camera._viewport),
    _transformOrder(camera._transformOrder),
    _projectionResizePolicy(camera._projectionResizePolicy),
    _projectionMatrix(camera._projectionMatrix),
    _viewMatrix(camera._viewMatrix),
    _renderOrder(camera._renderOrder),
    _renderOrderNum(camera._renderOrderNum),
    _drawBuffer(camera._drawBuffer),
    _readBuffer(camera._readBuffer),
    _renderTargetImplementation(camera._renderTargetImplementation),
    _renderTargetFallback(camera._renderTargetFallback),
    _bufferAttachmentMap(camera._bufferAttachmentMap),
    _implicitBufferAttachmentRenderMask(camera._implicitBufferAttachmentRenderMask),
    _implicitBufferAttachmentResolveMask(camera._implicitBufferAttachmentResolveMask),
    _initialDrawCallback(camera._initialDrawCallback),
    _preDrawCallback(camera._preDrawCallback),
    _postDrawCallback(camera._postDrawCallback),
    _finalDrawCallback(camera._finalDrawCallback)
{
    // need to copy/share graphics context?
}


Camera::~Camera()
{
    setCameraThread(0);

    if (_graphicsContext.valid()) _graphicsContext->removeCamera(this);
}

void Camera::DrawCallback::operator () (osg::RenderInfo& renderInfo) const
{
    if (renderInfo.getCurrentCamera())
    {
        operator()(*(renderInfo.getCurrentCamera()));
    }
    else
    {
        OSG_WARN<<"Error: Camera::DrawCallback called without valid camera."<<std::endl;
    }
}


void Camera::setGraphicsContext(GraphicsContext* context)
{
    if (_graphicsContext == context) return;

    if (_graphicsContext.valid()) _graphicsContext->removeCamera(this);

    _graphicsContext = context;

    if (_graphicsContext.valid()) _graphicsContext->addCamera(this);
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
        OSG_NOTICE<<"Warning: Camera::setRenderTargetImplementation(impl,fallback) must have a lower rated fallback than the main target implementation."<<std::endl;
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
                                           double& zNear, double& zFar) const
{
    return _projectionMatrix.getOrtho(left, right,
                                       bottom, top,
                                       zNear, zFar);
}

bool Camera::getProjectionMatrixAsFrustum(double& left, double& right,
                                             double& bottom, double& top,
                                             double& zNear, double& zFar) const
{
    return _projectionMatrix.getFrustum(left, right,
                                         bottom, top,
                                         zNear, zFar);
}

bool Camera::getProjectionMatrixAsPerspective(double& fovy,double& aspectRatio,
                                                 double& zNear, double& zFar) const
{
    return _projectionMatrix.getPerspective(fovy, aspectRatio, zNear, zFar);
}

void Camera::setViewMatrixAsLookAt(const Vec3d& eye,const Vec3d& center,const Vec3d& up)
{
    setViewMatrix(osg::Matrixd::lookAt(eye,center,up));
}

void Camera::getViewMatrixAsLookAt(Vec3d& eye,Vec3d& center,Vec3d& up,double lookDistance) const
{
    _viewMatrix.getLookAt(eye,center,up,lookDistance);
}

void Camera::getViewMatrixAsLookAt(Vec3f& eye,Vec3f& center,Vec3f& up,float lookDistance) const
{
    _viewMatrix.getLookAt(eye,center,up,lookDistance);
}


void Camera::attach(BufferComponent buffer, GLenum internalFormat)
{
    switch(buffer)
    {
    case DEPTH_BUFFER:
        if(_bufferAttachmentMap.find(PACKED_DEPTH_STENCIL_BUFFER) != _bufferAttachmentMap.end())
        {
            OSG_WARN << "Camera: DEPTH_BUFFER already attached as PACKED_DEPTH_STENCIL_BUFFER !" << std::endl;
        }
        break;

    case STENCIL_BUFFER:
        if(_bufferAttachmentMap.find(PACKED_DEPTH_STENCIL_BUFFER) != _bufferAttachmentMap.end())
        {
            OSG_WARN << "Camera: STENCIL_BUFFER already attached as PACKED_DEPTH_STENCIL_BUFFER !" << std::endl;
        }
        break;

    case PACKED_DEPTH_STENCIL_BUFFER:
        if(_bufferAttachmentMap.find(DEPTH_BUFFER) != _bufferAttachmentMap.end())
        {
            OSG_WARN << "Camera: DEPTH_BUFFER already attached !" << std::endl;
        }
        if(_bufferAttachmentMap.find(STENCIL_BUFFER) != _bufferAttachmentMap.end())
        {
            OSG_WARN << "Camera: STENCIL_BUFFER already attached !" << std::endl;
        }
        break;
    default:
        break;
    }
    _bufferAttachmentMap[buffer]._internalFormat = internalFormat;
}

void Camera::attach(BufferComponent buffer, osg::Texture* texture, unsigned int level, unsigned int face, bool mipMapGeneration,
                    unsigned int multisampleSamples,
                    unsigned int multisampleColorSamples)
{
    _bufferAttachmentMap[buffer]._texture = texture;
    _bufferAttachmentMap[buffer]._level = level;
    _bufferAttachmentMap[buffer]._face = face;
    _bufferAttachmentMap[buffer]._mipMapGeneration = mipMapGeneration;
    _bufferAttachmentMap[buffer]._multisampleSamples = multisampleSamples;
    _bufferAttachmentMap[buffer]._multisampleColorSamples = multisampleColorSamples;
}

void Camera::attach(BufferComponent buffer, osg::Image* image,
                    unsigned int multisampleSamples,
                    unsigned int multisampleColorSamples)
{
    _bufferAttachmentMap[buffer]._image = image;
    _bufferAttachmentMap[buffer]._multisampleSamples = multisampleSamples;
    _bufferAttachmentMap[buffer]._multisampleColorSamples = multisampleColorSamples;
}

void Camera::detach(BufferComponent buffer)
{
    _bufferAttachmentMap.erase(buffer);
}

void Camera::resizeGLObjectBuffers(unsigned int maxSize)
{
    if (_renderingCache.valid())
    {
        const_cast<Camera*>(this)->_renderingCache->resizeGLObjectBuffers(maxSize);
    }

    Transform::resizeGLObjectBuffers(maxSize);
}

void Camera::releaseGLObjects(osg::State* state) const
{
    if (_renderingCache.valid())
    {
        const_cast<Camera*>(this)->_renderingCache->releaseGLObjects(state);
    }

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

void Camera::inheritCullSettings(const CullSettings& settings, unsigned int inheritanceMask)
{
    CullSettings::inheritCullSettings(settings, inheritanceMask);

    const Camera* camera = dynamic_cast<const Camera*>(&settings);
    if (camera)
    {
        //OSG_NOTICE<<"Inheriting slave Camera"<<std::endl;
        if (inheritanceMask & CLEAR_COLOR)
            _clearColor = camera->_clearColor;

        if (inheritanceMask & CLEAR_MASK)
            _clearMask = camera->_clearMask;

        if (inheritanceMask & DRAW_BUFFER)
            _drawBuffer = camera->_drawBuffer;

        if (inheritanceMask & READ_BUFFER)
            _drawBuffer = camera->_readBuffer;
    }
}

void Camera::createCameraThread()
{
    if (!_cameraThread)
    {
        setCameraThread(new OperationThread);
    }
}

void Camera::setCameraThread(OperationThread* gt)
{
    if (_cameraThread==gt) return;

    if (_cameraThread.valid())
    {
        // need to kill the thread in some way...
        _cameraThread->cancel();
        _cameraThread->setParent(0);
    }

    _cameraThread = gt;

    if (_cameraThread.valid())
    {
        _cameraThread->setParent(this);
    }
}


