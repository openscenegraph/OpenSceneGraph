// ReaderWriterFLT.cpp

#if defined(_MSC_VER)
#pragma warning( disable : 4786 )
#endif

#include "ReaderWriterFLT.h"
#include "FltFile.h"
#include "Registry.h"

#include <osg/Object>
#include <osg/Node>
#include <osg/Notify>

#include <osgDB/Registry>

using namespace flt;

osgDB::ReaderWriter::ReadResult ReaderWriterFLT::readObject(const std::string& fileName, const osgDB::ReaderWriter::Options* opt)
{
    return readNode(fileName,opt);
}


osgDB::ReaderWriter::ReadResult ReaderWriterFLT::readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options)
{
    if( !acceptsExtension(osgDB::getFileExtension(fileName) ))
        return ReadResult::FILE_NOT_HANDLED;

    osg::ref_ptr<FltFile> read = new FltFile;

    if (options)
    {
        read->setUseTextureAlphaForTransparancyBinning(options->getOptionString().find("noTextureAlphaForTransparancyBinning")==std::string::npos);
        osg::notify(osg::DEBUG_INFO) << "FltFile.getUseTextureAlphaForTransparancyBinning()=" << read->getUseTextureAlphaForTransparancyBinning() << std::endl;
    }

    osg::Node* node = read->readNode(fileName);

    flt::Registry::instance()->clearObjectCache();

    if (node) return node;
    else return ReadResult::FILE_NOT_HANDLED;
}


// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterFLT> g_fltReaderWriterProxy;
