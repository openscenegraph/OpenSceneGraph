// RoadRecords.cpp

#include "flt.h"
#include "Registry.h"
#include "RoadRecords.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                         RoadSegmentRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<RoadSegmentRecord> g_RoadSegmentProxy;

RoadSegmentRecord::RoadSegmentRecord()
{
}


// virtual
RoadSegmentRecord::~RoadSegmentRecord()
{
}

////////////////////////////////////////////////////////////////////
//
//                         RoadConstructionRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<RoadConstructionRecord> g_RoadConstructionProxy;

RoadConstructionRecord::RoadConstructionRecord()
{
}


// virtual
RoadConstructionRecord::~RoadConstructionRecord()
{
}

////////////////////////////////////////////////////////////////////
//
//                         RoadPathRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<RoadPathRecord> g_RoadPathProxy;

RoadPathRecord::RoadPathRecord()
{
}


// virtual
RoadPathRecord::~RoadPathRecord()
{
}
