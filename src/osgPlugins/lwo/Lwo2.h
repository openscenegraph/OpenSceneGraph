#ifndef LWO2_H
#define LWO2_H 1

#include <vector>
#include <map>
#include <string>
#include <fstream>

#include <osg/Referenced>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Geometry>
#include <osg/Notify>

using namespace osg;
using namespace std;

class Lwo2Layer;
struct Lwo2Surface;
struct Lwo2PolygonMapping;

typedef vector< short > PointsList;

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
  bool GenerateGeode( Geode& );

 private:
  map< int, Lwo2Layer* > _layers;
  map< string, Lwo2Surface* > _surfaces;
  Lwo2Layer* _current_layer;
  vector< string > _tags;
  vector< string > _images;
  ifstream _fin;
  Geode* _geode;

  unsigned char _read_char();
  unsigned short _read_short();
  unsigned long _read_long();
  float _read_float();
  string& _read_string(string&);

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
};

// makes 4-byte integer tag from four chars 
// used in IFF standard

unsigned long make_id(const char* tag);

const unsigned long tag_FORM = make_id("FORM");
const unsigned long tag_LWO2 = make_id("LWO2");
const unsigned long tag_LAYR = make_id("LAYR");
const unsigned long tag_TAGS = make_id("TAGS");
const unsigned long tag_PNTS = make_id("PNTS");
const unsigned long tag_VMAP = make_id("VMAP");
const unsigned long tag_VMAD = make_id("VMAD");
const unsigned long tag_TXUV = make_id("TXUV");
const unsigned long tag_POLS = make_id("POLS");
const unsigned long tag_FACE = make_id("FACE");
const unsigned long tag_PTAG = make_id("PTAG");
const unsigned long tag_SURF = make_id("SURF");
const unsigned long tag_CLIP = make_id("CLIP");
const unsigned long tag_STIL = make_id("STIL");
const unsigned long tag_BLOK = make_id("BLOK");
const unsigned long tag_IMAP = make_id("IMAP");
const unsigned long tag_TMAP = make_id("TMAP");
const unsigned long tag_IMAG = make_id("IMAG");

#endif
