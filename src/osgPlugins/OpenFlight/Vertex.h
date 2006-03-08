//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#ifndef FLT_VERTEX_H
#define FLT_VERTEX_H 1

#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>

namespace flt {

class Vertex
{
public:

    Vertex();
    Vertex(const Vertex& vertex);

    void setCoord(osg::Vec3 coord);
    void setColor(osg::Vec4 color);
    void setNormal(osg::Vec3 normal);
    void setUV(osg::Vec2 uv);

    bool validColor() const { return _validColor; }
    bool validNormal() const { return _validNormal; }
    bool validUV() const { return _validUV; }

    osg::Vec3 _coord;
    osg::Vec4 _color;
    osg::Vec3 _normal;
    osg::Vec2 _uv;

    bool _validColor;
    bool _validNormal;
    bool _validUV;
};

} // end namespace

#endif
