#include "ReaderWriterTXP.h"
#include <TrPageArchive.h>

#include <osg/Group>
#include <osg/Object>
#include <osg/Node>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/FileUtils>

#include <iostream>

#include "TerrapageNode.h"

using namespace txp;
using namespace osg;


osgDB::ReaderWriter::ReadResult ReaderWriterTXP::readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options)
{
    if( !acceptsExtension(osgDB::getFileExtension(fileName) ))
        return ReadResult::FILE_NOT_HANDLED;

    ref_ptr<TerrapageNode> pager = new TerrapageNode;
    
    pager->setDatabaseName(fileName);
    
    if (options)
    {
        pager->setDatabaseOptions(options->getOptionString());
    }
    
    if (pager->loadDatabase())
    {
        return pager.get();
    }
    else
        return ReadResult::ERROR_IN_READING_FILE;
    
}

osgDB::RegisterReaderWriterProxy<ReaderWriterTXP> g_txpReaderWriterProxy;

