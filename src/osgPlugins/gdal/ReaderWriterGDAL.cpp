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


            osgDB::ImageOptions::TexCoordRange* texCoordRange = 0;

            const osgDB::ImageOptions* imageOptions = dynamic_cast<const osgDB::ImageOptions*>(options);
            if (imageOptions)
            {
                std::cout<<"Got ImageOptions"<<std::endl;
                
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
                        std::cout<<"tex coord range "<<texCoordRange->_x<<" "<<texCoordRange->_y<<" "<<texCoordRange->_w<<" "<<texCoordRange->_h<<std::endl;
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
            
            int internalFormat = GL_LUMINANCE;
            unsigned int pixelFormat = GL_LUMINANCE;
            unsigned int dataType = 0;
            unsigned int numBytesPerPixel = 0;
            
            GDALDataType targetGDALType = GDT_Byte;

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
                else bandGray = band;
                
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

                    std::cout << "reading RGBA"<<std::endl;

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

                    std::cout << "reading RGB"<<std::endl;

                    bandRed->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+0),destWidth,destHeight,targetGDALType,pixelSpace,lineSpace);
                    bandGreen->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+1),destWidth,destHeight,targetGDALType,pixelSpace,lineSpace);
                    bandBlue->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+2),destWidth,destHeight,targetGDALType,pixelSpace,lineSpace);
                    
                }
            } else if (bandGray)
            {
                if (bandAlpha)
                {
                    // Luminance alpha
                    int pixelSpace=2*numBytesPerPixel;
                    int lineSpace=destWidth * pixelSpace;

                    imageData = new unsigned char[destWidth * destHeight * pixelSpace];
                    pixelFormat = GL_LUMINANCE_ALPHA;
                    internalFormat = GL_LUMINANCE_ALPHA;

                    std::cout << "reading grey + alpha"<<std::endl;

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

                    std::cout << "reading grey"<<std::endl;

                    bandGray->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+0),destWidth,destHeight,targetGDALType,pixelSpace,lineSpace);
                }
            } else if (bandAlpha)
            {
                // alpha map
                int pixelSpace=1*numBytesPerPixel;
                int lineSpace=destWidth * pixelSpace;

                imageData = new unsigned char[destWidth * destHeight * pixelSpace];
                pixelFormat = GL_ALPHA;
                internalFormat = GL_ALPHA;

                std::cout << "reading alpha"<<std::endl;

                bandAlpha->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)(imageData+0),destWidth,destHeight,targetGDALType,pixelSpace,lineSpace);


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
                    
                if (texCoordRange) image->setUserData(texCoordRange);
                
                image->flipVertical();
                
                return image;

            }
            
            return 0;            

        }
        
        virtual ReadResult readHeightField(const std::string& fileName, const osgDB::ReaderWriter::Options* options)
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

            osgDB::ImageOptions::TexCoordRange* texCoordRange = 0;

            const osgDB::ImageOptions* imageOptions = dynamic_cast<const osgDB::ImageOptions*>(options);
            if (imageOptions)
            {
                std::cout<<"Got ImageOptions"<<std::endl;
                
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
                        std::cout<<"tex coord range "<<texCoordRange->_x<<" "<<texCoordRange->_y<<" "<<texCoordRange->_w<<" "<<texCoordRange->_h<<std::endl;
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
                else bandGray = band;
                
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
		
                
//                 unsigned short* data = new unsigned short[destWidth*destHeight];
// 		bandSelected->RasterIO(GF_Read,windowX,windowY,windowWidth,windowHeight,(void*)data,destWidth,destHeight,GDT_UInt16,0,0);
//                 
//                 // copy ushorts across.
// 		for(unsigned int r=0;r<hf->getNumRows();++r)
// 		{
// 		    for(unsigned int c=0;c<hf->getNumColumns();++c)
// 		    {
//                         hf->setHeight(c,r,*data);
//                         ++data;
//                     }
//                 }
                		
		// now need to flip since the OSG's origin is in lower left corner.
                std::cout<<"flipping"<<std::endl;
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
		
		return hf;
	    }
	               
            return ReadResult::FILE_NOT_HANDLED;

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
