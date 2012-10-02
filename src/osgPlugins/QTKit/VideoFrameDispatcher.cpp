
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

#include "VideoFrameDispatcher.h"
#include <iostream>
#include <osg/Timer>

namespace osgVideo {

VideoImageStream::VideoImageStream()
    : osg::ImageStream()
    , _needsDispatching(false)
    , _dispatcher(NULL)
    , _queue(NULL)
{
}

VideoImageStream::VideoImageStream(const VideoImageStream& image,const osg::CopyOp& copyop)
    : osg::ImageStream(image, copyop)
    , _needsDispatching(image._needsDispatching)
    , _dispatcher(image._dispatcher)
    , _queue(NULL)
{
}

VideoImageStream::~VideoImageStream()
{
    setNeedsDispatching(StopUpdate);
    _dispatcher = NULL;
}

bool VideoImageStream::setNeedsDispatching(RequestMode request_mode)
{
    _needsDispatching = (_needsDispatching || (request_mode == RequestContinuousUpdate)) && (request_mode != StopUpdate);
    if (!_dispatcher)
        return false;
    
    if (request_mode == StopUpdate) {
        _dispatcher->removeFromQueue(this);
    }
    else
    {
        _dispatcher->addToQueue(this);
    }
    
    return (_dispatcher != NULL);
}

#pragma mark 

VideoFrameDispatchQueue::VideoFrameDispatchQueue()
    : OpenThreads::Thread()
    , osg::Referenced()
    , _queue()
    , _numItems(0)
    , _block()
    , _mutex()
    , _finished(false)
{
}

void VideoFrameDispatchQueue::run()
{
    osg::Timer t;
    static unsigned int frame_delay = 1000 * 1000 / 120;
    
    _block.reset();
    _block.block();
    
    while(!_finished)
    {
        unsigned int num_items(0);
        {
            osg::Timer_t last_tick(t.tick());
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
            for(Queue::iterator i = _queue.begin(); i != _queue.end(); )
            {
                osg::observer_ptr<VideoImageStream> stream(*i);
                
                if (stream.valid() && stream->needsDispatching())
                {
                    if (stream.valid())
                        stream->decodeFrame();
                    ++num_items;
                    ++i;
                }
                else
                {
                    if (stream.valid())
                        stream->setDispatchQueue(NULL);
                    _queue.erase(i++);
                }
                
            }
            _numItems = num_items;
            if (_numItems > 0)
            {
                unsigned int dt = t.delta_u(last_tick, t.tick());
                if (dt < frame_delay) {
                    OpenThreads::Thread::microSleep(frame_delay - dt);
                }
            }
        }
        
        if (_numItems == 0)
        {
            // std::cout << this << " blocking" << std::endl;
            _block.reset();
            _block.block();
        }
        
    }
}

void VideoFrameDispatchQueue::addItem(osgVideo::VideoImageStream *stream)
{
    if (_finished) return;
    
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _queue.insert(stream);
    stream->setDispatchQueue(this);
    
    _numItems = _queue.size();
    _block.release();
    // std::cout << this << " release" << std::endl;
}

void VideoFrameDispatchQueue::removeItem(osgVideo::VideoImageStream* stream)
{
    stream->setDispatchQueue(NULL);
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _queue.erase(stream);
    _numItems = _queue.size();
}

VideoFrameDispatchQueue::~VideoFrameDispatchQueue()
{
    _finished = true;
    _block.release();
    join();
}

#pragma mark

VideoFrameDispatcher::VideoFrameDispatcher(unsigned int num_threads)
    : osg::Referenced()
    , _queues()
{
    num_threads = num_threads ? num_threads : OpenThreads::GetNumberOfProcessors();
    OSG_ALWAYS << "VideoFrameDispatcher: creating " << num_threads << " queues." << std::endl;
    for(unsigned int i = 0; i < num_threads; ++i)
    {
        VideoFrameDispatchQueue* q = new VideoFrameDispatchQueue();
        q->start();
        _queues.push_back(q);
    }
}


void VideoFrameDispatcher::addToQueue(VideoImageStream *stream)
{
    stream->setThreadSafeRefUnref(true);
    if (stream->getDispatchQueue())
        return;
    
    VideoFrameDispatchQueue* queue = *std::min_element(_queues.begin(), _queues.end(), VideoFrameDispatchQueueComparator());
    queue->addItem(stream);
}

void VideoFrameDispatcher::removeFromQueue(VideoImageStream* stream)
{
   if (stream->getDispatchQueue())
        stream->getDispatchQueue()->removeItem(stream);
}

}