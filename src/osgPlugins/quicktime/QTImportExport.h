/*
 *  QTImportExport.h
 *  cefix
 *
 *  Created by Stephan Huber on 07.02.08.
 *  Copyright 2008 StephanMaximilianHuber, digitalmind. All rights reserved.
 *
 */

#include <osg/Image>

#ifndef QT_IMPORT_EXPORT_HEADER
#define QT_IMPORT_EXPORT_HEADER

/** small helper class handling the im- and export of image data via quicktime */
class QuicktimeImportExport {

    public:
        /** ctor */
        QuicktimeImportExport();

        /** readFromString tries to read a chunk of bytes and interpret it as an image.
         *  @param istream the input stream
         *  @param fileTypeHint you can speed up the conversion by providing a filename with extension, so quicktime has not to guess the image's type
         *  @param sizeHint useful, if you are streaming data, if you provide a sizeHint ony sizeHint bytes are read from the stream
         */
        osg::Image* readFromStream(std::istream & inStream, const std::string& fileTypeHint, long sizeHint = 0);

        /** writes an osg::Image to a stream, using fileTypeHint as a hint whar format you want to write. */
        void writeToStream(std::ostream& outStream, osg::Image* image, const std::string& fileTypeHint) ;

        /** get the last error-message */
        const std::string getLastErrorString() { return _lastError; }

        /** return true if no error occurred */
        bool success() { return (_error == false); }

    protected:

        /** flips an image */
        void flipImage(unsigned char* buffer, int bytesPerPixel, unsigned int width, unsigned height);

        /** do some swizzling, so osg can use the image */
        unsigned char* pepareBufferForOSG(unsigned char * buffer, int bytesPerPixel, unsigned int width, unsigned height);

        /** do some swizzling, so quicktime can use the image */
        unsigned char* prepareBufferForQuicktime(unsigned char* buffer, GLenum pixelFormat, int bytesPerPixel, unsigned int width, unsigned int height)  ;

        /** sets an error-msg */
        void setError(const std::string& msg) { _lastError = msg; _error = true; }

        /** do the import */
        osg::Image* doImport(unsigned char* buffer, unsigned int dataSize, const std::string& fileTypeHint);


    private:
        bool            _error;
        std::string _lastError;


};



#endif