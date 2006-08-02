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
#include <osgUtil/Simplifier>
#include <osgProducer/Viewer>


class KeyboardEventHandler : public osgGA::GUIEventHandler
{
public:
    
    KeyboardEventHandler(unsigned int& flag) : _flag(flag)
    {}
    
    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
    {
        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYDOWN):
            {
                if (ea.getKey()=='n')
                {
                    _flag = 1;
                    return true;
                }
                if (ea.getKey()=='p')
                {
                    _flag = 2;
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

private:

    unsigned int& _flag;
};


int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" examples illustrates simplification of triangle meshes.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--ratio <ratio>","Specify the sample ratio","0.5]");
    arguments.getApplicationUsage()->addCommandLineOption("--max-error <error>","Specify the maximum error","4.0");
    

    float sampleRatio = 0.5f;
    float maxError = 4.0f;

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    // read the sample ratio if one is supplied
    while (arguments.read("--ratio",sampleRatio)) {}
    while (arguments.read("--max-error",maxError)) {}

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

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

    osg::Timer_t start_tick = osg::Timer::instance()->tick();

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

    // if no model has been successfully loaded report failure.
    if (!loadedModel) 
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
    }

    osg::Timer_t end_tick = osg::Timer::instance()->tick();

    std::cout << "Time to load = "<<osg::Timer::instance()->delta_s(start_tick,end_tick)<<std::endl;

    
    //loadedModel->accept(simplifier);

    unsigned int keyFlag = 0;
    viewer.getEventHandlerList().push_front(new KeyboardEventHandler(keyFlag));

    // set the scene to render
    viewer.setSceneData(loadedModel.get());

    // create the windows and run the threads.
    viewer.realize();

    float multiplier = 0.99f;
    float minRatio = 0.001f;
    float ratio = sampleRatio;

    while( !viewer.done() )
    {
        // wait for all cull and draw threads to complete.
        viewer.sync();

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        viewer.update();
         
        // fire off the cull and draw traversals of the scene.
        viewer.frame();
    
        if (keyFlag == 1 || keyFlag == 2)
        {
            if (keyFlag == 1) ratio *= multiplier;
            if (keyFlag == 2) ratio /= multiplier;
            if (ratio<minRatio) ratio=minRatio;
            
            osgUtil::Simplifier simplifier(ratio, maxError);

            std::cout<<"Runing osgUtil::Simplifier with SampleRatio="<<ratio<<" maxError="<<maxError<<" ...";
            std::cout.flush();
            
            osg::ref_ptr<osg::Node> root = (osg::Node*)loadedModel->clone(osg::CopyOp::DEEP_COPY_ALL);

            root->accept(simplifier);
            
            std::cout<<"done"<<std::endl;
            
            viewer.setSceneData(root.get());
            keyFlag = 0;
        }
    }
    
    // wait for all cull and draw threads to complete.
    viewer.sync();

    // run a clean up frame to delete all OpenGL objects.
    viewer.cleanup_frame();

    // wait for all the clean up frame to complete.
    viewer.sync();

    return 0;
}

