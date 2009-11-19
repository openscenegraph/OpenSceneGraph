#ifndef READERWRITERFBX_H
#define READERWRITERFBX_H

#include <OpenThreads/ReentrantMutex>
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
    }

    const char* className() const { return "FBX reader/writer"; }

    /// The FBX SDK interprets the filename as UTF-8
    ReadResult readNode(const std::string& utf8filename, const Options*) const;
  
private:
    mutable OpenThreads::ReentrantMutex _serializerMutex;
};

///////////////////////////////////////////////////////////////////////////

#endif
