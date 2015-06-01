/* -*-c++-*- OpenSceneGraph - Copyright (C) 2009 Tharsis Software
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
 *
 * Authors:
 *         Cedric Pinson <cedric.pinson@plopbyte.net>
*/

#include "DirectShowTexture"
#include <osg/Notify>
#include <osgDB/WriteFile>
#include <sstream>
#include <windows.h>
#include <mmsystem.h>
#include <comdef.h>

#include <strsafe.h>
#include <functional>
#include <locale>
#include <string>

HRESULT GetPin(IBaseFilter* pFilter, LPCWSTR pName, IPin** ppPin);
HRESULT GetPin(IBaseFilter* pFilter, const GUID* pFormat, PIN_DIRECTION PinDir, IPin** ppPin);


struct NamedGuid
{
    const GUID *pguid;
    const TCHAR *psz;
};

// 73646976-0000-0010-8000-00AA00389B71  'vids' == WMMEDIATYPE_Video
EXTERN_GUID(WMMEDIATYPE_Video,
0x73646976, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
// 73647561-0000-0010-8000-00AA00389B71  'auds' == WMMEDIATYPE_Audio
EXTERN_GUID(WMMEDIATYPE_Audio,
0x73647561, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
// 73636d64-0000-0010-8000-00AA00389B71  'scmd' == MEDIATYPE_Script
EXTERN_GUID(WMMEDIATYPE_Script,
0x73636d64, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
// 34A50FD8-8AA5-4386-81FE-A0EFE0488E31            WMMEDIATYPE_Image
EXTERN_GUID(WMMEDIATYPE_Image,
0x34a50fd8, 0x8aa5, 0x4386, 0x81, 0xfe, 0xa0, 0xef, 0xe0, 0x48, 0x8e, 0x31);
// D9E47579-930E-4427-ADFC-AD80F290E470  'fxfr' == WMMEDIATYPE_FileTransfer
EXTERN_GUID(WMMEDIATYPE_FileTransfer,
0xd9e47579, 0x930e, 0x4427, 0xad, 0xfc, 0xad, 0x80, 0xf2, 0x90, 0xe4, 0x70);
// 9BBA1EA7-5AB2-4829-BA57-0940209BCF3E      'text' == WMMEDIATYPE_Text
EXTERN_GUID(WMMEDIATYPE_Text,
0x9bba1ea7, 0x5ab2, 0x4829, 0xba, 0x57, 0x9, 0x40, 0x20, 0x9b, 0xcf, 0x3e);

// 00000000-0000-0010-8000-00AA00389B71            WMMEDIASUBTYPE_Base
EXTERN_GUID(WMMEDIASUBTYPE_Base,
0x00000000, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
// e436eb78-524f-11ce-9f53-0020af0ba770            MEDIASUBTYPE_RGB1
EXTERN_GUID(WMMEDIASUBTYPE_RGB1,
0xe436eb78, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
// e436eb79-524f-11ce-9f53-0020af0ba770            MEDIASUBTYPE_RGB4
EXTERN_GUID(WMMEDIASUBTYPE_RGB4,
0xe436eb79, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
// e436eb7a-524f-11ce-9f53-0020af0ba770            MEDIASUBTYPE_RGB8
EXTERN_GUID(WMMEDIASUBTYPE_RGB8,
0xe436eb7a, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
// e436eb7b-524f-11ce-9f53-0020af0ba770            MEDIASUBTYPE_RGB565
EXTERN_GUID(WMMEDIASUBTYPE_RGB565,
0xe436eb7b, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
// e436eb7c-524f-11ce-9f53-0020af0ba770            MEDIASUBTYPE_RGB555
EXTERN_GUID(WMMEDIASUBTYPE_RGB555,
0xe436eb7c, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
// e436eb7d-524f-11ce-9f53-0020af0ba770            MEDIASUBTYPE_RGB24
EXTERN_GUID(WMMEDIASUBTYPE_RGB24,
0xe436eb7d, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
// e436eb7e-524f-11ce-9f53-0020af0ba770            MEDIASUBTYPE_RGB32
EXTERN_GUID(WMMEDIASUBTYPE_RGB32,
0xe436eb7e, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
// 30323449-0000-0010-8000-00AA00389B71  'YV12' ==  MEDIASUBTYPE_I420
EXTERN_GUID(WMMEDIASUBTYPE_I420,
0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
// 56555949-0000-0010-8000-00AA00389B71  'YV12' ==  MEDIASUBTYPE_IYUV
EXTERN_GUID(WMMEDIASUBTYPE_IYUV,
0x56555949, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
// 31313259-0000-0010-8000-00AA00389B71  'YV12' ==  MEDIASUBTYPE_YV12
EXTERN_GUID(WMMEDIASUBTYPE_YV12,
0x32315659, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
// 32595559-0000-0010-8000-00AA00389B71  'YUY2' == MEDIASUBTYPE_YUY2
EXTERN_GUID(WMMEDIASUBTYPE_YUY2,
0x32595559, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
// 59565955-0000-0010-8000-00AA00389B71  'UYVY' ==  MEDIASUBTYPE_UYVY
EXTERN_GUID(WMMEDIASUBTYPE_UYVY,
0x59565955, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
// 55595659-0000-0010-8000-00AA00389B71  'YVYU' == MEDIASUBTYPE_YVYU
EXTERN_GUID(WMMEDIASUBTYPE_YVYU,
0x55595659, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
// 39555659-0000-0010-8000-00AA00389B71  'YVU9' == MEDIASUBTYPE_YVU9
EXTERN_GUID(WMMEDIASUBTYPE_YVU9,
0x39555659, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
// 3334504D-0000-0010-8000-00AA00389B71            WMMEDIASUBTYPE_MP43
EXTERN_GUID(WMMEDIASUBTYPE_MP43,
0x3334504D, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
// 5334504D-0000-0010-8000-00AA00389B71            WMMEDIASUBTYPE_MP4S
EXTERN_GUID(WMMEDIASUBTYPE_MP4S,
0x5334504D, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
// 31564D57-0000-0010-8000-00AA00389B71            WMMEDIASUBTYPE_WMV1
EXTERN_GUID(WMMEDIASUBTYPE_WMV1,
0x31564D57, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
// 32564D57-0000-0010-8000-00AA00389B71            WMMEDIASUBTYPE_WMV2
EXTERN_GUID(WMMEDIASUBTYPE_WMV2,
0x32564D57, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
// 3153534D-0000-0010-8000-00AA00389B71            WMMEDIASUBTYPE_MSS1
EXTERN_GUID(WMMEDIASUBTYPE_MSS1,
0x3153534D, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
// e06d8026-db46-11cf-b4d1-00805f6cbbea            WMMEDIASUBTYPE_MPEG2_VIDEO
EXTERN_GUID(WMMEDIASUBTYPE_MPEG2_VIDEO,
0xe06d8026, 0xdb46, 0x11cf, 0xb4, 0xd1, 0x00, 0x80, 0x5f, 0x6c, 0xbb, 0xea);
// 00000001-0000-0010-8000-00AA00389B71            WMMEDIASUBTYPE_PCM
EXTERN_GUID(WMMEDIASUBTYPE_PCM,
0x00000001, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
// 00000009-0000-0010-8000-00AA00389B71            WMMEDIASUBTYPE_DRM
EXTERN_GUID(WMMEDIASUBTYPE_DRM,
0x00000009, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
// 00000162-0000-0010-8000-00AA00389B71            WMMEDIASUBTYPE_WMAudioV9
EXTERN_GUID(WMMEDIASUBTYPE_WMAudioV9,
0x00000162, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
// 00000163-0000-0010-8000-00AA00389B71            WMMEDIASUBTYPE_WMAudio_Lossless
EXTERN_GUID(WMMEDIASUBTYPE_WMAudio_Lossless,
0x00000163, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
// 3253534D-0000-0010-8000-00AA00389B71            WMMEDIASUBTYPE_MSS2
EXTERN_GUID(WMMEDIASUBTYPE_MSS2,
0x3253534D, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
// 0000000A-0000-0010-8000-00AA00389B71        WMMEDIASUBTYPE_WMSP1
EXTERN_GUID( WMMEDIASUBTYPE_WMSP1,
0x0000000A,0x0000,0x0010,0x80,0x00,0x00,0xAA,0x00,0x38,0x9B,0x71);
// 33564D57-0000-0010-8000-00AA00389B71            WMMEDIASUBTYPE_WMV3
EXTERN_GUID(WMMEDIASUBTYPE_WMV3,
0x33564D57, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
// 00000161-0000-0010-8000-00AA00389B71            WMMEDIASUBTYPE_WMAudioV8
EXTERN_GUID(WMMEDIASUBTYPE_WMAudioV8,
0x00000161, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
// 00000161-0000-0010-8000-00AA00389B71            WMMEDIASUBTYPE_WMAudioV7
EXTERN_GUID(WMMEDIASUBTYPE_WMAudioV7,
0x00000161, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
// 00000161-0000-0010-8000-00AA00389B71            WMMEDIASUBTYPE_WMAudioV2
EXTERN_GUID(WMMEDIASUBTYPE_WMAudioV2,
0x00000161, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
// 00000130-0000-0010-8000-00AA00389B71            WMMEDIASUBTYPE_ACELPnet
EXTERN_GUID(WMMEDIASUBTYPE_ACELPnet,
0x00000130, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
// 00000050-0000-0010-8000-00AA00389B71            WMMEDIASUBTYPE_MP3
EXTERN_GUID(WMMEDIASUBTYPE_MP3,
0x00000055, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
// 776257d4-c627-41cb-8f81-7ac7ff1c40cc            WMMEDIASUBTYPE_WebStream
EXTERN_GUID(WMMEDIASUBTYPE_WebStream,
0x776257d4, 0xc627, 0x41cb, 0x8f, 0x81, 0x7a, 0xc7, 0xff, 0x1c, 0x40, 0xcc);

// 05589f80-c356-11ce-bf01-00aa0055595a        WMFORMAT_VideoInfo
EXTERN_GUID(WMFORMAT_VideoInfo,
0x05589f80, 0xc356, 0x11ce, 0xbf, 0x01, 0x00, 0xaa, 0x00, 0x55, 0x59, 0x5a);
// 05589f81-c356-11ce-bf01-00aa0055595a        WMFORMAT_WaveFormatEx
EXTERN_GUID(WMFORMAT_WaveFormatEx,
0x05589f81, 0xc356, 0x11ce, 0xbf, 0x01, 0x00, 0xaa, 0x00, 0x55, 0x59, 0x5a);
// 5C8510F2-DEBE-4ca7-BBA5-F07A104F8DFF        WMFORMAT_Script
EXTERN_GUID(WMFORMAT_Script,
0x5c8510f2, 0xdebe, 0x4ca7, 0xbb, 0xa5, 0xf0, 0x7a, 0x10, 0x4f, 0x8d, 0xff);

// 82f38a70-c29f-11d1-97ad-00a0c95ea850        WMSCRIPTTYPE_TwoStrings
EXTERN_GUID( WMSCRIPTTYPE_TwoStrings,
0x82f38a70,0xc29f,0x11d1,0x97,0xad,0x00,0xa0,0xc9,0x5e,0xa8,0x50);

// e06d80e3-db46-11cf-b4d1-00805f6cbbea        WMFORMAT_MPEG2Video
EXTERN_GUID(WMFORMAT_MPEG2Video,
0xe06d80e3, 0xdb46, 0x11cf, 0xb4, 0xd1, 0x00, 0x80, 0x05f, 0x6c, 0xbb, 0xea);

EXTERN_GUID( CLSID_WMMUTEX_Language, 0xD6E22A00,0x35DA,0x11D1,0x90,0x34,0x00,0xA0,0xC9,0x03,0x49,0xBE );
EXTERN_GUID( CLSID_WMMUTEX_Bitrate, 0xD6E22A01,0x35DA,0x11D1,0x90,0x34,0x00,0xA0,0xC9,0x03,0x49,0xBE );
EXTERN_GUID( CLSID_WMMUTEX_Presentation, 0xD6E22A02,0x35DA,0x11D1,0x90,0x34,0x00,0xA0,0xC9,0x03,0x49,0xBE );
EXTERN_GUID( CLSID_WMMUTEX_Unknown, 0xD6E22A03,0x35DA,0x11D1,0x90,0x34,0x00,0xA0,0xC9,0x03,0x49,0xBE );
EXTERN_GUID( CLSID_WMBandwidthSharing_Exclusive, 0xaf6060aa,0x5197,0x11d2,0xb6,0xaf,0x00,0xc0,0x4f,0xd9,0x08,0xe9 );
EXTERN_GUID( CLSID_WMBandwidthSharing_Partial, 0xaf6060ab,0x5197,0x11d2,0xb6,0xaf,0x00,0xc0,0x4f,0xd9,0x08,0xe9 );

const NamedGuid MediaType[]=
{
    {&MEDIASUBTYPE_AIFF, TEXT("AIFF\0")},
    {&MEDIASUBTYPE_AU, TEXT("AU\0")},
    {&MEDIASUBTYPE_AnalogVideo_NTSC_M, TEXT("AnalogVideo_NTSC_M\0")},
    {&MEDIASUBTYPE_AnalogVideo_PAL_B, TEXT("AnalogVideo_PAL_B\0")},
    {&MEDIASUBTYPE_AnalogVideo_PAL_D, TEXT("AnalogVideo_PAL_D\0")},
    {&MEDIASUBTYPE_AnalogVideo_PAL_G, TEXT("AnalogVideo_PAL_G\0")},
    {&MEDIASUBTYPE_AnalogVideo_PAL_H, TEXT("AnalogVideo_PAL_H\0")},
    {&MEDIASUBTYPE_AnalogVideo_PAL_I, TEXT("AnalogVideo_PAL_I\0")},
    {&MEDIASUBTYPE_AnalogVideo_PAL_M, TEXT("AnalogVideo_PAL_M\0")},
    {&MEDIASUBTYPE_AnalogVideo_PAL_N, TEXT("AnalogVideo_PAL_N\0")},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_B, TEXT("AnalogVideo_SECAM_B\0")},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_D, TEXT("AnalogVideo_SECAM_D\0")},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_G, TEXT("AnalogVideo_SECAM_G\0")},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_H, TEXT("AnalogVideo_SECAM_H\0")},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_K, TEXT("AnalogVideo_SECAM_K\0")},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_K1, TEXT("AnalogVideo_SECAM_K1\0")},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_L, TEXT("AnalogVideo_SECAM_L\0")},

    {&MEDIASUBTYPE_ARGB1555, TEXT("ARGB1555\0")},
    {&MEDIASUBTYPE_ARGB4444, TEXT("ARGB4444\0")},
    {&MEDIASUBTYPE_ARGB32, TEXT("ARGB32\0")},
    {&MEDIASUBTYPE_A2R10G10B10, TEXT("A2R10G10B10\0")},
    {&MEDIASUBTYPE_A2B10G10R10, TEXT("A2B10G10R10\0")},

    {&MEDIASUBTYPE_AYUV, TEXT("AYUV\0")},
    {&MEDIASUBTYPE_AI44, TEXT("AI44\0")},
    {&MEDIASUBTYPE_IA44, TEXT("IA44\0")},
    {&MEDIASUBTYPE_NV12, TEXT("NV12\0")},
    {&MEDIASUBTYPE_IMC1, TEXT("IMC1\0")},
    {&MEDIASUBTYPE_IMC2, TEXT("IMC2\0")},
    {&MEDIASUBTYPE_IMC3, TEXT("IMC3\0")},
    {&MEDIASUBTYPE_IMC4, TEXT("IMC4\0")},

    {&MEDIASUBTYPE_Asf, TEXT("Asf\0")},
    {&MEDIASUBTYPE_Avi, TEXT("Avi\0")},
    {&MEDIASUBTYPE_CFCC, TEXT("CFCC\0")},
    {&MEDIASUBTYPE_CLJR, TEXT("CLJR\0")},
    {&MEDIASUBTYPE_CPLA, TEXT("CPLA\0")},
    {&MEDIASUBTYPE_CLPL, TEXT("CLPL\0")},
    {&MEDIASUBTYPE_DOLBY_AC3, TEXT("DOLBY_AC3\0")},
    {&MEDIASUBTYPE_DOLBY_AC3_SPDIF, TEXT("DOLBY_AC3_SPDIF\0")},
    {&MEDIASUBTYPE_DVCS, TEXT("DVCS\0")},
    {&MEDIASUBTYPE_DVD_LPCM_AUDIO, TEXT("DVD_LPCM_AUDIO\0")},
    {&MEDIASUBTYPE_DVD_NAVIGATION_DSI, TEXT("DVD_NAVIGATION_DSI\0")},
    {&MEDIASUBTYPE_DVD_NAVIGATION_PCI, TEXT("DVD_NAVIGATION_PCI\0")},
    {&MEDIASUBTYPE_DVD_NAVIGATION_PROVIDER, TEXT("DVD_NAVIGATION_PROVIDER\0")},
    {&MEDIASUBTYPE_DVD_SUBPICTURE, TEXT("DVD_SUBPICTURE\0")},
    {&MEDIASUBTYPE_DVSD, TEXT("DVSD\0")},
    {&MEDIASUBTYPE_DRM_Audio, TEXT("DRM_Audio\0")},
    {&MEDIASUBTYPE_DssAudio, TEXT("DssAudio\0")},
    {&MEDIASUBTYPE_DssVideo, TEXT("DssVideo\0")},
    {&MEDIASUBTYPE_IF09, TEXT("IF09\0")},
    {&MEDIASUBTYPE_IEEE_FLOAT, TEXT("IEEE_FLOAT\0")},
    {&MEDIASUBTYPE_IJPG, TEXT("IJPG\0")},
    {&MEDIASUBTYPE_IYUV, TEXT("IYUV\0")},
    {&MEDIASUBTYPE_Line21_BytePair, TEXT("Line21_BytePair\0")},
    {&MEDIASUBTYPE_Line21_GOPPacket, TEXT("Line21_GOPPacket\0")},
    {&MEDIASUBTYPE_Line21_VBIRawData, TEXT("Line21_VBIRawData\0")},
    {&MEDIASUBTYPE_MDVF, TEXT("MDVF\0")},
    {&MEDIASUBTYPE_MJPG, TEXT("MJPG\0")},
    {&MEDIASUBTYPE_MPEG1Audio, TEXT("MPEG1Audio\0")},
    {&MEDIASUBTYPE_MPEG1AudioPayload, TEXT("MPEG1AudioPayload\0")},
    {&MEDIASUBTYPE_MPEG1Packet, TEXT("MPEG1Packet\0")},
    {&MEDIASUBTYPE_MPEG1Payload, TEXT("MPEG1Payload\0")},
    {&MEDIASUBTYPE_MPEG1System, TEXT("MPEG1System\0")},
    {&MEDIASUBTYPE_MPEG1Video, TEXT("MPEG1Video\0")},
    {&MEDIASUBTYPE_MPEG1VideoCD, TEXT("MPEG1VideoCD\0")},
    {&MEDIASUBTYPE_MPEG2_AUDIO, TEXT("MPEG2_AUDIO\0")},
    {&MEDIASUBTYPE_MPEG2_PROGRAM, TEXT("MPEG2_PROGRAM\0")},
    {&MEDIASUBTYPE_MPEG2_TRANSPORT, TEXT("MPEG2_TRANSPORT\0")},
    {&MEDIASUBTYPE_MPEG2_VIDEO, TEXT("MPEG2_VIDEO\0")},
    {&MEDIASUBTYPE_None, TEXT("None\0")},
    {&MEDIASUBTYPE_Overlay, TEXT("Overlay\0")},
    {&MEDIASUBTYPE_PCM, TEXT("PCM\0")},
    {&MEDIASUBTYPE_PCMAudio_Obsolete, TEXT("PCMAudio_Obsolete\0")},
    {&MEDIASUBTYPE_Plum, TEXT("Plum\0")},
    {&MEDIASUBTYPE_QTJpeg, TEXT("QTJpeg\0")},
    {&MEDIASUBTYPE_QTMovie, TEXT("QTMovie\0")},
    {&MEDIASUBTYPE_QTRle, TEXT("QTRle\0")},
    {&MEDIASUBTYPE_QTRpza, TEXT("QTRpza\0")},
    {&MEDIASUBTYPE_QTSmc, TEXT("QTSmc\0")},
    {&MEDIASUBTYPE_RAW_SPORT, TEXT("RAW_SPORT\0")},
    {&MEDIASUBTYPE_RGB1, TEXT("RGB1\0")},
    {&MEDIASUBTYPE_RGB24, TEXT("RGB24\0")},
    {&MEDIASUBTYPE_RGB32, TEXT("RGB32\0")},
    {&MEDIASUBTYPE_RGB4, TEXT("RGB4\0")},
    {&MEDIASUBTYPE_RGB555, TEXT("RGB555\0")},
    {&MEDIASUBTYPE_RGB565, TEXT("RGB565\0")},
    {&MEDIASUBTYPE_RGB8, TEXT("RGB8\0")},
    {&MEDIASUBTYPE_SPDIF_TAG_241h, TEXT("SPDIF_TAG_241h\0")},
    {&MEDIASUBTYPE_TELETEXT, TEXT("TELETEXT\0")},
    {&MEDIASUBTYPE_TVMJ, TEXT("TVMJ\0")},
    {&MEDIASUBTYPE_UYVY, TEXT("UYVY\0")},
    {&MEDIASUBTYPE_VPVBI, TEXT("VPVBI\0")},
    {&MEDIASUBTYPE_VPVideo, TEXT("VPVideo\0")},
    {&MEDIASUBTYPE_WAKE, TEXT("WAKE\0")},
    {&MEDIASUBTYPE_WAVE, TEXT("WAVE\0")},
    {&MEDIASUBTYPE_Y211, TEXT("Y211\0")},
    {&MEDIASUBTYPE_Y411, TEXT("Y411\0")},
    {&MEDIASUBTYPE_Y41P, TEXT("Y41P\0")},
    {&MEDIASUBTYPE_YUY2, TEXT("YUY2\0")},
    {&MEDIASUBTYPE_YV12, TEXT("YV12\0")},
    {&MEDIASUBTYPE_YVU9, TEXT("YVU9\0")},
    {&MEDIASUBTYPE_YVYU, TEXT("YVYU\0")},
    {&MEDIASUBTYPE_YUYV, TEXT("YUYV\0")},
    {&MEDIASUBTYPE_dvhd, TEXT("dvhd\0")},
    {&MEDIASUBTYPE_dvsd, TEXT("dvsd\0")},
    {&MEDIASUBTYPE_dvsl, TEXT("dvsl\0")},

    {&MEDIATYPE_AUXLine21Data, TEXT("AUXLine21Data\0")},
    {&MEDIATYPE_AnalogAudio, TEXT("AnalogAudio\0")},
    {&MEDIATYPE_AnalogVideo, TEXT("AnalogVideo\0")},
    {&MEDIATYPE_Audio, TEXT("Audio\0")},
    {&MEDIATYPE_DVD_ENCRYPTED_PACK, TEXT("DVD_ENCRYPTED_PACK\0")},
    {&MEDIATYPE_DVD_NAVIGATION, TEXT("DVD_NAVIGATION\0")},
    {&MEDIATYPE_File, TEXT("File\0")},
    {&MEDIATYPE_Interleaved, TEXT("Interleaved\0")},
    {&MEDIATYPE_LMRT, TEXT("LMRT\0")},
    {&MEDIATYPE_MPEG1SystemStream, TEXT("MPEG1SystemStream\0")},
    {&MEDIATYPE_MPEG2_PES, TEXT("MPEG2_PES\0")},
    {&MEDIATYPE_Midi, TEXT("Midi\0")},
    {&MEDIATYPE_ScriptCommand, TEXT("ScriptCommand\0")},
    {&MEDIATYPE_Stream, TEXT("Stream\0")},
    {&MEDIATYPE_Text, TEXT("Text\0")},
    {&MEDIATYPE_Timecode, TEXT("Timecode\0")},
    {&MEDIATYPE_URL_STREAM, TEXT("URL_STREAM\0")},
    {&MEDIATYPE_VBI, TEXT("VBI\0")},
    {&MEDIATYPE_Video, TEXT("Video\0")},

    {&WMMEDIATYPE_Audio,  TEXT("WMMEDIATYPE_Audio\0")},
    {&WMMEDIATYPE_Video,  TEXT("WMMEDIATYPE_Video\0")},
    {&WMMEDIATYPE_Script, TEXT("WMMEDIATYPE_Script\0")},
    {&WMMEDIATYPE_Image,  TEXT("WMMEDIATYPE_Image\0")},
    {&WMMEDIATYPE_FileTransfer, TEXT("WMMEDIATYPE_FileTransfer\0")},
    {&WMMEDIATYPE_Text,   TEXT("WMMEDIATYPE_Text\0")},

    {&WMMEDIASUBTYPE_Base, TEXT("WMMEDIASUBTYPE_Base\0")},
    {&WMMEDIASUBTYPE_RGB1, TEXT("WMMEDIASUBTYPE_RGB1\0")},
    {&WMMEDIASUBTYPE_RGB4, TEXT("WMMEDIASUBTYPE_RGB4\0")},
    {&WMMEDIASUBTYPE_RGB8, TEXT("WMMEDIASUBTYPE_RGB8\0")},
    {&WMMEDIASUBTYPE_RGB565, TEXT("WMMEDIASUBTYPE_RGB565\0")},
    {&WMMEDIASUBTYPE_RGB555, TEXT("WMMEDIASUBTYPE_RGB555\0")},
    {&WMMEDIASUBTYPE_RGB24, TEXT("WMMEDIASUBTYPE_RGB24\0")},
    {&WMMEDIASUBTYPE_RGB32, TEXT("WMMEDIASUBTYPE_RGB32\0")},
    {&WMMEDIASUBTYPE_I420, TEXT("WMMEDIASUBTYPE_I420\0")},
    {&WMMEDIASUBTYPE_IYUV, TEXT("WMMEDIASUBTYPE_IYUV\0")},
    {&WMMEDIASUBTYPE_YV12, TEXT("WMMEDIASUBTYPE_YV12\0")},
    {&WMMEDIASUBTYPE_YUY2, TEXT("WMMEDIASUBTYPE_YUY2\0")},
    {&WMMEDIASUBTYPE_UYVY, TEXT("WMMEDIASUBTYPE_UYVY\0")},
    {&WMMEDIASUBTYPE_YVYU, TEXT("WMMEDIASUBTYPE_YVYU\0")},
    {&WMMEDIASUBTYPE_YVU9, TEXT("WMMEDIASUBTYPE_YVU9\0")},
    {&WMMEDIASUBTYPE_MP43, TEXT("WMMEDIASUBTYPE_MP43\0")},
    {&WMMEDIASUBTYPE_MP4S, TEXT("WMMEDIASUBTYPE_MP4S\0")},

    {&WMMEDIASUBTYPE_WMV1, TEXT("WMMEDIASUBTYPE_WMV1\0")},
    {&WMMEDIASUBTYPE_WMV2, TEXT("WMMEDIASUBTYPE_WMV2\0")},
    {&WMMEDIASUBTYPE_WMV3, TEXT("WMMEDIASUBTYPE_WMV3\0")},
    {&WMMEDIASUBTYPE_MSS1, TEXT("WMMEDIASUBTYPE_MSS1\0")},
    {&WMMEDIASUBTYPE_MSS2, TEXT("WMMEDIASUBTYPE_MSS2\0")},
    {&WMMEDIASUBTYPE_MPEG2_VIDEO,  TEXT("WMMEDIASUBTYPE_MPEG2_VIDEO\0")},
    {&WMMEDIASUBTYPE_PCM, TEXT("WMMEDIASUBTYPE_PCM\0")},
    {&WMMEDIASUBTYPE_DRM, TEXT("WMMEDIASUBTYPE_DRM\0")},
    {&WMMEDIASUBTYPE_WMAudioV9, TEXT("WMMEDIASUBTYPE_WMAudioV9\0")},
    {&WMMEDIASUBTYPE_WMAudio_Lossless, TEXT("WMMEDIASUBTYPE_WMAudio_Lossless\0")},
    {&WMMEDIASUBTYPE_WMAudioV8, TEXT("WMMEDIASUBTYPE_WMAudioV8\0")},
    {&WMMEDIASUBTYPE_WMAudioV7, TEXT("WMMEDIASUBTYPE_WMAudioV7\0")},
    {&WMMEDIASUBTYPE_WMAudioV2, TEXT("WMMEDIASUBTYPE_WMAudioV2\0")},
    {&WMMEDIASUBTYPE_ACELPnet, TEXT("WMMEDIASUBTYPE_ACELPnet\0")},
    {&WMMEDIASUBTYPE_WMSP1, TEXT("WMMEDIASUBTYPE_WMSP1\0")},

    {&WMFORMAT_VideoInfo,    TEXT("WMFORMAT_VideoInfo\0")},
    {&WMFORMAT_WaveFormatEx, TEXT("WMFORMAT_WaveFormatEx\0")},
    {&WMFORMAT_Script,       TEXT("WMFORMAT_Script\0")},
    {&WMFORMAT_MPEG2Video,   TEXT("WMFORMAT_MPEG2Video\0")},

    {&WMSCRIPTTYPE_TwoStrings, TEXT("WMSCRIPTTYPE_TwoStrings\0")},

    {&PIN_CATEGORY_ANALOGVIDEOIN, TEXT("PIN_CATEGORY_ANALOGVIDEOIN\0")},
    {&PIN_CATEGORY_CAPTURE, TEXT("PIN_CATEGORY_CAPTURE\0")},
    {&PIN_CATEGORY_CC, TEXT("PIN_CATEGORY_CC\0")},
    {&PIN_CATEGORY_EDS, TEXT("PIN_CATEGORY_EDS\0")},
    {&PIN_CATEGORY_NABTS, TEXT("PIN_CATEGORY_NABTS\0")},
    {&PIN_CATEGORY_PREVIEW, TEXT("PIN_CATEGORY_PREVIEW\0")},
    {&PIN_CATEGORY_STILL, TEXT("PIN_CATEGORY_STILL\0")},
    {&PIN_CATEGORY_TELETEXT, TEXT("PIN_CATEGORY_TELETEXT\0")},
    {&PIN_CATEGORY_TIMECODE, TEXT("PIN_CATEGORY_TIMECODE\0")},
    {&PIN_CATEGORY_VBI, TEXT("PIN_CATEGORY_VBI\0")},
    {&PIN_CATEGORY_VIDEOPORT, TEXT("PIN_CATEGORY_VIDEOPORT\0")},
    {&PIN_CATEGORY_VIDEOPORT_VBI, TEXT("PIN_CATEGORY_VIDEOPORT_VBI\0")},

    {&CLSID_ACMWrapper, TEXT("CLSID_ACMWrapper\0")},
    {&CLSID_AVICo, TEXT("CLSID_AVICo\0")},
    {&CLSID_AVIDec, TEXT("CLSID_AVIDec\0")},
    {&CLSID_AVIDoc, TEXT("CLSID_AVIDoc\0")},
    {&CLSID_AVIDraw, TEXT("CLSID_AVIDraw\0")},
    {&CLSID_AVIMIDIRender, TEXT("CLSID_AVIMIDIRender\0")},
    {&CLSID_ActiveMovieCategories, TEXT("CLSID_ActiveMovieCategories\0")},
    {&CLSID_AnalogVideoDecoderPropertyPage, TEXT("CLSID_AnalogVideoDecoderPropertyPage\0")},
    {&CLSID_WMAsfReader, TEXT("CLSID_WMAsfReader\0")},
    {&CLSID_WMAsfWriter, TEXT("CLSID_WMAsfWriter\0")},
    {&CLSID_AsyncReader, TEXT("CLSID_AsyncReader\0")},
    {&CLSID_AudioCompressorCategory, TEXT("CLSID_AudioCompressorCategory\0")},
    {&CLSID_AudioInputDeviceCategory, TEXT("CLSID_AudioInputDeviceCategory\0")},
    {&CLSID_AudioProperties, TEXT("CLSID_AudioProperties\0")},
    {&CLSID_AudioRecord, TEXT("CLSID_AudioRecord\0")},
    {&CLSID_AudioRender, TEXT("CLSID_AudioRender\0")},
    {&CLSID_AudioRendererCategory, TEXT("CLSID_AudioRendererCategory\0")},
    {&CLSID_AviDest, TEXT("CLSID_AviDest\0")},
    {&CLSID_AviMuxProptyPage, TEXT("CLSID_AviMuxProptyPage\0")},
    {&CLSID_AviMuxProptyPage1, TEXT("CLSID_AviMuxProptyPage1\0")},
    {&CLSID_AviReader, TEXT("CLSID_AviReader\0")},
    {&CLSID_AviSplitter, TEXT("CLSID_AviSplitter\0")},
    {&CLSID_CAcmCoClassManager, TEXT("CLSID_CAcmCoClassManager\0")},
    {&CLSID_CDeviceMoniker, TEXT("CLSID_CDeviceMoniker\0")},
    {&CLSID_CIcmCoClassManager, TEXT("CLSID_CIcmCoClassManager\0")},
    {&CLSID_CMidiOutClassManager, TEXT("CLSID_CMidiOutClassManager\0")},
    {&CLSID_CMpegAudioCodec, TEXT("CLSID_CMpegAudioCodec\0")},
    {&CLSID_CMpegVideoCodec, TEXT("CLSID_CMpegVideoCodec\0")},
    {&CLSID_CQzFilterClassManager, TEXT("CLSID_CQzFilterClassManager\0")},
    {&CLSID_CVidCapClassManager, TEXT("CLSID_CVidCapClassManager\0")},
    {&CLSID_CWaveOutClassManager, TEXT("CLSID_CWaveOutClassManager\0")},
    {&CLSID_CWaveinClassManager, TEXT("CLSID_CWaveinClassManager\0")},
    {&CLSID_CameraControlPropertyPage, TEXT("CLSID_CameraControlPropertyPage\0")},
    {&CLSID_CaptureGraphBuilder, TEXT("CLSID_CaptureGraphBuilder\0")},
    {&CLSID_CaptureProperties, TEXT("CLSID_CaptureProperties\0")},
    {&CLSID_Colour, TEXT("CLSID_Colour\0")},
    {&CLSID_CrossbarFilterPropertyPage, TEXT("CLSID_CrossbarFilterPropertyPage\0")},
    {&CLSID_DSoundRender, TEXT("CLSID_DSoundRender\0")},
    {&CLSID_DVDHWDecodersCategory, TEXT("CLSID_DVDHWDecodersCategory\0")},
    {&CLSID_DVDNavigator, TEXT("CLSID_DVDNavigator\0")},
    {&CLSID_DVDecPropertiesPage, TEXT("CLSID_DVDecPropertiesPage\0")},
    {&CLSID_DVEncPropertiesPage, TEXT("CLSID_DVEncPropertiesPage\0")},
    {&CLSID_DVMux, TEXT("CLSID_DVMux\0")},
    {&CLSID_DVMuxPropertyPage, TEXT("CLSID_DVMuxPropertyPage\0")},
    {&CLSID_DVSplitter, TEXT("CLSID_DVSplitter\0")},
    {&CLSID_DVVideoCodec, TEXT("CLSID_DVVideoCodec\0")},
    {&CLSID_DVVideoEnc, TEXT("CLSID_DVVideoEnc\0")},
    {&CLSID_DirectDraw, TEXT("CLSID_DirectDraw\0")},
    {&CLSID_DirectDrawClipper, TEXT("CLSID_DirectDrawClipper\0")},
    {&CLSID_DirectDrawProperties, TEXT("CLSID_DirectDrawProperties\0")},
    {&CLSID_Dither, TEXT("CLSID_Dither\0")},
    {&CLSID_DvdGraphBuilder, TEXT("CLSID_DvdGraphBuilder\0")},
    {&CLSID_FGControl, TEXT("CLSID_FGControl\0")},
    {&CLSID_FileSource, TEXT("CLSID_FileSource\0")},
    {&CLSID_FileWriter, TEXT("CLSID_FileWriter\0")},
    {&CLSID_FilterGraph, TEXT("CLSID_FilterGraph\0")},
    {&CLSID_FilterGraphNoThread, TEXT("CLSID_FilterGraphNoThread\0")},
    {&CLSID_FilterMapper, TEXT("CLSID_FilterMapper\0")},
    {&CLSID_FilterMapper2, TEXT("CLSID_FilterMapper2\0")},
    {&CLSID_InfTee, TEXT("CLSID_InfTee\0")},
    {&CLSID_LegacyAmFilterCategory, TEXT("CLSID_LegacyAmFilterCategory\0")},
    {&CLSID_Line21Decoder, TEXT("CLSID_Line21Decoder\0")},
    {&CLSID_MOVReader, TEXT("CLSID_MOVReader\0")},
    {&CLSID_MPEG1Doc, TEXT("CLSID_MPEG1Doc\0")},
    {&CLSID_MPEG1PacketPlayer, TEXT("CLSID_MPEG1PacketPlayer\0")},
    {&CLSID_MPEG1Splitter, TEXT("CLSID_MPEG1Splitter\0")},
    {&CLSID_MediaPropertyBag, TEXT("CLSID_MediaPropertyBag\0")},
    {&CLSID_MemoryAllocator, TEXT("CLSID_MemoryAllocator\0")},
    {&CLSID_MidiRendererCategory, TEXT("CLSID_MidiRendererCategory\0")},
    {&CLSID_ModexProperties, TEXT("CLSID_ModexProperties\0")},
    {&CLSID_ModexRenderer, TEXT("CLSID_ModexRenderer\0")},
    {&CLSID_OverlayMixer, TEXT("CLSID_OverlayMixer\0")},
    {&CLSID_PerformanceProperties, TEXT("CLSID_PerformanceProperties\0")},
    {&CLSID_PersistMonikerPID, TEXT("CLSID_PersistMonikerPID\0")},
    {&CLSID_ProtoFilterGraph, TEXT("CLSID_ProtoFilterGraph\0")},
    {&CLSID_QualityProperties, TEXT("CLSID_QualityProperties\0")},
    {&CLSID_SeekingPassThru, TEXT("CLSID_SeekingPassThru\0")},
    {&CLSID_SmartTee, TEXT("CLSID_SmartTee\0")},
    {&CLSID_SystemClock, TEXT("CLSID_SystemClock\0")},
    {&CLSID_SystemDeviceEnum, TEXT("CLSID_SystemDeviceEnum\0")},
    {&CLSID_TVAudioFilterPropertyPage, TEXT("CLSID_TVAudioFilterPropertyPage\0")},
    {&CLSID_TVTunerFilterPropertyPage, TEXT("CLSID_TVTunerFilterPropertyPage\0")},
    {&CLSID_TextRender, TEXT("CLSID_TextRender\0")},
    {&CLSID_URLReader, TEXT("CLSID_URLReader\0")},
    {&CLSID_VBISurfaces, TEXT("CLSID_VBISurfaces\0")},
    {&CLSID_VPObject, TEXT("CLSID_VPObject\0")},
    {&CLSID_VPVBIObject, TEXT("CLSID_VPVBIObject\0")},
    {&CLSID_VfwCapture, TEXT("CLSID_VfwCapture\0")},
    {&CLSID_VideoCompressorCategory, TEXT("CLSID_VideoCompressorCategory\0")},
    {&CLSID_VideoInputDeviceCategory, TEXT("CLSID_VideoInputDeviceCategory\0")},
    {&CLSID_VideoProcAmpPropertyPage, TEXT("CLSID_VideoProcAmpPropertyPage\0")},
    {&CLSID_VideoRenderer, TEXT("CLSID_VideoRenderer\0")},
    {&CLSID_VideoStreamConfigPropertyPage, TEXT("CLSID_VideoStreamConfigPropertyPage\0")},

    {&CLSID_WMMUTEX_Language,      TEXT("CLSID_WMMUTEX_Language\0")},
    {&CLSID_WMMUTEX_Bitrate,       TEXT("CLSID_WMMUTEX_Bitrate\0")},
    {&CLSID_WMMUTEX_Presentation,  TEXT("CLSID_WMMUTEX_Presentation\0")},
    {&CLSID_WMMUTEX_Unknown,       TEXT("CLSID_WMMUTEX_Unknown\0")},

    {&CLSID_WMBandwidthSharing_Exclusive, TEXT("CLSID_WMBandwidthSharing_Exclusive\0")},
    {&CLSID_WMBandwidthSharing_Partial,   TEXT("CLSID_WMBandwidthSharing_Partial\0")},

    {&FORMAT_AnalogVideo, TEXT("FORMAT_AnalogVideo\0")},
    {&FORMAT_DVD_LPCMAudio, TEXT("FORMAT_DVD_LPCMAudio\0")},
    {&FORMAT_DolbyAC3, TEXT("FORMAT_DolbyAC3\0")},
    {&FORMAT_DvInfo, TEXT("FORMAT_DvInfo\0")},
    {&FORMAT_MPEG2Audio, TEXT("FORMAT_MPEG2Audio\0")},
    {&FORMAT_MPEG2Video, TEXT("FORMAT_MPEG2Video\0")},
    {&FORMAT_MPEG2_VIDEO, TEXT("FORMAT_MPEG2_VIDEO\0")},
    {&FORMAT_MPEGStreams, TEXT("FORMAT_MPEGStreams\0")},
    {&FORMAT_MPEGVideo, TEXT("FORMAT_MPEGVideo\0")},
    {&FORMAT_None, TEXT("FORMAT_None\0")},
    {&FORMAT_VIDEOINFO2, TEXT("FORMAT_VIDEOINFO2\0")},
    {&FORMAT_VideoInfo, TEXT("FORMAT_VideoInfo\0")},
    {&FORMAT_VideoInfo2, TEXT("FORMAT_VideoInfo2\0")},
    {&FORMAT_WaveFormatEx, TEXT("FORMAT_WaveFormatEx\0")},

    {&TIME_FORMAT_BYTE, TEXT("TIME_FORMAT_BYTE\0")},
    {&TIME_FORMAT_FIELD, TEXT("TIME_FORMAT_FIELD\0")},
    {&TIME_FORMAT_FRAME, TEXT("TIME_FORMAT_FRAME\0")},
    {&TIME_FORMAT_MEDIA_TIME, TEXT("TIME_FORMAT_MEDIA_TIME\0")},
    {&TIME_FORMAT_SAMPLE, TEXT("TIME_FORMAT_SAMPLE\0")},

    {&AMPROPSETID_Pin, TEXT("AMPROPSETID_Pin\0")},
    {&AM_INTERFACESETID_Standard, TEXT("AM_INTERFACESETID_Standard\0")},
    {&AM_KSCATEGORY_AUDIO, TEXT("AM_KSCATEGORY_AUDIO\0")},
    {&AM_KSCATEGORY_CAPTURE, TEXT("AM_KSCATEGORY_CAPTURE\0")},
    {&AM_KSCATEGORY_CROSSBAR, TEXT("AM_KSCATEGORY_CROSSBAR\0")},
    {&AM_KSCATEGORY_DATACOMPRESSOR, TEXT("AM_KSCATEGORY_DATACOMPRESSOR\0")},
    {&AM_KSCATEGORY_RENDER, TEXT("AM_KSCATEGORY_RENDER\0")},
    {&AM_KSCATEGORY_TVAUDIO, TEXT("AM_KSCATEGORY_TVAUDIO\0")},
    {&AM_KSCATEGORY_TVTUNER, TEXT("AM_KSCATEGORY_TVTUNER\0")},
    {&AM_KSCATEGORY_VIDEO, TEXT("AM_KSCATEGORY_VIDEO\0")},
    {&AM_KSPROPSETID_AC3, TEXT("AM_KSPROPSETID_AC3\0")},
    {&AM_KSPROPSETID_CopyProt, TEXT("AM_KSPROPSETID_CopyProt\0")},
    {&AM_KSPROPSETID_DvdSubPic, TEXT("AM_KSPROPSETID_DvdSubPic\0")},
    {&AM_KSPROPSETID_TSRateChange, TEXT("AM_KSPROPSETID_TSRateChange\0")},

    {&IID_IAMDirectSound, TEXT("IID_IAMDirectSound\0")},
    {&IID_IAMLine21Decoder, TEXT("IID_IAMLine21Decoder\0")},
    {&IID_IBaseVideoMixer, TEXT("IID_IBaseVideoMixer\0")},
    {&IID_IDDVideoPortContainer, TEXT("IID_IDDVideoPortContainer\0")},
    {&IID_IDirectDraw, TEXT("IID_IDirectDraw\0")},
    {&IID_IDirectDraw2, TEXT("IID_IDirectDraw2\0")},
    {&IID_IDirectDrawClipper, TEXT("IID_IDirectDrawClipper\0")},
    {&IID_IDirectDrawColorControl, TEXT("IID_IDirectDrawColorControl\0")},
    {&IID_IDirectDrawKernel, TEXT("IID_IDirectDrawKernel\0")},
    {&IID_IDirectDrawPalette, TEXT("IID_IDirectDrawPalette\0")},
    {&IID_IDirectDrawSurface, TEXT("IID_IDirectDrawSurface\0")},
    {&IID_IDirectDrawSurface2, TEXT("IID_IDirectDrawSurface2\0")},
    {&IID_IDirectDrawSurface3, TEXT("IID_IDirectDrawSurface3\0")},
    {&IID_IDirectDrawSurfaceKernel, TEXT("IID_IDirectDrawSurfaceKernel\0")},
    {&IID_IDirectDrawVideo, TEXT("IID_IDirectDrawVideo\0")},
    {&IID_IFullScreenVideo, TEXT("IID_IFullScreenVideo\0")},
    {&IID_IFullScreenVideoEx, TEXT("IID_IFullScreenVideoEx\0")},
    {&IID_IKsDataTypeHandler, TEXT("IID_IKsDataTypeHandler\0")},
    {&IID_IKsInterfaceHandler, TEXT("IID_IKsInterfaceHandler\0")},
    {&IID_IKsPin, TEXT("IID_IKsPin\0")},
    {&IID_IMixerPinConfig, TEXT("IID_IMixerPinConfig\0")},
    {&IID_IMixerPinConfig2, TEXT("IID_IMixerPinConfig2\0")},
    {&IID_IMpegAudioDecoder, TEXT("IID_IMpegAudioDecoder\0")},
    {&IID_IQualProp, TEXT("IID_IQualProp\0")},
    {&IID_IVPConfig, TEXT("IID_IVPConfig\0")},
    {&IID_IVPControl, TEXT("IID_IVPControl\0")},
    {&IID_IVPNotify, TEXT("IID_IVPNotify\0")},
    {&IID_IVPNotify2, TEXT("IID_IVPNotify2\0")},
    {&IID_IVPObject, TEXT("IID_IVPObject\0")},
    {&IID_IVPVBIConfig, TEXT("IID_IVPVBIConfig\0")},
    {&IID_IVPVBINotify, TEXT("IID_IVPVBINotify\0")},
    {&IID_IVPVBIObject, TEXT("IID_IVPVBIObject\0")},

    {&LOOK_DOWNSTREAM_ONLY, TEXT("LOOK_DOWNSTREAM_ONLY\0")},
    {&LOOK_UPSTREAM_ONLY, TEXT("LOOK_UPSTREAM_ONLY\0")},
    {0, 0},
};

//-----------------------------------------------------------------------------
// GetPin
// Find the pin of the specified format type on the given filter
// This method leaves an outstanding reference on the pin if successful
HRESULT GetPin(IBaseFilter* pFilter, const GUID* pFormat, PIN_DIRECTION PinDir, IPin** ppPin)
{
    HRESULT hr = S_OK;

    if (pFilter && pFormat && ppPin)
    {
        IEnumPins* pIEnumPins = NULL;
        hr = pFilter->EnumPins(&pIEnumPins);
        if (SUCCEEDED(hr))
        {
            // find the pin with the specified format
            IPin* pIPin = NULL;
            while (S_OK == pIEnumPins->Next(1, &pIPin, NULL))
            {
                // match the pin direction
                PIN_DIRECTION pinDir;
                pIPin->QueryDirection(&pinDir);
                if (pinDir == PinDir)
                {
                    // match pin direction check the first media type returned from the upstream pin
                    IEnumMediaTypes* pIEnumMT = NULL;
                    hr = pIPin->EnumMediaTypes(&pIEnumMT);
                    if (SUCCEEDED(hr))
                    {
                        AM_MEDIA_TYPE* pmt = NULL;
                        hr = pIEnumMT->Next(1, &pmt, NULL);
                        if (S_OK == hr)
                        {
                            if (pmt->majortype == *pFormat)
                            {
                                // found the pin with the specified format
                                *ppPin = pIPin;
                                DeleteMediaType(pmt);
                                break;
                            }
                            else
                            {
                                DeleteMediaType(pmt);
                            }
                        }
                        pIEnumMT->Release();
                    }
                }
                if (pIPin) pIPin->Release(); pIPin = 0;
            }

            if (NULL == *ppPin)
            {
                // failed to find the named pin
                hr = E_FAIL;
            }
        pIEnumPins->Release();
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}
//-----------------------------------------------------------------------------
// GetPin
// Find the pin of the specified name on the given filter
// This method leaves an outstanding reference on the pin if successful
HRESULT GetPin(IBaseFilter* pFilter, LPCWSTR pName, IPin** ppPin)
{
    HRESULT hr = S_OK;

    if (pFilter && pName && ppPin)
    {
        IEnumPins* pIEnumPins = NULL;
        hr = pFilter->EnumPins(&pIEnumPins);
        if (SUCCEEDED(hr))
        {
            IPin* pIPin = NULL;
            while (S_OK == pIEnumPins->Next(1, &pIPin, NULL))
            {
                PIN_INFO info = {0};
                hr = pIPin->QueryPinInfo(&info);
                if (SUCCEEDED(hr))
                {
                    if (info.pFilter) info.pFilter->Release(); info.pFilter = 0;

                    if (0 == wcsncmp(info.achName, pName, wcslen(pName)))
                    {
                        // matched the pin category
                        *ppPin = pIPin;
                        break;
                    }
                }
                if (pIPin) pIPin->Release(); pIPin = 0;
            }
            pIEnumPins->Release();
        }

        if (NULL == *ppPin)
        {
            // failed to find the named pin
            hr = E_FAIL;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

//-----------------------------------------------------------------------------
// FindPinInterface
// Attempt to locate the interface on the pin with the specified format or on the first pin if no
// format is provided.
HRESULT FindPinInterface(IBaseFilter* pFilter, const GUID* pFormat, PIN_DIRECTION PinDir, const IID& riid, void** ppvInterface)
{
    HRESULT hr = S_OK;

    if (pFilter && ppvInterface)
    {
        IPin* pIPin = NULL;
        if (pFormat)
        {
            hr = GetPin(pFilter, pFormat, PinDir, &pIPin);
        }
        else
        {
            IEnumPins* pIEnumPins = NULL;
            hr = pFilter->EnumPins(&pIEnumPins);
            if (SUCCEEDED(hr))
            {
                hr = pIEnumPins->Next(1, &pIPin, NULL);
            }
            pIEnumPins->Release();
        }

        if (SUCCEEDED(hr))
        {
            hr = pIPin->QueryInterface(riid, ppvInterface);
        }
        pIPin->Release();
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

std::string getStringFromGUID(const GUID *pGUID)
{
    int i=0;
    // Find format GUID's name in the named guids table
    while (MediaType[i].pguid != 0)
    {
        if(*pGUID == *(MediaType[i].pguid))
        {
            return std::string(MediaType[i].psz);
        }
        i++;
    }

    // return the guid if does not recognize the type
    const int maxChar = 60;
    LPOLESTR pwszClsid = (LPOLESTR) CoTaskMemAlloc(maxChar*2);
    CHAR  szCLSID[maxChar];
    int nchar = StringFromGUID2(*pGUID, pwszClsid, maxChar);
    // Convert result to ANSI
    WideCharToMultiByte(CP_ACP, 0, pwszClsid, -1, szCLSID, nchar, NULL, NULL);
    CoTaskMemFree(pwszClsid);
    return std::string(szCLSID);
}
const GUID* getGUIDFromString(const std::string& str)
{
    int i=0;
    // Find format GUID's name in the named guids table
    while (MediaType[i].pguid != 0)
    {
        if(str == std::string((MediaType[i].psz)))
            return MediaType[i].pguid;
        i++;
    }
    return 0;
}
static std::string getErrorMessage(HRESULT hr)
{
    if (FAILED(hr))
    {
        TCHAR szErr[MAX_ERROR_TEXT_LEN];
        DWORD res = AMGetErrorText(hr, szErr, MAX_ERROR_TEXT_LEN);
        if (res == 0)
        {
            StringCchPrintf(szErr, MAX_ERROR_TEXT_LEN, "Unknown Error: 0x%2x", hr);
        }
        return std::string (szErr);
    }
    return std::string("");
}

static bool checkError(const std::string& prefix, HRESULT hr)
{
    if (FAILED(hr))
    {
        OSG_WARN << prefix << " " << getErrorMessage(hr) << std::endl;
    }

    if (hr == E_ABORT)
        return false;
    if (hr == E_FAIL)
        return false;
    if (hr == E_OUTOFMEMORY)
        return false;
    if (hr == E_POINTER)
        return false;
    if (hr == VFW_E_CANNOT_CONNECT)
        return false;
    if (hr == 0x80040256)
        return false;
    if (hr == 0x80040216)
        return false;

    return true;
}

bool CTextureRenderer::initBuildGraph()
{
    std::string prefixForMessage;
    {
    std::stringstream ss;
    ss << _imageStream.get();
    prefixForMessage = ss.str();
    }

    HRESULT hr;
    hr = CoCreateInstance( CLSID_FilterGraph, 0, CLSCTX_INPROC,IID_IGraphBuilder, (void **)&_graphBuilder );
    if (!checkError(prefixForMessage, hr))
        return false;

    hr = _graphBuilder->QueryInterface( IID_IMediaControl, (void **)&_mediaControl );
    if (!checkError(prefixForMessage, hr))
        return false;

    hr = _graphBuilder->QueryInterface( IID_IMediaSeeking, (void **)&_mediaSeeking );
    if (!checkError(prefixForMessage, hr))
        return false;

    hr = _graphBuilder->AddFilter((IBaseFilter*)this, L"Sampler");
    if (!checkError(prefixForMessage, hr))
        return false;


    hr = _graphBuilder->QueryInterface( IID_IBasicAudio, (void **)&_basicAudio);
    checkError(prefixForMessage, hr); //May be no sound so dont effect return result

    return true;
}


struct ListDeviceAvailable
{
    struct DeviceEntry
    {
        std::string _name;
        std::string _clsid;
        IMoniker* _device;
        DeviceEntry(const std::string& name = "" , const std::string& clsid = "", IMoniker* device = 0) : _name(name), _clsid(clsid), _device(device) {}
    };
    std::vector<DeviceEntry> _listDevice;
    IEnumMoniker* _enumMoniker;
    ListDeviceAvailable(IEnumMoniker *enumMoniker) : _enumMoniker(enumMoniker)
    {
        createList();
    }
    ~ListDeviceAvailable()
    {
        for (int i = 0; i < (int)_listDevice.size(); i++)
            if (_listDevice[i]._device)
                _listDevice[i]._device->Release();
    }

    void displayDevicesFound(const std::string& prefixForMessage, osg::NotifySeverity serverity = osg::NOTICE) const
    {
        for (int i = 0; i < (int)_listDevice.size(); i++)
        {
            OSG_NOTIFY(serverity) << prefixForMessage << " device \"" << _listDevice[i]._name << "\" clsid " << _listDevice[i]._clsid << std::endl;
        }
    }

    DeviceEntry getDevice(const std::string& name)
    {
        for (int i = 0; i < (int)_listDevice.size(); i++)
            if (_listDevice[i]._name == name)
                return _listDevice[i];
        //if (!_listDevice.empty())
        //    return _listDevice.front();

        int deviceId = atoi(name.c_str());
        if(deviceId >= 0 && deviceId < (int)_listDevice.size())
          return _listDevice[deviceId];

        return DeviceEntry();
    }

    void createList()
    {
        IMoniker *device = NULL;
        HRESULT hr;
        // Enumerate all items associated with the moniker
        while (_enumMoniker->Next(1, &device, NULL) == S_OK)
        {
            IPropertyBag *pPropBag = NULL;
            CLSID clsidFilter;

            VARIANT varName;
            VARIANT varFilterClsid;

            VariantInit(&varName);
            VariantInit(&varFilterClsid);

            // Associate moniker with a file
            hr = device->BindToStorage(0, 0, IID_IPropertyBag,
                                         (void **)&pPropBag);

            // Read filter name from property bag
            if (SUCCEEDED(hr))
            {
                hr = pPropBag->Read(L"FriendlyName", &varName, 0);
            }

            // Read filter's CLSID from property bag.  This CLSID string will be
            // converted to a binary CLSID and passed to AddFilter(), which will
            // add the filter's name to the listbox and its CLSID to the listbox
            // item's DataPtr item.  When the user clicks on a filter name in
            // the listbox, we'll read the stored CLSID, convert it to a string,
            // and use it to find the filter's filename in the registry.

            if (SUCCEEDED(hr))
            {
                // Read CLSID string from property bag
                hr = pPropBag->Read(L"CLSID", &varFilterClsid, 0);

                // Add filter name and CLSID to listbox
                if (SUCCEEDED(hr))
                {
                    hr = CLSIDFromString(varFilterClsid.bstrVal, &clsidFilter);
                }
                else if (hr == E_PROP_ID_UNSUPPORTED)
                {
                    clsidFilter = GUID_NULL; // No CLSID is listed.
                    hr = S_OK;
                }
            }

            // covert to std::string
            _bstr_t bstr_t(varName.bstrVal);
            std::string deviceName(bstr_t);
            std::string deviceGUID;

            // Add filter name and filename to list
            if(SUCCEEDED(hr))
            {
                LPOLESTR pwszClsid;
                CHAR  szCLSID[60];
                hr = StringFromCLSID(clsidFilter, &pwszClsid);
                if (!FAILED(hr))
                {
                    // Convert result to ANSI
                    WideCharToMultiByte(CP_ACP, 0, pwszClsid, -1, szCLSID, 60, NULL, NULL);
                    //OSG_NOTICE << "device \"" << truename << "\" id " << szCLSID << std::endl;
                    deviceGUID = std::string(szCLSID);
                }
                CoTaskMemFree(pwszClsid);
            }

            VariantClear(&varName);
            VariantClear(&varFilterClsid);

            // Cleanup interfaces
            if (pPropBag) pPropBag->Release();

            _listDevice.push_back(DeviceEntry(deviceName, deviceGUID, device));
        }
    }


};


struct ListCapDeviceAvailable
{
    typedef std::pair<AM_MEDIA_TYPE*, VIDEOINFOHEADER *> CapEntry;
    std::vector<CapEntry> _capsList;
    IAMStreamConfig* _config;
    ListCapDeviceAvailable(IAMStreamConfig* config) : _config(config)
    {
        createList();
    }
    ~ListCapDeviceAvailable()
    {
        for (int i = 0; i < (int)_capsList.size(); i++)
            if (_capsList[i].first)
                DeleteMediaType(_capsList[i].first);
    }

    void displayCapsFound(const std::string& prefixForMessage = "") const
    {
        for (int i = 0; i < (int)_capsList.size(); i++)
        {
            VIDEOINFOHEADER* video= _capsList[i].second;
            displayCap(video, prefixForMessage);
        }
    }

    static void displayCap(VIDEOINFOHEADER* video, const std::string& prefix = "")
    {
        if (!video)
            return;
        double fps = 1.0/ (video->AvgTimePerFrame * 100.0 * 1e-9);
        FOURCCMap fccMap(video->bmiHeader.biCompression);
        GUID g1 = (GUID)fccMap;
        OSG_NOTICE << prefix << " cap " << video->bmiHeader.biWidth << " x " << video->bmiHeader.biHeight << " bit per pixel " << video->bmiHeader.biBitCount << " (" << getStringFromGUID(&g1) << ") at " << fps << " fps" << std::endl;
    }

    std::pair<AM_MEDIA_TYPE*, VIDEOINFOHEADER *> getCaps(int width, int height, double fps)
    {
        std::vector<CapEntry> filterResolution;
        for (int i = 0; i < (int)_capsList.size(); i++)
        {
            VIDEOINFOHEADER* video= _capsList[i].second;
            if (video->bmiHeader.biWidth == width && video->bmiHeader.biHeight == height)
            {
                filterResolution.push_back(_capsList[i]);
            }
        }
        // get the max fps if the fps are not reach on the desired resolution
        typedef std::multimap<double, CapEntry> ContainerFrameRateSorted;
        ContainerFrameRateSorted filterFrameRate;
        for (int i = 0; i < (int)filterResolution.size(); i++)
        {
            VIDEOINFOHEADER* video= filterResolution[i].second;
            double capfps = 1.0/  (video->AvgTimePerFrame * 100.0 * 1e-9);
            double error = fabs(capfps - fps);
            filterFrameRate.insert(std::pair<double, CapEntry>(error, filterResolution[i]));
        }

        CapEntry nullCapEntry(static_cast<AM_MEDIA_TYPE*>(NULL), static_cast<VIDEOINFOHEADER*>(NULL));
        CapEntry best = nullCapEntry;
        CapEntry first = nullCapEntry;

        for (ContainerFrameRateSorted::iterator it = filterFrameRate.begin();
               it != filterFrameRate.end();
               ++it)
        {
            if (first == nullCapEntry)
                first = it->second;

            if (it->first < 1e-3)
            {
                VIDEOINFOHEADER* video= it->second.second;
                FOURCCMap fccMap(video->bmiHeader.biCompression);
                GUID g1 = (GUID)fccMap;
                if (getStringFromGUID(&g1) == std::string("YUY2"))
                    best = it->second;
            }
        }
        if (best != nullCapEntry)
            return best;
        if (first != nullCapEntry)
            return first;

        if (!_capsList.empty())
            return _capsList.front();
        return nullCapEntry;
    }

    void createList()
    {
        std::string device = "capture";
        // Use the IPin to get the interface:

        // get the number of formats and make sure the strutucre size matches
        int iCount, iSize;
        VIDEO_STREAM_CONFIG_CAPS caps;
        _config->GetNumberOfCapabilities(&iCount, &iSize);
        if( sizeof(caps) != iSize )
            return;

        // now go through all formats and use the one you like
        for(int i=0; i < iCount; i++)
        {
            // GetStreamCaps allocats the AM_MEDIA_TYPE, which must be deleted by using DeleteMediaType
            AM_MEDIA_TYPE *pmt = NULL;
            if( _config->GetStreamCaps(i, &pmt, (BYTE*)&caps) == S_OK )
            {
                if (pmt->formattype == FORMAT_VideoInfo && pmt->cbFormat >= sizeof(VIDEOINFOHEADER))
                {
                    VIDEOINFOHEADER *video=
                        reinterpret_cast<VIDEOINFOHEADER*>(pmt->pbFormat);
                    _capsList.push_back(CapEntry(pmt, video));
                }
                else
                {
                    DeleteMediaType(pmt);
                }
            }
        }
    }
};


bool CTextureRenderer::setupOutputSoundDevice(ICreateDevEnum* devs)
{
    if (!devs)
        return false;

    std::string prefixForMessage;
    {
    std::stringstream ss;
    ss << _imageStream.get();
    prefixForMessage = ss.str();
    }

    HRESULT hr;
    std::string outputdevice = "Default DirectSound Device";
    IEnumMoniker*   audioRenderer = 0; hr = devs?devs->CreateClassEnumerator (CLSID_AudioRendererCategory, &audioRenderer, 0):0;
    if (!checkError(prefixForMessage, hr))
        return false;
    ListDeviceAvailable deviceFinder(audioRenderer);

    deviceFinder.displayDevicesFound(prefixForMessage + " sounddevice", osg::INFO);

    ListDeviceAvailable::DeviceEntry device = deviceFinder.getDevice(outputdevice);
    if (!device._device)
    {
        OSG_WARN << prefixForMessage << " no output sound device \"" << outputdevice << "\" found" << std::endl;
        return false;
    }

    _soundOutputDeviceName = device._name;
    IMoniker* mon  = device._device;
    hr = mon?mon->BindToObject(0,0,IID_IBaseFilter, (void**)&_soundOutputDevice):0;
    checkError(prefixForMessage, hr);
    if (FAILED(hr))
        return false;

    hr = _graphBuilder->AddFilter(_soundOutputDevice,NULL);
    checkError(prefixForMessage, hr);
    if (FAILED(hr))
    {
        if (_soundOutputDevice) _soundOutputDevice->Release(); _soundOutputDevice = 0;
        return false;
    }

    return true;
}

bool CTextureRenderer::openVideoCaptureDevice(const std::string& capture, int wantWidth, int wantHeight, double wantFps)
{
    std::string prefixForMessage;
    {
    std::stringstream ss;
    ss << _imageStream.get();
    prefixForMessage = ss.str();
    }

    HRESULT hr;
    ICreateDevEnum* devs = 0; hr = CoCreateInstance (CLSID_SystemDeviceEnum, 0, CLSCTX_INPROC, IID_ICreateDevEnum, (void **) &devs);
    if (!checkError(prefixForMessage, hr))
        return false;

    IEnumMoniker*   cams = 0; hr = devs?devs->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, &cams, 0):0;
    if (!checkError(prefixForMessage, hr))
        return false;

    ListDeviceAvailable deviceFinder(cams);
    {
    std::stringstream ss;
    ss << std::hex << _imageStream.get() << " capture";
    deviceFinder.displayDevicesFound(ss.str(), osg::INFO);
    }
    ListDeviceAvailable::DeviceEntry device = deviceFinder.getDevice(capture);

    if (!device._device)
    {
        OSG_WARN << _imageStream.get() << " no capture device \"" << capture << "\" found" << std::endl;
        return false;
    }
    _videoCaptureDeviceName = device._name;
    OSG_NOTICE << _imageStream.get() << " use capture device \"" << getVideoCaptureDeviceName() << "\"" << std::endl;

    {
    std::stringstream ss;
    ss << _imageStream.get() << " \"" << getVideoCaptureDeviceName() << "\"";
    prefixForMessage = ss.str();
    }


    IMoniker* mon  = device._device;

    hr = mon?mon->BindToObject(0,0,IID_IBaseFilter, (void**)&_videoCaptureDevice):0;
    if (!checkError(prefixForMessage, hr))
        return false;

    IEnumPins* pins = 0; hr = _videoCaptureDevice?_videoCaptureDevice->EnumPins(&pins):0;
    if (!checkError(prefixForMessage, hr))
        return false;

    IPin* cap  = 0; hr = pins?pins->Next(1,&cap, 0):0;
    if (pins) pins->Release(); pins = 0;
    if (!checkError(prefixForMessage, hr))
        return false;


    IAMStreamConfig* capConfig = 0; hr = cap->QueryInterface( IID_IAMStreamConfig, (void **)&capConfig);
    if (capConfig)
    {
        ListCapDeviceAvailable capsDevice(capConfig);
        capsDevice.displayCapsFound(prefixForMessage);
        ListCapDeviceAvailable::CapEntry found = capsDevice.getCaps(wantWidth, wantHeight, wantFps);
        if (found.first)
        {
            ListCapDeviceAvailable::displayCap(found.second,  prefixForMessage + " use ");
            capConfig->SetFormat(found.first);
        }
        capConfig->Release();
    }

    hr = _graphBuilder->AddFilter(_videoCaptureDevice, L"Capture Source");
    if (!checkError(prefixForMessage, hr))
        return false;

    IPin*           rnd  = 0;
    hr = FindPin(L"In", &rnd);
    if (!checkError(prefixForMessage, hr))
        return false;

    hr = _graphBuilder->Connect(cap,rnd);

    if (rnd) rnd->Release();
    if (cap) cap->Release();

    bool result = checkError(prefixForMessage, hr);
    return result;
}


bool CTextureRenderer::openSoundCaptureDevice(const std::string& capture, int nbChannels)
{
    std::string prefixForMessage;
    {
    std::stringstream ss;
    ss << _imageStream.get();
    prefixForMessage = ss.str();
    }

    HRESULT hr;
    ICreateDevEnum* devs = 0; hr = CoCreateInstance (CLSID_SystemDeviceEnum, 0, CLSCTX_INPROC, IID_ICreateDevEnum, (void **) &devs);
    if (!checkError(prefixForMessage, hr))
        return false;

    IEnumMoniker*   captureDevices = 0; hr = devs?devs->CreateClassEnumerator (CLSID_AudioInputDeviceCategory, &captureDevices, 0):0;
    if (!checkError(prefixForMessage, hr))
        return false;

    ListDeviceAvailable deviceFinder(captureDevices);
    {
    std::stringstream ss;
    ss << std::hex << _imageStream.get() << " capture sound ";
    deviceFinder.displayDevicesFound(ss.str(), osg::INFO);
    }
    ListDeviceAvailable::DeviceEntry device = deviceFinder.getDevice(capture);

    if (!device._device)
    {
        OSG_WARN << _imageStream.get() << " no sound capture device \"" << capture << "\" found" << std::endl;
        return false;
    }
    _soundCaptureDeviceName = device._name;
    OSG_NOTICE << _imageStream.get() << " use sound capture device \"" << getSoundCaptureDeviceName() << "\"" << std::endl;

    {
    std::stringstream ss;
    ss << _imageStream.get() << " \"" << getSoundCaptureDeviceName() << "\"";
    prefixForMessage = ss.str();
    }


    IMoniker* mon  = device._device;

    hr = mon?mon->BindToObject(0,0,IID_IBaseFilter, (void**)&_soundCaptureDevice):0;
    if (FAILED(hr))
    {
        checkError(prefixForMessage, hr);
        return false;
    }

    IAMStreamConfig* pISC = NULL;
    hr = FindPinInterface(_soundCaptureDevice, &MEDIATYPE_Audio, PINDIR_OUTPUT, IID_IAMStreamConfig, reinterpret_cast<void**>(&pISC));
    if (FAILED(hr))
    {
        checkError(prefixForMessage, hr);
        return false;
    }
    // loop through all the capabilities (audio formats) and populate the control
    int count, size;
    hr = pISC->GetNumberOfCapabilities(&count, &size);
    if (SUCCEEDED(hr))
    {
        if (sizeof(AUDIO_STREAM_CONFIG_CAPS) == size)
        {
            AM_MEDIA_TYPE* pmt = NULL;
            AUDIO_STREAM_CONFIG_CAPS ascc;
            WAVEFORMATEX* pwfex = NULL;

            for (int index=0; index<count; ++index)
            {
                hr = pISC->GetStreamCaps(index, &pmt, reinterpret_cast<BYTE*>(&ascc));
                if (SUCCEEDED(hr))
                {
                    TCHAR buffer[32];

                    ZeroMemory(buffer, sizeof(buffer));

                    pwfex = (WAVEFORMATEX*)pmt->pbFormat;

                    // provide a useful description of the formats
                    if (1 == pwfex->nChannels)
                    {
                        StringCbPrintf(buffer, sizeof(buffer), TEXT("%d channel, %2.1fkHz, %d-bit"), (int)pwfex->nChannels, (float)pwfex->nSamplesPerSec / 1000, (int)pwfex->wBitsPerSample);
                    }
                    else
                    {
                        StringCbPrintf(buffer, sizeof(buffer), TEXT("%d channels, %2.1fkHz, %d-bit"), (int)pwfex->nChannels, (float)pwfex->nSamplesPerSec / 1000, (int)pwfex->wBitsPerSample);
                    }

                    // set default format
                    if (pwfex->nChannels == nbChannels)
                    {
                        pISC->SetFormat(pmt);
                        OSG_NOTICE << prefixForMessage << " use format " << buffer << std::endl;
                        break;
                    }
                    else
                    {
                        OSG_NOTICE << prefixForMessage << buffer << std::endl;
                    }

                }
            }
        }
        else
        {
            OSG_WARN << prefixForMessage << " can t retrieve informations pins" << std::endl;
        }
    }
    if (pISC) pISC->Release(); pISC = 0;
    IPin* captureOutputDevicePinOut = 0; hr = _soundCaptureDevice? ::GetPin(_soundCaptureDevice,L"Capture",&captureOutputDevicePinOut) : 0;
    if (FAILED(hr)) {
        checkError(prefixForMessage, hr);
        return false;
    }

    hr = _graphBuilder->AddFilter(_soundCaptureDevice, L"Sound Capture Source");
    if (FAILED(hr))
    {
        checkError(prefixForMessage, hr);
        return false;
    }

    if (!setupOutputSoundDevice(devs))
    {
        devs->Release(); devs = 0;
        if (captureOutputDevicePinOut) captureOutputDevicePinOut->Release(); captureOutputDevicePinOut = 0;
        return false;
    }
    devs->Release(); devs = 0;

    std::string prefixForMessageSound;
    {
    std::stringstream ss;
    ss << _imageStream.get() << " " << getSoundOutputDeviceName();
    prefixForMessageSound = ss.str();
    }

    IPin* soundOutputDevicePinIn = 0;
    hr = _soundOutputDevice->FindPin(L"Audio Input pin (rendered)", &soundOutputDevicePinIn);
    if (FAILED(hr))
    {
        checkError(prefixForMessageSound, hr);
        if (soundOutputDevicePinIn)    soundOutputDevicePinIn->Release(); soundOutputDevicePinIn = 0;
        if (captureOutputDevicePinOut) captureOutputDevicePinOut->Release(); captureOutputDevicePinOut = 0;
        return false;
    }

    hr = _graphBuilder->Connect(captureOutputDevicePinOut, soundOutputDevicePinIn);

    if (soundOutputDevicePinIn)    soundOutputDevicePinIn->Release(); soundOutputDevicePinIn = 0;
    if (captureOutputDevicePinOut) captureOutputDevicePinOut->Release(); captureOutputDevicePinOut = 0;

    if (FAILED(hr))
    {
        checkError(prefixForMessageSound, hr);
        return false;
    }

    return true;
}


bool CTextureRenderer::openCaptureDevices(const DirectShowImageStream::Options& o)
{
    std::string prefixForMessage;
    {
    std::stringstream ss;
    ss << _imageStream.get();
    prefixForMessage = ss.str();
    }

    DirectShowImageStream::Options options = o;
    syncStreams(false);

    for (DirectShowImageStream::Options::iterator it = options.begin(); it != options.end(); it++)
    {
        OSG_NOTICE << prefixForMessage << " option " << it->first << " = " << it->second << std::endl;
    }

    std::string soundCaptureDevice = options["captureSoundDevice"];
    std::string videoCaptureDevice = options["captureVideoDevice"];

    OSG_NOTICE << prefixForMessage << " try to open video capture device " << videoCaptureDevice;
    if (!soundCaptureDevice .empty())
    {
        OSG_NOTICE << " and sound capture device " << soundCaptureDevice ;
    }

    OSG_NOTICE << std::endl;

    if (!initBuildGraph())
        return false;

    int wantWidth = atoi(options["captureWantedWidth"].c_str());
    int wantHeight = atoi(options["captureWantedHeight"].c_str());
    float wantFps = atof(options["captureWantedFps"].c_str());
    if (!openVideoCaptureDevice(videoCaptureDevice, wantWidth, wantHeight, wantFps))
        return false;

    if (!soundCaptureDevice.empty() && !openSoundCaptureDevice(soundCaptureDevice ))
    {
        OSG_WARN << prefixForMessage << " failed to setup sound capture device " << soundCaptureDevice  << std::endl;
    }
    return true;
}


bool CTextureRenderer::openFile(const std::string& file)
{
    syncStreams(true);
    WCHAR wFileName[MAX_PATH];
    wFileName[MAX_PATH-1] = 0;    // NULL-terminate
    if (file.empty())
        return false;

    _filename = file;
    std::string prefixForMessage;
    {
    std::stringstream ss;
    ss << _imageStream.get() << " " << getFilename();
    prefixForMessage = ss.str();
    }

    const char *ansistr = file.c_str();
    int lenA = lstrlenA(ansistr);
    int lenW;
    BSTR unicodestr = 0;

    lenW = ::MultiByteToWideChar(CP_ACP, 0, ansistr, lenA, 0, 0);
    if (lenW > 0)
    {
      // Check whether conversion was successful
      unicodestr = ::SysAllocStringLen(0, lenW);
      ::MultiByteToWideChar(CP_ACP, 0, ansistr, lenA, unicodestr, lenW);
    }

    if (unicodestr!=0)
    {
        (void)StringCchCopyW(wFileName, NUMELMS(wFileName), unicodestr);

        // when done, free the BSTR
        ::SysFreeString(unicodestr);
    }

    HRESULT hr;
    if (!initBuildGraph())
        return false;

    IPin* videoOutputPin = 0;

    std::string lowercase = file;
    std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(), tolower);

    hr = _graphBuilder->AddSourceFilter(wFileName, L"Source", &_fileSource);
    if (!checkError(prefixForMessage, hr))
        return false;

    //Find the video pin
    hr = _fileSource? ::GetPin(_fileSource, &MEDIATYPE_Video, PINDIR_OUTPUT, &videoOutputPin):0;

    if (!checkError(prefixForMessage, hr))
        return false;

    IPin*           rnd  = 0;
    hr = FindPin(L"In", &rnd);
    if (!checkError(prefixForMessage, hr))
        return false;

    hr = _graphBuilder->Connect(videoOutputPin, rnd);
    if (videoOutputPin) videoOutputPin->Release(); videoOutputPin = 0;

    if (rnd) rnd->Release(); rnd = 0;
    if (!checkError(prefixForMessage, hr))
        return false;

    // check if we find the sounds output pin on the streams
    IBaseFilter* soundFilter;
    hr = _graphBuilder->FindFilterByName(L"AVI Splitter", &soundFilter);
    if (FAILED(hr))
    {
        //Could not find the AVI Splitter filter, try the main source itself
        soundFilter = _fileSource;
    }

    if (soundFilter)
    {
        IPin* soundStreamPinOut = 0;
        //Try to find the audio pin
        hr = ::GetPin(soundFilter, &MEDIATYPE_Audio, PINDIR_OUTPUT, &soundStreamPinOut);
        if (soundFilter != _fileSource) soundFilter->Release();
        soundFilter = 0;

        if (FAILED(hr))
        {
            OSG_WARN << prefixForMessage << " can't find audio pin" << getErrorMessage(hr) << std::endl;
        }

        if (soundStreamPinOut)
        {
            // connect sounds to graph
            ICreateDevEnum* devs = 0; hr = CoCreateInstance (CLSID_SystemDeviceEnum, 0, CLSCTX_INPROC, IID_ICreateDevEnum, (void **) &devs);
            checkError(prefixForMessage, hr);
            if (devs && setupOutputSoundDevice(devs))
            {
                devs->Release(); devs = 0;
                std::string prefixForMessageSound;
                {
                std::stringstream ss;
                ss << _imageStream.get() << " " << getSoundOutputDeviceName();
                prefixForMessageSound = ss.str();
                }

                IPin* soundOutputDevicePinIn = 0;
                hr = _soundOutputDevice->FindPin(L"Audio Input pin (rendered)", &soundOutputDevicePinIn);
                checkError(prefixForMessageSound, hr);

                hr = _graphBuilder->Connect(soundStreamPinOut, soundOutputDevicePinIn);

                if (soundOutputDevicePinIn)    soundOutputDevicePinIn->Release(); soundOutputDevicePinIn = 0;
                if (soundStreamPinOut) soundStreamPinOut->Release(); soundStreamPinOut = 0;

                if (!checkError(prefixForMessageSound, hr) && _soundOutputDevice)
                {
                    _graphBuilder->RemoveFilter(_soundOutputDevice);
                    _soundOutputDevice->Release(); _soundOutputDevice = 0;
                }
            }
        }
    }

    return true;
}



//-----------------------------------------------------------------------------
// CleanupDShow
//-----------------------------------------------------------------------------
void CTextureRenderer::releaseRessources()
{
    // Shut down the graph
    if (_mediaControl) _mediaControl->Stop();
    _imageStream = 0;

    if (_mediaControl) _mediaControl->Release(); _mediaControl = 0;
    if (_mediaEvent) _mediaEvent->Release(); _mediaEvent = 0;
    if (_mediaSeeking) _mediaSeeking->Release(); _mediaSeeking = 0;
    if (_basicAudio) _basicAudio->Release(); _basicAudio = 0;
    // remove filter outside because this is a filter too.
}


//-----------------------------------------------------------------------------
// CTextureRenderer constructor
//-----------------------------------------------------------------------------

CTextureRenderer::CTextureRenderer( DirectShowImageStream* is, HRESULT* valid)
    : CBaseVideoRenderer(__uuidof(CLSID_TextureRenderer),
                         NAME("Texture Renderer"),
                         NULL,
                         valid)
{
    std::string prefixForMessage;
    {
    std::stringstream ss;
    ss << is;
    prefixForMessage = ss.str();
    }

    if (FAILED(*valid))
        checkError(prefixForMessage, *valid);

    _imageStream = is;
    _fileSource = 0;
    _mediaControl = 0;
    _mediaEvent = 0;
    _mediaSeeking = 0;
    _graphBuilder = 0;
    _videoCaptureDevice = 0;
    _soundOutputDevice = 0;
    _soundCaptureDevice = 0;
    _basicAudio = 0;
}


//-----------------------------------------------------------------------------
// CTextureRenderer destructor
//-----------------------------------------------------------------------------
CTextureRenderer::~CTextureRenderer()
{
}


//-----------------------------------------------------------------------------
// CheckMediaType: This method forces the graph to give us an R8G8B8 video
// type, making our copy to texture memory trivial.
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::CheckMediaType(const CMediaType *pmt)
{
    HRESULT   hr = E_FAIL;
    VIDEOINFO *pvi=0;

    CheckPointer(pmt,E_POINTER);

    // Reject the connection if this is not a video type
    if( *pmt->FormatType() != FORMAT_VideoInfo ) {
        return E_INVALIDARG;
    }

    // Only accept RGB24 video
    pvi = (VIDEOINFO *)pmt->Format();

    if (!IsEqualGUID( *pmt->Type(),    MEDIATYPE_Video))
    {
        OSG_WARN << _imageStream.get() << " media type not a video format" << std::endl;
    }

    if( IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_RGB24))
    {
        OSG_NOTICE << _imageStream.get() << " Texture Renderer use media " << getStringFromGUID(pmt->Subtype()) << std::endl;
        hr = S_OK;
    }
    else
    {
        OSG_INFO << _imageStream.get() << " Texture Renderer check media " << getStringFromGUID(pmt->Subtype()) << std::endl;
    }

    return hr;
}

//-----------------------------------------------------------------------------
// SetMediaType: Graph connection has been made.
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::SetMediaType(const CMediaType *pmt)
{
    // Retrive the size of this media type
    VIDEOINFO *pviBmp;                      // Bitmap info header
    pviBmp = (VIDEOINFO *)pmt->Format();

    _width  = pviBmp->bmiHeader.biWidth;
    _height = abs(pviBmp->bmiHeader.biHeight);
    _pitch  = (_width * 3 + 3) & ~(3); // We are forcing RGB24
    _imageStream->setImage(_width, _height, 1, GL_RGB, GL_BGR, GL_UNSIGNED_BYTE, 0, osg::Image::NO_DELETE);
    return S_OK;
}

//-----------------------------------------------------------------------------
// DoRenderSample: A sample has been delivered. Copy it to the texture.
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::DoRenderSample( IMediaSample * pSample )
{
    BYTE  *pBmpBuffer; // Bitmap buffer, texture buffer
    CheckPointer(pSample,E_POINTER);

    // Get the video bitmap buffer
    pSample->GetPointer( &pBmpBuffer );
    if (_imageStream.valid())
        _imageStream->setImage(_width, _height, 1, GL_RGB, GL_BGR, GL_UNSIGNED_BYTE, pBmpBuffer, osg::Image::NO_DELETE);
    return S_OK;
}

void CTextureRenderer::syncStreams(bool state)
{
    if (state == true)
        _dropFrame = !S_OK;
    else
        _dropFrame = S_OK;
}
HRESULT CTextureRenderer::ShouldDrawSampleNow(IMediaSample *sample, REFERENCE_TIME *start, REFERENCE_TIME *stop) {
    return _dropFrame; // disable droping of frames
}

bool CTextureRenderer::StopFilters()
{
    HRESULT hr;
    if (_mediaControl)
    {
        hr = _mediaControl->Stop();
        if (FAILED(hr))
        {
            OSG_WARN << _imageStream.get() << " " << getErrorMessage(hr) << std::endl;
            return false;
        }
    }
    return true;
}

DirectShowImageStream::DirectShowImageStream()
{
    _options["captureWantedWidth"] = "1920";
    _options["captureWantedHeight"] = "1080";
    _options["captureWantedFps"] = "30";

    HRESULT hr = CoInitialize (NULL);
    if (FAILED(hr))
    {
        OSG_WARN << this << " error in constructor " << getErrorMessage(hr) << std::endl;
    }

}
DirectShowImageStream::DirectShowImageStream(const DirectShowImageStream& d,const osg::CopyOp& c) : osg::ImageStream(d, c)
{
    // i guess it's invalid
}

DirectShowImageStream::~DirectShowImageStream()
{
    stop();
    CoUninitialize();
}

bool DirectShowImageStream::openFile(const std::string& file)
{
    HRESULT valid = S_OK;
    _renderer = new CTextureRenderer(this, &valid);
    if (FAILED(valid))
    {
        return false;
    }
    if (!_renderer->openFile(file))
    {
        return false;
    }
    return true;
}

bool DirectShowImageStream::openCaptureDevices()
{
    HRESULT valid = S_OK;
    _renderer = new CTextureRenderer(this, &valid);
    if (FAILED(valid))
    {
        return false;
    }
    if (!_renderer->openCaptureDevices(_options))
    {
        return false;
    }

    return true;
}



void DirectShowImageStream::play()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    if (_status == PLAYING)
        return;

    if (_renderer.valid() && _renderer->_mediaControl)
    {
        HRESULT hr;
        hr = _renderer->_mediaControl->Run();
        // Start the graph running;
        if (FAILED(hr))
        {
            OSG_WARN << this << " can't run the graph " <<  getErrorMessage(hr) <<  std::endl;
        }
        else
        {
            _status = PLAYING;
        }
    }
}

void DirectShowImageStream::pause()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    if (_status == PAUSED)
        return;

    if (_renderer.valid() && _renderer->_mediaControl)
    {
        HRESULT hr;
        hr = _renderer->_mediaControl->Pause();
        if (FAILED(hr))
        {
            OSG_NOTICE << this << " " << getErrorMessage(hr) << std::endl;
        }
        else
        {
            _status = PAUSED;
        }
    }
}

void DirectShowImageStream::rewind()
{
    seek(0);
}

osg::ImageStream::StreamStatus DirectShowImageStream::getStatus()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    return _status;
}

void DirectShowImageStream::seek(double time)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    if (_renderer.valid() && _renderer->_mediaSeeking)
    {
        double start = time / (100 * 1e-9);
        LONGLONG start2 = static_cast<LONGLONG>(start);
        HRESULT hr = _renderer->_mediaSeeking->SetPositions(&start2,AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
        if (FAILED(hr))
        {
            OSG_NOTICE << this << " " << getErrorMessage(hr) << std::endl;
        }
    }

}

double DirectShowImageStream::getCurrentTime() const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    double currentTime = -1;
    if (_renderer.valid() && _renderer->_mediaSeeking)
    {

       LONGLONG curTimeLL = 0;
       HRESULT hr = _renderer->_mediaSeeking->GetCurrentPosition(&curTimeLL);
       if (FAILED(hr))
       {
            OSG_NOTICE << this << " " << getErrorMessage(hr) << std::endl;
       }
       else
       {
           currentTime = static_cast<double>(curTimeLL);
           currentTime = currentTime * (100.0 * 1e-9); // default unit in directshow IMediaSeeking
       }
    }
    return currentTime;
}

void DirectShowImageStream::setOptions(const Options& map)
{
    for (Options::const_iterator it = map.begin(); it != map.end(); it++)
        _options[it->first] = it->second;
}

void DirectShowImageStream::quit(bool waitForThreadToExit)
{
    stop();
}

double DirectShowImageStream::getLength() const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    double duration = 0;
    if (_renderer.valid() && _renderer->_mediaSeeking)
    {
        LONGLONG d = 0;
        _renderer->_mediaSeeking->GetDuration(&d);
        duration = static_cast<double>(d);
        duration = duration * (100.0 * 1e-9); // default unit in directshow IMediaSeeking
    }
    return duration;
}

double DirectShowImageStream::getFrameRate() const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    int frameRate = 0;
    if (_renderer.valid())
        _renderer->get_AvgFrameRate(&frameRate);
    return static_cast<double>(frameRate) * 1e-2;
}

void DirectShowImageStream::setTimeMultiplier(double rate)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    if (_renderer.valid() && _renderer->_mediaSeeking)
        _renderer->_mediaSeeking->SetRate(rate);
}

double DirectShowImageStream::getTimeMultiplier() const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    double rate = 1.0;
    if (_renderer.valid() && _renderer->_mediaSeeking)
        _renderer->_mediaSeeking->GetRate(&rate);
    return rate;
}

void DirectShowImageStream::setVolume(float vol) {
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
   if (_renderer.valid() && _renderer->_basicAudio)
   {
     _renderer->_basicAudio->put_Volume(vol);
   }
}

float DirectShowImageStream::getVolume() const {
  OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
  double vol = 0;
  if (_renderer.valid() && _renderer->_basicAudio)
  {
    long d = 0;
    _renderer->_basicAudio->get_Volume(&d);
    vol = static_cast<double>(d);
  }
  return vol;
}

void DirectShowImageStream::stop()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    if (!_renderer.valid())
        return;

    if (!_renderer->StopFilters())
    {
        OSG_WARN << this << " can't stop filters" << std::endl;
    }

    _renderer->releaseRessources();
    // Enumerate the filters in the graph.
    IEnumFilters *pEnum = NULL;
    IGraphBuilder* gb = _renderer->getGraphBuilder();
    if (!gb)
        return;
    HRESULT hr = gb->EnumFilters(&pEnum);
    if (SUCCEEDED(hr))
    {
        IBaseFilter *pFilter = NULL;
        while (S_OK == pEnum->Next(1, &pFilter, NULL))
         {
             // Remove the filter.
             gb->RemoveFilter(pFilter);
             // Reset the enumerator.
             pEnum->Reset();
             pFilter->Release();
        }
        pEnum->Release();
    }
    if (gb) gb->Release(); gb = 0;
    _renderer = 0;
}
