/*
 * PLY  ( Stanford Triangle Format )  File Loader for OSG
 * Copyright (C) 2009 by VizExperts Limited
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
        supportsExtension("ply","Stanford Triangle Format");
    }

    virtual const char* className() { return "ReaderWriterPLY"; }
    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*) const;
protected:
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
    ply::VertexData vertexData;
    osg::Node* node = vertexData.readPlyFile(fileName.c_str());

    if (node)
        return node;

    return ReadResult::FILE_NOT_HANDLED;
}
