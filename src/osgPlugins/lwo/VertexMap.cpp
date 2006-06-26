/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#include "VertexMap.h"

#include <osg/Notify>

using namespace lwosg;

osg::Vec4Array *VertexMap::asVec4Array(int num_vertices, const osg::Vec4 &default_value, const osg::Vec4 &modulator) const
{
    osg::ref_ptr<osg::Vec4Array> array = new osg::Vec4Array;
    array->assign(num_vertices, default_value);
    for (VertexMap::const_iterator i=begin(); i!=end(); ++i) {
        osg::Vec4 value = i->second;
        value.x() *= modulator.x();
        value.y() *= modulator.y();
        value.z() *= modulator.z();
        value.w() *= modulator.w();
        array->at(i->first) = value;
    }
    return array.release();
}

osg::Vec2Array *VertexMap::asVec2Array(int num_vertices, const osg::Vec2 &default_value, const osg::Vec2 &modulator) const
{
    osg::ref_ptr<osg::Vec2Array> array = new osg::Vec2Array;
    array->assign(num_vertices, default_value);
    for (VertexMap::const_iterator i=begin(); i!=end(); ++i) {
        osg::Vec4 value = i->second;
        value.x() *= modulator.x();
        value.y() *= modulator.y();
        array->at(i->first) = osg::Vec2(value.x(), value.y());
    }
    return array.release();
}

osg::Vec3Array *VertexMap::asVec3Array(int num_vertices, const osg::Vec3 &default_value, const osg::Vec3 &modulator) const
{
    osg::ref_ptr<osg::Vec3Array> array = new osg::Vec3Array;
    array->assign(num_vertices, default_value);
    for (VertexMap::const_iterator i=begin(); i!=end(); ++i) {
        osg::Vec4 value = i->second;
        value.x() *= modulator.x();
        value.y() *= modulator.y();
        value.z() *= modulator.z();
        array->at(i->first) = osg::Vec3(value.x(), value.y(), value.z());
    }
    return array.release();
}

VertexMap *VertexMap::remap(const std::vector<int> &remapping) const
{
    osg::ref_ptr<VertexMap> result = new VertexMap;

    for (VertexMap::const_iterator i=begin(); i!=end(); ++i) {
        if (i->first >= static_cast<int>(remapping.size())) {
            osg::notify(osg::WARN) << "Warning: lwosg::remap(): remapping index not found for vertex " << i->first << " (map size " << remapping.size() << ")" << std::endl;
        } else {
            int new_index = remapping[i->first];
            if (new_index != -1) {
                (*result.get())[new_index] = i->second;
            }
        }
    }

    return result.release();
}

VertexMap_map *VertexMap_map::remap(const std::vector<int> &remapping) const
{
    osg::ref_ptr<VertexMap_map> result = new VertexMap_map;
    for (VertexMap_map::const_iterator i=begin(); i!=end(); ++i) {
        (*result.get())[i->first] = i->second->remap(remapping);
    }
    return result.release();
}
