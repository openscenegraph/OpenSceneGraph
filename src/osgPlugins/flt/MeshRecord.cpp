// MeshRecord.cpp

#if defined(_MSC_VER)
#pragma warning(disable:4786) // Truncated debug names.
#endif

#include "flt.h"
#include "Registry.h"
#include "MeshRecord.h"
#include "Input.h"
#include <assert.h>

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                          MeshRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<MeshRecord> g_MeshProxy;

MeshRecord::MeshRecord() : FaceRecord()
{
}


// virtual
MeshRecord::~MeshRecord()
{
}


void MeshRecord::endian()
{
  // Should only be in here for little-endian machines.
  assert ( flt::isLittleEndianMachine() );

  // Call the base class's function.
  // Note 1: We can do this because all the data members are the same.
  // Note 2: FaceRecord::endian() checks for file version. This is 
  // unnecessary for a MeshRecord because it is new with 15.7. Therefore it 
  // could be optimized here, but doing this shouldn't slow us down too much.
  FaceRecord::endian();
}


// virtual
bool MeshRecord::readLocalData ( Input &fr )
{
  // Bypass FaceRecord's readLocalData() because it doesn't look for the 
  // correct kind of records. Instead, we call PrimNodeRecord's readLocalData()
  // which looks for Ancillary Record's (we should at least find a 
  // LocalVertexPoolRecord), and then reads from a PushLevelRecord to a 
  // PopLevelRecord. Inside the push-pop pair we should have at least one 
  // MeshPrimitiveRecord.
  return PrimNodeRecord::readLocalData ( fr );
}
