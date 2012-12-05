//
//  OSXAVFoundationVideo.h
//  cefix_presentation_ios
//
//  Created by Stephan Maximilian Huber on 25.07.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

#pragma once



#include <osg/Timer>
#include "../QTKit/VideoFrameDispatcher.h"


class OSXAVFoundationVideo : public osgVideo::VideoImageStream {

public:
    OSXAVFoundationVideo();
            
    /// Destructor
    ~OSXAVFoundationVideo();
        
    virtual Object* clone() const { return new OSXAVFoundationVideo(); }
    virtual bool isSameKindAs(const Object* obj) const {
        return dynamic_cast<const OSXAVFoundationVideo*>(obj) != NULL;
    }
    
    virtual const char* className() const { return "OSXAVFoundationVideo"; }

    /// Start or continue stream.
    virtual void play();
    
    /** @return true, if a movie is playing */
    
    bool isPlaying() const { return (getStatus() == PLAYING); }
    
    /// sets the movierate
    void setTimeMultiplier(double rate);
    
    /// gets the movierate
    double getTimeMultiplier() const;
    
    /// Pause stream at current position.
    virtual void pause();

    
    /// stop playing 
    virtual void quit(bool /*waitForThreadToExit*/ = true);

    /// Get total length in seconds.
    virtual double getLength() const { return _videoDuration; }
    
    /// jumps to a specific position 
    virtual void seek(double pos);
    
    virtual void rewind() {
        seek(0);
    }
    
      
    /// returns the current playing position
    virtual double     getCurrentTime () const;
    
    
    void open(const std::string& filename);
    
    /** @return the current volume as float */
    virtual float getVolume() const;
    
    /** sets the volume of this quicktime to v*/
    virtual void setVolume(float v);
    
    /** @return the current balance-setting (0 = neutral, -1 = left, 1 = right */
    virtual float getAudioBalance();    
    /** sets the current balance-setting (0 = neutral, -1 = left, 1 = right */
    virtual void setAudioBalance(float b);
    
    virtual double getFrameRate () const { return _framerate; }
            
    virtual void decodeFrame();
    
    virtual bool valid() const { return (getStatus() != INVALID); }
    
    virtual bool requiresUpdateCall () const { return true; }
    
    virtual void update(osg::NodeVisitor *);
    
    virtual void applyLoopingMode();
    
    void setUseCoreVideo(bool b) { _useCoreVideo = b; }
    bool isCoreVideoUsed() const { return _useCoreVideo; }
    void lazyInitCoreVideoTextureCache(osg::State& state);
    bool getCurrentCoreVideoTexture(GLenum& target, GLint& name, int& width, int& height) const;
    
    virtual osg::Texture* createSuitableTexture();

protected:
   
    virtual bool needsDispatching() const;
    
    void requestNewFrame();
private:
    class Data;
    
    void clear();
    
    float _videoDuration;
    double _volume;
    bool _fileOpened, _waitForFrame;
    
    Data* _data;
    bool _useCoreVideo, _dimensionsChangedCallbackNeeded;
    double _framerate;
 
};

