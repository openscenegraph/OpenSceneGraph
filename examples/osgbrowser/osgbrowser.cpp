#include <osg/Image>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/io_utils>
#include <osg/GraphicsThread>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>

#include <nsGUIEvent.h>

#include "llmozlib2.h"

#include "UBrowser.h"


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


int main( int argc, char* argv[] )
{
    osg::ArgumentParser arguments(&argc, argv);
    
    osgWidget::BrowserManager::instance()->init(arguments[0]);

    osgViewer::Viewer viewer(arguments);

    typedef std::list< osg::ref_ptr<osg::Image> > Images;
    Images images;

    for(int i=1; i<arguments.argc(); ++i)
    {
        if (!arguments.isOption(i))
        {
            std::string url_browser = std::string(arguments[i])+std::string(".browser");
            osg::ref_ptr<osg::Image> image = osgDB::readImageFile(url_browser);
            if (image.valid()) images.push_back(image.get());
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
    
    return viewer.run();
}
