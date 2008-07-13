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



#include <osg/Image>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/GL>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/ImageOptions>

#include <OpenThreads/ScopedLock>
#include <OpenThreads/ReentrantMutex>

#include <memory>

#include <gdal_priv.h>

#include "DataSetLayer.h"

#define SERIALIZER() OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_serializerMutex)  

// From easyrgb.com
float Hue_2_RGB( float v1, float v2, float vH )
{
   if ( vH < 0 ) vH += 1;
   if ( vH > 1 ) vH -= 1;
   if ( ( 6 * vH ) < 1 ) return ( v1 + ( v2 - v1 ) * 6 * vH );
   if ( ( 2 * vH ) < 1 ) return ( v2 );
   if ( ( 3 * vH ) < 2 ) return ( v1 + ( v2 - v1 ) * ( ( 2 / 3 ) - vH ) * 6 );
   return ( v1 );
}

class ReaderWriterGDAL : public osgDB::ReaderWriter
{
    public:
    
        ReaderWriterGDAL()
        {
            supportsExtension("gdal","GDAL Image reader");
        }
        
        virtual const char* className() const { return "GDAL Image Reader"; }

        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            if (osgDB::equalCaseInsensitive(osgDB::getFileExtension(file),"gdal"))
            {
                return readObject(osgDB::getNameLessExtension(file),options);
            }

            OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_serializerMutex);

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            initGDAL();

            // open a DataSetLayer.
            osg::ref_ptr<GDALPlugin::DataSetLayer> dataset = new GDALPlugin::DataSetLayer(fileName);
            dataset->setGdalReader(this);
            
            if (dataset->isOpen()) return dataset.release();
            
            return ReadResult::FILE_NOT_HANDLED;
        }

        virtual ReadResult readImage(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            if (osgDB::equalCaseInsensitive(osgDB::getFileExtension(fileName),"gdal"))
            {
                return readImage(osgDB::getNameLessExtension(fileName),options);
            }

            OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_serializerMutex);
            return const_cast<ReaderWriterGDAL*>(this)->local_readImage(fileName, options);
        }
        
        virtual ReadResult readHeightField(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            if (osgDB::equalCaseInsensitive(osgDB::getFileExtension(fileName),"gdal"))
            {
                return readHeightField(osgDB::getNameLessExtension(fileName),options);
            }

            OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_serializerMutex);
            return const_cast<ReaderWriterGDAL*>(this)->local_readHeightField(fileName, options);
        }

        virtual ReadResult local_readImage(const std::string& file, const osgDB::ReaderWriter::Options* options)
        {
            // Looks like gdal's GDALRasterBand::GetColorInterpretation()
            // is not giving proper values for ecw images. There is small
            // hack to get around
            bool ecwLoad = osgDB::equalCaseInsensitive(osgDB::getFileExtension(file),"ecw");

            //if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            osg::notify(osg::INFO) << "GDAL : " << file << std::endl;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            initGDAL();

            std::auto_ptr<GDALDataset> dataset((GDALDataset*)GDALOpen(fileName.c_str(),GA_ReadOnly));
            if (!dataset.get()) return ReadResult::FILE_NOT_HANDLED;
            
            int dataWidth = dataset->GetRasterXSize();
            int dataHeight = dataset->GetRasterYSize();
            
            int windowX = 0;
            int windowY = 0;
            int windowWidth = dataWidth;
            int windowHeight = dataHeight;

            int destX = 0;
            int destY = 0;
            int destWidth = osg::minimum(dataWidth,4096);
            int destHeight = osg::minimum(dataHeight,4096);
//             int destWidth = osg::minimum(dataWidth,4096);
//             int destHeight = osg::minimum(dataHeight,4096);


            osgDB::ImageOptions::TexCoordRange* texCoordRange = 0;

            osgDB::ImageOptions* imageOptions = dynamic_cast<osgDB::ImageOptions*>(const_cast<osgDB::ReaderWriter::Options*>(options));
            if (imageOptions)
            {
                osg::notify(osg::INFO)<<"Got ImageOptions"<<std::endl;
                
                int margin = 0;
                switch(imageOptions->_sourceImageWindowMode)
                {
                case(osgDB::ImageOptions::RATIO_WINDOW):
                    {
                        double desiredX = (double)dataWidth * imageOptions->_sourceRatioWindow.windowX;
                        double desiredY = (double)dataHeight * imageOptions->_sourceRatioWindow.windowY;
                        double desiredWidth = (double)dataWidth * imageOptions->_sourceRatioWindow.windowWidth;
                        double desiredHeight = (double)dataHeight * imageOptions->_sourceRatioWindow.windowHeight;
                        
                        windowX = osg::maximum((int)(floor(desiredX))-margin,0);
                        windowY = osg::maximum((int)(floor(desiredY))-margin,0);
                        windowWidth = osg::minimum((int)(ceil(desiredX + desiredWidth))+margin,dataWidth)-windowX;
                        windowHeight = osg::minimum((int)(ceil(desiredY + desiredHeight))+margin,dataHeight)-windowY;
                        
                        texCoordRange = new osgDB::ImageOptions::TexCoordRange;
                        texCoordRange->set((desiredX-(double)windowX)/(double)windowWidth,
                                           ((double)(windowY+windowHeight) -(desiredY+desiredHeight))/(double)windowHeight,
                                           (desiredWidth)/(double)windowWidth,
                                           (desiredHeight)/(double)windowHeight);
                        osg::notify(osg::INFO)<<"tex coord range "<<texCoordRange->_x<<" "<<texCoordRange->_y<<" "<<texCoordRange->_w<<" "<<texCoordRange->_h<<std::endl;
                    }
                    break;
                case(osgDB::ImageOptions::PIXEL_WINDOW):
                    windowX = imageOptions->_sourcePixelWindow.windowX;
                    windowY = imageOptions->_sourcePixelWindow.windowY;
                    windowWidth = imageOptions->_sourcePixelWindow.windowWidth;
                    windowHeight = imageOptions->_sourcePixelWindow.windowHeight;
                    break;
                default:
                    // leave source window dimensions as whole image.
                    break;
                }
                
                // reapply the window coords to the pixel window so that calling code 
                // knows the original pixel size
                imageOptions->_sourcePixelWindow.windowX = windowX;
                imageOptions->_sourcePixelWindow.windowY = windowY;
                imageOptions->_sourcePixelWindow.windowWidth = windowWidth;
                imageOptions->_sourcePixelWindow.windowHeight = windowHeight;

                switch(imageOptions->_destinationImageWindowMode)
                {
                case(osgDB::ImageOptions::RATIO_WINDOW):
                    destX = (unsigned int)(floor((double)dataWidth * imageOptions->_destinationRatioWindow.windowX));
                    destY = (unsigned int)(floor((double)dataHeight * imageOptions->_destinationRatioWindow.windowY));
                    destWidth = (unsigned int)(ceil((double)dataWidth * (imageOptions->_destinationRatioWindow.windowX + imageOptions->_destinationRatioWindow.windowWidth)))-windowX;
                    destHeight = (unsigned int)(ceil((double)dataHeight * (imageOptions->_destinationRatioWindow.windowY + imageOptions->_destinationRatioWindow.windowHeight)))-windowY;
                    break;
                case(osgDB::ImageOptions::PIXEL_WINDOW):
                    destX = imageOptions->_destinationPixelWindow.windowX;
                    destY = imageOptions->_destinationPixelWindow.windowY;
                    destWidth = imageOptions->_destinationPixelWindow.windowWidth;
                    destHeight = imageOptions->_destinationPixelWindow.windowHeight;
                    break;
                default:
                    // leave source window dimensions as whole image.
                    break;
                }

            }            

//             windowX =     0;        
//             windowY =     0; 
//            windowWidth = destWidth;
//            windowHeight = destHeight;

            osg::notify(osg::INFO) << "    windowX = "<<windowX<<std::endl;
            osg::notify(osg::INFO) << "    windowY = "<<windowY<<std::endl;
            osg::notify(osg::INFO) << "    windowWidth = "<<windowWidth<<std::endl;
            osg::notify(osg::INFO) << "    windowHeight = "<<windowHeight<<std::endl;

            osg::notify(osg::INFO) << std::endl;

            osg::notify(osg::INFO) << "    destX = "<<destX<<std::endl;
            osg::notify(osg::INFO) << "    destY = "<<destY<<std::endl;
            osg::notify(osg::INFO) << "    destWidth = "<<destWidth<<std::endl;
            osg::notify(osg::INFO) << "    destHeight = "<<destHeight<<std::endl;

            osg::notify(osg::INFO) << std::endl;
            
            osg::notify(osg::INFO) << "    GetRaterCount() "<< dataset->GetRasterCount()<<std::endl;
            osg::notify(osg::INFO) << "    GetProjectionRef() "<< dataset->GetProjectionRef()<<std::endl;
            
            
            double geoTransform[6];
            if (dataset->GetGeoTransform(geoTransform)==CE_None)
            {
                osg::notify(osg::INFO) << "    GetGeoTransform "<< std::endl;
                osg::notify(osg::INFO) << "        Origin = "<<geoTransform[0]<<" "<<geoTransform[3]<<std::endl;
                osg::notify(osg::INFO) << "        Pixel X = "<<geoTransform[1]<<" "<<geoTransform[4]<<std::endl;
                osg::notify(osg::INFO) << "        Pixel Y = "<<geoTransform[2]<<" "<<geoTransform[5]<<std::endl;
            }

            int numBands = dataset->GetRasterCount();
            
            
            GDALRasterBand* bandGray = 0;
            GDALRasterBand* bandRed = 0;
            GDALRasterBand* bandGreen = 0;
            GDALRasterBand* bandBlue = 0;
            GDALRasterBand* bandAlpha = 0;
            GDALRasterBand* bandPalette = 0;
            
            int internalFormat = GL_LUMINANCE;
            unsigned int pixelFormat = GL_LUMINANCE;
            unsigned int dataType = 0;
            unsigned int numBytesPerPixel = 0;
            
            GDALDataType targetGDALType = GDT_Byte;

            for(int b=1;b<=numBands;++b)
            {
            
                GDALRasterBand* band = dataset->GetRasterBand(b);
                
                osg::notify(osg::INFO) << "    Band "<<b<<std::endl;

                osg::notify(osg::INFO) << "        GetOverviewCount() = "<< band->GetOverviewCount()<<std::endl;
                osg::notify(osg::INFO) << "        GetColorTable() = "<< band->GetColorTable()<<std::endl;
                osg::notify(osg::INFO) << "        DataTypeName() = "<< GDALGetDataTypeName(band->GetRasterDataType())<<std::endl;
                osg::notify(osg::INFO) << "        ColorIntepretationName() = "<< GDALGetColorInterpretationName(band->GetColorInterpretation())<<std::endl;
                
                bool bandNotHandled = true;
                if (ecwLoad)
                {
                    bandNotHandled = false;
                    switch (b)
                    {
                    case 1:
                        bandRed = band;
                        break;
                    case 2:
                        bandGreen = band;
                        break;
                    case 3:
                        bandBlue = band;
                        break;
                    default:
                        bandNotHandled = true;
                    }
                }

                if (bandNotHandled)
                {
                    if (band->GetColorInterpretation()==GCI_GrayIndex) bandGray = band;
                    else if (band->GetColorInterpretation()==GCI_RedBand) bandRed = band;
                    else if (band->GetColorInterpretation()==GCI_GreenBand) bandGreen = band;
                    else if (band->GetColorInterpretation()==GCI_BlueBand) bandBlue = band;
                    else if (band->GetColorInterpretation()==GCI_AlphaBand) bandAlpha = band;
                    else if (band->GetColorInterpretation()==GCI_PaletteIndex) bandPalette = band;
                    else bandGray = band;
                }

                if (bandPalette)
                {
                    osg::notify(osg::INFO) << "        Palette Interpretation: " << GDALGetPaletteInterpretationName(band->GetColorTable()->GetPaletteInterpretation()) << std::endl;
                }
                
//                 int gotMin,gotMax;
//                 double minmax[2];
//                 
//                 minmax[0] = band->GetMinimum(&gotMin);
//                 minmax[1] = band->GetMaximum(&gotMax);
//                 if (!(gotMin && gotMax))
//                 {
//                     osg::notify(osg::INFO)<<" computing min max"<<std::endl;
//                     GDALComputeRasterMinMax(band,TRUE,minmax);
//                 }
//                 
//                 osg::notify(osg::INFO) << "        min "<<minmax[0]<<std::endl;
//                 osg::notify(osg::INFO) << "        max "<<minmax[1]<<std::endl;

                if (dataType==0)
                {
                    targetGDALType = band->GetRasterDataType();
                    switch(band->GetRasterDataType())
                    {
                      case(GDT_Byte): dataType = GL_UNSIGNED_BYTE; numBytesPerPixel = 1; break;
                      case(GDT_UInt16): dataType = GL_UNSIGNED_SHORT; numBytesPerPixel = 2; break;
                      case(GDT_Int16): dataType = GL_SHORT; numBytesPerPixel = 2; break;
                      case(GDT_UInt32): dataType = GL_UNSIGNED_INT; numBytesPerPixel = 4; break;
                      case(GDT_Int32): dataType = GL_INT; numBytesPerPixel = 4; break;
                      case(GDT_Float32): dataType = GL_FLOAT; numBytesPerPixel = 4; break;
                      case(GDT_Float64): dataType = GL_DOUBLE; numBytesPerPixel = 8; break;  // not handled
                      default: dataType = 0; numBytesPerPixel = 0; break; // not handled
                    }
                }
            }
            
            
            int s = destWidth;
            int t = destHeight;
            int r = 1;

            
            if (dataType==0)
            {
                dataType = GL_UNSIGNED_BYTE;
                numBytesPerPixel = 1;
                targetGDALType = GDT_Byte;
            }

            unsigned char* imageData = 0;

            if (bandRed && bandGreen && bandBlue)
            {
                if (bandAlpha)
                {
                    // RGBA

                    int pixelSpace=4*numBytesPerPixel;
                    int lineSpace=destWidth * pixelSpace;

                    imageData = new unsigned char[destWidth * destHeight * pixelSpace];
                    pixelFormat = GL_RGBA;
                    internalFormat = GL_RGBA;

                    osg::notify(osg::INFO) << "reading RGBA"<<std::endl;

                    bandRed->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+0),destWidth,destHeight,targetGDALType,pixelSpace,lineSpace);
                    bandGreen->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+1),destWidth,destHeight,targetGDALType,pixelSpace,lineSpace);
                    bandBlue->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+2),destWidth,destHeight,targetGDALType,pixelSpace,lineSpace);
                    bandAlpha->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+3),destWidth,destHeight,targetGDALType,pixelSpace,lineSpace);
                    
                }
                else
                {
                    // RGB

                    int pixelSpace=3*numBytesPerPixel;
                    int lineSpace=destWidth * pixelSpace;

                    imageData = new unsigned char[destWidth * destHeight * pixelSpace];
                    pixelFormat = GL_RGB;
                    internalFormat = GL_RGB;

                    osg::notify(osg::INFO) << "reading RGB"<<std::endl;

                    bandRed->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+0),destWidth,destHeight,targetGDALType,pixelSpace,lineSpace);
                    bandGreen->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+1),destWidth,destHeight,targetGDALType,pixelSpace,lineSpace);
                    bandBlue->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+2),destWidth,destHeight,targetGDALType,pixelSpace,lineSpace);
                    
                }
            }
            else if (bandGray)
            {
                if (bandAlpha)
                {
                    // Luminance alpha
                    int pixelSpace=2*numBytesPerPixel;
                    int lineSpace=destWidth * pixelSpace;

                    imageData = new unsigned char[destWidth * destHeight * pixelSpace];
                    pixelFormat = GL_LUMINANCE_ALPHA;
                    internalFormat = GL_LUMINANCE_ALPHA;

                    osg::notify(osg::INFO) << "reading grey + alpha"<<std::endl;

                    bandGray->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+0),destWidth,destHeight,targetGDALType,pixelSpace,lineSpace);
                    bandAlpha->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+1),destWidth,destHeight,targetGDALType,pixelSpace,lineSpace);
                }
                else
                {
                    // Luminance map
                    int pixelSpace=1*numBytesPerPixel;
                    int lineSpace=destWidth * pixelSpace;

                    imageData = new unsigned char[destWidth * destHeight * pixelSpace];
                    pixelFormat = GL_LUMINANCE;
                    internalFormat = GL_LUMINANCE;

                    osg::notify(osg::INFO) << "reading grey"<<std::endl;

                    bandGray->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+0),destWidth,destHeight,targetGDALType,pixelSpace,lineSpace);
                }
            }
            else if (bandAlpha)
            {
                // alpha map
                int pixelSpace=1*numBytesPerPixel;
                int lineSpace=destWidth * pixelSpace;

                imageData = new unsigned char[destWidth * destHeight * pixelSpace];
                pixelFormat = GL_ALPHA;
                internalFormat = GL_ALPHA;

                osg::notify(osg::INFO) << "reading alpha"<<std::endl;

                bandAlpha->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+0),destWidth,destHeight,targetGDALType,pixelSpace,lineSpace);

            }
            else if (bandPalette)
            {
                // Paletted map
                int pixelSpace=1*numBytesPerPixel;
                int lineSpace=destWidth * pixelSpace;

                unsigned char *rawImageData;
                rawImageData = new unsigned char[destWidth * destHeight * pixelSpace];
                imageData = new unsigned char[destWidth * destHeight * 4/*RGBA*/];
                pixelFormat = GL_RGBA;
                internalFormat = GL_RGBA;

                osg::notify(osg::INFO) << "reading palette"<<std::endl;
                osg::notify(osg::INFO) << "numBytesPerPixel: " << numBytesPerPixel << std::endl;

                bandPalette->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(rawImageData),destWidth,destHeight,targetGDALType,pixelSpace,lineSpace);
                
                // Map the indexes to an actual RGBA Value.
                for (int i = 0; i < destWidth * destHeight; i++)
                {
                    const GDALColorEntry *colorEntry = bandPalette->GetColorTable()->GetColorEntry(rawImageData[i]);
                    GDALPaletteInterp interp = bandPalette->GetColorTable()->GetPaletteInterpretation();
                    if (!colorEntry)
                    {
                        //FIXME: What to do here?

                        //osg::notify(osg::INFO) << "NO COLOR ENTRY FOR COLOR " << rawImageData[i] << std::endl;
                        imageData[4*i+0] = 255;
                        imageData[4*i+1] = 0;
                        imageData[4*i+2] = 0;
                        imageData[4*i+3] = 1;

                    }
                    else
                    {
                        if (interp == GPI_RGB)
                        {
                            imageData[4*i+0] = colorEntry->c1;
                            imageData[4*i+1] = colorEntry->c2;
                            imageData[4*i+2] = colorEntry->c3;
                            imageData[4*i+3] = colorEntry->c4;
                        }
                        else if (interp == GPI_CMYK)
                        {
                            // from wikipedia.org
                            short C = colorEntry->c1;
                            short M = colorEntry->c2;
                            short Y = colorEntry->c3;
                            short K = colorEntry->c4;
                            imageData[4*i+0] = 255 - C*(255 - K) - K;
                            imageData[4*i+1] = 255 - M*(255 - K) - K;
                            imageData[4*i+2] = 255 - Y*(255 - K) - K;
                            imageData[4*i+3] = 255;
                        }
                        else if (interp == GPI_HLS)
                        {
                            // from easyrgb.com
                            float H = colorEntry->c1;
                            float S = colorEntry->c3;
                            float L = colorEntry->c2;
                            float R, G, B;
                            if ( S == 0 )                       //HSL values = 0 - 1
                            {
                                R = L;                      //RGB results = 0 - 1 
                                G = L;
                                B = L;
                            }
                            else
                            {
                                float var_2, var_1;
                                if ( L < 0.5 )
                                    var_2 = L * ( 1 + S );
                                else
                                    var_2 = ( L + S ) - ( S * L );

                                var_1 = 2 * L - var_2;

                                R = Hue_2_RGB( var_1, var_2, H + ( 1 / 3 ) );
                                G = Hue_2_RGB( var_1, var_2, H );
                                B = Hue_2_RGB( var_1, var_2, H - ( 1 / 3 ) );                                
                            } 
                            imageData[4*i+0] = static_cast<unsigned char>(R*255.0f);
                            imageData[4*i+1] = static_cast<unsigned char>(G*255.0f);
                            imageData[4*i+2] = static_cast<unsigned char>(B*255.0f);
                            imageData[4*i+3] = static_cast<unsigned char>(255.0f);
                        }
                        else if (interp == GPI_Gray)
                        {
                            imageData[4*i+0] = static_cast<unsigned char>(colorEntry->c1*255.0f);
                            imageData[4*i+1] = static_cast<unsigned char>(colorEntry->c1*255.0f);
                            imageData[4*i+2] = static_cast<unsigned char>(colorEntry->c1*255.0f);
                            imageData[4*i+3] = static_cast<unsigned char>(255.0f);
                        }
                    }
                }
                delete [] rawImageData;
            }
            else
            {
                osg::notify(osg::INFO) << "not found any usable bands in file."<<std::endl;
            }
            

            //GDALOpen(dataset);
 
            if (imageData)
            {
                osg::Image* image = new osg::Image;
                image->setFileName(fileName.c_str());
                image->setImage(s,t,r,
                    internalFormat,
                    pixelFormat,
                    dataType,
                    (unsigned char *)imageData,
                    osg::Image::USE_NEW_DELETE);
                    
                if (texCoordRange) image->setUserData(texCoordRange);
                
                image->flipVertical();
                
                return image;

            }
            
            return 0;            

        }
        

        ReadResult local_readHeightField(const std::string& fileName, const osgDB::ReaderWriter::Options* options)
        {
            //std::string ext = osgDB::getFileExtension(fileName);
            //if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            initGDAL();

            std::auto_ptr<GDALDataset> dataset((GDALDataset*)GDALOpen(fileName.c_str(),GA_ReadOnly));
            if (!dataset.get()) return ReadResult::FILE_NOT_HANDLED;
            
            int dataWidth = dataset->GetRasterXSize();
            int dataHeight = dataset->GetRasterYSize();
            
            int windowX = 0;
            int windowY = 0;
            int windowWidth = dataWidth;
            int windowHeight = dataHeight;

            int destX = 0;
            int destY = 0;
            int destWidth = osg::minimum(dataWidth,4096);
            int destHeight = osg::minimum(dataHeight,4096);

            osgDB::ImageOptions::TexCoordRange* texCoordRange = 0;

            const osgDB::ImageOptions* imageOptions = dynamic_cast<const osgDB::ImageOptions*>(options);
            if (imageOptions)
            {
                osg::notify(osg::INFO)<<"Got ImageOptions"<<std::endl;
                
                int margin = 0;
                switch(imageOptions->_sourceImageWindowMode)
                {
                case(osgDB::ImageOptions::RATIO_WINDOW):
                    {
                        double desiredX = (double)dataWidth * imageOptions->_sourceRatioWindow.windowX;
                        double desiredY = (double)dataHeight * imageOptions->_sourceRatioWindow.windowY;
                        double desiredWidth = (double)dataWidth * imageOptions->_sourceRatioWindow.windowWidth;
                        double desiredHeight = (double)dataHeight * imageOptions->_sourceRatioWindow.windowHeight;
                        
                        windowX = osg::maximum((int)(floor(desiredX))-margin,0);
                        windowY = osg::maximum((int)(floor(desiredY))-margin,0);
                        windowWidth = osg::minimum((int)(ceil(desiredX + desiredWidth))+margin,dataWidth)-windowX;
                        windowHeight = osg::minimum((int)(ceil(desiredY + desiredHeight))+margin,dataHeight)-windowY;
                        
                        texCoordRange = new osgDB::ImageOptions::TexCoordRange;
                        texCoordRange->set((desiredX-(double)windowX)/(double)windowWidth,
                                           ((double)(windowY+windowHeight) -(desiredY+desiredHeight))/(double)windowHeight,
                                           (desiredWidth)/(double)windowWidth,
                                           (desiredHeight)/(double)windowHeight);
                        osg::notify(osg::INFO)<<"tex coord range "<<texCoordRange->_x<<" "<<texCoordRange->_y<<" "<<texCoordRange->_w<<" "<<texCoordRange->_h<<std::endl;
                    }
                    break;
                case(osgDB::ImageOptions::PIXEL_WINDOW):
                    windowX = imageOptions->_sourcePixelWindow.windowX;
                    windowY = imageOptions->_sourcePixelWindow.windowY;
                    windowWidth = imageOptions->_sourcePixelWindow.windowWidth;
                    windowHeight = imageOptions->_sourcePixelWindow.windowHeight;
                    break;
                default:
                    // leave source window dimensions as whole image.
                    break;
                }
                
                switch(imageOptions->_destinationImageWindowMode)
                {
                case(osgDB::ImageOptions::RATIO_WINDOW):
                    destX = (unsigned int)(floor((double)dataWidth * imageOptions->_destinationRatioWindow.windowX));
                    destY = (unsigned int)(floor((double)dataHeight * imageOptions->_destinationRatioWindow.windowY));
                    destWidth = (unsigned int)(ceil((double)dataWidth * (imageOptions->_destinationRatioWindow.windowX + imageOptions->_destinationRatioWindow.windowWidth)))-windowX;
                    destHeight = (unsigned int)(ceil((double)dataHeight * (imageOptions->_destinationRatioWindow.windowY + imageOptions->_destinationRatioWindow.windowHeight)))-windowY;
                    break;
                case(osgDB::ImageOptions::PIXEL_WINDOW):
                    destX = imageOptions->_destinationPixelWindow.windowX;
                    destY = imageOptions->_destinationPixelWindow.windowY;
                    destWidth = imageOptions->_destinationPixelWindow.windowWidth;
                    destHeight = imageOptions->_destinationPixelWindow.windowHeight;
                    break;
                default:
                    // leave source window dimensions as whole image.
                    break;
                }

            }            

//             windowX =     0;        
//             windowY =     0; 
//            windowWidth = destWidth;
//            windowHeight = destHeight;

            osg::notify(osg::INFO) << "    windowX = "<<windowX<<std::endl;
            osg::notify(osg::INFO) << "    windowY = "<<windowY<<std::endl;
            osg::notify(osg::INFO) << "    windowWidth = "<<windowWidth<<std::endl;
            osg::notify(osg::INFO) << "    windowHeight = "<<windowHeight<<std::endl;

            osg::notify(osg::INFO) << std::endl;

            osg::notify(osg::INFO) << "    destX = "<<destX<<std::endl;
            osg::notify(osg::INFO) << "    destY = "<<destY<<std::endl;
            osg::notify(osg::INFO) << "    destWidth = "<<destWidth<<std::endl;
            osg::notify(osg::INFO) << "    destHeight = "<<destHeight<<std::endl;

            osg::notify(osg::INFO) << std::endl;
            
            osg::notify(osg::INFO) << "    GetRaterCount() "<< dataset->GetRasterCount()<<std::endl;
            osg::notify(osg::INFO) << "    GetProjectionRef() "<< dataset->GetProjectionRef()<<std::endl;
            
            
            double geoTransform[6];
            osg::notify(osg::INFO) << "   GetGeoTransform == "<< dataset->GetGeoTransform(geoTransform)<<" == CE_None"<<std::endl;
            osg::notify(osg::INFO) << "    GetGeoTransform "<< std::endl;
            osg::notify(osg::INFO) << "        Origin = "<<geoTransform[0]<<" "<<geoTransform[3]<<std::endl;
            osg::notify(osg::INFO) << "        Pixel X = "<<geoTransform[1]<<" "<<geoTransform[4]<<std::endl;
            osg::notify(osg::INFO) << "        Pixel Y = "<<geoTransform[2]<<" "<<geoTransform[5]<<std::endl;
            

            double TopLeft[2],BottomLeft[2],BottomRight[2],TopRight[2];
            TopLeft[0] = geoTransform[0];
            TopLeft[1] = geoTransform[3];
            BottomLeft[0] = TopLeft[0]+geoTransform[2]*(dataHeight-1);
            BottomLeft[1] = TopLeft[1]+geoTransform[5]*(dataHeight-1);
            BottomRight[0] = BottomLeft[0]+geoTransform[1]*(dataWidth-1);
            BottomRight[1] = BottomLeft[1]+geoTransform[4]*(dataWidth-1);
            TopRight[0] = TopLeft[0]+geoTransform[1]*(dataWidth-1);
            TopRight[1] = TopLeft[1]+geoTransform[4]*(dataWidth-1);
            

            double rotation = atan2(geoTransform[2], geoTransform[1]);
            osg::notify(osg::INFO)<<"GDAL rotation = "<<rotation<<std::endl;
            
            osg::notify(osg::INFO) << "TopLeft     "<<TopLeft[0]<<"\t"<<TopLeft[1]<<std::endl;
            osg::notify(osg::INFO) << "BottomLeft  "<<BottomLeft[0]<<"\t"<<BottomLeft[1]<<std::endl;
            osg::notify(osg::INFO) << "BottomRight "<<BottomRight[0]<<"\t"<<BottomRight[1]<<std::endl;
            osg::notify(osg::INFO) << "TopRight    "<<TopRight[0]<<"\t"<<TopRight[1]<<std::endl;

            osg::notify(osg::INFO)<<"    GDALGetGCPCount "<<dataset->GetGCPCount()<<std::endl;

            int numBands = dataset->GetRasterCount();
            
            
            GDALRasterBand* bandGray = 0;
            GDALRasterBand* bandRed = 0;
            GDALRasterBand* bandGreen = 0;
            GDALRasterBand* bandBlue = 0;
            GDALRasterBand* bandAlpha = 0;
            
            for(int b=1;b<=numBands;++b)
            {
            
                GDALRasterBand* band = dataset->GetRasterBand(b);
                
                osg::notify(osg::INFO) << "    Band "<<b<<std::endl;

                osg::notify(osg::INFO) << "        GetOverviewCount() = "<< band->GetOverviewCount()<<std::endl;
                osg::notify(osg::INFO) << "        GetColorTable() = "<< band->GetColorTable()<<std::endl;
                osg::notify(osg::INFO) << "        DataTypeName() = "<< GDALGetDataTypeName(band->GetRasterDataType())<<std::endl;
                osg::notify(osg::INFO) << "        ColorIntepretationName() = "<< GDALGetColorInterpretationName(band->GetColorInterpretation())<<std::endl;
                
                
                osg::notify(osg::INFO) << std::endl;
                osg::notify(osg::INFO) << "        GetNoDataValue() = "<< band->GetNoDataValue()<<std::endl;
                osg::notify(osg::INFO) << "        GetMinimum() = "<< band->GetMinimum()<<std::endl;
                osg::notify(osg::INFO) << "        GetMaximum() = "<< band->GetMaximum()<<std::endl;
                osg::notify(osg::INFO) << "        GetOffset() = "<< band->GetOffset()<<std::endl;
                osg::notify(osg::INFO) << "        GetScale() = "<< band->GetScale()<<std::endl;
                osg::notify(osg::INFO) << "        GetUnitType() = '"<< band->GetUnitType()<<"'"<<std::endl;
                
                
                if (band->GetColorInterpretation()==GCI_GrayIndex) bandGray = band;
                else if (band->GetColorInterpretation()==GCI_RedBand) bandRed = band;
                else if (band->GetColorInterpretation()==GCI_GreenBand) bandGreen = band;
                else if (band->GetColorInterpretation()==GCI_BlueBand) bandBlue = band;
                else if (band->GetColorInterpretation()==GCI_AlphaBand) bandAlpha = band;
                else bandGray = band;
                
            }
            
                
            GDALRasterBand* bandSelected = 0;
            if (!bandSelected && bandGray) bandSelected = bandGray;
            else if (!bandSelected && bandAlpha) bandSelected = bandAlpha;
            else if (!bandSelected && bandRed) bandSelected = bandRed;
            else if (!bandSelected && bandGreen) bandSelected = bandGreen;
            else if (!bandSelected && bandBlue) bandSelected = bandBlue;

            if (bandSelected)
            {
                osg::HeightField* hf = new osg::HeightField;
                hf->allocate(destWidth,destHeight);

                bandSelected->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(&(hf->getHeightList().front())),destWidth,destHeight,GDT_Float32,0,0);

                // now need to flip since the OSG's origin is in lower left corner.
                osg::notify(osg::INFO)<<"flipping"<<std::endl;
                unsigned int copy_r = hf->getNumRows()-1;
                for(unsigned int r=0;r<copy_r;++r,--copy_r)
                {
                    for(unsigned int c=0;c<hf->getNumColumns();++c)
                    {
                        float temp = hf->getHeight(c,r);
                        hf->setHeight(c,r,hf->getHeight(c,copy_r));
                        hf->setHeight(c,copy_r,temp);
                    }
                }
                hf->setOrigin(osg::Vec3(BottomLeft[0],BottomLeft[1],0));
                
                hf->setXInterval(sqrt(geoTransform[1]*geoTransform[1] + geoTransform[2]*geoTransform[2]));
                hf->setYInterval(sqrt(geoTransform[4]*geoTransform[4] + geoTransform[5]*geoTransform[5]));
                
                hf->setRotation(osg::Quat(rotation, osg::Vec3d(0.0, 0.0, 1.0)));

                return hf;
            }
                   
            return ReadResult::FILE_NOT_HANDLED;

        }

        void initGDAL() const
        {
            static bool s_initialized = false;
            if (!s_initialized)
            {
                s_initialized = true;
                GDALAllRegister();
            }
        }
        
        mutable OpenThreads::ReentrantMutex _serializerMutex;
        
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterGDAL> g_readerWriter_GDAL_Proxy;
