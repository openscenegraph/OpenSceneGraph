// RoadRecords.h

#ifndef __FLT_ROAD_RECORDS_H
#define __FLT_ROAD_RECORDS_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {



////////////////////////////////////////////////////////////////////
//
//                           RoadSegmentRecord
//
////////////////////////////////////////////////////////////////////

class RoadSegmentRecord : public PrimNodeRecord
{
    public:

        RoadSegmentRecord();
        virtual ~RoadSegmentRecord();

        virtual Record* clone() const { return new RoadSegmentRecord(); }
        virtual const char* className() const { return "RoadSegmentRecord"; }
        virtual int classOpcode() const { return ROAD_SEGMENT_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:


};

////////////////////////////////////////////////////////////////////
//
//                           RoadConstructionRecord
//
////////////////////////////////////////////////////////////////////

class RoadConstructionRecord : public PrimNodeRecord
{
    public:

        RoadConstructionRecord();
        virtual ~RoadConstructionRecord();

        virtual Record* clone() const { return new RoadConstructionRecord(); }
        virtual const char* className() const { return "RoadConstructionRecord"; }
        virtual int classOpcode() const { return ROAD_CONSTRUCTION_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:


};

////////////////////////////////////////////////////////////////////
//
//                           RoadPathRecord
//
////////////////////////////////////////////////////////////////////

class RoadPathRecord : public PrimNodeRecord
{
    public:

        RoadPathRecord();
        virtual ~RoadPathRecord();

        virtual Record* clone() const { return new RoadPathRecord(); }
        virtual const char* className() const { return "RoadPathRecord"; }
        virtual int classOpcode() const { return ROAD_PATH_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:


};

// Note: RoadZoneRecord is an ancillary record, so it is left undefined to
//       be treated as an unknown (ancillary) record, just like the
//       eyepoint trackplane record or the translate, scale, and rotate 
//       records, whose contents are not interpreted. 
//       The above three types of records are basically treated like 
//       unknown records except that they are primary records and can have
//       substructures under them.

}; // end namespace flt

#endif // __FLT_ROAD_RECORDS_H
