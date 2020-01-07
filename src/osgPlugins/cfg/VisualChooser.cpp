/* -*-c++-*- Producer - Copyright (C) 2001-2004  Don Burns
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
#include <iostream>

#include "VisualChooser.h"
#include <osg/Referenced>

using namespace std;
using namespace osgProducer;


VisualChooser::VisualChooser( void )
{
    _visual_id = 0;
    _strictAdherence = false;
}

VisualChooser::~VisualChooser(void)
{
   clear();
}

#if 0
void VisualChooser::setVisual( VisualInfo *vinfo )
{
    clear();
    _vinfo = vinfo;
}
#endif
void VisualChooser::resetVisualInfo()
{
#ifdef _WIN32_IMPLEMENTATION
    if (_vinfo != NULL) delete _vinfo;
#endif

#ifdef _OSX_AGL_IMPLEMENTATION
    if(_vinfo != NULL) free(_vinfo);
#endif

    //    _vinfo = 0L;
}

void VisualChooser::addAttribute( AttributeName attribute )
{
    resetVisualInfo();
    _visual_attributes.push_back( VisualAttribute( attribute ) );
}

void VisualChooser::addAttribute( AttributeName attribute, int parameter )
{
    resetVisualInfo();
    _visual_attributes.push_back( VisualAttribute( attribute, parameter ) );
}

void VisualChooser::addExtendedAttribute( unsigned int attribute )
{
    resetVisualInfo();
    _visual_attributes.push_back( VisualAttribute( attribute ) );
}

void VisualChooser::addExtendedAttribute( unsigned int attribute, int parameter )
{
    resetVisualInfo();
    _visual_attributes.push_back( VisualAttribute( attribute, parameter ) );
}

void VisualChooser::setBufferSize( unsigned int size )
{
    addAttribute( BufferSize, size );
}

void VisualChooser::setLevel( int level )
{
    addAttribute( Level, level );

}
void VisualChooser::useRGBA()
{
    addAttribute( RGBA );
}
void VisualChooser::useDoubleBuffer()
{
    addAttribute( DoubleBuffer );
}
void VisualChooser::useStereo()
{
    addAttribute( Stereo );
}

void VisualChooser::setAuxBuffers( unsigned int num )
{
    addAttribute( AuxBuffers, num );
}

void VisualChooser::setRedSize( unsigned int size )
{
    addAttribute( RedSize, size );
}

void VisualChooser::setGreenSize( unsigned int size )
{
    addAttribute( GreenSize, size );
}

void VisualChooser::setBlueSize( unsigned int size )
{
    addAttribute( BlueSize, size );
}

void VisualChooser::setAlphaSize( unsigned int size )
{
    addAttribute( AlphaSize, size );
}

void VisualChooser::setDepthSize( unsigned int size )
{
    addAttribute( DepthSize, size );
}

void VisualChooser::setStencilSize( unsigned int size )
{
    addAttribute( StencilSize, size );
}

void VisualChooser::setAccumRedSize( unsigned int size )
{
    addAttribute( AccumRedSize, size );
}

void VisualChooser::setAccumGreenSize( unsigned int size )
{
    addAttribute( AccumGreenSize, size );
}

void VisualChooser::setAccumBlueSize( unsigned int size )
{
    addAttribute( AccumBlueSize, size );
}

void VisualChooser::setAccumAlphaSize( unsigned int size )
{
    addAttribute( AccumAlphaSize, size );
}

void VisualChooser::setSamples( unsigned int size )
{
    addAttribute( Samples, size );
}

void VisualChooser::setSampleBuffers( unsigned int size )
{
    addAttribute( SampleBuffers, size );
}

void VisualChooser::setVisualID( unsigned int id )
{
    _visual_id = id;
}

void VisualChooser::setSimpleConfiguration( bool doublebuffer )
{
    clear();
    addAttribute( RGBA );
    addAttribute( DepthSize, 24 );
    if (doublebuffer)
        addAttribute( DoubleBuffer );
}

void VisualChooser::clear()
{
    _visual_attributes.clear();
    resetVisualInfo();

    // Always use UseGL
    addAttribute( UseGL );
}


#ifdef _OSX_AGL_IMPLEMENTATION
#include <AGL/agl.h>

void VisualChooser::applyAttribute(const VisualAttribute &va, std::vector<int> &attribs)
{
    if(va.isExtension())
    {
      attribs.push_back(static_cast<int>(va.attribute()));
    }
    else
    {
      switch(va.attribute())
      {
        case UseGL:          attribs.push_back(AGL_COMPLIANT); break;
        case BufferSize:     attribs.push_back(AGL_BUFFER_SIZE); break;
        case Level:          attribs.push_back(AGL_LEVEL); break;
        case RGBA:           attribs.push_back(AGL_RGBA); break;
        case DoubleBuffer:   attribs.push_back(AGL_DOUBLEBUFFER); break;
        case Stereo:         attribs.push_back(AGL_STEREO); break;
        case AuxBuffers:     attribs.push_back(AGL_AUX_BUFFERS); break;
        case RedSize:        attribs.push_back(AGL_RED_SIZE); break;
        case GreenSize:      attribs.push_back(AGL_GREEN_SIZE); break;
        case BlueSize:       attribs.push_back(AGL_BLUE_SIZE); break;
        case AlphaSize:      attribs.push_back(AGL_ALPHA_SIZE); break;
        case DepthSize:      attribs.push_back(AGL_DEPTH_SIZE); break;
        case StencilSize:    attribs.push_back(AGL_STENCIL_SIZE); break;
        case AccumRedSize:   attribs.push_back(AGL_ACCUM_RED_SIZE); break;
        case AccumGreenSize: attribs.push_back(AGL_ACCUM_GREEN_SIZE); break;
        case AccumBlueSize:  attribs.push_back(AGL_ACCUM_BLUE_SIZE); break;
        case AccumAlphaSize: attribs.push_back(AGL_ACCUM_ALPHA_SIZE); break;
#if defined (AGL_SAMPLE_BUFFERS_ARB) && defined (AGL_SAMPLES_ARB)
        case SampleBuffers:  attribs.push_back(AGL_SAMPLE_BUFFERS_ARB); break;
        case Samples:        attribs.push_back(AGL_SAMPLES_ARB); break;
#endif
        default: attribs.push_back((int)va.attribute()); break;
      }
    }

    if (va.hasParameter())
    {
      attribs.push_back(va.parameter());
    }
}

VisualInfo *VisualChooser::choose( Display *dpy, int screen, bool strict_adherence)
{
    if(_vinfo != NULL)
    {
      // If VisualInfo exists, then we may be able to reuse it
      GLint val;
      if((*_vinfo) && (aglDescribePixelFormat(*_vinfo, AGL_NO_RECOVERY, &val))) return _vinfo;
    }
    else
    {
      // Use malloc() since new() causes a bus error
      _vinfo = (VisualInfo*)malloc(sizeof(VisualInfo*));
    }
    // Set up basic attributes if needed
    if( _visual_attributes.size() == 0 ) setSimpleConfiguration();

    bool fullscreen = (screen == 1); // Whether to fullscreen or not
    std::vector<int> va; // Visual attributes

    va.push_back(AGL_NO_RECOVERY);
    if(fullscreen) va.push_back(AGL_FULLSCREEN);

    vector<VisualAttribute>::const_iterator p;
    for( p = _visual_attributes.begin(); p != _visual_attributes.end(); p++ )
    {
      applyAttribute(*p, va);
    }
    va.push_back(AGL_NONE); // Must be last element

    // Copy to GLint vector, since this is what the agl functions require
    std::vector<GLint> glva;
    std::vector<int>::iterator i;
    for(i = va.begin(); i != va.end(); i++)
    {
      glva.push_back(*i);
    }

    if(fullscreen)
    {
      GDHandle gdhDisplay;
      DMGetGDeviceByDisplayID((DisplayIDType)*dpy, &gdhDisplay, false);
      *_vinfo = aglChoosePixelFormat(&gdhDisplay, 1, &(glva.front()));
    }
    else
    {
      *_vinfo = aglChoosePixelFormat(NULL, 0, &(glva.front()));
    }

    if(*_vinfo == NULL)
    {
      std::cerr<< "aglChoosePixelFormat failed: " << aglGetError() << std::endl;
      for(i=va.begin(); i!=va.end(); ++i)
      {
        std::cerr << (*i) << ", ";
      }
      std::cerr << std::endl;
    }
    return _vinfo;
}

#endif

#ifdef _OSX_CGL_IMPLEMENTATION
void VisualChooser::applyAttribute(const VisualAttribute &va, std::vector<int> &attribs)
{
    if(va.isExtension())
    {
      attribs.push_back(static_cast<int>(va.attribute()));
    }
    else
    {
      switch(va.attribute())
      {
        case UseGL:          attribs.push_back(kCGLPFACompliant); break;
        case BufferSize:     attribs.push_back(kCGLPFAColorSize); break;
        //case Level:          attribs.push_back(GLX_LEVEL); break;
        //case RGBA:           attribs.push_back(GLX_RGBA); break;
        case DoubleBuffer:   attribs.push_back(kCGLPFADoubleBuffer); break;
        case Stereo:         attribs.push_back(kCGLPFAStereo); break;
        case AuxBuffers:     attribs.push_back(kCGLPFAAuxBuffers); break;
        case RedSize:        attribs.push_back(kCGLPFAColorSize); break;
        case GreenSize:      attribs.push_back(kCGLPFAColorSize); break;
        case BlueSize:       attribs.push_back(kCGLPFAColorSize); break;
        case AlphaSize:      attribs.push_back(kCGLPFAAlphaSize); break;
        case DepthSize:      attribs.push_back(kCGLPFADepthSize); break;
        case StencilSize:    attribs.push_back(kCGLPFAStencilSize); break;
        case AccumRedSize:   attribs.push_back(kCGLPFAAccumSize); break;
        case AccumGreenSize: attribs.push_back(kCGLPFAAccumSize); break;
        case AccumBlueSize:  attribs.push_back(kCGLPFAAccumSize); break;
        case AccumAlphaSize: attribs.push_back(kCGLPFAAccumSize); break;
        case SampleBuffers:  attribs.push_back(kCGLPFASampleBuffers); break;
        case Samples:        attribs.push_back(kCGLPFASamples); break;
        default: attribs.push_back(va.attribute()); break;
      }
    }

    if (va.hasParameter())
    {
      attribs.push_back(va.parameter());
    }
}

VisualInfo *VisualChooser::choose( Display *dpy, int screen, bool strict_adherence)
{
    if(_vinfo != NULL)
    {
      // If VisualInfo exists, then we may be able to reuse it
      GLint val;
      if(!CGLDescribePixelFormat(*_vinfo, 0L, kCGLPFANoRecovery, &val))
        return _vinfo;
    }

    // Set up basic attributes if needed
    if( _visual_attributes.size() == 0 ) setSimpleConfiguration();

    u_int32_t display_id = CGDisplayIDToOpenGLDisplayMask(*dpy);
    std::vector<int> va; // Visual attributes

    va.push_back(kCGLPFADisplayMask);
    va.push_back((CGLPixelFormatAttribute)display_id);
    va.push_back(kCGLPFANoRecovery);
    va.push_back( kCGLPFAAccelerated );
    va.push_back( kCGLPFAFullScreen );

    vector<VisualAttribute>::const_iterator p;
    for( p = _visual_attributes.begin(); p != _visual_attributes.end(); p++ )
    {
      applyAttribute(*p, va);
    }
    va.push_back(CGLPixelFormatAttribute(0)); // Must be last element

    long nvirt;
    CGLPixelFormatAttribute *array = new CGLPixelFormatAttribute[va.size()];
    memcpy(array, &(va.front()), va.size()*sizeof(CGLPixelFormatAttribute));
    CGLError err = CGLChoosePixelFormat(array, _vinfo, &nvirt);
    if(err)
    {
      std::cerr<< "CGLChoosePixelFormat failed: " << CGLErrorString(err) << std::endl;
      std::vector<int>::iterator i;
      for(i=va.begin(); i!=va.end(); ++i)
      {
        std::cerr << (*i) << ", ";
      }
      std::cerr << std::endl;
    }

    return _vinfo;
}

#endif

#ifdef _X11_IMPLEMENTATION
#include <GL/glx.h>

void VisualChooser::applyAttribute(const VisualAttribute &va, std::vector<int> &attribs)
{
    if (va.isExtension())
    {
        attribs.push_back(static_cast<int>(va.attribute()));
    }
    else
    {
        switch (va.attribute())
        {
            case UseGL:          attribs.push_back(GLX_USE_GL); break;
            case BufferSize:     attribs.push_back(GLX_BUFFER_SIZE); break;
            case Level:          attribs.push_back(GLX_LEVEL); break;
            case RGBA:           attribs.push_back(GLX_RGBA); break;
            case DoubleBuffer:   attribs.push_back(GLX_DOUBLEBUFFER); break;
            case Stereo:         attribs.push_back(GLX_STEREO); break;
            case AuxBuffers:     attribs.push_back(GLX_AUX_BUFFERS); break;
            case RedSize:        attribs.push_back(GLX_RED_SIZE); break;
            case GreenSize:      attribs.push_back(GLX_GREEN_SIZE); break;
            case BlueSize:       attribs.push_back(GLX_BLUE_SIZE); break;
            case AlphaSize:      attribs.push_back(GLX_ALPHA_SIZE); break;
            case DepthSize:      attribs.push_back(GLX_DEPTH_SIZE); break;
            case StencilSize:    attribs.push_back(GLX_STENCIL_SIZE); break;
            case AccumRedSize:   attribs.push_back(GLX_ACCUM_RED_SIZE); break;
            case AccumGreenSize: attribs.push_back(GLX_ACCUM_GREEN_SIZE); break;
            case AccumBlueSize:  attribs.push_back(GLX_ACCUM_BLUE_SIZE); break;
            case AccumAlphaSize: attribs.push_back(GLX_ACCUM_ALPHA_SIZE); break;
#if defined(GLX_SAMPLE_BUFFERS) && defined (GLX_SAMPLES)
            case SampleBuffers:  attribs.push_back(GLX_SAMPLE_BUFFERS); break;
            case Samples:        attribs.push_back(GLX_SAMPLES); break;
#endif
            default:             attribs.push_back(static_cast<int>(va.attribute())); break;
        }
    }

    if (va.hasParameter())
    {
        attribs.push_back(va.parameter());
    }
}


                                                           // we now ignore this...
VisualInfo *VisualChooser::choose( Display *dpy, int screen, bool strict_adherence)
{
    // Visual info was previously forced.
    if( _vinfo != NULL )
        return _vinfo;

    if( _visual_id != 0 )
    {

        VisualInfo temp;
        long mask = VisualIDMask;
        int n;
        temp.visualid = _visual_id;
        _vinfo = XGetVisualInfo( dpy, mask, &temp, &n );

    if( _vinfo != NULL || _strictAdherence )
        return _vinfo;

    }

    if( _visual_attributes.size() == 0 )
        setSimpleConfiguration();

    vector<VisualAttribute>::const_iterator p;
    vector<int>va;

    for( p = _visual_attributes.begin(); p != _visual_attributes.end(); p++ )
    {
        applyAttribute(*p, va);
    }
    va.push_back(0);

    if( _strictAdherence )
    {
    _vinfo = glXChooseVisual( dpy, screen, &(va.front()) );
    }
    else
    {
        p = _visual_attributes.end() - 1;

        while( _vinfo == NULL && va.size() > 0)
        {
            _vinfo = glXChooseVisual( dpy, screen, &(va.front()) );
        if( _vinfo == NULL && va.size() > 0 )
        {

                // should we report an message that we're relaxing constraints here?
                // std::cout << "Popping attributes"<<std::endl;

            va.pop_back(); // Pop the NULL terminator
            if( (*p).hasParameter() && va.size() >= 2 )
            {
            va.pop_back();
            va.pop_back();
            }
            else
            va.pop_back();
            va.push_back(0); // Push a new NULL terminator
        if( p == _visual_attributes.begin() )
            break;
            p --;
        }
        }
    }

    if( _vinfo != 0L )
        _visual_id = _vinfo->visualid;
    else
        _visual_id = 0;

    return  _vinfo;
}
#endif

#ifdef _WIN32_IMPLEMENTATION
#include "WGLExtensions.h"

void VisualChooser::applyAttribute(const VisualAttribute &va, std::vector<int> &attribs)
{
    if (va.attribute() == UseGL)
    {
        attribs.push_back(WGL_SUPPORT_OPENGL_ARB);
        attribs.push_back(1);
        attribs.push_back(WGL_ACCELERATION_ARB);
        attribs.push_back(WGL_FULL_ACCELERATION_ARB);
        return;
    }

    if (va.attribute() == DoubleBuffer)
    {
        attribs.push_back(WGL_DOUBLE_BUFFER_ARB);
        attribs.push_back(1);
        attribs.push_back(WGL_SWAP_METHOD_ARB);
        attribs.push_back(WGL_SWAP_EXCHANGE_ARB);
        return;
    }

    // please note that *all* WGL attributes must have a parameter!
    // to keep compatibility we'll explicitly set a boolean
    // parameter value where needed.

    std::pair<int, int> attr = std::make_pair(static_cast<int>(va.attribute()), va.parameter());

    switch (va.attribute())
    {
        case Level:          return;
        case BufferSize:     attr.first = WGL_COLOR_BITS_ARB; break;
        case RGBA:           attr.first = WGL_PIXEL_TYPE_ARB; attr.second = WGL_TYPE_RGBA_ARB; break;
        case Stereo:         attr.first = WGL_STEREO_ARB; attr.second = 1;  break;
        case AuxBuffers:     attr.first = WGL_AUX_BUFFERS_ARB; break;
        case RedSize:        attr.first = WGL_RED_BITS_ARB; break;
        case GreenSize:      attr.first = WGL_GREEN_BITS_ARB; break;
        case BlueSize:       attr.first = WGL_BLUE_BITS_ARB; break;
        case AlphaSize:      attr.first = WGL_ALPHA_BITS_ARB; break;
        case DepthSize:      attr.first = WGL_DEPTH_BITS_ARB; break;
        case StencilSize:    attr.first = WGL_STENCIL_BITS_ARB; break;
        case AccumRedSize:   attr.first = WGL_ACCUM_RED_BITS_ARB; break;
        case AccumGreenSize: attr.first = WGL_ACCUM_GREEN_BITS_ARB; break;
        case AccumBlueSize:  attr.first = WGL_ACCUM_BLUE_BITS_ARB; break;
        case AccumAlphaSize: attr.first = WGL_ACCUM_ALPHA_BITS_ARB; break;
#if defined(WGL_SAMPLE_BUFFERS_ARB) && defined (WGL_SAMPLES_ARB)
        case SampleBuffers:  attr.first = WGL_SAMPLE_BUFFERS_ARB; break;
        case Samples:        attr.first = WGL_SAMPLES_ARB; break;
#endif
        default: ;
    }

    attribs.push_back(attr.first);
    attribs.push_back(attr.second);
}

VisualInfo *VisualChooser::choose( Display *dpy, int screen, bool strict_adherence)
{
    if (_vinfo)
        return _vinfo;

    if (_visual_attributes.empty())
        setSimpleConfiguration();

    int vid;
    bool failed = false;

    WGLExtensions *ext = WGLExtensions::instance();
    if (ext && ext->isSupported(WGLExtensions::ARB_pixel_format))
    {
        do
        {
            std::vector<int> attribs;
            for (std::vector<VisualAttribute>::const_iterator i=_visual_attributes.begin(); i!=_visual_attributes.end(); ++i)
                applyAttribute(*i, attribs);
            attribs.push_back(0);

            unsigned int num_formats;
            failed = !ext->wglChoosePixelFormat(*dpy, &attribs.front(), 0, 1, &vid, &num_formats) || num_formats == 0;
            if (failed)
            {
                // **** DRIVER BUG? It seems that some ATI cards don't support
                // **** the WGL_SWAP_METHOD_ARB attribute. Now we try to remove
                // **** it from the attribute list and choose a pixel format again.
                for (std::vector<int>::iterator k=attribs.begin(); k!=attribs.end(); ++k)
                {
                    if (*k == WGL_SWAP_METHOD_ARB)
                    {
                        // attribute come in sequential attribute,parameter pairs
                        // as WGL specifications so we need to delete two entries
                        attribs.erase(k,k+2);
                        break;
                    }
                }

                failed = !ext->wglChoosePixelFormat(*dpy, &attribs.front(), 0, 1, &vid, &num_formats) || num_formats == 0;
            }

            if (failed)
            {
                if (strict_adherence || _visual_attributes.empty())
                    break;

                std::cerr << "Producer::VisualChooser: the requested visual is not available, trying to relax attributes..." << std::endl;
                _visual_attributes.pop_back();
            }

        } while (failed);
    }

    if (failed || !ext || !ext->isSupported(WGLExtensions::ARB_pixel_format))
    {
        std::cerr << "Producer::VisualChooser: unable to setup a valid visual with WGL extensions, switching to compatibility mode" << std::endl;

        failed = false;
        do
        {
            PIXELFORMATDESCRIPTOR pfd;
            ZeroMemory(&pfd, sizeof(pfd));
            pfd.nSize = sizeof(pfd);
            pfd.nVersion = 1;
            pfd.dwFlags = PFD_DRAW_TO_WINDOW;
            pfd.iLayerType = PFD_MAIN_PLANE;

            for (std::vector<VisualAttribute>::const_iterator i=_visual_attributes.begin(); i!=_visual_attributes.end(); ++i)
            {
                if (i->attribute() == UseGL)             pfd.dwFlags |= PFD_SUPPORT_OPENGL;
                if (i->attribute() == DoubleBuffer)      pfd.dwFlags |= PFD_DOUBLEBUFFER | PFD_SWAP_EXCHANGE;
                if (i->attribute() == Stereo)            pfd.dwFlags |= PFD_STEREO;
                if (i->attribute() == RGBA)              pfd.iPixelType = PFD_TYPE_RGBA;
                if (i->attribute() == BufferSize)        pfd.cColorBits = i->parameter();
                if (i->attribute() == RedSize)           pfd.cRedBits = i->parameter();
                if (i->attribute() == GreenSize)         pfd.cGreenBits = i->parameter();
                if (i->attribute() == BlueSize)          pfd.cBlueBits = i->parameter();
                if (i->attribute() == AlphaSize)         pfd.cAlphaBits = i->parameter();
                if (i->attribute() == AccumRedSize)      pfd.cAccumRedBits = i->parameter();
                if (i->attribute() == AccumGreenSize)    pfd.cAccumGreenBits = i->parameter();
                if (i->attribute() == AccumBlueSize)     pfd.cAccumBlueBits = i->parameter();
                if (i->attribute() == AccumAlphaSize)    pfd.cAccumAlphaBits = i->parameter();
                if (i->attribute() == DepthSize)         pfd.cDepthBits = i->parameter();
                if (i->attribute() == StencilSize)       pfd.cStencilBits = i->parameter();
                if (i->attribute() == AuxBuffers)        pfd.cAuxBuffers = i->parameter();
            }

            pfd.cAccumBits = pfd.cAccumRedBits + pfd.cAccumGreenBits + pfd.cAccumBlueBits + pfd.cAccumAlphaBits;

            vid = ChoosePixelFormat(*dpy, &pfd);
            if ( vid != 0 )
            {
              // Is this additional check necessary ?
              // Did anyone encountered a situation where
              // ChoosePixelFormat returned PXIELFORMAT worse than required ?
              _visual_id = static_cast<unsigned int>(vid);
              VisualInfo pfd;
              DescribePixelFormat(*dpy, _visual_id, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
              bool boolOK = true;
              for (std::vector<VisualAttribute>::const_iterator i=_visual_attributes.begin(); i!=_visual_attributes.end(); ++i)
              {
                  if (i->attribute() == UseGL)             boolOK &= !!( pfd.dwFlags & PFD_SUPPORT_OPENGL );
                  if (i->attribute() == DoubleBuffer)      boolOK &= !!( pfd.dwFlags & ( PFD_DOUBLEBUFFER | PFD_SWAP_EXCHANGE ) );
                  if (i->attribute() == Stereo)            boolOK &= !!( pfd.dwFlags & PFD_STEREO );
                  if (i->attribute() == RGBA)              boolOK &= pfd.iPixelType == PFD_TYPE_RGBA;
                  if (i->attribute() == BufferSize)        boolOK &= pfd.cColorBits >= i->parameter();
                  if (i->attribute() == RedSize)           boolOK &= pfd.cRedBits >= i->parameter();
                  if (i->attribute() == GreenSize)         boolOK &= pfd.cGreenBits >= i->parameter();
                  if (i->attribute() == BlueSize)          boolOK &= pfd.cBlueBits >= i->parameter();
                  if (i->attribute() == AlphaSize)         boolOK &= pfd.cAlphaBits >= i->parameter();
                  if (i->attribute() == AccumRedSize)      boolOK &= pfd.cAccumRedBits >= i->parameter();
                  if (i->attribute() == AccumGreenSize)    boolOK &= pfd.cAccumGreenBits >= i->parameter();
                  if (i->attribute() == AccumBlueSize)     boolOK &= pfd.cAccumBlueBits >= i->parameter();
                  if (i->attribute() == AccumAlphaSize)    boolOK &= pfd.cAccumAlphaBits >= i->parameter();
                  if (i->attribute() == DepthSize)         boolOK &= pfd.cDepthBits >= i->parameter();
                  if (i->attribute() == StencilSize)       boolOK &= pfd.cStencilBits >= i->parameter();
                  if (i->attribute() == AuxBuffers)        boolOK &= pfd.cAuxBuffers >= i->parameter();
              }
              if ( !boolOK )
                vid = 0;
            }

            if( vid == 0 )
            {
                failed = true;
                if (strict_adherence || _visual_attributes.empty())
                    break;

                std::cerr << "Producer::VisualChooser: the requested visual is not available, trying to relax attributes..." << std::endl;
                _visual_attributes.pop_back();
            }

        } while (failed);

    }

    if (failed)
    {
        std::cerr << "Producer::VisualChooser: could not choose the pixel format" << std::endl;
        return 0;
    }

    _visual_id = static_cast<unsigned int>(vid);

    // we set _vinfo for compatibility, but it's going to be unused
    // because the pixel format is usually chosen by visual ID rather
    // than by descriptor.
    _vinfo = new VisualInfo;
    DescribePixelFormat(*dpy, _visual_id, sizeof(PIXELFORMATDESCRIPTOR), _vinfo);

    return _vinfo;
}

#endif

#if 0
unsigned int VisualChooser::getVisualID()  const
{
    return 0;
}

bool VisualChooser::getStrictAdherence()
{
    return _strictAdherence;
}

void VisualChooser::setStrictAdherence(bool strictAdherence)
{
    _strictAdherence = strictAdherence;
}
#endif

bool VisualChooser::isDoubleBuffer() const
{
    for (std::vector<VisualAttribute>::const_iterator i=_visual_attributes.begin(); i!=_visual_attributes.end(); ++i)
        if (i->attribute() == DoubleBuffer)
            return true;

    return false;
}
