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
 
#include <osg/GLExtensions>
#include <osg/GL>
#include <osg/PointSprite>

using namespace osg;

int PointSprite::compare(const StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Paramter macro's below.
    COMPARE_StateAttribute_Types(PointSprite,sa)
    return 0; // passed all the above comparison macro's, must be equal.
}

void PointSprite::apply(osg::State&) const
{
    glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, 1);
}
