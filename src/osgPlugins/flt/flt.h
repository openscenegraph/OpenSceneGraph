
#ifndef __FLT_H
#define __FLT_H

// include osg/Export simply for the Win32 #pragma to stop VisualStudio barffing
// nonesense, other than that there is no need for it...
#include <osg/Export>

#include <osg/Vec4>

#include <iostream>
#include <assert.h>

#if defined(_MSC_VER)
#include <sys/types.h>
#endif

#include <osg/Notify>
#define CERR    osg::notify( osg::INFO ) << __FILE__ << ":" << __LINE__ << ": "
#define CERR2    osg::notify( osg::NOTICE )<< __FILE__ << ":" << __LINE__ << ": "

namespace flt {

#define ENDIAN2(SRC, DST) endian2((void*)&(SRC), sizeof(SRC), (void*)&(DST), sizeof(DST))
#define ENDIAN(A) ENDIAN2((A), (A))

#define BIT0        0x0001
#define BIT1        0x0002
#define BIT2        0x0004
#define BIT3        0x0008
#define BIT4        0x0010
#define BIT5        0x0020
#define BIT6        0x0040
#define BIT7        0x0080
#define BIT8        0x0100
#define BIT9        0x0200
#define BIT10       0x0400
#define BIT11       0x0800
#define BIT12       0x1000
#define BIT13       0x2000
#define BIT14       0x4000
#define BIT15       0x8000

#define BIT16       0x00010000
#define BIT17       0x00020000
#define BIT18       0x00040000
#define BIT19       0x00080000
#define BIT20       0x00100000
#define BIT21       0x00200000
#define BIT22       0x00400000
#define BIT23       0x00800000
#define BIT24       0x01000000
#define BIT25       0x02000000
#define BIT26       0x04000000
#define BIT27       0x08000000
#define BIT28       0x10000000
#define BIT29       0x20000000
#define BIT30       0x40000000
#define BIT31       0x80000000


////////////////////////////////////////////////////////////////////

typedef signed char     int8;
typedef unsigned char   uint8;
typedef signed short    int16;
typedef unsigned short  uint16;
typedef signed int     	int32;	
typedef unsigned int   	uint32;	
typedef float           float32;
typedef double          float64;

////////////////////////////////////////////////////////////////////


extern int  isLittleEndianMachine();
extern void endian2( void* pSrc, int nSrc, void* pDst, int nDst );


struct float32x2
{
    float32 _v[2];

    inline float32 x() { return _v[0]; }
    inline float32 y() { return _v[1]; }
    inline float32 operator [] (int i) { return _v[i]; }
    void endian() {
        ENDIAN( _v[0] );
        ENDIAN( _v[1] );
    }

    friend inline std::ostream& operator << (std::ostream& output, const float32x2& f)
    {
        output << f._v[0] << " "
               << f._v[1];
        return output;     // to enable cascading
    }
};


struct float32x3
{
    float32 _v[3];
    inline float32 x() { return _v[0]; }
    inline float32 y() { return _v[1]; }
    inline float32 z() { return _v[2]; }
    inline float32 operator [] (int i) { return _v[i]; }
    void endian() {
        ENDIAN( _v[0] );
        ENDIAN( _v[1] );
        ENDIAN( _v[2] );
    }

    friend inline std::ostream& operator << (std::ostream& output, const float32x3& f)
    {
        output << f._v[0] << " "
               << f._v[1] << " "
               << f._v[2];
        return output;     // to enable cascading
    }
};


struct float64x2
{
    float64 _v[2];
    inline float64 x() { return _v[0]; }
    inline float64 y() { return _v[1]; }
    inline float64 operator [] (int i) { return _v[i]; }
    void endian() {
        ENDIAN( _v[0] );
        ENDIAN( _v[1] );
    }

    friend inline std::ostream& operator << (std::ostream& output, const float64x2& f)
    {
        output << f._v[0] << " "
               << f._v[1];
        return output;     // to enable cascading
    }
};


struct float64x3
{
    float64 _v[3];

    inline float64 x() { return _v[0]; }
    inline float64 y() { return _v[1]; }
    inline float64 z() { return _v[2]; }
    inline float64 operator [] (int i) { return _v[i]; }
    void endian() {
        ENDIAN( _v[0] );
        ENDIAN( _v[1] );
        ENDIAN( _v[2] );
    }

    friend inline std::ostream& operator << (std::ostream& output, const float64x3& f)
    {
    output << f._v[0] << " "
           << f._v[1] << " "
           << f._v[2];
    return output;     // to enable cascading
    }
};

struct color32
{
    uint8 _alpha;
    uint8 _blue;
    uint8 _green;
    uint8 _red;

    inline float red()      { return (float)_red/255; }
    inline float green()    { return (float)_green/255; }
    inline float blue()     { return (float)_blue/255; }
    inline float alpha()    { return (float)_alpha/255; }
    inline osg::Vec4 get()
    { return osg::Vec4( red(), green(), blue(), alpha()); }
};


struct color48      // version 11, 12 & 13
{
    uint16 _red;
    uint16 _green;
    uint16 _blue;

    inline float red()      { return (float)_red/255; }
    inline float green()    { return (float)_green/255; }
    inline float blue()     { return (float)_blue/255; }
    inline osg::Vec4 get()
    { return osg::Vec4( red(), green(), blue(), 1); }
};

struct SRecHeader
{
    uint16    _wOpcode;            // identifies record type
    uint16    _wLength;            // total length of record

    inline int opcode() { return (int)_wOpcode; }
    inline size_t length() { return (size_t)_wLength; }
    void endian() {
        ENDIAN( _wOpcode );
        ENDIAN( _wLength );
    }
};


//////////////////////////////////////////////////////////////////////////
//
//  Perform a byte swap.
//
//////////////////////////////////////////////////////////////////////////

template<class PointerType> inline void swapBytes ( const size_t &numBytes, PointerType *pointer )
{
  assert ( numBytes >= 2 );
  assert ( pointer );
  flt::endian2 ( (void *) pointer, numBytes, (void *) pointer, numBytes );
}


//////////////////////////////////////////////////////////////////////////
//
//  Perform a byte swap on all elements in the array.
//
//////////////////////////////////////////////////////////////////////////

template<class PointerType, class IndexType> inline void swapBytesArray ( const size_t &numBytes, const IndexType &numElements, PointerType *pointer )
{
  // For the way this function is being used, this should be true. If it 
  // becomes necessary to pass in a "numBytes" that is not actually the number 
  // of bytes at this pointer type, then remove this assertion.
  // I pass in the "numBytes" instead of calculating it here for the purposes 
  // of speeding it up. I didn't test the performance benefits, there may not 
  // be any.
  assert ( numBytes == sizeof ( PointerType ) );

  // Loop through the array and byte-swap the elements.
  for ( IndexType i = 0; i < numElements; ++i )
  {
    // Swap the byte order at the address of i'th element.
    flt::swapBytes ( numBytes, &(pointer[i]) );
  }
}


//////////////////////////////////////////////////////////////////////////
//
//  See if the "bits" are in "number".
//
//////////////////////////////////////////////////////////////////////////

template<class DataTypeNumber, class DataTypeBits> inline bool hasBits 
  ( const DataTypeNumber &number, const DataTypeBits &bits )
{
  return ( ( number & bits ) == bits );
}


}; // end namespace flt


#endif
