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

#include <osgProducer/GraphicsContextImplementation>
#include <osg/TextureRectangle>
#include <osg/TextureCubeMap>
#include <osg/Notify>

using namespace osgProducer;

namespace osgProducer
{
    struct MyCreateGraphicContexCallback : public osg::GraphicsContext::CreateGraphicContextCallback
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
        
        ~RegisterCreateGraphicsContextCallbackProxy()
        {
            osg::GraphicsContext::setCreateGraphicsContextCallback(0);
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
    _rs->setDisplayNum(traits->_displayNum);
    _rs->setScreenNum(traits->_screenNum);
    

    // set the visual chooser
    Producer::VisualChooser* rs_vc = _rs->getVisualChooser();
    if (!rs_vc)
    {
        rs_vc = new Producer::VisualChooser;
        _rs->setVisualChooser(rs_vc);
    }
    
    rs_vc->setRedSize(_traits->_red);
    rs_vc->setGreenSize(_traits->_green);
    rs_vc->setBlueSize(_traits->_blue);
    rs_vc->setAlphaSize(_traits->_alpha);
    
    rs_vc->setDepthSize(_traits->_depth);
    rs_vc->setStencilSize(_traits->_stencil);
    
    if (_traits->_doubleBuffer) rs_vc->useDoubleBuffer();

    rs_vc->addAttribute( Producer::VisualChooser::RGBA );

    // Always use UseGL
    rs_vc->addAttribute( Producer::VisualChooser::UseGL );
 
    if (traits->_pbuffer)
    {
        _rs->setDrawableType(Producer::RenderSurface::DrawableType_PBuffer);

        if (traits->_target)
        {

            _rs->setRenderToTextureOptions(traits->_mipMapGeneration ? Producer::RenderSurface::RequestSpaceForMipMaps :
                                                                       Producer::RenderSurface::RenderToTextureOptions_Default);
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
                    osg::notify(osg::NOTICE)<<"PBuffer render to Texture3D not supported."<<std::endl;
                    // not supported. 
                    // _rs->setRenderToTextureTarget(Producer::RenderSurface::Texture3D);
                    break;
                case(GL_TEXTURE_RECTANGLE) : 
                    osg::notify(osg::NOTICE)<<"PBuffer render to TextureRectangle not supported."<<std::endl;
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
            getState()->setContextID( osg::GraphicsContext::createNewContextID() );
        }
        
        // but we share texture objects etc. so we also share the same contextID
        //_rs->realize( 0, sharedContext->_rs->getGLContext() );
        
    }
    else
    {
    
        // need to do something here....    
        setState( new osg::State );
        getState()->setContextID( osg::GraphicsContext::createNewContextID() );

        //_rs->realize();
    }
    
    // _rs->useConfigEventThread(false);

    _closeOnDestruction = true;
}

GraphicsContextImplementation::GraphicsContextImplementation(Producer::RenderSurface* rs)
{
    _rs = rs;
    _closeOnDestruction = false;

    _traits = new osg::GraphicsContext::Traits;
    _traits->_windowName = _rs->getWindowName();
    _traits->_displayNum = _rs->getDisplayNum();
    _traits->_screenNum = _rs->getScreenNum();
}

GraphicsContextImplementation::~GraphicsContextImplementation()
{
    if (_closeOnDestruction) close();
}

bool GraphicsContextImplementation::realizeImplementation()
{
    if (_rs.valid()) 
    {
        GraphicsContextImplementation* sharedContext = dynamic_cast<GraphicsContextImplementation*>(_traits->_sharedContext);

        if (sharedContext)
        {
            _rs->realize( 0, sharedContext->_rs->getGLContext() );
        }
        else
        {
            osg::notify(osg::NOTICE)<<"GraphicsContextImplementation::realize"<<std::endl;

            _rs->realize();
        }
        return _rs->isRealized();
    }
    else
    {
        return false;
    }
}

void GraphicsContextImplementation::makeCurrentImplementation()
{
    if (!_rs)
    {
        osg::notify(osg::NOTICE)<<"Error: GraphicsContextImplementation::makeCurrentImplementation() no RenderSurface."<<std::endl;
        return;
    }

    if (!isRealized())
    {
        osg::notify(osg::NOTICE)<<"Error: GraphicsContextImplementation::makeCurrentImplementation() not Realized."<<std::endl;
        return;
    }

//    osg::notify(osg::INFO)<<"GraphicsContextImplementation::makeCurrentImplementation()"<<std::endl;

    _rs->setReadDrawable( 0 );

    // comment out right now, as Producer's setReadDrawable() is doing a call for us.
    // _rs->makeCurrent();
}

void GraphicsContextImplementation::makeContextCurrentImplementation(osg::GraphicsContext* readContext)
{
    if (!_rs) return;

    GraphicsContextImplementation* readContextImplemention = dynamic_cast<GraphicsContextImplementation*>(readContext);

    if (readContextImplemention)
    {
        _rs->setReadDrawable( readContextImplemention->getRenderSurface() );
    }
    else
    {
        _rs->setReadDrawable( 0 );
    }

    // comment out right now, as Producer's setReadDrawable() is doing a call for us.
    // _rs->makeCurrent();
}

void GraphicsContextImplementation::closeImplementation()
{
    if (!_rs) return;
    
    // close render surface by deleting it 
    _rs = 0;
}

void GraphicsContextImplementation::bindPBufferToTextureImplementation(GLenum buffer)
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

void GraphicsContextImplementation::swapBuffersImplementation()
{
    _rs->swapBuffers();
}
