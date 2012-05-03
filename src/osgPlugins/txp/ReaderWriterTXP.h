/***************************************************************************
 * December 2003
 *
 * This TerraPage loader was re-written in a fashion to use PagedLOD
 * to manage paging entirely, also includes a version of Terrex's smart mesh
 * adapted to work with PagedLOD. The essential code by Boris Bralo is still present,
 * slight modified.
 * nick at terrex dot com
 *
 * Ported to PagedLOD technology by Trajce Nikolov (Nick) & Robert Osfield
 *****************************************************************************/

/***************************************************************************
 * OpenSceneGraph loader for Terrapage format database
 * by Boris Bralo 2002
 *
 * based on/modifed  sgl (Scene Graph Library) loader by Bryan Walsh
 *
 * This loader is based on/modified from Terrain Experts Performer Loader,
 * and was ported to SGL by Bryan Walsh / bryanw at earthlink dot net
 *
 * That loader is redistributed under the terms listed on Terrain Experts
 * website (www.terrex.com/www/pages/technology/technologypage.htm)
 *
 * "TerraPage is provided as an Open Source format for use by anyone...
 * We supply the TerraPage C++ source code free of charge.  Anyone
 * can use it and redistribute it as needed (including our competitors).
 * We do, however, ask that you keep the TERREX copyrights intact."
 *
 * Copyright Terrain Experts Inc. 1999.
 * All Rights Reserved.
 *
 *****************************************************************************/
#ifndef __READERWRITER_TXP_H_
#define __READERWRITER_TXP_H_

#include "trpage_sys.h"

#include <osg/Object>
#include <osg/Node>
#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osg/Referenced>
#include <map>

#include <OpenThreads/ScopedLock>
#include <OpenThreads/ReentrantMutex>

#include "TXPArchive.h"

namespace txp
{
class ReaderWriterTXP : public osgDB::ReaderWriter
{
public:

    ReaderWriterTXP()
    {
        supportsExtension("txp","Terrapage txp format");
    }

    virtual const char* className() const
    {
        return "TXP Reader/Writer";
    }

    virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        if( !acceptsExtension(osgDB::getFileExtension(file) ))
            return ReadResult::FILE_NOT_HANDLED;

        OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_serializerMutex);

        return const_cast<ReaderWriterTXP*>(this)->local_readNode(file, options);
    }

    bool removeArchive( int id );

protected:


    ReadResult local_readNode(const std::string& file, const osgDB::ReaderWriter::Options* options);

    std::string getArchiveName(const std::string& dir);
    osg::ref_ptr< TXPArchive > createArchive(int id, const std::string& dir);
    osg::ref_ptr< TXPArchive > getArchive(int id, const std::string&);

    osg::Node* getTileContent(const TXPArchive::TileInfo &info, int x, int y, int lod, TXPArchive* archive,  std::vector<TXPArchive::TileLocationInfo>& childrenLoc);
    osg::Node* getTileContent(const TXPArchive::TileInfo &info, const TXPArchive::TileLocationInfo& loc, TXPArchive* archive,  std::vector<TXPArchive::TileLocationInfo>& childrenLoc);
    void createChildrenLocationString(const std::vector<TXPArchive::TileLocationInfo>& locs, std::string& locString) const;
    bool extractChildrenLocations(const std::string& name, int parentLod, std::vector<TXPArchive::TileLocationInfo>& locs, int nbChild) const;

    mutable OpenThreads::ReentrantMutex               _serializerMutex;

    std::map< int,osg::ref_ptr<TXPArchive> >    _archives;
    static int                                  _archiveId;
};

} // namespace

#endif // __READERWRITER_TXP_H_
