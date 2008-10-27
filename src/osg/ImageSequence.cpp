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
    
    _seekTime = 0.0;
    _seekTimeSet = false;
    
    _previousAppliedImageIndex = -1;
}

ImageSequence::ImageSequence(const ImageSequence& is,const CopyOp& copyop):
    osg::ImageStream(is,copyop),
    _referenceTime(is._referenceTime),
    _timeMultiplier(is._timeMultiplier),
    _mode(is._mode),
    _length(is._length),
    _timePerImage(is._timePerImage)
{
    _seekTime = is._seekTime;
    _seekTimeSet = is._seekTimeSet;

    _previousAppliedImageIndex = -1;
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
    if (length<=0.0)
    {
        osg::notify(osg::NOTICE)<<"ImageSequence::setLength("<<length<<") invalid length value, must be greater than 0."<<std::endl;
        return;
    }
    
    _length = length;
    computeTimePerImage();
}

void ImageSequence::computeTimePerImage()
{
    if (!_fileNames.empty()) _timePerImage = _length / double(_fileNames.size());
    else if (!_images.empty()) _timePerImage = _length / double(_images.size());
    else _timePerImage = _length;
}

void ImageSequence::setImageFile(unsigned int pos, const std::string& fileName)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    if (pos>=_fileNames.size()) _fileNames.resize(pos);
    _fileNames[pos] = fileName;
}

std::string ImageSequence::getImageFile(unsigned int pos) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    return pos<_fileNames.size() ? _fileNames[pos] : std::string();
}
        
void ImageSequence::addImageFile(const std::string& fileName)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _fileNames.push_back(fileName);
    computeTimePerImage();
}

void ImageSequence::setImage(unsigned int pos, osg::Image* image)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    osg::notify(osg::INFO)<<"ImageSequence::setImage("<<pos<<","<<image->getFileName()<<")"<<std::endl;

    if (pos>=_images.size()) _images.resize(pos+1);
    
    _images[pos] = image;

    // prune from file requested list.
    FilesRequested::iterator itr = _filesRequested.find(image->getFileName());
    if (itr!=_filesRequested.end()) _filesRequested.erase(itr);
}

Image* ImageSequence::getImage(unsigned int pos)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    return pos<_images.size() ? _images[pos].get() : 0;
}

const Image* ImageSequence::getImage(unsigned int pos) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    return pos<_images.size() ? _images[pos].get() : 0;
}

void ImageSequence::addImage(osg::Image* image)
{
    if (image==0) return;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    // osg::notify(osg::NOTICE)<<"merging image in order expected : "<<image->getFileName()<<std::endl;
    _images.push_back(image);

    computeTimePerImage();
    
    if (data()==0)
    {
        setImageToChild(_images.front().get());
    }
}

void ImageSequence::setImageToChild(const osg::Image* image)
{
    // osg::notify(osg::NOTICE)<<"setImageToChild("<<image<<")"<<std::endl;

    if (image==0) return;

    // check to see if data is changing, if not don't apply
    if (image->data() == data()) return;

    setImage(image->s(),image->t(),image->r(),
             image->getInternalTextureFormat(),
             image->getPixelFormat(),image->getDataType(),
             const_cast<unsigned char*>(image->data()),
             osg::Image::NO_DELETE,
             image->getPacking());
}

void ImageSequence::applyLoopingMode()
{
}

int ImageSequence::imageIndex(double time)
{
    if (getLoopingMode()==LOOPING) 
    {
        double positionRatio = time/_length;
        time = (positionRatio - floor(positionRatio))*_length;
    }

    if (time<0.0) return 0;
    int index = int(time/_timePerImage);
    if (index>=int(_images.size())) return int(_images.size())-1;
    return index;
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
    else
    {
        if (looping)
        {
            while (time>_length)
            {
                _referenceTime += _length/_timeMultiplier;
                time -= _length;
            }
        }
        else
        {
            if (time>_length)
            {
                _referenceTime = fs->getSimulationTime() - _length/_timeMultiplier;
                time = _length;
            }
        }
    }
    
    _seekTime = time;
    _seekTimeSet = false;

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

    int index = int(time/_timePerImage);
    // osg::notify(osg::NOTICE)<<"time= "<<time<<" _timePerImage="<<_timePerImage<<" index="<<index<<" _length="<<_length<<std::endl;

    if (index>=int(_images.size())) index = int(_images.size())-1;

    if (index>=0 && index<int(_images.size()))
    {

        if (pruneOldImages)
        {
            while (index>=0 && !_images[index].valid())
            {
                --index;
            }
        }
                
        if (index>=0)
        {
            // osg::notify(osg::NOTICE)<<"at time "<<time<<" setting child = "<<index<<std::endl;

            if (_previousAppliedImageIndex!=index)
            {
                if (_previousAppliedImageIndex >= 0 && 
                    _previousAppliedImageIndex<int(_images.size()) && 
                    pruneOldImages)
                {
                    _images[_previousAppliedImageIndex] = 0;
                }
            
                setImageToChild(_images[index].get());
                
                _previousAppliedImageIndex = index;
            }
        }
    }

    // osg::notify(osg::NOTICE)<<"time = "<<time<<std::endl;

    if (irh)
    {
        double preLoadTime = time + osg::minimum(irh->getPreLoadTime()*_timeMultiplier, _length);
        
        int startLoadIndex = int(time/_timePerImage);
        if (startLoadIndex>=int(_images.size())) startLoadIndex = int(_images.size())-1;
        if (startLoadIndex<0) startLoadIndex = 0;

        int endLoadIndex = int(preLoadTime/_timePerImage);
        if (endLoadIndex>=int(_fileNames.size())) 
        {
            if (looping)
            {
                endLoadIndex -= int(_fileNames.size());
            }
            else
            {
                endLoadIndex = int(_fileNames.size())-1;
            }
        }
        if (endLoadIndex<0) endLoadIndex = 0;
        
        double requestTime = time;
        
        if (endLoadIndex<startLoadIndex)
        {
            for(int i=startLoadIndex; i<int(_fileNames.size()); ++i)
            {
                if ((i>=int(_images.size()) || !_images[i]) && _filesRequested.count(_fileNames[i])==0)
                {
                    _filesRequested.insert(_fileNames[i]);
                    irh->requestImageFile(_fileNames[i], this, i, requestTime, fs);
                }
                requestTime += _timePerImage;
            }

            for(int i=0; i<=endLoadIndex; ++i)
            {
                if ((i>=int(_images.size()) || !_images[i]) && _filesRequested.count(_fileNames[i])==0)
                {
                    _filesRequested.insert(_fileNames[i]);
                    irh->requestImageFile(_fileNames[i], this, i, requestTime, fs);
                }
                requestTime += _timePerImage;
            }
        }
        else
        {
            for(int i=startLoadIndex; i<=endLoadIndex; ++i)
            {
                if ((i>=int(_images.size()) || !_images[i]) && _filesRequested.count(_fileNames[i])==0)
                {
                    _filesRequested.insert(_fileNames[i]);
                    irh->requestImageFile(_fileNames[i], this, i, requestTime, fs);
                }
                requestTime += _timePerImage;
            }
        }


    }

}
