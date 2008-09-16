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
                std::cout<<"contents = "<<localFile<<std::endl;
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
                        
            for(Files::iterator itr = files.begin();
                itr != files.end();
                ++itr)
            {
                std::auto_ptr<DicomImage> dcmImage(new DicomImage((*itr).c_str()));
                if (dcmImage.get())
                {
                    if (dcmImage->getStatus()==EIS_Normal)
                    {
                        // get the pixel data
                        const DiPixel* pixelData = dcmImage->getInterData();
                        if(!pixelData) return ReadResult::ERROR_IN_READING_FILE;

                        if (!image)
                        {
                            // read dicom file format to extra spacing info
                            DcmFileFormat fileformat;
                            OFCondition status = fileformat.loadFile((*itr).c_str());
                            if(!status.good()) return ReadResult::ERROR_IN_READING_FILE;

                            // get the dimension of the dicom image
                            OFString spacingString;
                            if(fileformat.getDataset()->findAndGetOFString(DCM_PixelSpacing, spacingString).good())
                            {
                                float xy_spacing = atof(spacingString.c_str());
                                (*matrix)(0,0) = xy_spacing;
                                (*matrix)(1,1) = xy_spacing * dcmImage->getWidthHeightRatio();
                            }

                            // Get slice thickness
                            if(fileformat.getDataset()->findAndGetOFString(DCM_SliceThickness, spacingString).good())
                            {
                                (*matrix)(2,2) = atof(spacingString.c_str());
                            }


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

                            image = new osg::Image;
                            image->setFileName(fileName.c_str());
                            image->allocateImage(dcmImage->getWidth(), dcmImage->getHeight(), files.size() * dcmImage->getFrameCount(),
                                                 pixelFormat, dataType);
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
                            memcpy(image->data(0,0,imageNum), pixelData->getData(), dataSize);
                            
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

};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(dicom, ReaderWriterDICOM)
