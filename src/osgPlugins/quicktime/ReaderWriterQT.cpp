#include "osg/Image"
#include "osg/Notify"

#include <osg/Geode>

#include "osg/GL"

#include "osgDB/FileNameUtils"
#include "osgDB/Registry"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef SEEK_SET
#  define SEEK_SET 0
#endif

#include "QTtexture.h"

using namespace osg;

class ReaderWriterQT : public osgDB::ReaderWriter
{
    public:
        virtual const char* className() { return "Default Quicktime Image Reader/Writer"; }
        
        virtual bool acceptsExtension(const std::string& extension)
        {
            // this should be the only image importer required on the Mac
			// dont know what else it supports, but these will do
            return osgDB::equalCaseInsensitive(extension,"rgb") ||
                osgDB::equalCaseInsensitive(extension,"rgba") ||
                osgDB::equalCaseInsensitive(extension,"jpg") || 
                osgDB::equalCaseInsensitive(extension,"jpeg") ||
                osgDB::equalCaseInsensitive(extension,"tiff") ||               
                osgDB::equalCaseInsensitive(extension,"pict") ||
                osgDB::equalCaseInsensitive(extension,"gif") ||
                osgDB::equalCaseInsensitive(extension,"png") ||
                osgDB::equalCaseInsensitive(extension,"pict") ||
                osgDB::equalCaseInsensitive(extension,"pct") ||
                osgDB::equalCaseInsensitive(extension,"tga");
        }

        virtual ReadResult readImage(const std::string& fileName, const osgDB::ReaderWriter::Options*)
        {					
            std::string ext = osgDB::getFileExtension(fileName);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;
			    
			long origWidth, origHeight,buffWidth,buffHeight,buffDepth,origDepth;
		
			// NOTE - implememntation means that this will always return 32 bits, so it is hard to work out if 
			// an image was monochrome. So it will waste texture memory unless it gets a monochrome hint.
			
			unsigned char *pixels = LoadBufferFromDarwinPath ( fileName.c_str(), &origWidth,&origHeight,&origDepth,
								&buffWidth,&buffHeight,
								&buffDepth);
		
			// IMPORTANT - 
			// origDepth in BYTES, buffDepth in BITS 
			
			if (pixels == 0) {
					std::cerr << "LoadBufferFromDarwinPath failed " << fileName.c_str() << QTfailureMessage() << std::endl;
					return 0;
			}			          
            
			unsigned int pixelFormat;
			
			switch(origDepth) {
				case 1 :
					pixelFormat = GL_RGB;
					break;
				case 2 :
					pixelFormat = GL_LUMINANCE_ALPHA;
					break;
				case 3 :
					pixelFormat = GL_RGB;
					break;
				case 4 :
					pixelFormat = GL_RGBA;
					break;
				default :
					std::cerr << "Unknown file type in " << fileName.c_str() << " with " << origDepth << std::endl;
					pixelFormat = (GLenum)-1;
					return 0;
					break;
			}
					
			{
				unsigned char *srcp=pixels, *dstp=pixels;
          
				int i, j;
          
				// swizzle entire image in-place
				for (i=0; i<buffHeight; i++ ) {
					unsigned char r, g, b, a;
					switch (origDepth) {
					/*
						since 8-bit tgas will get expanded into colour, have to use RGB code for 8-bit images
					
						case 1 :
							for (j=0; j<buffWidth; j++ ) {
								dstp[0]=srcp[2];
								srcp+=4;
								dstp++;
							}
							break;
						*/
						case 2 :
							for (j=0; j<buffWidth; j++ ) {
								dstp[1]=srcp[0];
								dstp[0]=srcp[2];
								srcp+=4;
								dstp+=2;
							}
							break;
						case 1 :
						case 3 :
							for (j=0; j<buffWidth; j++ ) {
								dstp[0]=srcp[1];
								dstp[1]=srcp[2];
								dstp[2]=srcp[3];
								srcp+=4;
								dstp+=3;
							}
							break;
						case 4 :
							for (j=0; j<buffWidth; j++ ) {
								r=srcp[1];
								g=srcp[2];
								b=srcp[3];
								a=srcp[0];
								
								dstp[0]=r;
								dstp[1]=g;
								dstp[2]=b;
								dstp[3]=a;
								
								srcp+=4;
								dstp+=4;
							}
							break;
						default :
							// std::cerr << "ERROR IN RETURNED PIXEL DEPTH, CANNOT COPE" << std::endl;
							return 0;
							break;
					}
				}
			}

            Image* image = new Image();
            image->setFileName(fileName.c_str());
            image->setImage(buffWidth,buffHeight,1,
                buffDepth >> 3,
                pixelFormat,
                GL_UNSIGNED_BYTE,
                pixels,
		        osg::Image::USE_NEW_DELETE );
		        
            notify(INFO) << "image read ok "<<buffWidth<<"  "<<buffHeight<<std::endl;
            return image;
        }

};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterQT> g_readerWriter_QT_Proxy;
