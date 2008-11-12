#include <osg/Image>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/io_utils>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgDB/ReadFile>

#include <cairo.h>
#include <poppler.h>

class CarioImage : public osg::Image
{
    public:
    
        CarioImage():
            _surface(0),
            _context(0) {}
            
        
        void create(unsigned int width, unsigned int height)
        {
            if (data() && width==s() && height==t()) return;

            osg::notify(osg::NOTICE)<<"Create cario surface/context "<<width<<", "<<height<<std::endl;

            // allocate the image data
            allocateImage(width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE);
            setPixelFormat(GL_BGRA);
            setDataVariance(osg::Object::DYNAMIC);            
            setOrigin(osg::Image::TOP_LEFT);
            
            
            // create a cairo surface for this image data
            _surface = cairo_image_surface_create_for_data(
                    data(),
                    CAIRO_FORMAT_ARGB32, 
                    width, height, 
                    getRowSizeInBytes());
            
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
    
        virtual ~CarioImage()
        {
            destroy();
        }

        cairo_surface_t* _surface;
        cairo_t*         _context;
};

class PdfImage : public CarioImage
{
    public:
    
        PdfImage():
            _doc(0),
            _pageNum(0)
        {
        }
            
        virtual ~PdfImage()
        {
            if (_doc)
            {
        	g_object_unref(_doc);
            }
        }
    
        PopplerDocument* _doc;
        int _pageNum;
    
	int getNumOfPages() { return _doc ? poppler_document_get_n_pages(_doc) : 0; }

        bool open(const std::string& filename)
        {
            osg::notify(osg::NOTICE)<<"open("<<filename<<")"<<std::endl;

	    static bool gTypeInit = false;

	    if(!gTypeInit)
            {
		    g_type_init();

		    gTypeInit = true;
	    }

            PopplerDocument* doc = poppler_document_new_from_file(filename.c_str(), NULL, NULL);
            if (!doc) 
            {
                osg::notify(osg::NOTICE)<<" could not open("<<filename<<")"<<std::endl;

                return false;
            }
            
            if (_doc) 
            {
                g_object_unref(_doc);
            }
            
            _doc = doc;
            _pageNum = 0;
            
            setFileName(filename);

            osg::notify(osg::NOTICE)<<"getNumOfPages()=="<<getNumOfPages()<<std::endl;

            if (getNumOfPages()==0)
            {
                return false;
            }

            page(0);

            return true;
        }
        
        virtual void sendKeyEvent(int key, bool keyDown)
        {
            if (keyDown)
            {
                if (key=='n') next();
                else if (key=='p') previous();
            }
        }

 
        bool previous()
        {
            return page(_pageNum-1);
        }
        
        bool next()
        { 
             return page(_pageNum+1);
        }
        
        bool page(int pageNum)
        {
            if (!_doc) return false;
            
            if (pageNum<0 || pageNum>=getNumOfPages()) return false;

	    PopplerPage* page = poppler_document_get_page(_doc, pageNum);

	    if(!page) return false;
            
            _pageNum = pageNum;

            double w = 0.0f;
            double h = 0.0f;

            poppler_page_get_size(page, &w, &h);

            create((unsigned int)(w),(unsigned int)(h));

            double r = 1.0;
            double g = 1.0;
            double b = 1.0;
            double a = 1.0;
            
            cairo_save(_context);
            
                cairo_set_source_rgba(_context, r, g, b, a);
                cairo_rectangle(_context, 0.0, 0.0, w, h);
                cairo_fill(_context);

                poppler_page_render(page, getContext());
            
            cairo_restore(_context);

            dirty();

	}


};


osg::Node* createInteractiveQuad(const osg::Vec3& origin, osg::Vec3& widthAxis, osg::Vec3& heightAxis, 
                                 osg::Image* image)
{
    bool flip = image->getOrigin()==osg::Image::TOP_LEFT;

    osg::Geometry* pictureQuad = osg::createTexturedQuadGeometry(origin, widthAxis, heightAxis,
                                       0.0f, flip ? 1.0f : 0.0f , 1.0f, flip ? 0.0f : 1.0f);

    osg::Texture2D* texture = new osg::Texture2D(image);
    texture->setResizeNonPowerOfTwoHint(false);
    texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
    texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    
    pictureQuad->getOrCreateStateSet()->setTextureAttributeAndModes(0,
                texture,
                osg::StateAttribute::ON);
                
    pictureQuad->setEventCallback(new osgViewer::InteractiveImageHandler(image));

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(pictureQuad);
    
    return geode;
}

class PageHandler : public osgGA::GUIEventHandler
{
    public:
    
        PageHandler() {}

        bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
        {
            if (ea.getHandled()) return false;

            switch(ea.getEventType())
            {
                case(osgGA::GUIEventAdapter::KEYDOWN):
                {
                    if (ea.getKey()=='n')
                    {
                        osg::notify(osg::NOTICE)<<"Next page"<<std::endl;
                        return true;
                    }
                    else
                    if (ea.getKey()=='p')
                    {
                        osg::notify(osg::NOTICE)<<"Previous page"<<std::endl;
                        return true;
                    }
                }

                default:
                    return false;
            }
            return false;
        }
};

int main(int argc,char** argv)
{

    osg::ArgumentParser arguments(&argc, argv);
    osgViewer::Viewer viewer(arguments);
    
    typedef std::list< osg::ref_ptr<osg::Image> > Images;
    Images images;

    for(int i=1; i<arguments.argc(); ++i)
    {
        if (!arguments.isOption(i))
        {
            osg::ref_ptr<PdfImage> pdfImage= new PdfImage;
            if (pdfImage->open(arguments[i]))
            {            
                images.push_back(pdfImage.get());
            }
        }
    }

    bool xyPlane = false;

    osg::Group* group = new osg::Group;

    osg::Vec3 origin = osg::Vec3(0.0f,0.0f,0.0f);
    for(Images::iterator itr = images.begin();
        itr != images.end();
        ++itr)
    {
        osg::Image* image = itr->get();
        float width = 1.0;
        float height = float(image->t())/float(image->s());
        osg::Vec3 widthAxis = osg::Vec3(width,0.0f,0.0f);
        osg::Vec3 heightAxis = xyPlane ? osg::Vec3(0.0f,height,0.0f) : osg::Vec3(0.0f,0.0f,height);
        group->addChild(createInteractiveQuad(origin, widthAxis, heightAxis, image));
        
        origin += widthAxis*1.1f;
    }
    
    viewer.setSceneData(group);

    viewer.addEventHandler(new osgViewer::StatsHandler);
    
    //viewer.addEventHandler(new PageHandler);

    return viewer.run();
}

