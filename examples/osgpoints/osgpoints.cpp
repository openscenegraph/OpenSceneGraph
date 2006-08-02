/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgProducer/Viewer>

#include <osg/Point>
#include <osg/BlendFunc>
#include <osg/Texture2D>
#include <osg/PointSprite>

class KeyboardEventHandler : public osgGA::GUIEventHandler
{
public:
    
        KeyboardEventHandler(osg::StateSet* stateset):
            _stateset(stateset)
        {
            _point = new osg::Point;
            _point->setDistanceAttenuation(osg::Vec3(0.0,0.0000,0.05f));
            _stateset->setAttribute(_point.get());
        }
    
        virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
        {
            switch(ea.getEventType())
            {
                case(osgGA::GUIEventAdapter::KEYDOWN):
                {
                    if (ea.getKey()=='+' || ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Add)
                    {
                       changePointSize(1.0f);
                       return true;
                    }
                    else if (ea.getKey()=='-' || ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Subtract)
                    {
                       changePointSize(-1.0f);
                       return true;
                    }
                    else if (ea.getKey()=='<')
                    {
                       changePointAttenuation(1.1f);
                       return true;
                    }
                    else if (ea.getKey()=='>')
                    {
                       changePointAttenuation(1.0f/1.1f);
                       return true;
                    }
                    break;
                }
                default:
                    break;
            }
            return false;
        }

        virtual void accept(osgGA::GUIEventHandlerVisitor& v)
        {
            v.visit(*this);
        }
        
        
        float getPointSize() const
        {
            return _point->getSize();
        }
        
        void setPointSize(float psize)
        {
            if (psize>0.0)
            {
                _point->setSize(psize);
            }
            std::cout<<"Point size "<<psize<<std::endl;
        }

        void changePointSize(float delta)
        {
            setPointSize(getPointSize()+delta);
        }

        void changePointAttenuation(float scale)
        {
            _point->setDistanceAttenuation(_point->getDistanceAttenuation()*scale);
        }
        
        osg::ref_ptr<osg::StateSet> _stateset;
        osg::ref_ptr<osg::Point>    _point;
        
};

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" example provides an interactive viewer for visualising point clouds..");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--sprites","Point sprites.");
    

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    bool shader = false;
    while (arguments.read("--shader")) shader = true;

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }
    
    bool usePointSprites = false;
    while (arguments.read("--sprites")) { usePointSprites = true; };

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

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

    osg::Timer timer;
    osg::Timer_t start_tick = timer.tick();

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

    // if no model has been successfully loaded report failure.
    if (!loadedModel) 
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    osg::Timer_t end_tick = timer.tick();

    std::cout << "Time to load = "<<timer.delta_s(start_tick,end_tick)<<std::endl;

    // optimize the scene graph, remove rendundent nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel.get());


    // set the scene to render
    viewer.setSceneData(loadedModel.get());
    

    if (usePointSprites)    
    {
        osg::StateSet* stateset = loadedModel->getOrCreateStateSet();

        /// Setup cool blending
        osg::BlendFunc *fn = new osg::BlendFunc();
        stateset->setAttributeAndModes(fn, osg::StateAttribute::ON);

        /// Setup the point sprites
        osg::PointSprite *sprite = new osg::PointSprite();
        stateset->setTextureAttributeAndModes(0, sprite, osg::StateAttribute::ON);

        /// The texture for the sprites
        osg::Texture2D *tex = new osg::Texture2D();
        tex->setImage(osgDB::readImageFile("Images/particle.rgb"));
        stateset->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON);
    }
    

    // register the handler for modifying the point size
    viewer.getEventHandlerList().push_front(new KeyboardEventHandler(viewer.getGlobalStateSet()));


    if (shader)
    {
        osg::StateSet* stateset = loadedModel->getOrCreateStateSet();
    
        ///////////////////////////////////////////////////////////////////
        // vertex shader using just Vec4 coefficients
        char vertexShaderSource[] = 
            "void main(void) \n"
            "{ \n"
            "\n"
            "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
            "}\n";



        osg::Program* program = new osg::Program;
        stateset->setAttribute(program);

        osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource);
        program->addShader(vertex_shader);

#if 0
        //////////////////////////////////////////////////////////////////
        // fragment shader
        //
        char fragmentShaderSource[] = 
            "void main(void) \n"
            "{ \n"
            "    gl_FragColor = gl_Color; \n"
            "}\n";

        osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource);
        program->addShader(fragment_shader);
#endif
    }



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
    
    // wait for all cull and draw threads to complete.
    viewer.sync();

    // run a clean up frame to delete all OpenGL objects.
    viewer.cleanup_frame();

    // wait for all the clean up frame to complete.
    viewer.sync();

    return 0;
}

