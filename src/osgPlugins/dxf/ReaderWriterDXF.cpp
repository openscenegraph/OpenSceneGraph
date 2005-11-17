/* dxfReader for OpenSceneGraph  Copyright (C) 2005 by GraphArchitecture ( grapharchitecture.com )
 * Programmed by Paul de Repentigny <pdr@grapharchitecture.com>
 * 
 * OpenSceneGraph is (C) 2004 Robert Osfield
 * 
 * This library is provided as-is, without support of any kind.
 *
 * Read DXF docs or OSG docs for any related questions.
 * 
 * You may contact the author if you have suggestions/corrections/enhancements.
 */
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <map>
#include <iostream>
#include <utility>
#include <string>
#include <sstream>

#include "dxfFile.h"

using namespace osg;
using namespace osgDB;
using namespace std;


class ReaderWriterdxf : public osgDB::ReaderWriter
{
public:
    ReaderWriterdxf() { }
    virtual const char* className() { return "Autodesk DXF Reader"; }
    virtual bool acceptsExtension(const std::string& extension) const {
        return osgDB::equalCaseInsensitive(extension,"dxf");
    }
    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*) const;
protected:
};

// register with Registry to instantiate the above reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterdxf> g_dxfReaderWriterProxy;


// read file and convert to OSG.
osgDB::ReaderWriter::ReadResult 
ReaderWriterdxf::readNode(const std::string& filename, const osgDB::ReaderWriter::Options*) const
{
    std::string ext = osgDB::getFileExtension(filename);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;
    // Open
    dxfFile df(filename);
    if (df.parseFile()) {
        // convert to OSG
        osg::Group* osg_top = df.dxf2osg();
        return (osg_top);
    }
    return ReadResult::FILE_NOT_HANDLED;
}

