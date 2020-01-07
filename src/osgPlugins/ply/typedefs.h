/*
    typedefs.h
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
    All rights reserved.

    Type definitions for the mesh classes.
*/

/** note, derived from Equalizer LGPL source.*/


#ifndef MESH_TYPEDEFS_H
#define MESH_TYPEDEFS_H

#   if defined(_MSC_VER)
#      include <winsock2.h>
#      include <windows.h>
#   endif

#   include <osg/Notify>

#ifdef NDEBUG
#   define MESHASSERT( x )
#else
#    define MESHASSERT(x) { if( !(x) ) { OSG_WARN << "Ply Loader ##### Assert: " << #x << " #####" << std::endl; } }
#endif

#   define MESHERROR   OSG_WARN
#   define MESHWARN    OSG_WARN
#   define MESHINFO    OSG_INFO

#if defined(_MSC_VER)
typedef int        socklen_t;

typedef UINT64     uint64_t;
typedef INT64      int64_t;
typedef UINT32     uint32_t;
typedef INT32      int32_t;
typedef UINT16     uint16_t;
typedef UINT8      uint8_t;
#    ifndef HAVE_SSIZE_T
typedef SSIZE_T    ssize_t;
#    endif

#endif // defined(_MSC_VER)

#include <exception>
#include <iostream>
#include <string>

namespace ply
{



    typedef size_t                      Index;
//    typedef unsigned short            ShortIndex;


    // mesh exception
    struct MeshException : public std::exception
    {
        explicit MeshException( const std::string& msg ) : _message( msg ) {}
        virtual ~MeshException() throw() {}
        virtual const char* what() const throw() { return _message.c_str(); }
    private:
        std::string _message;
    };

    // null output stream that discards everything written to it
    struct NullOStream : std::ostream
    {
        struct NullStreamBuf : std::streambuf
        {
            int overflow( int c ) { return traits_type::not_eof( c ); }
        } _nullBuf;

        NullOStream() : std::ios( &_nullBuf ), std::ostream( &_nullBuf ) {}
    };

    // wrapper to enable array use where arrays would not be allowed otherwise
    template< class T, size_t d >
    struct ArrayWrapper
    {
        T& operator[]( const size_t i )
        {
            MESHASSERT( i < d );
            return data[i];
        }

        const T& operator[]( const size_t i ) const
        {
            MESHASSERT( i < d );
            return data[i];
        }

    private:
        T data[d];
    };


    // binary mesh file version, increment if changing the file format
    const unsigned short    FILE_VERSION ( 0x0114 );


    // enumeration for the sort axis
    enum Axis
    {
        AXIS_X,
        AXIS_Y,
        AXIS_Z
    };

}


#endif // MESH_TYPEDEFS_H
