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

using namespace osg;
using namespace osgUtil;

// register a RenderToTextureStage prototype with the RenderBin prototype list.
//RegisterRenderBinProxy<RenderToTextureStage> s_registerRenderToTextureStageProxy;

RenderToTextureStage::RenderToTextureStage()
{
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
        _texture->copyTexImage2D(state,_viewport->x(),_viewport->y(),_viewport->width(),_viewport->height());
    }
    
    if (_image.valid())
    {
        _image->readPixels(_viewport->x(),_viewport->y(),_viewport->width(),_viewport->height(),_imageReadPixelFormat,_imageReadPixelDataType);
    }
       
    if (fbo_supported)
    {
        // switch of the frame buffer object
        fbo_ext->glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    }

    if (callingContext && useContext != callingContext)
    {
        // restore the graphics context.
        callingContext->makeCurrent();

        glReadBuffer(GL_BACK);
    }
}

