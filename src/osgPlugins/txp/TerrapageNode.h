//C++ header.

#ifndef TERRAPAGENODE_H
#define TERRAPAGENODE_H

#include "trPagePageManager.h"

#include <osg/Node>
#include <osg/BoundingBox>


namespace txp
{
class TrPageArchive;
class TerrapageNode : public osg::Group
{
    public:
    
        TerrapageNode();
        
        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        TerrapageNode(const TerrapageNode&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Node(txp, TerrapageNode);

        virtual void traverse(osg::NodeVisitor& nv);
        
        
        bool loadDatabase();
        
        void setDatabaseName(const std::string& name) { _databaseName = name; }
        const std::string& getDatabaseName() const { return _databaseName; }

        void setDatabaseOptions(const std::string& name) { _databaseOptions = name; }
        const std::string& getDatabaseOptions() const { return _databaseOptions; }

        void setDatabaseDimensions(const osg::BoundingBox& box) { _databaseDimensions = box; }
        const osg::BoundingBox& getDatabaseDimensions() const { return _databaseDimensions; }


        virtual void updateSceneGraph();
        
        virtual void updateEyePoint(const osg::Vec3& eyepoint) const;
        
        
    protected:
    
        virtual ~TerrapageNode();

        virtual bool computeBound() const;

        osg::BoundingBox    _databaseDimensions;
        std::string         _databaseName;
        std::string         _databaseOptions;
        OSGPageManager*     _pageManager;
        mutable osg::Vec3   _lastRecordEyePoint;

		static osg::ref_ptr<TrPageArchive> _archive;
		bool _dbLoaded;
};


} // namespace txp
#endif
