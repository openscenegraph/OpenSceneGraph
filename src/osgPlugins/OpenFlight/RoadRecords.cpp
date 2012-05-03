/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
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
        META_setMultitexture(_roadSegment)
        META_addChild(_roadSegment)
        META_dispose(_roadSegment)

    protected:

        virtual ~RoadSegment() {}
        virtual void readRecord(RecordInputStream& in, Document& /*document*/)
        {
            _roadSegment = new osg::Group;
            std::string id = in.readString(8);

            _roadSegment->setName(id);

            // Add to parent.
            if (_parent.valid())
                _parent->addChild(*_roadSegment);
        }
};

REGISTER_FLTRECORD(RoadSegment, ROAD_SEGMENT_OP)


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
        META_setMultitexture(_roadConstruction)
        META_addChild(_roadConstruction)
        META_dispose(_roadConstruction)

    protected:

        virtual ~RoadConstruction() {}
        virtual void readRecord(RecordInputStream& in, Document& /*document*/)
        {
            _roadConstruction = new osg::Group;

            std::string id = in.readString(8);

            _roadConstruction->setName(id);

            // Add to parent.
            if (_parent.valid())
                _parent->addChild(*_roadConstruction);
        }
};

REGISTER_FLTRECORD(RoadConstruction, ROAD_CONSTRUCTION_OP)



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
        META_setMultitexture(_roadPath)
        META_addChild(_roadPath)
        META_dispose(_roadPath)

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

REGISTER_FLTRECORD(RoadPath, ROAD_PATH_OP)



} // end namespace
