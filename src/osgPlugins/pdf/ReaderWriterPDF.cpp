/* -*-c++-*- OpenSceneGraph - Copyright (C) 1999-2008 Robert Osfield
 *
 * This software is open source and may be redistributed and/or modified under
 * the terms of the GNU General Public License (GPL) version 2.0.
 * The full license is in LICENSE.txt file included with this distribution,.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * include LICENSE.txt for more details.
*/

#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>
#include <osgDB/FileUtils>

#include <osgWidget/PdfReader>
#include <osg/ImageUtils>

#include <cairo.h>
#include <poppler.h>

class CairoImage : public osg::Referenced
{
    public:

        CairoImage(osg::Image* image):
            _image(image),
            _surface(0),
            _context(0) {}


        void create(int width, int height)
        {
            if (_image->data() && width==_image->s() && height==_image->t())
            {
                return;
            }

            OSG_NOTICE<<"Create cario surface/context "<<width<<", "<<height<<std::endl;

            // allocate the image data
            _image->allocateImage(width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE);
            _image->setPixelFormat(GL_BGRA);
            _image->setDataVariance(osg::Object::DYNAMIC);
            _image->setOrigin(osg::Image::TOP_LEFT);


            // create a cairo surface for this image data
            _surface = cairo_image_surface_create_for_data(
                    _image->data(),
                    CAIRO_FORMAT_ARGB32,
                    width, height,
                    _image->getRowSizeInBytes());

            // create a context for the surface
            _context = cairo_create(_surface);
        }

        void destroy()
        {
            if (_surface) cairo_surface_destroy(_surface);
            if (_context) cairo_destroy(_context);
        }

        cairo_surface_t* getSurface() { return _surface; }
        const cairo_surface_t* getSurface() const { return _surface; }

        cairo_t* getContext() { return _context; }
        const cairo_t* getContext() const { return _context; }

    protected:

        virtual ~CairoImage()
        {
            destroy();
        }

        osg::observer_ptr<osg::Image> _image;
        cairo_surface_t*              _surface;
        cairo_t*                      _context;
};

class PopplerPdfImage : public osgWidget::PdfImage
{
    public:

        PopplerPdfImage():
            _doc(0)
        {
            _cairoImage = new CairoImage(this);
        }

        virtual ~PopplerPdfImage()
        {
            _cairoImage = 0;

            if (_doc)
            {
                g_object_unref(_doc);
            }
        }

        PopplerDocument* _doc;

        int getNumOfPages() { return _doc ? poppler_document_get_n_pages(_doc) : 0; }

        bool open(const std::string& filename)
        {
            OSG_NOTICE<<"open("<<filename<<")"<<std::endl;

            std::string foundFile = osgDB::findDataFile(filename);
            if (foundFile.empty())
            {
                OSG_NOTICE<<"could not find filename="<<filename<<std::endl;
                return false;
            }

            OSG_NOTICE<<"foundFile = "<<foundFile<<std::endl;
            foundFile = osgDB::getRealPath(foundFile);
            OSG_NOTICE<<"foundFile = "<<foundFile<<std::endl;

#if defined(WIN32) && !defined(__CYGWIN__)
            std::string uri = std::string("file:///") + foundFile;
#else
            std::string uri = std::string("file:") + foundFile;
#endif

            PopplerDocument* doc = poppler_document_new_from_file(uri.c_str(), NULL, NULL);
            if (!doc)
            {
                OSG_NOTICE<<" could not open("<<filename<<"), uri="<<uri<<std::endl;

                return false;
            }

            if (_doc)
            {
                g_object_unref(_doc);
            }

            _doc = doc;
            _pageNum = 0;

            setFileName(filename);

            OSG_NOTICE<<"getNumOfPages()=="<<getNumOfPages()<<std::endl;

            if (getNumOfPages()==0)
            {
                return false;
            }

            page(0);

            return true;
        }

        virtual bool sendKeyEvent(int key, bool keyDown)
        {
            if (keyDown && key!=0)
            {
                if (key==_nextPageKeyEvent)
                {
                    next();
                    return true;
                }
                else if (key==_previousPageKeyEvent)
                {
                    previous();
                    return true;
                }
            }
            return false;
        }


        virtual bool page(int pageNum)
        {
            if (!_doc) return false;

            if (pageNum<0 || pageNum>=getNumOfPages()) return false;

            PopplerPage* page = poppler_document_get_page(_doc, pageNum);

            if(!page) return false;

            _pageNum = pageNum;

            double w = 0.0f;
            double h = 0.0f;

            poppler_page_get_size(page, &w, &h);

            _cairoImage->create((unsigned int)(w*2.0),(unsigned int)(h*2.0));

            osg::clearImageToColor(this, _backgroundColor);

            cairo_save(_cairoImage->getContext());

                cairo_rectangle(_cairoImage->getContext(), 0.0, 0.0, double(s()), double(t()));
                cairo_scale(_cairoImage->getContext(), double(s())/w, double(t())/h);
                poppler_page_render(page, _cairoImage->getContext());

            cairo_restore(_cairoImage->getContext());

            dirty();

            return true;
    }


    protected:

        osg::ref_ptr<CairoImage> _cairoImage;

};


class ReaderWriterPDF : public osgDB::ReaderWriter
{
    public:

        ReaderWriterPDF()
        {
            supportsExtension("pdf","PDF plugin");
        }

        virtual const char* className() const { return "PDF plugin"; }

        virtual osgDB::ReaderWriter::ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            return readImage(file,options);
        }

        virtual osgDB::ReaderWriter::ReadResult readImage(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            if (!osgDB::equalCaseInsensitive(osgDB::getFileExtension(fileName),"pdf"))
            {
                return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;
            }

            std::string file = osgDB::findDataFile(fileName);
            if (file.empty())
            {
                return osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND;
            }

            osg::ref_ptr<PopplerPdfImage> image = new PopplerPdfImage;
            image->setDataVariance(osg::Object::DYNAMIC);

            image->setOrigin(osg::Image::TOP_LEFT);

            if (!image->open(file))
            {
                return "Could not open "+file;
            }

            return image.get();
        }

        virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            osgDB::ReaderWriter::ReadResult result = readImage(fileName, options);
            if (!result.validImage()) return result;


            osg::ref_ptr<osgWidget::PdfReader> pdfReader = new osgWidget::PdfReader();
            if (pdfReader->assign(dynamic_cast<osgWidget::PdfImage*>(result.getImage())))
            {
                return pdfReader.release();
            }
            else
            {
                return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;
            }
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(pdf, ReaderWriterPDF)

