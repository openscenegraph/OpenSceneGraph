#include <iostream>
#include <fstream>

#include <osg/Notify>
#include <osg/Geode>
#include <osg/Texture>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include "Lwo2.h"
#include "Lwo2Layer.h"

Lwo2::Lwo2()
{
};

Lwo2::~Lwo2()
{
  // delete all layers
  for (IteratorLayers itr = _layers.begin(); itr != _layers.end(); itr++)
    {
      delete (*itr).second;
    }

  // delete all surfaces
  for (IteratorSurfaces itr_surf = _surfaces.begin(); itr_surf != _surfaces.end(); itr_surf++)
    {
      delete (*itr_surf).second;
    }
};

bool 
Lwo2::ReadFile( const string& filename )
{
  notify(INFO)  << "Opening file: " << filename << endl;

  _fin.open(filename.c_str(), ios::in | ios::binary );
  if (!_fin.is_open())
    {
      notify(FATAL) << "Can't open file '" << filename << "'" << endl;
      return false;
    }

  // checking EA-IFF85 format
  // http://www.lightwave3d.com/developer/75lwsdk/docs/filefmts/eaiff85.html
  if (_read_long() != tag_FORM) 
    {
      notify(FATAL) << "File '" << filename << "' is not IFF format file." << endl;
      _fin.close();
      return false;
    }
  else 
    {
      notify(INFO) << "Detected EA-IFF85 format" << endl;
    }

  unsigned long form_size = _read_long();
  notify(DEBUG_INFO) << "Form size: " << form_size << endl;

  // checking LWO2 format 
  // http://www.lightwave3d.com/developer/75lwsdk/docs/filefmts/lwo2.html
  if (_read_long() != tag_LWO2) 
    {
      unsigned long make_id(const char*);
      notify(DEBUG_INFO) << "File '" << filename << "' is not LWO2 format file." << endl;
      _fin.close();
      return false;
    }
  else 
    {
      notify(INFO) << "Detected LWO2 format" << endl;
    }

  _geode = new osg::Geode();

  unsigned long read_bytes = 4;
  unsigned long current_tag_name;
  unsigned long current_tag_size;

  // main loop for reading tags
  while (read_bytes < form_size && !_fin.eof()) {
    current_tag_name = _read_long();
    current_tag_size = _read_long();
    read_bytes += 8 + current_tag_size + current_tag_size % 2;

    _print_tag(current_tag_name, current_tag_size);

    if (current_tag_name == tag_TAGS) 
      {
	_read_tag_strings(current_tag_size);
      } 
    else if (current_tag_name == tag_LAYR) 
      {
	_read_layer(current_tag_size);
      }
    else if (current_tag_name == tag_PNTS) 
      {
	_read_points(current_tag_size);
      }
    else if (current_tag_name == tag_VMAP) 
      {
	_read_vertex_mapping(current_tag_size);
      }
    else if (current_tag_name == tag_VMAD) 
      {
	_read_polygons_mapping(current_tag_size);
      }
    else if (current_tag_name == tag_POLS) 
      {
	_read_polygons(current_tag_size);
      }
    else if (current_tag_name == tag_PTAG) 
      {
	_read_polygon_tag_mapping(current_tag_size);
      }
    else if (current_tag_name == tag_CLIP) 
      {
	_read_image_definition(current_tag_size);
      }
    else if (current_tag_name == tag_SURF) 
      {
	_read_surface(current_tag_size);
      } 
    else 
      {
	_fin.seekg(current_tag_size + current_tag_size % 2, ios::cur);
      }
  }

  _fin.close();

  return true;
}

unsigned char 
Lwo2::_read_char()
{
  char c;
  if (_fin.is_open())
    {
      _fin.read(&c, 1);
    }
  return static_cast<unsigned char>(c);
}

unsigned long 
Lwo2::_read_long()
{
  return
    (_read_char() << 24) | 
    (_read_char() << 16) |  
    (_read_char() <<  8) | 
    _read_char();
}

unsigned short 
Lwo2::_read_short()
{
  return
    (_read_char() <<  8) | 
    _read_char();
}

float 
Lwo2::_read_float()
{
  unsigned long x = _read_long();
  return *(float*)&x;
}

// read null terminated string 

string&
Lwo2::_read_string(string& str)
{
  char c;
  do {
    str += c = _read_char();
  } while (c != 0);

  // if length of string (including \0) is odd skip another byte 
  if (str.length() % 2) {
    _read_char();
  }

  return str;
}

// print 4-char tag to debug out

void 
Lwo2::_print_tag(unsigned int tag, unsigned int size) {
  notify(DEBUG_INFO) << "Found tag " 
		     << char(tag >> 24) 
		     << char(tag >> 16) 
		     << char(tag >>  8) 
		     << char(tag) 
		     << " size " << size << " bytes" 
		     << endl;
}

// print 4-char type
void 
Lwo2::_print_type(unsigned int type) {
  notify(DEBUG_INFO) << "  type   \t" 
		     << char(type >> 24) 
		     << char(type >> 16) 
		     << char(type >>  8) 
		     << char(type) 
		     << endl;
}

// read TAGS info

void 
Lwo2::_read_tag_strings(unsigned long size)
{
  while (size > 0)
    {
      string name;
      _read_string(name);
      size -= name.length() + name.length() % 2; 
      _tags.push_back(name);

      notify(DEBUG_INFO) << "  name   \t'" << name << "'" << endl;
    }
}

// read LAYR info

void 
Lwo2::_read_layer(unsigned long size) 
{
  unsigned short number = _read_short();
  size -= 2;

  Lwo2Layer* layer = new Lwo2Layer();
  _layers[number] = layer;
  _current_layer = layer;
  layer->_number = number;

  layer->_flags = _read_short();
  size -= 2;

  layer->_pivot.set(_read_float(), _read_float(), _read_float());
  size -= 4 * 3;

  _read_string(layer->_name);
  size -= layer->_name.length() + layer->_name.length() % 2; 

  if (size > 2) {
    layer->_parent = _read_short();
    size -= 2;
  }

  _fin.seekg(size + size % 2, ios::cur);
}

// read PNTS info

void 
Lwo2::_read_points(unsigned long size) 
{
  int count = size / 12;
  notify(DEBUG_INFO) << "  count \t" << count << endl;

  while (count--)
    {
      _current_layer->_points.push_back(Vec3(_read_float(), _read_float(), _read_float()));
    } 
}

// read POLS info

void 
Lwo2::_read_polygons(unsigned long size) 
{
  unsigned int type = _read_long();
  size -= 4;

  _print_type(type);

  if (type == tag_FACE) 
    {
      unsigned short vertex_count;

      while (size > 0)
	{
	  vertex_count = _read_short() & 0x03FF;
	  size -= 2;

          PointsList* points_list = new PointsList;
          _current_layer->_polygons.push_back(points_list);
      
	  while (vertex_count--)
	    {
	      points_list->push_back(_read_short());
	      size -= 2;
	    }     
	}
    }
  else 
    {
  
      // not recognized yet
      notify(DEBUG_INFO) << "  skipping..." << endl;
      _fin.seekg(size + size % 2, ios::cur);
    }
}

// read VMAP info

void 
Lwo2::_read_vertex_mapping(unsigned long size) 
{
  unsigned int type = _read_long();
  size -= 4;

  _print_type(type);

  short dimension = _read_short();
  size -= 2;

  notify(DEBUG_INFO) << "  dimension \t" << dimension << endl;

  string name;
  _read_string(name);
  size -= name.length() + name.length() % 2; 
  notify(DEBUG_INFO) << "  name   \t'" << name << "'" << endl;

  if (type == tag_TXUV && dimension == 2) 
    {
      int count = size / 10;
      _current_layer->_points_map.resize(count);

      short n;
      float u;
      float v;
      while (count--)
	{
	  n = _read_short();
	  u = _read_float();
	  v = _read_float();
	  _current_layer->_points_map[n].set(u, v);
	}
    }
  else 
    {
  
      // not recognized yet
      notify(DEBUG_INFO) << "  skipping..." << endl;
      _fin.seekg(size + size % 2, ios::cur);
    }
	 
}

// read VMAD info

void 
Lwo2::_read_polygons_mapping(unsigned long size) 
{
  unsigned int type = _read_long();
  size -= 4;

  _print_type(type);

  short dimension = _read_short();
  size -= 2;

  notify(DEBUG_INFO) << "  dimension \t" << dimension << endl;

  string name;
  _read_string(name);
  size -= name.length() + name.length() % 2; 
  notify(DEBUG_INFO) << "  name   \t'" << name << "'" << endl;

  if (type == tag_TXUV && dimension == 2) 
    {
      int count = size / 12;
      //      _current_layer->_points_map.resize(count);

      short point_index;
      short polygon_index;
      float u;
      float v;
      while (count--)
	{
	  point_index = _read_short();
	  polygon_index = _read_short();
	  u = _read_float();
	  v = _read_float();

	  Lwo2PolygonMapping pm(polygon_index, Vec2(u, v));
	  _current_layer->_polygons_map.insert(PairVMAD(point_index, pm));

	}
    }
  else 
    {
  
      // not recognized yet
      notify(DEBUG_INFO) << "  skipping..." << endl;
      _fin.seekg(size + size % 2, ios::cur);
    }
	 
}

// read PTAG info

void 
Lwo2::_read_polygon_tag_mapping(unsigned long size) 
{
  unsigned int type = _read_long();
  size -= 4;

  _print_type(type);

  if (type == tag_SURF) 
    {
      int count = size / 4;
      _current_layer->_polygons_tag.resize(count);

      short polygon_index;
      short tag_index;

      while (count--)
	{
	  polygon_index = _read_short();
	  tag_index = _read_short();
	  _current_layer->_polygons_tag[polygon_index] = tag_index;
	}
    }
  else 
    {
  
      // not recognized yet
      notify(DEBUG_INFO) << "  skipping..." << endl;
      _fin.seekg(size + size % 2, ios::cur);
    }
}

// read CLIP info

void 
Lwo2::_read_image_definition(unsigned long size)
{
  unsigned int index = _read_long();
  size -= 4;
  notify(DEBUG_INFO) << "  index  \t" << index << endl;

  unsigned int type;
  while (size > 0)
    {
      type = _read_long();
      size -= 4;

      _print_type(type);

      // size of name
      // not included in specification ??
      _read_short();
      size -= 2;

      string name;
      _read_string(name);
      size -= name.length() + name.length() % 2; 
		
      if (index + 1 > _images.size())
	{
	  _images.resize(index + 1);
	}
		
      _images[index] = name.c_str();
		
      notify(DEBUG_INFO) << "  name   \t'" << name << "'" << endl;
    }
}

// read SURF info

void 
Lwo2::_read_surface(unsigned long size)
{
  Lwo2Surface* surface = new Lwo2Surface();

  _read_string(surface->name);
  size -= surface->name.length() + surface->name.length() % 2; 
  notify(DEBUG_INFO) << "  name   \t'" << surface->name << "'" << endl;

  string source;
  _read_string(source);
  size -= source.length() + source.length() % 2; 
  notify(DEBUG_INFO) << "  source   \t'" << source << "'" << endl;

  unsigned long current_tag_name;
  unsigned short current_tag_size;

  while (size > 0) 
    {
      current_tag_name = _read_long();
      size -= 4;
      current_tag_size = _read_short();
      size -= 2;

      _print_tag(current_tag_name, current_tag_size);
    
      if (current_tag_name == tag_BLOK) 
	{

	  // BLOK
	  int blok_size = current_tag_size;
	  size -= blok_size;
	  while (blok_size > 0) 
	    {
	      current_tag_name = _read_long();
	      blok_size -= 4;
	      current_tag_size = _read_short();
	      blok_size -= 2;
	      notify(DEBUG_INFO) << "  ";
	      _print_tag(current_tag_name, current_tag_size);

	      if (current_tag_name == tag_IMAG)
		{
		  surface->image_index = _read_short();
		  notify(DEBUG_INFO) << "    image index\t" << surface->image_index << endl;
		  blok_size -= 2;
		}
	      else if (current_tag_name == tag_IMAP) 
		{
           
		  // IMAP
		  int imap_size = current_tag_size;
		  blok_size -= imap_size;
            
		  string ordinal;
		  _read_string(ordinal);
		  imap_size -= ordinal.length() + ordinal.length() % 2; 
		  notify(DEBUG_INFO) << "    ordinal   \t'" << ordinal << "'" << endl;

		  while(imap_size > 0)
		    {
		      current_tag_name = _read_long();
		      imap_size -= 4;
		      current_tag_size = _read_short();
		      imap_size -= 2;
		      notify(DEBUG_INFO) << "    ";
		      _print_tag(current_tag_name, current_tag_size);

                      _fin.seekg(current_tag_size + current_tag_size % 2, ios::cur);
		      imap_size -= current_tag_size + current_tag_size % 2;
		    }
		}
	      else 
		{
		  _fin.seekg(current_tag_size + current_tag_size % 2, ios::cur);
		  blok_size -= current_tag_size + current_tag_size % 2;
		}
	    }
	}
      else 
	{
	  _fin.seekg(current_tag_size + current_tag_size % 2, ios::cur);
	  size -= current_tag_size + current_tag_size % 2;
	}
    }

  _surfaces[surface->name] = surface;
}

// Generation OSG Geode object from parsed LWO2 file

bool 
Lwo2::GenerateGeode( Geode& geode )
{

  // loading images
  vector< Image* > _loaded_images;
  osg::notify(DEBUG_INFO) << "  images:\t" << _images.size() << endl;
  IteratorString string_itr;
  for (string_itr = _images.begin(); string_itr != _images.end(); string_itr++)
    {
      string name = *string_itr;
      if (name.length() > 1)
	{
	  notify(DEBUG_INFO) << "\tloading '" << name << "'" << endl;
	  Image* image = osgDB::readImageFile(name);
	  notify(DEBUG_INFO) << "\tresult - " << image << endl;
	  _loaded_images.push_back(image);
	}
      else
	{
	  _loaded_images.push_back(0);
	}
    }

  // create geometry from all layers
  for (IteratorLayers itr = _layers.begin(); itr != _layers.end(); itr++)
    {
      osg::Geometry* geometry = new osg::Geometry();
      (*itr).second->GenerateGeometry(*geometry);

      // assign StateSet for each PTAG group

      // simple case with one texture only
      if (_tags.size() == 1)
	{
	  Image* image = _loaded_images[_surfaces[_tags[0]]->image_index];
	  if (image)
	    {
	      StateSet* state_set = new osg::StateSet;

	      Texture* texture = new osg::Texture;
	      texture->setImage(image);
                        
	      state_set->setTextureAttributeAndModes(0, texture, StateAttribute::ON);
                        
	      geometry->setStateSet(state_set);
	    }
	}

      geode.addDrawable(geometry);
    }

  return true;
}

// makes 4-byte integer tag from four chars 
// used in IFF standard

unsigned long
make_id(const char* tag) 
{
  unsigned long result = 0;
  for (unsigned int i = 0; i < strlen(tag) && i < 4; i++)
    {
      result <<= 8;
      result += int(tag[i]);
    }
  return result;
}                   
