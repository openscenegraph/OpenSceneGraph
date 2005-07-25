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

#include <osgProducer/GraphicsContextImplementation>
#include <osg/TextureRectangle>
#include <osg/TextureCubeMap>

using namespace osgProducer;

namespace osgProducer
{
    struct MyCreateGraphicContexCallback : public osg::GraphicsContext::CreateGraphicContexCallback
    {
        virtual osg::GraphicsContext* createGraphicsContext(osg::GraphicsContext::Traits* traits)
        {
            return new GraphicsContextImplementation(traits);
        }
    };

    struct RegisterCreateGraphicsContextCallbackProxy
    {
        RegisterCreateGraphicsContextCallbackProxy()
        {
            osg::GraphicsContext::setCreateGraphicsContextCallback(new MyCreateGraphicContexCallback);
        }
    };
    
    RegisterCreateGraphicsContextCallbackProxy createGraphicsContextCallbackProxy;
};
    


GraphicsContextImplementation::GraphicsContextImplementation(Traits* traits)
{
    _traits = traits;

    _rs = new Producer::RenderSurface;
    _rs->setWindowName(traits->_windowName);
    _rs->setWindowRectangle(traits->_x, traits->_y, traits->_width, traits->_height);
    _rs->useBorder(traits->_windowDecoration);
 
    if (traits->_pbuffer)
    {
        _rs->setDrawableType(Producer::RenderSurface::DrawableType_PBuffer);

        if (traits->_target)
        {

            _rs->setRenderToTextureOptions(Producer::RenderSurface::RenderToTextureOptions_Default);
            _rs->setRenderToTextureMipMapLevel(traits->_level);
            _rs->setRenderToTextureMode(traits->_alpha>0 ? Producer::RenderSurface::RenderToRGBATexture : 
                                                           Producer::RenderSurface::RenderToRGBTexture);

            switch(traits->_target)
            {
                case(GL_TEXTURE_1D) : 
                    _rs->setRenderToTextureTarget(Producer::RenderSurface::Texture1D);
                    break;
                case(GL_TEXTURE_2D) : 
                    _rs->setRenderToTextureTarget(Producer::RenderSurface::Texture2D);
                    break;
                case(GL_TEXTURE_3D) :
                    // not supported. 
                    // _rs->setRenderToTextureTarget(Producer::RenderSurface::Texture3D);
                    break;
                case(GL_TEXTURE_RECTANGLE) : 
                    // not supported.
                    // _rs->setRenderToTextureTarget(Producer::RenderSurface::TextureRectangle);
                    break;
                case(GL_TEXTURE_CUBE_MAP_POSITIVE_X) : 
                case(GL_TEXTURE_CUBE_MAP_NEGATIVE_X) : 
                case(GL_TEXTURE_CUBE_MAP_POSITIVE_Y) : 
                case(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y) : 
                case(GL_TEXTURE_CUBE_MAP_POSITIVE_Z) : 
                case(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z) : 
                    _rs->setRenderToTextureTarget(Producer::RenderSurface::TextureCUBE);
                    _rs->setRenderToTextureFace( Producer::RenderSurface::CubeMapFace(traits->_target - GL_TEXTURE_CUBE_MAP_POSITIVE_X));
                    break;
            }

        }
        
    }
    
    GraphicsContextImplementation* sharedContext = dynamic_cast<GraphicsContextImplementation*>(traits->_sharedContext);

    if (sharedContext)
    {
        // different graphics context so we have our own state.
        setState(new osg::State);
        
        if (sharedContext->getState())
        {
            getState()->setContextID( sharedContext->getState()->getContextID() );
            incrementContextIDUsageCount( sharedContext->getState()->getContextID() );   
        }
        else
        {
            getState()->setContextID( GraphicsContext::createNewContextID() );
        }
        
        // but we share texture objects etc. so we also share the same contextID
        _rs->realize( 0, sharedContext->_rs->getGLContext() );
        
    }
    else
    {
    
        // need to do something here....    
        setState( new osg::State );
        getState()->setContextID( GraphicsContext::createNewContextID() );

        _rs->realize();
    }

}

GraphicsContextImplementation::GraphicsContextImplementation(Producer::RenderSurface* rs)
{
    _rs = rs;
}

GraphicsContextImplementation::~GraphicsContextImplementation()
{
    release();
}

void GraphicsContextImplementation::makeCurrent()
{
    if (!_rs) return;

    _rs->setReadDrawable( 0 );

    _rs->makeCurrent();
}

void GraphicsContextImplementation::makeContextCurrent(GraphicsContext* readContext)
{
    if (!_rs) return;

    GraphicsContextImplementation* readContextImplemention = dynamic_cast<GraphicsContextImplementation*>(readContext);

    if (readContextImplemention)
    {
        _rs->setReadDrawable( readContextImplemention->getRenderSurface() );
    }

    _rs->makeCurrent();
}

void GraphicsContextImplementation::release()
{
    if (!_rs) return;
    
    // need to close render surface... 
}

void GraphicsContextImplementation::bindPBufferToTexture(GLenum buffer)
{
    if (!_rs) return;
 
    Producer::RenderSurface::BufferType bufferType = Producer::RenderSurface::FrontBuffer;
    switch(buffer)
    {
        case(GL_BACK): bufferType = Producer::RenderSurface::BackBuffer; break;
        case(GL_FRONT): bufferType = Producer::RenderSurface::FrontBuffer; break;
        default: bufferType = Producer::RenderSurface::FrontBuffer; break;
    }

    _rs->bindPBufferToTexture(bufferType);
}

void GraphicsContextImplementation::swapBuffers()
{
    _rs->swapBuffers();
}
