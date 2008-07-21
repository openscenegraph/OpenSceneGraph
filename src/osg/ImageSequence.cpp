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

#include <math.h>

using namespace osg;

ImageSequence::ImageSequence()
{
    _referenceTime = DBL_MAX;
    _imageIteratorTime = DBL_MAX;
    _timeMultiplier = 1.0;
    _imageIterator = _imageDuationSequence.end();
}

ImageSequence::ImageSequence(const ImageSequence& is,const CopyOp& copyop):
    osg::ImageStream(is,copyop),
    _referenceTime(is._referenceTime),
    _timeMultiplier(is._timeMultiplier)
{
}

int ImageSequence::compare(const Image& rhs) const
{
    return ImageStream::compare(rhs);
}

void ImageSequence::addImageFile(const std::string& fileName, double duration)
{
    if (duration<0.01) duration = 0.01;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _fileNameDurationSequence.push_back(FileNameDurationPair(fileName, duration));
}

void ImageSequence::addImage(osg::Image* image, double duration)
{
    if (duration<0.01) duration = 0.01;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _imageDuationSequence.push_back(ImageDurationPair(image, duration));

    if (_imageIterator==_imageDuationSequence.end())
    {
        _imageIterator = _imageDuationSequence.begin();
        _imageIteratorTime = _referenceTime;
        setImageToChild(_imageIterator->first.get());
    }

}

void ImageSequence::setImageToChild(const osg::Image* image)
{
    setImage(image->s(),image->t(),image->r(),
             image->getInternalTextureFormat(),
             image->getPixelFormat(),image->getDataType(),
             const_cast<unsigned char*>(image->data()),
             osg::Image::NO_DELETE,
             image->getPacking());
}

void ImageSequence::update(const osg::FrameStamp* fs)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    // osg::notify(osg::NOTICE)<<"ImageSequence::update("<<fs<<")"<<std::endl;

    if (_referenceTime == DBL_MAX)
    {
        _referenceTime = fs->getSimulationTime();
    }
    
    if (_imageIteratorTime == DBL_MAX)
    {
        _imageIteratorTime = _referenceTime;
    }
    
    double time = (fs->getSimulationTime() - _referenceTime)*_timeMultiplier;
    
    // osg::notify(osg::NOTICE)<<"time = "<<time<<std::endl;

    if (_imageIterator!=_imageDuationSequence.end())
    {
        // osg::notify(osg::NOTICE)<<"   _imageIteratorTime = "<<_imageIteratorTime<<std::endl;
        while(time > (_imageIteratorTime + _imageIterator->second))
        {
            _imageIteratorTime += _imageIterator->second;
            // osg::notify(osg::NOTICE)<<"   _imageIteratorTime = "<<_imageIteratorTime<<std::endl;
            ++_imageIterator;
            
            if (_imageIterator ==_imageDuationSequence.end())
            {
                // return iterator to begining of set.            
                _imageIterator = _imageDuationSequence.begin();
            }
        }
    }
    
    if (_imageIterator==_imageDuationSequence.end())
    {
        _imageIterator = _imageDuationSequence.begin();
    }
    
    if (_imageIterator!=_imageDuationSequence.end())
    {
        setImageToChild(_imageIterator->first.get());
    }
}
