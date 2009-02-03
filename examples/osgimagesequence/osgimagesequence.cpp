/* OpenSceneGraph example, osgtexture3D.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osg/Node>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/TextureRectangle>
#include <osg/ImageSequence>
#include <osg/Geode>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <iostream>




//
// A simple demo demonstrating how to set on an animated texture using an osg::ImageSequence
//

osg::StateSet* createState(osg::ArgumentParser& arguments)
{
    osg::ref_ptr<osg::ImageSequence> imageSequence = new osg::ImageSequence;

    bool preLoad = true;
        
    while (arguments.read("--page-and-discard"))
    {
        imageSequence->setMode(osg::ImageSequence::PAGE_AND_DISCARD_USED_IMAGES);
        preLoad = false;
    }
    
    while (arguments.read("--page-and-retain"))
    {
        imageSequence->setMode(osg::ImageSequence::PAGE_AND_RETAIN_IMAGES);
        preLoad = false;
    }
    
    while (arguments.read("--preload"))
    {
        imageSequence->setMode(osg::ImageSequence::PRE_LOAD_ALL_IMAGES);
        preLoad = true;
    }
    
    double length = -1.0;
    while (arguments.read("--length",length)) {}
    
    if (arguments.argc()>1)
    {
        for(int i=1; i<arguments.argc(); ++i)
        {
            if (preLoad)
            {
                osg::ref_ptr<osg::Image> image = osgDB::readImageFile(arguments[i]);
                if (image.valid())
                {
                    imageSequence->addImage(image.get());
                }
            }
            else
            {
                imageSequence->addImageFile(arguments[i]);
            }
        }
        
        if (length>0.0)
        {
            imageSequence->setLength(length);
        }
        else
        {
            unsigned int maxNum = osg::maximum(imageSequence->getFileNames().size(),
                                               imageSequence->getImages().size());
                                               
            imageSequence->setLength(float(maxNum)*0.1f);
        }
    }
    else
    {
        if (length>0.0)
        {
            imageSequence->setLength(length);
        }
        else
        {
            imageSequence->setLength(4.0);
        }
        imageSequence->addImage(osgDB::readImageFile("Cubemap_axis/posx.png"));
        imageSequence->addImage(osgDB::readImageFile("Cubemap_axis/negx.png"));
        imageSequence->addImage(osgDB::readImageFile("Cubemap_axis/posy.png"));
        imageSequence->addImage(osgDB::readImageFile("Cubemap_axis/negy.png"));
        imageSequence->addImage(osgDB::readImageFile("Cubemap_axis/posz.png"));
        imageSequence->addImage(osgDB::readImageFile("Cubemap_axis/negz.png"));
    }
        
    // start the image sequence playing
    imageSequence->play();

#if 1
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
    texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
    texture->setWrap(osg::Texture::WRAP_R,osg::Texture::REPEAT);
    texture->setResizeNonPowerOfTwoHint(false);
    texture->setImage(imageSequence.get());
    //texture->setTextureSize(512,512);
#else    
    osg::TextureRectangle* texture = new osg::TextureRectangle;
    texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
    texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
    texture->setWrap(osg::Texture::WRAP_R,osg::Texture::REPEAT);
    texture->setImage(imageSequence.get());
    //texture->setTextureSize(512,512);
#endif

    // create the StateSet to store the texture data
    osg::StateSet* stateset = new osg::StateSet;

    stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);

    return stateset;
}

osg::Node* createModel(osg::ArgumentParser& arguments)
{

    // create the geometry of the model, just a simple 2d quad right now.    
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(osg::createTexturedQuadGeometry(osg::Vec3(0.0f,0.0f,0.0), osg::Vec3(1.0f,0.0f,0.0), osg::Vec3(0.0f,0.0f,1.0f)));

    geode->setStateSet(createState(arguments));
    
    return geode;

}


osg::ImageStream* s_imageStream = 0;
class MovieEventHandler : public osgGA::GUIEventHandler
{
public:

    MovieEventHandler():_playToggle(true),_trackMouse(false) {}
    
    void setMouseTracking(bool track) { _trackMouse = track; }
    bool getMouseTracking() const { return _trackMouse; }
    
    void set(osg::Node* node);

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* nv);
    
    virtual void getUsage(osg::ApplicationUsage& usage) const;

    typedef std::vector< osg::observer_ptr<osg::ImageStream> > ImageStreamList;

protected:

    virtual ~MovieEventHandler() {}

    class FindImageStreamsVisitor : public osg::NodeVisitor
    {
    public:
        FindImageStreamsVisitor(ImageStreamList& imageStreamList):
            _imageStreamList(imageStreamList) {}
            
        virtual void apply(osg::Geode& geode)
        {
            apply(geode.getStateSet());

            for(unsigned int i=0;i<geode.getNumDrawables();++i)
            {
                apply(geode.getDrawable(i)->getStateSet());
            }
        
            traverse(geode);
        }

        virtual void apply(osg::Node& node)
        {
            apply(node.getStateSet());
            traverse(node);
        }
        
        inline void apply(osg::StateSet* stateset)
        {
            if (!stateset) return;
            
            osg::StateAttribute* attr = stateset->getTextureAttribute(0,osg::StateAttribute::TEXTURE);
            if (attr)
            {
                osg::Texture2D* texture2D = dynamic_cast<osg::Texture2D*>(attr);
                if (texture2D) apply(dynamic_cast<osg::ImageStream*>(texture2D->getImage()));

                osg::TextureRectangle* textureRec = dynamic_cast<osg::TextureRectangle*>(attr);
                if (textureRec) apply(dynamic_cast<osg::ImageStream*>(textureRec->getImage()));
            }
        }
        
        inline void apply(osg::ImageStream* imagestream)
        {
            if (imagestream)
            {
                _imageStreamList.push_back(imagestream); 
                s_imageStream = imagestream;
            }
        }
        
        ImageStreamList& _imageStreamList;
        
    protected:
    
        FindImageStreamsVisitor& operator = (const FindImageStreamsVisitor&) { return *this; }
    };


    bool            _playToggle;
    bool            _trackMouse;
    ImageStreamList _imageStreamList;
    
};



void MovieEventHandler::set(osg::Node* node)
{
    _imageStreamList.clear();
    if (node)
    {
        FindImageStreamsVisitor fisv(_imageStreamList);
        node->accept(fisv);
    }
}


bool MovieEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* nv)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            if (ea.getKey()=='p')
            {
                for(ImageStreamList::iterator itr=_imageStreamList.begin();
                    itr!=_imageStreamList.end();
                    ++itr)
                {
                    if ((*itr)->getStatus()==osg::ImageStream::PLAYING)
                    {
                        // playing, so pause
                        std::cout<<"Pause"<<std::endl;
                        (*itr)->pause();
                    }
                    else
                    {
                        // playing, so pause
                        std::cout<<"Play"<<std::endl;
                        (*itr)->play();
                    }
                }
                return true;
            }
            else if (ea.getKey()=='r')
            {
                for(ImageStreamList::iterator itr=_imageStreamList.begin();
                    itr!=_imageStreamList.end();
                    ++itr)
                {
                    std::cout<<"Restart"<<std::endl;
                    (*itr)->rewind();
                }
                return true;
            }
            else if (ea.getKey()=='L')
            {
                for(ImageStreamList::iterator itr=_imageStreamList.begin();
                    itr!=_imageStreamList.end();
                    ++itr)
                {
                    if ( (*itr)->getLoopingMode() == osg::ImageStream::LOOPING)
                    {
                        std::cout<<"Toggle Looping Off"<<std::endl;
                        (*itr)->setLoopingMode( osg::ImageStream::NO_LOOPING );
                    }
                    else
                    {
                        std::cout<<"Toggle Looping On"<<std::endl;
                        (*itr)->setLoopingMode( osg::ImageStream::LOOPING );
                    }
                }
                return true;
            }
            return false;
        }

        default:
            return false;
    }
}

void MovieEventHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("p","Play/Pause movie");
    usage.addKeyboardMouseBinding("r","Restart movie");
    usage.addKeyboardMouseBinding("l","Toggle looping of movie");
}




int main(int argc, char **argv)
{
    osg::ArgumentParser arguments(&argc,argv);

    // construct the viewer.
    osgViewer::Viewer viewer(arguments);

    std::string filename;
    arguments.read("-o",filename);

    // create a model from the images and pass it to the viewer.
    viewer.setSceneData(createModel(arguments));

    // pass the model to the MovieEventHandler so it can pick out ImageStream's to manipulate.
    MovieEventHandler* meh = new MovieEventHandler();
    meh->set( viewer.getSceneData() );
    viewer.addEventHandler( meh );

    viewer.addEventHandler( new osgViewer::StatsHandler());

    if (!filename.empty())
    {
        osgDB::writeNodeFile(*viewer.getSceneData(),filename);
    }

    return viewer.run();
}
