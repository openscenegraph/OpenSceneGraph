// Released under the OSGPL license, as part of the OpenSceneGraph distribution.
//
// ReaderWriter for sgi's .rgb format.
// specification can be found at http://local.wasp.uwa.edu.au/~pbourke/dataformats/sgirgb/sgiversion.html

#include <osg/Image>
#include <osg/Notify>

#include <osg/Geode>

#include <osg/GL>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImage.h>
#include <itkImageRegionConstIterator.h>
#include <itkMetaDataDictionary.h>
#include <itkMetaDataObject.h>
#include <itkGDCMImageIO.h>


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
            supportsExtension("*","dicom image format");
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

};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(dicom, ReaderWriterDICOM)
