/*
 *  QTUtils.h
 *  NativeContext
 *
 *  Created by Stephan Huber on Fri Sep 06 2002.
 *  Copyright (c) 2002 digital mind. All rights reserved.
 *
 */

#ifndef QTUTILS_HEADER_
#define QTUTILS_HEADER_

// Quicktime plugin is able to load regular 2D images 
// besides movie streams. 
// It is used as default image loader on __APPLE__
// Uncomment this define to use it as image loader
// on other platforms. 
// #define QT_HANDLE_IMAGES_ALSO

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

    extern "C" {
        /** legacy function for Windows */
        inline void GetPortBounds(GWorldPtr gw, Rect* rect) {
            (*rect) = (gw->portRect);
        }
        /** legacy function for Windows */
        inline PixMapHandle GetPortPixMap (CGrafPtr port) {
            return port->portPixMap;
        }
 
    }

#define SetRect MacSetRect
#define OffsetRect MacOffsetRect

#endif
    


#endif
