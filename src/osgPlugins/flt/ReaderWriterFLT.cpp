// ReaderWriterFLT.cpp

#if defined(WIN32) && !defined(__CYGWIN__)
#pragma warning( disable : 4786 )
#endif

#include "ReaderWriterFLT.h"
#include "FltFile.h"

#include <osg/Object>
#include <osg/Node>

#include <osgDB/Registry>

using namespace flt;

osgDB::ReaderWriter::ReadResult ReaderWriterFLT::readObject(const std::string& fileName, const osgDB::ReaderWriter::Options* opt)
{
    return readNode(fileName,opt);
}


osgDB::ReaderWriter::ReadResult ReaderWriterFLT::readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*)
{
    if( !acceptsExtension(osgDB::getFileExtension(fileName) ))
    return ReadResult::FILE_NOT_HANDLED;

    osg::ref_ptr<FltFile> read = new FltFile;

    osg::Node* node = read.get()->readNode(fileName);
    if (node) return node;
    else return ReadResult::FILE_NOT_HANDLED;
}


// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterFLT> g_fltReaderWriterProxy;
