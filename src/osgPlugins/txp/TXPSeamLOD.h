#ifndef TXPSeamLOD_H
#define TXPSeamLOD_H

#include <osg/LOD>
#include <map>
#include <string>

#include "TXPTileNode.h"
#include "TXPArchive.h"

namespace txp
{

class TXPSeamLOD : public osg::Group
{
public:
    TXPSeamLOD();
    TXPSeamLOD(int x, int y, int lod, const osg::Vec3& center, float dmin, float dmid, float dmax);

    TXPSeamLOD(const TXPSeamLOD&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

    META_Node(txp, TXPSeamLOD);

    virtual void traverse(osg::NodeVisitor& nv);

    void setTileRef(bool* b)
    {
        _tileRef = b;
    }

    void setTxpNode(TXPTileNode* txpNode) { _txpNode = txpNode; }
    TXPTileNode* getTxpNode() const { return _txpNode; }

    void setArchive(TXPArchive* ar) { _archive = ar; }

protected:

    int _neighbourTileX;
    int _neighbourTileY;
    int _neighbourTileLOD;

    osg::Vec3 _center;
    float _min;
    float _mid;
    float _max;

    bool* _tileRef;

    TXPTileNode* _txpNode;
    TXPArchive* _archive;
    
    
};

}

#endif
