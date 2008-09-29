// Released under the OSGPL license, as part of the OpenSceneGraph distribution.
//
// ReaderWriter for sgi's .rgb format.
// specification can be found at http://local.wasp.uwa.edu.au/~pbourke/dataformats/sgirgb/sgiversion.html

#include <osg/Image>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/GL>
#include <osg/io_utils>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>

#include <osgVolume/Volume>
#include <osgVolume/Brick>

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

class ReaderWriterDICOM : public osgDB::ReaderWriter
{
    public:
    
        ReaderWriterDICOM()
        {
            supportsExtension("dic","dicom image format");
            supportsExtension("dcm","dicom image format");
            supportsExtension("dicom","dicom image format");
            // supportsExtension("*","dicom image format");
        }
        
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
            
            osg::ref_ptr<osgVolume::Volume> volume = new osgVolume::Volume;

            osg::ref_ptr<osgVolume::Brick> brick = new osgVolume::Brick;
            brick->setVolume(volume.get());
            brick->setImage(result.getImage());

            // get matrix providing size of texels (in mm)
            osg::RefMatrix* matrix = dynamic_cast<osg::RefMatrix*>(result.getImage()->getUserData());
        
            if (matrix)
            {
            
            
                // scale up to provide scale of complete brick
                osg::Vec3d scale(osg::Vec3(result.getImage()->s(),result.getImage()->t(), result.getImage()->r()));
                matrix->postMultScale(scale);

                brick->setLocator(matrix);
                
                result.getImage()->setUserData(0);
                
                osg::notify(osg::NOTICE)<<"Locator "<<*matrix<<std::endl;
            }
            
            volume->addChild(brick.get());
                        
            return volume.release();
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

            osg::notify(osg::NOTICE)<<"Reading DICOM file "<<fileName<<std::endl;

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
            
            osg::notify(osg::NOTICE)<<"width = "<<width<<" height = "<<height<<" depth = "<<depth<<std::endl;
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

            return image;
        }
#endif

#ifdef USE_DCMTK

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
                osg::notify(osg::INFO)<<"contents = "<<localFile<<std::endl;
                if (osgDB::getLowerCaseFileExtension(localFile)=="dcm" &&
                    osgDB::fileType(localFile) == osgDB::REGULAR_FILE)
                {
                    files.push_back(localFile);
                }
            }
            
            return !files.empty();
        }


        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            osg::notify(osg::NOTICE)<<"Reading DICOM file "<<file<<" using DCMTK"<<std::endl;

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
            EP_Representation pixelRep;
            unsigned int numPlanes = 0;
            GLenum pixelFormat = 0;
            GLenum dataType = 0;
            unsigned int pixelSize = 0;
            bool invertOrigiantion = false;
            
            typedef std::list<FileInfo> FileInfoList;
            FileInfoList fileInfoList;

            typedef std::map<double, FileInfo> DistanceFileInfoMap;
            typedef std::map<osg::Vec3d, DistanceFileInfoMap> OrientationFileInfoMap;
            OrientationFileInfoMap orientationFileInfoMap;

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
                if (fileformat.getDataset()->findAndGetFloat64(DCM_PixelSpacing, pixelSize_y,0).good())
                {
                    fileInfo.matrix(1,1) = pixelSize_y;
                }

                double pixelSize_x = 1.0;
                if (fileformat.getDataset()->findAndGetFloat64(DCM_PixelSpacing, pixelSize_x,1).good())
                {
                    fileInfo.matrix(0,0) = pixelSize_x;
                }

                // Get slice thickness
                double sliceThickness = 1.0;
                if (fileformat.getDataset()->findAndGetFloat64(DCM_SliceThickness, sliceThickness).good())
                {
                    fileInfo.matrix(2,2) = sliceThickness;
                }

                double imagePositionPatient[3] = {0, 0, 0};

                // patient position
                for(int i=0; i<3; ++i)
                {
		    if (fileformat.getDataset()->findAndGetFloat64(DCM_ImagePositionPatient, imagePositionPatient[i],i).good())
                    {
                        osg::notify(osg::INFO)<<"Read DCM_ImagePositionPatient["<<i<<"], "<<imagePositionPatient[i]<<std::endl;
                    }
                    else
                    {
                        osg::notify(osg::INFO)<<"Have not read DCM_ImagePositionPatient["<<i<<"]"<<std::endl;
                    }
                }
                
                fileInfo.matrix.setTrans(imagePositionPatient[0],imagePositionPatient[1],imagePositionPatient[2]);

	        double imageOrientationPatient[6] = { 1.0, 0.0, 0.0,
		                                      0.0, -1.0, 0.0 };
                for(int i=0; i<6; ++i)
                {
		    if (fileformat.getDataset()->findAndGetFloat64(DCM_ImageOrientationPatient, imageOrientationPatient[i],i).good())
                    {
                        osg::notify(osg::INFO)<<"Read imageOrientationPatient["<<i<<"], "<<imageOrientationPatient[i]<<std::endl;
                    }
                    else
                    {
                        osg::notify(osg::INFO)<<"Have not read imageOrientationPatient["<<i<<"]"<<std::endl;
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

                osg::notify(osg::INFO)<<"dirX = "<<dirX<<std::endl;
                osg::notify(osg::INFO)<<"dirY = "<<dirY<<std::endl;
                osg::notify(osg::INFO)<<"dirZ = "<<dirZ<<std::endl;
                osg::notify(osg::INFO)<<"matrix = "<<fileInfo.matrix<<std::endl;
                osg::notify(osg::INFO)<<"pos = "<<osg::Vec3d(0.0,0.0,0.0)*fileInfo.matrix<<std::endl;
                osg::notify(osg::INFO)<<"dist = "<<fileInfo.distance<<std::endl;
                osg::notify(osg::INFO)<<std::endl;

                (orientationFileInfoMap[dirZ])[fileInfo.distance] = fileInfo;

            }

            if (orientationFileInfoMap.empty()) return 0;
            
            typedef std::map<double, FileInfo> DistanceFileInfoMap;
            typedef std::map<osg::Vec3d, DistanceFileInfoMap> OrientationFileInfoMap;
            for(OrientationFileInfoMap::iterator itr = orientationFileInfoMap.begin();
                itr != orientationFileInfoMap.end();
                ++itr)
            {
                osg::notify(osg::INFO)<<"Orientation = "<<itr->first<<std::endl;
                DistanceFileInfoMap& dfim = itr->second;
                for(DistanceFileInfoMap::iterator ditr = dfim.begin();
                    ditr != dfim.end();
                    ++ditr)
                {
                    FileInfo& fileInfo = ditr->second;
                    osg::notify(osg::INFO)<<"   d = "<<fileInfo.distance<<" "<<fileInfo.filename<<std::endl;
                }
            }
            

            DistanceFileInfoMap& dfim = orientationFileInfoMap.begin()->second;
            if (dfim.empty()) return 0;

            for(DistanceFileInfoMap::iterator ditr = dfim.begin();
                ditr != dfim.end();
                ++ditr)
            {
                FileInfo& fileInfo = ditr->second;
                fileInfoList.push_back(fileInfo);
                osg::notify(osg::INFO)<<"   d = "<<fileInfo.distance<<" "<<fileInfo.filename<<std::endl;
            }
            
            
            double totalDistance = dfim.rbegin()->first - dfim.begin()->first;
            double averageThickness = dfim.size()<=1 ? 1.0 : totalDistance / double(dfim.size()-1);
            
            osg::notify(osg::INFO)<<"Average thickness "<<averageThickness<<std::endl;


            for(FileInfoList::iterator itr =  fileInfoList.begin();
                itr !=  fileInfoList.end();
                ++itr)
            {
                FileInfo& fileInfo = *itr;
                std::auto_ptr<DicomImage> dcmImage(new DicomImage(fileInfo.filename.c_str()));
                if (dcmImage.get())
                {
                    if (dcmImage->getStatus()==EIS_Normal)
                    {
                        // get the pixel data
                        const DiPixel* pixelData = dcmImage->getInterData();
                        if(!pixelData) return ReadResult::ERROR_IN_READING_FILE;

                        if (!image)
                        {
                            osg::notify(osg::NOTICE)<<"dcmImage->getWidth() = "<<dcmImage->getWidth()<<std::endl;
                            osg::notify(osg::NOTICE)<<"dcmImage->getHeight() = "<<dcmImage->getHeight()<<std::endl;
                            osg::notify(osg::NOTICE)<<"dcmImage->getFrameCount() = "<<dcmImage->getFrameCount()<<std::endl;
                            osg::notify(osg::NOTICE)<<"pixelData->getCount() = "<<pixelData->getCount()<<std::endl;
                            osg::notify(osg::NOTICE)<<"pixelFormat = ";

                            dataType = GL_UNSIGNED_BYTE;
                            pixelRep = pixelData->getRepresentation();
                            switch(pixelRep)
                            {
                                case(EPR_Uint8):
                                    dataType = GL_UNSIGNED_BYTE;
                                    pixelSize = 1;
                                    osg::notify(osg::NOTICE)<<"unsigned char"<<std::endl;
                                    break;
                                case(EPR_Sint8):
                                    dataType = GL_BYTE;
                                    pixelSize = 1;
                                    osg::notify(osg::NOTICE)<<"char"<<std::endl;
                                    break;
                                case(EPR_Uint16):
                                    dataType = GL_UNSIGNED_SHORT;
                                    pixelSize = 2;
                                    osg::notify(osg::NOTICE)<<"unsigned short"<<std::endl;
                                    break;
                                case(EPR_Sint16):
                                    dataType = GL_SHORT;
                                    pixelSize = 2;
                                    osg::notify(osg::NOTICE)<<"short"<<std::endl;
                                    break;
                                case(EPR_Uint32):
                                    dataType = GL_UNSIGNED_INT;
                                    pixelSize = 4;
                                    osg::notify(osg::NOTICE)<<"unsigned int"<<std::endl;
                                    break;
                                case(EPR_Sint32):
                                    osg::notify(osg::NOTICE)<<"int"<<std::endl;
                                    dataType = GL_INT;
                                    pixelSize = 4;
                                    break;
                                default:
                                    osg::notify(osg::NOTICE)<<"unidentified"<<std::endl;
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
                            image->allocateImage(dcmImage->getWidth(), dcmImage->getHeight(), files.size() * dcmImage->getFrameCount(),
                                                 pixelFormat, dataType);
                                                 
                                                 
                            osg::notify(osg::NOTICE)<<"Image dimensions = "<<image->s()<<", "<<image->t()<<", "<<image->r()<<std::endl;
                        }
                        
                        if (pixelData->getPlanes()==numPlanes &&
                            pixelData->getRepresentation()==pixelRep &&
                            dcmImage->getWidth()==image->s() &&
                            dcmImage->getHeight()==image->t())
                        {
                            int numFramesToCopy = std::min(static_cast<unsigned int>(image->r()-imageNum),
                                                           static_cast<unsigned int>(dcmImage->getFrameCount())); 
                            unsigned int numPixels = dcmImage->getWidth() * dcmImage->getHeight() * numFramesToCopy;
                            unsigned int dataSize = numPixels * pixelSize;
                            
                            if (invertOrigiantion)
                            {
                                memcpy(image->data(0,0,image->r()-imageNum-1), pixelData->getData(), dataSize);
                            }
                            else
                            {
                                memcpy(image->data(0,0,imageNum), pixelData->getData(), dataSize);
                            }
                            
                            imageNum += numFramesToCopy;
                        }
                    }
                    else
                    {
                        osg::notify(osg::NOTICE)<<"Error in reading dicom file "<<fileName.c_str()<<", error = "<<DicomImage::getString(dcmImage->getStatus())<<std::endl;
                    }
                }
            }
            
            if (!image)
            {
                return ReadResult::ERROR_IN_READING_FILE;
            }

            osg::notify(osg::NOTICE)<<"Spacing = "<<*matrix<<std::endl;
            
            return image.get();
        }
#endif

        struct FileInfo
        {
            FileInfo():
                numX(0),
                numY(0),
                numSlices(0),
                distance(0.0) {}

            FileInfo(const FileInfo& rhs):
                filename(rhs.filename),
                matrix(rhs.matrix),
                numX(rhs.numX),
                numY(rhs.numY),
                numSlices(rhs.numSlices),
                distance(distance) {}

            FileInfo& operator = (const FileInfo& rhs)
            {
                if (&rhs == this) return *this;
                
                filename = rhs.filename;
                matrix = rhs.matrix;
                numX = rhs.numX;
                numY = rhs.numY;
                numSlices = rhs.numSlices;
                distance = rhs.distance;
            }

            std::string filename;
            osg::Matrixd matrix;
            unsigned int numX;
            unsigned int numY;
            unsigned int numSlices;            
            double  distance;
        };

};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(dicom, ReaderWriterDICOM)
