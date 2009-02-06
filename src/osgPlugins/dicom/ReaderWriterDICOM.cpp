// Released under the OSGPL license, as part of the OpenSceneGraph distribution.
//
// ReaderWriter for sgi's .rgb format.
// specification can be found at http://local.wasp.uwa.edu.au/~pbourke/dataformats/sgirgb/sgiversion.html

#include <osg/Image>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/GL>
#include <osg/io_utils>
#include <osg/ImageUtils>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>

#include <osgVolume/Volume>
#include <osgVolume/VolumeTile>
#include <osgVolume/RayTracedTechnique>

#ifdef  USE_DCMTK
    #define HAVE_CONFIG_H
    #include <dcmtk/config/osconfig.h> 
    #include <dcmtk/dcmdata/dcfilefo.h>
    #include <dcmtk/dcmdata/dcdeftag.h>
    #include <dcmtk/dcmimgle/dcmimage.h>
#endif

#ifdef USE_ITK
    #include <itkImageFileReader.h>
    #include <itkImageFileWriter.h>
    #include <itkImage.h>
    #include <itkImageRegionConstIterator.h>
    #include <itkMetaDataDictionary.h>
    #include <itkMetaDataObject.h>
    #include <itkGDCMImageIO.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <memory>

class ReaderWriterDICOM : public osgDB::ReaderWriter
{
    public:
    
        ReaderWriterDICOM()
        {
            supportsExtension("mag","dicom image format");
            supportsExtension("ph","dicom image format");
            supportsExtension("ima","dicom image format");
            supportsExtension("dic","dicom image format");
            supportsExtension("dcm","dicom image format");
            supportsExtension("dicom","dicom image format");
            // supportsExtension("*","dicom image format");
        }

        std::ostream& warning() const { return osg::notify(osg::WARN); }
        std::ostream& notice() const { return osg::notify(osg::NOTICE); }
        std::ostream& info() const { return osg::notify(osg::NOTICE); }
        
        template<typename T>
        T* readData(std::istream& fin, unsigned int length, unsigned int& numComponents) const
        {
            numComponents = length/sizeof(T);
            T* data = new T[numComponents];
            fin.read((char*)data, numComponents*sizeof(T));
            
            // read over any padding
            length -= numComponents*sizeof(T);
            while(fin && length>0) { fin.get(); --length; }
            
            return data;
        }

        template<typename T>
        void printData(std::ostream& out, T* data, unsigned int numComponents) const
        {
            if (sizeof(T)==1)
            {
                for(unsigned int i=0; i<numComponents; ++i)
                {
                    if (data[i]>32) out<<data[i];
                    else out<<".";
                }
            }
            else 
            {                
                for(unsigned int i=0; i<numComponents; ++i)
                {
                    if (i==0) out<<data[i];
                    else out<<", "<<data[i];
                }
            }
        }
    
        virtual const char* className() const { return "DICOM Image Reader/Writer"; }
        
        virtual ReadResult readObject(std::istream& fin,const osgDB::ReaderWriter::Options* options =NULL) const
        {
            return readImage(fin, options);
        }

        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options =NULL) const
        {
            return readImage(file, options);
        }

        virtual ReadResult readNode(std::istream& fin,const osgDB::ReaderWriter::Options* options =NULL) const
        {
            return readImage(fin, options);
        }

        virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options =NULL) const
        {
            ReadResult result = readImage(file, options);
            if (!result.validImage()) return result;
            
            osg::ref_ptr<osgVolume::VolumeTile> tile = new osgVolume::VolumeTile;
            tile->setVolumeTechnique(new osgVolume::RayTracedTechnique());
            
            osg::ref_ptr<osgVolume::ImageLayer> layer= new osgVolume::ImageLayer(result.getImage());
            layer->rescaleToZeroToOneRange();
            
            osgVolume::SwitchProperty* sp = new osgVolume::SwitchProperty;
            sp->setActiveProperty(0);
            
            float alphaFunc = 0.1f;

            osgVolume::AlphaFuncProperty* ap = new osgVolume::AlphaFuncProperty(alphaFunc);
            osgVolume::SampleDensityProperty* sd = new osgVolume::SampleDensityProperty(0.005);
            osgVolume::TransparencyProperty* tp = new osgVolume::TransparencyProperty(1.0);

            {
                // Standard
                osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
                cp->addProperty(ap);
                cp->addProperty(sd);
                cp->addProperty(tp);

                sp->addProperty(cp);
            }

            {
                // Light
                osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
                cp->addProperty(ap);
                cp->addProperty(sd);
                cp->addProperty(tp);
                cp->addProperty(new osgVolume::LightingProperty);

                sp->addProperty(cp);
            }

            {
                // Isosurface
                osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
                cp->addProperty(sd);
                cp->addProperty(tp);
                cp->addProperty(new osgVolume::IsoSurfaceProperty(alphaFunc));

                sp->addProperty(cp);
            }

            {
                // MaximumIntensityProjection
                osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
                cp->addProperty(ap);
                cp->addProperty(sd);
                cp->addProperty(tp);
                cp->addProperty(new osgVolume::MaximumIntensityProjectionProperty);

                sp->addProperty(cp);
            }

            layer->addProperty(sp);

            tile->setLayer(layer.get());

            // get matrix providing size of texels (in mm)
            osg::RefMatrix* matrix = dynamic_cast<osg::RefMatrix*>(result.getImage()->getUserData());
        
            if (matrix)
            {
                osgVolume::Locator* locator = new osgVolume::Locator(*matrix);

                tile->setLocator(locator);
                layer->setLocator(locator);
                
                // result.getImage()->setUserData(0);
                
                osg::notify(osg::NOTICE)<<"Locator "<<*matrix<<std::endl;
            }
            else
            {
                osg::notify(osg::NOTICE)<<"No Locator found on osg::Image"<<std::endl;
            }
            
            return tile.release();
        }


        virtual ReadResult readImage(std::istream& fin,const osgDB::ReaderWriter::Options*) const
        {
            return 0;
        }


#ifdef USE_ITK
        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            notice()<<"Reading DICOM file "<<fileName<<std::endl;

            typedef unsigned short PixelType;
            const unsigned int Dimension = 3;
            typedef itk::Image< PixelType, Dimension > ImageType;


            typedef itk::ImageFileReader< ImageType > ReaderType;
            ReaderType::Pointer reader = ReaderType::New();
            reader->SetFileName( fileName.c_str() );

            typedef itk::GDCMImageIO           ImageIOType;
            ImageIOType::Pointer gdcmImageIO = ImageIOType::New();
            reader->SetImageIO( gdcmImageIO );

            try
            {
                reader->Update();
            }
            catch (itk::ExceptionObject & e)
            {
                std::cerr << "exception in file reader " << std::endl;
                std::cerr << e.GetDescription() << std::endl;
                std::cerr << e.GetLocation() << std::endl;
                return ReadResult::ERROR_IN_READING_FILE;
            }
            
            ImageType::Pointer inputImage = reader->GetOutput();
            
            ImageType::RegionType region = inputImage->GetBufferedRegion();
            ImageType::SizeType size = region.GetSize();
            ImageType::IndexType start = region.GetIndex();
            
            //inputImage->GetSpacing();
            //inputImage->GetOrigin();
            
            unsigned int width = size[0];
            unsigned int height = size[1];
            unsigned int depth = size[2];
            
            osg::RefMatrix* matrix = new osg::RefMatrix;
            
            notice()<<"width = "<<width<<" height = "<<height<<" depth = "<<depth<<std::endl;
            for(unsigned int i=0; i<Dimension; ++i)
            {
                (*matrix)(i,i) = inputImage->GetSpacing()[i];
                (*matrix)(3,i) = inputImage->GetOrigin()[i];
            }
            
            osg::Image* image = new osg::Image;
            image->allocateImage(width, height, depth, GL_LUMINANCE, GL_UNSIGNED_BYTE, 1);

            unsigned char* data = image->data();            
            typedef itk::ImageRegionConstIterator< ImageType > IteratorType;
            IteratorType it(inputImage, region);
            
            it.GoToBegin();
            while (!it.IsAtEnd())
            {
                *data = it.Get();
                ++data;
                ++it;
            }
            
            image->setUserData(matrix);
            
            matrix->preMult(osg::Matrix::scale(double(image->s()), double(image->t()), double(image->r())));

            return image;
        }
#endif

#ifdef USE_DCMTK

        void convertPixelTypes(const DiPixel* pixelData, 
                               EP_Representation& pixelRep, int& numPlanes, 
                               GLenum& dataType, GLenum& pixelFormat, unsigned int& pixelSize) const
        {
            dataType = GL_UNSIGNED_BYTE;
            pixelRep = pixelData->getRepresentation();
            switch(pixelRep)
            {
                case(EPR_Uint8):
                    dataType = GL_UNSIGNED_BYTE;
                    pixelSize = 1;
                    break;
                case(EPR_Sint8):
                    dataType = GL_BYTE;
                    pixelSize = 1;
                    break;
                case(EPR_Uint16):
                    dataType = GL_UNSIGNED_SHORT;
                    pixelSize = 2;
                    break;
                case(EPR_Sint16):
                    dataType = GL_SHORT;
                    pixelSize = 2;
                    break;
                case(EPR_Uint32):
                    dataType = GL_UNSIGNED_INT;
                    pixelSize = 4;
                    break;
                case(EPR_Sint32):
                    dataType = GL_INT;
                    pixelSize = 4;
                    break;
                default:
                    dataType = 0;
                    break;
            }

            pixelFormat = GL_INTENSITY;
            numPlanes = pixelData->getPlanes();
            switch(numPlanes)
            {
                case(1):
                    pixelFormat = GL_LUMINANCE;
                    break;
                case(2):
                    pixelFormat = GL_LUMINANCE_ALPHA;
                    pixelSize *= 2;
                    break;
                case(3):
                    pixelFormat = GL_RGB;
                    pixelSize *= 3;
                    break;
                case(4):
                    pixelFormat = GL_RGBA;
                    pixelSize *= 4;
                    break;
            }
        }
        

        typedef std::vector<std::string> Files;
        bool getDicomFilesInDirectory(const std::string& path, Files& files) const
        {
            osgDB::DirectoryContents contents = osgDB::getDirectoryContents(path);

            std::sort(contents.begin(), contents.end());

            for(osgDB::DirectoryContents::iterator itr = contents.begin();
                itr != contents.end();
                ++itr)
            {
                std::string localFile = path + "/" + *itr;
                info()<<"contents = "<<localFile<<std::endl;
                if (acceptsExtension(osgDB::getLowerCaseFileExtension(localFile)) &&
                    osgDB::fileType(localFile) == osgDB::REGULAR_FILE)
                {
                    files.push_back(localFile);
                }
            }
            
            return !files.empty();
        }


        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            notice()<<"Reading DICOM file "<<file<<" using DCMTK"<<std::endl;

            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;
            
            std::string fileName = file;
            if (ext=="dicom")
            {
                fileName = osgDB::getNameLessExtension(file);
            }

            fileName = osgDB::findDataFile( fileName, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            Files files;
            
            osgDB::FileType fileType = osgDB::fileType(fileName);
            if (fileType==osgDB::DIRECTORY)
            {
                getDicomFilesInDirectory(fileName, files);
            }
            else
            {
#if 1
                files.push_back(fileName);
#else                            
                if (!getDicomFilesInDirectory(osgDB::getFilePath(fileName), files))
                {
                    files.push_back(fileName);
                }
#endif            
            }

            if (files.empty())
            {
                return ReadResult::FILE_NOT_FOUND;
            }

            osg::ref_ptr<osg::RefMatrix> matrix = new osg::RefMatrix;
            osg::ref_ptr<osg::Image> image;
            unsigned int imageNum = 0;
            EP_Representation pixelRep = EPR_Uint8;
            int numPlanes = 0;
            GLenum pixelFormat = 0;
            GLenum dataType = 0;
            unsigned int pixelSize = 0;
            
            typedef std::list<FileInfo> FileInfoList;
            FileInfoList fileInfoList;

            typedef std::map<double, FileInfo> DistanceFileInfoMap;
            typedef std::map<osg::Vec3d, DistanceFileInfoMap> OrientationFileInfoMap;
            OrientationFileInfoMap orientationFileInfoMap;
            
            unsigned int totalNumSlices = 0;

            for(Files::iterator itr = files.begin();
                itr != files.end();
                ++itr)
            {
                DcmFileFormat fileformat;
                OFCondition status = fileformat.loadFile((*itr).c_str());
                if(!status.good()) return ReadResult::ERROR_IN_READING_FILE;
                
                FileInfo fileInfo;
                fileInfo.filename = *itr;

                double pixelSize_y = 1.0;
                double pixelSize_x = 1.0;
                double sliceThickness = 1.0;
                double imagePositionPatient[3] = {0, 0, 0};
                double imageOrientationPatient[6] = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0 };
                Uint16 numOfSlices = 1;
                
                double value = 0.0;
                if (fileformat.getDataset()->findAndGetFloat64(DCM_PixelSpacing, value,0).good())
                {
                    pixelSize_y = value;
                    fileInfo.matrix(1,1) = pixelSize_y;
                }

                if (fileformat.getDataset()->findAndGetFloat64(DCM_PixelSpacing, value,1).good())
                {
                    pixelSize_x = value;
                    fileInfo.matrix(0,0) = pixelSize_x;
                }

                // Get slice thickness
                if (fileformat.getDataset()->findAndGetFloat64(DCM_SliceThickness, value).good())
                {
                    sliceThickness = value;
                    notice()<<"sliceThickness = "<<sliceThickness<<std::endl;
                    fileInfo.sliceThickness = sliceThickness;
                }
    
                notice()<<"tagExistsWithValue(DCM_NumberOfFrames)="<<fileformat.getDataset()->tagExistsWithValue(DCM_NumberOfFrames)<<std::endl;
                notice()<<"tagExistsWithValue(DCM_NumberOfSlices)="<<fileformat.getDataset()->tagExistsWithValue(DCM_NumberOfSlices)<<std::endl;

                Uint32 numFrames;
                if (fileformat.getDataset()->findAndGetUint32(DCM_NumberOfFrames, numFrames).good())
                {
                    fileInfo.numSlices = numFrames;
                    notice()<<"Read number of frames = "<<numFrames<<std::endl;
                }

                
                OFString numFramesStr;
                if (fileformat.getDataset()->findAndGetOFString(DCM_NumberOfFrames, numFramesStr).good())
                {
                    fileInfo.numSlices = atoi(numFramesStr.c_str());
                    notice()<<"Read number of frames = "<<numFramesStr<<std::endl;
                }

                if (fileformat.getDataset()->findAndGetUint16(DCM_NumberOfFrames, numOfSlices).good())
                {
                    fileInfo.numSlices = numOfSlices;
                    notice()<<"Read number of frames = "<<numOfSlices<<std::endl;
                }
                
                if (fileformat.getDataset()->findAndGetUint16(DCM_NumberOfSlices, numOfSlices).good())
                {
                    fileInfo.numSlices = numOfSlices;
                    notice()<<"Read number of slices = "<<numOfSlices<<std::endl;
                }



                // patient position
                for(int i=0; i<3; ++i)
                {
                    if (fileformat.getDataset()->findAndGetFloat64(DCM_ImagePositionPatient, imagePositionPatient[i],i).good())
                    {
                        notice()<<"Read DCM_ImagePositionPatient["<<i<<"], "<<imagePositionPatient[i]<<std::endl;
                    }
                    else
                    {
                        notice()<<"Have not read DCM_ImagePositionPatient["<<i<<"]"<<std::endl;
                    }
                }
                //notice()<<"imagePositionPatient[2]="<<imagePositionPatient[2]<<std::endl;
                
                fileInfo.matrix.setTrans(imagePositionPatient[0],imagePositionPatient[1],imagePositionPatient[2]);

                for(int i=0; i<6; ++i)
                {
                    double value = 0.0;
                    if (fileformat.getDataset()->findAndGetFloat64(DCM_ImageOrientationPatient, value,i).good())
                    {
                        imageOrientationPatient[i] = value;
                        notice()<<"Read imageOrientationPatient["<<i<<"], "<<imageOrientationPatient[i]<<std::endl;
                    }
                    else
                    {
                        notice()<<"Have not read imageOrientationPatient["<<i<<"]"<<std::endl;
                    }
                }

                osg::Vec3d dirX(imageOrientationPatient[0],imageOrientationPatient[1],imageOrientationPatient[2]);
                osg::Vec3d dirY(imageOrientationPatient[3],imageOrientationPatient[4],imageOrientationPatient[5]);
                osg::Vec3d dirZ = dirX ^ dirY;
                dirZ.normalize();
                
                dirX *= pixelSize_x;
                dirY *= pixelSize_y;
                
                fileInfo.matrix(0,0) = dirX[0];
                fileInfo.matrix(1,0) = dirX[1];
                fileInfo.matrix(2,0) = dirX[2];
                
                fileInfo.matrix(0,1) = dirY[0];
                fileInfo.matrix(1,1) = dirY[1];
                fileInfo.matrix(2,1) = dirY[2];
                
                fileInfo.matrix(0,2) = dirZ[0];
                fileInfo.matrix(1,2) = dirZ[1];
                fileInfo.matrix(2,2) = dirZ[2];
                
                fileInfo.distance = dirZ * (osg::Vec3d(0.0,0.0,0.0)*fileInfo.matrix);

                notice()<<"dirX = "<<dirX<<std::endl;
                notice()<<"dirY = "<<dirY<<std::endl;
                notice()<<"dirZ = "<<dirZ<<std::endl;
                notice()<<"matrix = "<<fileInfo.matrix<<std::endl;
                notice()<<"pos = "<<osg::Vec3d(0.0,0.0,0.0)*fileInfo.matrix<<std::endl;
                notice()<<"dist = "<<fileInfo.distance<<std::endl;
                notice()<<std::endl;

                (orientationFileInfoMap[dirZ])[fileInfo.distance] = fileInfo;

                totalNumSlices += fileInfo.numSlices;

            }

            if (orientationFileInfoMap.empty()) return 0;
            
            typedef std::map<double, FileInfo> DistanceFileInfoMap;
            typedef std::map<osg::Vec3d, DistanceFileInfoMap> OrientationFileInfoMap;
            for(OrientationFileInfoMap::iterator itr = orientationFileInfoMap.begin();
                itr != orientationFileInfoMap.end();
                ++itr)
            {
                notice()<<"Orientation = "<<itr->first<<std::endl;
                DistanceFileInfoMap& dfim = itr->second;
                for(DistanceFileInfoMap::iterator ditr = dfim.begin();
                    ditr != dfim.end();
                    ++ditr)
                {
                    FileInfo& fileInfo = ditr->second;
                    notice()<<"   d = "<<fileInfo.distance<<" "<<fileInfo.filename<<std::endl;
                }
            }
            

            DistanceFileInfoMap& dfim = orientationFileInfoMap.begin()->second;
            if (dfim.empty()) return 0;

            double totalDistance = 0.0;
            if (dfim.size()>1)
            {
                totalDistance = fabs(dfim.rbegin()->first - dfim.begin()->first);
            }
            else
            {
                totalDistance = dfim.begin()->second.sliceThickness * double(dfim.begin()->second.numSlices);
            }
            
            notice()<<"Total Distance including ends "<<totalDistance<<std::endl;

            double averageThickness = totalNumSlices<=1 ? 1.0 : totalDistance / double(totalNumSlices-1);
            
            notice()<<"Average thickness "<<averageThickness<<std::endl;

            for(DistanceFileInfoMap::iterator ditr = dfim.begin();
                ditr != dfim.end();
                ++ditr)
            {
                FileInfo& fileInfo = ditr->second;
                
                std::auto_ptr<DicomImage> dcmImage(new DicomImage(fileInfo.filename.c_str()));
                if (dcmImage.get())
                {
                    if (dcmImage->getStatus()==EIS_Normal)
                    {

                        // get the pixel data
                        const DiPixel* pixelData = dcmImage->getInterData();
                        if(!pixelData) 
                        {
                            warning()<<"Error: no data in DicomImage object."<<std::endl;
                            return ReadResult::ERROR_IN_READING_FILE;
                        }
                        
                        osg::ref_ptr<osg::Image> imageAdapter = new osg::Image;
                        
                        EP_Representation curr_pixelRep;
                        int curr_numPlanes;
                        GLenum curr_pixelFormat;
                        GLenum curr_dataType;
                        unsigned int curr_pixelSize;

                        // create the new image
                        convertPixelTypes(pixelData,
                                          curr_pixelRep, curr_numPlanes, 
                                          curr_dataType, curr_pixelFormat, curr_pixelSize);

                        imageAdapter->setImage(dcmImage->getWidth(), dcmImage->getHeight(), dcmImage->getFrameCount(),
                                              curr_pixelFormat,
                                              curr_pixelFormat,
                                              curr_dataType,
                                              (unsigned char*)(pixelData->getData()),
                                              osg::Image::NO_DELETE);
                        if (!image)
                        {
                            pixelRep = curr_pixelRep;
                            numPlanes = curr_numPlanes;
                            dataType = curr_dataType;
                            pixelFormat = curr_pixelFormat;
                            pixelSize = curr_pixelSize;

                            (*matrix)(0,0) = fileInfo.matrix(0,0);
                            (*matrix)(1,0) = fileInfo.matrix(1,0);
                            (*matrix)(2,0) = fileInfo.matrix(2,0);
                            (*matrix)(0,1) = fileInfo.matrix(0,1);
                            (*matrix)(1,1) = fileInfo.matrix(1,1);
                            (*matrix)(2,1) = fileInfo.matrix(2,1);
                            (*matrix)(0,2) = fileInfo.matrix(0,2) * averageThickness;
                            (*matrix)(1,2) = fileInfo.matrix(1,2) * averageThickness;
                            (*matrix)(2,2) = fileInfo.matrix(2,2) * averageThickness;
                            

                            
                            image = new osg::Image;
                            image->setUserData(matrix.get());
                            image->setFileName(fileName.c_str());
                            image->allocateImage(dcmImage->getWidth(), dcmImage->getHeight(), totalNumSlices, 
                                                 pixelFormat, dataType);
                                                 
                                                 
                            matrix->preMult(osg::Matrix::scale(double(image->s()), double(image->t()), double(image->r())));

                            notice()<<"Image dimensions = "<<image->s()<<", "<<image->t()<<", "<<image->r()<<" pixelFormat=0x"<<std::hex<<pixelFormat<<" dataType=0x"<<std::hex<<dataType<<std::dec<<std::endl;
                        }
                        else if (pixelData->getPlanes()>numPlanes ||
                                 pixelData->getRepresentation()>pixelRep)
                        {
                            notice()<<"Need to reallocated "<<image->s()<<", "<<image->t()<<", "<<image->r()<<std::endl;
                            
                            // record the previous image settings to use when we copy back the content.
                            osg::ref_ptr<osg::Image> previous_image = image;

                            // create the new image
                            convertPixelTypes(pixelData,
                                              pixelRep, numPlanes, 
                                              dataType, pixelFormat, pixelSize);
                                              
                            image = new osg::Image;
                            image->setUserData(previous_image->getUserData());
                            image->setFileName(fileName.c_str());
                            image->allocateImage(dcmImage->getWidth(), dcmImage->getHeight(), totalNumSlices, 
                                                 pixelFormat, dataType);
                                                 
                            osg::copyImage(previous_image.get(), 0,0,0, previous_image->s(), previous_image->t(), imageNum,
                                           image.get(), 0, 0, 0,                                                 
                                           false);
                            
                        }
                        
                        osg::copyImage(imageAdapter.get(), 0,0,0, imageAdapter->s(), imageAdapter->t(), imageAdapter->r(), 
                                       image.get(), 0, 0, imageNum,
                                       false);
                                             
                        imageNum += dcmImage->getFrameCount();
                    }
                    else
                    {
                        warning()<<"Error in reading dicom file "<<fileName.c_str()<<", error = "<<DicomImage::getString(dcmImage->getStatus())<<std::endl;
                    }
                }
            }
            
            if (!image)
            {
                return ReadResult::ERROR_IN_READING_FILE;
            }

            notice()<<"Spacing = "<<*matrix<<std::endl;
            
            return image.get();
        }
#endif

        struct FileInfo
        {
            FileInfo():
                numX(0),
                numY(0),
                numSlices(1),
                sliceThickness(0.0),
                distance(0.0) {}

            FileInfo(const FileInfo& rhs):
                filename(rhs.filename),
                matrix(rhs.matrix),
                numX(rhs.numX),
                numY(rhs.numY),
                numSlices(rhs.numSlices),
                sliceThickness(rhs.sliceThickness),
                distance(distance) {}

            FileInfo& operator = (const FileInfo& rhs)
            {
                if (&rhs == this) return *this;
                
                filename = rhs.filename;
                matrix = rhs.matrix;
                numX = rhs.numX;
                numY = rhs.numY;
                sliceThickness = rhs.sliceThickness;
                numSlices = rhs.numSlices;
                distance = rhs.distance;
                
                return *this;
            }

            std::string     filename;
            osg::Matrixd    matrix;
            unsigned int    numX;
            unsigned int    numY;
            unsigned int    numSlices;
            double          sliceThickness;
            double          distance;
        };

};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(dicom, ReaderWriterDICOM)
