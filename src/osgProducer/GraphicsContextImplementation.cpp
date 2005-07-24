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
#include <osg/Notify>

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
        _rs->setDrawableType( Producer::RenderSurface::DrawableType_PBuffer );
        
        if (traits->_alpha>0)
        {
            _rs->setRenderToTextureMode(Producer::RenderSurface::RenderToRGBATexture);
        }
        else
        {
            _rs->setRenderToTextureMode(Producer::RenderSurface::RenderToRGBTexture);
        }
    }
    
    setState(new osg::State);
    getState()->setContextID(1);
    
    _rs->realize();

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
