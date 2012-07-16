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

#include <osg/ImageStream>

using namespace osg;

ImageStream::ImageStream():
    _status(INVALID),
    _loopingMode(LOOPING)
{
    setDataVariance(DYNAMIC);

//#ifndef __APPLE__
    // disabled under OSX for time being while we resolve why PBO
    // doesn't function properly under OSX.
    setPixelBufferObject(new PixelBufferObject(this));
//#endif

}

ImageStream::ImageStream(const ImageStream& image,const CopyOp& copyop):
    Image(image,copyop),
    _status(image._status),
    _loopingMode(image._loopingMode),
    _audioStreams(image._audioStreams)
{
}

int ImageStream::compare(const Image& rhs) const
{
    return Image::compare(rhs);
}

