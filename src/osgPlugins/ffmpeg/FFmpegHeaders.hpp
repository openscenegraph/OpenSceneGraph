
#ifndef HEADER_GUARD_FFMPEG_HEADERS_H
#define HEADER_GUARD_FFMPEG_HEADERS_H


extern "C"
{
#define FF_API_OLD_SAMPLE_FMT 0
#include <errno.h>    // for error codes defined in avformat.h
#include <stdint.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#ifndef ANDROID
#include <libavdevice/avdevice.h>
#endif

#include <libavutil/mathematics.h>

#ifdef USE_SWSCALE    
    #include <libswscale/swscale.h>
#endif

#if LIBAVUTIL_VERSION_INT <  AV_VERSION_INT(50,38,0)
#define AV_SAMPLE_FMT_NONE SAMPLE_FMT_NONE
#define AV_SAMPLE_FMT_U8   SAMPLE_FMT_U8
#define AV_SAMPLE_FMT_S16  SAMPLE_FMT_S16
#define AV_SAMPLE_FMT_S32  SAMPLE_FMT_S32
#define AV_SAMPLE_FMT_FLT  SAMPLE_FMT_FLT
#define AV_SAMPLE_FMT_DBL  SAMPLE_FMT_DBL
#define AV_SAMPLE_FMT_NB   SAMPLE_FMT_NB
#endif

}



#endif // HEADER_GUARD_FFMPEG_HEADERS_H
