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

#include <osgDB/ImagePager>
#include <osgDB/ReadFile>

#include <osg/Notify>
#include <osg/ImageSequence>

using namespace osgDB;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SortFileRequestFunctor
//
struct ImagePager::SortFileRequestFunctor
{
    bool operator() (const osg::ref_ptr<ImagePager::ImageRequest>& lhs,const osg::ref_ptr<ImagePager::ImageRequest>& rhs) const
    {
        return (lhs->_timeToMergeBy < rhs->_timeToMergeBy);
    }
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ReadQueue
//
void ImagePager::RequestQueue::sort()
{
    std::sort(_requestList.begin(),_requestList.end(),SortFileRequestFunctor());
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ReadQueue
//
ImagePager::ReadQueue::ReadQueue(ImagePager* pager, const std::string& name):
    _pager(pager),
    _name(name)
{
    _block = new osg::RefBlock;
}

void ImagePager::ReadQueue::clear()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);

    for(RequestList::iterator citr = _requestList.begin();
        citr != _requestList.end();
        ++citr)
    {
        (*citr)->_objectToAttachTo = 0;
        (*citr)->_requestQueue = 0;
    }

    _requestList.clear();

    updateBlock();
}

void ImagePager::ReadQueue::add(ImagePager::ImageRequest* databaseRequest)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);
    _requestList.push_back(databaseRequest);
    databaseRequest->_requestQueue = this;

    updateBlock();
}

void ImagePager::ReadQueue::takeFirst(osg::ref_ptr<ImageRequest>& databaseRequest)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);

    if (!_requestList.empty())
    {
        sort();
    
        databaseRequest = _requestList.front();
        databaseRequest->_requestQueue = 0;
        _requestList.erase(_requestList.begin());

        updateBlock();
    }
}

//////////////////////////////////////////////////////////////////////////////////////
//
// ImageThread
//
ImagePager::ImageThread::ImageThread(ImagePager* pager, Mode mode, const std::string& name):
    _done(false),
    _mode(mode),
    _pager(pager),
    _name(name)
{
}

ImagePager::ImageThread::ImageThread(const ImageThread& dt, ImagePager* pager):
    _done(false),
    _mode(dt._mode),
    _pager(pager),
    _name(dt._name)
{
}

ImagePager::ImageThread::~ImageThread()
{
}

int ImagePager::ImageThread::cancel()
{
    int result = 0;

    if( isRunning() )
    {
    
        _done = true;
        
        switch(_mode)
        {
            case(HANDLE_ALL_REQUESTS):
                _pager->_readQueue->release();
                break;
            case(HANDLE_NON_HTTP):
                _pager->_readQueue->release();
                break;
            case(HANDLE_ONLY_HTTP):
                _pager->_readQueue->release();
                break;
        }

        // release the frameBlock and _databasePagerThreadBlock in case its holding up thread cancellation.
        // _databasePagerThreadBlock->release();

        // then wait for the the thread to stop running.
        while(isRunning())
        {
            // commenting out debug info as it was cashing crash on exit, presumable
            // due to osg::notify or std::cout destructing earlier than this destructor.
            // osg::notify(osg::DEBUG_INFO)<<"Waiting for DatabasePager to cancel"<<std::endl;
            OpenThreads::Thread::YieldCurrentThread();
        }
        
        // _startThreadCalled = false;
    }
    //std::cout<<"DatabasePager::~DatabasePager() stopped running"<<std::endl;
    return result;
}

void ImagePager::ImageThread::run()
{
    osg::notify(osg::NOTICE)<<"ImagePager::ImageThread::run()"<<std::endl;
    bool firstTime = true;

    osg::ref_ptr<ImagePager::ReadQueue> read_queue;
    
    switch(_mode)
    {
        case(HANDLE_ALL_REQUESTS):
            read_queue = _pager->_readQueue;
            break;
        case(HANDLE_NON_HTTP):
            read_queue = _pager->_readQueue;
            break;
        case(HANDLE_ONLY_HTTP):
            read_queue = _pager->_readQueue;
            break;
    }

    do
    {
        read_queue->block();

        osg::ref_ptr<ImageRequest> imageRequest;
        read_queue->takeFirst(imageRequest);
                
        if (imageRequest.valid())
        {
            osg::ref_ptr<osg::Image> image = osgDB::readImageFile(imageRequest->_fileName);
            if (image.valid())
            {
                osg::ImageSequence* is = dynamic_cast<osg::ImageSequence*>(imageRequest->_objectToAttachTo.get());
                if (is)
                {
                    is->addImage(image.get());
                }
                else
                {
                    osg::Texture* texture = dynamic_cast<osg::Texture*>(imageRequest->_objectToAttachTo.get());
                    if (texture)
                    {
                        texture->setImage(0, image.get());
                    }
                }
            }

        }

        else
        {
            OpenThreads::Thread::YieldCurrentThread();
        }
        
        
        // go to sleep till our the next time our thread gets scheduled.

        if (firstTime)
        {
            // do a yield to get round a peculiar thread hang when testCancel() is called 
            // in certain circumstances - of which there is no particular pattern.
            YieldCurrentThread();
            firstTime = false;
        }

    } while (!testCancel() && !_done);

    osg::notify(osg::NOTICE)<<"ImagePager::ImageThread::done()"<<std::endl;

}

//////////////////////////////////////////////////////////////////////////////////////
//
// ImagePager
//
ImagePager::ImagePager():
    _done(false)
{
    _startThreadCalled = false;
    _databasePagerThreadPaused = false;
    
    _readQueue = new ReadQueue(this,"Image Queue");
    _imageThread = new ImageThread(this, ImageThread::HANDLE_ALL_REQUESTS, "Image Thread");
}

ImagePager::~ImagePager()
{
    cancel();
}

int ImagePager::cancel()
{
    int result = 0;

    _imageThread->setDone(true);

    // release the frameBlock and _databasePagerThreadBlock in case its holding up thread cancellation.
    _readQueue->release();

    _imageThread->cancel();

    _done = true;
    _startThreadCalled = false;

    //std::cout<<"DatabasePager::~DatabasePager() stopped running"<<std::endl;
    return result;
}

void ImagePager::requestImageFile(const std::string& fileName,osg::Object* attachmentPoint, double timeToMergeBy, const osg::FrameStamp* framestamp)
{
    // osg::notify(osg::NOTICE)<<"ImagePager::requestNodeFile("<<fileName<<")"<<std::endl;
    

    osg::ref_ptr<ImageRequest> request = new ImageRequest;
    request->_timeToMergeBy = timeToMergeBy;
    request->_fileName = fileName;
    request->_objectToAttachTo = attachmentPoint;
    request->_requestQueue = _readQueue.get();
   
    _readQueue->add(request.get());

    if (!_startThreadCalled)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_run_mutex);
        
        if (!_startThreadCalled)
        {
            _startThreadCalled = true;
            _done = false;
            _imageThread->startThread();
            
        }
    }
}

bool ImagePager::requiresUpdateSceneGraph() const
{
    //osg::notify(osg::NOTICE)<<"ImagePager::requiresUpdateSceneGraph()"<<std::endl;
    return false;
}

void ImagePager::updateSceneGraph(double currentFrameTime)
{
    //osg::notify(osg::NOTICE)<<"ImagePager::updateSceneGraph(double currentFrameTime)"<<std::endl;
}

