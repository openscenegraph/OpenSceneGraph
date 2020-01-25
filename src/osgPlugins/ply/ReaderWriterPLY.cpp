/*
 * PLY  ( Stanford Triangle Format )  File Loader for OSG
 * Copyright (C) 2009 by VizExperts Limited
 * Copyright (C) 2020 Julien Valentin (mp3butcher@hotmail.com)
 * All rights reserved.
 *
 * This program is  free  software;  you can redistribute it and/or modify it
 * under the terms of the  GNU Lesser General Public License  as published by
 * the  Free Software Foundation;  either version 2.1 of the License,  or (at
 * your option) any later version.
 *
 * This  program  is  distributed in  the  hope that it will  be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or  FITNESS FOR A  PARTICULAR PURPOSE.  See the  GNU Lesser General Public
 * License for more details.
 *
 * You should  have received  a copy of the GNU Lesser General Public License
 * along with  this program;  if not, write to the  Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>


#include "vertexData.h"

using namespace osg;
using namespace osgDB;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
//!
//! \class ReaderWriterPLY
//! \brief This is the Reader for the ply file format
//!
//////////////////////////////////////////////////////////////////////////////
///
/** Note semantics from Equalizer LGPL source.*/
struct _Vertex
{
    float           x;
    float           y;
    float           z;
    float           nx;
    float           ny;
    float           nz;
    unsigned char   red;
    unsigned char   green;
    unsigned char   blue;
    unsigned char   alpha;
    float   ambient_red;
   float   ambient_green;
    float   ambient_blue;
    unsigned char   diffuse_red;
    unsigned char   diffuse_green;
    unsigned char   diffuse_blue;
    unsigned char   specular_red;
    unsigned char   specular_green;
    unsigned char   specular_blue;
    float           specular_coeff;
    float           specular_power;
    float texture_u;
    float texture_v;
} vertex;

class ReaderWriterPLY : public osgDB::ReaderWriter
{
public:
    ReaderWriterPLY()
    {
        supportsExtension("ply","Stanford Triangle Meta Format");
        _semantic.push_back(ply::VertexSemantic({ "x", PLY_FLOAT, PLY_FLOAT, offsetof( _Vertex, x ), 0, 0, 0, 0 },0));
        _semantic.push_back(ply::VertexSemantic({ "y", PLY_FLOAT, PLY_FLOAT, offsetof( _Vertex, y ), 0, 0, 0, 0 },0));
        _semantic.push_back(ply::VertexSemantic({ "z", PLY_FLOAT, PLY_FLOAT, offsetof( _Vertex, z ), 0, 0, 0, 0 },0));
        _semantic.push_back(ply::VertexSemantic({ "nx", PLY_FLOAT, PLY_FLOAT, offsetof( _Vertex, nx ), 0, 0, 0, 0 },1));
        _semantic.push_back(ply::VertexSemantic({ "ny", PLY_FLOAT, PLY_FLOAT, offsetof(_Vertex, ny), 0, 0, 0, 0 },1));
        _semantic.push_back(ply::VertexSemantic({ "nz", PLY_FLOAT, PLY_FLOAT, offsetof(_Vertex, nz), 0, 0, 0, 0 },1));
        _semantic.push_back(ply::VertexSemantic({ "red", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, red ), 0, 0, 0, 0 },2));
        _semantic.push_back(ply::VertexSemantic({ "green", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, green ), 0, 0, 0, 0 },2));
        _semantic.push_back(ply::VertexSemantic({ "blue", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, blue ), 0, 0, 0, 0 },2));
        _semantic.push_back(ply::VertexSemantic({ "alpha", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, alpha ), 0, 0, 0, 0 },2));
        _semantic.push_back(ply::VertexSemantic({ "ambient_red", PLY_UCHAR, PLY_FLOAT, offsetof( _Vertex, ambient_red ), 0, 0, 0, 0 },3));
        _semantic.push_back(ply::VertexSemantic({ "ambient_green", PLY_UCHAR, PLY_FLOAT, offsetof( _Vertex, ambient_green ), 0, 0, 0, 0 },3));
        _semantic.push_back(ply::VertexSemantic({ "ambient_blue", PLY_UCHAR, PLY_FLOAT, offsetof( _Vertex, ambient_blue ), 0, 0, 0, 0 },3));
        _semantic.push_back(ply::VertexSemantic({ "diffuse_red", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, diffuse_red ), 0, 0, 0, 0 },4));
        _semantic.push_back(ply::VertexSemantic({ "diffuse_green", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, diffuse_green ), 0, 0, 0, 0 },4));
        _semantic.push_back(ply::VertexSemantic({ "diffuse_blue", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, diffuse_blue ), 0, 0, 0, 0 },4));
        _semantic.push_back(ply::VertexSemantic({ "specular_red", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, specular_red ), 0, 0, 0, 0 },5));
        _semantic.push_back(ply::VertexSemantic({ "specular_green", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, specular_green ), 0, 0, 0, 0 },5));
        _semantic.push_back(ply::VertexSemantic({ "specular_blue", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, specular_blue ), 0, 0, 0, 0 },5));
        _semantic.push_back(ply::VertexSemantic({ "specular_coeff", PLY_FLOAT, PLY_FLOAT, offsetof( _Vertex, specular_coeff ), 0, 0, 0, 0 },6));
        _semantic.push_back(ply::VertexSemantic({ "specular_power", PLY_FLOAT, PLY_FLOAT, offsetof( _Vertex, specular_power ), 0, 0, 0, 0 },6));
        _semantic.push_back(ply::VertexSemantic({ "texture_u", PLY_FLOAT, PLY_FLOAT, offsetof(_Vertex, texture_u), 0, 0, 0, 0 },7));
        _semantic.push_back(ply::VertexSemantic({ "texture_v", PLY_FLOAT, PLY_FLOAT, offsetof(_Vertex, texture_v), 0, 0, 0, 0 },7));
    }

    virtual const char* className() const { return "ReaderWriterPLY"; }

    void setVertexSemantics(const ply::VertexSemantics &meta) { _semantic = meta; }
    ply::VertexSemantics& getVertexSemantics() { return _semantic;}
    
    virtual ReadResult readObject(const std::string& filename, const osgDB::ReaderWriter::Options* options) const
    {
        return readNode(filename, options);
    }

    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*) const;
protected:
    ply::VertexSemantics _semantic;
};

// register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN(ply, ReaderWriterPLY)

///////////////////////////////////////////////////////////////////////////////
//!
//! \brief Function which is called when any ply file is requested to load in
//! \osgDB. Load read ply file and if it successes return the osg::Node
//!
///////////////////////////////////////////////////////////////////////////////
osgDB::ReaderWriter::ReadResult ReaderWriterPLY::readNode(const std::string& filename, const osgDB::ReaderWriter::Options* options) const
{
    // Get the file extension
    std::string ext = osgDB::getFileExtension(filename);

    // If the file extension does not support then return that file is not handled
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    // Check whether or not file exist or not
    std::string fileName = osgDB::findDataFile(filename, options);
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

    //Instance of vertex data which will read the ply file and convert in to osg::Node
    ply::VertexData vertexData(_semantic);
    osg::Node* node = vertexData.readPlyFile(fileName.c_str());

    if (node)
        return node;

    return ReadResult::FILE_NOT_HANDLED;
}
