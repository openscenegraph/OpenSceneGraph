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

    protected:

        virtual ~TXPNode();

        virtual bool computeBound() const;

        void updateEye(osg::NodeVisitor& nv);
        void updateSceneGraph();

        osg::Node* addPagedLODTile(int x, int y, int lod);

        std::string    _archiveName;
        std::string _options;

        osg::ref_ptr<TXPArchive>        _archive;
        osg::ref_ptr<TXPPageManager>    _pageManager;

        double _originX;
        double _originY;
        osg::BoundingBox    _extents;

        std::vector<osg::Node*> _nodesToAdd;
        std::vector<osg::Node*> _nodesToRemove;

    };
    

} // namespace

#endif // __TXPNODE_H_
