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

#include "Lwo2Layer.h"

Lwo2Layer::Lwo2Layer():
  _number(0),
  _flags(0),
  _parent(0)
{
}

Lwo2Layer::~Lwo2Layer()
{

  // deleting points lists
  IteratorPointsList pol_itr;
  for (pol_itr = _polygons.begin(); pol_itr != _polygons.end(); pol_itr++)
    {
      osgDelete (*pol_itr);
    }
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
  IteratorVec3 itr;
  for (itr = _points.begin(); itr != _points.end(); itr++)
    {
      osg::notify(severity) << "    \t" << (*itr) << endl;
    }
  
  // points mappings
  osg::notify(severity) << "  points mappings:\t" << _points_map.size() << endl;
  IteratorVec2 itr_vec2;
  for (itr_vec2 = _points_map.begin(); itr_vec2 != _points_map.end(); itr_vec2++)
    {
      osg::notify(severity) << "    \t" << (*itr_vec2) << endl;
    }
  
  // polygons
  osg::notify(severity) << "  polygons:\t" << _polygons.size() << endl;
  IteratorPointsList pol_itr;
  IteratorShort short_itr;
  for (pol_itr = _polygons.begin(); pol_itr != _polygons.end(); pol_itr++)
    {
      osg::notify(severity) << "    " << (*pol_itr)->size() << ":";
      for (short_itr = (*pol_itr)->begin(); short_itr != (*pol_itr)->end(); short_itr++)
    {
      osg::notify(severity) << "\t" << (*short_itr);
    }
      osg::notify(severity) << endl;
    }

  // polygons tags
  osg::notify(severity) << "  polygons tags:\t" << _polygons_tag.size() << endl;
  for (short_itr = _polygons_tag.begin(); short_itr != _polygons_tag.end(); short_itr++)
    {
      osg::notify(severity) << "\t" << (*short_itr) << endl;
    }
}

void
Lwo2Layer::GenerateGeode( Geode& geode, short tags_count )
{
  notify(DEBUG_INFO);
  
  // create diffirent geomerty for each tag
  for (short current_tag = 0; current_tag < tags_count; current_tag++)
    {
      // new geometry
      ref_ptr<Geometry> geometry = osgNew Geometry;

      // create coords array
      ref_ptr<Vec3Array> coords = osgNew Vec3Array;

      // create texture array
      ref_ptr<Vec2Array> texcoords = osgNew Vec2Array;

      // variables for VMAD data processing
      pair<multimap< short, Lwo2PolygonMapping >::iterator,
      multimap< short, Lwo2PolygonMapping >::iterator> range; 
      multimap< short, Lwo2PolygonMapping >::iterator itr;

      // all polygons
      int polygon_index = 0;
      for (IteratorPointsList pol_itr = _polygons.begin(); pol_itr != _polygons.end(); pol_itr++, polygon_index++)
    {

      // polygons of current tag only
      if (_polygons_tag[polygon_index] == current_tag)
        {

          // all points
          vector< short >::iterator short_itr;
          for (short_itr = (*pol_itr)->begin(); short_itr != (*pol_itr)->end(); short_itr++)
        {

          // polygons coords
          (*coords).push_back(_points[*short_itr]);

          // point texture coords
          if (_points_map.size() > 0)
            {
              // VMAP data
              Vec2 uv = _points_map[*short_itr];

              // chech if present VMAD data
              if (_polygons_map.size() > 0)
            {

              // select data for current point
              range = _polygons_map.equal_range(*short_itr); 

              for (itr = range.first; itr != range.second; itr++)
                {

                  // found current polygon
                  if ((*itr).second.polygon_index == polygon_index)
                {
                  uv = (*itr).second.uv;
                }
                }
            }

              (*texcoords).push_back(uv);
            }
        }
          geometry->addPrimitiveSet(osgNew DrawArrays(PrimitiveSet::POLYGON, 
                           (*coords).size() - (*pol_itr)->size(), 
                           (*pol_itr)->size()));
        }
    }

      // add geometry if it contains any points
      if (coords->size() != 0) 
    {
      geometry->setVertexArray(coords.get());

      // assign texture array
      if ((*texcoords).size() > 0)
        {
          geometry->setTexCoordArray(0, texcoords.get());
        }

      // generate normals
      osgUtil::SmoothingVisitor smoother;
      smoother.smooth(*(geometry.get()));

      geode.addDrawable(geometry.get());
    }
    }
}
