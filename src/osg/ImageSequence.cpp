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

using namespace osg;

ImageSequence::ImageSequence()
{
    _referenceTime = 0.0;
    _timeMultiplier = 1.0;
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
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _fileNameDurationSequence.push_back(FileNameDurationPair(fileName, duration));
}

void ImageSequence::addImage(osg::Image* image, double duration)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _imageDuationSequence.push_back(ImageDurationPair(image, duration));
}

void ImageSequence::update(osg::FrameStamp* fs)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
}
