#include <cassert>
//#include <osgDB/ReadFile>

#include "RenderToTextureStage.h"

//using namespace osg;
//using namespace osgUtil;

// register a RenderToTextureStage prototype with the RenderBin prototype list.
//RegisterRenderBinProxy<RenderToTextureStage> s_registerRenderToTextureStageProxy;

MyRenderToTextureStage::MyRenderToTextureStage()
{
    _pbuffer = 0L;
    _localState = new osg::State;
}

MyRenderToTextureStage::~MyRenderToTextureStage()
{
}

void MyRenderToTextureStage::reset()
{
    RenderStage::reset();
}

void MyRenderToTextureStage::draw(osg::State& state, osgUtil::RenderLeaf*& previous)
{
    if (_pbuffer && _texture.valid())
    {
        // Create pbuffer texture
        const unsigned int contextID = state.getContextID();
        osg::Texture::TextureObject* textureObject = _texture->getTextureObject(contextID);
        if (textureObject == 0)
        {
            // Create dynamic texture, subload callback required.
            _texture->apply(state);
        }
    
        HDC hdc = ::wglGetCurrentDC();
        HGLRC hglrc = ::wglGetCurrentContext();

        // Release pbuffer from "render to texture".
        _pbuffer->releaseTexImage();

        // Make the p-buffer's context current.
        _pbuffer->makeCurrent();

        // Render in p-buffer.
        RenderStage::draw(*_localState,previous);

        // restore window's context as current.
        if (!::wglMakeCurrent(hdc, hglrc))
        {
            assert(0);
        }

        if (true /*_isRenderTextureSupported*/)
        {
            // transfer contents of p-buffer to texture
            _pbuffer->bindTexImage(textureObject->_id);
        }
        else
        {
// TODO:
//          _pbuffer->copyTexImage(state);
        }

    }
    else
    {
        RenderStage::draw(state,previous);

        // now copy the rendered image to attached texture.
        if (_texture.valid())
            _texture->copyTexImage2D(state,_viewport->x(),_viewport->y(),_viewport->width(),_viewport->height());
    
        if (_image.valid())
            _image->readPixels(_viewport->x(),_viewport->y(),_viewport->width(),_viewport->height(),GL_RGBA,GL_UNSIGNED_BYTE);
    }
}
