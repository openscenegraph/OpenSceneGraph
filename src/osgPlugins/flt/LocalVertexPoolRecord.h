// LocalVertexPoolRecord.h

#ifndef __FLT_LOCAL_VERTEX_POOL_RECORDS_H
#define __FLT_LOCAL_VERTEX_POOL_RECORDS_H

#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"

namespace flt {


////////////////////////////////////////////////////////////////////
//
//                       LocalVertexPoolRecord
//
////////////////////////////////////////////////////////////////////

struct SLocalVertexPool // Local vertex info, new with 15.7
{
  SRecHeader RecHeader;   // Header (opcode and size)
  uint32 numVerts;        // Number of vertices contained in this record
  uint32 attributeMask;   // 32 bit mask indicating what kind of vertex information is contained in this vertex list. Bits are ordered from left to right (bit 1 is leftmost).
};


class LocalVertexPoolRecord : public AncillaryRecord
{
public:

  enum AttributeMask
  {
    POSITION =    0x80000000, // Has Position -   indicates that each vertex in the list includes x, y, and z coordinates (three double-precision floating point values)
    COLOR_INDEX = 0x40000000, // Has Color Index- indicates that each vertex in the list includes a color value that is a color table index (one integer value)
    RGB_COLOR =   0x20000000, // Has RGB Color -  indicates that each vertex in the list includes a color value that is a packed RGB color value (one integer value) NOTE: Bits 2 and 3 are mutually exclusive - a vertex can have either a color index or an RGB color value or neither, but cannot have both a color index and an RGB value.
                              //    In 15.8, has RGBA color
    NORMAL =      0x10000000, // Has Normal -     indicates that each vertex in the list includes a normal (three single-precision floating point values)
    BASE_UV =     0x08000000, // Has Base UV -    indicates that each vertex in the list includes uv texture coordinates for the base texture (two single-precision floating point values)
    UV_1 =        0x04000000, // Has UV 1 -       indicates that each vertex in the list includes uv texture coordinates for layer 1 (two single-precision floating point values)
    UV_2 =        0x02000000, // Has UV 2 -       indicates that each vertex in the list includes uv texture coordinates for layer 2 (two single-precision floating point values)
    UV_3 =        0x01000000, // Has UV 3 -       indicates that each vertex in the list includes uv texture coordinates for layer 3 (two single-precision floating point values)
    UV_4 =        0x00800000, // Has UV 4 -       indicates that each vertex in the list includes uv texture coordinates for layer 4 (two single-precision floating point values)
    UV_5 =        0x00400000, // Has UV 5 -       indicates that each vertex in the list includes uv texture coordinates for layer 5 (two single-precision floating point values)
    UV_6 =        0x00200000, // Has UV 6 -       indicates that each vertex in the list includes uv texture coordinates for layer 6 (two single-precision floating point values)
    UV_7 =        0x00100000  // Has UV 7 -       indicates that each vertex in the list includes uv texture coordinates for layer 7 (two single-precision floating point values)
  };

  LocalVertexPoolRecord();

  virtual void          accept ( RecordVisitor &rv ) { rv.apply ( *this ); }
  
  virtual const char *  className() const { return "LocalVertexPoolRecord"; }
  virtual int           classOpcode() const { return LOCAL_VERTEX_POOL_OP; }
  virtual Record *      clone() const { return new LocalVertexPoolRecord(); }

  virtual SLocalVertexPool* getData() const { return (SLocalVertexPool *) _pData; }
  uint32                getNumVertices() const { return this->getData()->numVerts; }
  // Vertex attributes.
  bool                  getColorIndex ( const uint32 &whichVertex, uint32 &colorIndex ) const;
  bool                  getColorRGBA ( const uint32 &whichVertex, float32 &r, float32 &g, float32 &b, float32 &a ) const;
  bool                  getNormal ( const uint32 &whichVertex, float32 &nx, float32 &ny, float32 &nz ) const;
  bool                  getPosition ( const uint32 &whichVertex, float64 &px, float64 &py, float64 &pz ) const;
  bool                  getUV ( const uint32 &whichVertex, const AttributeMask &whichUV, float32 &u, float32 &v ) const;

  bool                  hasAttribute ( const uint32 &bits ) const { return flt::hasBits ( this->getData()->attributeMask, bits ); }

  bool                  isInRange ( const uint32 &i ) const { return i < this->getNumVertices(); }

  virtual void          postReadInit();
  
  virtual size_t        sizeofData() const { return sizeof ( SLocalVertexPool ); }

protected:

  class Offset
  {
  public:
    uint32 position, color, normal, baseUV, uv1, uv2, uv3, uv4, uv5, uv6, uv7;
    Offset() : position ( 0 ), color ( 0 ), normal ( 0 ), baseUV ( 0 ), uv1 ( 0 ), uv2 ( 0 ), uv3 ( 0 ), uv4 ( 0 ), uv5 ( 0 ), uv6 ( 0 ), uv7 ( 0 ){}
  } _offset;

  virtual ~LocalVertexPoolRecord();

  virtual void          endian();

  uint32                _getOffset ( const AttributeMask &attribute ) const;
  char *                _getStartOfVertices() const;
  char *                _getStartOfAttribute ( const uint32 &whichVertex, const uint32 &attributeOffset ) const;

  void                  _initAttributeOffsets();

  int                   _getVertexSizeBytes() const;
  mutable int           _vertexSizeBytesCache;
};


}; // end namespace flt


///////////////////////////////////////////////////////////////////////////////
//
//  Used in cpp file. Put it here because (if memory serves) gcc doesn't like 
//  macros in cpp files, only headers. If this is wrong then move to cpp file.
//
//  Note: Asking for an attribute the vertex does not have will just return 
//  false. However, asking for a vertex that is out of range should assert
//  and then return false. This was an arbitrary decision.
//
///////////////////////////////////////////////////////////////////////////////

#define ARGUMENT_CHECK_FOR_GET_FUNCTION(which,attribute) \
  if ( false == this->hasAttribute ( attribute ) ) \
    return false; \
  if ( false == this->isInRange ( which ) ) \
  { \
    assert ( 0 ); \
    return false; \
  }

#endif // __FLT_LOCAL_VERTEX_POOL_RECORDS_H
