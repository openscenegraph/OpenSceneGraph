#include <osg/Group>
#include <osg/Object>
#include <osg/Node>
#include <osg/Notify>
#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <iostream>

#include "ReaderWriterTXP.h"
#include "TXPNode.h"
#include "TXPArchive.h"
#include "TXPPagedLOD.h"
#include "TXPSeamLOD.h"
#include "TileMapper.h"

#define ReaderWriterTXPERROR(s) osg::notify(osg::NOTICE) << "txp::ReaderWriterTXP::" << (s) << " error: "

using namespace txp;

int ReaderWriterTXP::_archiveId = 0;

osgDB::ReaderWriter::ReadResult ReaderWriterTXP::local_readNode(const std::string& file, const osgDB::ReaderWriter::Options* options)
{

    std::string name = osgDB::getSimpleFileName(file);

    // We load archive.txp
    if (strncmp(name.c_str(),"archive",7)==0)
    {
        std::string fileName = osgDB::findDataFile( file, options );
        if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

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
        TXPArchive* archive = getArchive(id,osgDB::getFilePath(file));

        TXPArchive::TileInfo info;
        if (!archive->getTileInfo(x,y,lod,info))
            return ReadResult::ERROR_IN_READING_FILE;

        osg::ref_ptr<osg::Node> tileContent = getTileContent(info,x,y,lod,archive);
        
        tileContent->setName("TileContent");

        int numLods = archive->getNumLODs();
        if (lod < (numLods-1))
        {
            char pagedLODfile[1024];
            sprintf(pagedLODfile,"%s\\subtiles%d_%dx%d_%d.txp",
                archive->getDir(),
                lod,
                x,
                y,
                archive->getId()
            );

            osg::ref_ptr<TXPPagedLOD> pagedLOD = new TXPPagedLOD;
            // not use maximum(info.maxRange,1e7) as just maxRange would result in some corner tiles from being culled out.
            pagedLOD->addChild(tileContent.get(),info.minRange,osg::maximum(info.maxRange,1e7));
            pagedLOD->setFileName(1,pagedLODfile);
            pagedLOD->setRange(1,0,info.minRange);
            pagedLOD->setCenter(info.center);
            pagedLOD->setRadius(info.radius);
            pagedLOD->setPriorityOffset(0,numLods-lod);
            pagedLOD->setPriorityScale(0,1.0f);
            pagedLOD->setNumChildrenThatCannotBeExpired(1);
            pagedLOD->setTileId(x,y,lod);

            return pagedLOD.get();
        }
        else
            return tileContent.get();
    }

    // We load subtilesLOD_XxY_ID.txp
    if (strncmp(name.c_str(),"sub",3)==0)
    {
        int x,y,lod;
        unsigned int id;
        sscanf(name.c_str(),"subtiles%d_%dx%d_%d",&lod,&x,&y,&id);
        TXPArchive* archive = getArchive(id,osgDB::getFilePath(file));

        osg::ref_ptr<osg::Group> subtiles = new osg::Group;

        int numLods = archive->getNumLODs();
        int sizeX, sizeY;
        archive->getLODSize(lod+1,sizeX,sizeY);

        for (int ix = 0; ix < 2; ix++)
        for (int iy = 0; iy < 2; iy++)
        {
            int tileX = x*2+ix;
            int tileY = y*2+iy;
            int tileLOD = lod+1;

            TXPArchive::TileInfo info;
            if (!archive->getTileInfo(tileX,tileY,tileLOD,info))
                continue;

            osg::ref_ptr<osg::Node> tileContent = getTileContent(info,tileX,tileY,tileLOD,archive);

            tileContent->setName("TileContent");

            if (tileLOD < (numLods-1))
            {
                char pagedLODfile[1024];
                sprintf(pagedLODfile,"%s\\subtiles%d_%dx%d_%d.txp",
                    archive->getDir(),
                    tileLOD,
                    tileX,
                    tileY,
                    archive->getId()
                );

                osg::ref_ptr<TXPPagedLOD> pagedLOD = new TXPPagedLOD;
                // not use maximum(info.maxRange,1e7) as just maxRange would result in some corner tiles from being culled out.
                pagedLOD->addChild(tileContent.get(),info.minRange,osg::maximum(info.maxRange,1e7));
                pagedLOD->setFileName(1,pagedLODfile);
                pagedLOD->setRange(1,0,info.minRange);
                pagedLOD->setCenter(info.center);
                pagedLOD->setRadius(info.radius);
                pagedLOD->setPriorityOffset(0,numLods-lod);
                pagedLOD->setPriorityScale(0,1.0f);
                pagedLOD->setNumChildrenThatCannotBeExpired(1);
                pagedLOD->setTileId(tileX,tileY,tileLOD);

                subtiles->addChild(pagedLOD.get());
            }
            else
            {
                subtiles->setUserData(new TileIdentifier(tileX,tileY,tileLOD));
                subtiles->addChild(tileContent.get());
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

        if (archive->loadModels() == false)
        {
            ReaderWriterTXPERROR("getArchive()") << "failed to load models from archive: \"" << archiveName << "\"" << std::endl;
            return NULL;
        }

        if (archive->loadLightAttributes() == false)
        {
            ReaderWriterTXPERROR("getArchive()") << "failed to load light attributes from archive: \"" << archiveName << "\"" << std::endl;
            return NULL;
        }

		if (archive->loadTextStyles() == false)
		{
			ReaderWriterTXPERROR("getArchive()") << "failed to load text styles from archive: \"" << archiveName << "\"" << std::endl;
			return NULL;
		}

        archive->setId(id);

        _archives[id] = archive;
    }

    return archive;
}

class SeamFinder: public osg::NodeVisitor
{
public:
    SeamFinder(int x, int y, int lod, TXPArchive::TileInfo& info, TXPArchive *archive ):
    osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
    _x(x), _y(y), _lod(lod), _info(info), _archive(archive) 
    {};

    virtual void apply(osg::Group& group)
    {
        for (unsigned int i = 0; i < group.getNumChildren(); i++)
        {
            osg::Node* child = group.getChild(i);
            osg::Node* seam = seamReplacement(child);
            if (child != seam)
            {
                group.replaceChild(child,seam);
            }
            else
            {
                child->accept(*this);
            }
        }
    }

protected:
    osg::Node* seamReplacement(osg::Node* node);

    int _x, _y, _lod;
    TXPArchive::TileInfo& _info;
    TXPArchive *_archive;
};

osg::Node* SeamFinder::seamReplacement(osg::Node* node)
{
    osg::Group* group = node->asGroup();
    if (group == 0) return node;

    std::vector<osg::Node*> nonSeamChildren;
    osg::LOD* hiRes = 0;
    osg::LOD* loRes = 0;

    for (unsigned int i = 0; i < group->getNumChildren(); i++)
    {
        osg::LOD* lod = dynamic_cast<osg::LOD*>(group->getChild(i));
        if (lod == 0)
        {
            nonSeamChildren.push_back(group->getChild(i));
            continue;
        }

        bool nonSeamChild = true;

        // seam center is outside the bounding box of the tile
        if (!_info.bbox.contains(lod->getCenter()))
        {
            // seams have center as the neighbour tile
            osg::Vec3 d = _info.center - lod->getCenter();
            if (((fabs(d.x())-_info.size.x()) > 0.0001) && ((fabs(d.y())-_info.size.y()) > 0.0001))
            {
                nonSeamChildren.push_back(lod);
                continue;
            }

            // low res seam has min/max ranges of lod+1 range/lod 0 range
            if ((fabs(_info.minRange-lod->getMinRange(0))<0.001)&&(fabs(_info.lod0Range-lod->getMaxRange(0))<0.001))
            {

                if (loRes==0)
                {
                    loRes = lod;
                    nonSeamChild = false;
                }
            }

            // hi res seam has min/max ranges of 0 range/lod+1 range
            if ((lod->getMinRange(0)==0.0f)&&(fabs(_info.minRange-lod->getMaxRange(0))<0.001))
            {
                if (hiRes==0)
                {
                    hiRes = lod;
                    nonSeamChild = false;
                }
            }
        }
        if (nonSeamChild)
        {
            nonSeamChildren.push_back(lod);
        }
    }

    if (loRes)
    {
        int dx = 0;
        int dy = 0;
        int lod = _lod;
        osg::Vec3 delta = loRes->getCenter()-_info.center;
        if (fabs(delta.x())>fabs(delta.y()))
        {
            if (delta.x()<0.0) --dx;    // west
            else dx++;                  // east
        }
        else
        {
            if (delta.y()<0.0) --dy;    // south
            else ++dy;                  // north
        }
        TXPSeamLOD* seam = new TXPSeamLOD(
            _x,
            _y,
            lod,
            dx,
            dy
        );
        seam->addChild(loRes->getChild(0));                // low res
        if (hiRes) 
        {
            seam->addChild(hiRes->getChild(0));    // high res
        }
  
        if (nonSeamChildren.empty())
        {
            return seam;
        }
        else
        {
            osg::Group* newGroup = new osg::Group;

            newGroup->addChild(seam);
            
            for (unsigned int i = 0; i < nonSeamChildren.size(); i++)
                newGroup->addChild(nonSeamChildren[i]);
                
            return newGroup;
        }
    }

    return node;
}

osg::Node* ReaderWriterTXP::getTileContent(TXPArchive::TileInfo &info, int x, int y, int lod, TXPArchive* archive)
{
    if (archive == 0) return false;

    int numLods = archive->getNumLODs();

    double realMinRange = info.minRange;
    double realMaxRange = info.maxRange;
    double usedMaxRange = osg::maximum(info.maxRange,1e7);
    osg::Vec3 tileCenter;
    osg::Group* tileGroup = archive->getTileContent(x,y,lod,realMinRange,realMaxRange,usedMaxRange,tileCenter);

    // if group has only one child, then simply use its child.    
#if 1
    while (tileGroup->getNumChildren()==1 && tileGroup->getChild(0)->asGroup())
    {
        tileGroup = tileGroup->getChild(0)->asGroup();
    }
#endif
        
    // Handle seams
    if (lod < (numLods-1))
    {
        SeamFinder sfv(x,y,lod,info,archive);
        tileGroup->accept(sfv);
    }

    return tileGroup;
}

osgDB::RegisterReaderWriterProxy<ReaderWriterTXP> g_txpReaderWriterProxy;

