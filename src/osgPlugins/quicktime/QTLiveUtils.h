/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2007 Robert Osfield
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

#ifdef __APPLE__
    #include <QuickTime/QuickTime.h>
    #include <Carbon/Carbon.h>
    #define QT_HANDLE_IMAGES_ALSO
#else
    #include <QTML.h>
    #include <Movies.h>
    #include <Quickdraw.h>
    #include <QDOffscreen.h>
    #include <QuicktimeComponents.h>
    #include <FixMath.h>
    #include <CGBitmapContext.h>
    #include <CGImage.h>
    #include <CGColorSpace.h>
    #include <ImageCompression.h>
    #include <TextUtils.h>
#endif
//QT
//#include "Components.h"
//#include "QuickTimeComponents.h"

// QTML
void initialize_quicktime_qtml();
void terminite_quicktime_qtml();

class QTScopedQTMLInitialiser
{
public:
    QTScopedQTMLInitialiser();
   ~QTScopedQTMLInitialiser();
private:
    QTScopedQTMLInitialiser(const QTScopedQTMLInitialiser&);
    const QTScopedQTMLInitialiser& operator=(const QTScopedQTMLInitialiser&);
};

// QT Movies
void enter_quicktime_movies();
void leave_quicktime_movies();

class QTScopedMovieInitialiser
{
public:
    QTScopedMovieInitialiser();
   ~QTScopedMovieInitialiser();
private:
    QTScopedMovieInitialiser(const QTScopedMovieInitialiser&);
    const QTScopedMovieInitialiser& operator=(const QTScopedMovieInitialiser&);
};

#if TARGET_OS_MAC
// QT Movies_MT (QT multi-thread support API)
/*
 * EnterMovies initializes a single, non-reentrant QuickTime environment for your application.
 * If your application uses QuickTime on multiple threads simultaneously, call EnterMoviesOnThread from each thread that uses QuickTime to create a local QuickTime environment for that thread (requires QuickTime 6 or later).
 * For more information about threaded programming and QuickTime, see Technical Note TN2125, Thread-safe programming in QuickTime.
 * http://developer.apple.com/technotes/tn/tn2125.html
 */
void enter_quicktime_movies_mt();
void leave_quicktime_movies_mt();

class QTScopedMovieInitialiser_MT
{
public:
    QTScopedMovieInitialiser_MT();
   ~QTScopedMovieInitialiser_MT();
private:
    QTScopedMovieInitialiser_MT(const QTScopedMovieInitialiser_MT&);
    const QTScopedMovieInitialiser_MT& operator=(const QTScopedMovieInitialiser_MT&);
};
#endif

// Utils
char* pstr_printable(StringPtr src);

typedef std::pair<std::string,std::string> OSG_SGDevicePair;
typedef std::vector<OSG_SGDevicePair>      OSG_SGDeviceList;

// Capability Video
void print_video_component_capability(VideoDigitizerComponent aComponent);
void probe_video_digitizer_components();
// Capability Sequence Grabber
OSG_SGDeviceList              print_sequence_grabber_device_list(SGDeviceList deviceList);
std::vector<OSG_SGDeviceList> probe_sequence_grabber_components();
//
void get_video_device_bounds_idstr(short videoDeviceID, short videoDeviceInputID, short& out_width, short& out_height, Str63& out_videoDeviceIDStr);
void get_sound_device_idstr(short soundDeviceID, short soundDeviceInputID, Str63& out_soundDeviceIDStr);










