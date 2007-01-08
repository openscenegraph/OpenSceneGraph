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
    struct MyWindowingSystemInterface : public osg::GraphicsContext::WindowingSystemInterface
    {
        virtual unsigned int getNumScreens(const osg::GraphicsContext::ScreenIdentifier& /*screenIdentifier*/) 
        {
            return Producer::RenderSurface::getNumberOfScreens();
        }

        virtual void getScreenResolution(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier, unsigned int& width, unsigned int& height)
        {
            osg::ref_ptr<Producer::RenderSurface> rs = new Producer::RenderSurface;
            rs->setHostName(screenIdentifier.hostName);
            rs->setDisplayNum(screenIdentifier.displayNum);
            rs->setScreenNum(screenIdentifier.screenNum);
            rs->getScreenSize(width, height);
        }


        virtual osg::GraphicsContext* createGraphicsContext(osg::GraphicsContext::Traits* traits)
        {
            return new GraphicsContextImplementation(traits);
        }
    };

    struct RegisterWindowingSystemInterfaceProxy
    {
        RegisterWindowingSystemInterfaceProxy()
        {
            osg::GraphicsContext::setWindowingSystemInterface(new MyWindowingSystemInterface);
        }
        
        ~RegisterWindowingSystemInterfaceProxy()
        {
            osg::GraphicsContext::setWindowingSystemInterface(0);
        }
    };
    
    RegisterWindowingSystemInterfaceProxy createWindowingSystemInterfaceProxy;
};
    


GraphicsContextImplementation::GraphicsContextImplementation(Traits* traits)
{
    _traits = traits;

    _rs = new Producer::RenderSurface;
    _rs->setWindowName(traits->windowName);
    _rs->setWindowRectangle(traits->x, traits->y, traits->width, traits->height);
    _rs->useBorder(traits->windowDecoration);
    _rs->setDisplayNum(traits->displayNum);
    _rs->setScreenNum(traits->screenNum);
    

    // set the visual chooser
    Producer::VisualChooser* rs_vc = _rs->getVisualChooser();
    if (!rs_vc)
    {
        rs_vc = new Producer::VisualChooser;
        _rs->setVisualChooser(rs_vc);
    }
    
    rs_vc->setRedSize(_traits->red);
    rs_vc->setGreenSize(_traits->green);
    rs_vc->setBlueSize(_traits->blue);
    rs_vc->setAlphaSize(_traits->alpha);
    
    rs_vc->setDepthSize(_traits->depth);
    rs_vc->setStencilSize(_traits->stencil);
    
    if (_traits->doubleBuffer) rs_vc->useDoubleBuffer();

    rs_vc->addAttribute( Producer::VisualChooser::RGBA );

    // Always use UseGL
    rs_vc->addAttribute( Producer::VisualChooser::UseGL );
 
    if (traits->pbuffer)
    {
        _rs->setDrawableType(Producer::RenderSurface::DrawableType_PBuffer);

        if (traits->target)
        {

            _rs->setRenderToTextureOptions(traits->mipMapGeneration ? Producer::RenderSurface::RequestSpaceForMipMaps :
                                                                       Producer::RenderSurface::RenderToTextureOptions_Default);
            _rs->setRenderToTextureMipMapLevel(traits->level);
            _rs->setRenderToTextureMode(traits->alpha>0 ? Producer::RenderSurface::RenderToRGBATexture : 
                                                           Producer::RenderSurface::RenderToRGBTexture);

            switch(traits->target)
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
                    _rs->setRenderToTextureFace( Producer::RenderSurface::CubeMapFace(traits->target - GL_TEXTURE_CUBE_MAP_POSITIVE_X));
                    break;
            }

        }
        
    }
    
    GraphicsContextImplementation* sharedContext = dynamic_cast<GraphicsContextImplementation*>(traits->sharedContext);

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
    _traits->windowName = _rs->getWindowName();
    _traits->displayNum = _rs->getDisplayNum();
    _traits->screenNum = _rs->getScreenNum();
}

GraphicsContextImplementation::~GraphicsContextImplementation()
{
    if (_closeOnDestruction) close();
}

bool GraphicsContextImplementation::realizeImplementation()
{
    if (_rs.valid()) 
    {
        GraphicsContextImplementation* sharedContext = dynamic_cast<GraphicsContextImplementation*>(_traits->sharedContext);

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

bool GraphicsContextImplementation::makeCurrentImplementation()
{
    if (!_rs)
    {
        osg::notify(osg::NOTICE)<<"Error: GraphicsContextImplementation::makeCurrentImplementation() no RenderSurface."<<std::endl;
        return false;
    }

    if (!isRealized())
    {
        osg::notify(osg::NOTICE)<<"Error: GraphicsContextImplementation::makeCurrentImplementation() not Realized."<<std::endl;
        return false;
    }

//    osg::notify(osg::INFO)<<"GraphicsContextImplementation::makeCurrentImplementation()"<<std::endl;

    _rs->setReadDrawable( 0 );

    // comment out right now, as Producer's setReadDrawable() is doing a call for us.
    // _rs->makeCurrent();
    
    return true;
}

bool GraphicsContextImplementation::makeContextCurrentImplementation(osg::GraphicsContext* readContext)
{
    if (!_rs) return false;

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
    
    return true;
}

bool GraphicsContextImplementation::releaseContextImplementation()
{
    osg::notify(osg::NOTICE)<<"GraphicsContextImplementation::releaseContextImplementation(): not implemented - release not supported under Producer."<<std::endl;
    return false;
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
