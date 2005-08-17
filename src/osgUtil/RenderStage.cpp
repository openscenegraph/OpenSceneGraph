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

void RenderStage::draw(osg::State& state,RenderLeaf*& previous)
{
    if (_stageDrawnThisFrame) return;

    _stageDrawnThisFrame = true;

    // note, SceneView does call to drawPreRenderStages explicitly
    // so there is no need to call it here.
    drawPreRenderStages(state,previous);
    

    osg::State* useState = &state;
    osg::GraphicsContext* callingContext = state.getGraphicsContext();
    osg::GraphicsContext* useContext = callingContext;

    if (_graphicsContext.valid() && _graphicsContext != callingContext)
    {
        useState = _graphicsContext->getState();
        useContext = _graphicsContext.get();
        useContext->makeCurrent();
    }

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
        _fbo->apply(*useState);
    }


    // do the drawing itself.    
    RenderBin::draw(state,previous);


    // now copy the rendered image to attached texture.
    if (_texture.valid() && !fbo_supported)
    {
        if (callingContext && useContext!= callingContext)
        {
            // make the calling context use the pbuffer context for reading.
            callingContext->makeContextCurrent(useContext);

            if (_readBuffer != GL_NONE)
            {
                glReadBuffer(_readBuffer);
            }
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

        if (_readBuffer != GL_NONE)
        {
            glReadBuffer(_readBuffer);
        }

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
    if (_inheritedRenderStageLighting.valid())
    {
        _inheritedRenderStageLighting->draw(state, previous, &_inheritedRenderStageLightingMatrix);
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
        RenderGraph::moveToRootRenderGraph(state,previous->_parent);
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
