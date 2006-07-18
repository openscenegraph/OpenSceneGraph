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
#include <osgUtil/HighlightMapGenerator>

using namespace osgUtil;

HighlightMapGenerator::HighlightMapGenerator(const osg::Vec3 &light_direction,
                                             const osg::Vec4 &light_color,
                                             float specular_exponent,
                                             int texture_size)
:    CubeMapGenerator(texture_size),
    ldir_(light_direction),
    lcol_(light_color),
    sexp_(specular_exponent)
{
    ldir_.normalize();
}

HighlightMapGenerator::HighlightMapGenerator(const HighlightMapGenerator &copy, const osg::CopyOp &copyop)
:    CubeMapGenerator(copy, copyop),
    ldir_(copy.ldir_),
    lcol_(copy.lcol_),
    sexp_(copy.sexp_)
{
}
