/*
 *  QTUtils.cpp
 *  NativeContext
 *
 *  Created by Stephan Huber on Fri Sep 06 2002.
 *  Copyright (c) 2002 digital mind. All rights reserved.
 *
 */

#include <osg/Notify>
#include <QuickTime/QuickTime.h>
#include <Carbon/Carbon.h>
#include "QTUtils.h"

using namespace std;

namespace osgQuicktime {

    
    void initQuicktime() {

        static bool s_fQuicktimeInited = 0;
        
        OSErr err;
        if (!s_fQuicktimeInited) {
            err = EnterMovies();
            if (err!=0)
               osg::notify(osg::FATAL) << "Error while initializing quicktime: " << err << endl;
            else
               osg::notify(osg::DEBUG_INFO) << "Quicktime initialized successfully"  << endl;
            s_fQuicktimeInited = true;
        }
        else
            osg::notify(osg::DEBUG_INFO) << "Quicktime already initialized ..."  << endl;

    }


    

    // ---------------------------------------------------------------------------
    // MakeFSSPecFromPath
    // wandelt einen Posix-Pfad in ein FSSpec um.
    // ---------------------------------------------------------------------------
    OSStatus MakeFSSpecFromPath(const char* path, FSSpec* spec) {
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
    }

    // ---------------------------------------------------------------------------
    // MakeMovieFromPath
    // erzeugt movie-objekt aus Pfad
    // ---------------------------------------------------------------------------
    OSStatus MakeMovieFromPath(const char* path, Movie* movie) {
        OSStatus err;
        FSSpec   spec;
        short    resref;
        MakeFSSpecFromPath(path, &spec);
        err = OpenMovieFile(&spec, &resref, fsRdPerm);
        if (err!=0) return err;
        err = NewMovieFromFile(movie, resref, NULL, NULL, 0, NULL);
        if (err==0) err=GetMoviesError();
        return err;
    }



} // namespace
