#ifndef LWO2LAYER_H
#define LWO2LAYER_H 1

#include <vector>
#include <map>
#include <string>

#include <osg/Referenced>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Notify>
#include <osg/Geometry>

#include <osgUtil/SmoothingVisitor>

using namespace osg;
using namespace std;

typedef vector< short > PointsList;

typedef vector< PointsList* >::iterator IteratorPointsList;
typedef vector< Vec3 >::iterator IteratorVec3;
typedef vector< Vec2 >::iterator IteratorVec2;
typedef vector< short >::iterator IteratorShort;

struct Lwo2Surface
{
  short image_index;
  string name;
};

struct Lwo2PolygonMapping
{
  short polygon_index;
  Vec2 uv;
};

class Lwo2Layer
{
 public:
  friend class Lwo2;
  Lwo2Layer();
  ~Lwo2Layer();
  void notify(NotifySeverity);
  void GenerateGeometry( Geometry& );

 private:
  short _number;
  short _flags;
  short _parent;
  Vec3 _pivot;
  string _name;
  vector< Vec3 > _points;
  vector< Vec2 > _points_map;
  vector< PointsList* > _polygons;
  vector< short > _polygons_tag;
  multimap< short, Lwo2PolygonMapping > _polygons_map;
};
 
#endif
