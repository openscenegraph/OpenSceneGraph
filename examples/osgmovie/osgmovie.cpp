// -*-c++-*-

#include <osgProducer/Viewer>

#include <osgDB/ReadFile>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/TexMat>
#include <osg/CullFace>
#include <osg/ImageStream>

#include <osgGA/TrackballManipulator>

class MovieEventHandler : public osgGA::GUIEventHandler
{
public:

    MovieEventHandler() {}
    
    void set(osg::Node* node);

    virtual void accept(osgGA::GUIEventHandlerVisitor& v) { v.visit(*this); }

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);
    
    virtual void getUsage(osg::ApplicationUsage& usage) const;

    typedef std::vector< osg::ref_ptr<osg::ImageStream> > ImageStreamList;

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
            if (imagestream) _imageStreamList.push_back(imagestream); 
        }
        
        ImageStreamList& _imageStreamList;
    };


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


bool MovieEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            if (ea.getKey()=='s')
            {
                for(ImageStreamList::iterator itr=_imageStreamList.begin();
                    itr!=_imageStreamList.end();
                    ++itr)
                {
                    std::cout<<"Play"<<std::endl;
                     (*itr)->play();
                }
                return true;
            }
            else if (ea.getKey()=='p')
            {
                for(ImageStreamList::iterator itr=_imageStreamList.begin();
                    itr!=_imageStreamList.end();
                    ++itr)
                {
                    std::cout<<"Pause"<<std::endl;
                    (*itr)->pause();
                }
                return true;
            }
            else if (ea.getKey()=='r')
            {
                return true;
            }
            else if (ea.getKey()=='l')
            {
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
    usage.addKeyboardMouseBinding("p","Pause movie");
    usage.addKeyboardMouseBinding("s","Play movie");
    usage.addKeyboardMouseBinding("r","Start movie");
    usage.addKeyboardMouseBinding("l","Toggle looping of movie");
}


osg::Geometry* createTexturedQuadGeometry(const osg::Vec3& pos,float width,float height, osg::Image* image)
{
    bool useTextureRectangle = true;
    if (useTextureRectangle)
    {
        osg::Geometry* pictureQuad = osg::createTexturedQuadGeometry(pos,
                                           osg::Vec3(width,0.0f,0.0f),
                                           osg::Vec3(0.0f,0.0f,height),
                                           image->s(),image->t());
                                       
        pictureQuad->getOrCreateStateSet()->setTextureAttributeAndModes(0,
                    new osg::TextureRectangle(image),
                    osg::StateAttribute::ON);
                    
        return pictureQuad;
    }
    else
    {
        osg::Geometry* pictureQuad = osg::createTexturedQuadGeometry(pos,
                                           osg::Vec3(width,0.0f,0.0f),
                                           osg::Vec3(0.0f,0.0f,height),
                                           1.0f,1.0f);
                                       
        pictureQuad->getOrCreateStateSet()->setTextureAttributeAndModes(0,
                    new osg::Texture2D(image),
                    osg::StateAttribute::ON);

        return pictureQuad;
    }
}

int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the standard OpenSceneGraph example which loads and visualises 3d models.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // register the handler to add keyboard and mosue handling.
    MovieEventHandler* meh = new MovieEventHandler();
    viewer.getEventHandlerList().push_front(meh);


    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    osg::Geode* geode = new osg::Geode;
    osg::Vec3 pos(0.0f,0.0f,0.0f);
    
    for(int i=1;i<arguments.argc();++i)
    {
        if (arguments.isString(i))
        {
            osg::Image* image = osgDB::readImageFile(arguments[i]);
            geode->addDrawable(createTexturedQuadGeometry(pos,image->s(),image->t(),image));
        }

    }

    // pass the model to the MovieEventHandler so it can pick out ImageStream's to manipulate.
    meh->set(geode);

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }


    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
    }

    // set the scene to render
    viewer.setSceneData(geode);

    // create the windows and run the threads.
    viewer.realize();

    while( !viewer.done() )
    {
        // wait for all cull and draw threads to complete.
        viewer.sync();
        
        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        viewer.update();
         
        // fire off the cull and draw traversals of the scene.
        viewer.frame();
        
    }
    
    // wait for all cull and draw threads to complete before exit.
    viewer.sync();

    return 0;


}
