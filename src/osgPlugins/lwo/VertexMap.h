/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#ifndef LWOSG_VERTEXMAP_
#define LWOSG_VERTEXMAP_

#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Array>
#include <osg/Referenced>

#include <string>
#include <map>
#include <vector>

namespace lwosg
{

    /////////////////////////////////////////////////////////////////////////
    // VERTEX MAP

    typedef std::map<int, osg::Vec4> VertexMap_impl;

    class VertexMap: public VertexMap_impl, public osg::Referenced {
    public:
        VertexMap(): VertexMap_impl(), osg::Referenced() {}

        osg::Vec2Array *asVec2Array(int num_vertices, const osg::Vec2 &default_value = osg::Vec2(0, 0), const osg::Vec2 &modulator = osg::Vec2(1, 1)) const;
        osg::Vec3Array *asVec3Array(int num_vertices, const osg::Vec3 &default_value = osg::Vec3(0, 0, 0), const osg::Vec3 &modulator = osg::Vec3(1, 1, 1)) const;
        osg::Vec4Array *asVec4Array(int num_vertices, const osg::Vec4 &default_value = osg::Vec4(0, 0, 0, 0), const osg::Vec4 &modulator = osg::Vec4(1, 1, 1, 1)) const;

        VertexMap *remap(const std::vector<int> &remapping) const;

    protected:
        virtual ~VertexMap() {}
        VertexMap &operator=(const VertexMap &) { return *this; }
    };


    /////////////////////////////////////////////////////////////////////////
    // VERTEX MAP MAP

    typedef std::map<std::string, osg::ref_ptr<VertexMap> > VertexMap_map_impl;
    typedef std::multimap<std::string, int> VertexMap_binding_map;

    class VertexMap_map: public VertexMap_map_impl, public osg::Referenced {
    public:
        VertexMap_map(): VertexMap_map_impl(), osg::Referenced() {}

        VertexMap *getOrCreate(const std::string &name)
        {
            osg::ref_ptr<VertexMap> &vmap = operator[](name);
            if (!vmap.valid()) {
                vmap = new VertexMap;
            }
            return vmap.get();
        }

        VertexMap_map *remap(const std::vector<int> &remapping) const;

    protected:
        virtual ~VertexMap_map() {}
        VertexMap_map &operator=(const VertexMap_map &) { return *this; }

    private:
    };

}

#endif
