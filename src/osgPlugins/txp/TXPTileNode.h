#ifndef __TXPTILENODE_H_
#define __TXPTILENODE_H_

#include <osg/Group>
#include "TXPArchive.h"

namespace txp
{
    class TXPArchive;
    class TXPTileNode : public osg::Group
    {
    public:
        TXPTileNode();
        
        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        TXPTileNode(const TXPTileNode&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Node(txp, TXPTileNode);

        void setArchive(TXPArchive* archive);
        bool loadTile(int x, int y, int lod);

        osg::Node* seamReplacement(osg::Node* child,int x, int y, int level, TXPArchive::TileInfo& info);

    protected:
        
        virtual ~TXPTileNode();

        TXPArchive*    _archive;
    };

} // namespace

#endif // __TXPTILENODE_H_
