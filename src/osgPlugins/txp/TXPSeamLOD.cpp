#include "TXPSeamLOD.h"
#include "TXPArchive.h"
#include "TXPTileNode.h"
#include "TXPPagedLOD.h"
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
    _hiResPresent = false;
    _nonSeamChildrenIndex = -1;
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
    _hiResPresent = false;
    _nonSeamChildrenIndex = -1;
}

TXPSeamLOD::TXPSeamLOD(const TXPSeamLOD& ttg,const osg::CopyOp& copyop) :
    Group(ttg,copyop)
{
    _neighbourTileX = ttg._neighbourTileX;
    _neighbourTileY = ttg._neighbourTileY;
    _neighbourTileLOD = ttg._neighbourTileLOD;
    _tileRef = ttg._tileRef;
    _archive = ttg._archive;
    _hiResPresent = ttg._hiResPresent;
    _nonSeamChildrenIndex = ttg._nonSeamChildrenIndex;
}

void TXPSeamLOD::traverse(osg::NodeVisitor& nv)
{
    if (nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR && _children.size()==2)
    {

        //osg::PagedLOD* pagedLOD = TileMapper::instance()->getPagedLOD(_neighbourTileX,_neighbourTileY, _neighbourTileLOD);
        TXPPagedLOD* pagedLOD = dynamic_cast<TXPPagedLOD*>(TileMapper::instance()->getPagedLOD(_neighbourTileX,_neighbourTileY, _neighbourTileLOD));
#if 1
        if (pagedLOD && pagedLOD->getLastTraversedChild()>0)
            getChild(1)->accept(nv);
        else
            getChild(0)->accept(nv);
#else
        bool acceptLoRes = true;
        if (pagedLOD)
        {
            
            float distance = nv.getDistanceToEyePoint(_center,true);
            distance = nv.getDistanceToEyePoint(pagedLOD->getCenter(),true);

            //std::cout<<"distance to eye from center = "<<distance<<" min = "<< _min<<" mid = "<< _mid << " max = "<<_max<<std::endl;
            //std::cout<<"   TXPSeam::_center "<<_center<<"   PageLOD::_center "<<pagedLOD->getCenter()<<std::endl;
            int numChildren = osg::minimum(getNumChildren(),pagedLOD->getNumChildren());
            for(int i=numChildren-1;i>=0;--i)
            {
                //std::cout<<"   child "<<i<<" range = min "<<pagedLOD->getMinRange(i)<<" max = "<<pagedLOD->getMaxRange(i)<<std::endl;
                if (distance<=pagedLOD->getMaxRange(i)) 
                {
                    getChild(i)->accept(nv);
                    acceptLoRes = false;
                    break;
                }
            }
        }
        if (acceptLoRes)
        {
            getChild(0)->accept(nv); // pick low res
        }

        if (_nonSeamChildrenIndex > -1)
        {
            for (int i = _nonSeamChildrenIndex; i < (int)getNumChildren(); i++ )
                getChild(i)->accept(nv);
        }
#endif
    }
    else
    {
        Group::traverse(nv);
    }
}
