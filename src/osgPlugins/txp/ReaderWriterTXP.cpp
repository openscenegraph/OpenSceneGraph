#include <osg/Group>
#include <osg/Object>
#include <osg/Node>
#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/BoundingSphere>
#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <osg/io_utils>

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>

#include "ReaderWriterTXP.h"
#include "TXPNode.h"
#include "TXPArchive.h"
#include "TXPPagedLOD.h"
#include "TXPSeamLOD.h"
#include "TileMapper.h"

#define ReaderWriterTXPERROR(s) OSG_NOTICE << "txp::ReaderWriterTXP::" << (s) << " error: "



namespace
{
    char gbuf[2048];
}

using namespace txp;

int ReaderWriterTXP::_archiveId = 0;

osgDB::ReaderWriter::ReadResult ReaderWriterTXP::local_readNode(const std::string& file, const osgDB::ReaderWriter::Options* options)
{

    std::string name = osgDB::getSimpleFileName(file);

    // We load archive.txp
    if (strncmp(name.c_str(),"archive",7)==0)
    {
        std::string fileName = osgDB::findDataFile( file, options );
        if ( fileName.empty() )
            return ReadResult::FILE_NOT_FOUND;

        osg::ref_ptr<TXPNode> txpNode = new TXPNode;
        txpNode->setArchiveName(fileName);
        if (options)
        {
            txpNode->setOptions(options->getOptionString());
        }


        //modified by Brad Anderegg on May-27-08
        //calling getArchive will create a new TXPArchive if the specified one does not exist
        //we will set our osgdb loader options on the archive and set the appropriate archive on
        //the txpNode.
        int id = ++_archiveId;
        osg::ref_ptr< TXPArchive > archive = createArchive(id,osgDB::getFilePath(fileName));

        if (archive != NULL)
        {
            archive->setId(id);

            if (options && options->getOptionString().find("loadMaterialsToStateSet")!=std::string::npos)
            {
               archive->SetMaterialAttributesToStateSetVar(true);
            }

            txpNode->loadArchive(archive.get());

            return txpNode.get();
        }
        else
        {
            return ReadResult::ERROR_IN_READING_FILE;
        }
    }

    // We load tileLOD_XxY_ID.txp
    else if (strncmp(name.c_str(),"tile",4)==0)
    {
        int x,y,lod;
        unsigned int id;
        sscanf(name.c_str(),"tile%d_%dx%d_%u",&lod,&x,&y,&id);
        osg::ref_ptr< TXPArchive > archive = getArchive(id,osgDB::getFilePath(file));
        if (archive == NULL)
            return ReadResult::ERROR_IN_READING_FILE;

        // The way this is done a 'tile' should only be created for lod 0 only,
        // something is wrong if this is no the case
        if(lod != 0)
        {
            ReaderWriterTXPERROR("ReaderWriterTXP::local_readNode()") << "paged 'tile' should be at lod 0" << std::endl;
            return ReadResult::ERROR_IN_READING_FILE;
        }

        trpgEndian endian = archive->GetEndian();
        archive->ReadSubArchive( 0, 0, endian);
        archive->ReadSubArchive( y, x, endian);

//    std::cout << "Attempted " << x << " " << y << std::endl;

        TXPArchive::TileInfo info;
        if (!archive->getTileInfo(x,y,lod,info))
            return ReadResult::ERROR_IN_READING_FILE;

        std::vector<TXPArchive::TileLocationInfo> childrenLoc;
        osg::ref_ptr<osg::Node> tileContent = getTileContent(info,x,y,lod,archive.get(), childrenLoc);

        tileContent->setName("TileContent");

        bool asChildren = false;
        std::string childrenInfoStr;

        int numLods = archive->getNumLODs();

        int majorVersion, minorVersion;
        archive->GetVersion(majorVersion, minorVersion);
        if(majorVersion ==2 && minorVersion >=1)
        {
            // Version 2.1 and over
            // The tile table only contains lod 0 and the children
            // info are stored in its parent. SO if we do not want
            // to be forced to reparse the parent we need to save that
            // info. For now we just add it to the node name

            if(childrenLoc.size() > 0)
            {
                asChildren = true;
                createChildrenLocationString(childrenLoc, childrenInfoStr);
            }
        }
        else
        {
            if (lod < (numLods-1)) asChildren = true;
        }

        if (asChildren)
        {
            char pagedLODfile[1024];
            sprintf(pagedLODfile,"%s\\subtiles%d_%dx%d_%d",
            archive->getDir(),
            lod,
            x,
            y,
            archive->getId());

            strcat(pagedLODfile, childrenInfoStr.c_str());
            strcat(pagedLODfile, ".txp");


            // there are tile sets which do not maintain the z extents in
            // the tile table.  This attempt to address the issue by using
            // the geometry bounding sphere.  The downside is that this is
            // not coupled to the generation and may result in runtime cracks
            if (info.center.z() == 0)
            {
                osg::BoundingSphere bSphere = tileContent->getBound();

                info.center.z() = bSphere.center().z();
                info.radius = bSphere.radius();
            }


            osg::ref_ptr<TXPPagedLOD> pagedLOD = new TXPPagedLOD;
            // note: use maximum(info.maxRange,1e7) as just maxRange would result in some corner tiles from being culled out.
            pagedLOD->addChild(tileContent.get(),info.minRange,osg::maximum(info.maxRange,1e7));
            pagedLOD->setFileName(1,pagedLODfile);
            pagedLOD->setRange(1,0,info.minRange);
            pagedLOD->setCenter(info.center);
            pagedLOD->setRadius(info.radius);
            pagedLOD->setPriorityOffset(0,numLods-lod);
            pagedLOD->setPriorityScale(0,1.0f);
            pagedLOD->setNumChildrenThatCannotBeExpired(1);
            pagedLOD->setTileId(x,y,lod);

            const trpgHeader* header = archive->GetHeader();
            trpgHeader::trpgTileType tileType;
            header->GetTileOriginType(tileType);
            if(tileType == trpgHeader::TileLocal)
            {
                osg::Vec3d sw(info.bbox._min);
                pagedLOD->setCenter(info.center - sw);
            }

            return pagedLOD.get();
        }
        else
            return tileContent.get();
    }


    // For 2.0 and lower we load subtilesLOD_XxY_ID.txp
    // For 2.1 and over  we load subtilesLOD_XxY_ID_NBCHILD_{X_Y_FID_FOFFSET_ZMIN_ZMAX_X_Y_ADDR ....}.txp
    else if (strncmp(name.c_str(),"sub",3)==0)
    {
        int x,y,lod;
        unsigned int id;
        sscanf(name.c_str(),"subtiles%d_%dx%d_%u",&lod,&x,&y,&id);
        osg::ref_ptr< TXPArchive > archive = getArchive(id,osgDB::getFilePath(file));
        if (archive == NULL)
            return ReadResult::ERROR_IN_READING_FILE;

        int majorVersion, minorVersion;
        archive->GetVersion(majorVersion, minorVersion);

        std::vector<TXPArchive::TileLocationInfo> childrenLoc;

        osg::ref_ptr<osg::Group> subtiles = new osg::Group;

        int numLods = archive->getNumLODs();

        if(majorVersion == 2  && minorVersion >= 1)
        {
            int nbChild;

            sscanf(name.c_str(),"subtiles%d_%dx%d_%u_%d",&lod,&x,&y,&id, &nbChild);
            std::vector<TXPArchive::TileLocationInfo> locs;
            bool status = true;
            status = extractChildrenLocations(name, lod, locs, nbChild);
            if(majorVersion >= TRPG_NOMERGE_VERSION_MAJOR && minorVersion >=TRPG_NOMERGE_VERSION_MINOR && archive->GetHeader()->GetIsMaster())
            {
                for(int idx=0;idx<nbChild;idx++)
                {
                    //figure out the block row/col
                    int blockx,blocky;
                    unsigned int denom = (1 << locs[idx].lod); // this should work up to lod 31
                    blockx = locs[idx].x/denom;
                    blocky = locs[idx].y/denom;
                    locs[idx].addr.col = blockx;
                    locs[idx].addr.row = blocky;
                }
            }

            if(!status)
            {
                ReaderWriterTXPERROR("ReaderWriterTXP::local_readNode()") << "'subtile' filename children parsing failed " << std::endl;
                return ReadResult::ERROR_IN_READING_FILE;
            }

            const trpgHeader* header = archive->GetHeader();
            trpgHeader::trpgTileType tileType;
            header->GetTileOriginType(tileType);

            TXPArchive::TileLocationInfo plInfo;
            plInfo.x = x;
            plInfo.y = y;
            plInfo.lod = lod;
            TXPArchive::TileInfo parentInfo;
            archive->getTileInfo(plInfo,parentInfo);

            for(int idx = 0; idx < nbChild; ++idx)
            {
                std::vector<TXPArchive::TileLocationInfo> childrenChildLoc;

                TXPArchive::TileLocationInfo& loc = locs[idx];

                TXPArchive::TileInfo info;
                if (!archive->getTileInfo(loc,info))
                    continue;

                osg::ref_ptr<osg::Node> tileContent = getTileContent(info, loc, archive.get(), childrenChildLoc);

                tileContent->setName("TileContent");

                if(childrenChildLoc.size() > 0)
                {
                    std::string childInfoStr;
                    createChildrenLocationString(childrenChildLoc, childInfoStr);

                    char pagedLODfile[1024];
                    sprintf(pagedLODfile,"%s\\subtiles%d_%dx%d_%d%s.txp",
                        archive->getDir(),
                        loc.lod,
                        loc.x,
                        loc.y,
                        archive->getId(),
                        childInfoStr.c_str());

                    // there are tile sets which do not maintain the z extents in
                    // the tile table.  This attempt to address the issue by using
                    // the geometry bounding sphere.  The downside is that this is
                    // not coupled to the generation and may result in runtime cracks
                    if (info.center.z() == 0)
                    {
                        osg::BoundingSphere bSphere = tileContent->getBound();

                        info.center.z() = bSphere.center().z();
                        info.radius = bSphere.radius();
                    }

                    osg::ref_ptr<TXPPagedLOD> pagedLOD = new TXPPagedLOD;
                            // note: use maximum(info.maxRange,1e7) as just maxRange would result in some corner tiles from being culled out.
                    pagedLOD->addChild(tileContent.get(),info.minRange,osg::maximum(info.maxRange,1e7));
                    pagedLOD->setFileName(1,pagedLODfile);
                    pagedLOD->setRange(1,0,info.minRange);
                    pagedLOD->setCenter(info.center);
                    pagedLOD->setRadius(info.radius);
                    pagedLOD->setPriorityOffset(0,numLods - loc.lod);
                    pagedLOD->setPriorityScale(0,1.0f);
                    pagedLOD->setNumChildrenThatCannotBeExpired(1);
                    pagedLOD->setTileId(loc.x, loc.y, loc.lod);

                    if(tileType == trpgHeader::TileLocal)
                    {
                        osg::Vec3d center(info.center - parentInfo.bbox._min);
                        osg::Vec3d sw(info.bbox._min - parentInfo.bbox._min);
                        sw[2] = 0.0;
                        pagedLOD->setCenter(center - sw);
                        osg::Matrix offset;
                        offset.setTrans(sw);
                        osg::MatrixTransform *tform = new osg::MatrixTransform(offset);
                        tform->addChild(pagedLOD.get());
                        subtiles->addChild(tform);
                    }
                    else
                        subtiles->addChild(pagedLOD.get());
                        subtiles->setUserData(new TileIdentifier(loc.x, loc.y, loc.lod)); // is this really needed?
                }
                else
                {
                    subtiles->setUserData(new TileIdentifier(loc.x, loc.y, loc.lod));
                    if(tileType == trpgHeader::TileLocal)
                    {
                        osg::Vec3d center(info.center - parentInfo.bbox._min);
                        osg::Vec3d sw(info.bbox._min - parentInfo.bbox._min);
                        sw[2] = 0.0;
                        osg::Matrix offset;
                        offset.setTrans(sw);
                        osg::MatrixTransform *tform = new osg::MatrixTransform(offset);
                        tform->addChild(tileContent.get());
                        subtiles->addChild(tform);
                    }
                    else
                        subtiles->addChild(tileContent.get());
                }
            }
        }
        else
        {

            int sizeX, sizeY;
            archive->getLODSize(lod+1,sizeX,sizeY);

            const trpgHeader* header = archive->GetHeader();
            trpgHeader::trpgTileType tileType;
            header->GetTileOriginType(tileType);

            TXPArchive::TileInfo parentInfo;
            archive->getTileInfo(x,y,lod,parentInfo);

            for (int ix = 0; ix < 2; ix++)
            {
                for (int iy = 0; iy < 2; iy++)
                {
                    int tileX = x*2+ix;
                    int tileY = y*2+iy;
                    int tileLOD = lod+1;

                    TXPArchive::TileInfo info;
                    if (!archive->getTileInfo(tileX,tileY,tileLOD,info))
                    continue;

                    osg::ref_ptr<osg::Node> tileContent = getTileContent(info,tileX,tileY,tileLOD,archive.get(), childrenLoc);

                    tileContent->setName("TileContent");

                    if (tileLOD < (numLods-1))
                    {
                        char pagedLODfile[1024];
                        sprintf(pagedLODfile,"%s\\subtiles%d_%dx%d_%d.txp",
                            archive->getDir(),
                            tileLOD,
                            tileX,
                            tileY,
                            archive->getId());

                        // there are tile sets which do not maintain the z extents in
                        // the tile table.  This attempt to address the issue by using
                        // the geometry bounding sphere.  The downside is that this is
                        // not coupled to the generation and may result in runtime cracks
                        if (info.center.z() == 0)
                        {
                            osg::BoundingSphere bSphere = tileContent->getBound();

                            info.center.z() = bSphere.center().z();
                            info.radius = bSphere.radius();
                        }

                        osg::ref_ptr<TXPPagedLOD> pagedLOD = new TXPPagedLOD;
                                    // note: use maximum(info.maxRange,1e7) as just maxRange would result in some corner tiles from being culled out.
                        pagedLOD->addChild(tileContent.get(),info.minRange,osg::maximum(info.maxRange,1e7));
                        pagedLOD->setFileName(1,pagedLODfile);
                        pagedLOD->setRange(1,0,info.minRange);
                        pagedLOD->setCenter(info.center);
                        pagedLOD->setRadius(info.radius);
                        pagedLOD->setPriorityOffset(0,numLods-lod);
                        pagedLOD->setPriorityScale(0,1.0f);
                        pagedLOD->setNumChildrenThatCannotBeExpired(1);
                        pagedLOD->setTileId(tileX,tileY,tileLOD);

                        if(tileType == trpgHeader::TileLocal)
                        {
                            osg::Vec3d center(info.center - parentInfo.bbox._min);
                            osg::Vec3d sw(info.bbox._min - parentInfo.bbox._min);
                            sw[2] = 0.0;
                            pagedLOD->setCenter(center - sw);
                            osg::Matrix offset;
                            offset.setTrans(sw);
                            osg::MatrixTransform *tform = new osg::MatrixTransform(offset);
                            tform->addChild(pagedLOD.get());
                            subtiles->addChild(tform);
                        }
                        else
                            subtiles->addChild(pagedLOD.get());
                    }
                    else
                    {
                        subtiles->setUserData(new TileIdentifier(tileX,tileY,tileLOD));
                        if(tileType == trpgHeader::TileLocal)
                        {
                            osg::Vec3d center(info.center - parentInfo.bbox._min);
                            osg::Vec3d sw(info.bbox._min - parentInfo.bbox._min);
                            sw[2] = 0.0;
                            osg::Matrix offset;
                            offset.setTrans(sw);
                            osg::MatrixTransform *tform = new osg::MatrixTransform(offset);
                            tform->addChild(tileContent.get());
                            subtiles->addChild(tform);
                        }
                        else
                            subtiles->addChild(tileContent.get());
                    }

                }
            }
        }

        //OSG_NOTICE << "Subtiles for " << x << " " << y << " " << lod << " lodaded" << std::endl;
        return subtiles.get();
    }

    return ReadResult::ERROR_IN_READING_FILE;
}

// If you change this then you have to change extractChildrenLocation()
void ReaderWriterTXP::createChildrenLocationString(const std::vector<TXPArchive::TileLocationInfo>& locs, std::string& locString) const
{
    std::stringstream theLoc;

    if(locs.size() == 0)
    {
        theLoc << "_" << locs.size();
    }
    else
    {

        theLoc << "_" << locs.size() << "_" << "{" ;

        for(unsigned int idx = 0; idx < locs.size(); ++idx)
        {
            const TXPArchive::TileLocationInfo& loc = locs[idx];

            theLoc << loc.x
                   << "_"
                   << loc.y
                   << "_"
                   << loc.addr.file
                   << "_"
                   << loc.addr.offset
                   << "_"
                   << loc.zmin
                   << "_"
                   << loc.zmax;
            if(idx != locs.size() -1)
                theLoc << "_";
        }
    }

    theLoc << "}" << std::ends;

    locString = theLoc.str();
}
bool ReaderWriterTXP::extractChildrenLocations(const std::string& name, int parentLod, std::vector<TXPArchive::TileLocationInfo>& locs, int nbChild) const
{
    locs.clear();

    if(nbChild == 0)
        return true;

    locs.resize(nbChild);

    // We look for '{', which should be the start of the list of {x,y,addr} children data
    // '}' should end the list.
    // We expect: X,Y,FID,FOFFSET,ZMIN,ZMAX
    std::string::size_type startOfList = name.find_last_of('{');
    if(startOfList == std::string::npos)
        return false;

    std::string::size_type endOfList = name.find_last_of('}');
    if(endOfList == std::string::npos)
        return false;

    // Extract the data
    strcpy(gbuf, name.substr(startOfList + 1, endOfList - startOfList - 1).c_str());
    char *token = strtok( gbuf, "_" );

    int nbTokenRead = 0;
    for(int idx = 0; idx < nbChild; idx++)
    {
        // X
        if(!token)
            break;
        locs[idx].x = atoi(token);
        nbTokenRead++;

        // Y
        token = strtok(0, "_");
        if(!token)
            break;
        locs[idx].y = atoi(token);
        nbTokenRead++;

        // FID
        token = strtok(0, "_");
        if(!token)
            break;
        locs[idx].addr.file = atoi(token);
        nbTokenRead++;

        // OFFSET
        token = strtok(0, "_");
        if(!token)
            break;
        locs[idx].addr.offset = atoi(token);
        nbTokenRead++;

        // ZMIN
        token = strtok(0, "_");
        if(!token)
            break;
        locs[idx].zmin = osg::asciiToFloat(token);
        nbTokenRead++;

        // ZMAX
        token = strtok(0, "_");
        if(!token)
            break;
        locs[idx].zmax = osg::asciiToFloat(token);
        nbTokenRead++;

        locs[idx].lod = parentLod+1;



        token = strtok(0, "_");
    }

    if(nbTokenRead != nbChild*6)
        return false;
    else
        return true;


}

std::string ReaderWriterTXP::getArchiveName(const std::string& dir)
{
#ifdef _WIN32
    const char _PATHD = '\\';
#elif defined(macintosh)
    const char _PATHD = ':';
#else
    const char _PATHD = '/';
#endif

    return dir+_PATHD+"archive.txp";
}

osg::ref_ptr< TXPArchive > ReaderWriterTXP::getArchive(int id, const std::string& dir)
{
    osg::ref_ptr< TXPArchive > archive = NULL;

    std::map< int,osg::ref_ptr<TXPArchive> >::iterator iter = _archives.find(id);

    if (iter != _archives.end())
    {
        archive = iter->second.get();
    }
    else
    {
        std::string archiveName = getArchiveName(dir);
        ReaderWriterTXPERROR("getArchive()") << "archive id " << id << " not found: \"" << archiveName << "\"" << std::endl;
    }
    return archive;
}

osg::ref_ptr< TXPArchive > ReaderWriterTXP::createArchive(int id, const std::string& dir)
{
    std::string archiveName = getArchiveName(dir);

    osg::ref_ptr< TXPArchive > archive = getArchive(id, dir);
    if (archive != NULL)
    {
        ReaderWriterTXPERROR("createArchive()") << "archive id " << id << " already exists: \"" << archiveName << "\"" << std::endl;
        return NULL;
    }

    archive = new TXPArchive;
    if (archive->openFile(archiveName) == false)
    {
        ReaderWriterTXPERROR("createArchive()") << "failed to load archive: \"" << archiveName << "\"" << std::endl;
        return NULL;
    }

    if (archive->loadMaterials() == false)
    {
        ReaderWriterTXPERROR("createArchive()") << "failed to load materials from archive: \"" << archiveName << "\"" << std::endl;
        return NULL;
    }

    if (archive->loadModels() == false)
    {
        ReaderWriterTXPERROR("createArchive()") << "failed to load models from archive: \"" << archiveName << "\"" << std::endl;
        return NULL;
    }

    if (archive->loadLightAttributes() == false)
    {
        ReaderWriterTXPERROR("createArchive()") << "failed to load light attributes from archive: \"" << archiveName << "\"" << std::endl;
        return NULL;
    }

    if (archive->loadTextStyles() == false)
    {
        ReaderWriterTXPERROR("createArchive()") << "failed to load text styles from archive: \"" << archiveName << "\"" << std::endl;
        return NULL;
    }

    archive->setId(id);

    _archives[id] = archive;

    return archive;
}

bool ReaderWriterTXP::removeArchive( int id )
{
    OSG_INFO<<"ReaderWriterTXP::removeArchive(id="<<id<<")"<<std::endl;
    //return (_archives.erase(id) >= 1);
    bool result=_archives.erase(id) >= 1;
    OSG_WARN<<"remove archive " << id << " size " << _archives.size()
        << " result " << result << std::endl;
    return result;

}

class SeamFinder: public osg::NodeVisitor
{
public:
    SeamFinder(int x, int y, int lod, const TXPArchive::TileInfo& info, TXPArchive *archive ):
    osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
    _x(x), _y(y), _lod(lod), _info(info), _archive(archive)
    {}

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

    SeamFinder& operator = (const SeamFinder&) { return *this; }

    int _x, _y, _lod;
    const TXPArchive::TileInfo& _info;
    TXPArchive *_archive;
};

#define equalDoubles(a, b) (fabs(a-b) < 0.001)

osg::Node* SeamFinder::seamReplacement(osg::Node* node)
{
    osg::Group* group = node->asGroup();
    if ( group == 0 )
        return node;

    std::vector<osg::Node*> nonSeamChildren;
    osg::LOD* hiRes = 0;
    osg::LOD* loRes = 0;

    const trpgHeader* header = _archive->GetHeader();
    trpgHeader::trpgTileType tileType;
    header->GetTileOriginType(tileType);

    for (unsigned int i = 0; i < group->getNumChildren(); i++)
    {
        osg::LOD* lod = dynamic_cast<osg::LOD*>(group->getChild(i));
        if (lod == 0)
        {
            nonSeamChildren.push_back(group->getChild(i));
            continue;
        }

        bool nonSeamChild = true;

        // looks like the problem is in here - likely due to seamLOD info
        // not being adjusted properly in tiled databases
            // seam center is outside the bounding box of the tile
        osg::Vec3 lodCenter = lod->getCenter();

        if(tileType == trpgHeader::TileLocal)
        {
            trpg2dPoint tileExtents;
            header->GetTileSize(0, tileExtents);
            osg::BoundingBox bbox;
            _archive->getExtents(bbox);
            osg::Vec3 offset(0.0, 0.0, 0.0);

            int divider = (0x1 << _lod);
            // calculate which tile model is located in
            tileExtents.x /= divider;
            tileExtents.y /= divider;
            offset[0] = _x*tileExtents.x;// + tileExtents.x*0.5;
            offset[1] = _y*tileExtents.y;// + tileExtents.y*0.5;
            lodCenter += offset;
        }

        if (!_info.bbox.contains(lodCenter))
        {
            const osg::LOD::RangeList& rangeList = lod->getRangeList();
            if (!rangeList.size())
            {
                // TODO: Warn here
                continue;
            }

            TXPArchive::TileInfo lod_plus_one_info;
            if (!this->_archive->getTileInfo(_x,_y,_lod+1,lod_plus_one_info))
            {
                // TODO: Warn here
                continue;
            }

            double lod_plus_oneSwitchInDistance =  lod_plus_one_info.maxRange;
            double lod0SwitchInDistance =  _info.lod0Range;

            // low res seam has min/max ranges of lod+1 range/lod 0 range
            if (equalDoubles(lod_plus_oneSwitchInDistance,rangeList.at(0).first) && equalDoubles(lod0SwitchInDistance,rangeList.at(0).second))
            {
                if (loRes==0)
                {
                    loRes = lod;
                    nonSeamChild = false;
                }
            }
            else
            // hi res seam has min/max ranges of 0 range/lod+1 range
            if (rangeList.at(0).first==0.0 && equalDoubles(lod_plus_oneSwitchInDistance,rangeList.at(0).second))
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
        osg::Vec3 lodCenter = loRes->getCenter();

        if(tileType == trpgHeader::TileLocal)
        {
            trpg2dPoint tileExtents;
            header->GetTileSize(0, tileExtents);
            osg::BoundingBox bbox;
            _archive->getExtents(bbox);
            osg::Vec3 offset(0.0, 0.0, 0.0);

            int divider = (0x1 << _lod);
            // calculate which tile model is located in
            tileExtents.x /= divider;
            tileExtents.y /= divider;
            offset[0] = _x*tileExtents.x;// + tileExtents.x*0.5;
            offset[1] = _y*tileExtents.y;// + tileExtents.y*0.5;
            lodCenter += offset;
        }

        osg::Vec3 delta = lodCenter-_info.center;
        if (fabs(delta.x())>fabs(delta.y()))
        {
            if ( delta.x() < 0.0 )
                --dx;    // west
            else
                dx++;                  // east
        }
        else
        {
            if ( delta.y() < 0.0 )
                --dy;    // south
            else
                ++dy;                  // north
        }

        TXPSeamLOD* seam = new TXPSeamLOD(_x, _y, lod, dx, dy);
        seam->setCenter(loRes->getCenter());
        seam->addChild(loRes->getChild(0));        // low res
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

osg::Node* ReaderWriterTXP::getTileContent(const TXPArchive::TileInfo &info, int x, int y, int lod, TXPArchive* archive,  std::vector<TXPArchive::TileLocationInfo>& childrenLoc)
{
    if ( archive == 0 )
        return 0;

    int majorVersion, minorVersion;
    archive->GetVersion(majorVersion, minorVersion);

    double realMinRange = info.minRange;
    double realMaxRange = info.maxRange;
    double  usedMaxRange = osg::maximum(info.maxRange,1e7);
    osg::Vec3 tileCenter;
    osg::Group* tileGroup = archive->getTileContent(x,y,lod,realMinRange,realMaxRange,usedMaxRange,tileCenter, childrenLoc);

    // if group has only one child, then simply use its child.
    while (tileGroup->getNumChildren()==1 && tileGroup->getChild(0)->asGroup())
    {
        tileGroup = tileGroup->getChild(0)->asGroup();
    }

    bool doSeam = false;
    if(majorVersion == 2 && minorVersion >= 1)
        doSeam = (childrenLoc.size() > 0);
    else
        doSeam = (lod < (archive->getNumLODs() - 1));

    // Handle seams
    if (doSeam)
    {
        SeamFinder sfv(x,y,lod,info,archive);
        tileGroup->accept(sfv);
    }

    return tileGroup;
}

// this version only gets called if the TXP version is >= than 2.1
osg::Node* ReaderWriterTXP::getTileContent(const TXPArchive::TileInfo &info, const TXPArchive::TileLocationInfo& loc, TXPArchive* archive,  std::vector<TXPArchive::TileLocationInfo>& childrenLoc)
{
    if ( archive == 0 )
        return 0;

    // int numLods = archive->getNumLODs();

    double realMinRange = info.minRange;
    double realMaxRange = info.maxRange;
    double usedMaxRange = osg::maximum(info.maxRange,1e7);
    osg::Vec3 tileCenter;
    osg::Group* tileGroup = archive->getTileContent(loc,realMinRange,realMaxRange,usedMaxRange,tileCenter, childrenLoc);

    // if group has only one child, then simply use its child.
    while (tileGroup->getNumChildren()==1 && tileGroup->getChild(0)->asGroup())
    {
        tileGroup = tileGroup->getChild(0)->asGroup();
    }

    // Handle seams
    if (childrenLoc.size() > 0)
    {
        SeamFinder sfv(loc.x, loc.y, loc.lod, info, archive);
        tileGroup->accept(sfv);
    }

    return tileGroup;
}

REGISTER_OSGPLUGIN(txp, ReaderWriterTXP)

