
#ifndef HEADER_GUARD_FFMPEG_HEADERS_H
#define HEADER_GUARD_FFMPEG_HEADERS_H


extern "C"
{
#define __STDC_CONSTANT_MACROS
#define FF_API_OLD_SAMPLE_FMT 0
#include <errno.h>    // for error codes defined in avformat.h
#include <stdint.h>
#include <avcodec.h>
#include <avformat.h>
#include <avdevice.h>

#ifdef USE_SWSCALE    
    #include <swscale.h>
#endif

}



#endif // HEADER_GUARD_FFMPEG_HEADERS_H
