#include <osg/Image>
#include <osg/Notify>

#include <osg/Geode>

#include <osg/GL>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>

#include <string>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

extern "C"
{
    #include <jasper/jasper.h>
}

#ifndef SEEK_SET
#  define SEEK_SET 0
#endif

using namespace osg;


extern "C" {

    static int putdata(jas_stream_t *out, jas_image_t *image, int numcmpts)
    {
        int ret;
        int cmptno;
        int x;
        int y;
        jas_matrix_t *data[4];
        jas_seqent_t *d[4];
        jas_seqent_t v;
        int linelen;
        int width, height;

        width = jas_image_cmptwidth(image, 0);
        height = jas_image_cmptheight(image, 0);

        ret = -1;

        data[0] = 0;
        data[1] = 0;
        data[2] = 0;
        data[3] = 0;
        for (cmptno = 0; cmptno < numcmpts; ++cmptno) {
            if (!(data[cmptno] = jas_matrix_create(1, width))) {
                goto done;
            }
        }

        for (y = height - 1; y >= 0; --y) {
            for (cmptno = 0; cmptno < numcmpts; ++cmptno) {
                if (jas_image_readcmpt(image, cmptno, 0, y, width, 1,
                  data[cmptno])) {
                    goto done;
                }
                d[cmptno] = jas_matrix_getref(data[cmptno], 0, 0);
            }
            linelen = 0;
            for (x = 0; x < width; ++x) {
                for (cmptno = 0; cmptno < numcmpts; ++cmptno) {
                    v = *d[cmptno];
                    if (v < 0) {
                        v = 0;
                    }
                    if (v > 255) {
                        v = 255;
                    }
                    unsigned char c;
                    c = v;
                    if (jas_stream_putc(out, c) == EOF) {
                        goto done;
                    }
                    ++d[cmptno];
                }
            }
            if (jas_stream_error(out)) {
                goto done;
            }
        }

        jas_stream_flush(out);
        ret = 0;

    done:

        for (cmptno = 0; cmptno < numcmpts; ++cmptno) {
            if (data[cmptno]) {
                jas_matrix_destroy(data[cmptno]);
            }
        }

        return ret;
    }

    static int getdata(jas_stream_t *in, jas_image_t *image)
    {
        int ret;
        int numcmpts;
        int cmptno;
        jas_matrix_t *data[4];
        int x;
        int y;
        int width, height;

        width = jas_image_cmptwidth(image, 0);
        height = jas_image_cmptheight(image, 0);
        numcmpts = jas_image_numcmpts(image);

        ret = -1;

        data[0] = 0;
        data[1] = 0;
        data[2] = 0;
        data[3] = 0;
        for (cmptno = 0; cmptno < numcmpts; ++cmptno) {
            if (!(data[cmptno] = jas_matrix_create(1, width))) {
                goto done;
            }
        }

        for (y = height - 1; y >= 0; --y)
//        for (y = 0; y < height; ++y)
        {
            for (x = 0; x < width; ++x)
            {
                for (cmptno = 0; cmptno < numcmpts; ++cmptno)
                {
                    /* The sample data is unsigned. */
                    int c;
                    if ((c = jas_stream_getc(in)) == EOF) {
                        return -1;
                    }
                    jas_matrix_set(data[cmptno], 0, x, c);
                }
            }
            for (cmptno = 0; cmptno < numcmpts; ++cmptno) {
                if (jas_image_writecmpt(image, cmptno, 0, y, width, 1,
                  data[cmptno])) {
                    goto done;
                }
            }
        }

        jas_stream_flush(in);

        ret = 0;

    done:

        for (cmptno = 0; cmptno < numcmpts; ++cmptno) {
            if (data[cmptno]) {
                jas_matrix_destroy(data[cmptno]);
            }
        }

        return ret;
    }

}

class ReaderWriterJP2 : public osgDB::ReaderWriter
{
    public:
        virtual const char* className() const { return "RGB Image Reader/Writer"; }
        
        virtual bool acceptsExtension(const std::string& extension) const
        {
            return osgDB::equalCaseInsensitive(extension,"jp2") ||
                osgDB::equalCaseInsensitive(extension,"jpc");
        }

        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            return readImage(file,options);
        }

        virtual ReadResult readObject(std::istream& fin, const Options* options) const
        {
            return readImage(fin,options);
        }


        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if(fileName.empty())
            {
                // note from Robert, Dec03, I find returning a valid image when no
                // file exists a bit odd...
                osg::Image *img = new osg::Image;
                img->setFileName(fileName);
                return img;
            }

            jas_init();

            jas_stream_t* in = jas_stream_fopen(fileName.c_str(), "rb");

            char* opt = 0;
            if(options)
            {
                opt = new char[options->getOptionString().size() + 1];
                strcpy(opt, options->getOptionString().c_str());
            }
            jas_image_t* jimage = jas_image_decode(in, -1, opt); // last is the option string whatto put there?
            if(opt) delete[] opt;

            int internalFormat = jimage->numcmpts_;

            int s = jas_image_width(jimage);
            int t = jas_image_height(jimage);
            int r = 1;

            unsigned char* data = new unsigned char[internalFormat*s*t];

            jas_stream_t* mem = jas_stream_memopen((char*)data, internalFormat*s*t);

            putdata(mem, jimage, internalFormat);

            jas_image_destroy(jimage);
            jas_stream_close(in);
            jas_image_clearfmts();

            unsigned int pixelFormat =
                internalFormat == 1 ? GL_LUMINANCE :
                internalFormat == 2 ? GL_LUMINANCE_ALPHA :
                internalFormat == 3 ? GL_RGB :
                internalFormat == 4 ? GL_RGBA : (GLenum)-1;

            unsigned int dataType = GL_UNSIGNED_BYTE;

            Image* image = new Image();
            image->setFileName(fileName.c_str());
            image->setImage(s,t,r,
                internalFormat,
                pixelFormat,
                dataType,
                data,
//                osg::Image::USE_NEW_DELETE);
                osg::Image::NO_DELETE);

            notify(INFO) << "image read ok "<<s<<"  "<<t<< std::endl;
            return image;

        }

        virtual ReadResult readImage(std::istream& fin,const Options* options) const
        {
            char c;
            char * sdata;
            long ssize;
            std::vector<char> vdata;

            while(!fin.eof())
            {
                fin.read(&c, 1);
                vdata.push_back(c);
            }
            ssize = vdata.size();

            sdata = &vdata[0];

            jas_init();

            jas_stream_t* in = jas_stream_memopen((char*)sdata, ssize);

            char* opt = 0;
            if(options && !options->getOptionString().empty())
            {
                opt = new char[options->getOptionString().size() + 1];
                strcpy(opt, options->getOptionString().c_str());
            }
            jas_image_t* jimage = jas_image_decode(in, -1, opt); // last is the option string whatto put there?
            if(opt) delete[] opt;
            
            if (!jimage) return ReadResult::FILE_NOT_HANDLED; 

            int internalFormat = jimage->numcmpts_;

            int s = jas_image_width(jimage);
            int t = jas_image_height(jimage);
            int r = 1;

            unsigned char* data = new unsigned char[internalFormat*s*t];

            jas_stream_t* mem = jas_stream_memopen((char*)data, internalFormat*s*t);

            putdata(mem, jimage, internalFormat);

            jas_image_destroy(jimage);
            jas_stream_close(in);
            jas_image_clearfmts();

            unsigned int pixelFormat =
                internalFormat == 1 ? GL_LUMINANCE :
                internalFormat == 2 ? GL_LUMINANCE_ALPHA :
                internalFormat == 3 ? GL_RGB :
                internalFormat == 4 ? GL_RGBA : (GLenum)-1;

            unsigned int dataType = GL_UNSIGNED_BYTE;

            Image* image = new Image();
//            image->setFileName(fileName.c_str());
            image->setImage(s,t,r,
                internalFormat,
                pixelFormat,
                dataType,
                data,
//                osg::Image::USE_NEW_DELETE);
                osg::Image::NO_DELETE);

            notify(INFO) << "image read ok "<<s<<"  "<<t<< std::endl;
            return image;
        }

        virtual WriteResult writeObject(const osg::Object& object,const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            const osg::Image* image = dynamic_cast<const osg::Image*>(&object);
            if (!image) return WriteResult::FILE_NOT_HANDLED;

            return writeImage(*image,file,options);
        }

        virtual WriteResult writeObject(const osg::Object& object,std::ostream& fout,const Options* options) const
        {
            const osg::Image* image = dynamic_cast<const osg::Image*>(&object);
            if (!image) return WriteResult::FILE_NOT_HANDLED;

            return writeImage(*image,fout,options);
        }

        virtual WriteResult writeImage(const osg::Image &img,const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getFileExtension(fileName);
            if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;
            
            jas_init();

            jas_image_cmptparm_t cmptparms[4];
            jas_image_cmptparm_t *cmptparm;

            int internalFormat = osg::Image::computeNumComponents(img.getPixelFormat());

            jas_stream_t* mem = jas_stream_memopen((char*)img.data(), internalFormat*img.s()*img.t());

            /* Create an image of the correct size. */
            jas_image_t* jimage;
            int i;
            for (i = 0, cmptparm = cmptparms; i < internalFormat; ++i, ++cmptparm) {
                cmptparm->tlx = 0;
                cmptparm->tly = 0;
                cmptparm->hstep = 1;
                cmptparm->vstep = 1;
                cmptparm->width = img.s();
                cmptparm->height = img.t();
                cmptparm->prec = 8;
                cmptparm->sgnd = 0;
            }
            if (!(jimage = jas_image_create(internalFormat, cmptparms, JAS_CLRSPC_UNKNOWN))) {
                return WriteResult::ERROR_IN_WRITING_FILE;
            }

            if(internalFormat == 1)
            {
                jas_image_setclrspc(jimage, JAS_CLRSPC_GENGRAY);
                jas_image_setcmpttype(jimage, 0, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_GRAY_Y));
            }
            else if(internalFormat == 2)
            {
                jas_image_setclrspc(jimage, JAS_CLRSPC_GENGRAY);
                jas_image_setcmpttype(jimage, 0, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_GRAY_Y));
                jas_image_setcmpttype(jimage, 1, JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_OPACITY));
            }
            else if(internalFormat == 3)
            {
                jas_image_setclrspc(jimage, JAS_CLRSPC_SRGB);
                jas_image_setcmpttype(jimage, 0, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R));
                jas_image_setcmpttype(jimage, 1, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G));
                jas_image_setcmpttype(jimage, 2, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B));
            }
            else if(internalFormat == 4)
            {
                jas_image_setclrspc(jimage, JAS_CLRSPC_SRGB);
                jas_image_setcmpttype(jimage, 0, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R));
                jas_image_setcmpttype(jimage, 1, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G));
                jas_image_setcmpttype(jimage, 2, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B));
                jas_image_setcmpttype(jimage, 3, JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_OPACITY));
            }

            getdata(mem, jimage);

            jas_stream_t* out = jas_stream_fopen(fileName.c_str(), "wb");
            if (!out) 
                return WriteResult::ERROR_IN_WRITING_FILE;

            char* opt = 0;
            if(options)
            {
                opt = new char[options->getOptionString().size() + 1];
                strcpy(opt, options->getOptionString().c_str());
            }
            jas_image_encode(jimage, out, jas_image_strtofmt("jp2"),  opt);
            if(opt) delete[] opt;

            jas_stream_flush(out);

            jas_stream_close(out);
            jas_image_destroy(jimage);
            jas_image_clearfmts();

            return WriteResult::FILE_SAVED;
        }

        WriteResult writeImage(const osg::Image& img, std::ostream& fout, const Options* options) const
        {
            jas_init();

            jas_image_cmptparm_t cmptparms[4];
            jas_image_cmptparm_t *cmptparm;

            int internalFormat = osg::Image::computeNumComponents(img.getPixelFormat());

            jas_stream_t* mem = jas_stream_memopen((char*)img.data(), internalFormat*img.s()*img.t());

            /* Create an image of the correct size. */
            jas_image_t* jimage;
            int i;
            for (i = 0, cmptparm = cmptparms; i < internalFormat; ++i, ++cmptparm) {
                cmptparm->tlx = 0;
                cmptparm->tly = 0;
                cmptparm->hstep = 1;
                cmptparm->vstep = 1;
                cmptparm->width = img.s();
                cmptparm->height = img.t();
                cmptparm->prec = 8;
                cmptparm->sgnd = 0;
            }
            if (!(jimage = jas_image_create(internalFormat, cmptparms, JAS_CLRSPC_UNKNOWN))) {
                return WriteResult::ERROR_IN_WRITING_FILE;
            }

            if(internalFormat == 1)
            {
                jas_image_setclrspc(jimage, JAS_CLRSPC_SGRAY);
                jas_image_setcmpttype(jimage, 0, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_GRAY_Y));
            }
            else if(internalFormat == 2)
            {
                jas_image_setclrspc(jimage, JAS_CLRSPC_SGRAY);
                jas_image_setcmpttype(jimage, 0, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_GRAY_Y));
                jas_image_setcmpttype(jimage, 1, JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_OPACITY));
            }
            else if(internalFormat == 3)
            {
                jas_image_setclrspc(jimage, JAS_CLRSPC_SRGB);
                jas_image_setcmpttype(jimage, 0, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R));
                jas_image_setcmpttype(jimage, 1, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G));
                jas_image_setcmpttype(jimage, 2, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B));
            }
            else if(internalFormat == 4)
            {
                jas_image_setclrspc(jimage, JAS_CLRSPC_SRGB);
                jas_image_setcmpttype(jimage, 0, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R));
                jas_image_setcmpttype(jimage, 1, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G));
                jas_image_setcmpttype(jimage, 2, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B));
                jas_image_setcmpttype(jimage, 3, JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_OPACITY));
            }

            getdata(mem, jimage);

            jas_stream_t* out = jas_stream_memopen(0, 0);
//            jas_stream_t* out = jas_stream_fopen(fileName.c_str(), "wb");
            if (!out) 
                return WriteResult::ERROR_IN_WRITING_FILE;

            char* opt = 0;
            if(options)
            {
                opt = new char[options->getOptionString().size() + 1];
                strcpy(opt, options->getOptionString().c_str());
            }
            jas_image_encode(jimage, out, jas_image_strtofmt("jp2"),  opt);
            if(opt) delete[] opt;

            jas_stream_flush(out);

            // now the encoded jp2 image resides in the out->buf_ member with size out->len_ we now need to stream it to a std::ostream
            jas_stream_memobj_t* obj = (jas_stream_memobj_t*) out->obj_;

            fout.write((char*)obj->buf_, obj->len_);

            fout << std::flush;

            jas_stream_close(out);

            jas_image_destroy(jimage);
            jas_image_clearfmts();

            return WriteResult::FILE_SAVED;
        }

};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterJP2> g_readerWriter_JP2_Proxy;
