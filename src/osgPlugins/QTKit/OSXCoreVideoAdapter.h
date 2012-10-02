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

#pragma once


#include <CoreVideo/CoreVideo.h>
#include <Quicktime/Quicktime.h>
#include "OSXQTKitVideo.h"


class OSXCoreVideoAdapter : public osg::Referenced {
    
    public:
        OSXCoreVideoAdapter(osg::State& state, osg::Image* image);
                
        void setVideo(osg::Image* image);
        
        void setTimeStamp(const CVTimeStamp* ts) {_timestamp = ts; getFrame();}
        bool getFrame();
        
        inline GLenum getTextureName() { return _currentTexName; }
        inline GLenum getTextureTarget() { return _currentTexTarget; }
        
        QTVisualContextRef getVisualContext() { return _context; }
    
        virtual ~OSXCoreVideoAdapter();
        
    private:
        osg::ref_ptr<OSXQTKitVideo>     _video;
        QTVisualContextRef                _context;
        const CVTimeStamp*                _timestamp;
        CVOpenGLTextureRef                _currentFrame;
        GLint                            _currentTexName;
        GLenum                            _currentTexTarget;

};



