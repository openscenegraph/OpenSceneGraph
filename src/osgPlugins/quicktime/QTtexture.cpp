/*
 *  QTtexture.c
 *  Cocoa rostrumMIP
 
 *
 *  Created by philatki on Thu Nov 29 2001.
 *  Copyright (c) 2001 __MyCompanyName__. All rights reserved.
 *
 */

/*
PORTIONS OF THIS CODE ARE COPYRIGHT APPLE COMPUTER - 

    Copyright:    Copyright © 2001 Apple Computer, Inc., All Rights Reserved

    Disclaimer:    IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
                ("Apple") in consideration of your agreement to the following terms, and your
                use, installation, modification or redistribution of this Apple software
                constitutes acceptance of these terms.  If you do not agree with these terms,
                please do not use, install, modify or redistribute this Apple software.

                In consideration of your agreement to abide by the following terms, and subject
                to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
                copyrights in this original Apple software (the "Apple Software"), to use,
                reproduce, modify and redistribute the Apple Software, with or without
                modifications, in source and/or binary forms; provided that if you redistribute
                the Apple Software in its entirety and without modifications, you must retain
                this notice and the following text and disclaimers in all such redistributions of
                the Apple Software.  Neither the name, trademarks, service marks or logos of
                Apple Computer, Inc. may be used to endorse or promote products derived from the
                Apple Software without specific prior written permission from Apple.  Except as
                expressly stated in this notice, no other rights or licenses, express or implied,
                are granted by Apple herein, including but not limited to any patent rights that
                may be infringed by your derivative works or by other works in which the Apple
                Software may be incorporated.

                The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
                WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
                WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
                PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
                COMBINATION WITH YOUR PRODUCTS.

                IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
                CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
                GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
                ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
                OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
                (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
                ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <Carbon/Carbon.h>
    
#include <QuickTime/ImageCompression.h> // for image loading and decompression
#include <QuickTime/QuickTimeComponents.h> // for file type support

#include <OpenGL/gl.h> // for OpenGL API
#include <OpenGL/glu.h> // for OpenGL API
#include <OpenGL/glext.h> // for OpenGL extension support 

#include "QTtexture.h"

// ==================================

enum // how to scale image to power of two on read if scaling
{
    kNone = 1,
    kNearest, // find nearest power of 2
    kNearestLess, // nearest power of 2 which is less than or equal image dimension
    KNearestGreater, // nearest power of 2 which is greater than or equal image dimension
    k32, // use this size specifically
    k64,
    k128,
    k256,
    k512,
    k1024,
    k2048,
    k4096,
    k8192
};

// Default values for images loading
short gTextureScale = k1024; // for non-tiled images the type of texture scaling to do
short gMaxTextureSize = 4096; // maximum texture size to use for application
Boolean gfTileTextures = true; // are multiple tiled textures used to display image?
Boolean gfOverlapTextures = true; // do tiled textures overlapped to create correct filtering between tiles? (only applies if using tiled textures)
Boolean gfClientTextures = false; // 10.1+ only: texture from client memory
Boolean gfAGPTextures = false; // 10.1+ only: texture from AGP memory without loading to GPU can be set after inmage loaded
Boolean gfNPOTTextures = false; // 10.1+ only: use Non-Power Of Two (NPOT) textures

// ---------------------------------

static long GetScaledTextureDimFromImageDim (long imageDimension, short scaling);  
static unsigned char * LoadBufferFromImageFile (FSSpec fsspecImage, short imageScale, 
    long * pOrigImageWidth, long * pOrigImageHeight, long *pOrigDepth,
    long * pBufferWidth, long * pBufferHeight, long * pBufferDepth);

// ---------------------------------

// based on scaling determine the texture dimension which fits the image dimension passe in
//  kNone: no scaling just use image dimension (will not guarentee support power for 2 textures)
//  kNearest: find nearest power of 2 texture
//  kNearestLess: find nearest power of 2 texture which is less than image dimension
//  kNearestGreater: find nearest power of 2 texture which is greater than image dimension
//  k32 - k1024: use this specific texture size

static long GetScaledTextureDimFromImageDim (long imageDimension, short scaling)  
{
    switch (scaling)
        {
        case kNone: // no scaling
                    return imageDimension;
                    break;
        case kNearest: // scale to nearest power of two
                {
                    // find power of 2 greater
                    long i = 0, texDim = 1, imageTemp = imageDimension;
                    while (imageTemp >>= 1) // while the dimension still has bits of data shift right (losing a bit at a time)
                        i++; // count shifts (i.e., powers of two)
                    texDim = texDim << i; // shift our one bit representation left the like amount (i.e., 2 ^ i)
                    if (texDim >= gMaxTextureSize) // if we are too big or at max size
                        return gMaxTextureSize; // then must use max texture size
                    // are we closer to greater pow 2 or closer to higher pow 2?
                    // compare to the power of two that is double of initial guess
                    else if (((texDim << 1) - imageDimension) <  (imageDimension - texDim))
                        return (texDim << 1); // if it is closer away then return double guess
                    else
                        return texDim; // otherwise orginal guess is closer so return this
                }
                break;
        case kNearestLess:
                {
                    // find power of 2 lower
                    long i = 0, texDim = 1; 
                    while (imageDimension >>= 1) // while the dimension still has bits of data shift right (losing a bit at a time) 
                        i++; // count shifts (i.e., powers of two)
                    texDim = texDim << i; // shift our one bit representation left the like amount (i.e., 2 ^ i)
                    return texDim; // returns the maxium power of two that is less than or equal the texture dimension
                }
                break;
        case KNearestGreater:
                {
                // find power of 2 greater
                    long i = 0, texDim = 1;
                    while (imageDimension >>= 1) // while the dimension still has bits of data shift right (losing a bit at a time)
                        i++; // count shifts (i.e., powers of two)
                    texDim = texDim << (i + 1); // shift our one bit representation left the like amount (i.e., 2 ^ (i + 1))
                    return texDim; // returns the minimum power of two that is greater than or equal the texture dimension
                }
                break;
        case k32: // return hard values for texture dimension
                        return 32;
                        break;
        case k64:
            return 64;
            break;
        case k128:
            return 128;
            break;
        case k256:
            return 256;
            break;
        case k512:
            return 512;
            break;
        case k1024:
            return 1024;
            break;
        case k2048:
            return 2048;
            break;
        case k4096:
            return 8192;
            break;
        case k8192:
            return 8192;
            break;
    }
    return 0;
}

static char errMess[256];
 
char *QTfailureMessage(void) { return errMess; }

static unsigned char * LoadBufferFromImageFile ( FSSpec fsspecImage, 
        short imageScale,
        long *pOrigImageWidth, long *pOrigImageHeight, long *pOrigImageDepth,
        long *pBufferWidth, long *pBufferHeight, long *pBufferDepth)
{
    unsigned char * pImageBuffer = NULL;
    GWorldPtr pGWorld = NULL;
    OSType pixelFormat;
    long rowStride; // length, in bytes, of a pixel row in the image
    GraphicsImportComponent giComp; // componenet for importing image
    Rect rectImage; // rectangle of source image
        ImageDescriptionHandle hImageDesc; // handle to image description used to get image depth
        MatrixRecord matrix;
    GDHandle origDevice; // save field for current graphics device
    CGrafPtr origPort; // save field for current graphics port
    OSStatus err = noErr; // err return value
    
    // zero output params
    *pOrigImageWidth = 0;
    *pOrigImageHeight = 0;
    *pOrigImageDepth = 0;
    *pBufferWidth = 0;
    *pBufferHeight = 0;
    *pBufferDepth = 0;
        
    // get imorter for the image tyoe in file
    GetGraphicsImporterForFile (&fsspecImage, &giComp);
    if (err != noErr) { // if we have an error
        sprintf ( errMess, "couldnt find importer\n");
        return NULL; // go away
    }
    
    // Create GWorld
    err = GraphicsImportGetNaturalBounds (giComp, &rectImage); // get the image bounds
    if (err != noErr) {
        sprintf ( errMess, "failed to GraphicsImportGetNaturalBounds");

        return NULL; // go away if error
    }
    // create a handle for the image description
    hImageDesc = (ImageDescriptionHandle) NewHandle (sizeof (ImageDescriptionHandle)); 
    HLock ((Handle) hImageDesc); // lock said handle
    err = GraphicsImportGetImageDescription (giComp, &hImageDesc); // retrieve the image description
    if (err != noErr) {
        sprintf ( errMess, "failed to GraphicsImportGetImageDescription");

        return NULL; // go away if error
    }
    *pOrigImageWidth = (long) (rectImage.right - rectImage.left); // find width from right side - left side bounds
    *pOrigImageHeight = (long) (rectImage.bottom - rectImage.top); // same for height

    // we will use a 24-bit rgb texture or a 32-bit rgba
    if ((**hImageDesc).depth == 32) *pOrigImageDepth=4;
    else *pOrigImageDepth=3;

    *pBufferDepth = 32; // we will use a 32 bbit texture (this includes 24 bit images)
    pixelFormat = k32ARGBPixelFormat;

    bool doScaling = false;
    if (doScaling)
    {
        int scalefac;
        // note - we want texels to stay square, so 
        if ((*pOrigImageWidth) > (*pOrigImageHeight))
        {
            *pBufferWidth = GetScaledTextureDimFromImageDim ( *pOrigImageWidth,  imageScale ) ; 
            *pBufferHeight=*pBufferWidth;
            scalefac = X2Fix ((float) (*pBufferWidth) / (float) *pOrigImageWidth);
        }
        else
        {
            *pBufferHeight = GetScaledTextureDimFromImageDim (*pOrigImageHeight, imageScale ); 
            *pBufferWidth = *pBufferHeight;
            scalefac = X2Fix ((float) (*pBufferHeight) / (float) *pOrigImageHeight);
        }
    }
    else
    {
        // NOTE: scaling of the image removed, this is already done inside osg::Image
        *pBufferWidth = *pOrigImageWidth;
        *pBufferHeight= *pOrigImageHeight;
    }
    
    SetRect (&rectImage, 0, 0, (short) *pBufferWidth, (short) *pBufferHeight); // l, t, r. b  set image rectangle for creation of GWorld
    rowStride = *pBufferWidth * *pBufferDepth >> 3; // set stride in bytes width of image * pixel depth in bytes

    const long len = rowStride * *pBufferHeight;

    pImageBuffer = new unsigned char [ len ]; // build new buffer exact size of image (stride * height)

    // pImageBuffer = (unsigned char *) NewPtrClear (rowStride * *pBufferHeight); // build new buffer exact size of image (stride * height)


    if (NULL == pImageBuffer)
    {
        sprintf ( errMess, "failed to allocate image buffer");
        CloseComponent(giComp); // dump component
        return NULL; // if we failed to allocate buffer
        }
    // create a new gworld using our unpadded buffer, ensure we set the pixel type correctly for the expected image bpp
    QTNewGWorldFromPtr (&pGWorld, pixelFormat, &rectImage, NULL, NULL, 0, pImageBuffer, rowStride); 
    if (NULL == pGWorld)
    {
        sprintf ( errMess, "failed to create GWorld");
        // DisposePtr ((Ptr) pImageBuffer); // dump image buffer
        delete [] pImageBuffer;
        pImageBuffer = NULL;
        CloseComponent(giComp);
        return NULL; // if we failed to create gworld
    }
    
    GetGWorld (&origPort, &origDevice); // save onscreen graphics port

    // decompress (draw) to gworld and thus fill buffer
    SetIdentityMatrix (&matrix); // set transform matrix to identity (basically pass through)
    
    TranslateMatrix ( &matrix, -X2Fix(0.5f * *pOrigImageWidth), -X2Fix(0.5f * *pOrigImageHeight));
    ScaleMatrix (&matrix, X2Fix(1.0), X2Fix(-1.0), X2Fix (0.0), X2Fix (0.0));
    TranslateMatrix ( &matrix, X2Fix(0.5f * *pOrigImageWidth), X2Fix(0.5f * *pOrigImageHeight));
    
    err = GraphicsImportSetMatrix(giComp, &matrix); // set our matrix as the importer matrix

    if (err == noErr)
        err = GraphicsImportSetGWorld (giComp, pGWorld, NULL); // set the destination of the importer component
    if (err == noErr)
        err = GraphicsImportSetQuality (giComp, codecLosslessQuality); // we want lossless decompression
    if ((err == noErr) && GetGWorldPixMap (pGWorld) && LockPixels (GetGWorldPixMap (pGWorld)))
        GraphicsImportDraw (giComp); // if everything looks good draw image to locked pixmap
    else
    {
        sprintf ( errMess, "failed to Set Matrix or GWorld or Quality or GetPixMap");

        DisposeGWorld (pGWorld); // dump gworld
        pGWorld = NULL;
        // DisposePtr ((Ptr) pImageBuffer); // dump image buffer
        delete [] pImageBuffer;
        pImageBuffer = NULL;
        CloseComponent(giComp); // dump component
        return NULL;
    }
    
    UnlockPixels (GetGWorldPixMap (pGWorld)); // unlock pixels
    CloseComponent(giComp); // dump component
    SetGWorld(origPort, origDevice); // set current graphics port to offscreen
    // done with gworld and image since they are loaded to a texture
    // DisposeGWorld (pGWorld); // do not need gworld
    // pGWorld = NULL;
        
    return pImageBuffer;
}

// new implementation of darwinPathToFSSpec
// the old code fails for me under os x 10.1.5
// the code below is from an example of apple, modified to fit our needs
// the example can be found at
// <http://developer.apple.com/samplecode/Sample_Code/Files/MoreFilesX.htm>

FSSpec *darwinPathToFSSpec (char *fname ) {

    FSSpec *fs;
    OSStatus    result;
    FSRef    ref;
    
    /* convert the POSIX path to an FSRef */
#if defined( __APPLE__ ) && ( __GNUC__ > 3 )
    result = FSPathMakeRef( (UInt8*)fname, &ref, false); // fname is not a directory
#else
    result = FSPathMakeRef(fname, &ref, false); // fname is not a directory
#endif

    if (result!=0) return NULL;
    
    /* and then convert the FSRef to an FSSpec */
    fs = (FSSpec *) malloc(sizeof(FSSpec));
    result = FSGetCatalogInfo(&ref, kFSCatInfoNone, NULL, NULL, fs, NULL);
    if (result==0) return fs;  // success

    // failed:
    free(fs);
    return NULL;
}


unsigned char*
LoadBufferFromDarwinPath ( const char *fname, long *origWidth, long *origHeight, long *origDepth,
                                long *buffWidth, long *buffHeight,
                                long *buffDepth)
{
    FSSpec *fs;
    
    fs=darwinPathToFSSpec ( const_cast<char*>( fname ) );
    
    if (fs == NULL) {
        sprintf ( errMess, "error creating path from fsspec" );
        return NULL;
    }
    else 
        return LoadBufferFromImageFile ( *fs, kNone, origWidth,origHeight,origDepth,buffWidth,buffHeight,buffDepth);
}
