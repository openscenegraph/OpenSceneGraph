// LocalVertexPoolRecords.cpp

#ifdef _WIN32
#pragma warning(disable:4786) // Truncated debug names.
#endif

#include "LocalVertexPoolRecord.h"
#include "Registry.h"
#include <assert.h>

// The sizes (in bytes) of the various attribute sections of the vertex data.
#define SIZE_POSITION       24 // 3 * float64 = 3 * 8
#define SIZE_COLOR           4 // 1 uint32
#define SIZE_NORMAL         12 // 3 * float32 = 3 * 4
#define SIZE_BASE_UV         8 // 2 * float32 = 2 * 4
#define SIZE_UV_1           SIZE_BASE_UV
#define SIZE_UV_2           SIZE_BASE_UV
#define SIZE_UV_3           SIZE_BASE_UV
#define SIZE_UV_4           SIZE_BASE_UV
#define SIZE_UV_5           SIZE_BASE_UV
#define SIZE_UV_6           SIZE_BASE_UV
#define SIZE_UV_7           SIZE_BASE_UV


using namespace flt;


////////////////////////////////////////////////////////////////////
//
//                       LocalVertexPoolRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<LocalVertexPoolRecord> g_LocalVertexPoolProxy;
LocalVertexPoolRecord *_current = NULL;

LocalVertexPoolRecord::LocalVertexPoolRecord()
  : AncillaryRecord(),
    _vertexSizeBytesCache(0)
{
}


// virtual
LocalVertexPoolRecord::~LocalVertexPoolRecord()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the address of the beginning of the vertices.
//
///////////////////////////////////////////////////////////////////////////////

char *LocalVertexPoolRecord::_getStartOfVertices() const
{
  SLocalVertexPool *pool = (SLocalVertexPool *) this->getData();
  char *vertex = (char *) (&(pool[1]));
  return vertex;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the address of the beginning of the specified vertex attribute.
//
///////////////////////////////////////////////////////////////////////////////

char *LocalVertexPoolRecord::_getStartOfAttribute ( const uint32 &whichVertex, const uint32 &offset ) const
{
  assert ( whichVertex < this->getNumVertices() );

  // Get pointer to start of vertex data.
  char *startOfVertices = (char *) this->_getStartOfVertices();

  // The number of vertices.
  //uint32 numVertices = this->getNumVertices();

  // NOTE This function formerly computed sizeOfVertex by taking the record length and dividing
  //   by the number of vertices. This is wrong in 15.7 and beyond, as the record could
  //   exceed 65535 bytes and be continued, in which case the "original" record length
  //   field does not contain the actual length of the data.

  // The size of each individual vertex depends on its attributes, and is cached.
  size_t sizeOfVertex = this->_getVertexSizeBytes();

  // Set the start of the desired vertex's data.
  char *startOfAttribute = &startOfVertices[whichVertex * sizeOfVertex];

  // Now adjust for the offset within this vertex.
  startOfAttribute += offset;

  // Return the pointer.
  return startOfAttribute;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the specified vertex attribute.
//
///////////////////////////////////////////////////////////////////////////////

bool LocalVertexPoolRecord::getColorIndex ( const uint32 &whichVertex, uint32 &colorIndex ) const
{
  ARGUMENT_CHECK_FOR_GET_FUNCTION ( whichVertex, COLOR_INDEX );

  // Offset to the start of the color.
  uint32 *color = (uint32 *) this->_getStartOfAttribute ( whichVertex, _offset.color );
  if ( NULL == color ) 
    return false;

  // Set the color index and return.
  colorIndex = *color;
  return true;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the specified vertex attribute.
//
///////////////////////////////////////////////////////////////////////////////

bool LocalVertexPoolRecord::getColorRGBA ( const uint32 &whichVertex, float32 &r, float32 &g, float32 &b, float32 &a ) const
{
  ARGUMENT_CHECK_FOR_GET_FUNCTION ( whichVertex, RGB_COLOR );

  // Offset to the start of the color.
  uint32 *color = (uint32 *) this->_getStartOfAttribute ( whichVertex, _offset.color );
  if ( NULL == color ) 
    return false;

  // Since we already byte-swapped, we have to check the endian of this 
  // machine so that we know how to grab the channels.
  uint32 red, green, blue, alpha;

  // If this machine is little-endian...
  if ( flt::isLittleEndianMachine() )
  {
    red   = ( ( (*color) & 0xFF000000 ) >>  0 );
    green = ( ( (*color) & 0x00FF0000 ) >>  8 );
    blue  = ( ( (*color) & 0x0000FF00 ) >> 16 );
    alpha = ( ( (*color) & 0x000000FF ) >> 24 );
  }

  // Otherwise, big-endian...
  else
  {
    red   = ( ( (*color) & 0x000000FF ) >>  0 );
    green = ( ( (*color) & 0x0000FF00 ) >>  8 );
    blue  = ( ( (*color) & 0x00FF0000 ) >> 16 );
    alpha = ( ( (*color) & 0xFF000000 ) >> 24 );
  }

  // Normalize.
  float32 scale = 1.0f / 255.0f;
  r = scale * red;
  g = scale * green;
  b = scale * blue;
  a = scale * alpha;

  // It worked.
  return true;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the specified vertex attribute.
//
///////////////////////////////////////////////////////////////////////////////

bool LocalVertexPoolRecord::getNormal ( const uint32 &whichVertex, float32 &nx, float32 &ny, float32 &nz ) const
{
  ARGUMENT_CHECK_FOR_GET_FUNCTION ( whichVertex, NORMAL );

  // Offset to the start of the normals.
  float32 *vec = (float32 *) this->_getStartOfAttribute ( whichVertex, _offset.normal );
  if ( NULL == vec ) 
    return false;

  // Set the normal vector and return.
  nx = vec[0];
  ny = vec[1];
  nz = vec[2];
  return true;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the specified vertex attribute.
//
///////////////////////////////////////////////////////////////////////////////

bool LocalVertexPoolRecord::getPosition ( const uint32 &whichVertex, float64 &px, float64 &py, float64 &pz ) const
{
  ARGUMENT_CHECK_FOR_GET_FUNCTION ( whichVertex, POSITION );

  // Offset to the start of the normals.
  float64 *point = (float64 *) this->_getStartOfAttribute ( whichVertex, _offset.position );
  if ( NULL == point ) 
    return false;

  // Set the normal vector and return.
  px = point[0];
  py = point[1];
  pz = point[2];
  return true;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the specified vertex attribute.
//
///////////////////////////////////////////////////////////////////////////////

bool LocalVertexPoolRecord::getUV ( const uint32 &whichVertex, const AttributeMask &whichUV, float32 &u, float32 &v ) const
{
  ARGUMENT_CHECK_FOR_GET_FUNCTION ( whichVertex, (uint32) whichUV );

  // Offset to the start of the normals.
  float32 *uv = (float32 *) this->_getStartOfAttribute ( whichVertex, this->_getOffset ( whichUV ) );
  if ( NULL == uv ) 
    return false;

  // Set the normal vector and return.
  u = uv[0];
  v = uv[1];
  return true;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Getthe offset that corresponds to the given attribute.
//
///////////////////////////////////////////////////////////////////////////////

uint32 LocalVertexPoolRecord::_getOffset ( const AttributeMask &attribute ) const
{
  switch ( attribute )
  {
  case POSITION:    return _offset.position;
  case COLOR_INDEX: return _offset.color;
  case RGB_COLOR:   return _offset.color;
  case NORMAL:      return _offset.normal;
  case BASE_UV:     return _offset.baseUV;
  case UV_1:        return _offset.uv1;
  case UV_2:        return _offset.uv2;
  case UV_3:        return _offset.uv3;
  case UV_4:        return _offset.uv4;
  case UV_5:        return _offset.uv5;
  case UV_6:        return _offset.uv6;
  case UV_7:        return _offset.uv7;
  default:
    assert ( 0 );
    return 0;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Convert the data from big-endian to little-endian.
//
///////////////////////////////////////////////////////////////////////////////

void LocalVertexPoolRecord::endian()
{
  // Should only be in here for little-endian machines.
  assert ( flt::isLittleEndianMachine() );

  // Get a pointer to the vertex pool structure. Note: The members of 
  // SLocalVertexPool have already been endian-corrected in 
  // _initAttributeOffsets().
  SLocalVertexPool *pool = (SLocalVertexPool *) this->getData();

  // Get pointer to start of vertex data.
  char *vertex = this->_getStartOfVertices();

  // Loop through the vertices and byte-swap the data.
  for ( uint32 i = 0; i < pool->numVerts; ++i )
  {
    if ( this->hasAttribute ( POSITION ) )
    {
      // Get the size of the coordinates.
      size_t size = sizeof ( float64 );

      // Get the values.
      float64 *x = (float64 *) vertex;
      float64 *y = &(x[1]);
      float64 *z = &(y[1]);

      // Swap the data.
      flt::swapBytes ( size, x );
      flt::swapBytes ( size, y );
      flt::swapBytes ( size, z );

      // Move pointer to the end of this chunk of data.
      vertex = (char *) ( &(z[1]) );
    }

    if ( this->hasAttribute ( COLOR_INDEX ) || 
         this->hasAttribute ( RGB_COLOR ) )
    {
      // Get the size of the coordinates.
      size_t size = sizeof ( uint32 );

      // Get the value.
      uint32 *color = (uint32 *) vertex;

      // Swap the data.
      flt::swapBytes ( size, color );

      // Move pointer to the end of this chunk of data.
      vertex = (char *) ( &(color[1]) );
    }

    if ( this->hasAttribute ( NORMAL ) )
    {
      // Get the size of the elements.
      size_t size = sizeof ( float32 );

      // Get the values.
      float32 *x = (float32 *) vertex;
      float32 *y = &(x[1]);
      float32 *z = &(y[1]);

      // Swap the data.
      flt::swapBytes ( size, x );
      flt::swapBytes ( size, y );
      flt::swapBytes ( size, z );

      // Move pointer to the end of this chunk of data.
      vertex = (char *) ( &(z[1]) );
    }

    if ( this->hasAttribute ( BASE_UV ) || 
         this->hasAttribute ( UV_1 ) ||
         this->hasAttribute ( UV_2 ) ||
         this->hasAttribute ( UV_3 ) ||
         this->hasAttribute ( UV_4 ) ||
         this->hasAttribute ( UV_5 ) ||
         this->hasAttribute ( UV_6 ) ||
         this->hasAttribute ( UV_7 ) )
    {
      // Get the size of the elements.
      size_t size = sizeof ( float32 );

      // Get the values.
      float32 *u = (float32 *) vertex;
      float32 *v = &(u[1]);

      // Swap the data.
      flt::swapBytes ( size, u );
      flt::swapBytes ( size, v );

      // Move pointer to the end of this chunk of data.
      vertex = (char *) ( &(v[1]) );
    }
  }

  // Should be true. It means that we walked the pointer "vertex" to the 
  // end of the record, but not past it.
  // If equal, vertex pool record was not continued.
  // If 16bit record length is less than length of vertex pool, then the original vertex
  //   pool record was continued with one or more CONTINUATION_OP records.
  assert ( pool->RecHeader._wLength <= ( ( (unsigned long) vertex ) - ( (unsigned long) pool ) ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  The offsets to the start of each attribute depends on the what attributes 
//  are in the vertices. Here we initialize those integer data members.
//
///////////////////////////////////////////////////////////////////////////////

void LocalVertexPoolRecord::_initAttributeOffsets()
{
  // When we get to here we just got done reading the data (i.e., it is still 
  // big-endian). Correct the endian of the members of SLocalVertexPool 
  // because we need the SLocalVertexPool::attributeMask to determine the 
  // offsets (but do not endian-correct the vertex data that follows). The 
  // rest of the data (the vertices) will be endian-corrected in endian().

  // If we are on a little-endian machine...
  if ( flt::isLittleEndianMachine() )
  {
    // Fix these data members.
    SLocalVertexPool *pool = (SLocalVertexPool *) this->getData();
    ENDIAN ( pool->numVerts );
    ENDIAN ( pool->attributeMask );
  }

  // Initialize.
  uint32 current = 0;

  // The order is important because the data will be in this order. We drop 
  // down through here and set the offsets. Each time we update the current
  // position so that we can set the next one.

  if ( this->hasAttribute ( POSITION ) )
  {
    _offset.position = current;
    current += SIZE_POSITION;
  }

  if ( this->hasAttribute ( COLOR_INDEX ) || this->hasAttribute ( RGB_COLOR ) )
  {
    // According to 15.7 manual, we should not have both flags.
    assert ( false == ( this->hasAttribute ( COLOR_INDEX ) && this->hasAttribute ( RGB_COLOR ) ) );

    _offset.color = current;
    current += SIZE_COLOR;
  }

  if ( this->hasAttribute ( NORMAL ) )
  {
    _offset.normal = current;
    current += SIZE_NORMAL;
  }

  if ( this->hasAttribute ( BASE_UV ) )
  {
    _offset.baseUV = current;
    current += SIZE_BASE_UV;
  }

  if ( this->hasAttribute ( UV_1 ) )
  {
    _offset.uv1 = current;
    current += SIZE_UV_1;
  }

  if ( this->hasAttribute ( UV_2 ) )
  {
    _offset.uv2 = current;
    current += SIZE_UV_2;
  }

  if ( this->hasAttribute ( UV_3 ) )
  {
    _offset.uv3 = current;
    current += SIZE_UV_3;
  }

  if ( this->hasAttribute ( UV_4 ) )
  {
    _offset.uv4 = current;
    current += SIZE_UV_4;
  }

  if ( this->hasAttribute ( UV_5 ) )
  {
    _offset.uv5 = current;
    current += SIZE_UV_5;
  }

  if ( this->hasAttribute ( UV_6 ) )
  {
    _offset.uv6 = current;
    current += SIZE_UV_6;
  }

  if ( this->hasAttribute ( UV_7 ) )
  {
    _offset.uv7 = current;
    current += SIZE_UV_7;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Compute the size of each vertex based on its attributes. Cache the
//  result for later reference.
//
///////////////////////////////////////////////////////////////////////////////

int LocalVertexPoolRecord::_getVertexSizeBytes() const
{
  if (_vertexSizeBytesCache == 0)
  {
    if ( hasAttribute ( POSITION ) )
      _vertexSizeBytesCache += SIZE_POSITION;

    if ( hasAttribute ( COLOR_INDEX ) || hasAttribute ( RGB_COLOR ) )
      _vertexSizeBytesCache += SIZE_COLOR;

    if ( hasAttribute ( NORMAL ) )
      _vertexSizeBytesCache += SIZE_NORMAL;

    if ( hasAttribute ( BASE_UV ) )
      _vertexSizeBytesCache += SIZE_BASE_UV;

    if ( hasAttribute ( UV_1 ) )
      _vertexSizeBytesCache += SIZE_UV_1;

    if ( hasAttribute ( UV_2 ) )
      _vertexSizeBytesCache += SIZE_UV_2;

    if ( hasAttribute ( UV_3 ) )
      _vertexSizeBytesCache += SIZE_UV_3;

    if ( hasAttribute ( UV_4 ) )
      _vertexSizeBytesCache += SIZE_UV_4;

    if ( hasAttribute ( UV_5 ) )
      _vertexSizeBytesCache += SIZE_UV_5;

    if ( hasAttribute ( UV_6 ) )
      _vertexSizeBytesCache += SIZE_UV_6;

    if ( hasAttribute ( UV_7 ) )
      _vertexSizeBytesCache += SIZE_UV_7;
  }

  return _vertexSizeBytesCache;
}


///////////////////////////////////////////////////////////////////////////////
//
//  This is called after the record is read.
//
///////////////////////////////////////////////////////////////////////////////

void LocalVertexPoolRecord::postReadInit()
{
  // Initialize the attribute offsets. 
  this->_initAttributeOffsets();

  // Call the base class's function.
  AncillaryRecord::postReadInit();
}


