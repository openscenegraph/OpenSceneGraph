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

#include <osg/Notify>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Texture2D>
#include <osg/Material>
#include <osg/CullFace>
#include <osg/BlendFunc>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <iostream>
#include <fstream>
#include <string.h>

#include "old_Lwo2.h"
#include "old_Lwo2Layer.h"
#include "lwo2read.h"

Lwo2::Lwo2():
  _current_layer(0),
  _successfully_read(false)
{
}

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
}

bool
Lwo2::ReadFile( const string& filename )
{
    OSG_INFO  << "Opening file: " << filename << std::endl;

    _fin.open(filename.c_str(), ios::in | ios::binary );
    if (!_fin.is_open())
    {
        OSG_INFO << "Can't open file '" << filename << "'" << std::endl;
        return false;
    }

    // checking EA-IFF85 format
    // http://www.lightwave3d.com/developer/75lwsdk/docs/filefmts/eaiff85.html
    if (_read_uint() != tag_FORM)
    {
        OSG_INFO << "File '" << filename << "' is not IFF format file." << std::endl;
        _fin.close();
        return false;
    }
    else
    {
        OSG_INFO << "Detected EA-IFF85 format" << std::endl;
    }

    unsigned int form_size = _read_uint();
    OSG_INFO << "Form size: " << form_size << std::endl;

    // checking LWO2 format
    // http://www.lightwave3d.com/developer/75lwsdk/docs/filefmts/lwo2.html
    if (_read_uint() != tag_LWO2)
    {
        unsigned long make_id(const char*);
        OSG_INFO << "File '" << filename << "' is not LWO2 format file." << std::endl;
        _fin.close();
        return false;
    }
    else
    {
        OSG_INFO << "Detected LWO2 format" << std::endl;
    }

    unsigned long read_bytes = 4;
    unsigned long current_tag_name;
    unsigned long current_tag_size;

    // main loop for reading tags
    while (read_bytes < form_size && !_fin.eof())
    {
        current_tag_name = _read_uint();
        current_tag_size = _read_uint();
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

    return _successfully_read = true;
}

unsigned char
Lwo2::_read_char()
{
  char c = 0;
  if (_fin.is_open())
    {
      _fin.read(&c, 1);
    }
  return static_cast<unsigned char>(c);
}

unsigned int
Lwo2::_read_uint()
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
  return lwo2::changeType4<float, unsigned int>(_read_uint());
}

// read null terminated string

string&
Lwo2::_read_string(string& str)
{
  char c;
  do {
    c = _read_char();
    str += c;
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
  OSG_DEBUG << "Found tag "
                     << char(tag >> 24)
                     << char(tag >> 16)
                     << char(tag >>  8)
                     << char(tag)
                     << " size " << size << " bytes"
                     << std::endl;
}

// print 4-char type
void
Lwo2::_print_type(unsigned int type) {
  OSG_DEBUG << "  type   \t"
                     << char(type >> 24)
                     << char(type >> 16)
                     << char(type >>  8)
                     << char(type)
                     << std::endl;
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

        OSG_DEBUG << "  name   \t'" << name.c_str() << "'" << std::endl;
    }
}

// read LAYR info

void Lwo2::_read_layer(unsigned long size)
{
    unsigned short number = _read_short();
    size -= 2;

    Lwo2Layer* layer = new Lwo2Layer();
    _layers[number] = layer;
    _current_layer = layer;
    layer->_number = number;

    layer->_flags = _read_short();
    size -= 2;

    float x = _read_float();
    float y = _read_float();
    float z = _read_float();
    layer->_pivot.set(x, y, z);
    size -= 4 * 3;

    _read_string(layer->_name);
    size -= layer->_name.length() + layer->_name.length() % 2;

    if (size > 2)
    {
        layer->_parent = _read_short();
        size -= 2;
    }

    _fin.seekg(size + size % 2, ios::cur);
}

// read PNTS info

void Lwo2::_read_points(unsigned long size)
{
    int count = size / 12;
    OSG_DEBUG << "  count \t" << count << std::endl;

    while (count--)
    {
        PointData point;

        float x = _read_float();
        float y = _read_float();
        float z = _read_float();
        point.coord = Vec3(x, y, z);
        _current_layer->_points.push_back(point);
    }
}

// read VMAP info

void Lwo2::_read_vertex_mapping(unsigned long size)
{
    unsigned int type = _read_uint();
    size -= 4;

    _print_type(type);

    short dimension = _read_short();
    size -= 2;

    OSG_DEBUG << "  dimension \t" << dimension << std::endl;

    string name;
    _read_string(name);
    size -= name.length() + name.length() % 2;
    OSG_DEBUG << "  name   \t'" << name.c_str() << "'" << std::endl;

    if (type == tag_TXUV && dimension == 2)
    {
        int count = size / 10;
        unsigned short n;
        float u;
        float v;
        while (count--)
        {
            n = _read_short();
            u = _read_float();
            v = _read_float();

            // point coords must be read previously
            if (n < _current_layer->_points.size())
            {
                _current_layer->_points[n].texcoord = Vec2(u, v);
            }
        }
    }
    else
    {

        // not recognized yet
        OSG_DEBUG << "  skipping..." << std::endl;
        _fin.seekg(size + size % 2, ios::cur);
    }
}

// read POLS info

void
Lwo2::_read_polygons(unsigned long size)
{
    unsigned int type = _read_uint();
    size -= 4;

    _print_type(type);

    if (type == tag_FACE)
    {
        unsigned short vertex_count;

        while (size > 0)
        {
            PointData point;
            vertex_count = _read_short() & 0x03FF;
            size -= 2;

            PointsList points_list;

            while (vertex_count--)
            {
                unsigned short point_index = _read_short();

                point = _current_layer->_points[point_index];
                point.point_index = point_index;

                points_list.push_back(point);
                size -= 2;
            }

            _current_layer->_polygons.push_back(points_list);
        }
    }
    else
    {

        // not recognized yet
        OSG_DEBUG << "  skipping..." << std::endl;
        _fin.seekg(size + size % 2, ios::cur);
    }
}

// read PTAG info

void Lwo2::_read_polygon_tag_mapping(unsigned long size)
{
    unsigned int type = _read_uint();
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
        OSG_DEBUG << "  skipping..." << std::endl;
        _fin.seekg(size + size % 2, ios::cur);
    }
}

// read VMAD info

void Lwo2::_read_polygons_mapping(unsigned long size)
{
    unsigned int type = _read_uint();
    size -= 4;

    _print_type(type);

    short dimension = _read_short();
    size -= 2;

    OSG_DEBUG << "  dimension \t" << dimension << std::endl;

    string name;
    _read_string(name);
    size -= name.length() + name.length() % 2;
    OSG_DEBUG << "  name   \t'" << name.c_str() << "'" << std::endl;

    if (type == tag_TXUV && dimension == 2)
    {
        OSG_DEBUG << "  polygons mappings:" << endl;
        OSG_DEBUG << "\tpoint\tpolygon\ttexcoord" <<  endl;
        OSG_DEBUG << "\t=====\t=======\t========" <<  endl;

        int count = size / 12;

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

            OSG_DEBUG << "    \t" << point_index << "\t" << polygon_index << "\t" << Vec2(u, v) << endl;

            // apply texture coordinates
            PointsList& points_list = _current_layer->_polygons[polygon_index];
            for (unsigned int i = 0; i < points_list.size(); i++)
            {
                if (points_list[i].point_index == point_index)
                {
                    points_list[i].texcoord = Vec2(u, v);
                }
            }
        }
    }
    else
    {

        // not recognized yet
        OSG_DEBUG << "  skipping..." << std::endl;
        _fin.seekg(size + size % 2, ios::cur);
    }

}

// read CLIP info

void
Lwo2::_read_image_definition(unsigned long size)
{
    unsigned int index = _read_uint();
    size -= 4;
    OSG_DEBUG << "  index  \t" << index << std::endl;

    unsigned int type;
    while (size > 0)
    {
        type = _read_uint();
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

        OSG_DEBUG << "  name   \t'" << name.c_str() << "'" << std::endl;
    }
}

// read SURF info

void Lwo2::_read_surface(unsigned long size)
{
    Lwo2Surface* surface = new Lwo2Surface();
    surface->image_index = -1;
    surface->state_set = NULL;

    _read_string(surface->name);
    size -= surface->name.length() + surface->name.length() % 2;
    OSG_DEBUG << "  name   \t'" << surface->name.c_str() << "'" << std::endl;

    string source;
    _read_string(source);
    size -= source.length() + source.length() % 2;
    OSG_DEBUG << "  source   \t'" << source.c_str() << "'" << std::endl;

    unsigned long current_tag_name;
    unsigned short current_tag_size;

    while (size > 0 && !_fin.eof())
    {
        current_tag_name = _read_uint();
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
                current_tag_name = _read_uint();
                blok_size -= 4;
                current_tag_size = _read_short();
                blok_size -= 2;
                OSG_DEBUG << "  ";
                _print_tag(current_tag_name, current_tag_size);

                if (current_tag_name == tag_IMAG)
                {
                    surface->image_index = _read_short();
                    OSG_DEBUG << "    image index\t" << surface->image_index << std::endl;
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
                    OSG_DEBUG << "    ordinal   \t'" << ordinal.c_str() << "'" << std::endl;

                    while(imap_size > 0)
                    {
                        current_tag_name = _read_uint();
                        imap_size -= 4;
                        current_tag_size = _read_short();
                        imap_size -= 2;
                        OSG_DEBUG << "    ";
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
        else if (current_tag_name == tag_COLR)
        {
            float r = _read_float();
            float g = _read_float();
            float b = _read_float();
            surface->color.set(r,g,b);
            OSG_DEBUG << "  color   \t" << surface->color << std::endl;
            current_tag_size -= 12;
            size -= 12;

            // skip ununderstooded envelope
            _fin.seekg(current_tag_size + current_tag_size % 2, ios::cur);
            size -= current_tag_size + current_tag_size % 2;
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
Lwo2::GenerateGroup( Group& group )
{
  if (!_successfully_read) return false;

  // generate StateSets for each surface
  _generate_statesets_from_surfaces();

  // create geometry from all layers
  for (IteratorLayers itr = _layers.begin(); itr != _layers.end(); itr++)
    {
      osg::Geode* geode = new osg::Geode();

      OSG_DEBUG << "Generate geode for layer " << (*itr).first << std::endl;
      DrawableToTagMapping tag_mapping;
      (*itr).second->GenerateGeode(*geode, _tags.size(), tag_mapping);

      // assign StateSet for each PTAG group
      for (unsigned int i = 0; i < geode->getNumDrawables(); i++)
        {
          OSG_DEBUG << "  Assigning surface " << _tags[tag_mapping[i]] << " to drawable " << i << std::endl;
          geode->getDrawable(i)->setStateSet(_surfaces[_tags[tag_mapping[i]]]->state_set);

          // copy material color to color array of geometry
          // because when lighting off color not applyed
          Geometry* geometry = geode->getDrawable(i)->asGeometry();
          if (geometry)
            {
              Material* material = dynamic_cast<Material*>(_surfaces[_tags[tag_mapping[i]]]->state_set->getAttribute(StateAttribute::MATERIAL));
              if (material) {
                Vec4Array* colors = new Vec4Array();
                colors->push_back(material->getDiffuse(Material::FRONT_AND_BACK));
                geometry->setColorArray(colors, osg::Array::BIND_OVERALL);
              }
            }
        }

      group.addChild(geode);
    }

  return true;
}

// generate StateSets for each surface
void
Lwo2::_generate_statesets_from_surfaces()
{
    ref_ptr<BlendFunc> blending = new BlendFunc();
    blending->setFunction(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ref_ptr<CullFace> culling = new CullFace();
    culling->setMode(CullFace::BACK);

    for (IteratorSurfaces itr_surf = _surfaces.begin(); itr_surf != _surfaces.end(); itr_surf++)
    {
        Lwo2Surface* surface = (*itr_surf).second;
        StateSet* state_set = new osg::StateSet;
        bool use_blending = false;

        OSG_DEBUG << "\tcreating surface " << (*itr_surf).first << std::endl;

        // check if exist texture image for this surface
        if (surface->image_index >= 0)
        {
            osg::ref_ptr<Image> image = osgDB::readRefImageFile(_images[surface->image_index]);
            OSG_DEBUG << "\tloaded image '" << _images[surface->image_index] << "'" << std::endl;
            OSG_DEBUG << "\tresult - " << image << std::endl;
            if (image.valid())
            {
                // create texture
                Texture2D* texture = new osg::Texture2D;
                texture->setImage(image.get());
                state_set->setTextureAttributeAndModes(0, texture, StateAttribute::ON);

                // setup texture wrapping
                texture->setWrap(Texture::WRAP_S, Texture::REPEAT);
                texture->setWrap(Texture::WRAP_T, Texture::REPEAT);

                // detect blending
                if (image->getPixelSizeInBits() == 32)
                {
                    for (int i = 0; i < image->s(); i++)
                    {
                        for (int j = 0; j < image->t(); j++)
                        {
                            unsigned char* data = image->data(i, j);
                            data++; // skip r
                            data++; // skip g
                            data++; // skip b

                            // check alpha
                            if (*data < 255)
                            {
                                use_blending = true;
                                break;
                            }
                        }
                        if (use_blending) break;
                    }
                }
            }
        }

        // set color
        Material* material = new Material();
        material->setDiffuse(Material::FRONT_AND_BACK, Vec4(surface->color, 1.0f));
        state_set->setAttribute(material);

        state_set->setMode(GL_NORMALIZE, StateAttribute::ON);

        if (use_blending)
        {
            // setup blending
            state_set->setAttribute(blending.get());
            state_set->setMode(GL_BLEND, StateAttribute::ON);

            // setup depth sorting
            state_set->setRenderingHint(StateSet::TRANSPARENT_BIN);
        }
        else
        {
            // setup culling
            state_set->setAttribute(culling.get());
            state_set->setMode(GL_CULL_FACE, StateAttribute::ON);
        }

        surface->state_set = state_set;
    }
}

// makes 4-byte integer tag from four chars
// used in IFF standard

unsigned long make_id(const char* tag)
{
    unsigned long result = 0;
    for (unsigned int i = 0; i < strlen(tag) && i < 4; i++)
    {
        result <<= 8;
        result += int(tag[i]);
    }
    return result;
}
