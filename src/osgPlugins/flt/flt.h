
#ifndef __FLT_H
#define __FLT_H

#include <osg/Vec4>

#ifdef OSG_USE_IO_DOT_H
#include <iostream.h>
#else
#include <iostream>
using namespace std;
#endif

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

////////////////////////////////////////////////////////////////////

typedef signed char     int8;
typedef unsigned char   uint8;
typedef signed short    int16;
typedef unsigned short  uint16;
typedef signed long     int32;
typedef unsigned long   uint32;
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

    friend inline ostream& operator << (ostream& output, const float32x2& f)
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

    friend inline ostream& operator << (ostream& output, const float32x3& f)
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

    friend inline ostream& operator << (ostream& output, const float64x2& f)
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

    friend inline ostream& operator << (ostream& output, const float64x3& f)
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
    inline int length() { return (int)_wLength; }
    void endian() {
        ENDIAN( _wOpcode );
        ENDIAN( _wLength );
    }
};


}; // end namespace flt

#endif


