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

#include "TXPArchive.h"

namespace txp
{
class ReaderWriterTXP : public osgDB::ReaderWriter
{
public:
    virtual const char* className() const
    {
        return "TXP Reader/Writer";
    }
    
    virtual bool acceptsExtension(const std::string& extension)
    {
        return osgDB::equalCaseInsensitive(extension,"txp");
    }
    
    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*);
    
protected:
    TXPArchive *getArchive(int id, const std::string&);
    std::map< int,osg::ref_ptr<TXPArchive> >    _archives;

	osg::Node* getTileContent(TXPArchive::TileInfo &info, int x, int y, int lod, TXPArchive* archive);
    
    static int _archiveId;
};

} // namespace

#endif // __READERWRITER_TXP_H_
