#include <osg/Group>
#include <osg/Object>
#include <osg/Node>
#include <osg/Notify>
#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <iostream>

#include "ReaderWriterTXP.h"
#include "TXPNode.h"
#include "TXPTileNode.h"
#include "TXPArchive.h"

#define ReaderWriterTXPERROR(s) osg::notify(osg::NOTICE) << "txp::ReaderWriterTXP::" << (s) << " error: "

using namespace txp;

int ReaderWriterTXP::_archiveId = 0;

osgDB::ReaderWriter::ReadResult ReaderWriterTXP::readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options)
{
    if( !acceptsExtension(osgDB::getFileExtension(fileName) ))
        return ReadResult::FILE_NOT_HANDLED;

    std::string name = osgDB::getSimpleFileName(fileName);

    // We load archive.txp
    if (strncmp(name.c_str(),"archive",7)==0)
    {
        osg::ref_ptr<TXPNode> txpNode = new TXPNode;
        txpNode->setArchiveName(fileName);
        if (options) 
        {
            txpNode->setOptions(options->getOptionString());
        }
        
        if (txpNode->loadArchive())
        {
            TXPArchive* archive = txpNode->getArchive();
            if (archive) 
            {
                int id = _archiveId++;
                archive->setId(id);
                getArchive(id,osgDB::getFilePath(fileName));
            }
            return txpNode.get();
        }
        else
        {
            return ReadResult::ERROR_IN_READING_FILE;
        }
    }

    // We load tileLOD_XxY_ID.txp
    if (strncmp(name.c_str(),"tile",4)==0)
    {
        int x,y,lod;
        unsigned int id;
        sscanf(name.c_str(),"tile%d_%dx%d_%d",&lod,&x,&y,&id);
        TXPArchive* archive = getArchive(id,osgDB::getFilePath(fileName));

        osg::ref_ptr<TXPTileNode> txpTileNode = new TXPTileNode;
        txpTileNode->setArchive(archive);
        
        if (txpTileNode->loadTile(x,y,lod))
        {
            //osg::notify(osg::NOTICE) << "Tile " << x << " " << y << " " << lod << " lodaded" << std::endl;

            return txpTileNode.get();
        }
        else
            return ReadResult::ERROR_IN_READING_FILE;
    }

    // We load subtilesLOD_XxY_ID.txp
    if (strncmp(name.c_str(),"sub",3)==0)
    {
        int x,y,lod;
        unsigned int id;
        sscanf(name.c_str(),"subtiles%d_%dx%d_%d",&lod,&x,&y,&id);
        TXPArchive* archive = getArchive(id,osgDB::getFilePath(fileName));

        osg::ref_ptr<osg::Group> subtiles = new osg::Group;

        for (int ix = 0; ix < 2; ix++)
        for (int iy = 0; iy < 2; iy++)
        {
            int tileX = x*2+ix;
            int tileY = y*2+iy;
            int tileLOD = lod+1;

            TXPTileNode* txpTileNode = new TXPTileNode;
            txpTileNode->setArchive(archive);
            
            if (txpTileNode->loadTile(tileX,tileY,tileLOD))
            {
                subtiles->addChild(txpTileNode);
            }

        }

        //osg::notify(osg::NOTICE) << "Subtiles for " << x << " " << y << " " << lod << " lodaded" << std::endl;

        return subtiles.get();
    }
    
    return ReadResult::ERROR_IN_READING_FILE;
}

TXPArchive *ReaderWriterTXP::getArchive(int id, const std::string& dir)
{
    TXPArchive* archive = NULL;

    std::map< int,osg::ref_ptr<TXPArchive> >::iterator iter = _archives.find(id);
    

    if (iter != _archives.end())
    {
        archive = iter->second.get();
    }

    if (archive == NULL)
    {
#ifdef _WIN32
                const char _PATHD = '\\';
#elif defined(macintosh)
                const char _PATHD = ':';
#else
                const char _PATHD = '/';
#endif
        std::string archiveName = dir+_PATHD+"archive.txp";
        archive = new TXPArchive;
        if (archive->openFile(archiveName) == false)
        {
            ReaderWriterTXPERROR("getArchive()") << "failed to load archive: \"" << archiveName << "\"" << std::endl;
            return NULL;
        }

        if (archive->loadMaterials() == false)
        {
            ReaderWriterTXPERROR("getArchive()") << "failed to load materials from archive: \"" << archiveName << "\"" << std::endl;
            return NULL;
        }

        /*
        // We load the models on demand
        if (archive->loadModels() == false)
        {
            ReaderWriterTXPERROR("getArchive()") << "failed to load models from archive: \"" << archiveName << "\"" << std::endl;
            return NULL;
        }
        */

        if (archive->loadLightAttributes() == false)
        {
            ReaderWriterTXPERROR("getArchive()") << "failed to load light attributes from archive: \"" << archiveName << "\"" << std::endl;
            return NULL;
        }

        archive->setId(id);

        _archives[id] = archive;
    }

    return archive;
}

osgDB::RegisterReaderWriterProxy<ReaderWriterTXP> g_txpReaderWriterProxy;

