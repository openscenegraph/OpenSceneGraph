#include "ReaderWriterTXP.h"
#include "TrPageArchive.h"

#include <osg/Group>
#include <osg/Object>
#include <osg/Node>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/FileUtils>

#include <iostream>

using namespace txp;
using namespace osg;
//----------------------------------------------------------------------------
// private class for txp file 
class TXPFile
{
public:
    TrPageArchive archive;

    TXPFile() 
    {
    };

    ~TXPFile()
    {
    };

    Node* readNode(const std::string &filename)
    {
        Group* ret = 0; 
        // search the SGL data path
        std::string foundname =  osgDB::findFile(filename.c_str());
        if( !foundname.empty())
        {
            if (archive.OpenFile(foundname.c_str()))
            {
                notify(INFO) << "TXPFile::loadFile(): loading archive: "
                    << foundname << std::endl;
                archive.LoadMaterials();
                archive.LoadModels();
                
                notify(INFO) << "TXPFile::loadFile(): loading geometry"
                    << std::endl;

                ret = archive.LoadAllTiles();
                
                notify(INFO) << "TXPFile::loadFile(): loaded archive: "
                    << foundname << std::endl;
                //sgluOutputTree(sceneGraph, cout, 3);
            }
            else
            {
                notify(WARN) << "Failed to load archive: " << foundname << std::endl;
            }
        }
        else
        {
            notify(WARN) <<"sglTrPageGroup::loadFile() failed to find archive: "
                << foundname << std::endl;
        }
        return ret;
    };

    Object* readObject(const std::string &filename)
    {
        return readNode(filename);
    };
};

osgDB::ReaderWriter::ReadResult ReaderWriterTXP::readObject(const std::string& fileName, const osgDB::ReaderWriter::Options*)
{
    if( !acceptsExtension(osgDB::getFileExtension(fileName) ))
        return ReadResult::FILE_NOT_HANDLED;

    TXPFile read;

    Object* obj = read.readObject(fileName);
    if (obj) return obj;
    else return ReadResult::FILE_NOT_HANDLED;
}


osgDB::ReaderWriter::ReadResult ReaderWriterTXP::readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*)
{
    if( !acceptsExtension(osgDB::getFileExtension(fileName) ))
        return ReadResult::FILE_NOT_HANDLED;

    TXPFile read;
    Node* node = read.readNode(fileName);
    if (node) return node;
    else return ReadResult::FILE_NOT_HANDLED;
}

osgDB::RegisterReaderWriterProxy<ReaderWriterTXP> g_txpReaderWriterProxy;

