//  -*-c++-*- 
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
#ifndef __TXPNODE_H_
#define __TXPNODE_H_

#include <osg/Group>
#include <osg/NodeVisitor>
#include <osg/NodeCallback>
#include <osg/ref_ptr>

#include "TXPArchive.h"
#include "TXPPageManager.h"

namespace txp
{

class TXPNode : public osg::Group
{
public:

    TXPNode();
    
    /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
    TXPNode(const TXPNode&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    
    META_Node(txp, TXPNode);
    
    virtual void traverse(osg::NodeVisitor& nv);
    
    void setArchiveName(const std::string& archiveName);
    void setOptions(const std::string& options);
    
    const std::string& getOptions() const;
    const std::string& getArchiveName() const;
    
    bool loadArchive();
    
    TXPArchive* getArchive();

	void setArchive( TXPArchive* archive )
	{
		_archive = archive;
	}
    
    virtual osg::BoundingSphere computeBound() const;
    
protected:

    virtual ~TXPNode();
    
    void updateEye(osg::NodeVisitor& nv);
    void updateSceneGraph();
    
    // Create a page lod for lod 0 with givin grid location (x,y)
    osg::Node* addPagedLODTile(int x, int y);
    
    std::string			 _archiveName;
    std::string			 _options;
    
    osg::ref_ptr<TXPArchive>     _archive;
    osg::ref_ptr<TXPPageManager> _pageManager;
    
    double			 _originX;
    double			 _originY;
    osg::BoundingBox		 _extents;
    
    std::vector<osg::Node*>	 _nodesToAdd;
    std::vector<osg::Node*>	 _nodesToRemove;
    
};


} // namespace

#endif // __TXPNODE_H_
