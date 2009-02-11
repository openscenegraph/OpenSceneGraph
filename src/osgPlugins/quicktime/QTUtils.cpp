/*
 *  QTUtils.cpp
 *  NativeContext
 *
 *  Created by Stephan Huber on Fri Sep 06 2002.
 *  Copyright (c) 2002 digital mind. All rights reserved.
 *
 */
#include <osg/ref_ptr>
#include <osg/Referenced>
#include <osg/Notify>
#include "QTUtils.h"
#include "osgDB/Registry"


using namespace std;


    // ---------------------------------------------------------------------------
    // MakeFSSPecFromPath
    // wandelt einen Posix-Pfad in ein FSSpec um.
    // ---------------------------------------------------------------------------
    OSStatus MakeFSSpecFromPath(const char* path, FSSpec* spec) {
#ifdef __APPLE__
        OSStatus err;
        FSRef fsref;
        Boolean isdir;
          /*
           FSPathMakeRef is only available in Carbon.  It takes a POSIX path and
           tries to convert it into a MacOS FSRef object.
           We don't want folders, only files, so we'll fail here if we got a
           directory.
           */
        err = FSPathMakeRef((const UInt8*)path, &fsref, &isdir);
        if (err!=0) return err;
        if (isdir) return 1;
          // Ditto
        err = FSGetCatalogInfo(&fsref, kFSCatInfoNone, NULL, NULL, spec, NULL);
        return err;
#else
        return -1;
#endif
    }

    // ---------------------------------------------------------------------------
    // MakeMovieFromPath
    // ---------------------------------------------------------------------------
    OSStatus MakeMovieFromPath(const char* path, Movie* movie, short& resref) {
        OSStatus err;
        FSSpec   spec;
#ifdef __APPLE__
        MakeFSSpecFromPath(path, &spec);
#else
        err = NativePathNameToFSSpec((char*)path, &spec, 0 /* flags */);
#endif
        err = OpenMovieFile(&spec, &resref, fsRdPerm);
        if (err!=0) return err;
        err = NewMovieFromFile(movie, resref, NULL, NULL, 0, NULL);
        if (err==0) err=GetMoviesError();
        return err;
    }



