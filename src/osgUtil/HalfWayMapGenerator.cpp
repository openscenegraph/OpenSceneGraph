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
#include <osgUtil/HalfWayMapGenerator>

using namespace osgUtil;

HalfWayMapGenerator::HalfWayMapGenerator(const osg::Vec3 &light_direction, int texture_size)
:    CubeMapGenerator(texture_size),
    ldir_(light_direction)
{
    ldir_.normalize();
}

HalfWayMapGenerator::HalfWayMapGenerator(const HalfWayMapGenerator &copy, const osg::CopyOp &copyop)
:    CubeMapGenerator(copy, copyop),
    ldir_(copy.ldir_)
{
}
