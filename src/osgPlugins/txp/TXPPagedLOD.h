// -*-c++-*-
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
#ifndef __TXPPAGEDLOD_H_
#define __TXPPAGEDLOD_H_

#include <osg/PagedLOD>
#include "TileMapper.h"

namespace txp
{
    class TXPPagedLOD : public osg::PagedLOD
    {
    public:
        TXPPagedLOD();

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        TXPPagedLOD(const TXPPagedLOD&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Node(txp, TXPPagedLOD);
        
        virtual void traverse(osg::NodeVisitor& nv);

        inline void setTileId(int x, int y, int lod)
        {
            _tileIdentifier.set(x,y,lod);
        }

        TileIdentifier _tileIdentifier;

        virtual osg::BoundingSphere computeBound() const;

    protected:
        virtual ~TXPPagedLOD();

    };

} // namespace

#endif // __TXPPAGEDLOD_H_
