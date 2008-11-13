#include <osg/Image>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/io_utils>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>


class HttpImage : public osg::Image
{
    public:
    
        HttpImage() {}
        
        bool open(const std::string& filename)
        {
            osg::notify(osg::NOTICE)<<"open("<<filename<<")"<<std::endl;

            int width = 1024;
            int height = 1024;

            allocateImage(width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE);
            setPixelFormat(GL_BGRA);
            setDataVariance(osg::Object::DYNAMIC);            
            setOrigin(osg::Image::TOP_LEFT);

            return true;
        }
        
        virtual void sendPointerEvent(int x, int y, int buttonMask)
        {
            osg::notify(osg::NOTICE)<<"sendPointerEvent("<<x<<","<<y<<","<<buttonMask<<")"<<std::endl;
        }

        virtual void sendKeyEvent(int key, bool keyDown)
        {
            osg::notify(osg::NOTICE)<<"sendKeyEvent("<<key<<","<<keyDown<<")"<<std::endl;
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
            osg::ref_ptr<HttpImage> httpImage= new HttpImage;
            if (httpImage->open(arguments[i]))
            {            
                images.push_back(httpImage.get());
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

