// MeshPrimitiveRecord.h

#ifndef __FLT_MESH_PRIMITIVE_RECORDS_H
#define __FLT_MESH_PRIMITIVE_RECORDS_H

#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"

namespace flt {


////////////////////////////////////////////////////////////////////
//
//                       MeshPrimitiveRecord
//
////////////////////////////////////////////////////////////////////

struct SMeshPrimitive
{
  SRecHeader RecHeader;   // Header (opcode and size)
  uint16 primitiveType;   // Primitive Type - can be one of the following values:
                          // 1 - Triangle Strip
                          // 2 - Triangle Fan
                          // 3 - Quadrilateral Strip
                          // 4 - Indexed Polygon
                          // Note: This field specifies how the vertices of the primitive are interpreted.
  uint16 indexSize;       // Specifies the length (in bytes) of each of the vertex indices that follow - will be 1, 2, or 4
  uint32 numVerts;        // Number of vertices contained in this primitive
};


class MeshPrimitiveRecord : public PrimNodeRecord
{
public:

  enum PrimitiveType
  {
    TRIANGLE_STRIP =      1,
    TRIANGLE_FAN =        2,
    QUADRILATERAL_STRIP = 3,
    INDEXED_POLYGON =     4
  };

  MeshPrimitiveRecord();

  virtual void          accept ( RecordVisitor &rv ) { rv.apply ( *this ); }
  
  virtual const char *  className() const { return "MeshPrimitiveRecord"; }
  virtual int           classOpcode() const { return MESH_PRIMITIVE_OP; }
  virtual Record *      clone() const { return new MeshPrimitiveRecord(); }

  virtual SMeshPrimitive* getData() const { return (SMeshPrimitive *) _pData; }
  uint32                getNumVertices() const { return this->getData()->numVerts; }
  bool                  getVertexIndex ( const uint32 &whichVertex, uint32 &index ) const;

  virtual size_t        sizeofData() const { return sizeof ( SMeshPrimitive ); }

protected:

  virtual ~MeshPrimitiveRecord();

  virtual void          endian();

  char *                _getStartOfVertexIndices() const;
};


}; // end namespace flt


#endif // __FLT_MESH_PRIMITIVE_RECORDS_H
