/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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
#include <osg/ClipPlane>
#include <osg/Notify>

using namespace osg;

ClipPlane::ClipPlane()
{
    _clipPlane[0] = 0.0;
    _clipPlane[1] = 0.0;
    _clipPlane[2] = 0.0;
    _clipPlane[3] = 0.0;
    _clipPlaneNum = 0;
}


ClipPlane::~ClipPlane()
{
}

void ClipPlane::setClipPlane(const Vec4& plane)
{
    _clipPlane[0] = plane[0];
    _clipPlane[1] = plane[1];
    _clipPlane[2] = plane[2];
    _clipPlane[3] = plane[3];
}

void ClipPlane::setClipPlane(const Plane& plane)
{
    _clipPlane[0] = plane[0];
    _clipPlane[1] = plane[1];
    _clipPlane[2] = plane[2];
    _clipPlane[3] = plane[3];
}

void ClipPlane::setClipPlane(const double* plane)
{
    if (plane)
    {
        _clipPlane[0] = plane[0];
        _clipPlane[1] = plane[1];
        _clipPlane[2] = plane[2];
        _clipPlane[3] = plane[3];
    }
    else
    {
        notify(WARN)<<"Warning: ClipPlane::setClipPlane() passed NULL plane array, ignoring operation."<<std::endl;
    }
}

void ClipPlane::getClipPlane(Vec4& plane) const
{
    plane[0] = (float)_clipPlane[0];
    plane[1] = (float)_clipPlane[1];
    plane[2] = (float)_clipPlane[2];
    plane[3] = (float)_clipPlane[3];
}

void ClipPlane::getClipPlane(Plane& plane) const
{
    plane[0] = _clipPlane[0];
    plane[1] = _clipPlane[1];
    plane[2] = _clipPlane[2];
    plane[3] = _clipPlane[3];
}

void ClipPlane::getClipPlane(double* plane) const
{
    plane[0] = _clipPlane[0];
    plane[1] = _clipPlane[1];
    plane[2] = _clipPlane[2];
    plane[3] = _clipPlane[3];
}

void ClipPlane::setClipPlaneNum(unsigned int num)
{
    _clipPlaneNum = num;
}

unsigned int ClipPlane::getClipPlaneNum() const
{
    return _clipPlaneNum;
}

void ClipPlane::apply(State&) const
{
    glClipPlane((GLenum)(GL_CLIP_PLANE0+_clipPlaneNum),_clipPlane);
}

