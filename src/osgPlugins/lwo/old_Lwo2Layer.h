/*
 * Lightwave Object version 2 loader for Open Scene Graph
 * Version 2 introduced in Lightwave v6.0
 *
 * Copyright (C) 2002 Pavel Moloshtan <pasha@moloshtan.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * The Open Scene Graph (OSG) is a cross platform C++/OpenGL library for 
 * real-time rendering of large 3D photo-realistic models. 
 * The OSG homepage is http://www.openscenegraph.org/
 */

#ifndef LWO2LAYER_H
#define LWO2LAYER_H

#include <osg/Referenced>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/StateSet>

#include <osgUtil/SmoothingVisitor>

#include <vector>
#include <map>
#include <string>

using namespace osg;
using namespace std;

struct PointData
{
  PointData():
    point_index(0),
    coord(Vec3(0.0f, 0.0f, 0.0f)),
       texcoord(Vec2(-1.0f, -1.0f)) {}

  short point_index;
  Vec3 coord;
  Vec2 texcoord;

  inline bool operator == (const PointData& p) const 
  { 
    return coord == p.coord && texcoord == p.texcoord; 
  }

};

struct Lwo2Surface
{
    Lwo2Surface():
        image_index(-1),
        state_set(0) {}

    short image_index;
    string name;
    Vec3 color;
    StateSet* state_set;
};

typedef vector< PointData > PointsList;
typedef vector< PointsList > PolygonsList;
typedef PolygonsList::iterator IteratorPolygonsList;

typedef map< int, int > DrawableToTagMapping;
typedef pair< int, int > PairDrawableToTag;

typedef vector< PointData >::iterator IteratorPoint;
typedef vector< Vec2 >::iterator IteratorVec2;
typedef vector< short >::iterator IteratorShort;

class Lwo2Layer
{
 public:
  friend class Lwo2;
  Lwo2Layer();
  ~Lwo2Layer();
  void notify(NotifySeverity);
  void GenerateGeode( Geode&, short, DrawableToTagMapping& );

 private:
  bool _find_triangle_fans(PolygonsList&, PolygonsList&);
  bool _find_triangle_fan(PolygonsList&, PolygonsList&);
  bool _find_triangle_strips(PolygonsList&, PolygonsList&);
  bool _find_triangle_strip(PolygonsList&, PolygonsList&);
  int _find_triangle_begins_with(PolygonsList&, PointData&, PointData&);

  short _number;
  short _flags;
  short _parent;
  Vec3 _pivot;
  string _name;
  vector< PointData > _points;
  PolygonsList _polygons;
  vector< short > _polygons_tag;
};
 
#endif
