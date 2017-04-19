/* -*-c++-*- OpenSceneGraph - Copyright (C) 20017 Robert Osfield
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

#include <osg/Polytope>
#include <osg/Notify>

using namespace osg;

bool Polytope::contains(const osg::Vec3f& v0, const osg::Vec3f& v1, const osg::Vec3f& v2) const
{
    if (contains(v0)) return true;
    if (contains(v1)) return true;
    if (contains(v2)) return true;

    return false;
}
