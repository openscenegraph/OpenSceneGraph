#include "Lwo2Layer.h"

Lwo2Layer::Lwo2Layer()
{
  _parent = 0;
}

Lwo2Layer::~Lwo2Layer()
{

  // deleting points lists
  IteratorPointsList pol_itr;
  for (pol_itr = _polygons.begin(); pol_itr != _polygons.end(); pol_itr++)
    {
      delete (*pol_itr);
    }
}


void
Lwo2Layer::notify(NotifySeverity severity)
{
  osg::notify(severity) << "Current layer: " << _number << endl;
  osg::notify(severity) << "  flags  \t" << _flags << endl;
  osg::notify(severity) << "  pivot  \t" << _pivot << endl;
  osg::notify(severity) << "  name:  \t'" << _name << "'" << endl;
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
Lwo2Layer::GenerateGeometry( Geometry& geometry )
{
    notify(DEBUG_INFO);

    IteratorPointsList pol_itr;
    Vec3Array* coords = new Vec3Array;

    // create texture array
    Vec2Array* texcoords = 0;
    if (_points_map.size() > 0)
    {
          texcoords = new Vec2Array;
    }

    // variables for VMAD data processing
    pair<multimap< short, Lwo2PolygonMapping >::iterator,
    multimap< short, Lwo2PolygonMapping >::iterator> range; 
    multimap< short, Lwo2PolygonMapping >::iterator itr;

    // all polygons
    int polygon_index = 0;
    for (pol_itr = _polygons.begin(); pol_itr != _polygons.end(); pol_itr++, polygon_index++)
    {
        vector< short >::reverse_iterator short_itr;
        for (short_itr = (*pol_itr)->rbegin(); short_itr != (*pol_itr)->rend(); short_itr++)
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
        geometry.addPrimitive(new DrawArrays(Primitive::POLYGON, 
                                                        (*coords).size() - (*pol_itr)->size(), 
                                                        (*pol_itr)->size()));
    }
    geometry.setVertexArray(coords);

    // assign texture array
    if (_points_map.size() > 0)
    {
        geometry.setTexCoordArray(0, texcoords);
    }

    // generate normals
    osgUtil::SmoothingVisitor smoother;
    smoother.smooth(geometry);
}
