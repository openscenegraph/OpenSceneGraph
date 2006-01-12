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
#include <osg/io_utils>

#include <osgGA/TrackballManipulator>
#include <osgGA/EventVisitor>

osg::ImageStream* s_imageStream = 0;
class PostSwapFinishCallback : public Producer::Camera::Callback
{
public:

    PostSwapFinishCallback() {}

    virtual void operator()(const Producer::Camera& camera)
    {
        // osg::Timer_t start_tick = osg::Timer::instance()->tick();
        
        osgProducer::OsgSceneHandler* sh = const_cast<osgProducer::OsgSceneHandler*>(dynamic_cast<const osgProducer::OsgSceneHandler*>(camera.getSceneHandler()));
    
        if (s_imageStream && s_imageStream->getPixelBufferObject()) s_imageStream->getPixelBufferObject()->compileBuffer(*(sh->getSceneView()->getState()));
        // glFinish();

        //osg::notify(osg::NOTICE)<<"callback after PBO "<<osg::Timer::instance()->delta_m(start_tick,osg::Timer::instance()->tick())<<"ms"<<std::endl;
    }
};

class MovieEventHandler : public osgGA::GUIEventHandler
{
public:

    MovieEventHandler() {}
    
    void set(osg::Node* node);

    virtual void accept(osgGA::GUIEventHandlerVisitor& v) { v.visit(*this); }

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* nv);
    
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
            if (imagestream)
            {
                _imageStreamList.push_back(imagestream); 
                s_imageStream = imagestream;
            }
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


bool MovieEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* nv)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::MOVE):
        case(osgGA::GUIEventAdapter::PUSH):
        case(osgGA::GUIEventAdapter::RELEASE):
        {
            osgProducer::Viewer* viewer = dynamic_cast<osgProducer::Viewer*>(&aa);
            osgUtil::IntersectVisitor::HitList hlist;
            if (viewer->computeIntersections(ea.getX(),ea.getY(), nv->getNodePath().back(), hlist))
            {
                if (!hlist.empty())
                {
                    // use the nearest intersection                 
                    osgUtil::Hit& hit = hlist.front();
                    osg::Drawable* drawable = hit.getDrawable();
                    osg::Geometry* geometry = drawable ? drawable->asGeometry() : 0;
                    osg::Vec3Array* vertices = geometry ? dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray()) : 0;

                    if (vertices)
                    {
                        // get the vertex indices.
                        const osgUtil::Hit::VecIndexList& vil = hit.getVecIndexList();
                        
                        if (vil.size()==3)
                        {

                            int i1 = vil[0];
                            int i2 = vil[1];
                            int i3 = vil[2];
                            osg::Vec3 v1 = (*vertices)[i1];
                            osg::Vec3 v2 = (*vertices)[i2];
                            osg::Vec3 v3 = (*vertices)[i3];
                            osg::Vec3 v = hit.getLocalIntersectPoint();
                            osg::Vec3 p1 = hit.getLocalLineSegment()->start();
                            osg::Vec3 p2 = hit.getLocalLineSegment()->end();
                            
                            osg::Vec3 p12 = p1-p2;
                            osg::Vec3 v13 = v1-v3;
                            osg::Vec3 v23 = v2-v3;
                            osg::Vec3 p1v3 = p1-v3;
                            
                            osg::Matrix matrix(p12.x(), v13.x(), v23.x(), 0.0,
                                               p12.y(), v13.y(), v23.y(), 0.0,
                                               p12.z(), v13.z(), v23.z(), 0.0,
                                               0.0,    0.0,    0.0,    1.0);
                                               
                            osg::Matrix inverse;
                            inverse.invert(matrix);
                            
                            osg::Vec3 ratio = inverse*p1v3;

                            // extract the baricentric coordinates.                            
                            float r1 = ratio.y();
                            float r2 = ratio.z();
                            float r3 = 1.0f-r1-r2;

                            osg::Array* texcoords = (geometry->getNumTexCoordArrays()>0) ? geometry->getTexCoordArray(0) : 0;
                            osg::Vec2Array* texcoords_Vec2Array = dynamic_cast<osg::Vec2Array*>(texcoords);
                            if (texcoords_Vec2Array)
                            {
                                // we have tex coord array so now we can compute the final tex coord at the point of intersection.                                
                                osg::Vec2 tc1 = (*texcoords_Vec2Array)[i1];
                                osg::Vec2 tc2 = (*texcoords_Vec2Array)[i2];
                                osg::Vec2 tc3 = (*texcoords_Vec2Array)[i3];
                                osg::Vec2 tc = tc1*r1 + tc2*r2 + tc3*r3;
                                
                                osg::notify(osg::NOTICE)<<"We hit tex coords "<<tc<<std::endl;
                                
                            }
                            
                            
                        }
                        else
                        {
                            osg::notify(osg::NOTICE)<<"Hit but insufficient indices to work with";
                        }
    
                    }

                } 
            }
            else
            {
                osg::notify(osg::NOTICE)<<"No hit"<<std::endl;
            }
            break;
        }
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
                for(ImageStreamList::iterator itr=_imageStreamList.begin();
                    itr!=_imageStreamList.end();
                    ++itr)
                {
                    std::cout<<"Restart"<<std::endl;
                    (*itr)->rewind();
                    (*itr)->play();
                }
                return true;
            }
            else if (ea.getKey()=='l')
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
    return false;
}

void MovieEventHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("p","Pause movie");
    usage.addKeyboardMouseBinding("s","Play movie");
    usage.addKeyboardMouseBinding("r","Restart movie");
    usage.addKeyboardMouseBinding("l","Toggle looping of movie");
}


osg::Geometry* myCreateTexturedQuadGeometry(const osg::Vec3& pos,float width,float height, osg::Image* image, bool useTextureRectangle)
{
    if (useTextureRectangle)
    {
        osg::Geometry* pictureQuad = osg::createTexturedQuadGeometry(pos,
                                           osg::Vec3(width,0.0f,0.0f),
                                           osg::Vec3(0.0f,0.0f,height),
                                           0.0f,image->t(), image->s(),0.0f);

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
                                           0.0f,1.0f, 1.0f,0.0f);
                                    
        osg::Texture2D* texture = new osg::Texture2D(image);
        texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);  
                                       
        pictureQuad->getOrCreateStateSet()->setTextureAttributeAndModes(0,
                    texture,
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
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" example demonstrates the use of ImageStream for rendering movies as textures.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    
    bool useTextureRectangle = true;
    bool useShader = false;

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);
    
    while (arguments.read("--texture2D")) useTextureRectangle=false;
    while (arguments.read("--shader")) useShader=true;

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    osg::Vec3 pos(0.0f,0.0f,0.0f);
        
    osg::StateSet* stateset = geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    if (useShader)
    {
        //useTextureRectangle = false;
        
        static const char *shaderSourceTextureRec = {
            "uniform vec4 cutoff_color;\n"
            "uniform samplerRect movie_texture;\n"
            "void main(void)\n"
            "{\n"
            "    vec4 texture_color = textureRect(movie_texture, gl_TexCoord[0]); \n"
            "    if (all(lessThanEqual(texture_color,cutoff_color))) discard; \n"
            "    gl_FragColor = texture_color;\n"
            "}\n"
        };

        static const char *shaderSourceTexture2D = {
            "uniform vec4 cutoff_color;\n"
            "uniform sampler2D movie_texture;\n"
            "void main(void)\n"
            "{\n"
            "    vec4 texture_color = texture2D(movie_texture, gl_TexCoord[0]); \n"
            "    if (all(lessThanEqual(texture_color,cutoff_color))) discard; \n"
            "    gl_FragColor = texture_color;\n"
            "}\n"
        };

        osg::Program* program = new osg::Program;
        
        program->addShader(new osg::Shader(osg::Shader::FRAGMENT,
                                           useTextureRectangle ? shaderSourceTextureRec : shaderSourceTexture2D));

        stateset->addUniform(new osg::Uniform("cutoff_color",osg::Vec4(0.1f,0.1f,0.1f,1.0f)));
        stateset->addUniform(new osg::Uniform("movie_texture",0));

        stateset->setAttribute(program);

    }


    for(int i=1;i<arguments.argc();++i)
    {
        if (arguments.isString(i))
        {
            osg::Image* image = osgDB::readImageFile(arguments[i]);
            osg::ImageStream* imagestream = dynamic_cast<osg::ImageStream*>(image);
            if (imagestream) imagestream->play();
            
            if (image)
            {
                geode->addDrawable(myCreateTexturedQuadGeometry(pos,image->s(),image->t(),image, useTextureRectangle));
                
                pos.z() += image->t()*1.5f;
            }
            else
            {
                std::cout<<"Unable to read file "<<arguments[i]<<std::endl;
            }            
        }
    }
    
    if (geode->getNumDrawables()==0)
    {
        // nothing loaded.
        return 1;
    }
    
    

    // pass the model to the MovieEventHandler so it can pick out ImageStream's to manipulate.
    MovieEventHandler* meh = new MovieEventHandler();
    geode->setEventCallback(meh);
    meh->set(geode.get());

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
/*
    // set up a post swap callback to flush deleted GL objects and compile new GL objects            
    for(unsigned int cameraNum=0;cameraNum<viewer.getNumberOfCameras();++cameraNum)
    {
        Producer::Camera* camera=viewer.getCamera(cameraNum);
        camera->addPostSwapCallback(new PostSwapFinishCallback());
    }
*/
    // set the scene to render
    viewer.setSceneData(geode.get());

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
