// ReaderWriterFLT.cpp

#ifdef WIN32
#pragma warning( disable : 4786 )
#endif

#include "ReaderWriterFLT.h"
#include "FltFile.h"

#include <osg/Object>
#include <osg/Node>

#include <osgDB/Registry>

using namespace flt;

osgDB::ReaderWriter::ReadResult ReaderWriterFLT::readObject(const std::string& fileName, const osgDB::ReaderWriter::Options*)
{
    FltFile read;

    osg::Object* obj=read.readObject(fileName);
    if (obj) return obj;
    else return ReadResult::FILE_NOT_HANDLED;
}


osgDB::ReaderWriter::ReadResult ReaderWriterFLT::readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*)
{
    FltFile read;

    osg::Node* obj=read.readNode(fileName);
    if (obj) return obj;
    else return ReadResult::FILE_NOT_HANDLED;
}


// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterFLT> g_fltReaderWriterProxy;
