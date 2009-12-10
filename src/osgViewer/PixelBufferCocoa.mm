/*
 *  PixelBufferCocoa.cpp
 *  OpenSceneGraph
 *
 *  Created by Stephan Huber on 27.06.08.
 *  Copyright 2008 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#include <iostream>
#include <osgViewer/api/Cocoa/PixelBufferCocoa>
#include <osgViewer/api/Cocoa/GraphicsWindowCocoa>
#include <Cocoa/Cocoa.h>

namespace osgViewer {


void PixelBufferCocoa::init()
{
    //std::cout << "PixelBufferCocoa :: init not implemented yet " << std::endl;
    
    _valid = _initialized = true;    
    

}

bool PixelBufferCocoa::realizeImplementation() 
{
    std::cout << "PixelBufferCocoa :: realizeImplementation not implemented yet " << std::endl;
    
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
         
    NSOpenGLPixelFormatAttribute attr[32];
    int i = 0;
    
    attr[i++] = NSOpenGLPFADepthSize;
    attr[i++] = static_cast<NSOpenGLPixelFormatAttribute>(_traits->depth);

    if (_traits->doubleBuffer) {
        attr[i++] = NSOpenGLPFADoubleBuffer;
    }
    
    if (_traits->alpha) { 
        attr[i++] = NSOpenGLPFAAlphaSize;
        attr[i++] = static_cast<NSOpenGLPixelFormatAttribute>(_traits->alpha);
    }

    if (_traits->stencil) {
        attr[i++] = NSOpenGLPFAStencilSize;
        attr[i++] = static_cast<NSOpenGLPixelFormatAttribute>(_traits->stencil);
    }
  

    if (_traits->sampleBuffers) {
        attr[i++] = NSOpenGLPFASampleBuffers;
        attr[i++] = static_cast<NSOpenGLPixelFormatAttribute>(_traits->sampleBuffers);
        attr[i++] = NSOpenGLPFASamples;
        attr[i++] = static_cast<NSOpenGLPixelFormatAttribute>(_traits->samples);
    }

    attr[i++] = NSOpenGLPFAPixelBuffer; // for pbuffer usage
    attr[i++] = NSOpenGLPFAAccelerated;
    attr[i] = static_cast<NSOpenGLPixelFormatAttribute>(0);
    
    // create the context
    NSOpenGLContext* sharedContext = NULL;
    
    GraphicsHandleCocoa* graphicsHandleCocoa = dynamic_cast<GraphicsHandleCocoa*>(_traits->sharedContext);
    if (graphicsHandleCocoa) 
    {
        sharedContext = graphicsHandleCocoa->getNSOpenGLContext();
    }
    
    
    NSOpenGLPixelFormat* pixelformat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attr ];
    _context = [[NSOpenGLContext alloc] initWithFormat: pixelformat shareContext: sharedContext];
    NSOpenGLPixelBuffer* pbuffer = [[NSOpenGLPixelBuffer alloc] initWithTextureTarget: _traits->target textureInternalFormat: _traits->format textureMaxMipMapLevel: _traits->level pixelsWide: _traits->width pixelsHigh: _traits->height];
    
    [_context setPixelBuffer: pbuffer cubeMapFace: _traits->face mipMapLevel:_traits->level currentVirtualScreen: nil];
    
    [pool release];
    
    _realized = (_context != nil);
    return _realized;
}


void PixelBufferCocoa::closeImplementation()
{
    _realized = false;
    
    
}



/** Make this graphics context current.*/
bool PixelBufferCocoa::makeCurrentImplementation()
{
    // osg::notify(osg::INFO) << "PixelBufferCocoa::makeCurrentImplementation" << std::endl;
    
    [_context makeCurrentContext];
    return true;
}


/** Make this graphics context current with specified read context implementation. */
bool PixelBufferCocoa::makeContextCurrentImplementation(osg::GraphicsContext* readContext)
{
    return makeCurrentImplementation();
}

/** Release the graphics context.*/
bool PixelBufferCocoa::releaseContextImplementation()
{
    // osg::notify(osg::INFO) << "PixelBufferCocoa::releaseContextImplementation" << std::endl;
    
    [NSOpenGLContext clearCurrentContext];
    return true;
}

/** Bind the graphics context to associated texture implementation.*/
void PixelBufferCocoa::bindPBufferToTextureImplementation( GLenum buffer )
{
    std::cout << "PixelBufferCocoa :: bindPBufferToTextureImplementation not implemented yet " << std::endl;
}

/** Swap the front and back buffers.*/
void PixelBufferCocoa::swapBuffersImplementation()
{
    osg::notify(osg::INFO) << "PixelBufferCocoa::swapBuffersImplementation" << std::endl;
    [_context flushBuffer];
}

 PixelBufferCocoa::~PixelBufferCocoa()
 {    
    [_context release];
 }




}

