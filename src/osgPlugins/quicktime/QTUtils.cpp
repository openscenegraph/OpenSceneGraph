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



    class QuicktimeInitializer : public osg::Referenced {
        public:
            QuicktimeInitializer() :osg::Referenced() {
               
                 #ifndef __APPLE__
                     InitializeQTML(0);
                 #endif
                 OSErr err = EnterMovies();
                 if (err!=0)
                    osg::notify(osg::FATAL) << "Error while initializing quicktime: " << err << endl; 
                 else
                    osg::notify(osg::DEBUG_INFO) << "Quicktime initialized successfully"  << endl;

                 static bool registered = false;

                 if (!registered){
                  registerQTReader();                  
                 }                 
            }
            
            ~QuicktimeInitializer() {
                #ifndef __APPLE__
                    ExitMovies();
                #endif
                //osg::notify(osg::DEBUG_INFO) << "Quicktime deinitialized successfully"  << endl;
            }

    protected:
        void registerQTReader() {
            osgDB::Registry* r = osgDB::Registry::instance();
         r->addFileExtensionAlias("mov",  "qt");

         #ifdef QT_HANDLE_IMAGES_ALSO
            r->addFileExtensionAlias("jpg",  "qt");
            r->addFileExtensionAlias("jpe",  "qt");
            r->addFileExtensionAlias("jpeg", "qt");
            r->addFileExtensionAlias("tif",  "qt");
            r->addFileExtensionAlias("tiff", "qt");
            r->addFileExtensionAlias("gif",  "qt");
            r->addFileExtensionAlias("png",  "qt");
            r->addFileExtensionAlias("psd",  "qt");
            r->addFileExtensionAlias("tga",  "qt");
            r->addFileExtensionAlias("mov",  "qt");
            r->addFileExtensionAlias("avi",  "qt");
            r->addFileExtensionAlias("mpg",  "qt");
            r->addFileExtensionAlias("mpv",  "qt");
            r->addFileExtensionAlias("dv",   "qt");
            r->addFileExtensionAlias("mp4",  "qt");
            r->addFileExtensionAlias("m4v",  "qt");         
         #endif
        }
                
    };
    
    void initQuicktime(bool erase) {

        static osg::ref_ptr<QuicktimeInitializer> s_qt_init = new QuicktimeInitializer();
        if (erase) {
            s_qt_init = NULL;
        } else if (!s_qt_init.valid())
        {
            s_qt_init = new QuicktimeInitializer();
        }
    }

    
    void exitQuicktime() {
        initQuicktime(true);
    }


    

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



