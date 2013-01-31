#include <osg/Image>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/Image>
#include <osg/GL>

#if defined _WIN32 && !defined OSG_LIBRARY_STATIC
//Make the half format work against openEXR libs
#define OPENEXR_DLL
#endif

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <ImfRgbaFile.h>
#include <ImfIO.h>
#include <ImfArray.h>

using namespace std;
using namespace Imf;
using namespace Imath;

/****************************************************************************
 *
 * Follows is code written by FOI (www.foi.se)
 * it is a wraper of openEXR(www.openexr.com)
 * to add suport of exr images into osg
 *
 * Ported to a OSG-plugin, Ragnar Hammarqvist.
 * For patches, bugs and new features
 * please send them direct to the OSG dev team.
 **********************************************************************/
class C_IStream: public Imf::IStream
{
public:
    C_IStream (istream *fin) :
      IStream(""),_inStream(fin){}

      virtual bool    read (char c[/*n*/], int n)
      {
        return _inStream->read(c,n).good();
      };
      virtual Int64    tellg ()
      {
          return _inStream->tellg();
      };
      virtual void    seekg (Int64 pos)
      {
        _inStream->seekg(pos);
      };
      virtual void    clear ()
      {
        _inStream->clear();
      };

private:
    std::istream * _inStream;
};

class C_OStream: public Imf::OStream
{
public:
    C_OStream (ostream *fin) :
      OStream(""),_outStream(fin)
      {};

      virtual void    write (const char c[/*n*/], int n)
      {
        _outStream->write(c,n);
      };
      virtual Int64    tellp ()
      {
        return _outStream->tellp();
      };
      virtual void seekp (Int64 pos)
      {
        _outStream->seekp(pos);
      };

private:
    std::ostream * _outStream;
};


unsigned char *exr_load(std::istream& fin,
                        int *width_ret,
                        int *height_ret,
                        int *numComponents_ret,
                        unsigned int *dataType_ret)
{
    unsigned char *buffer=NULL; // returned to sender & as read from the disk
    bool inputError = false;
    Array2D<Rgba> pixels;
    int width,height,numComponents;

    try
    {
        C_IStream inStream(&fin);
        RgbaInputFile rgbafile(inStream);

        Box2i dw = rgbafile.dataWindow();
        /*RgbaChannels channels =*/ rgbafile.channels();
        (*width_ret) = width = dw.max.x - dw.min.x + 1;
        (*height_ret)=height = dw.max.y - dw.min.y + 1;
        (*dataType_ret) = GL_HALF_FLOAT;

        pixels.resizeErase (height, width);

        rgbafile.setFrameBuffer((&pixels)[0][0] - dw.min.x - dw.min.y * width, 1, width);
        rgbafile.readPixels(dw.min.y, dw.max.y);
    }
    catch( char * str ) {
        inputError = true;
    }

    //If error during stream read return a empty pointer
    if (inputError)
    {
        return buffer;
    }

    //If there is no information in alpha channel do not store the alpha channel
    numComponents = 3;
    for (long i = height-1; i >= 0; i--)
    {
        for (long j = 0 ; j < width; j++)
        {
            if (pixels[i][j].a != half(1.0f) )
            {
                numComponents = 4;
                break;
            }
        }
    }
    (*numComponents_ret) = numComponents;

    if (!(    numComponents == 3 ||
            numComponents == 4))
    {
        return NULL;
    }

    //Copy and allocate data to a unsigned char array that OSG can use for texturing
    unsigned dataSize = (sizeof(half) * height * width * numComponents);
    //buffer = new unsigned char[dataSize];
    buffer = (unsigned char*)malloc(dataSize);
    half* pOut = (half*) buffer;

    for (long i = height-1; i >= 0; i--)
    {
        for (long j = 0 ; j < width; j++)
        {
            (*pOut) = pixels[i][j].r;
            pOut++;
            (*pOut) = pixels[i][j].g;
            pOut++;
            (*pOut) = pixels[i][j].b;
            pOut++;
            if (numComponents >= 4)
            {
                (*pOut) = pixels[i][j].a;
                pOut++;
            }
        }
    }

    return buffer;
}


 class ReaderWriterEXR : public osgDB::ReaderWriter
{
public:
    ReaderWriterEXR()
    {
    }

    virtual bool acceptsExtension(const std::string& extension) const { return osgDB::equalCaseInsensitive(extension,"exr"); }

    virtual const char* className() const { return "EXR Image Reader"; }

    virtual ReadResult readObject(std::istream& fin,const osgDB::ReaderWriter::Options* options =NULL) const
    {
        return readImage(fin, options);
    }

    virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options =NULL) const
    {
        return readImage(file, options);
    }

    virtual ReadResult readImage(std::istream& fin,const Options* =NULL) const
    {
        return readEXRStream(fin);
    }

    virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

        std::string fileName = osgDB::findDataFile( file, options );
        if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

        osgDB::ifstream istream(fileName.c_str(), std::ios::in | std::ios::binary);
        if(!istream) return ReadResult::FILE_NOT_HANDLED;

        ReadResult rr = readEXRStream(istream);
        if(rr.validImage())
        {
            rr.getImage()->setFileName(fileName);
        }
        return rr;
    }

    virtual WriteResult writeImage(const osg::Image& image,std::ostream& fout,const Options*) const
    {
        bool success = writeEXRStream(image, fout, "<output stream>");

        if(success)
            return WriteResult::FILE_SAVED;
        else
            return WriteResult::ERROR_IN_WRITING_FILE;
    }

    virtual WriteResult writeImage(const osg::Image &img,const std::string& fileName, const osgDB::ReaderWriter::Options*) const
     {
        std::string ext = osgDB::getFileExtension(fileName);
        if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

        osgDB::ofstream fout(fileName.c_str(), std::ios::out | std::ios::binary);
        if(!fout) return WriteResult::ERROR_IN_WRITING_FILE;

        bool success = writeEXRStream(img, fout, fileName);

        fout.close();

        if(success)
            return WriteResult::FILE_SAVED;
        else
            return WriteResult::ERROR_IN_WRITING_FILE;
     }
protected:
    bool writeEXRStream(const osg::Image &img, std::ostream& fout, const std::string &fileName) const
    {
        bool writeOK = true;

        //Obtain data from texture
        int width = img.s();
        int height = img.t();
        unsigned int pixelFormat = img.getPixelFormat();
        int numComponents = img.computeNumComponents(pixelFormat);
        unsigned int dataType = img.getDataType();

        //Validates image data
        //if numbers of components matches
        if (!(    numComponents == 3 ||
                numComponents == 4))
        {
            writeOK = false;
            return false;
        }
        if (!(    dataType == GL_HALF_FLOAT ||
                dataType == GL_FLOAT))
        {
            writeOK = false;
            return false;
        }

         //Create a stream to save to
         C_OStream outStream(&fout);

         //Copy data from texture to rgba pixel format
        Array2D<Rgba> outPixels(height,width);
         //If texture is half format
         if (dataType == GL_HALF_FLOAT)
         {
             for (long i = height-1; i >= 0; i--)
             {
                half* pOut = (half*) img.data(0,i);
                 for (long j = 0 ; j < width; j++)
                 {
                     outPixels[i][j].r = (*pOut);
                     pOut++;
                     outPixels[i][j].g = (*pOut);
                     pOut++;
                     outPixels[i][j].b = (*pOut);
                     pOut++;
                    if (numComponents >= 4)
                    {
                        outPixels[i][j].a = (*pOut);
                        pOut++;
                    }
                    else{outPixels[i][j].a = 1.0f;}
                 }
             }
         }
        else if (dataType == GL_FLOAT)
        {
            float* pOut = (float*) img.data();
            for (long i = height-1; i >= 0; i--)
            {
                for (long j = 0 ; j < width; j++)
                {
                    outPixels[i][j].r = half(*pOut);
                    pOut++;
                    outPixels[i][j].g = half(*pOut);
                    pOut++;
                    outPixels[i][j].b = half(*pOut);
                    pOut++;
                    if (numComponents >= 4)
                    {
                        outPixels[i][j].a = half(*pOut);
                        pOut++;
                    }
                    else
                    {outPixels[i][j].a = 1.0f;}
                }
            }
        }
         else
         {
             //If texture format not supported
             return false;
         }

         try
         {
             //Write to stream
             Header outHeader(width, height);
             RgbaOutputFile rgbaFile (outStream, outHeader, WRITE_RGBA);
             rgbaFile.setFrameBuffer ((&outPixels)[0][0], 1, width);
             rgbaFile.writePixels (height);
         }
         catch( char * str )
         {
             writeOK = false;
         }


        return writeOK;
    }

    ReadResult readEXRStream(std::istream& fin) const
    {
        unsigned char *imageData = NULL;
        int width_ret = 0;
        int height_ret = 0;
        int numComponents_ret = 4;
        unsigned int dataType_ret = GL_UNSIGNED_BYTE;
        unsigned int pixelFormat = GL_RGB;
        unsigned int interNalTextureFormat = GL_RGB;

        imageData = exr_load(fin,&width_ret,&height_ret,&numComponents_ret,&dataType_ret);

        if (imageData==NULL)
            return ReadResult::FILE_NOT_HANDLED;

        int s = width_ret;
        int t = height_ret;
        int r = 1;

        if (dataType_ret == GL_HALF_FLOAT)
        {
            interNalTextureFormat =
                numComponents_ret == 1 ? GL_LUMINANCE16F_ARB :
                numComponents_ret == 2 ? GL_LUMINANCE_ALPHA16F_ARB :
                numComponents_ret == 3 ? GL_RGB16F_ARB :
                numComponents_ret == 4 ? GL_RGBA16F_ARB : (GLenum)-1;
        }
        else if (dataType_ret == GL_FLOAT)
        {
            interNalTextureFormat =
                numComponents_ret == 1 ? GL_LUMINANCE32F_ARB :
                numComponents_ret == 2 ? GL_LUMINANCE_ALPHA32F_ARB :
                numComponents_ret == 3 ? GL_RGB32F_ARB :
                numComponents_ret == 4 ? GL_RGBA32F_ARB : (GLenum)-1;
        }
        pixelFormat =
            numComponents_ret == 1 ? GL_LUMINANCE :
            numComponents_ret == 2 ? GL_LUMINANCE_ALPHA :
            numComponents_ret == 3 ? GL_RGB :
            numComponents_ret == 4 ? GL_RGBA : (GLenum)-1;

        unsigned int dataType = dataType_ret;

        osg::Image* pOsgImage = new osg::Image;
        pOsgImage->setImage(s,t,r,
            interNalTextureFormat,
            pixelFormat,
            dataType,
            imageData,
            osg::Image::USE_MALLOC_FREE);

        return pOsgImage;
    }
};

// now register with Registry to instantiate the exr suport
// reader/writer.
REGISTER_OSGPLUGIN(exr, ReaderWriterEXR)
