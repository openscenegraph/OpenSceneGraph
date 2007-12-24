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

//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
//

#include "Vertex.h"

using namespace flt;

Vertex::Vertex():
    _coord(0,0,0),
    _color(1,1,1,1),
    _normal(0,0,1),
    _validColor(false),
    _validNormal(false)
{
    for (int layer=0; layer<MAX_LAYERS; layer++)
        _validUV[layer] = false;
}

Vertex::Vertex(const Vertex& vertex):
    _coord(vertex._coord),
    _color(vertex._color),
    _normal(vertex._normal),
    _validColor(vertex._validColor),
    _validNormal(vertex._validNormal)
{
    for (int layer=0; layer<MAX_LAYERS; layer++)
    {
        _uv[layer] = vertex._uv[layer];
        _validUV[layer] = vertex._validUV[layer];
    }
}

void Vertex::setCoord(const osg::Vec3& coord)
{
    _coord = coord;
}

void Vertex::setColor(const osg::Vec4& color)
{
    _color = color;
    _validColor = true;
}

void Vertex::setNormal(const osg::Vec3& normal)
{
    _normal = normal;
    _validNormal = true;
}

void Vertex::setUV(int layer, const osg::Vec2& uv)
{
    if (layer>=0 && layer<MAX_LAYERS)
    {
        _uv[layer] = uv;
        _validUV[layer] = true;
    }
}


