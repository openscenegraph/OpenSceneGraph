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

#include <osg/ImageStream>
#include "VideoFrameDispatcher.h"

#ifdef __OBJC__
@class QTMovie;
#else
class QTMovie;
#endif

class QTVisualContext;
class OSXCoreVideoAdapter;

class OSXQTKitVideo : public osgVideo::VideoImageStream {

public:
    OSXQTKitVideo();
    ~OSXQTKitVideo();
    
    virtual void setTimeMultiplier(double r);
    virtual double getTimeMultiplier() const;
    
    virtual double getCurrentTime() const;
    
    virtual bool isPlaying() const { return (getStatus() == PLAYING); }
    
    virtual bool valid() const { return (getStatus() != INVALID); }
    
    void open(const std::string& file_name);
    
    virtual void setVolume (float);
    virtual float getVolume () const;
    
    virtual float getAudioBalance();
    virtual void setAudioBalance(float b);
    
    virtual double getLength() const { return _duration; }
    
    virtual void seek (double t);
    virtual void play ();
    virtual void pause ();
    
    void setCoreVideoAdapter(OSXCoreVideoAdapter* adapter);
    OSXCoreVideoAdapter* getCoreVideoAdapter() const { return _coreVideoAdapter; }
    
    void decodeFrame(bool force);
    
    virtual void decodeFrame() { decodeFrame(_waitForFirstFrame); }
       
    virtual bool requiresUpdateCall () const { return (!getCoreVideoAdapter() && !getVideoFrameDispatcher() ); }
    
    virtual void update(osg::NodeVisitor *)
    {
        requestNewFrame(_waitForFirstFrame);
    }
    
    void requestNewFrame(bool force)
    {
        if (!setNeedsDispatching(RequestSingleUpdate))
            decodeFrame(force);
        else
            _waitForFirstFrame = true;
    }
    
    virtual bool needsDispatching() const
    {
        return  _waitForFirstFrame || getNeedsDispatching();
    }
    
    static void initializeQTKit();
    
    virtual osg::Texture* createSuitableTexture();
    
protected:
    
    virtual void applyLoopingMode();
    struct Data;
private:
    bool _isActive, _isValid;
    double _duration;
    QTMovie* _movie;
    Data* _data;
    mutable double _rate;
    bool _waitForFirstFrame;
    OSXCoreVideoAdapter* _coreVideoAdapter;


};

