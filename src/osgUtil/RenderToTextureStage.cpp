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
#include <osgUtil/RenderToTextureStage>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/TextureRectangle>
#include <osg/TextureCubeMap>
#include <osg/Notify>

using namespace osg;
using namespace osgUtil;

// register a RenderToTextureStage prototype with the RenderBin prototype list.
//RegisterRenderBinProxy<RenderToTextureStage> s_registerRenderToTextureStageProxy;

RenderToTextureStage::RenderToTextureStage()
{
    _camera = 0;
    
    _level = 0;
    _face = 0;
    
    _imageReadPixelFormat = GL_RGBA;
    _imageReadPixelDataType = GL_UNSIGNED_BYTE;
}

RenderToTextureStage::~RenderToTextureStage()
{
}

void RenderToTextureStage::reset()
{
    RenderStage::reset();
}


void RenderToTextureStage::draw(osg::State& state,RenderLeaf*& previous)
{
    
    if (_stageDrawnThisFrame) return;

    //cout << "begining RTTS draw "<<this<< "  "<<_viewport->x()<<","<< _viewport->y()<<","<< _viewport->width()<<","<< _viewport->height()<<std::endl;
    
    osg::State* useState = &state;
    osg::GraphicsContext* callingContext = state.getGraphicsContext();
    osg::GraphicsContext* useContext = callingContext;

    if (_graphicsContext.valid() && _graphicsContext != callingContext)
    {
        useState = _graphicsContext->getState();
        useContext = _graphicsContext.get();
        useContext->makeCurrent();

        glDrawBuffer(GL_FRONT);
        glReadBuffer(GL_FRONT);

    }

    osg::FBOExtensions* fbo_ext = _fbo.valid() ? osg::FBOExtensions::instance(state.getContextID()) : 0;
    bool fbo_supported = fbo_ext && fbo_ext->isSupported();

    if (fbo_supported)
    {
        _fbo->apply(*useState);
    }

    // do the actual rendering of the scene.    
    RenderStage::draw(*useState,previous);

    // now copy the rendered image to attached texture.
    if (_texture.valid() && !fbo_supported)
    {
        if (callingContext && useContext!= callingContext)
        {
            // make the calling context use the pbuffer context for reading.
            callingContext->makeContextCurrent(useContext);

            glReadBuffer(GL_FRONT);
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
    
    if (_image.valid())
    {
        _image->readPixels(_viewport->x(),_viewport->y(),_viewport->width(),_viewport->height(),_imageReadPixelFormat,_imageReadPixelDataType);
    }
       
    
    if (_camera && _camera->getPostDrawCallback())
    {
        // if we have a camera with a post draw callback invoke it.
        (*(_camera->getPostDrawCallback()))(*_camera);
    }

    if (fbo_supported)
    {
        // switch of the frame buffer object
        fbo_ext->glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
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
                itr->second._texture->apply(*useState);
                // fbo_ext->glGenerateMipmapEXT(itr->second._texture->getTextureTarget());
            }
        }
    }

    if (callingContext && useContext != callingContext)
    {
        // restore the graphics context.
        callingContext->makeCurrent();

        glReadBuffer(GL_BACK);
    }
}

