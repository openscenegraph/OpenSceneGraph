// MeshPrimitiveRecords.cpp

#if defined(_MSC_VER)
#pragma warning(disable:4786) // Truncated debug names.
#endif

#include "MeshPrimitiveRecord.h"
#include "Registry.h"
#include <assert.h>

using namespace flt;


////////////////////////////////////////////////////////////////////
//
//                       MeshPrimitiveRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<MeshPrimitiveRecord> g_MeshPrimitiveProxy;

MeshPrimitiveRecord::MeshPrimitiveRecord() : PrimNodeRecord()
{
}


// virtual
MeshPrimitiveRecord::~MeshPrimitiveRecord()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the address of the beginning of the vertex indices.
//
///////////////////////////////////////////////////////////////////////////////

char *MeshPrimitiveRecord::_getStartOfVertexIndices() const
{
  SMeshPrimitive *mesh = this->getData();
  char *index = (char *) (&(mesh[1]));
  return index;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the index into the local vertex pool for the given i'th vertex of 
//  this mesh primitive.
//
///////////////////////////////////////////////////////////////////////////////

bool MeshPrimitiveRecord::getVertexIndex ( const uint32 &whichVertex, uint32 &index ) const
{
  assert ( whichVertex < this->getNumVertices() );

  // Get pointer to start of vertex indices.
  char *start = (char *) this->_getStartOfVertexIndices();

  // Need a pointer to the mesh structure.
  SMeshPrimitive *mesh = this->getData();

  // Move the pointer to the beginning of the requested vertex index. We are 
  // treating the array of indices as an array of chars (8 bits) so we have to
  // multiply times the correct elements size (mesh->indexSize) to get the 
  // correct index.
  uint32 adjust = whichVertex * ((uint32) mesh->indexSize);
  start = &start[adjust];

  // The index "adjust" that we just calculated should not walk off the end.
  assert ( adjust <= mesh->indexSize * mesh->numVerts );

  // Interpret the address "start" correctly...
  switch ( mesh->indexSize )
  {
  case sizeof ( uint32 ): index =           *((uint32 *) start);  break; // No cast required.
  case sizeof ( uint16 ): index = (uint32) (*((uint16 *) start)); break;
  case sizeof ( uint8  ): index = (uint32) (*((uint8 *)  start)); break;
  default:

    assert ( 0 ); // Invalid index size (according to 15.7.0 specs).
    return false;
  }

  // It worked.
  return true;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Convert the data from big-endian to little-endian.
//
///////////////////////////////////////////////////////////////////////////////

void MeshPrimitiveRecord::endian()
{
  // Should only be in here for little-endian machines.
  assert ( flt::isLittleEndianMachine() );

  SMeshPrimitive *mesh = this->getData();
  ENDIAN ( mesh->primitiveType );
  ENDIAN ( mesh->indexSize );
  ENDIAN ( mesh->numVerts );

  // Get pointer to start of vertex indices.
  char *index = this->_getStartOfVertexIndices();

  // Determine the size of the elements in the array of vertex indices.
  switch ( mesh->indexSize )
  {
  case sizeof ( uint32 ): // 32 bits.

    // Perform byte-swap on all the elements in the array.
    flt::swapBytesArray ( sizeof ( uint32 ), mesh->numVerts, (uint32 *) index );
    break;

  case sizeof ( uint16 ): // 16 bits.

    // Perform byte-swap on all the elements in the array.
    flt::swapBytesArray ( sizeof ( uint16 ), mesh->numVerts, (uint16 *) index );
    break;

  case sizeof ( uint8 ): // 8 bits.

    // We don't have to swap bytes because there is only one byte.
    break;

  default:

    assert ( 0 ); // Invalid index size (according to 15.7.0 specs).
  }

#ifdef _DEBUG
  // Sanity check. The address immediately following the last element, 
  // minus the start of this record, should be the size of this record.
  char *offEndC = &(index[mesh->numVerts * mesh->indexSize]);
  uint32 offEndI = (uint32) offEndC;
  uint32 indexI = (uint32) mesh;
  uint32 diff32 = ( offEndI ) - ( indexI );
  uint16 diff16 = (uint16) ( diff32 );
  assert ( mesh->RecHeader._wLength == diff16 );
#endif
}
