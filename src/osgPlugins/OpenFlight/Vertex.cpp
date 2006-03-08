//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#include "Vertex.h"

using namespace flt;

Vertex::Vertex():
    _coord(0,0,0),
    _color(1,1,1,1),
    _normal(0,0,1),
    _validColor(false),
    _validNormal(false),
    _validUV(false)
{
}

Vertex::Vertex(const Vertex& vertex):
    _coord(vertex._coord),
    _color(vertex._color),
    _normal(vertex._normal),
    _uv(vertex._uv),
    _validColor(vertex._validColor),
    _validNormal(vertex._validNormal),
    _validUV(vertex._validUV)
{
}

void Vertex::setCoord(osg::Vec3 coord)
{
    _coord = coord;
}

void Vertex::setColor(osg::Vec4 color)
{
    _color = color;
    _validColor = true;
}

void Vertex::setNormal(osg::Vec3 normal)
{
    _normal = normal;
    _validNormal = true;
}

void Vertex::setUV(osg::Vec2 uv)
{
    _uv = uv;
    _validUV = true;
}


