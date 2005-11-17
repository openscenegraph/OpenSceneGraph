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

#include "old_Lwo2Layer.h"
#include <osg/io_utils>

Lwo2Layer::Lwo2Layer():
  _number(0),
  _flags(0),
  _parent(0)
{
}

Lwo2Layer::~Lwo2Layer()
{
}


void
Lwo2Layer::notify(NotifySeverity severity)
{
  osg::notify(severity) << "Current layer: " << _number << endl;
  osg::notify(severity) << "  flags  \t" << _flags << endl;
  osg::notify(severity) << "  pivot  \t" << _pivot << endl;
  osg::notify(severity) << "  name:  \t'" << _name.c_str() << "'" << endl;
  osg::notify(severity) << "  parent:\t" << _parent << endl;

  // points
  osg::notify(severity) << "  points:\t" << _points.size() << endl;
  osg::notify(severity) << "\tcoord\t\t\t\ttexcoord" <<  endl;
  osg::notify(severity) << "\t=====\t\t\t\t========" <<  endl;
  IteratorPoint itr;
  for (itr = _points.begin(); itr != _points.end(); itr++)
    {
      osg::notify(severity) << "    \t" << (*itr).coord << "\t\t" << (*itr).texcoord << endl;
    }
  
  // polygons
  osg::notify(severity) << "  polygons:\t" << _polygons.size() << endl;
  osg::notify(severity) << "\tcoord\t\t\t\ttexcoord" <<  endl;
  osg::notify(severity) << "\t=====\t\t\t\t========" <<  endl;
  IteratorPolygonsList polygon_iterator;
  int polygon_index = 0;
  for (polygon_iterator = _polygons.begin(); polygon_iterator != _polygons.end(); polygon_iterator++, polygon_index++)
    {
      osg::notify(severity) << "    \t" << polygon_index << " ("<< (*polygon_iterator).size() << " vertexes" << "):" << endl;
      for (itr = (*polygon_iterator).begin(); itr != (*polygon_iterator).end(); itr++)
        {
          osg::notify(severity) << "    \t" << (*itr).coord << "\t\t" << (*itr).texcoord << endl;
        }
      osg::notify(severity) << endl;
    }

  // polygons tags
  osg::notify(severity) << "  polygons tags:\t" << _polygons_tag.size() << endl;
  IteratorShort short_itr;
  for (short_itr = _polygons_tag.begin(); short_itr != _polygons_tag.end(); short_itr++)
    {
      osg::notify(severity) << "\t" << (*short_itr) << endl;
    }
}

void
Lwo2Layer::GenerateGeode( Geode& geode, short tags_count, DrawableToTagMapping& tag_mapping)
{
  notify(DEBUG_INFO);

  // variable used to track using textures
  bool have_texture_coords;
  
  // create diffirent geomerty for each tag
  for (short current_tag = 0; current_tag < tags_count; current_tag++)
    {
      have_texture_coords = false;

      // new geometry
      ref_ptr<Geometry> geometry = new Geometry;

      // create coords array
      ref_ptr<Vec3Array> coords = new Vec3Array;

      // create texture array
      ref_ptr<Vec2Array> texcoords = new Vec2Array;

      // selecting polygons for current layer only
      int polygon_index = 0;
      PolygonsList polygons;
      IteratorPolygonsList polygon_iterator;
      for (polygon_iterator = _polygons.begin(); polygon_iterator != _polygons.end(); polygon_iterator++, polygon_index++)
        {
          // *polygon_iterator it's a PolygonsList

          // polygons of current tag only
          if (_polygons_tag[polygon_index] == current_tag)
            {

              // reset point_index member for later comparing poins data
              PointsList points_list = *polygon_iterator;
              for (unsigned int i = 0; i < points_list.size(); i++)
                {
                  points_list[i].point_index = 0;
                }

              polygons.push_back(*polygon_iterator);
            }
        }

      // find and compose triangle fans
      PolygonsList triangle_fans;
      _find_triangle_fans(polygons, triangle_fans);

      // find and compose triangle strips
      PolygonsList triangle_strips;
      _find_triangle_strips(polygons, triangle_strips);

      // polygons of current layer
      polygon_index = 0;
      for (polygon_iterator = polygons.begin(); polygon_iterator != polygons.end(); polygon_iterator++, polygon_index++)
        {
          if ((*polygon_iterator)[0].point_index != -1)
            {
              // all points of polygon
              for (IteratorPoint itr = (*polygon_iterator).begin(); itr != (*polygon_iterator).end(); itr++)
                {
                  // *itr - it's a PointData

                  // polygons data
                  (*coords).push_back((*itr).coord);
                  (*texcoords).push_back((*itr).texcoord);

                  if ((*itr).texcoord.x() != -1.0f || (*itr).texcoord.y() != -1.0f) 
                    {
                      have_texture_coords = true;
                    }
                }
          
              unsigned int points_start = (*coords).size() - (*polygon_iterator).size();
              unsigned int points_count = (*polygon_iterator).size();
              if (points_count == 3)
                {
                  geometry->addPrimitiveSet(new DrawArrays(PrimitiveSet::TRIANGLES, points_start, points_count));
                }
              else if (points_count == 4)
                {
                  geometry->addPrimitiveSet(new DrawArrays(PrimitiveSet::QUADS, points_start, points_count));
                }
              else 
                {
                  geometry->addPrimitiveSet(new DrawArrays(PrimitiveSet::POLYGON, points_start, points_count));
                }
            }
        }

      // triangle fans of current layer
      polygon_index = 0;
      for (polygon_iterator = triangle_fans.begin(); polygon_iterator != triangle_fans.end(); polygon_iterator++, polygon_index++)
        {

          // all points of polygon
          for (IteratorPoint itr = (*polygon_iterator).begin(); itr != (*polygon_iterator).end(); itr++)
            {
              // *itr - it's a PointData

              // polygons data
              (*coords).push_back((*itr).coord);
              (*texcoords).push_back((*itr).texcoord);

              if ((*itr).texcoord.x() != -1.0f || (*itr).texcoord.y() != -1.0f) 
                {
                  have_texture_coords = true;
                }
            }
          
          unsigned int points_start = (*coords).size() - (*polygon_iterator).size();
          unsigned int points_count = (*polygon_iterator).size();
          geometry->addPrimitiveSet(new DrawArrays(PrimitiveSet::TRIANGLE_FAN, points_start, points_count));
        }

      // triangle strips of current layer
      polygon_index = 0;
      for (polygon_iterator = triangle_strips.begin(); polygon_iterator != triangle_strips.end(); polygon_iterator++, polygon_index++)
        {

          // all points of polygon
          for (IteratorPoint itr = (*polygon_iterator).begin(); itr != (*polygon_iterator).end(); itr++)
            {
              // *itr - it's a PointData

              // polygons data
              (*coords).push_back((*itr).coord);
              (*texcoords).push_back((*itr).texcoord);

              if ((*itr).texcoord.x() != -1.0f || (*itr).texcoord.y() != -1.0f) 
                {
                  have_texture_coords = true;
                }
            }
          
          unsigned int points_start = (*coords).size() - (*polygon_iterator).size();
          unsigned int points_count = (*polygon_iterator).size();
          geometry->addPrimitiveSet(new DrawArrays(PrimitiveSet::TRIANGLE_STRIP, points_start, points_count));
        }

      // add geometry if it contains any points
      if (coords->size() != 0) 
        {
          geometry->setVertexArray(coords.get());

          // assign texture array
          if (have_texture_coords)
            {
              geometry->setTexCoordArray(0, texcoords.get());
            }

          // generate normals
          osgUtil::SmoothingVisitor smoother;
          smoother.smooth(*(geometry.get()));

          geode.addDrawable(geometry.get());

          osg::notify(DEBUG_INFO) << "  inserting tag " << geode.getNumDrawables() - 1 << ":" << current_tag << std::endl;
          tag_mapping.insert(PairDrawableToTag(geode.getNumDrawables() - 1, current_tag));
        }
    }
}

bool 
Lwo2Layer::_find_triangle_fans(PolygonsList& polygons, PolygonsList& triangle_fans)
{
  bool found = false;

  while (_find_triangle_fan(polygons, triangle_fans)) 
    {
      found = true;
    }

  if (triangle_fans.size() > 0)
    {
      osg::notify(INFO) << "LWO2 loader, optimizing: found " << triangle_fans.size() << " triangle fans" << endl;
    }

  return found;
}

bool 
Lwo2Layer::_find_triangle_strips(PolygonsList& polygons, PolygonsList& triangle_strips)
{
  bool found = false;

  while (_find_triangle_strip(polygons, triangle_strips)) 
    {
      found = true;
    }

  if (triangle_strips.size() > 0)
    {
      osg::notify(INFO) << "LWO2 loader, optimizing: found " << triangle_strips.size() << " triangle strips" << endl;
    }

  return found;
}

bool 
Lwo2Layer::_find_triangle_fan(PolygonsList& polygons, PolygonsList& triangle_fans)
{
  bool found = false;
  IteratorPolygonsList polygon_iterator = polygons.begin();
  while (polygon_iterator != polygons.end())
    {
      PointsList& points_list = *polygon_iterator;
      if (points_list.size() == 3 && points_list[0].point_index != -1)
        {
          PointData a = points_list[0];
          PointData b = points_list[1];
          PointData c = points_list[2];

          int next_polygon_index = _find_triangle_begins_with(polygons, a, c);
          while (next_polygon_index >= 0) 
            {
              found = true;
              PointData d = polygons[next_polygon_index][2];

              PointsList point_list;
              point_list.push_back(a);
              point_list.push_back(b);
              point_list.push_back(c);
              point_list.push_back(d);

              // delete second triangle (mark as deleted)
              (*(polygons.begin() + next_polygon_index))[0].point_index = -1;

              // delete current (first) triangle (mark as deleted)
              (*polygon_iterator)[0].point_index = -1;

              c = d;
              while ((next_polygon_index = _find_triangle_begins_with(polygons, a, c)) >= 0)
                {
                  PointData d = polygons[next_polygon_index][2];
                  point_list.push_back(d);

                  // delete next triangle (mark as deleted)
                  (*(polygons.begin() + next_polygon_index))[0].point_index = -1;

                  c = d;
                }
              
              triangle_fans.push_back(point_list);
            }
        }
      polygon_iterator++;
    }
  return found;
}

bool 
Lwo2Layer::_find_triangle_strip(PolygonsList& polygons, PolygonsList& triangle_strips)
{
  bool found = false;
  IteratorPolygonsList polygon_iterator = polygons.begin();
  int polygon_index = 0;
  while (polygon_iterator != polygons.end())
    {
      PointsList& points_list = *polygon_iterator;
      if (points_list.size() == 3 && points_list[0].point_index != -1)
        {
          PointData a = points_list[0];
          PointData b = points_list[1];
          PointData c = points_list[2];

          int next_polygon_index = _find_triangle_begins_with(polygons, c, b);

          while (next_polygon_index >= 0) 
            {
              found = true;
              PointData d = polygons[next_polygon_index][2];

              PointsList point_list;
              point_list.push_back(a);
              point_list.push_back(b);
              point_list.push_back(c);
              point_list.push_back(d);

              // delete second triangle (mark as deleted)
              (*(polygons.begin() + next_polygon_index))[0].point_index = -1;

              // delete current (first) triangle (mark as deleted)
              (*polygon_iterator)[0].point_index = -1;

              PointData strip_a = c;
              PointData strip_b = d;
              bool current_strip_a = true;

              while ((next_polygon_index = _find_triangle_begins_with(polygons, strip_a, strip_b)) >= 0)
                {
                  PointData d = polygons[next_polygon_index][2];
                  point_list.push_back(d);

                  if (current_strip_a)
                    {
                      strip_a = d;
                    }
                  else 
                    {
                      strip_b = d;
                    }
                  current_strip_a = !current_strip_a;

                  // delete next triangle (mark as deleted)
                  (*(polygons.begin() + next_polygon_index))[0].point_index = -1;
                }
              
              triangle_strips.push_back(point_list);
            }
        }
      polygon_iterator++;
      polygon_index++;
    }
  return found;
}

int 
Lwo2Layer::_find_triangle_begins_with(PolygonsList& polygons, PointData& a, PointData& b)
{
  int result = -1;

  int polygon_index = 0;
  for (IteratorPolygonsList polygon_iterator = polygons.begin(); polygon_iterator != polygons.end(); polygon_iterator++, polygon_index++)
    {
      PointsList& points_list = *polygon_iterator;
      if (points_list.size() == 3 && points_list[0].point_index != -1)
        {
          if (points_list[0] == a && points_list[1] == b)
            {
              result = polygon_index;
              break;
            }
          else if (points_list[1] == a && points_list[2] == b)
            {
              // shift points
              PointData temp = points_list[0];
              points_list[0] = points_list[1];
              points_list[1] = points_list[2];
              points_list[2] = temp;

              result = polygon_index;
              break;
            }
          else if (points_list[2] == a && points_list[0] == b)
            {
              // shift points
              PointData temp = points_list[2];
              points_list[2] = points_list[1];
              points_list[1] = points_list[0];
              points_list[0] = temp;

              result = polygon_index;
              break;
            }
        }
    }
  return result;
}
