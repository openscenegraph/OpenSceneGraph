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

    _duration = 1.0;
    _preLoadTime = 1.0;
    _timePerImage = 1.0;
    _pruneOldImages = false;

    _fileNamesIterator = _fileNames.end();
    _fileNamesIteratorTime = DBL_MAX;

    _imageIterator = _images.end();
    _imageIteratorTime = DBL_MAX;
}

ImageSequence::ImageSequence(const ImageSequence& is,const CopyOp& copyop):
    osg::ImageStream(is,copyop),
    _referenceTime(is._referenceTime),
    _timeMultiplier(is._timeMultiplier),
    _duration(is._duration),
    _timePerImage(is._timePerImage),
    _preLoadTime(is._preLoadTime),
    _pruneOldImages(is._pruneOldImages)
{
    _fileNamesIterator = _fileNames.end();
    _fileNamesIteratorTime = DBL_MAX;

    _imageIteratorTime = DBL_MAX;
    _imageIterator = _images.end();
}

int ImageSequence::compare(const Image& rhs) const
{
    return ImageStream::compare(rhs);
}

void ImageSequence::setDuration(double duration)
{
    _duration = duration;
    computeTimePerImage();
}

void ImageSequence::computeTimePerImage()
{
    if (!_fileNames.empty()) _timePerImage = _duration / double(_fileNames.size());
    else if (!_images.empty()) _timePerImage = _duration / double(_images.size());
    else _timePerImage = _duration;
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
    _images.push_back(image);
    computeTimePerImage();

    if (_imageIterator==_images.end())
    {
        _imageIterator = _images.begin();
        _imageIteratorTime = _referenceTime;
        setImageToChild(_imageIterator->get());
    }
}

void ImageSequence::setImageToChild(const osg::Image* image)
{
    // osg::notify(osg::NOTICE)<<"setImageToChild("<<image<<")"<<std::endl;

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
    
    if (_fileNamesIteratorTime == DBL_MAX)
    {
        _fileNamesIteratorTime = _referenceTime;
    }
    
    if (_imageIteratorTime == DBL_MAX)
    {
        _imageIteratorTime = _referenceTime;
    }
    
    double time = (fs->getSimulationTime() - _referenceTime)*_timeMultiplier;
    double preLoadTime = (time+_preLoadTime)*_timeMultiplier;
            
    Images::iterator previous_imageIterator = _imageIterator;
    
    // osg::notify(osg::NOTICE)<<"time = "<<time<<std::endl;

    {
        //
        // Advance imageIterator
        //
        if (_fileNamesIterator!=_fileNames.end())
        {
            // osg::notify(osg::NOTICE)<<"   _fileNamesIteratorTime = "<<_fileNamesIteratorTime<<std::endl;
            while(preLoadTime > (_fileNamesIteratorTime + _timePerImage))
            {
                _fileNamesIteratorTime += _timePerImage;
                osg::notify(osg::NOTICE)<<"   _fileNamesIteratorTime = "<<_fileNamesIteratorTime<<std::endl;
                osg::notify(osg::NOTICE)<<"   need to preLoad = "<<*_fileNamesIterator<<std::endl;
                ++_fileNamesIterator;

                if (_fileNamesIterator ==_fileNames.end())
                {
                    // return iterator to begining of set.            
                    _fileNamesIterator = _fileNames.begin();
                }
            }
        }

        if (_fileNamesIterator==_fileNames.end())
        {
            _fileNamesIterator = _fileNames.begin();
        }
    }
        
    {
        //
        // Advance imageIterator
        //
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

                    if (_pruneOldImages)
                    {
                        _images.erase(previous_imageIterator, _imageIterator);
                    }

                    // return iterator to begining of set.            
                    _imageIterator = _images.begin();
                }
            }
        }

        if (_imageIterator==_images.end())
        {
            _imageIterator = _images.begin();
        }
    }
        
    if (_imageIterator!=_images.end() && previous_imageIterator != _imageIterator)
    {
        if (_pruneOldImages)
        {
            _images.erase(previous_imageIterator, _imageIterator);
        }
    
        setImageToChild(_imageIterator->get());
    }
}
