/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2008 Robert Osfield
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

#include "OSXCoreVideoAdapter.h"
#include <osg/GL>
#include <osg/State>
#include <osgViewer/api/Cocoa/GraphicsWindowCocoa>
#import <Cocoa/Cocoa.h>




OSXCoreVideoAdapter::OSXCoreVideoAdapter(osg::State& state, osg::Image* image) :
    osg::Referenced(),
    _context(NULL),
    _timestamp(NULL),
    _currentFrame(NULL),
    _currentTexTarget(GL_TEXTURE_RECTANGLE_EXT)
{
    setVideo(image);
    if (!_video.valid())
        return;
    
    
    CGLContextObj cglcntx(NULL);
    CGLPixelFormatObj cglPixelFormat;
    OSStatus err = noErr;
        
    if (cglcntx == NULL) {
        osgViewer::GraphicsWindowCocoa* win = dynamic_cast<osgViewer::GraphicsWindowCocoa*>(state.getGraphicsContext());
        if (win) 
        {
            NSOpenGLContext* context = win->getContext();
            cglcntx = (CGLContextObj)[context CGLContextObj];
            cglPixelFormat = (CGLPixelFormatObj)[ win->getPixelFormat() CGLPixelFormatObj];
        }
    }
    
    
    if ((cglcntx == NULL) || (err != noErr)) {
        OSG_WARN <<"CoreVideoTexture: could not get Context/Pixelformat " << err << std::endl;
        return;
    }
    
    CFTypeRef keys[] = { kQTVisualContextWorkingColorSpaceKey };
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CFDictionaryRef textureContextAttributes = CFDictionaryCreate(kCFAllocatorDefault,
                                                                  (const void **)keys,
                                                                  (const void **)&colorSpace, 1,
                                                                  &kCFTypeDictionaryKeyCallBacks,
                                                                  &kCFTypeDictionaryValueCallBacks);
    
    err = QTOpenGLTextureContextCreate(kCFAllocatorDefault, cglcntx, cglPixelFormat, textureContextAttributes, &_context);

    setVideo(_video.get());
    setTimeStamp(NULL);
}



OSXCoreVideoAdapter::~OSXCoreVideoAdapter()
{
    setVideo(NULL);
    
    if (_currentFrame) {
        CVOpenGLTextureRelease(_currentFrame);
        _currentFrame = NULL;
    }

    // release the OpenGL Texture Context
    if (_context) {
         CFRelease(_context);
         _context = NULL;
    }

}


 void OSXCoreVideoAdapter::setVideo(osg::Image* image)
 {
    if (_video.valid()) {
        _video->setCoreVideoAdapter(NULL);
    }
    _video = dynamic_cast<OSXQTKitVideo*>(image);
    
    if ((_context) && (_video.valid()))
    {
        _video->setCoreVideoAdapter(this);
        setTimeStamp(NULL);
    }    
}



bool OSXCoreVideoAdapter::getFrame()
{
    QTVisualContextTask(_context);
    bool b = QTVisualContextIsNewImageAvailable(_context, _timestamp);
    if (b){
        
        CVOpenGLTextureRef newFrame;
        QTVisualContextCopyImageForTime(_context, kCFAllocatorDefault, _timestamp, &newFrame);
        
        if (_currentFrame)
            CVOpenGLTextureRelease(_currentFrame);
        
        _currentFrame = newFrame;
        
        _currentTexTarget = CVOpenGLTextureGetTarget(_currentFrame);
        _currentTexName =  CVOpenGLTextureGetName(_currentFrame);
    }
    //std::cerr << _movie->getFileName() << ": " << b << " / " << _movie->isPlaying() << " " << _movie->getCurrentTime() << std::endl;
    return b;
}


