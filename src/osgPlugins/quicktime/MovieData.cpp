/*
 *  MovieData.cpp
 *  lala
 *
 *  Created by Stephan Huber on Wed Mar 10 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */
#include <osg/GL>
#include <osg/Endian>
#include <osg/Image>
#include <osgDB/FileNameUtils>

#include "MovieData.h"
#include "QTUtils.h"




MovieData::MovieData() : _pointer(NULL), _movie(NULL), _gw(NULL), _fError(false), _isLooping(false)
{

}


MovieData::~MovieData()
{
    if (_pointer) free(_pointer);
    if (_gw) DisposeGWorld(_gw);

    if (_movie) {
        DisposeMovie(_movie);
    }
}




void MovieData::load(osg::Image* image, std::string afilename, double startTime)
{
    bool isUrl( osgDB::containsServerAddress(afilename) );

    std::string filename = afilename;
    if (!isUrl) {
        if (!osgDB::isFileNameNativeStyle(filename))
            filename = osgDB::convertFileNameToNativeStyle(filename);
    }

    image->setFileName(filename);


    QTNewMoviePropertyElement newMovieProperties[2];
    CFStringRef movieLocation = CFStringCreateWithCString(NULL, filename.c_str(), kCFStringEncodingUTF8);
    CFURLRef movieURL(NULL);
    Boolean trueValue = true;

    newMovieProperties[0].propClass = kQTPropertyClass_DataLocation;
    if (!isUrl)
    {
        #ifdef __APPLE__
            newMovieProperties[0].propID = kQTDataLocationPropertyID_CFStringPosixPath;
        #else
            newMovieProperties[0].propID = kQTDataLocationPropertyID_CFStringWindowsPath;
        #endif

        newMovieProperties[0].propValueSize = sizeof(CFStringRef);
        newMovieProperties[0].propValueAddress = &movieLocation;
    }
    else
    {
        movieURL = CFURLCreateWithString(kCFAllocatorDefault, movieLocation, NULL);

        newMovieProperties[0].propID = kQTDataLocationPropertyID_CFURL;
        newMovieProperties[0].propValueSize = sizeof(movieURL);
        newMovieProperties[0].propValueAddress = (void*)&movieURL;
    }

    // make movie active
    newMovieProperties[1].propClass = kQTPropertyClass_NewMovieProperty;
    newMovieProperties[1].propID = kQTNewMoviePropertyID_Active;
    newMovieProperties[1].propValueSize = sizeof(trueValue);
    newMovieProperties[1].propValueAddress = &trueValue;

    // Instantiate the Movie
    OSStatus status = NewMovieFromProperties(2, newMovieProperties, 0, NULL, &_movie);
    CFRelease(movieLocation);
    if (movieURL) CFRelease(movieURL);

    if (status !=0) {
        _fError = true;
        OSG_FATAL << " MovieData :: NewMovieFromProperties failed with err " << status<< std::endl;
        return;
    }


    Rect bounds;

#ifdef __APPLE__
    GetRegionBounds(GetMovieBoundsRgn(_movie), &bounds);
#else
    bounds = (*GetMovieBoundsRgn(_movie))->rgnBBox;
#endif

    _checkMovieError("Can't get movie bounds\n");

    OffsetRect(&bounds, -bounds.left, -bounds.top);
    SetMovieBox(_movie, &bounds);
    _checkMovieError("Can't set movie box\n");

    _movieWidth = bounds.right;
    _movieHeight = bounds.bottom;

    _timescale = GetMovieTimeScale(_movie);

    _initImage(image);
    if (!_fError) _initGWorldStuff(image);


    if (!_fError) {

        if ( startTime == 0.0)
            GoToBeginningOfMovie(_movie);
        else {
            TimeValue t = (TimeValue) (startTime*_timescale);
            SetMovieTimeValue(_movie,t);
        }

        UpdateMovie(_movie);
        SetMovieRate(_movie,0.0);
        SetMovieActive(_movie, true);
        UpdateMovie(_movie);
        MoviesTask(_movie,0);
    }
}


// ---------------------------------------------------------------------------
// _intImage
// create image for storing
// ---------------------------------------------------------------------------

void MovieData::_initImage(osg::Image* image)
{

    void* buffer;


    _textureWidth = ((_movieWidth + 7) >> 3) << 3;
    _textureHeight = _movieHeight;

    // some magic alignment...
    _pointer = (char*)malloc(4 * _textureWidth * _textureHeight + 32);

    if (_pointer == NULL) {
        OSG_FATAL << "MovieData: " << "Can't allocate texture buffer" << std::endl;
        _fError= true;
    }

    buffer = (void*)(((unsigned long)(_pointer + 31) >> 5) << 5);

    GLenum internalFormat = (osg::getCpuByteOrder()==osg::BigEndian)?
                            GL_UNSIGNED_INT_8_8_8_8_REV :
                            GL_UNSIGNED_INT_8_8_8_8;

    image->setImage(_textureWidth,_textureHeight,1,
                   (GLint) GL_RGBA8,
                   (GLenum)GL_BGRA,
                   internalFormat,
                   (unsigned char*) buffer,osg::Image::NO_DELETE,4);

}

// ---------------------------------------------------------------------------
// _initGWorldStuff
// init gworld-stuff, so quicktime can play the movie into the gWorld.
// ---------------------------------------------------------------------------

void MovieData::_initGWorldStuff(osg::Image * image)  {

    Rect textureBounds;
    OSStatus err;
    GDHandle        origDevice;
    CGrafPtr        origPort;
    PixMapHandle pixmap = NULL;

    textureBounds.left = 0;
    textureBounds.top = 0;
    textureBounds.right = image->s();
    textureBounds.bottom = image->t();
    err = QTNewGWorldFromPtr(&_gw, k32ARGBPixelFormat, &textureBounds, NULL, NULL, 0, image->data(), 4 * image->s());

    if (err !=0 )
    {
        OSG_FATAL << "MovieData : Could not create gWorld" << std::endl;
    }

    GetGWorld (&origPort, &origDevice);
    SetGWorld(_gw, NULL);                                         // set current graphics port to offscreen
    SetMovieGWorld(_movie, (CGrafPtr)_gw, NULL);

    _checkMovieError("SetMovieGWorld failed");

    pixmap = GetGWorldPixMap (_gw);
    if (pixmap)
    {
        if (!LockPixels (pixmap))                                        // lock offscreen pixel map
        {
            OSG_FATAL << "Could not lock PixMap" << std::endl;
            ExitToShell ();
        }
    }
    else
    {
        OSG_FATAL << "Could not GetGWorldPixMap" << std::endl;
        ExitToShell ();
    }

    SetGWorld(origPort, origDevice);

}

void MovieData::setMovieTime(double atime) {
    double time = (atime > getMovieDuration()) ? getMovieDuration() : atime;

    TimeValue t = (TimeValue) (time * _timescale);
    SetMovieTimeValue(_movie,t);
    _checkMovieError("setMovieTime failed");
    UpdateMovie(_movie);
    MoviesTask(_movie,0);


}

void MovieData::setMovieRate(double rate) {
    // OSG_ALWAYS << "new movierate: " << rate << " current: " << getMovieRate() << std::endl;
    _movieRate = rate;
    if ((rate != 0) && (_preRolled == false)) {
        PrerollMovie(_movie, GetMovieTime(_movie,NULL), X2Fix(rate));
        _checkMovieError("PrerollMovie failed");
        _preRolled = true;
    }

    SetMovieRate(_movie, X2Fix(rate));
    _checkMovieError("setMovieRate failed");
    MoviesTask(_movie, 0);
    _checkMovieError("MoviesTask failed");

    UpdateMovie(_movie);
    _checkMovieError("UpdateMovie failed");

}




