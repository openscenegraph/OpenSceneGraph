// ReaderWriterFLT.cpp

#ifdef WIN32
#pragma warning( disable : 4786 )
#endif

#include "ReaderWriterFLT.h"
#include "FltFile.h"

#include <osg/Registry>
#include <osg/Object>
#include <osg/Node>


using namespace flt;


osg::Object* ReaderWriterFLT::readObject(const std::string& fileName)
{
    FltFile read;

    return read.readObject(fileName);
}


osg::Node* ReaderWriterFLT::readNode(const std::string& fileName)
{
    FltFile read;

    return read.readNode(fileName);
}


// now register with Registry to instantiate the above
// reader/writer.
osg::RegisterReaderWriterProxy<ReaderWriterFLT> g_fltReaderWriterProxy;





