/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2010 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/
// Written by Wang Rui, (C) 2010

#include <osg/Notify>
#include <osgDB/Registry>
#include <osgDB/Registry>
#include <osgDB/ObjectWrapper>
#include <sstream>

using namespace osgDB;

// Example compressor copying data to/from stream directly
class NullCompressor : public BaseCompressor
{
public:
    NullCompressor() {}

    virtual bool compress( std::ostream& fout, const std::string& src )
    {
        int size = src.size();
        fout.write( (char*)&size, INT_SIZE );
        fout.write( src.c_str(), src.size() );
        return true;
    }

    virtual bool decompress( std::istream& fin, std::string& target )
    {
        int size = 0; fin.read( (char*)&size, INT_SIZE );
        if ( size )
        {
            target.resize( size );
            fin.read( (char*)target.c_str(), size );
        }
        return true;
    }
};

REGISTER_COMPRESSOR( "null", NullCompressor )

#ifdef USE_ZLIB

#include <zlib.h>

#define CHUNK 32768

// ZLib compressor
class ZLibCompressor : public BaseCompressor
{
public:
    ZLibCompressor() {}

    virtual bool compress( std::ostream& fout, const std::string& src )
    {
        int ret, flush = Z_FINISH;
        unsigned have;
        z_stream strm;
        unsigned char out[CHUNK];

        int level = 6;
        int stategy = Z_DEFAULT_STRATEGY;

        /* allocate deflate state */
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        ret = deflateInit2( &strm, level, Z_DEFLATED,
                           15+16, // +16 to use gzip encoding
                           8, // default
                           stategy );
        if ( ret != Z_OK ) return false;

        strm.avail_in = src.size();
        strm.next_in = (Bytef*)( &(*src.begin()) );

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do
        {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */

            if ( ret == Z_STREAM_ERROR )
            {
                OSG_NOTICE << "Z_STREAM_ERROR" << std::endl;
                return false;
            }

            have = CHUNK - strm.avail_out;
            if ( have>0 ) fout.write( (const char*)out, have );

            if ( fout.fail() )
            {
                (void)deflateEnd( &strm );
                return false;
            }
        } while ( strm.avail_out==0 );

        /* clean up and return */
        (void)deflateEnd( &strm );
        return true;
    }

    virtual bool decompress( std::istream& fin, std::string& target )
    {
        int ret;
        unsigned have;
        z_stream strm;
        unsigned char in[CHUNK];
        unsigned char out[CHUNK];

        /* allocate inflate state */
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = 0;
        strm.next_in = Z_NULL;
        ret = inflateInit2( &strm,
                            15 + 32 ); // autodected zlib or gzip header

        if ( ret!=Z_OK )
        {
            OSG_INFO << "failed to init" << std::endl;
            return ret!=0;
        }

        /* decompress until deflate stream ends or end of file */
        do
        {
            fin.read( (char *)in, CHUNK );
            strm.avail_in = fin.gcount();
            if (strm.avail_in==0 ) break;

            /* run inflate() on input until output buffer not full */
            strm.next_in = in;
            do
            {
                strm.avail_out = CHUNK;
                strm.next_out = out;
                ret = inflate( &strm, Z_NO_FLUSH );

                switch (ret)
                {
                case Z_NEED_DICT:
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    (void)inflateEnd( &strm );
                    return false;
                }
                have = CHUNK - strm.avail_out;
                target.append( (char*)out, have );
            } while ( strm.avail_out==0 );

            /* done when inflate() says it's done */
        } while ( ret!=Z_STREAM_END );

        /* clean up and return */
        (void)inflateEnd( &strm );
        return ret==Z_STREAM_END ? true : false;
    }
};

REGISTER_COMPRESSOR( "zlib", ZLibCompressor )

#endif
