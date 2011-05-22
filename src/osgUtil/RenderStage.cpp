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
#include <stdio.h>

#include <osg/Notify>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture2DMultisample>
#include <osg/Texture3D>
#include <osg/TextureRectangle>
#include <osg/TextureCubeMap>
#include <osg/GLExtensions>
#include <osg/GLU>

#include <osgUtil/Statistics>

#include <osgUtil/RenderStage>


using namespace osg;
using namespace osgUtil;

// register a RenderStage prototype with the RenderBin prototype list.
//RegisterRenderBinProxy<RenderStage> s_registerRenderStageProxy;

RenderStage::RenderStage():
    RenderBin(getDefaultRenderBinSortMode()),
    _disableFboAfterRender(true)
{
    // point RenderBin's _stage to this to ensure that references to
    // stage don't go tempted away to any other stage.
    _stage = this;
    _stageDrawnThisFrame = false;

    _drawBuffer = GL_NONE;
    _readBuffer = GL_NONE;
    _clearMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    _clearColor.set(0.0f,0.0f,0.0f,0.0f);
    _clearAccum.set(0.0f,0.0f,0.0f,0.0f);
    _clearDepth = 1.0;
    _clearStencil = 0;

    _cameraRequiresSetUp = false;
    _camera = 0;
    
    _level = 0;
    _face = 0;
    
    _imageReadPixelFormat = GL_RGBA;
    _imageReadPixelDataType = GL_UNSIGNED_BYTE;
}

RenderStage::RenderStage(SortMode mode):
    RenderBin(mode),
    _disableFboAfterRender(true)
{
    // point RenderBin's _stage to this to ensure that references to
    // stage don't go tempted away to any other stage.
    _stage = this;
    _stageDrawnThisFrame = false;

    _drawBuffer = GL_NONE;
    _readBuffer = GL_NONE;
    _clearMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    _clearColor.set(0.0f,0.0f,0.0f,0.0f);
    _clearAccum.set(0.0f,0.0f,0.0f,0.0f);
    _clearDepth = 1.0;
    _clearStencil = 0;

    _cameraRequiresSetUp = false;
    _camera = 0;
    
    _level = 0;
    _face = 0;
    
    _imageReadPixelFormat = GL_RGBA;
    _imageReadPixelDataType = GL_UNSIGNED_BYTE;
}

RenderStage::RenderStage(const RenderStage& rhs,const osg::CopyOp& copyop):
        RenderBin(rhs,copyop),
        _stageDrawnThisFrame(false),
        _preRenderList(rhs._preRenderList),
        _postRenderList(rhs._postRenderList),
        _viewport(rhs._viewport),
        _drawBuffer(rhs._drawBuffer),
        _readBuffer(rhs._readBuffer),
        _clearMask(rhs._clearMask),
        _colorMask(rhs._colorMask),
        _clearColor(rhs._clearColor),
        _clearAccum(rhs._clearAccum),
        _clearDepth(rhs._clearDepth),
        _clearStencil(rhs._clearStencil),
        _cameraRequiresSetUp(rhs._cameraRequiresSetUp),
        _camera(rhs._camera),
        _level(rhs._level),
        _face(rhs._face),
        _imageReadPixelFormat(rhs._imageReadPixelFormat),
        _imageReadPixelDataType(rhs._imageReadPixelDataType),
        _disableFboAfterRender(rhs._disableFboAfterRender),
        _renderStageLighting(rhs._renderStageLighting)
{
    _stage = this;
}


RenderStage::~RenderStage()
{
}

void RenderStage::reset()
{
    _stageDrawnThisFrame = false;
    
    if (_renderStageLighting.valid()) _renderStageLighting->reset();

    for(RenderStageList::iterator pre_itr = _preRenderList.begin();
        pre_itr != _preRenderList.end();
        ++pre_itr)
    {
        pre_itr->second->reset();
    }

    RenderBin::reset();

    for(RenderStageList::iterator post_itr = _postRenderList.begin();
        post_itr != _postRenderList.end();
        ++post_itr)
    {
        post_itr->second->reset();
    }

    _preRenderList.clear();
    _postRenderList.clear();
}

void RenderStage::sort()
{
    for(RenderStageList::iterator pre_itr = _preRenderList.begin();
        pre_itr != _preRenderList.end();
        ++pre_itr)
    {
        pre_itr->second->sort();
    }

    RenderBin::sort();

    for(RenderStageList::iterator post_itr = _postRenderList.begin();
        post_itr != _postRenderList.end();
        ++post_itr)
    {
        post_itr->second->sort();
    }
}

void RenderStage::addPreRenderStage(RenderStage* rs, int order)
{
    if (rs)
    {
        RenderStageList::iterator itr;
        for(itr = _preRenderList.begin(); itr != _preRenderList.end(); ++itr) {
            if(order < itr->first) {
                break;
            }
        }
        if(itr == _preRenderList.end()) {
            _preRenderList.push_back(RenderStageOrderPair(order,rs));
        } else {
            _preRenderList.insert(itr,RenderStageOrderPair(order,rs));
        }
    }
}

void RenderStage::addPostRenderStage(RenderStage* rs, int order)
{
    if (rs)
    {
        RenderStageList::iterator itr;
        for(itr = _postRenderList.begin(); itr != _postRenderList.end(); ++itr) {
            if(order < itr->first) {
                break;
            }
        }
        if(itr == _postRenderList.end()) {
            _postRenderList.push_back(RenderStageOrderPair(order,rs));
        } else {
            _postRenderList.insert(itr,RenderStageOrderPair(order,rs));
        }
    }
}

void RenderStage::drawPreRenderStages(osg::RenderInfo& renderInfo,RenderLeaf*& previous)
{
    if (_preRenderList.empty()) return;
    
    //cout << "Drawing prerendering stages "<<this<< "  "<<_viewport->x()<<","<< _viewport->y()<<","<< _viewport->width()<<","<< _viewport->height()<<std::endl;
    for(RenderStageList::iterator itr=_preRenderList.begin();
        itr!=_preRenderList.end();
        ++itr)
    {
        itr->second->draw(renderInfo,previous);
    }
    //cout << "Done Drawing prerendering stages "<<this<< "  "<<_viewport->x()<<","<< _viewport->y()<<","<< _viewport->width()<<","<< _viewport->height()<<std::endl;
}


void RenderStage::runCameraSetUp(osg::RenderInfo& renderInfo)
{
    _cameraRequiresSetUp = false;

    if (!_camera) return;
    
    osg::State& state = *renderInfo.getState();

    osg::Camera::RenderTargetImplementation renderTargetImplementation = _camera->getRenderTargetImplementation();
    osg::Camera::RenderTargetImplementation renderTargetFallback = _camera->getRenderTargetFallback();

    osg::Camera::BufferAttachmentMap& bufferAttachments = _camera->getBufferAttachmentMap();

    // compute the required dimensions
    int width = static_cast<int>(_viewport->x() + _viewport->width());
    int height = static_cast<int>(_viewport->y() + _viewport->height());
    int depth = 1;
    osg::Camera::BufferAttachmentMap::iterator itr;
    for(itr = bufferAttachments.begin();
        itr != bufferAttachments.end();
        ++itr)
    {
        width = osg::maximum(width,itr->second.width());
        height = osg::maximum(height,itr->second.height());
        depth = osg::maximum(depth,itr->second.depth());
    }
    
    // osg::notify(osg::NOTICE)<<"RenderStage::runCameraSetUp viewport "<<_viewport->x()<<" "<<_viewport->y()<<" "<<_viewport->width()<<" "<<_viewport->height()<<std::endl;
    // osg::notify(osg::NOTICE)<<"RenderStage::runCameraSetUp computed "<<width<<" "<<height<<" "<<depth<<std::endl;

    // attach images that need to be copied after the stage is drawn.
    for(itr = bufferAttachments.begin();
        itr != bufferAttachments.end();
        ++itr)
    {
        // if one exist attach image to the RenderStage.
        if (itr->second._image.valid())
        {
            osg::Image* image = itr->second._image.get();
            GLenum pixelFormat = image->getPixelFormat();
            GLenum dataType = image->getDataType();

            if (image->data()==0)
            {
                if (pixelFormat==0) pixelFormat = itr->second._internalFormat;
                if (pixelFormat==0) pixelFormat = _imageReadPixelFormat;
                if (pixelFormat==0) pixelFormat = GL_RGBA;

                if (dataType==0) dataType = _imageReadPixelDataType;
                if (dataType==0) dataType = GL_UNSIGNED_BYTE;
            }

            _bufferAttachmentMap[itr->first]._imageReadPixelFormat = pixelFormat;
            _bufferAttachmentMap[itr->first]._imageReadPixelDataType = dataType;
            _bufferAttachmentMap[itr->first]._image = image;
        }
        
        if (itr->second._texture.valid())
        {
            osg::Texture* texture = itr->second._texture.get();
            osg::Texture1D* texture1D = 0;
            osg::Texture2D* texture2D = 0;
            osg::Texture2DMultisample* texture2DMS = 0;
            osg::Texture3D* texture3D = 0;
            osg::TextureCubeMap* textureCubeMap = 0;
            osg::TextureRectangle* textureRectangle = 0;
            if (0 != (texture1D=dynamic_cast<osg::Texture1D*>(texture)))
            {
                if (texture1D->getTextureWidth()==0)
                {
                    texture1D->setTextureWidth(width);
                }
            }
            else if (0 != (texture2D = dynamic_cast<osg::Texture2D*>(texture)))
            {
                if (texture2D->getTextureWidth()==0 || texture2D->getTextureHeight()==0)
                {
                    texture2D->setTextureSize(width,height);
                }
            }
            else if (0 != (texture2DMS = dynamic_cast<osg::Texture2DMultisample*>(texture)))
            {
                if (texture2DMS->getTextureWidth()==0 || texture2DMS->getTextureHeight()==0)
                {
                    texture2DMS->setTextureSize(width,height);
                }
            }
            else if (0 != (texture3D = dynamic_cast<osg::Texture3D*>(texture)))
            {
                if (texture3D->getTextureWidth()==0 || texture3D->getTextureHeight()==0 || texture3D->getTextureDepth()==0 )
                {
                    // note we dont' have the depth here, so we'll heave to assume that height and depth are the same..
                    texture3D->setTextureSize(width,height,height);
                }
            }
            else if (0 != (textureCubeMap = dynamic_cast<osg::TextureCubeMap*>(texture)))
            {
                if (textureCubeMap->getTextureWidth()==0 || textureCubeMap->getTextureHeight()==0)
                {
                    textureCubeMap->setTextureSize(width,height);
                }
            }
            else if (0 != (textureRectangle = dynamic_cast<osg::TextureRectangle*>(texture)))
            {
                if (textureRectangle->getTextureWidth()==0 || textureRectangle->getTextureHeight()==0)
                {
                    textureRectangle->setTextureSize(width,height);
                }
            }

        }
    }
    
    if (renderTargetImplementation==osg::Camera::FRAME_BUFFER_OBJECT)
    {
        osg::FBOExtensions* fbo_ext = osg::FBOExtensions::instance(state.getContextID(),true);
        bool fbo_supported = fbo_ext && fbo_ext->isSupported();
        
        if (fbo_supported && !_fbo)
        {
            osg::notify(osg::INFO)<<"Setting up osg::Camera::FRAME_BUFFER_OBJECT"<<std::endl;

            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*(_camera->getDataChangeMutex()));

            osg::ref_ptr<osg::FrameBufferObject> fbo = new osg::FrameBufferObject;
            osg::ref_ptr<osg::FrameBufferObject> fbo_multisample;
            
            bool colorAttached = false;
            bool depthAttached = false;
            bool stencilAttached = false;
            unsigned samples = 0;
            unsigned colorSamples = 0;

            if (fbo_ext->isMultisampleSupported())
            {
                for(osg::Camera::BufferAttachmentMap::iterator itr = bufferAttachments.begin();
                    itr != bufferAttachments.end();
                    ++itr)
                {
                    osg::Camera::Attachment& attachment = itr->second;
                    samples = maximum(samples, attachment._multisampleSamples);
                    colorSamples = maximum(colorSamples, attachment._multisampleColorSamples);
                }

                if (colorSamples > samples)
                {
                    osg::notify(WARN) << "Multisample color samples must be less than or "
                        "equal to samples. Setting color samples equal to samples." << std::endl;
                    colorSamples = samples;
                }

                if (samples)
                {
                    fbo_multisample = new osg::FrameBufferObject;
                }
            }
            
            for(osg::Camera::BufferAttachmentMap::iterator itr = bufferAttachments.begin();
                itr != bufferAttachments.end();
                ++itr)
            {

                osg::Camera::BufferComponent buffer = itr->first;
                osg::Camera::Attachment& attachment = itr->second;
                
                if (attachment._texture.valid() || attachment._image.valid())
                    fbo->setAttachment(buffer, osg::FrameBufferAttachment(attachment));
                else
                    fbo->setAttachment(buffer, osg::FrameBufferAttachment(new osg::RenderBuffer(width, height, attachment._internalFormat)));

                if (fbo_multisample.valid())
                {
                    GLenum internalFormat = attachment._internalFormat;
                    if (!internalFormat)
                    {
                        switch (buffer)
                        {
                        case Camera::DEPTH_BUFFER:
                            internalFormat = GL_DEPTH_COMPONENT24;
                            break;
                        case Camera::STENCIL_BUFFER:
                            internalFormat = GL_STENCIL_INDEX8_EXT;
                            break;
                        case Camera::PACKED_DEPTH_STENCIL_BUFFER:
                            internalFormat = GL_DEPTH_STENCIL_EXT;
                            break;

                        // all other buffers are color buffers
                        default:
                            // setup the internal format based on attached texture if such exists, otherwise just default format
                            if (attachment._texture)
                                internalFormat = attachment._texture->getInternalFormat();
                            else
                                internalFormat = GL_RGBA;
                            break;
                        }
                    }
                    fbo_multisample->setAttachment(buffer,
                        osg::FrameBufferAttachment(new osg::RenderBuffer(
                        width, height, internalFormat,
                        samples, colorSamples)));
                }
                
                if (buffer==osg::Camera::DEPTH_BUFFER) depthAttached = true;
                else if (buffer==osg::Camera::STENCIL_BUFFER) stencilAttached = true;
                else if (buffer==osg::Camera::PACKED_DEPTH_STENCIL_BUFFER)
                {
                    depthAttached = true;
                    stencilAttached = true;
                }
                else if (buffer>=osg::Camera::COLOR_BUFFER) colorAttached = true;
                
            }

            if (!depthAttached)
            {                
                fbo->setAttachment(osg::Camera::DEPTH_BUFFER, osg::FrameBufferAttachment(new osg::RenderBuffer(width, height, GL_DEPTH_COMPONENT24)));
                if (fbo_multisample.valid())
                {
                    fbo_multisample->setAttachment(osg::Camera::DEPTH_BUFFER,
                        osg::FrameBufferAttachment(new osg::RenderBuffer(width,
                        height, GL_DEPTH_COMPONENT24, samples, colorSamples)));
                }
            }

            if (!colorAttached)
            {                
                fbo->setAttachment(osg::Camera::COLOR_BUFFER, osg::FrameBufferAttachment(new osg::RenderBuffer(width, height, GL_RGB)));
                if (fbo_multisample.valid())
                {
                    fbo_multisample->setAttachment(osg::Camera::COLOR_BUFFER,
                        osg::FrameBufferAttachment(new osg::RenderBuffer(width,
                        height, GL_RGB, samples, colorSamples)));
                }
            }

            fbo->apply(state);
            
            GLenum status = fbo_ext->glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
            
            if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
            {
                osg::notify(osg::NOTICE)<<"RenderStage::runCameraSetUp(), FBO setup failed, FBO status= 0x"<<std::hex<<status<<std::dec<<std::endl;

                fbo_supported = false;
                fbo_ext->glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                fbo = 0;
                
                // clean up.
                double availableTime = 100.0f;
                double currentTime = state.getFrameStamp()?state.getFrameStamp()->getReferenceTime():0.0;
                osg::RenderBuffer::flushDeletedRenderBuffers(state.getContextID(),currentTime,availableTime);
                osg::FrameBufferObject::flushDeletedFrameBufferObjects(state.getContextID(),currentTime,availableTime);
               

            }
            else
            {
                setDrawBuffer(GL_NONE);
                setReadBuffer(GL_NONE);
       
                _fbo = fbo;

                if (fbo_multisample.valid())
                {
                    fbo_multisample->apply(state);

                    GLenum status = fbo_ext->glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

                    if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
                    {
                        notify(NOTICE) << "RenderStage::runCameraSetUp(), "
                            "multisample FBO setup failed, FBO status = 0x"
                            << std::hex << status << std::dec << std::endl;

                        fbo->apply(state);
                        fbo_multisample = 0;
                        _resolveFbo = 0;

                        // clean up.
                        double availableTime = 100.0f;
                        double currentTime = state.getFrameStamp()?state.getFrameStamp()->getReferenceTime():0.0;
                        osg::RenderBuffer::flushDeletedRenderBuffers(state.getContextID(),currentTime,availableTime);
                        osg::FrameBufferObject::flushDeletedFrameBufferObjects(state.getContextID(),currentTime,availableTime);
                    }
                    else
                    {
                        _resolveFbo.swap(_fbo);
                        _fbo = fbo_multisample;
                    }
                }
                else
                {
                    _resolveFbo = 0;
                }
            }
        }
        
        if (!fbo_supported)
        {
            if (renderTargetImplementation<renderTargetFallback)
                renderTargetImplementation = renderTargetFallback;
            else
                renderTargetImplementation = osg::Camera::PIXEL_BUFFER_RTT;
        }
    }
    
    // check whether PBuffer-RTT is supported or not
    if (renderTargetImplementation==osg::Camera::PIXEL_BUFFER_RTT && 
        !osg::isGLExtensionSupported(state.getContextID(), "WGL_ARB_render_texture"))
    {    
        if (renderTargetImplementation<renderTargetFallback)
            renderTargetImplementation = renderTargetFallback;
        else
            renderTargetImplementation = osg::Camera::PIXEL_BUFFER;
    }

    // if any of the renderTargetImplementations require a separate graphics context such as with pbuffer try in turn to
    // set up, but if each level fails then resort to the next level down.    
    while (!getGraphicsContext() &&
           (renderTargetImplementation==osg::Camera::PIXEL_BUFFER_RTT ||
            renderTargetImplementation==osg::Camera::PIXEL_BUFFER ||
            renderTargetImplementation==osg::Camera::SEPERATE_WINDOW) )
    {
        osg::ref_ptr<osg::GraphicsContext> context = getGraphicsContext();
        if (!context)
        {

            // set up the traits of the graphics context that we want
            osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;

            traits->width = width;
            traits->height = height;

            // osg::notify(osg::NOTICE)<<"traits = "<<traits->width<<" "<<traits->height<<std::endl;

            traits->pbuffer = (renderTargetImplementation==osg::Camera::PIXEL_BUFFER || renderTargetImplementation==osg::Camera::PIXEL_BUFFER_RTT);
            traits->windowDecoration = (renderTargetImplementation==osg::Camera::SEPERATE_WINDOW);
            traits->doubleBuffer = (renderTargetImplementation==osg::Camera::SEPERATE_WINDOW);

            osg::Texture* pBufferTexture = 0;
            GLenum bufferFormat = GL_NONE;
            unsigned int level = 0; 
            unsigned int face = 0; 

            bool colorAttached = false;
            bool depthAttached = false;
            bool stencilAttached = false;
            for(osg::Camera::BufferAttachmentMap::iterator itr = bufferAttachments.begin();
                itr != bufferAttachments.end();
                ++itr)
            {

                osg::Camera::BufferComponent buffer = itr->first;
                osg::Camera::Attachment& attachment = itr->second;
                switch(buffer)
                {
                    case(osg::Camera::DEPTH_BUFFER):
                    {
                        traits->depth = 24;
                        depthAttached = true;
                        break;
                    }
                    case(osg::Camera::STENCIL_BUFFER):
                    {
                        traits->stencil = 8;
                        stencilAttached = true;
                        break;
                    }
                    case(osg::Camera::PACKED_DEPTH_STENCIL_BUFFER):
                    {
                        traits->depth = 24;
                        depthAttached = true;
                        traits->stencil = 8;
                        stencilAttached = true;
                    }
                    case(osg::Camera::COLOR_BUFFER):
                    {
                        if (attachment._internalFormat!=GL_NONE)
                        {
                            bufferFormat = attachment._internalFormat;
                        }
                        else
                        {
                            if (attachment._texture.valid())
                            {
                                pBufferTexture = attachment._texture.get();
                                bufferFormat = attachment._texture->getInternalFormat();
                            }
                            else if (attachment._image.valid())
                            {
                                bufferFormat = attachment._image->getInternalTextureFormat();
                            }
                            else
                            {
                                bufferFormat = GL_RGBA;
                            }
                        }

                        level = attachment._level;
                        face = attachment._face;

                        if (renderTargetImplementation==osg::Camera::PIXEL_BUFFER_RTT)
                        {
                            traits->target = attachment._texture.valid() ? attachment._texture->getTextureTarget() : 0;
                            traits->format = bufferFormat;
                            traits->level = level;
                            traits->face = face;
                            traits->mipMapGeneration = attachment._mipMapGeneration;
                        }
                        break;
                    }
                    default:
                    {
                        if (renderTargetImplementation==osg::Camera::SEPERATE_WINDOW)
                            osg::notify(osg::NOTICE)<<"Warning: RenderStage::runCameraSetUp(State&) Window ";
                        else
                            osg::notify(osg::NOTICE)<<"Warning: RenderStage::runCameraSetUp(State&) Pbuffer ";

                        osg::notify(osg::NOTICE)<<"does not support multiple color outputs."<<std::endl;
                        break;
                    }

                }
            }

            if (!depthAttached)
            {                
                traits->depth = 24;
            }

            if (!colorAttached)
            {                
                if (bufferFormat == GL_NONE) bufferFormat = GL_RGB;

                traits->red = 8;
                traits->green = 8;
                traits->blue = 8;
                traits->alpha = (bufferFormat==GL_RGBA) ? 8 : 0;
            }

            // share OpenGL objects if possible...
            if (state.getGraphicsContext())
            {
                traits->sharedContext = state.getGraphicsContext();
                
                const osg::GraphicsContext::Traits* sharedTraits = traits->sharedContext->getTraits();
                if (sharedTraits)
                {
                    traits->hostName = sharedTraits->hostName;
                    traits->displayNum = sharedTraits->displayNum;
                    traits->screenNum = sharedTraits->screenNum;
                }
            }

            // create the graphics context according to these traits.
            context = osg::GraphicsContext::createGraphicsContext(traits.get());

            if (context.valid() && context->realize())
            {
                osg::notify(osg::INFO)<<"RenderStage::runCameraSetUp(State&) Context has been realized "<<std::endl;

                // successfully set up graphics context as requested,
                // will assign this graphics context to the RenderStage and 
                // associated parameters.  Setting the graphics context will
                // single this while loop to exit successful.
                setGraphicsContext(context.get());
                
                // how to do we detect that an attempt to set up RTT has failed??

                setDrawBuffer(GL_FRONT);
                setReadBuffer(GL_FRONT);

                if (pBufferTexture && renderTargetImplementation==osg::Camera::PIXEL_BUFFER_RTT)
                {
                   osg::notify(osg::INFO)<<"RenderStage::runCameraSetUp(State&) Assign graphis context to Texture"<<std::endl;
                   pBufferTexture->setReadPBuffer(context.get());
                }
                else
                {
                    osg::notify(osg::INFO)<<"RenderStage::runCameraSetUp(State&) Assigning texture to RenderStage so that it does the copy"<<std::endl;
                    setTexture(pBufferTexture, level, face);
                }
            }
            else
            {
                osg::notify(osg::INFO)<<"Failed to acquire Graphics Context"<<std::endl;
                
                if (renderTargetImplementation==osg::Camera::PIXEL_BUFFER_RTT)
                {
                    // fallback to using standard PBuffer, this will allow this while loop to continue
                    if (renderTargetImplementation<renderTargetFallback)
                        renderTargetImplementation = renderTargetFallback;
                    else
                        renderTargetImplementation = osg::Camera::PIXEL_BUFFER;
                }
                else 
                {
                    renderTargetImplementation = osg::Camera::FRAME_BUFFER;
                }
            }

        }
    }
    
    // finally if all else has failed, then the frame buffer fallback will come in to play.
    if (renderTargetImplementation==osg::Camera::FRAME_BUFFER)
    {
        osg::notify(osg::INFO)<<"Setting up osg::Camera::FRAME_BUFFER"<<std::endl;

        for(osg::Camera::BufferAttachmentMap::iterator itr = bufferAttachments.begin();
            itr != bufferAttachments.end();
            ++itr)
        {
            // assign the texture... 
            if (itr->second._texture.valid()) setTexture(itr->second._texture.get(), itr->second._level, itr->second._face);
        }
    }

}

void RenderStage::copyTexture(osg::RenderInfo& renderInfo)
{
    osg::State& state = *renderInfo.getState();

    if (_readBuffer != GL_NONE)
    {
        glReadBuffer(_readBuffer);
    }

    // need to implement texture cube map etc...
    osg::Texture1D* texture1D = 0;
    osg::Texture2D* texture2D = 0;
    osg::Texture3D* texture3D = 0;
    osg::TextureRectangle* textureRec = 0;
    osg::TextureCubeMap* textureCubeMap = 0;

    // use TexCopySubImage with the offset of the viewport into the texture
    // note, this path mirrors the pbuffer and fbo means for updating the texture.
    // Robert Osfield, 3rd August 2006.
    if ((texture2D = dynamic_cast<osg::Texture2D*>(_texture.get())) != 0)
    {
        texture2D->copyTexSubImage2D(state,
                                     static_cast<int>(_viewport->x()),
                                     static_cast<int>(_viewport->y()),
                                     static_cast<int>(_viewport->x()),
                                     static_cast<int>(_viewport->y()),
                                     static_cast<int>(_viewport->width()),
                                     static_cast<int>(_viewport->height()));
    }
    else if ((textureRec = dynamic_cast<osg::TextureRectangle*>(_texture.get())) != 0)
    {
        textureRec->copyTexSubImage2D(state,
                                     static_cast<int>(_viewport->x()),
                                     static_cast<int>(_viewport->y()),
                                     static_cast<int>(_viewport->x()),
                                     static_cast<int>(_viewport->y()),
                                     static_cast<int>(_viewport->width()),
                                     static_cast<int>(_viewport->height()));
    }
    else if ((texture1D = dynamic_cast<osg::Texture1D*>(_texture.get())) != 0)
    {
        // need to implement
        texture1D->copyTexSubImage1D(state,
                                     static_cast<int>(_viewport->x()),
                                     static_cast<int>(_viewport->x()),
                                     static_cast<int>(_viewport->y()),
                                     static_cast<int>(_viewport->width()));
    }
    else if ((texture3D = dynamic_cast<osg::Texture3D*>(_texture.get())) != 0)
    {
        // need to implement
        texture3D->copyTexSubImage3D(state, 
                                     static_cast<int>(_viewport->x()),
                                     static_cast<int>(_viewport->y()),
                                     _face,
                                     static_cast<int>(_viewport->x()),
                                     static_cast<int>(_viewport->y()),
                                     static_cast<int>(_viewport->width()),
                                     static_cast<int>(_viewport->height()));
    }
    else if ((textureCubeMap = dynamic_cast<osg::TextureCubeMap*>(_texture.get())) != 0)
    {
        // need to implement
        textureCubeMap->copyTexSubImageCubeMap(state, _face, 
                                     static_cast<int>(_viewport->x()),
                                     static_cast<int>(_viewport->y()),
                                     static_cast<int>(_viewport->x()),
                                     static_cast<int>(_viewport->y()),
                                     static_cast<int>(_viewport->width()),
                                     static_cast<int>(_viewport->height()));
    }
}

void RenderStage::drawInner(osg::RenderInfo& renderInfo,RenderLeaf*& previous, bool& doCopyTexture)
{
    struct SubFunc
    {
        static void applyReadFBO(bool& apply_read_fbo,
            const FrameBufferObject* read_fbo, osg::State& state)
        {
            if (read_fbo->isMultisample())
            {
                osg::notify(osg::WARN) << "Attempting to read from a"
                    " multisampled framebuffer object. Set a resolve"
                    " framebuffer on the RenderStage to fix this." << std::endl;
            }

            if (apply_read_fbo)
            {
                // Bind the monosampled FBO to read from
                read_fbo->apply(state, FrameBufferObject::READ_FRAMEBUFFER);
                apply_read_fbo = false;
            }
        }
    };

    osg::State& state = *renderInfo.getState();

    osg::FBOExtensions* fbo_ext = _fbo.valid() ? osg::FBOExtensions::instance(state.getContextID(),true) : 0;
    bool fbo_supported = fbo_ext && fbo_ext->isSupported();

    bool using_multiple_render_targets = fbo_supported && _fbo->hasMultipleRenderingTargets();
    
    if (!using_multiple_render_targets)
    {
        if (_drawBuffer != GL_NONE)
        {    
            glDrawBuffer(_drawBuffer);
        }

        if (_readBuffer != GL_NONE)
        {
            glReadBuffer(_readBuffer);
        }
    }

    if (fbo_supported)
    {
        _fbo->apply(state);
    }

    // do the drawing itself.    
    RenderBin::draw(renderInfo,previous);


    if(state.getCheckForGLErrors()!=osg::State::NEVER_CHECK_GL_ERRORS)
    {
        GLenum errorNo = glGetError();
        if (errorNo!=GL_NO_ERROR)
        {
            const char* error = (char*)gluErrorString(errorNo);
            if (error)  osg::notify(osg::NOTICE)<<"Warning: detected OpenGL error '"<<error<<"' after RenderBin::draw(,)"<<std::endl;
            else        osg::notify(osg::NOTICE)<<"Warning: detected OpenGL errorNo= 0x"<<std::hex<<errorNo<<" after RenderBin::draw(,)"<<std::dec<<std::endl;

            if (fbo_ext) osg::notify(osg::NOTICE)<<"RenderStage::drawInner(,) FBO status= 0x"<<std::hex<<fbo_ext->glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)<<std::dec<<std::endl;
        }
    }

    const FrameBufferObject* read_fbo = fbo_supported ? _fbo.get() : 0;
    bool apply_read_fbo = false;

    if (fbo_supported && _resolveFbo.valid() && fbo_ext->glBlitFramebufferEXT)
    {
        GLbitfield blitMask = 0;

        //find which buffer types should be copied
        for (FrameBufferObject::AttachmentMap::const_iterator
            it = _resolveFbo->getAttachmentMap().begin(),
            end =_resolveFbo->getAttachmentMap().end(); it != end; ++it)
        {
            switch (it->first)
            {
            case Camera::DEPTH_BUFFER:
#ifndef OSG_MULTISAMPLE_FBO_RESOLVE_IGNORES_DEPTH
                // This #define is a workaround for an OS X NVIDIA driver bug
                // confirmed on GeForce 8800 with driver v1.5.49.
                // If the glBlitFramebuffer mask includes depth, the OS X desktop
                // will hang. The #define is controlled from CMake and should be
                // set to ON to enable the workaround and avoid blitting depth.
                blitMask |= GL_DEPTH_BUFFER_BIT;
#endif
                break;
            case Camera::STENCIL_BUFFER:
                blitMask |= GL_STENCIL_BUFFER_BIT;
                break;
            case Camera::PACKED_DEPTH_STENCIL_BUFFER:
                blitMask |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
            default:
                blitMask |= GL_COLOR_BUFFER_BIT;
                break;
            }
        }

        // Bind the resolve framebuffer to blit into.
        _fbo->apply(state, FrameBufferObject::READ_FRAMEBUFFER);
        _resolveFbo->apply(state, FrameBufferObject::DRAW_FRAMEBUFFER);

        // Blit to the resolve framebuffer.
        // Note that (with nvidia 175.16 windows drivers at least) if the read
        // framebuffer is multisampled then the dimension arguments are ignored
        // and the whole framebuffer is always copied.
        fbo_ext->glBlitFramebufferEXT(
            0, 0, static_cast<GLint>(_viewport->width()), static_cast<GLint>(_viewport->height()),
            0, 0, static_cast<GLint>(_viewport->width()), static_cast<GLint>(_viewport->height()),
            blitMask, GL_NEAREST);

        apply_read_fbo = true;
        read_fbo = _resolveFbo.get();

        using_multiple_render_targets = read_fbo->hasMultipleRenderingTargets();
    }

    // now copy the rendered image to attached texture.
    if (doCopyTexture)
    {
        if (read_fbo) SubFunc::applyReadFBO(apply_read_fbo, read_fbo, state);
        copyTexture(renderInfo);
    }

    std::map< osg::Camera::BufferComponent, Attachment>::const_iterator itr;
    for(itr = _bufferAttachmentMap.begin();
        itr != _bufferAttachmentMap.end();
        ++itr)
    {
        if (itr->second._image.valid())
        {
            if (read_fbo) SubFunc::applyReadFBO(apply_read_fbo, read_fbo, state);

            if (using_multiple_render_targets)
            {
                int attachment=itr->first;
                if (attachment==osg::Camera::DEPTH_BUFFER || attachment==osg::Camera::STENCIL_BUFFER) {
                    // assume first buffer rendered to is the one we want
                    glReadBuffer(read_fbo->getMultipleRenderingTargets()[0]);
                } else {
                    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + (attachment - osg::Camera::COLOR_BUFFER0));
                }
            } else {
                if (_readBuffer != GL_NONE)
                {
                    glReadBuffer(_readBuffer);
                }
            }

            GLenum pixelFormat = itr->second._image->getPixelFormat();
            if (pixelFormat==0) pixelFormat = _imageReadPixelFormat;
            if (pixelFormat==0) pixelFormat = GL_RGB;

            GLenum dataType = itr->second._image->getDataType();
            if (dataType==0) dataType = _imageReadPixelDataType;
            if (dataType==0) dataType = GL_UNSIGNED_BYTE;       

            itr->second._image->readPixels(static_cast<int>(_viewport->x()),
                                           static_cast<int>(_viewport->y()),
                                           static_cast<int>(_viewport->width()),
                                           static_cast<int>(_viewport->height()), 
                                           pixelFormat, dataType);
        }
    }

    if (fbo_supported)
    {
        if (getDisableFboAfterRender())
        {
            // switch off the frame buffer object
            fbo_ext->glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        }

        doCopyTexture = true;
    }

    if (fbo_supported && _camera)
    {
        // now generate mipmaps if they are required.
        const osg::Camera::BufferAttachmentMap& bufferAttachments = _camera->getBufferAttachmentMap();
        for(osg::Camera::BufferAttachmentMap::const_iterator itr = bufferAttachments.begin();
            itr != bufferAttachments.end();
            ++itr)
        {
            if (itr->second._texture.valid() && itr->second._mipMapGeneration) 
            {
                state.setActiveTextureUnit(0);
                state.applyTextureAttribute(0, itr->second._texture.get());
                fbo_ext->glGenerateMipmapEXT(itr->second._texture->getTextureTarget());
            }
        }
    }
}

struct DrawInnerOperation : public osg::Operation
{
    DrawInnerOperation(RenderStage* stage, osg::RenderInfo& renderInfo) : 
        osg::Operation("DrawInnerStage",false),
        _stage(stage),
        _renderInfo(renderInfo) {}

    virtual void operator () (osg::Object* object)
    {
        osg::GraphicsContext* context = dynamic_cast<osg::GraphicsContext*>(object);
        if (!context) return;

        // osg::notify(osg::NOTICE)<<"DrawInnerOperation operator"<<std::endl;
        if (_stage && context)
        {
            RenderLeaf* previous = 0;
            bool doCopyTexture = false;
            _renderInfo.setState(context->getState());
            _stage->drawInner(_renderInfo, previous, doCopyTexture);
        }
    }
    
    RenderStage* _stage;
    RenderInfo _renderInfo;
};


void RenderStage::draw(osg::RenderInfo& renderInfo,RenderLeaf*& previous)
{
    if (_stageDrawnThisFrame) return;

    // push the stages camera so that drawing code can query it     
    if (_camera) renderInfo.pushCamera(_camera);

    _stageDrawnThisFrame = true;

    if (_camera && _camera->getInitialDrawCallback())
    {
        // if we have a camera with a intial draw callback invoke it.
        (*(_camera->getInitialDrawCallback()))(renderInfo);
    }

    // note, SceneView does call to drawPreRenderStages explicitly
    // so there is no need to call it here.
    drawPreRenderStages(renderInfo,previous);
    
    if (_cameraRequiresSetUp)
    {
        runCameraSetUp(renderInfo);
    }

    osg::State& state = *renderInfo.getState();

    osg::State* useState = &state;
    osg::GraphicsContext* callingContext = state.getGraphicsContext();
    osg::GraphicsContext* useContext = callingContext;
    osg::OperationThread* useThread = 0;
    osg::RenderInfo useRenderInfo(renderInfo);
    
    RenderLeaf* saved_previous = previous;

    if (_graphicsContext.valid() && _graphicsContext != callingContext)
    {
        // show we release the context so that others can use it?? will do so right
        // now as an experiment.
        callingContext->releaseContext();
    
        // osg::notify(osg::NOTICE)<<"  enclosing state before - "<<state.getStateSetStackSize()<<std::endl;

        useState = _graphicsContext->getState();
        useContext = _graphicsContext.get();
        useThread = useContext->getGraphicsThread();
        useRenderInfo.setState(useState);
        
        // synchronize the frame stamps
        useState->setFrameStamp(const_cast<osg::FrameStamp*>(state.getFrameStamp()));

        // map the DynamicObjectCount across to the new window
        useState->setDynamicObjectCount(state.getDynamicObjectCount());
        useState->setDynamicObjectRenderingCompletedCallback(state.getDynamicObjectRenderingCompletedCallback());
        
        if (!useThread) 
        {
            previous = 0;
            useContext->makeCurrent();
            
            // osg::notify(osg::NOTICE)<<"  nested state before - "<<useState->getStateSetStackSize()<<std::endl;
        }
    }

    unsigned int originalStackSize = useState->getStateSetStackSize();

    if (_camera && _camera->getPreDrawCallback())
    {
        // if we have a camera with a pre draw callback invoke it.
        (*(_camera->getPreDrawCallback()))(renderInfo);
    }
    
    bool doCopyTexture = _texture.valid() ? 
                        (callingContext != useContext) :
                        false;

    if (useThread)
    {
#if 1
        ref_ptr<osg::BlockAndFlushOperation> block = new osg::BlockAndFlushOperation;
    
        useThread->add(new DrawInnerOperation( this, renderInfo ));
        
        useThread->add(block.get());
        
        // wait till the DrawInnerOperations is complete.
        block->block();
        
        doCopyTexture = false;
        
#else
        useThread->add(new DrawInnerOperation( this, renderInfo ), true);
        
        doCopyTexture = false;
#endif        
    }
    else
    {
        drawInner( useRenderInfo, previous, doCopyTexture);
        
        if (useRenderInfo.getUserData() != renderInfo.getUserData())
        {
            renderInfo.setUserData(useRenderInfo.getUserData());
        }
        
    }

    if (useState != &state)
    {
        // reset the local State's DynamicObjectCount
        state.setDynamicObjectCount(useState->getDynamicObjectCount());
        useState->setDynamicObjectRenderingCompletedCallback(0);
    }


    // now copy the rendered image to attached texture.
    if (_texture.valid() && !doCopyTexture)
    {
        if (callingContext && useContext!= callingContext)
        {
            // make the calling context use the pbuffer context for reading.
            callingContext->makeContextCurrent(useContext);
        }

        copyTexture(renderInfo);
    }

    if (_camera && _camera->getPostDrawCallback())
    {
        // if we have a camera with a post draw callback invoke it.
        (*(_camera->getPostDrawCallback()))(renderInfo);
    }

    if (_graphicsContext.valid() && _graphicsContext != callingContext)
    {
        useState->popStateSetStackToSize(originalStackSize);

        if (!useThread)
        {


            // flush any command left in the useContex's FIFO
            // to ensure that textures are updated before main thread commenses.
            glFlush();
            
        
            useContext->releaseContext();
        }
    }

    if (callingContext && useContext != callingContext)
    {
        // restore the graphics context.
        
        previous = saved_previous;
        
        // osg::notify(osg::NOTICE)<<"  nested state after - "<<useState->getStateSetStackSize()<<std::endl;
        // osg::notify(osg::NOTICE)<<"  enclosing state after - "<<state.getStateSetStackSize()<<std::endl;

        callingContext->makeCurrent();
    }

    // render all the post draw callbacks
    drawPostRenderStages(renderInfo,previous);

    if (_camera && _camera->getFinalDrawCallback())
    {
        // if we have a camera with a final callback invoke it.
        (*(_camera->getFinalDrawCallback()))(renderInfo);
    }

    // pop the render stages camera.
    if (_camera) renderInfo.popCamera();
}

void RenderStage::drawImplementation(osg::RenderInfo& renderInfo,RenderLeaf*& previous)
{
    osg::State& state = *renderInfo.getState();

    if (!_viewport)
    {
        notify(FATAL) << "Error: cannot draw stage due to undefined viewport."<< std::endl;
        return;
    }

    // set up the back buffer.
    state.applyAttribute(_viewport.get());

    glScissor( static_cast<int>(_viewport->x()),
               static_cast<int>(_viewport->y()),
               static_cast<int>(_viewport->width()),
               static_cast<int>(_viewport->height()) );
    //cout << "    clearing "<<this<< "  "<<_viewport->x()<<","<< _viewport->y()<<","<< _viewport->width()<<","<< _viewport->height()<<std::endl;
    state.applyMode( GL_SCISSOR_TEST, true );

    // glEnable( GL_DEPTH_TEST );

    // set which color planes to operate on.
    if (_colorMask.valid()) _colorMask->apply(state);
    else glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

    if (_clearMask & GL_COLOR_BUFFER_BIT)
    {
        glClearColor( _clearColor[0], _clearColor[1], _clearColor[2], _clearColor[3]);
    }

    if (_clearMask & GL_DEPTH_BUFFER_BIT)
    {
        glClearDepth( _clearDepth);
        glDepthMask ( GL_TRUE );
        state.haveAppliedAttribute( osg::StateAttribute::DEPTH );
    }

    if (_clearMask & GL_STENCIL_BUFFER_BIT)
    {
        glClearStencil( _clearStencil);
        glStencilMask ( ~0u );
        state.haveAppliedAttribute( osg::StateAttribute::STENCIL );
    }

    if (_clearMask & GL_ACCUM_BUFFER_BIT)
    {
        glClearAccum( _clearAccum[0], _clearAccum[1], _clearAccum[2], _clearAccum[3]);
    }


    glClear( _clearMask );
    
#ifdef USE_SCISSOR_TEST
    glDisable( GL_SCISSOR_TEST );
#endif

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    // apply the positional state.
    if (_inheritedPositionalStateContainer.valid())
    {
        _inheritedPositionalStateContainer->draw(state, previous, &_inheritedPositionalStateContainerMatrix);
    }

    // apply the positional state.
    if (_renderStageLighting.valid())
    {
        _renderStageLighting->draw(state, previous, 0);
    }

    // draw the children and local.
    RenderBin::drawImplementation(renderInfo,previous);

    state.apply();

}

void RenderStage::drawPostRenderStages(osg::RenderInfo& renderInfo,RenderLeaf*& previous)
{
    if (_postRenderList.empty()) return;
    
    //cout << "Drawing prerendering stages "<<this<< "  "<<_viewport->x()<<","<< _viewport->y()<<","<< _viewport->width()<<","<< _viewport->height()<<std::endl;
    for(RenderStageList::iterator itr=_postRenderList.begin();
        itr!=_postRenderList.end();
        ++itr)
    {
        itr->second->draw(renderInfo,previous);
    }
    //cout << "Done Drawing prerendering stages "<<this<< "  "<<_viewport->x()<<","<< _viewport->y()<<","<< _viewport->width()<<","<< _viewport->height()<<std::endl;
}

// Statistics features
bool RenderStage::getStats(Statistics& stats) const
{
    bool statsCollected = false;

    for(RenderStageList::const_iterator pre_itr = _preRenderList.begin();
        pre_itr != _preRenderList.end();
        ++pre_itr)
    {
        if (pre_itr->second->getStats(stats))
        {
            statsCollected = true;
        }
    }

    for(RenderStageList::const_iterator post_itr = _postRenderList.begin();
        post_itr != _postRenderList.end();
        ++post_itr)
    {
        if (post_itr->second->getStats(stats))
        {
            statsCollected = true;
        }
    }
        
    if (RenderBin::getStats(stats))
    {
        statsCollected = true;
    }
    return statsCollected;
}

void RenderStage::attach(osg::Camera::BufferComponent buffer, osg::Image* image)
{
    _bufferAttachmentMap[buffer]._image = image;
}

unsigned int RenderStage::computeNumberOfDynamicRenderLeaves() const
{
    unsigned int count = 0;

    for(RenderStageList::const_iterator pre_itr = _preRenderList.begin();
        pre_itr != _preRenderList.end();
        ++pre_itr)
    {
        count += pre_itr->second->computeNumberOfDynamicRenderLeaves();
    }

    count += RenderBin::computeNumberOfDynamicRenderLeaves();

    for(RenderStageList::const_iterator post_itr = _postRenderList.begin();
        post_itr != _postRenderList.end();
        ++post_itr)
    {
        count += post_itr->second->computeNumberOfDynamicRenderLeaves();
    }
    
    return count;
}


void osgUtil::RenderStage::setMultisampleResolveFramebufferObject(osg::FrameBufferObject* fbo)
{
    if (fbo && fbo->isMultisample())
    {
        osg::notify(osg::WARN) << "Resolve framebuffer must not be"
            " multisampled." << std::endl;
    }
    _resolveFbo = fbo;
}
