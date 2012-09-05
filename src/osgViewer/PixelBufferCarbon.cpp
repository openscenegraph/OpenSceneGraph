/*
 *  PixelBufferCarbon.cpp
 *  OpenSceneGraph
 *
 *  Created by Stephan Huber on 27.06.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#if defined (__APPLE__) && (!__LP64__)

#include <osg/observer_ptr>
#include <osgViewer/api/Carbon/PixelBufferCarbon>
#include <osgViewer/api/Carbon/GraphicsWindowCarbon>
#include <Carbon/Carbon.h>
#include <OpenGL/OpenGL.h>
using namespace osgViewer;




/** creates a pixelformat from a Trait */
AGLPixelFormat PixelBufferCarbon::createPixelFormat(osg::GraphicsContext::Traits* traits) {

    std::vector<GLint> attributes;

    attributes.push_back(AGL_NO_RECOVERY);
    attributes.push_back(AGL_RGBA);
    if (!traits->pbuffer)
        attributes.push_back(AGL_COMPLIANT);
    else
        attributes.push_back(AGL_CLOSEST_POLICY);

    if (traits->doubleBuffer) attributes.push_back(AGL_DOUBLEBUFFER);
    if (traits->quadBufferStereo) attributes.push_back(AGL_STEREO);

    attributes.push_back(AGL_RED_SIZE); attributes.push_back(traits->red);
    attributes.push_back(AGL_GREEN_SIZE); attributes.push_back(traits->green);
    attributes.push_back(AGL_BLUE_SIZE); attributes.push_back(traits->blue);
    attributes.push_back(AGL_DEPTH_SIZE); attributes.push_back(traits->depth);

    if (traits->alpha) { attributes.push_back(AGL_ALPHA_SIZE); attributes.push_back(traits->alpha); }

    if (traits->stencil) { attributes.push_back(AGL_STENCIL_SIZE); attributes.push_back(traits->stencil); }

    // TODO
    // missing accumulation-buffer-stuff

#if defined(AGL_SAMPLE_BUFFERS_ARB) && defined (AGL_SAMPLES_ARB)

    if (traits->sampleBuffers) { attributes.push_back(AGL_SAMPLE_BUFFERS_ARB); attributes.push_back(traits->sampleBuffers); }
    if (traits->sampleBuffers) { attributes.push_back(AGL_SAMPLES_ARB); attributes.push_back(traits->samples); }

#endif
    attributes.push_back(AGL_NONE);

    return aglChoosePixelFormat(NULL, 0, &(attributes.front()));
}


void PixelBufferCarbon::init()
{
    _context = NULL;
    _pixelformat = PixelBufferCarbon::createPixelFormat(_traits.get());
    if (!_pixelformat)
        OSG_WARN << "PixelBufferCarbon::init could not create a valid pixelformat" << std::endl;
    _valid = (_pixelformat != NULL);
}


/** This is the class we need to create for pbuffers, note its not a GraphicsWindow as it won't need any of the event handling and window mapping facilities.*/
/** Realise the GraphicsContext implementation,
          * Pure virtual - must be implemented by concrate implementations of GraphicsContext. */
bool PixelBufferCarbon::realizeImplementation()
{
    if (!_valid) {
        OSG_WARN << "PixelBufferCarbon::realizeImplementation() aglChoosePixelFormat failed! " << aglErrorString(aglGetError()) << std::endl;
        return false;
    }

    AGLContext sharedContext = NULL;

    // get any shared AGL contexts
    GraphicsHandleCarbon* graphicsHandleCarbon = dynamic_cast<GraphicsHandleCarbon*>(_traits->sharedContext.get());
    if (graphicsHandleCarbon)
    {
        sharedContext = graphicsHandleCarbon->getAGLContext();
    }

    _context = aglCreateContext (_pixelformat, sharedContext);

    if (!_context) {
        OSG_WARN << "PixelBufferCarbon::realizeImplementation() aglCreateContext failed! " << aglErrorString(aglGetError()) << std::endl;
        return false;
    }



    _realized = aglCreatePBuffer (_traits->width, _traits->height, _traits->target, GL_RGBA, _traits->level, &(_pbuffer));
    if (!_realized) {
        OSG_WARN << "PixelBufferCarbon::realizeImplementation() aglCreatePBuffer failed! " << aglErrorString(aglGetError()) << std::endl;
    }

    makeCurrentImplementation();

    _realized = aglSetPBuffer(_context, _pbuffer, _traits->face, _traits->level, 0);
    if (!_realized) {
        OSG_WARN << "PixelBufferCarbon::realizeImplementation() aglSetPBuffer failed! " << aglErrorString(aglGetError()) << std::endl;
    }
    return _realized;
}

void  PixelBufferCarbon::closeImplementation()
{
    if (_pbuffer) aglDestroyPBuffer(_pbuffer);
    if (_context) aglDestroyContext(_context);
    if (_pixelformat) aglDestroyPixelFormat(_pixelformat);

    _pbuffer = NULL;
    _context = NULL;
    _pixelformat = NULL;

    _valid = _realized = false;
}

/** Make this graphics context current implementation.
  * Pure virtual - must be implemented by concrate implementations of GraphicsContext. */
bool  PixelBufferCarbon::makeCurrentImplementation()
{
    return (_realized) ? (aglSetCurrentContext(_context) == GL_TRUE) : false;
}

/** Make this graphics context current with specified read context implementation.
  * Pure virtual - must be implemented by concrate implementations of GraphicsContext. */
bool  PixelBufferCarbon::makeContextCurrentImplementation(GraphicsContext* /*readContext*/)  {
    return makeCurrentImplementation();
}

/** Release the graphics context.*/
bool PixelBufferCarbon::releaseContextImplementation()
{
     return (aglSetCurrentContext(NULL) == GL_TRUE);
}


/** Pure virtual, Bind the graphics context to associated texture implementation.
  * Pure virtual - must be implemented by concrate implementations of GraphicsContext. */
void PixelBufferCarbon::bindPBufferToTextureImplementation( GLenum buffer ){

    OSG_NOTICE<<"GraphicsWindow::void bindPBufferToTextureImplementation(..) not implemented."<<std::endl;
}

/** Swap the front and back buffers implementation.
  * Pure virtual - must be implemented by Concrate implementations of GraphicsContext. */
void PixelBufferCarbon::swapBuffersImplementation()
{
     aglSwapBuffers(_context);
}


PixelBufferCarbon::~PixelBufferCarbon()
{
    close(true);
}



#endif
