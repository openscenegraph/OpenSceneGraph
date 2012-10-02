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

#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>
#include <osg/ImageStream>
#include <osg/observer_ptr>
#include <OpenThreads/Block>

namespace osgVideo {

class VideoFrameDispatchQueue;
class VideoFrameDispatcher;


class VideoImageStream : public osg::ImageStream {
public:
    enum RequestMode { RequestContinuousUpdate, RequestSingleUpdate, StopUpdate };
    
    VideoImageStream();
    VideoImageStream(const VideoImageStream& image,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    ~VideoImageStream();
    virtual bool needsDispatching() const { return _needsDispatching; }
    
    virtual void decodeFrame() = 0;
    
protected:

    bool setNeedsDispatching(RequestMode request_mode);
    VideoFrameDispatcher* getVideoFrameDispatcher() const { return _dispatcher; }
    void setVideoFrameDispatcher(VideoFrameDispatcher* dispatcher) { _dispatcher = dispatcher; }
    bool getNeedsDispatching() const { return _needsDispatching; }
    
    void setDispatchQueue(VideoFrameDispatchQueue* queue) { _queue = queue; }
    VideoFrameDispatchQueue* getDispatchQueue() const { return _queue; }

private:
    bool _needsDispatching;
    VideoFrameDispatcher* _dispatcher;
    VideoFrameDispatchQueue* _queue;
    
friend class VideoFrameDispatcher;
friend class VideoFrameDispatchQueue;
};

class VideoFrameDispatchQueue: public OpenThreads::Thread, public osg::Referenced {
public:
    typedef std::set< osg::observer_ptr<VideoImageStream> > Queue;
    
    VideoFrameDispatchQueue();
    ~VideoFrameDispatchQueue();
    unsigned int getNumItemsInQueue() const { return _numItems; }
    
    void addItem(VideoImageStream* stream);
    void removeItem(osgVideo::VideoImageStream* stream);

    virtual void run();
            
private:
    Queue _queue;
    unsigned int _numItems;
    OpenThreads::Block _block;
    OpenThreads::Mutex _mutex;
    bool _finished;
};

struct VideoFrameDispatchQueueComparator {
    bool operator() (const osg::ref_ptr<VideoFrameDispatchQueue>& lhs, const osg::ref_ptr<VideoFrameDispatchQueue>& rhs) const {
        return lhs->getNumItemsInQueue() < rhs->getNumItemsInQueue();
    }
};


class VideoFrameDispatcher : public osg::Referenced {

public:
    
    typedef std::vector< osg::ref_ptr<VideoFrameDispatchQueue> > DispatchQueues;
    
    VideoFrameDispatcher(unsigned int num_threads = 0);
   
    void addVideo(VideoImageStream* stream)
    {
        stream->setVideoFrameDispatcher(this);
    }
    
    void addToQueue(VideoImageStream* stream);
    void removeFromQueue(VideoImageStream* stream);
    
private:
    DispatchQueues _queues;

};

}
