// BoundingVolumeRecords.cpp

#include "flt.h"
#include "Registry.h"
#include "BoundingVolumeRecords.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                              BoundingBoxRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<BoundingBoxRecord> g_BoundingBoxProxy;

BoundingBoxRecord::BoundingBoxRecord()
{
}


// virtual
BoundingBoxRecord::~BoundingBoxRecord()
{
}


void BoundingBoxRecord::endian()
{
}


////////////////////////////////////////////////////////////////////
//
//                              BoundingSphereRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<BoundingSphereRecord> g_BoundingSphereProxy;

BoundingSphereRecord::BoundingSphereRecord()
{
}


// virtual
BoundingSphereRecord::~BoundingSphereRecord()
{
}


void BoundingSphereRecord::endian()
{
}


////////////////////////////////////////////////////////////////////
//
//                              BoundingCylinderRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<BoundingCylinderRecord> g_BoundingCylinderProxy;

BoundingCylinderRecord::BoundingCylinderRecord()
{
}


// virtual
BoundingCylinderRecord::~BoundingCylinderRecord()
{
}


void BoundingCylinderRecord::endian()
{
}


////////////////////////////////////////////////////////////////////
//
//                              BoundingVolumeCenterRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<BoundingVolumeCenterRecord> g_BoundingVolumeCenterProxy;

BoundingVolumeCenterRecord::BoundingVolumeCenterRecord()
{
}


// virtual
BoundingVolumeCenterRecord::~BoundingVolumeCenterRecord()
{
}


void BoundingVolumeCenterRecord::endian()
{
}


////////////////////////////////////////////////////////////////////
//
//                              BoundingVolumeOrientationRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<BoundingVolumeOrientationRecord> g_BoundingVolumeOrientationProxy;

BoundingVolumeOrientationRecord::BoundingVolumeOrientationRecord()
{
}


// virtual
BoundingVolumeOrientationRecord::~BoundingVolumeOrientationRecord()
{
}


void BoundingVolumeOrientationRecord::endian()
{
}
