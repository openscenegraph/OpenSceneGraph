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

#ifndef OSGTEXT_GLYPHGEOMETRY
#define OSGTEXT_GLYPHGEOMETRY 1

#include <osgText/Font3D>
#include "TextNode.h"

namespace osgText
{

extern osg::Geometry* computeGlyphGeometry(osgText::Glyph3D* glyph, float bevelThickness, float shellThickness);

extern osg::Geometry* computeTextGeometry(osg::Geometry* glyphGeometry, const Bevel& profile, float width);

extern osg::Geometry* computeShellGeometry(osg::Geometry* glyphGeometry, const Bevel& profile, float width);

}

#endif