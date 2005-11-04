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
#include <stdio.h>

#include <osg/Notify>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/TextureRectangle>
#include <osg/TextureCubeMap>
#include <osg/GLExtensions>

#include <osgUtil/Statistics>

#include <osgUtil/RenderStage>


using namespace osg;
using namespace osgUtil;

// register a RenderStage prototype with the RenderBin prototype list.
//RegisterRenderBinProxy<RenderStage> s_registerRenderStageProxy;

RenderStage::RenderStage():
    RenderBin(getDefaultRenderBinSortMode())
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

    _camera = 0;
    
    _level = 0;
    _face = 0;
    
    _imageReadPixelFormat = GL_RGBA;
    _imageReadPixelDataType = GL_UNSIGNED_BYTE;
}

RenderStage::RenderStage(SortMode mode):
    RenderBin(mode)
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
        _camera(rhs._camera),
        _level(rhs._level),
        _face(rhs._face),
        _imageReadPixelFormat(rhs._imageReadPixelFormat),
        _imageReadPixelDataType(rhs._imageReadPixelDataType),
        _renderStageLighting(rhs._renderStageLighting)
{
    _stage = this;
}


RenderStage::~RenderStage()
{
}

void RenderStage::reset()
{
    _preRenderList.clear();
    _stageDrawnThisFrame = false;
    
    if (_renderStageLighting.valid()) _renderStageLighting->reset();

    RenderBin::reset();
}

void RenderStage::addPreRenderStage(RenderStage* rs)
{
    if (rs) _preRenderList.push_back(rs);
}

void RenderStage::addPostRenderStage(RenderStage* rs)
{
    if (rs) _postRenderList.push_back(rs);
}

void RenderStage::drawPreRenderStages(osg::State& state,RenderLeaf*& previous)
{
    if (_preRenderList.empty()) return;
    
    //cout << "Drawing prerendering stages "<<this<< "  "<<_viewport->x()<<","<< _viewport->y()<<","<< _viewport->width()<<","<< _viewport->height()<<std::endl;
    for(RenderStageList::iterator itr=_preRenderList.begin();
        itr!=_preRenderList.end();
        ++itr)
    {
        (*itr)->draw(state,previous);
    }
    //cout << "Done Drawing prerendering stages "<<this<< "  "<<_viewport->x()<<","<< _viewport->y()<<","<< _viewport->width()<<","<< _viewport->height()<<std::endl;
}


void RenderStage::runCameraSetUp(osg::State& state)
{
    _cameraRequiresSetUp = false;

    if (!_camera) return;
    
    osg::CameraNode::RenderTargetImplementation renderTargetImplemntation = _camera->getRenderTargetImplementation();
    osg::CameraNode::RenderTargetImplementation renderTargetFallback = _camera->getRenderTargetFallback();

    osg::CameraNode::BufferAttachmentMap& bufferAttachements = _camera->getBufferAttachmentMap();

    // attach an images that need to be copied after the stage is drawn.
    for(osg::CameraNode::BufferAttachmentMap::iterator itr = bufferAttachements.begin();
        itr != bufferAttachements.end();
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

                image->allocateImage(_viewport->width(), _viewport->height(), 1, pixelFormat, dataType);

            }

            _imageReadPixelFormat = pixelFormat;
            _imageReadPixelDataType = dataType;

            setImage(itr->second._image.get());
        }
        
        if (itr->second._texture.valid())
        {
            osg::Texture* texture = itr->second._texture.get();
            osg::Texture1D* texture1D = 0;
            osg::Texture2D* texture2D = 0;
            osg::Texture3D* texture3D = 0;
            osg::TextureCubeMap* textureCubeMap = 0;
            osg::TextureRectangle* textureRectangle = 0;
            if (0 != (texture1D=dynamic_cast<osg::Texture1D*>(texture)))
            {
                if (texture1D->getTextureWidth()==0)
                {
                    texture1D->setTextureWidth(_viewport->width());
                }
            }
            else if (0 != (texture2D = dynamic_cast<osg::Texture2D*>(texture)))
            {
                if (texture2D->getTextureWidth()==0 || texture2D->getTextureHeight()==0)
                {
                    texture2D->setTextureSize(_viewport->width(),_viewport->height());
                }
            }
            else if (0 != (texture3D = dynamic_cast<osg::Texture3D*>(texture)))
            {
                if (texture3D->getTextureWidth()==0 || texture3D->getTextureHeight()==0 || texture3D->getTextureDepth()==0 )
                {
                    // note we dont' have the depth here, so we'll heave to assume that height and depth are the same..
                    texture3D->setTextureSize(_viewport->width(),_viewport->height(),_viewport->height());
                }
            }
            else if (0 != (textureCubeMap = dynamic_cast<osg::TextureCubeMap*>(texture)))
            {
                if (textureCubeMap->getTextureWidth()==0 || textureCubeMap->getTextureHeight()==0)
                {
                    textureCubeMap->setTextureSize(_viewport->width(),_viewport->height());
                }
            }
            else if (0 != (textureRectangle = dynamic_cast<osg::TextureRectangle*>(texture)))
            {
                if (textureRectangle->getTextureWidth()==0 || textureRectangle->getTextureHeight()==0)
                {
                    textureRectangle->setTextureSize(_viewport->width(),_viewport->height());
                }
            }

        }
    }
    
    if (renderTargetImplemntation==osg::CameraNode::FRAME_BUFFER_OBJECT)
    {
        osg::FBOExtensions* fbo_ext = osg::FBOExtensions::instance(state.getContextID());
        bool fbo_supported = fbo_ext && fbo_ext->isSupported();

        if (!fbo_supported)
        {
            if (renderTargetImplemntation<renderTargetFallback)
                renderTargetImplemntation = renderTargetFallback;
            else
                renderTargetImplemntation = osg::CameraNode::PIXEL_BUFFER_RTT;
        }
        else if (!_fbo)
        {
            osg::notify(osg::INFO)<<"Setting up osg::CameraNode::FRAME_BUFFER_OBJECT"<<std::endl;

            _fbo = new osg::FrameBufferObject;

            setDrawBuffer(GL_NONE);
            setReadBuffer(GL_NONE);

            bool colorAttached = false;
            bool depthAttached = false;
            bool stencilAttached = false;
            for(osg::CameraNode::BufferAttachmentMap::iterator itr = bufferAttachements.begin();
                itr != bufferAttachements.end();
                ++itr)
            {

                osg::CameraNode::BufferComponent buffer = itr->first;
                osg::CameraNode::Attachment& attachment = itr->second;
                
                switch(buffer)
                {
                    case(osg::CameraNode::DEPTH_BUFFER):
                    {
                        _fbo->setAttachment(GL_DEPTH_ATTACHMENT_EXT, osg::FrameBufferAttachment(attachment));
                        depthAttached = true;
                        break;
                    }
                    case(osg::CameraNode::STENCIL_BUFFER):
                    {
                        _fbo->setAttachment(GL_STENCIL_ATTACHMENT_EXT, osg::FrameBufferAttachment(attachment));
                        stencilAttached = true;
                        break;
                    }
                    default:
                    {
                        _fbo->setAttachment(GL_COLOR_ATTACHMENT0_EXT+(buffer-osg::CameraNode::COLOR_BUFFER0), osg::FrameBufferAttachment(attachment));
                        colorAttached = true;
                        break;
                    }

                }
            }

            if (!depthAttached)
            {                
                _fbo->setAttachment(GL_DEPTH_ATTACHMENT_EXT, osg::FrameBufferAttachment(new osg::RenderBuffer(_viewport->width(), _viewport->height(), GL_DEPTH_COMPONENT24)));
            }

            if (!colorAttached)
            {                
                _fbo->setAttachment(GL_COLOR_ATTACHMENT0_EXT, osg::FrameBufferAttachment(new osg::RenderBuffer(_viewport->width(), _viewport->height(), GL_RGB)));
            }
        }
    }
    
    // check whether PBuffer-RTT is supported or not
    if (renderTargetImplemntation==osg::CameraNode::PIXEL_BUFFER_RTT && 
        !osg::isGLExtensionSupported(state.getContextID(), "ARB_render_texture"))
    {    
        if (renderTargetImplemntation<renderTargetFallback)
            renderTargetImplemntation = renderTargetFallback;
        else
            renderTargetImplemntation = osg::CameraNode::PIXEL_BUFFER;
    }

    // if any of the renderTargetImplementations require a seperate graphics context such as with pbuffer try in turn to
    // set up, but if each level fails then resort to the next level down.    
    while (!getGraphicsContext() &&
           (renderTargetImplemntation==osg::CameraNode::PIXEL_BUFFER_RTT ||
            renderTargetImplemntation==osg::CameraNode::PIXEL_BUFFER ||
            renderTargetImplemntation==osg::CameraNode::SEPERATE_WINDOW) )
    {
        osg::ref_ptr<osg::GraphicsContext> context = getGraphicsContext();
        if (!context)
        {

            // set up the traits of the graphics context that we want
            osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;

            traits->_width = _viewport->width();
            traits->_height = _viewport->height();
            traits->_pbuffer = (renderTargetImplemntation==osg::CameraNode::PIXEL_BUFFER || renderTargetImplemntation==osg::CameraNode::PIXEL_BUFFER_RTT);
            traits->_windowDecoration = (renderTargetImplemntation==osg::CameraNode::SEPERATE_WINDOW);
            traits->_doubleBuffer = (renderTargetImplemntation==osg::CameraNode::SEPERATE_WINDOW);

            osg::Texture* pBufferTexture = 0;
            GLenum bufferFormat = GL_NONE;
            unsigned int level = 0; 
            unsigned int face = 0; 

            bool colorAttached = false;
            bool depthAttached = false;
            bool stencilAttached = false;
            for(osg::CameraNode::BufferAttachmentMap::iterator itr = bufferAttachements.begin();
                itr != bufferAttachements.end();
                ++itr)
            {

                osg::CameraNode::BufferComponent buffer = itr->first;
                osg::CameraNode::Attachment& attachment = itr->second;
                switch(buffer)
                {
                    case(osg::CameraNode::DEPTH_BUFFER):
                    {
                        traits->_depth = 24;
                        depthAttached = true;
                        break;
                    }
                    case(osg::CameraNode::STENCIL_BUFFER):
                    {
                        traits->_stencil = 8;
                        stencilAttached = true;
                        break;
                    }
                    case(osg::CameraNode::COLOR_BUFFER):
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

                        if (renderTargetImplemntation==osg::CameraNode::PIXEL_BUFFER_RTT)
                        {
                            traits->_target = bufferFormat;
                            traits->_level = level;
                            traits->_face = face;
                            traits->_mipMapGeneration = attachment._mipMapGeneration;
                        }
                        break;
                    }
                    default:
                    {
                        if (renderTargetImplemntation==osg::CameraNode::SEPERATE_WINDOW)
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
                traits->_depth = 24;
            }

            if (!colorAttached)
            {                
                if (bufferFormat == GL_NONE) bufferFormat = GL_RGB;

                traits->_red = 8;
                traits->_green = 8;
                traits->_blue = 8;
                traits->_alpha = (bufferFormat==GL_RGBA) ? 8 : 0;
            }

            // share OpenGL objects if possible...
            if (state.getGraphicsContext())
            {
                traits->_sharedContext = state.getGraphicsContext();
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

                if (pBufferTexture && renderTargetImplemntation==osg::CameraNode::PIXEL_BUFFER_RTT)
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
                osg::notify(osg::INFO)<<"Failed to aquire Graphics Context"<<std::endl;
                
                if (renderTargetImplemntation==osg::CameraNode::PIXEL_BUFFER_RTT)
                {
                    // fallback to using standard PBuffer, this will allow this while loop to continue
                    if (renderTargetImplemntation<renderTargetFallback)
                        renderTargetImplemntation = renderTargetFallback;
                    else
                        renderTargetImplemntation = osg::CameraNode::PIXEL_BUFFER;
                }
                else 
                {
                    renderTargetImplemntation = osg::CameraNode::FRAME_BUFFER;
                }
            }

        }
    }
    
    // finally if all else has failed, then the frame buffer fallback will come in to play.
    if (renderTargetImplemntation==osg::CameraNode::FRAME_BUFFER)
    {
        osg::notify(osg::INFO)<<"Setting up osg::CameraNode::FRAME_BUFFER"<<std::endl;

        for(osg::CameraNode::BufferAttachmentMap::iterator itr = bufferAttachements.begin();
            itr != bufferAttachements.end();
            ++itr)
        {
            // assign the texture... 
            if (itr->second._texture.valid()) setTexture(itr->second._texture.get(), itr->second._level, itr->second._face);
        }
    }

}

void RenderStage::copyTexture(osg::State& state)
{
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

    if ((texture2D = dynamic_cast<osg::Texture2D*>(_texture.get())) != 0)
    {
        texture2D->copyTexImage2D(state,_viewport->x(),_viewport->y(),_viewport->width(),_viewport->height());
    }
    else if ((textureRec = dynamic_cast<osg::TextureRectangle*>(_texture.get())) != 0)
    {
        textureRec->copyTexImage2D(state,_viewport->x(),_viewport->y(),_viewport->width(),_viewport->height());
    }
    else if ((texture1D = dynamic_cast<osg::Texture1D*>(_texture.get())) != 0)
    {
        // need to implement
        texture1D->copyTexImage1D(state,_viewport->x(),_viewport->y(),_viewport->width());
    }
    else if ((texture3D = dynamic_cast<osg::Texture3D*>(_texture.get())) != 0)
    {
        // need to implement
        texture3D->copyTexSubImage3D(state, 0, 0, _face, _viewport->x(), _viewport->y(), _viewport->width(), _viewport->height());
    }
    else if ((textureCubeMap = dynamic_cast<osg::TextureCubeMap*>(_texture.get())) != 0)
    {
        // need to implement
        textureCubeMap->copyTexSubImageCubeMap(state, _face, 0, 0, _viewport->x(),_viewport->y(),_viewport->width(),_viewport->height());
    }
}

void RenderStage::drawInner(osg::State& state,RenderLeaf*& previous, bool& doCopyTexture)
{
    if (_drawBuffer != GL_NONE)
    {    
        glDrawBuffer(_drawBuffer);
    }
    
    if (_readBuffer != GL_NONE)
    {
        glReadBuffer(_readBuffer);
    }

    osg::FBOExtensions* fbo_ext = _fbo.valid() ? osg::FBOExtensions::instance(state.getContextID()) : 0;
    bool fbo_supported = fbo_ext && fbo_ext->isSupported();

    if (fbo_supported)
    {
        _fbo->apply(state);
    }

    // do the drawing itself.    
    RenderBin::draw(state,previous);


    GLenum errorNo = glGetError();
    if (errorNo!=GL_NO_ERROR)
    {
        osg::notify(osg::NOTICE)<<"RenderStage::drawInner(,) OpenGL errorNo= 0x"<<std::hex<<errorNo<<std::endl;
        if (fbo_ext) osg::notify(osg::NOTICE)<<"RenderStage::drawInner(,) FBO status= 0x"<<std::hex<<fbo_ext->glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)<<std::endl;
    }
    // state.checkGLErrors("After RenderBin::draw");

    // now copy the rendered image to attached texture.
    if (doCopyTexture)
    {
        copyTexture(state);
    }
    
    if (_image.valid())
    {

        if (_readBuffer != GL_NONE)
        {
            glReadBuffer(_readBuffer);
        }

        GLenum pixelFormat = _image->getPixelFormat();
        if (pixelFormat==0) pixelFormat = _imageReadPixelFormat;
        if (pixelFormat==0) pixelFormat = GL_RGB;

        GLenum dataType = _image->getDataType();
        if (dataType==0) dataType =  _imageReadPixelDataType;
        if (dataType==0) dataType = GL_UNSIGNED_BYTE;       
        
        _image->readPixels(_viewport->x(), _viewport->y(),
                           _viewport->width(), _viewport->height(), 
                           pixelFormat, dataType);

    }
       
    
    if (fbo_supported)
    {
        // switch of the frame buffer object
        fbo_ext->glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

        doCopyTexture = true;
    }
    
    if (fbo_supported && _camera)
    {
        // now generate mipmaps if they are required.
        const osg::CameraNode::BufferAttachmentMap& bufferAttachements = _camera->getBufferAttachmentMap();
        for(osg::CameraNode::BufferAttachmentMap::const_iterator itr = bufferAttachements.begin();
            itr != bufferAttachements.end();
            ++itr)
        {
            if (itr->second._texture.valid() && itr->second._mipMapGeneration) 
            {
                itr->second._texture->apply(state);
                // fbo_ext->glGenerateMipmapEXT(itr->second._texture->getTextureTarget());
            }
        }
    }
}

struct DrawInnerOperation : public osg::GraphicsThread::Operation
{
    DrawInnerOperation(RenderStage* stage) : 
        osg::GraphicsThread::Operation("DrawInnerStage",false),
        _stage(stage) {}

    virtual void operator() (osg::GraphicsContext* context)
    {
        // osg::notify(osg::NOTICE)<<"DrawInnerOperation operator"<<std::endl;
        if (_stage && context)
        {
            RenderLeaf* previous = 0;
            bool doCopyTexture = false;
            _stage->drawInner(*(context->getState()), previous, doCopyTexture);
        }
    }
    
    RenderStage* _stage;
};


void RenderStage::draw(osg::State& state,RenderLeaf*& previous)
{
    if (_stageDrawnThisFrame) return;

    _stageDrawnThisFrame = true;

    // note, SceneView does call to drawPreRenderStages explicitly
    // so there is no need to call it here.
    drawPreRenderStages(state,previous);
    
    if (_cameraRequiresSetUp)
    {
        runCameraSetUp(state);
    }


    osg::State* useState = &state;
    osg::GraphicsContext* callingContext = state.getGraphicsContext();
    osg::GraphicsContext* useContext = callingContext;
    osg::GraphicsThread* useThread = 0;

    if (_graphicsContext.valid() && _graphicsContext != callingContext)
    {
        // show we release the context so that others can use it?? will do so right
        // now as an experiment.
        callingContext->releaseContext();
    
        useState = _graphicsContext->getState();
        useContext = _graphicsContext.get();
        
        useThread = useContext->getGraphicsThread();
        
        if (!useThread) useContext->makeCurrent();
    }
    
    bool doCopyTexture = _texture.valid() ? 
                        (callingContext != useContext) :
                        false;

    if (useThread)
    {
        useThread->add(new DrawInnerOperation( this ), true);
        
        doCopyTexture = false;
    }
    else
    {
        drawInner( *useState, previous, doCopyTexture);
    }


    // now copy the rendered image to attached texture.
    if (_texture.valid() && !doCopyTexture)
    {
        if (callingContext && useContext!= callingContext)
        {
            // make the calling context use the pbuffer context for reading.
            callingContext->makeContextCurrent(useContext);
        }

        copyTexture(state);
    }

    if (_camera && _camera->getPostDrawCallback())
    {
        // if we have a camera with a post draw callback invoke it.
        (*(_camera->getPostDrawCallback()))(*_camera);
    }

    if (_graphicsContext.valid() && _graphicsContext != callingContext)
    {
        if (!useThread) useContext->releaseContext();
    }

    if (callingContext && useContext != callingContext)
    {
        // restore the graphics context.
        callingContext->makeCurrent();
    }

    // place the post draw here temprorarily while we figure out how
    // best to do SceneView.
    drawPostRenderStages(state,previous);
}

void RenderStage::drawImplementation(osg::State& state,RenderLeaf*& previous)
{
    
    if (!_viewport)
    {
        notify(FATAL) << "Error: cannot draw stage due to undefined viewport."<< std::endl;
        return;
    }
     
    // set up the back buffer.
    state.applyAttribute(_viewport.get());

#define USE_SISSOR_TEST
#ifdef USE_SISSOR_TEST
    glScissor( _viewport->x(), _viewport->y(), _viewport->width(), _viewport->height() );
    //cout << "    clearing "<<this<< "  "<<_viewport->x()<<","<< _viewport->y()<<","<< _viewport->width()<<","<< _viewport->height()<<std::endl;
    
    glEnable( GL_SCISSOR_TEST );
#endif


    // glEnable( GL_DEPTH_TEST );

    // set which color planes to operate on.
    if (_colorMask.valid()) _colorMask->apply(state);
    else glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

    if (_clearMask & GL_COLOR_BUFFER_BIT)
        glClearColor( _clearColor[0], _clearColor[1], _clearColor[2], _clearColor[3]);

    if (_clearMask & GL_DEPTH_BUFFER_BIT)
        glClearDepth( _clearDepth);

    if (_clearMask & GL_STENCIL_BUFFER_BIT)
        glClearStencil( _clearStencil);

    if (_clearMask & GL_ACCUM_BUFFER_BIT)
        glClearAccum( _clearAccum[0], _clearAccum[1], _clearAccum[2], _clearAccum[3]);


    glClear( _clearMask );
    
#ifdef USE_SISSOR_TEST
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
    RenderBin::drawImplementation(state,previous);

    // now reset the state so its back in its default state.
    if (previous)
    {
        StateGraph::moveToRootStateGraph(state,previous->_parent);
        state.apply();
        previous = NULL;
    }

}

void RenderStage::drawPostRenderStages(osg::State& state,RenderLeaf*& previous)
{
    if (_postRenderList.empty()) return;
    
    //cout << "Drawing prerendering stages "<<this<< "  "<<_viewport->x()<<","<< _viewport->y()<<","<< _viewport->width()<<","<< _viewport->height()<<std::endl;
    for(RenderStageList::iterator itr=_postRenderList.begin();
        itr!=_postRenderList.end();
        ++itr)
    {
        (*itr)->draw(state,previous);
    }
    //cout << "Done Drawing prerendering stages "<<this<< "  "<<_viewport->x()<<","<< _viewport->y()<<","<< _viewport->width()<<","<< _viewport->height()<<std::endl;
}

// Statistics features
bool RenderStage::getStats(Statistics* primStats)
{
    if (_renderStageLighting.valid())
    {
        // need to re-implement by checking for lights in the scene
        // by downcasting the positioned attribute list. RO. May 2002.
        //primStats->addLight(_renderStageLighting->_lightList.size());
    }
    return RenderBin::getStats(primStats);
}
