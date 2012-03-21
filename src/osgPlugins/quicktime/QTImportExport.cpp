/*
 *  QTImportExport.cpp
 *  cefix
 *
 *  Created by Stephan Huber on 07.02.08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#include <map>
#include <sstream>
#include "QTImportExport.h"
#include "QTUtils.h"

#include <osgDB/FileNameUtils>
#include <osg/Image>


/** small exception class bundling a error-message */
class QTImportExportException : public std::exception {

    public:
        QTImportExportException(int err, const std::string& msg) : std::exception(), _err(err), _msg(msg) {}

        virtual const char* what() { return _msg.c_str(); }
        int getErrorCode() { return _err; }

        virtual ~QTImportExportException() throw () {}

    private:
        int _err;
        std::string _msg;
};

QuicktimeImportExport::QuicktimeImportExport()
:    _error(0),
    _lastError("")
{
}


// ----------------------------------------------------------------------------------------------------------
// flipImage
// ----------------------------------------------------------------------------------------------------------

void QuicktimeImportExport::flipImage(unsigned char* pixels, int bytesPerPixel, unsigned int width, unsigned height)
{
    // Flip the image
    unsigned imageSize = width * height * bytesPerPixel;
    char *tBuffer = new char [imageSize];
    unsigned int rowBytes = width * bytesPerPixel;
    unsigned int i,j;
    for (i = 0, j = imageSize - rowBytes; i < imageSize; i += rowBytes, j -= rowBytes)
        memcpy( &tBuffer[j], &pixels[i], (size_t)rowBytes );

    memcpy(pixels, tBuffer, (size_t)imageSize);
    delete[] tBuffer;
}




// ----------------------------------------------------------------------------------------------------------
// prepareBufferForOSG
// ----------------------------------------------------------------------------------------------------------

unsigned char* QuicktimeImportExport::pepareBufferForOSG(unsigned char * buffer, int bytesPerPixel, unsigned int width, unsigned int height)
{
    unsigned char *pixels = new unsigned char [height * width * 4];
    unsigned char *dstp = pixels;
    unsigned char *srcp = buffer;
    unsigned int i, j;

    int roffset, goffset, boffset, aoffset;
    aoffset = -1;
    int sourceStep;

    switch (bytesPerPixel) {
        case 1:
            sourceStep = 1;
            roffset = goffset = boffset = 0;
            break;
        case 3:
            sourceStep = 3;
            roffset = 0;
            goffset = 1;
            boffset = 2;
            break;
        case 4:
            sourceStep = 4;
            aoffset = 1;
            roffset = 2;
            goffset = 3;
            boffset = 0;
            break;

    }

    for (i = 0; i < height; ++i )
    {
        for (j = 0; j < width; ++j )
        {
            dstp[0] = (aoffset < 0) ? 0 : srcp[aoffset];
            dstp[1] = srcp[roffset];
            dstp[2] = srcp[goffset];
            dstp[3] = srcp[boffset];
            srcp+=sourceStep;
            dstp+=4;

        }
    }

    flipImage(pixels, bytesPerPixel, width, height);
    return pixels;

}



// ----------------------------------------------------------------------------------------------------------
// prepareBufferForQuicktime
// ----------------------------------------------------------------------------------------------------------

unsigned char* QuicktimeImportExport::prepareBufferForQuicktime(unsigned char* buffer, GLenum pixelFormat, int bytesPerPixel, unsigned int width, unsigned int height)
{
    unsigned char *pixels = new unsigned char [height * width * 4];
    unsigned char *dstp = pixels;
    unsigned char *srcp = buffer;
    unsigned int i, j;

    int roffset, goffset, boffset, aoffset;
    aoffset = -1;
    int sourceStep;

    switch (bytesPerPixel) {
        case 1:
            sourceStep = 1;
            roffset = goffset = boffset = 0;
            break;
        case 3:
            sourceStep = 3;
            roffset = 0;
            goffset = 1;
            boffset = 2;
            break;
        case 4:
            sourceStep = 4;
            switch (pixelFormat) {
                case GL_RGBA:
                    aoffset = 3;
                    roffset = 0;
                    goffset = 1;
                    boffset = 2;
                    break;

                case GL_BGRA:
                    aoffset = 0;
                    roffset = 1;
                    goffset = 2;
                    boffset = 3;
                    break;
            }
    }


    for (i = 0; i < height; ++i )
    {
        for (j = 0; j < width; ++j )
        {
            dstp[0] = (aoffset < 0) ? 0 : srcp[aoffset];
            dstp[1] = srcp[roffset];
            dstp[2] = srcp[goffset];
            dstp[3] = srcp[boffset];
            srcp+=sourceStep;
            dstp+=4;

        }
    }

    flipImage(pixels, 4, width, height);

    return pixels;

}

// ----------------------------------------------------------------------------------------------------------
// readFromStream
// ----------------------------------------------------------------------------------------------------------

osg::Image* QuicktimeImportExport::readFromStream(std::istream & inStream, const std::string& fileTypeHint, long sizeHint)
{
    char* content = NULL;
    long length = 0;
     if (sizeHint != 0)
    {
        length = sizeHint;
        content = new char[length];
        inStream.read (content,length);
    }
    else
    {
        int readBytes(0), newBytes(0);

        char buffer[10240];

        while (!inStream.eof()) {
            inStream.read(buffer, 10240);
            newBytes = inStream.gcount();
            if (newBytes > 0) {
                char* newcontent = new char[readBytes + newBytes];

                if (readBytes > 0)
                    memcpy(newcontent, content, readBytes);

                memcpy(&newcontent[readBytes], &buffer, newBytes);
                readBytes += newBytes;
                if (content) delete[] content;
                content = newcontent;
            }
        }
        length = readBytes;
    }

    osg::Image* img = doImport(reinterpret_cast<unsigned char*>(content), length, fileTypeHint);

    if (content) delete[] content;
    return img;
 }


Handle getPtrDataRef(unsigned char *data, unsigned int size, const std::string &filename)
{
     // Load Data Reference
     Handle dataRef;
     Handle fileNameHandle;
     PointerDataRefRecord ptrDataRefRec;
     ComponentInstance dataRefHandler;
     unsigned char pstr[255];

     ptrDataRefRec.data = data;
     ptrDataRefRec.dataLength = size;

     /*err = */PtrToHand(&ptrDataRefRec, &dataRef, sizeof(PointerDataRefRecord));

     // Open a Data Handler for the Data Reference
     /*err = */OpenADataHandler(dataRef, PointerDataHandlerSubType, NULL,
         (OSType)0, NULL, kDataHCanRead, &dataRefHandler);

     // Convert From CString in filename to a PascalString in pstr
     if (filename.length() > 255) {
         //hmm...not good, pascal string limit is 255!
         //do some error handling maybe?!
         throw QTImportExportException(0, "filename length limit exceeded");
     }
     CopyCStringToPascal(filename.c_str(), pstr);

    // Add filename extension
     /*err = */PtrToHand(pstr, &fileNameHandle, filename.length() + 1);
     /*err = */DataHSetDataRefExtension(dataRefHandler, fileNameHandle,
         kDataRefExtensionFileName);
     DisposeHandle(fileNameHandle);

     // Release old handler which does not have the extensions
     DisposeHandle(dataRef);

     // Grab the SAFE_NEW version of the data ref from the data handler
     /*err = */ DataHGetDataRef(dataRefHandler, &dataRef);

     CloseComponent(dataRefHandler);

     return dataRef;
}


osg::Image* QuicktimeImportExport::doImport(unsigned char* data, unsigned int sizeData, const std::string& fileTypeHint)
{
    GWorldPtr gworld = 0;
    OSType pixelFormat;
    int rowStride;
    GraphicsImportComponent gicomp = 0;
    Rect rectImage;
    GDHandle origDevice = 0;
    CGrafPtr origPort = 0;
    ImageDescriptionHandle desc = 0;
    int depth = 32;
    unsigned int xsize, ysize;
    unsigned char* imageData = 0;

    // Data Handle for file data ( & load data from file )
    Handle dataRef = getPtrDataRef(data, sizeData, fileTypeHint);

    try {
        OSErr err = noErr;

        // GraphicsImporter - Get Importer for our filetype
        GetGraphicsImporterForDataRef(dataRef, 'ptr ', &gicomp);

        // GWorld - Get Texture Info
        err = GraphicsImportGetNaturalBounds(gicomp, &rectImage);
        if (err != noErr) {
            throw QTImportExportException(err, "GraphicsImportGetNaturalBounds failed");

        }
        xsize = (unsigned int)(rectImage.right - rectImage.left);
        ysize = (unsigned int)(rectImage.bottom - rectImage.top);

        // ImageDescription - Get Image Description
        err = GraphicsImportGetImageDescription(gicomp, &desc);
        if (err != noErr) {
            throw QTImportExportException(err, "GraphicsImportGetImageDescription failed");
        }

        // ImageDescription - Get Bit Depth
        HLock(reinterpret_cast<char **>(desc));


        // GWorld - Pixel Format stuff
        pixelFormat = k32ARGBPixelFormat; // Make sure its forced...NOTE: i'm pretty sure this cannot be RGBA!

        // GWorld - Row stride
        rowStride = xsize * 4; // (width * depth_bpp / 8)

        // GWorld - Allocate output buffer
        imageData = new unsigned char[rowStride * ysize];

        // GWorld - Actually Create IT!
        QTNewGWorldFromPtr(&gworld, pixelFormat, &rectImage, 0, 0, 0, imageData, rowStride);
        if (!gworld) {
            throw QTImportExportException(-1, "QTNewGWorldFromPtr failed");
        }

        // Save old Graphics Device and Graphics Port to reset to later
        GetGWorld (&origPort, &origDevice);

        // GraphicsImporter - Set Destination GWorld (our buffer)
        err = GraphicsImportSetGWorld(gicomp, gworld, 0);
        if (err != noErr) {
            throw QTImportExportException(err, "GraphicsImportSetGWorld failed");
        }

        // GraphicsImporter - Set Quality Level
        err = GraphicsImportSetQuality(gicomp, codecLosslessQuality);
        if (err != noErr) {
            throw QTImportExportException(err, "GraphicsImportSetQuality failed");
        }

        // Lock pixels so that we can draw to our memory texture
        if (!GetGWorldPixMap(gworld) || !LockPixels(GetGWorldPixMap(gworld))) {
            throw QTImportExportException(0, "GetGWorldPixMap failed");
        }


        //*** Draw GWorld into our Memory Texture!
        GraphicsImportDraw(gicomp);

        // Clean up
        UnlockPixels(GetGWorldPixMap(gworld));
        SetGWorld(origPort, origDevice); // set graphics port to offscreen (we don't need it now)
        DisposeGWorld(gworld);
        CloseComponent(gicomp);
        DisposeHandle(reinterpret_cast<char **>(desc));
        DisposeHandle(dataRef);
    }
    catch (QTImportExportException& e)
    {
        setError(e.what());

        if (gworld) {
            UnlockPixels(GetGWorldPixMap(gworld));
            SetGWorld(origPort, origDevice); // set graphics port to offscreen (we don't need it now)
            DisposeGWorld(gworld);
        }
        if (gicomp)
            CloseComponent(gicomp);
        if (desc)
            DisposeHandle(reinterpret_cast<char **>(desc));

        if (imageData)
            delete[] imageData;
        if (dataRef)
            DisposeHandle(dataRef);

        return NULL;
    }



    unsigned int bytesPerPixel = depth / 8;
    unsigned int glpixelFormat;
    switch(bytesPerPixel) {
        case 3 :
            glpixelFormat = GL_RGB;
            break;
        case 4 :
            glpixelFormat = GL_RGBA;
            break;
        default :
            delete[] imageData;
            setError("unknown pixelformat");
            return NULL;
            break;
    }

    unsigned char* swizzled = pepareBufferForOSG(imageData, bytesPerPixel, xsize, ysize);

    delete[] imageData;

    osg::Image* image = new osg::Image();
    image->setFileName(fileTypeHint.c_str());
    image->setImage(xsize,ysize,1,
        bytesPerPixel,
        glpixelFormat,
        GL_UNSIGNED_BYTE,
        swizzled,
        osg::Image::USE_NEW_DELETE );


    return image;
}


 void QuicktimeImportExport::writeToStream(std::ostream& outStream, osg::Image* image, const std::string& fileTypeHint)
 {

    std::string ext = osgDB::getFileExtension(fileTypeHint);
    //Build map  of extension <-> osFileTypes
    static std::map<std::string, OSType> extmap;
    if (extmap.size() == 0) {
        extmap["jpg"]  = kQTFileTypeJPEG;
        extmap["jpeg"] = kQTFileTypeJPEG;
        extmap["bmp"]  = kQTFileTypeBMP;
        extmap["tif"]  = kQTFileTypeTIFF;
        extmap["tiff"] = kQTFileTypeTIFF;
        extmap["png"]  = kQTFileTypePNG;
        extmap["gif"]  = kQTFileTypeGIF;
        extmap["psd"]  = kQTFileTypePhotoShop;
        extmap["sgi"]  = kQTFileTypeSGIImage;
        extmap["rgb"]  = kQTFileTypeSGIImage;
        extmap["rgba"] = kQTFileTypeSGIImage;
    }


    std::map<std::string, OSType>::iterator cur = extmap.find(ext);

    // can not handle this type of file, perhaps a movie?
    if (cur == extmap.end())
        return;

    unsigned int numBytes = image->computeNumComponents(image->getPixelFormat());
    unsigned char* pixels = prepareBufferForQuicktime(
        image->data(),
        image->getPixelFormat(),
        numBytes,
        image->s(),
        image->t()
    );


    OSType desiredType = cur->second;
    GraphicsExportComponent geComp = NULL;
    GWorldPtr gw = 0;
    Handle dataHandle;
    dataHandle = NewHandle(0);

    try {
        OSErr err = OpenADefaultComponent(GraphicsExporterComponentType, desiredType, &geComp);
        Rect bounds = {0,0, image->t(), image->s()};

        err = QTNewGWorldFromPtr(&gw, k32ARGBPixelFormat, &bounds, 0,0,0, pixels, image->s()*4);
        if (err != noErr) {
            throw QTImportExportException(err,  "could not create gworld for type " + ext);
        }

        err = GraphicsExportSetInputGWorld(geComp, gw);
        if (err != noErr) {
            throw QTImportExportException(err, "could not set input gworld for type " + ext);
        }

        err = GraphicsExportSetOutputHandle( geComp, dataHandle);
        if (err != noErr) {
            throw QTImportExportException(err, "could not set output file for type " + ext);
        }

        // Set the compression quality (needed for JPEG, not necessarily for other formats)
        if (desiredType == kQTFileTypeJPEG) {
            err = GraphicsExportSetCompressionQuality(geComp, codecLosslessQuality);
            if (err != noErr) {
                throw QTImportExportException(err, "could not set compression for type " + ext);
            }
        }

        if(4 == numBytes)
        {
            err = GraphicsExportSetDepth( geComp, k32ARGBPixelFormat );    // depth
        }
        // else k24RGBPixelFormat???

        // do the export
        err = GraphicsExportDoExport(geComp, NULL);
        if (err != noErr) {
            throw QTImportExportException(err, "could not do the export for type " + ext);
        }

        if (geComp != NULL)
            CloseComponent(geComp);

        if (gw) DisposeGWorld (gw);
        if (pixels) free(pixels);

        outStream.write(*dataHandle, GetHandleSize(dataHandle));
        DisposeHandle(dataHandle);
    }


    catch (QTImportExportException& e)
    {
        setError(e.what());

        if (geComp != NULL) CloseComponent(geComp);
        if (gw != NULL) DisposeGWorld (gw);
        if (pixels) free(pixels);

        DisposeHandle(dataHandle);

    }

}

