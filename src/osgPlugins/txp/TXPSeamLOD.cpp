#include "TXPSeamLOD.h"
#include "TXPArchive.h"
#include "TXPTileNode.h"
#include "TileMapper.h"

using namespace txp;


TXPSeamLOD::TXPSeamLOD() :
    Group()
{
    _neighbourTileX =
    _neighbourTileY =
    _neighbourTileLOD = -1;
    _tileRef = 0;
    _txpNode = 0;
    _archive = 0;
}

TXPSeamLOD::TXPSeamLOD(int x, int y, int lod, const osg::Vec3& center, float dmin, float dmid, float dmax) :
    Group()
{
    _neighbourTileX = x;
    _neighbourTileY = y;
    _neighbourTileLOD = lod;
    _center = center;
    _min = dmin;
    _mid = dmid;
    _max = dmax;
    _txpNode = 0;
    _tileRef = 0;
    _archive = 0;
}

TXPSeamLOD::TXPSeamLOD(const TXPSeamLOD& ttg,const osg::CopyOp& copyop) :
    Group(ttg,copyop)
{
    _neighbourTileX = ttg._neighbourTileX;
    _neighbourTileY = ttg._neighbourTileY;
    _neighbourTileLOD = ttg._neighbourTileLOD;

    _tileRef = ttg._tileRef;
    _archive = ttg._archive;
}

void TXPSeamLOD::traverse(osg::NodeVisitor& nv)
{
    if (nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR && _children.size()==2)
    {

        float distance = nv.getDistanceToEyePoint(_center,true);
        
        if (distance<_mid)
        {
            // cap the lod's that can be used to what is available in the adjacent PagedLOD.
            osg::PagedLOD* pagedLOD = TileMapper::instance()->getPagedLOD(_neighbourTileX,_neighbourTileY, _neighbourTileLOD);
            if (pagedLOD && pagedLOD->getNumChildren()>1) getChild(1)->accept(nv);
            else getChild(0)->accept(nv);
        }
        else
        {
            getChild(0)->accept(nv);
        }

    }
    else
    {
        Group::traverse(nv);
    }
}
