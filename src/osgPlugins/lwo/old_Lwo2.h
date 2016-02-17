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

#ifndef LWO2_H
#define LWO2_H 1

#include <vector>
#include <map>
#include <string>

#include <osg/Referenced>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/Notify>

#include <osgDB/fstream>

using namespace osg;
using namespace std;

class Lwo2Layer;
struct Lwo2Surface;
struct Lwo2PolygonMapping;

typedef vector< string >::iterator IteratorString;
typedef map< int, Lwo2Layer* >::iterator IteratorLayers;
typedef map< string, Lwo2Surface* >::iterator IteratorSurfaces;
typedef pair< const short, Lwo2PolygonMapping > PairVMAD;

class Lwo2
{
 public:
  Lwo2();
  ~Lwo2();
  bool ReadFile( const string& filename );
  bool GenerateGroup( Group& );

 private:
  map< int, Lwo2Layer* > _layers;
  map< string, Lwo2Surface* > _surfaces;
  Lwo2Layer* _current_layer;
  vector< string > _tags;
  vector< string > _images;
  osgDB::ifstream _fin;

  unsigned char _read_char();
  unsigned short _read_short();
  unsigned int _read_uint();
  float _read_float();
  string& _read_string(string&);

  bool _successfully_read;

  void _print_tag(unsigned int, unsigned int);
  void _print_type(unsigned int);

  void _read_tag_strings(unsigned long);
  void _read_layer(unsigned long);
  void _read_points(unsigned long);
  void _read_vertex_mapping(unsigned long);
  void _read_polygons(unsigned long);
  void _read_polygons_mapping(unsigned long);
  void _read_polygon_tag_mapping(unsigned long);
  void _read_image_definition(unsigned long);
  void _read_surface(unsigned long);

  // generate StateSets for each surface
  void _generate_statesets_from_surfaces();
};

#endif
