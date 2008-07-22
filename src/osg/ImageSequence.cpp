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
    _pruneOldImages = false;

    _imageIteratorTime = DBL_MAX;
    _imageIterator = _images.end();
}

ImageSequence::ImageSequence(const ImageSequence& is,const CopyOp& copyop):
    osg::ImageStream(is,copyop),
    _referenceTime(is._referenceTime),
    _timeMultiplier(is._timeMultiplier),
    _duration(is._duration),
    _pruneOldImages(is._pruneOldImages)
{
    _imageIteratorTime = DBL_MAX;
    _imageIterator = _images.end();
}

int ImageSequence::compare(const Image& rhs) const
{
    return ImageStream::compare(rhs);
}

void ImageSequence::addImageFile(const std::string& fileName)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _fileNames.push_back(fileName);
}

void ImageSequence::addImage(osg::Image* image)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _images.push_back(image);

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
    
    if (_imageIteratorTime == DBL_MAX)
    {
        _imageIteratorTime = _referenceTime;
    }
    
    double time = (fs->getSimulationTime() - _referenceTime)*_timeMultiplier;
    double delta = _fileNames.empty() ?
            _duration / _images.size() : 
            _duration / _fileNames.size();
            
    Images::iterator previous_imageIterator = _imageIterator;
    
    // osg::notify(osg::NOTICE)<<"time = "<<time<<std::endl;

    if (_imageIterator!=_images.end())
    {
        // osg::notify(osg::NOTICE)<<"   _imageIteratorTime = "<<_imageIteratorTime<<std::endl;
        while(time > (_imageIteratorTime + delta))
        {
            _imageIteratorTime += delta;
            // osg::notify(osg::NOTICE)<<"   _imageIteratorTime = "<<_imageIteratorTime<<std::endl;
            ++_imageIterator;
            
            if (_imageIterator ==_images.end())
            {
                // return iterator to begining of set.            
                _imageIterator = _images.begin();
            }
        }
    }
    
    if (_imageIterator==_images.end())
    {
        _imageIterator = _images.begin();
    }
    
    if (_imageIterator!=_images.end() && previous_imageIterator != _imageIterator)
    {
        setImageToChild(_imageIterator->get());
    }
}
