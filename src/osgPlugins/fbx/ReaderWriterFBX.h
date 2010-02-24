#ifndef READERWRITERFBX_H
#define READERWRITERFBX_H

#include <osgDB/ReaderWriter>

///////////////////////////////////////////////////////////////////////////
// OSG reader plugin for the ".fbx" format.
// See http://usa.autodesk.com/adsk/servlet/index?siteID=123112&id=6837478

class ReaderWriterFBX : public osgDB::ReaderWriter
{
public:
    ReaderWriterFBX()
    {
        supportsExtension("fbx", "FBX format");
        supportsOption("Embedded", "Embed textures in FBX file when writing");
        supportsOption("UseFbxRoot", "(Read/write option) If the source OSG root node is a simple group with no stateset, the writer will put its children directly under the FBX root, and vice-versa for reading");
    }

    const char* className() const { return "FBX reader/writer"; }

    virtual ReadResult readNode(const std::string& filename, const Options*) const;
    virtual WriteResult writeNode(const osg::Node&, const std::string& filename, const Options*) const;
};

///////////////////////////////////////////////////////////////////////////

#endif
