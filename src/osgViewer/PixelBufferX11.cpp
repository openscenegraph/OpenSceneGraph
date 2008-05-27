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

/* Note, elements of PixelBufferX11 have used Prodcer/RenderSurface_X11.cpp as both
 * a guide to use of X11/GLX and copiying directly in the case of setBorder().
 * These elements are license under OSGPL as above, with Copyright (C) 2001-2004  Don Burns.
 */

#include <osgViewer/api/X11/PixelBufferX11>
#include <osgViewer/api/X11/GraphicsWindowX11>
#include <osg/GLExtensions>

#include <X11/Xlib.h>

#include <unistd.h>

using namespace osgViewer;

PixelBufferX11::PixelBufferX11(osg::GraphicsContext::Traits* traits)
  : _valid(false),
    _display(0),
    _pbuffer(0),
    _visualInfo(0),
    _glxContext(0),
    _initialized(false),
    _realized(false),
    _useGLX1_3(false)
{
    _traits = traits;

    init();
    
    if (valid())
    {
        setState( new osg::State );
        getState()->setGraphicsContext(this);

        if (_traits.valid() && _traits->sharedContext)
        {
            getState()->setContextID( _traits->sharedContext->getState()->getContextID() );
            incrementContextIDUsageCount( getState()->getContextID() );   
        }
        else
        {
            getState()->setContextID( osg::GraphicsContext::createNewContextID() );
        }

    }
}

PixelBufferX11::~PixelBufferX11()
{
    close(true);
}

#if defined(GLX_VERSION_1_3) || defined(GLX_SGIX_pbuffer)
bool PixelBufferX11::createVisualInfo()
{
    typedef std::vector<int> Attributes;
    Attributes attributes;
    
    attributes.push_back(GLX_USE_GL);
    
    attributes.push_back(GLX_RGBA);
    
    if (_traits->doubleBuffer) attributes.push_back(GLX_DOUBLEBUFFER);
    
    attributes.push_back(GLX_RED_SIZE); attributes.push_back(_traits->red);
    attributes.push_back(GLX_GREEN_SIZE); attributes.push_back(_traits->green);
    attributes.push_back(GLX_BLUE_SIZE); attributes.push_back(_traits->blue);
    attributes.push_back(GLX_DEPTH_SIZE); attributes.push_back(_traits->depth);
    
    if (_traits->alpha) { attributes.push_back(GLX_ALPHA_SIZE); attributes.push_back(_traits->alpha); }
    
    if (_traits->stencil) { attributes.push_back(GLX_STENCIL_SIZE); attributes.push_back(_traits->stencil); }

#if defined(GLX_SAMPLE_BUFFERS) && defined (GLX_SAMPLES)

    if (_traits->sampleBuffers) { attributes.push_back(GLX_SAMPLE_BUFFERS); attributes.push_back(_traits->sampleBuffers); }
    if (_traits->sampleBuffers) { attributes.push_back(GLX_SAMPLES); attributes.push_back(_traits->samples); }

#endif
    // TODO
    //  GLX_AUX_BUFFERS
    //  GLX_ACCUM_RED_SIZE
    //  GLX_ACCUM_GREEN_SIZE
    
    attributes.push_back(None);
    
    _visualInfo = glXChooseVisual( _display, _traits->screenNum, &(attributes.front()) );

    return _visualInfo != 0;
}

void PixelBufferX11::init()
{
    if (_initialized) return;

    if (!_traits)
    {
        _valid = false;
        return;
    }
    
    if (_traits->target != 0)
    {
        // we don't support Pbuffer render to texture under GLX.
        _valid = false;
        return;
    }


    _display = XOpenDisplay(_traits->displayName().c_str());
    
    unsigned int screen = _traits->screenNum;

    if (!_display)
    {
        osg::notify(osg::NOTICE)<<"Error: Unable to open display \"" << XDisplayName(_traits->displayName().c_str()) << "\"."<<std::endl;
        _valid = false;
        return;
    }

    // Query for GLX extension
    int errorBase, eventBase;
    if( glXQueryExtension( _display, &errorBase, &eventBase)  == False )
    {
        osg::notify(osg::NOTICE)<<"Error: " << XDisplayName(_traits->displayName().c_str()) <<" has no GLX extension." << std::endl;

        XCloseDisplay( _display );
        _display = 0;
        _valid = false;
        return;
    }

    // osg::notify(osg::NOTICE)<<"GLX extension, errorBase="<<errorBase<<" eventBase="<<eventBase<<std::endl;

    int major, minor;
    if (glXQueryVersion(_display, &major, &minor) == False)
    {
        osg::notify(osg::NOTICE) << "Error: " << XDisplayName(_traits->displayName().c_str())
                                 << " can not query GLX version." << std::endl;
        XCloseDisplay( _display );
        _display = 0;
        _valid = false;
        return;
    }

    // Just be paranoid, if we are older than 1.1, we cannot even call glxQueryExtensionString
    if (major < 1 || (1 == major && minor < 1))
    {
        osg::notify(osg::NOTICE) << "Error: " << XDisplayName(_traits->displayName().c_str())
                                 << " GLX version " << major << "." << minor << " is too old." << std::endl;
        XCloseDisplay( _display );
        _display = 0;
        _valid = false;
        return;
    }

    bool haveGLX1_3 = false;
    bool haveSGIX_pbuffer = false;

    // We need to have at least GLX 1.3 to use getFBConfigFromVisual and glXCreatePbuffer
    if (1 < major || (1 == major && 3 <= minor))
    {
        haveGLX1_3 = true;
    }

#if defined(GLX_VERSION_1_1)
    // We need at least GLX 1.1 for glXQueryExtensionsString
    if (!haveGLX1_3 && 1 <= minor)
    {
        const char *extensions = glXQueryExtensionsString(_display, screen);
        haveSGIX_pbuffer = osg::isExtensionInExtensionString("GLX_SGIX_pbuffer", extensions)
           && osg::isExtensionInExtensionString("GLX_SGIX_fbconfig", extensions);
    }
#endif

    if (!haveGLX1_3 && !haveSGIX_pbuffer)
    {
        osg::notify(osg::NOTICE) << "Error: " << XDisplayName(_traits->displayName().c_str())
                                 << " no Pbuffer support in GLX available." << std::endl;
        XCloseDisplay( _display );
        _display = 0;
        _valid = false;
        return;
    }

    
    if (!createVisualInfo())
    {
        _traits->red /= 2; 
        _traits->green /= 2; 
        _traits->blue /= 2; 
        _traits->alpha /= 2; 
        _traits->depth /= 2; 
        
        osg::notify(osg::INFO)<<"Relaxing traits"<<std::endl;

        if (!createVisualInfo())
        {
            osg::notify(osg::NOTICE)<<"Error: Not able to create requested visual." << std::endl;
            XCloseDisplay( _display );
            _display = 0;
            _valid = false;
            return;
        }    
    }
    
    GLXContext sharedContextGLX = NULL;

    // get any shared GLX contexts    
    GraphicsWindowX11* graphicsWindowX11 = dynamic_cast<GraphicsWindowX11*>(_traits->sharedContext);
    if (graphicsWindowX11) 
    {
        sharedContextGLX = graphicsWindowX11->getGLXContext();
    }
    else
    {
        PixelBufferX11* pixelBufferX11 = dynamic_cast<PixelBufferX11*>(_traits->sharedContext);
        if (pixelBufferX11)
        {
            sharedContextGLX = pixelBufferX11->getGLXContext();
        }
    }
    
    _glxContext = glXCreateContext( _display, _visualInfo, sharedContextGLX, True );

    if (!_glxContext)
    {
        osg::notify(osg::NOTICE)<<"Error: Unable to create OpenGL graphics context."<<std::endl;
        XCloseDisplay( _display );
        _display = 0;
        _valid = false;
        return;
    }
    
#ifdef GLX_VERSION_1_3
    // First try the regular glx extension if we have a new enough version available.
    if (haveGLX1_3)
    {
        int nelements;
        GLXFBConfig *fbconfigs = glXGetFBConfigs( _display, screen, &nelements );
        for ( int i = 0; i < nelements; ++i )
        {
            int visual_id;
            if ( glXGetFBConfigAttrib( _display, fbconfigs[i], GLX_VISUAL_ID, &visual_id ) == 0 )
            {
                if ( !_pbuffer && (unsigned int)visual_id == _visualInfo->visualid )
                {
                    typedef std::vector <int> AttributeList;
                   
                    AttributeList attributes;
                    attributes.push_back( GLX_PBUFFER_WIDTH );
                    attributes.push_back( _traits->width );
                    attributes.push_back( GLX_PBUFFER_HEIGHT );
                    attributes.push_back( _traits->height );
                    attributes.push_back( 0L );
                    
                    _pbuffer = glXCreatePbuffer(_display, fbconfigs[i], &attributes.front() );
                    _useGLX1_3 = true;
                }
            }
        }

        XFree( fbconfigs );
    }
#endif

#ifdef GLX_SGIX_pbuffer
    // If we still have no pbuffer but a capable display with the SGIX extension, try to use that
    if (!_pbuffer && haveSGIX_pbuffer)
    {
        GLXFBConfigSGIX fbconfig = glXGetFBConfigFromVisualSGIX( _display, _visualInfo );

        _pbuffer = glXCreateGLXPbufferSGIX(_display, fbconfig, _traits->width, _traits->height, 0 );

        XFree( fbconfig );
    }
#endif

    if (!_pbuffer)
    {
        osg::notify(osg::NOTICE)<<"Error: Unable to create pbuffer."<<std::endl;
        XCloseDisplay( _display );
        _display = 0;
        _glxContext = 0;
        _valid = false;
        return;
    }


    XFlush( _display );
    XSync( _display, 0 );

    _valid = true;
    _initialized = true;
}

void PixelBufferX11::closeImplementation()
{
    // osg::notify(osg::NOTICE)<<"Closing PixelBufferX11"<<std::endl;
    if (_display)
    {
        if (_glxContext)
        {
            glXDestroyContext(_display, _glxContext );
        }
    
        if (_pbuffer)
        {
            if (_useGLX1_3)
            {
#ifdef GLX_VERSION_1_3
                glXDestroyPbuffer(_display, _pbuffer);
#endif
            }
            else
            {
#ifdef GLX_SGIX_pbuffer
                glXDestroyGLXPbufferSGIX(_display, _pbuffer);
#endif
            }
        }

        XFlush( _display );
        XSync( _display,0 );
    }
    
    _pbuffer = 0;
    _glxContext = 0;

    if (_visualInfo)
    {
        XFree(_visualInfo);
        _visualInfo = 0;
    }

    if (_display)
    {
        XCloseDisplay( _display );
        _display = 0;
    }

    _initialized = false;
    _realized = false;
    _valid = false;
}

#else

// fallback for non GLX1.3 versions where pbuffers are not supported.
// note, this makes the rest of the pbuffer code a non op as init is false;
bool PixelBufferX11::createVisualInfo()
{
    return false;
}

void PixelBufferX11::init()
{
}

void PixelBufferX11::closeImplementation()
{
    // osg::notify(osg::NOTICE)<<"Closing PixelBufferX11"<<std::endl;
    _pbuffer = 0;
    _glxContext = 0;
    _initialized = false;
    _realized = false;
    _valid = false;
}

#endif

bool PixelBufferX11::realizeImplementation()
{
    if (_realized)
    {
        osg::notify(osg::NOTICE)<<"PixelBufferX11::realizeImplementation() Already realized"<<std::endl;
        return true;
    }

    if (!_initialized) init();
    
    if (!_initialized) return false;

    _realized = true;

    return true;
}

bool PixelBufferX11::makeCurrentImplementation()
{
    if (!_realized)
    {
        osg::notify(osg::NOTICE)<<"Warning: GraphicsWindow not realized, cannot do makeCurrent."<<std::endl;
        return false;
    }

    // osg::notify(osg::NOTICE)<<"PixelBufferX11::makeCurrentImplementation "<<this<<" "<<OpenThreads::Thread::CurrentThread()<<std::endl;
    // osg::notify(osg::NOTICE)<<"   glXMakeCurrent ("<<_display<<","<<_pbuffer<<","<<_glxContext<<std::endl;

    return glXMakeCurrent( _display, _pbuffer, _glxContext )==True;
}

bool PixelBufferX11::makeContextCurrentImplementation(osg::GraphicsContext* readContext)
{
    // osg::notify(osg::NOTICE)<<"PixelBufferX11::makeContextCurrentImplementation() not implementation yet."<<std::endl;
    return makeCurrentImplementation();
}


bool PixelBufferX11::releaseContextImplementation()
{
    if (!_realized)
    {
        osg::notify(osg::NOTICE)<<"Warning: GraphicsWindow not realized, cannot do makeCurrent."<<std::endl;
        return false;
    }

    // osg::notify(osg::NOTICE)<<"PixelBufferX11::releaseContextImplementation() "<<this<<" "<<OpenThreads::Thread::CurrentThread()<<std::endl;
    // osg::notify(osg::NOTICE)<<"   glXMakeCurrent ("<<_display<<std::endl;

    return glXMakeCurrent( _display, None, NULL )==True;
}


void PixelBufferX11::bindPBufferToTextureImplementation(GLenum buffer)
{
    osg::notify(osg::NOTICE)<<"PixelBufferX11::bindPBufferToTextureImplementation() not implementation yet."<<std::endl;
}

void PixelBufferX11::swapBuffersImplementation()
{
    if (!_realized) return;

    // osg::notify(osg::NOTICE)<<"PixelBufferX11::swapBuffersImplementation "<<this<<" "<<OpenThreads::Thread::CurrentThread()<<std::endl;

    glXSwapBuffers(_display, _pbuffer);
}
