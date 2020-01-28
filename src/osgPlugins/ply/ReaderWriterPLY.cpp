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

class ReaderWriterPLY : public osgDB::ReaderWriter
{
public:
    ReaderWriterPLY()
    {
        supportsExtension("ply","Stanford Meta Format");

        /** Note semantics from Equalizer LGPL source.*/
        //assuming compact aliasing (only name and internal format is meaningfull
/// element properties to read and their mapping
        _semantic.push_back({ "x", PLY_FLOAT, 0});
        _semantic.push_back({ "y", PLY_FLOAT, 0});
        _semantic.push_back({ "z", PLY_FLOAT, 0});
        _semantic.push_back({ "nx", PLY_FLOAT, 1});
        _semantic.push_back({ "ny", PLY_FLOAT, 1});
        _semantic.push_back({ "nz", PLY_FLOAT, 1});
        _semantic.push_back({ "red", PLY_FLOAT, 2});
        _semantic.push_back({ "green", PLY_FLOAT, 2});
        _semantic.push_back({ "blue", PLY_FLOAT, 2});
        _semantic.push_back({ "alpha", PLY_FLOAT, 2});
        _semantic.push_back({ "u", PLY_FLOAT, 3});
        _semantic.push_back({ "v", PLY_FLOAT, 3});
        _semantic.push_back({ "texture_u", PLY_FLOAT, 3});
        _semantic.push_back({ "texture_v", PLY_FLOAT, 3});
        _semantic.push_back({ "ambient_red", PLY_FLOAT, 4});
        _semantic.push_back({ "ambient_green", PLY_FLOAT, 4});
        _semantic.push_back({ "ambient_blue", PLY_FLOAT, 4});
        _semantic.push_back({ "diffuse_red", PLY_FLOAT, 5});
        _semantic.push_back({ "diffuse_green", PLY_FLOAT, 5});
        _semantic.push_back({ "diffuse_blue", PLY_FLOAT, 5});
        _semantic.push_back({ "specular_red", PLY_FLOAT, 6});
        _semantic.push_back({ "specular_green", PLY_FLOAT, 6});
        _semantic.push_back({ "specular_blue", PLY_FLOAT, 6});
        _semantic.push_back({ "specular_coeff", PLY_FLOAT, 7});
        _semantic.push_back({ "specular_power", PLY_FLOAT, 7});
        _semantic.push_back({ "tx", PLY_UCHAR, 8});
        _semantic.push_back({ "tn", PLY_UINT, 9});
/// list properties to read (first found is set as primitiveset)
        _semantic.push_back({ "vertex_indices", -1, -2}); //autotyped uint
        _semantic.push_back({ "vertex_index", -1, -2}); //autotyped uint
        _semantic.push_back({ "texture_vertex_indices", -1, -1}); //autotyped uint
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
