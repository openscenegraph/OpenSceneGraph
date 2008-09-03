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

#include <OpenThreads/ScopedLock>
#include <osg/ImageSequence>
#include <osg/Notify>
#include <osg/Camera>
#include <osg/NodeVisitor>
#include <osg/Texture2D>

#include <math.h>

using namespace osg;

void ImageSequence::UpdateCallback::operator () (osg::StateAttribute* attr, osg::NodeVisitor* nv)
{
    osg::Texture* texture = attr ? attr->asTexture() : 0;
    
    // osg::notify(osg::NOTICE)<<"ImageSequence::UpdateCallback::"<<texture<<std::endl;
    if (texture)
    {
        for(unsigned int i=0; i<texture->getNumImages(); ++i)
        {
            texture->getImage(i)->update(nv);
        }
    }
}

ImageSequence::ImageSequence()
{
    _referenceTime = DBL_MAX;
    _timeMultiplier = 1.0;

    _mode = PRE_LOAD_ALL_IMAGES;
    _length = 1.0;
    _timePerImage = 1.0;

    _fileNamesIterator = _fileNames.end();
    _fileNamesIteratorTime = 0.0;

    _imageIterator = _images.end();
    _imageIteratorTime = 0.0;
    
    _seekTime = 0.0;
    _seekTimeSet = false;
}

ImageSequence::ImageSequence(const ImageSequence& is,const CopyOp& copyop):
    osg::ImageStream(is,copyop),
    _referenceTime(is._referenceTime),
    _timeMultiplier(is._timeMultiplier),
    _mode(is._mode),
    _length(is._length),
    _timePerImage(is._timePerImage)
{
    _fileNamesIterator = _fileNames.end();
    _fileNamesIteratorTime = 0.0;

    _imageIterator = _images.end();
    _imageIteratorTime = 0.0;

    _seekTime = is._seekTime;
    _seekTimeSet = is._seekTimeSet;
}

int ImageSequence::compare(const Image& rhs) const
{
    return ImageStream::compare(rhs);
}

void ImageSequence::seek(double time)
{
    _seekTime = time;
    _seekTimeSet = true;
}

void ImageSequence::play()
{
    _status=PLAYING;
}

void ImageSequence::pause()
{
    _status=PAUSED;
}

void ImageSequence::rewind()
{
    seek(0.0f);
}

void ImageSequence::setMode(Mode mode)
{
    _mode = mode;
}

void ImageSequence::setLength(double length)
{
    _length = length;
    computeTimePerImage();
}

void ImageSequence::computeTimePerImage()
{
    if (!_fileNames.empty()) _timePerImage = _length / double(_fileNames.size());
    else if (!_images.empty()) _timePerImage = _length / double(_images.size());
    else _timePerImage = _length;
}

void ImageSequence::addImageFile(const std::string& fileName)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _fileNames.push_back(fileName);
    computeTimePerImage();
    
    if (_fileNamesIterator==_fileNames.end())
    {
        _fileNamesIterator = _fileNames.begin();
        _fileNamesIteratorTime = _referenceTime;
    }
}

void ImageSequence::addImage(osg::Image* image)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    if (!_filesRequested.empty())
    {
        // follows is a mechanism that ensures that requested images
        // get merged in the correct time order.
        if (_filesRequested.front().first != image->getFileName())
        {
            for(FileNameImageList::iterator itr = _filesRequested.begin();
                itr != _filesRequested.end();
                ++itr)
            {
                if (itr->first == image->getFileName())
                {
                    osg::notify(osg::NOTICE)<<"inserting image into waiting stake : "<<image->getFileName()<<std::endl;
                    itr->second = image;
                    return;
                }
            }
            // osg::notify(osg::NOTICE)<<"image not expected : "<<image->getFileName()<<std::endl;
            _images.push_back(image);
        }
        else
        {
            // osg::notify(osg::NOTICE)<<"merging image in order expected : "<<image->getFileName()<<std::endl;
            _images.push_back(image);

            _filesRequested.pop_front();
            
            FileNameImageList::iterator itr;
            for(itr = _filesRequested.begin();
                itr != _filesRequested.end() && itr->second.valid();
                ++itr)
            {
                // osg::notify(osg::NOTICE)<<"   merging previously loaded, but out of order file : "<<itr->first<<std::endl;
                _images.push_back(itr->second);
            }
            
            _filesRequested.erase(_filesRequested.begin(), itr);
        }
    }
    else
    {
        _images.push_back(image);
    }

    computeTimePerImage();
    
    if (data()==0)
    {
        _imageIterator = _images.begin();
        _imageIteratorTime = 0.0;
        setImageToChild(_imageIterator->get());
    }
}

void ImageSequence::setImageToChild(const osg::Image* image)
{
    // osg::notify(osg::NOTICE)<<"setImageToChild("<<image<<")"<<std::endl;

    if (image==0) return;

    setImage(image->s(),image->t(),image->r(),
             image->getInternalTextureFormat(),
             image->getPixelFormat(),image->getDataType(),
             const_cast<unsigned char*>(image->data()),
             osg::Image::NO_DELETE,
             image->getPacking());
}

void ImageSequence::update(osg::NodeVisitor* nv)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    
    osg::NodeVisitor::ImageRequestHandler* irh = nv->getImageRequestHandler();
    const osg::FrameStamp* fs = nv->getFrameStamp();

    // osg::notify(osg::NOTICE)<<"ImageSequence::update("<<fs<<", "<<irh<<")"<<std::endl;

    if (_referenceTime == DBL_MAX)
    {
        _referenceTime = fs->getSimulationTime();
    }
    
    bool looping = getLoopingMode()==LOOPING;
    double time = (fs->getSimulationTime() - _referenceTime)*_timeMultiplier;
    
    if (_seekTimeSet || _status==PAUSED || _status==INVALID)
    {
        time = _seekTime;        
        _referenceTime =  fs->getSimulationTime() - time/_timeMultiplier;
    }
    
    if (time>_length)
    {
        time -= floor(time/_length)*_length;
    }
            
    _seekTime = time;
    _seekTimeSet = false;

    FileNames::iterator previous_fileNamesIterator = _fileNamesIterator;
    Images::iterator previous_imageIterator = _imageIterator;
    
    bool pruneOldImages = false;
    
    
    switch(_mode)
    {
        case(PRE_LOAD_ALL_IMAGES):
        {
            if (_fileNames.size()>_images.size())
            {
                FileNames::iterator itr = _fileNames.begin();
                for(unsigned int i=0;i<_images.size();++i) ++itr;

                for(; itr!=_fileNames.end(); ++itr)
                {
                    osg::Image* image = irh->readImageFile(*itr);
                    _images.push_back(image);
                }
            }
        
            irh = 0;
            break;
        }
        case(PAGE_AND_RETAIN_IMAGES):
        {
            break;
        }
        case(PAGE_AND_DISCARD_USED_IMAGES):
        {
            pruneOldImages = true;
            break;
        }
    }

    // osg::notify(osg::NOTICE)<<"time = "<<time<<std::endl;

    if (irh && _images.size()<_fileNames.size())
    {
        double preLoadTime = time + osg::minimum(irh->getPreLoadTime()*_timeMultiplier, _length);
        
        if (preLoadTime>=_length)
        {
            //
            // Advance fileNameIterator to end and wrap around
            //
            for(; 
                _fileNamesIterator != _fileNames.end();
                ++_fileNamesIterator)
            {
                _fileNamesIteratorTime += _timePerImage;
                
                double effectTime = fs->getSimulationTime() + (preLoadTime - _fileNamesIteratorTime);
                _filesRequested.push_back(FileNameImagePair(*_fileNamesIterator,0));
                irh->requestImageFile(*_fileNamesIterator, this, effectTime, fs);
                
            }
            
            preLoadTime -= _length;

            if (looping)
            {
                _fileNamesIterator = _fileNames.begin();
                _fileNamesIteratorTime = 0.0;
            }
        }
        

        if (_fileNamesIterator!=_fileNames.end())
        {
            //
            // Advance fileNameIterator to encmpass preLoadTime
            //

            //osg::notify(osg::NOTICE)<<"   _fileNamesIteratorTime = "<<_fileNamesIteratorTime<<" "<<_timePerImage<<std::endl;
            while(preLoadTime > (_fileNamesIteratorTime + _timePerImage))
            {
                _fileNamesIteratorTime += _timePerImage;
                //osg::notify(osg::NOTICE)<<"   _fileNamesIteratorTime = "<<_fileNamesIteratorTime<<std::endl;
                //osg::notify(osg::NOTICE)<<"   need to preLoad = "<<*_fileNamesIterator<<std::endl;
                ++_fileNamesIterator;

                if (previous_fileNamesIterator==_fileNamesIterator) break;

                if (_fileNamesIterator ==_fileNames.end())
                {
                    // return iterator to begining of set.            
                    if (looping) _fileNamesIterator = _fileNames.begin();
                    else break;                    
                }

                double effectTime = fs->getSimulationTime() + (preLoadTime - _fileNamesIteratorTime);
                _filesRequested.push_back(FileNameImagePair(*_fileNamesIterator,0));
                irh->requestImageFile(*_fileNamesIterator, this, effectTime, fs);
            }
        }

        if (looping && _fileNamesIterator==_fileNames.end())
        {
            _fileNamesIterator = _fileNames.begin();
            _fileNamesIteratorTime = 0.0;
        }
    }
        


    {
        //
        // Advance imageIterator
        //

        if (time<_imageIteratorTime)
        {
            _imageIterator = _images.begin();
            _imageIteratorTime = 0.0;
        }

        if (_imageIterator!=_images.end())
        {
            // osg::notify(osg::NOTICE)<<"   _imageIteratorTime = "<<_imageIteratorTime<<std::endl;
            while(time > (_imageIteratorTime + _timePerImage))
            {
                _imageIteratorTime += _timePerImage;
                // osg::notify(osg::NOTICE)<<"   _imageIteratorTime = "<<_imageIteratorTime<<std::endl;
                ++_imageIterator;

                if (_imageIterator ==_images.end())
                {

                    if (pruneOldImages)
                    {
                        _images.erase(previous_imageIterator, _imageIterator);
                        previous_imageIterator = _images.begin();
                    }

                    if (looping)
                    {
                        // return iterator to begining of set.            
                        _imageIterator = _images.begin();
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }

        if (looping && _imageIterator==_images.end())
        {
            _imageIterator = _images.begin();
        }
    }
        
    if (_imageIterator!=_images.end() && previous_imageIterator != _imageIterator)
    {
        if (pruneOldImages)
        {
            _images.erase(previous_imageIterator, _imageIterator);
        }
    
        setImageToChild(_imageIterator->get());
    }
}
