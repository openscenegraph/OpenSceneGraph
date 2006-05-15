//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#ifndef FLT_VERTEX_H
#define FLT_VERTEX_H 1

#include <vector>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Referenced>

namespace flt {

class Vertex
{
public:

    Vertex();
    Vertex(const Vertex& vertex);

    void setCoord(const osg::Vec3& coord);
    void setColor(const osg::Vec4& color);
    void setNormal(const osg::Vec3& normal);
    void setUV(int layer, const osg::Vec2& uv);

    bool validColor() const { return _validColor; }
    bool validNormal() const { return _validNormal; }
    bool validUV(int layer) const { return layer>=0 && layer<MAX_LAYERS && _validUV[layer]; }

    static const int MAX_LAYERS = 8;

    osg::Vec3 _coord;
    osg::Vec4 _color;
    osg::Vec3 _normal;
    osg::Vec2 _uv[MAX_LAYERS];

    bool _validColor;
    bool _validNormal;
    bool _validUV[MAX_LAYERS];
};


class VertexList : public osg::Referenced , public std::vector<Vertex>
{
public:

    VertexList() {}

    explicit VertexList(int size) :
        std::vector<Vertex>(size) {}

protected:

    virtual ~VertexList() {}
};

} // end namespace

#endif
