#include <osg/Image>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/GL>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/ImageOptions>

#include <gdal_priv.h>

class ReaderWriterGDAL : public osgDB::ReaderWriter
{
    public:
        virtual const char* className() { return "GDAL Image Reader"; }
        virtual bool acceptsExtension(const std::string& extension)
        {
            return osgDB::equalCaseInsensitive(extension,"gdal") || osgDB::equalCaseInsensitive(extension,"gdal");
        }

        virtual ReadResult readImage(const std::string& fileName, const osgDB::ReaderWriter::Options* options)
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
            int destWidth = osg::minimum(dataWidth,1024);
            int destHeight = osg::minimum(dataHeight,1024);


            const osgDB::ImageOptions* imageOptions = dynamic_cast<const osgDB::ImageOptions*>(options);
            if (imageOptions)
            {
                std::cout<<"Got ImageOptions"<<std::endl;
                
                switch(imageOptions->_sourceImageWindowMode)
                {
                case(osgDB::ImageOptions::RATIO_WINDOW):
                    windowX = osg::maximum((int)(floor((double)dataWidth * imageOptions->_sourceRatioWindow.windowX)),0);
                    windowY = osg::maximum((int)(floor((double)dataHeight * imageOptions->_sourceRatioWindow.windowY)),0);
                    windowWidth = osg::minimum((int)(ceil((double)dataWidth * (imageOptions->_sourceRatioWindow.windowX + imageOptions->_sourceRatioWindow.windowWidth))),dataWidth)-windowX;
                    windowHeight = osg::minimum((int)(ceil((double)dataHeight * (imageOptions->_sourceRatioWindow.windowY + imageOptions->_sourceRatioWindow.windowHeight))),dataHeight)-windowY;
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

            std::cout << "    windowX = "<<windowX<<std::endl;
            std::cout << "    windowY = "<<windowY<<std::endl;
            std::cout << "    windowWidth = "<<windowWidth<<std::endl;
            std::cout << "    windowHeight = "<<windowHeight<<std::endl;

            std::cout << std::endl;

            std::cout << "    destX = "<<destX<<std::endl;
            std::cout << "    destY = "<<destY<<std::endl;
            std::cout << "    destWidth = "<<destWidth<<std::endl;
            std::cout << "    destHeight = "<<destHeight<<std::endl;

            std::cout << std::endl;
            
            std::cout << "    GetRaterCount() "<< dataset->GetRasterCount()<<std::endl;
            std::cout << "    GetProjectionRef() "<< dataset->GetProjectionRef()<<std::endl;
            
            
            double geoTransform[6];
            if (dataset->GetGeoTransform(geoTransform)!=CE_None)
            {
                std::cout << "    GetGeoTransform "<< std::endl;
                std::cout << "        Origin = "<<geoTransform[0]<<" "<<geoTransform[3]<<std::endl;
                std::cout << "        Pixel X = "<<geoTransform[1]<<" "<<geoTransform[4]<<std::endl;
                std::cout << "        Pixel Y = "<<geoTransform[2]<<" "<<geoTransform[5]<<std::endl;
            }

            int numBands = dataset->GetRasterCount();
            
            
            GDALRasterBand* bandGray = 0;
            GDALRasterBand* bandRed = 0;
            GDALRasterBand* bandGreen = 0;
            GDALRasterBand* bandBlue = 0;
            GDALRasterBand* bandAlpha = 0;
            
            for(int b=1;b<=numBands;++b)
            {
            
                GDALRasterBand* band = dataset->GetRasterBand(b);
                
                std::cout << "    Band "<<b<<std::endl;

                std::cout << "        GetOverviewCount() = "<< band->GetOverviewCount()<<std::endl;
                std::cout << "        GetColorTable() = "<< band->GetColorTable()<<std::endl;
                std::cout << "        DataTypeName() = "<< GDALGetDataTypeName(band->GetRasterDataType())<<std::endl;
                std::cout << "        ColorIntepretationName() = "<< GDALGetColorInterpretationName(band->GetColorInterpretation())<<std::endl;
                
                if (band->GetColorInterpretation()==GCI_GrayIndex) bandGray = band;
                else if (band->GetColorInterpretation()==GCI_RedBand) bandRed = band;
                else if (band->GetColorInterpretation()==GCI_GreenBand) bandGreen = band;
                else if (band->GetColorInterpretation()==GCI_BlueBand) bandBlue = band;
                else if (band->GetColorInterpretation()==GCI_AlphaBand) bandAlpha = band;
                
//                 int gotMin,gotMax;
//                 double minmax[2];
//                 
//                 minmax[0] = band->GetMinimum(&gotMin);
//                 minmax[1] = band->GetMaximum(&gotMax);
//                 if (!(gotMin && gotMax))
//                 {
//                     std::cout<<" computing min max"<<std::endl;
//                     GDALComputeRasterMinMax(band,TRUE,minmax);
//                 }
//                 
//                 std::cout << "        min "<<minmax[0]<<std::endl;
//                 std::cout << "        max "<<minmax[1]<<std::endl;
            }
            
            
            int s = destWidth;
            int t = destHeight;
            int r = 1;

            int internalFormat = GL_LUMINANCE;
            unsigned int pixelFormat = GL_LUMINANCE;
            unsigned int dataType = GL_FLOAT;

            unsigned char* imageData = 0;

            if (bandRed && bandGreen && bandBlue)
            {
                if (bandAlpha)
                {
                    // RGBA

                    int pixelSpace=4;
                    int lineSpace=destWidth * pixelSpace;

                    imageData = new unsigned char[destWidth * destHeight * pixelSpace];
                    pixelFormat = GL_RGBA;
                    internalFormat = GL_RGBA;
                    dataType = GL_UNSIGNED_BYTE;

                    std::cout << "reading RGBA"<<std::endl;

                    bandRed->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+0),destWidth,destHeight,GDT_Byte,pixelSpace,lineSpace);
                    bandGreen->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+1),destWidth,destHeight,GDT_Byte,pixelSpace,lineSpace);
                    bandBlue->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+2),destWidth,destHeight,GDT_Byte,pixelSpace,lineSpace);
                    bandAlpha->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+3),destWidth,destHeight,GDT_Byte,pixelSpace,lineSpace);
                    
                }
                else
                {
                    // RGB

                    int pixelSpace=3;
                    int lineSpace=destWidth * pixelSpace;

                    imageData = new unsigned char[destWidth * destHeight * pixelSpace];
                    pixelFormat = GL_RGB;
                    internalFormat = GL_RGB;
                    dataType = GL_UNSIGNED_BYTE;

                    std::cout << "reading RGB"<<std::endl;

                    bandRed->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+0),destWidth,destHeight,GDT_Byte,pixelSpace,lineSpace);
                    bandGreen->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+1),destWidth,destHeight,GDT_Byte,pixelSpace,lineSpace);
                    bandBlue->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+2),destWidth,destHeight,GDT_Byte,pixelSpace,lineSpace);
                    
                }
            } else if (bandGray)
            {
                if (bandAlpha)
                {
                    // Luminance alpha
                    int pixelSpace=2;
                    int lineSpace=destWidth * pixelSpace;

                    imageData = new unsigned char[destWidth * destHeight * pixelSpace];
                    pixelFormat = GL_LUMINANCE_ALPHA;
                    internalFormat = GL_LUMINANCE_ALPHA;
                    dataType = GL_UNSIGNED_BYTE;

                    std::cout << "reading grey + alpha"<<std::endl;

                    bandGray->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+0),destWidth,destHeight,GDT_Byte,pixelSpace,lineSpace);
                    bandAlpha->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+1),destWidth,destHeight,GDT_Byte,pixelSpace,lineSpace);
                }
                else
                {
                    // Luminance map
                    int pixelSpace=1;
                    int lineSpace=destWidth * pixelSpace;

                    imageData = new unsigned char[destWidth * destHeight * pixelSpace];
                    pixelFormat = GL_LUMINANCE;
                    internalFormat = GL_LUMINANCE;
                    dataType = GL_UNSIGNED_BYTE;

                    std::cout << "reading grey"<<std::endl;

                    bandGray->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+0),destWidth,destHeight,GDT_Byte,pixelSpace,lineSpace);
                }
            } else if (bandAlpha)
            {
                // alpha map (treat as a Luminance map)
                int pixelSpace=1;
                int lineSpace=destWidth * pixelSpace;

                imageData = new unsigned char[destWidth * destHeight * pixelSpace];
                pixelFormat = GL_LUMINANCE;
                internalFormat = GL_LUMINANCE;
                dataType = GL_UNSIGNED_BYTE;

                std::cout << "reading alpha"<<std::endl;

                bandAlpha->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+0),destWidth,destHeight,GDT_Byte,pixelSpace,lineSpace);


            }
            else
            {
                std::cout << "not found any usable bands in file."<<std::endl;
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

                image->flipVertical();
                
                return image;

            }
            
            return 0;            

        }
        
        void initGDAL()
        {
            static bool s_initialized = false;
            if (!s_initialized)
            {
                s_initialized = true;
                GDALAllRegister();
            }
        }
        
        
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterGDAL> g_readerWriter_GDAL_Proxy;
