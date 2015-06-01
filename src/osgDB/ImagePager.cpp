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
//  RequestQueue
//
void ImagePager::RequestQueue::sort()
{
    std::sort(_requestList.begin(),_requestList.end(),SortFileRequestFunctor());
}

unsigned int ImagePager::RequestQueue::size() const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);
    return _requestList.size();
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
        (*citr)->_attachmentPoint = 0;
        (*citr)->_requestQueue = 0;
    }

    _requestList.clear();

    updateBlock();
}

void ImagePager::ReadQueue::add(ImagePager::ImageRequest* imageRequest)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);

    _requestList.push_back(imageRequest);
    imageRequest->_requestQueue = this;

    OSG_INFO<<"ImagePager::ReadQueue::add("<<imageRequest->_fileName<<"), size()="<<_requestList.size()<<std::endl;

    updateBlock();
}

void ImagePager::ReadQueue::takeFirst(osg::ref_ptr<ImageRequest>& databaseRequest)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_requestMutex);

    if (!_requestList.empty())
    {
        sort();

        OSG_INFO<<"ImagePager::ReadQueue::takeFirst(..), size()="<<_requestList.size()<<std::endl;

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

        // then wait for the thread to stop running.
        join();

        // _startThreadCalled = false;
    }
    //std::cout<<"ImagePager::cancel() thread stopped running"<<std::endl;
    return result;
}

void ImagePager::signalBeginFrame(const osg::FrameStamp* framestamp)
{
    if (framestamp)
    {
        //OSG_INFO << "signalBeginFrame "<<framestamp->getFrameNumber()<<">>>>>>>>>>>>>>>>"<<std::endl;
        _frameNumber.exchange(framestamp->getFrameNumber());

    } //else OSG_INFO << "signalBeginFrame >>>>>>>>>>>>>>>>"<<std::endl;
}

void ImagePager::signalEndFrame()
{
}


void ImagePager::ImageThread::run()
{
    OSG_INFO<<"ImagePager::ImageThread::run() "<<this<<std::endl;
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
            // OSG_NOTICE<<"doing readImageFile("<<imageRequest->_fileName<<") index to assign = "<<imageRequest->_attachmentIndex<<std::endl;
            osg::ref_ptr<osg::Image> image = osgDB::readImageFile(imageRequest->_fileName, imageRequest->_readOptions.get());
            if (image.valid())
            {
                // OSG_NOTICE<<"   successful readImageFile("<<imageRequest->_fileName<<") index to assign = "<<imageRequest->_attachmentIndex<<std::endl;

                osg::ImageSequence* is = dynamic_cast<osg::ImageSequence*>(imageRequest->_attachmentPoint.get());
                if (is)
                {
                    if (imageRequest->_attachmentIndex >= 0)
                    {
                        is->setImage(imageRequest->_attachmentIndex, image.get());
                    }
                    else
                    {
                        is->addImage(image.get());
                    }
                }
                else
                {
                    imageRequest->_loadedImage = image;

                    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_pager->_completedQueue->_requestMutex);
                    _pager->_completedQueue->_requestList.push_back(imageRequest);
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

    OSG_INFO<<"ImagePager::ImageThread::done()"<<std::endl;

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
    _completedQueue = new RequestQueue;
    _imageThreads.push_back(new ImageThread(this, ImageThread::HANDLE_ALL_REQUESTS, "Image Thread 1"));
    _imageThreads.push_back(new ImageThread(this, ImageThread::HANDLE_ALL_REQUESTS, "Image Thread 2"));
    _imageThreads.push_back(new ImageThread(this, ImageThread::HANDLE_ALL_REQUESTS, "Image Thread 3"));
#if 0
    _imageThreads.push_back(new ImageThread(this, ImageThread::HANDLE_ALL_REQUESTS, "Image Thread 4"));
    _imageThreads.push_back(new ImageThread(this, ImageThread::HANDLE_ALL_REQUESTS, "Image Thread 5"));
    _imageThreads.push_back(new ImageThread(this, ImageThread::HANDLE_ALL_REQUESTS, "Image Thread 6"));
#endif
    // 1 second
    _preLoadTime = 1.0;
}

ImagePager::~ImagePager()
{
    cancel();
}

int ImagePager::cancel()
{
    int result = 0;

    for(ImageThreads::iterator itr = _imageThreads.begin();
        itr != _imageThreads.end();
        ++itr)
    {
        (*itr)->setDone(true);
    }

    // release the frameBlock and _databasePagerThreadBlock in case its holding up thread cancellation.
    _readQueue->release();

    for(ImageThreads::iterator itr = _imageThreads.begin();
        itr != _imageThreads.end();
        ++itr)
    {
        (*itr)->cancel();
    }

    _done = true;
    _startThreadCalled = false;

    //std::cout<<"DatabasePager::~DatabasePager() stopped running"<<std::endl;
    return result;
}

osg::Image* ImagePager::readImageFile(const std::string& fileName, const osg::Referenced* options)
{
    osgDB::Options* readOptions = dynamic_cast<osgDB::Options*>(const_cast<osg::Referenced*>(options));
    return osgDB::readImageFile(fileName, readOptions);
}

void ImagePager::requestImageFile(const std::string& fileName, osg::Object* attachmentPoint, int attachmentIndex, double timeToMergeBy, const osg::FrameStamp* /*framestamp*/, osg::ref_ptr<osg::Referenced>& imageRequest, const osg::Referenced* options)
{

    osgDB::Options* readOptions = dynamic_cast<osgDB::Options*>(const_cast<osg::Referenced*>(options));
    if (!readOptions)
    {
       readOptions = Registry::instance()->getOptions();
    }

    bool alreadyAssigned = dynamic_cast<ImageRequest*>(imageRequest.get()) && (imageRequest->referenceCount()>1);
    if (alreadyAssigned)
    {
        // OSG_NOTICE<<"ImagePager::requestImageFile("<<fileName<<") alreadyAssigned"<<std::endl;
        return;
    }

    osg::ref_ptr<ImageRequest> request = new ImageRequest;
    request->_timeToMergeBy = timeToMergeBy;
    request->_fileName = fileName;
    request->_attachmentPoint = attachmentPoint;
    request->_attachmentIndex = attachmentIndex;
    request->_requestQueue = _readQueue.get();
    request->_readOptions = readOptions;

    imageRequest = request;

    // OSG_NOTICE<<"ImagePager::requestImageFile("<<fileName<<") new request."<<std::endl;

    _readQueue->add(request.get());

    if (!_startThreadCalled)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_run_mutex);

        if (!_startThreadCalled)
        {
            _startThreadCalled = true;
            _done = false;

            for(ImageThreads::iterator itr = _imageThreads.begin();
                itr != _imageThreads.end();
                ++itr)
            {
                (*itr)->startThread();
            }

        }
    }
}

bool ImagePager::requiresUpdateSceneGraph() const
{
    //OSG_NOTICE<<"ImagePager::requiresUpdateSceneGraph()"<<std::endl;
    return !(_completedQueue->_requestList.empty());
}

void ImagePager::updateSceneGraph(const osg::FrameStamp&)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_completedQueue->_requestMutex);

    for(RequestQueue::RequestList::iterator itr = _completedQueue->_requestList.begin();
        itr != _completedQueue->_requestList.end();
        ++itr)
    {
        ImageRequest* imageRequest = itr->get();
        osg::Texture* texture = dynamic_cast<osg::Texture*>(imageRequest->_attachmentPoint.get());
        if (texture)
        {
            int attachmentIndex = imageRequest->_attachmentIndex > 0 ? imageRequest->_attachmentIndex : 0;
            texture->setImage(attachmentIndex, imageRequest->_loadedImage.get());
        }
        else
        {
            OSG_NOTICE<<"ImagePager::updateSceneGraph() : error, image request attachment type not handled yet."<<std::endl;
        }
    }

    _completedQueue->_requestList.clear();
}

