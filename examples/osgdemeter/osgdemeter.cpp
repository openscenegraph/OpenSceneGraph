/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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
#include <osgDB/Registry>
#include <osgUtil/Optimizer>
#include <osgProducer/Viewer>

#include <Demeter/Terrain.h>
#include <Demeter/Loader.h>
#include <Demeter/DemeterDrawable.h>

Demeter::Terrain* loadTerrain()
{

#ifdef _WIN32
    char fileSeparator = '\\';
#else
    char fileSeparator = '/';
#endif
    char szMediaPath[17];
    sprintf(szMediaPath,"..%cdata%cLlano",fileSeparator,fileSeparator);
    Demeter::Settings::GetInstance()->SetMediaPath(szMediaPath);

    Demeter::Settings::GetInstance()->SetTessellateMethod(Demeter::Settings::TM_2D_ROLLONLY);

    // Load a terrain that was generated in the Demeter Texture Editor
    Demeter::Terrain* pTerrain = new Demeter::Terrain(500000,0.0f,0.0f);
    try
    {
#ifdef _DEBUG
        Demeter::Loader::GetInstance()->LoadElevations("DemeterElevationLoaderDebug","Llano.terrain",pTerrain);
        Demeter::Loader::GetInstance()->LoadTerrainTexture("DemeterTextureLoaderDebug","Llano.terrain",pTerrain);
#else
        Demeter::Loader::GetInstance()->LoadElevations("DemeterElevationLoader","Llano.terrain",pTerrain);
        Demeter::Loader::GetInstance()->LoadTerrainTexture("DemeterTextureLoader","Llano.terrain",pTerrain);
#endif
    }
    catch(Demeter::DemeterException* pEx)
    {
        std::cerr<<pEx->GetErrorMessage()<<std::endl;
        return 0;
    }
    return pTerrain;
}

osg::Node* createSceneWithTerrain(Demeter::Terrain* pTerrain)
{
    Demeter::DemeterDrawable* pDrawable = new Demeter::DemeterDrawable;
    pDrawable->SetTerrain(pTerrain);

    osg::Geode* pGeode = new osg::Geode;
    pGeode->addDrawable(pDrawable);

    float detailThreshold = 9.0f;
    pTerrain->SetDetailThreshold(detailThreshold);

    return pGeode;
}

class KeyboardEventHandler : public osgGA::GUIEventHandler
{
public:
    
        KeyboardEventHandler(Demeter::Terrain* pTerrain):
            _pTerrain(pTerrain) {}
    
        virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
        {
            switch(ea.getEventType())
            {
                case(osgGA::GUIEventAdapter::KEYDOWN):
                {
                    if (ea.getKey()=='n')
                    {
                        _pTerrain->SetDetailThreshold(_pTerrain->GetDetailThreshold()+0.1f);
                        std::cout << "_pTerrain->GetDetailThreshold() = "<<_pTerrain->GetDetailThreshold() << std::endl;
                       return true;
                    }
                    else if (ea.getKey()=='n')
                    {
                        _pTerrain->SetDetailThreshold(_pTerrain->GetDetailThreshold()-0.1f);
                        std::cout << "_pTerrain->GetDetailThreshold() = "<<_pTerrain->GetDetailThreshold() << std::endl;
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
        
        Demeter::Terrain* _pTerrain;
        
};


int main( int argc, char **argv )
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

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
//     
//     if (arguments.argc()<=1)
//     {
//         arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
//         return 1;
//     }

    osg::Timer timer;
    osg::Timer_t start_tick = timer.tick();


    Demeter::Terrain* pTerrain = loadTerrain();

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = createSceneWithTerrain(pTerrain);


    // if no model has been successfully loaded report failure.
    if (!loadedModel) 
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    osg::Timer_t end_tick = timer.tick();

    std::cout << "Time to load = "<<timer.delta_s(start_tick,end_tick)<<std::endl;


    // set the scene to render
    viewer.setSceneData(loadedModel.get());


    viewer.getEventHandlerList().push_front(new KeyboardEventHandler(pTerrain));

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

