//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#include <osg/Notify>
#include <osg/Group>
#include "Registry.h"
#include "Document.h"
#include "RecordInputStream.h"


namespace flt {

/** RoadSegment
*/
class RoadSegment : public PrimaryRecord
{
    osg::ref_ptr<osg::Group> _roadSegment;

    public:

        RoadSegment() {}

        META_Record(RoadSegment)

        META_setID(_roadSegment)
        META_setComment(_roadSegment)
        META_setMatrix(_roadSegment)
        META_setMultitexture(_roadSegment)
        META_addChild(_roadSegment)

    protected:

        virtual ~RoadSegment() {}

        virtual void readRecord(RecordInputStream& /*in*/, Document& /*document*/)
        {
            _roadSegment = new osg::Group;

            // Add to parent.
            if (_parent.valid())
                _parent->addChild(*_roadSegment);
        }
};

RegisterRecordProxy<RoadSegment> g_RoadSegment(ROAD_SEGMENT_OP);


/** RoadConstruction
*/
class RoadConstruction : public PrimaryRecord
{
    osg::ref_ptr<osg::Group> _roadConstruction;

    public:

        RoadConstruction() {}

        META_Record(RoadConstruction)

        META_setID(_roadConstruction)
        META_setComment(_roadConstruction)
        META_setMatrix(_roadConstruction)
        META_setMultitexture(_roadConstruction)
        META_addChild(_roadConstruction)

    protected:

        virtual ~RoadConstruction() {}

        virtual void readRecord(RecordInputStream& /*in*/, Document& /*document*/)
        {
            _roadConstruction = new osg::Group;

            // Add to parent.
            if (_parent.valid())
                _parent->addChild(*_roadConstruction);
        }
};

RegisterRecordProxy<RoadConstruction> g_RoadConstruction(ROAD_CONSTRUCTION_OP);


/** RoadPath
*/
class RoadPath : public PrimaryRecord
{
    osg::ref_ptr<osg::Group> _roadPath;

    public:

        RoadPath() {}

        META_Record(RoadPath)

        META_setID(_roadPath)
        META_setComment(_roadPath)
        META_setMatrix(_roadPath)
        META_setMultitexture(_roadPath)
        META_addChild(_roadPath)

    protected:

        virtual ~RoadPath() {}

        virtual void readRecord(RecordInputStream& /*in*/, Document& /*document*/)
        {
            _roadPath = new osg::Group;

            // Add to parent.
            if (_parent.valid())
                _parent->addChild(*_roadPath);
        }
};

RegisterRecordProxy<RoadPath> g_RoadPath(ROAD_PATH_OP);


} // end namespace
