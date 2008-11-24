#include <osg/Image>
#include <osg/Geometry>
#include <osg/Texture2D>

#include <osgGA/TrackballManipulator>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <iostream>
#include <osg/io_utils>

#include <osgDB/ReadFile>


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
                
    
                
    osg::ref_ptr<osgViewer::InteractiveImageHandler> callback = new osgViewer::InteractiveImageHandler(image);

    pictureQuad->setEventCallback(callback.get());
    pictureQuad->setCullCallback(callback.get());

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(pictureQuad);
    
    return geode;
}

class EscapeHandler : public osgGA::GUIEventHandler
{
    public:
    
        EscapeHandler() {}

        bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
        {
            if (ea.getHandled()) return false;

            switch(ea.getEventType())
            {
                case(osgGA::GUIEventAdapter::KEYUP):
                {
                    if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Escape)
                    {
                        osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
                        if (view) view->getViewerBase()->setDone(true);
                        
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
    osgViewer::Viewer viewer;
    
    typedef std::list< osg::ref_ptr<osg::Image> > Images;
    Images images;

    std::string hostname;
    while (arguments.read("--host",hostname))
    {
        osg::ref_ptr<osg::Image> image = osgDB::readImageFile(hostname+std::string(".vnc"));
        if (image.valid()) images.push_back(image.get());
    }
    
    if (images.empty())
    {
        return 1;
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
    
    // add a custom escape handler, but disable the standard viewer one to enable the vnc images to handle
    // the escape without it getting caught by the viewer.
    viewer.addEventHandler(new EscapeHandler);    
    viewer.setKeyEventSetsDone(0);

    return viewer.run();
}

