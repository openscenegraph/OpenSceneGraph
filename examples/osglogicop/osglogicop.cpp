#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>
#include <osg/LogicOp>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgProducer/Viewer>

#include <osgUtil/Optimizer>

const int _ops_nb=16;
const osg::LogicOp::Opcode _operations[_ops_nb]=
{    
    osg::LogicOp::CLEAR,
    osg::LogicOp::SET,
    osg::LogicOp::COPY,
    osg::LogicOp::COPY_INVERTED,
    osg::LogicOp::NOOP,
    osg::LogicOp::INVERT,
    osg::LogicOp::AND,
    osg::LogicOp::NAND,
    osg::LogicOp::OR,
    osg::LogicOp::NOR,
    osg::LogicOp::XOR,
    osg::LogicOp::EQUIV,
    osg::LogicOp::AND_REVERSE,
    osg::LogicOp::AND_INVERTED,
    osg::LogicOp::OR_REVERSE,
    osg::LogicOp::OR_INVERTED
};

const char* _ops_name[_ops_nb]=
{    
    "osg::LogicOp::CLEAR",
    "osg::LogicOp::SET",
    "osg::LogicOp::COPY",
    "osg::LogicOp::COPY_INVERTED",
    "osg::LogicOp::NOOP",
    "osg::LogicOp::INVERT",
    "osg::LogicOp::AND",
    "osg::LogicOp::NAND",
    "osg::LogicOp::OR",
    "osg::LogicOp::NOR",
    "osg::LogicOp::XOR",
    "osg::LogicOp::EQUIV",
    "osg::LogicOp::AND_REVERSE",
    "osg::LogicOp::AND_INVERTED",
    "osg::LogicOp::OR_REVERSE",
    "osg::LogicOp::OR_INVERTED"
};

class TechniqueEventHandler : public osgGA::GUIEventHandler
{
public:

    TechniqueEventHandler(osg::LogicOp* logicOp) { _logicOp =logicOp;_ops_index=_ops_nb-1;}
    TechniqueEventHandler() { std::cerr<<"Error, can't initialize it!";}

    META_Object(osglogicopApp,TechniqueEventHandler);

    virtual void accept(osgGA::GUIEventHandlerVisitor& v) { v.visit(*this); }

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);

    virtual void getUsage(osg::ApplicationUsage& usage) const;

protected:

    ~TechniqueEventHandler() {}

    TechniqueEventHandler(const TechniqueEventHandler&,const osg::CopyOp&) {}

    osg::LogicOp*       _logicOp;
    int                 _ops_index;

};

bool TechniqueEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Right ||
                ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Right)
            {
                _ops_index++;
                if (_ops_index>=_ops_nb) _ops_index=0;
                _logicOp->setOpcode(_operations[_ops_index]);
                std::cout<<"Operation name = "<<_ops_name[_ops_index]<<std::endl;
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Left ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Left)
            {
                _ops_index--;
                if (_ops_index<0) _ops_index=_ops_nb-1;
                _logicOp->setOpcode(_operations[_ops_index]);
                std::cout<<"Operation name = "<<_ops_name[_ops_index]<<std::endl;
                return true;
            }
            return false;
        }

        default:
            return false;
    }
}

void TechniqueEventHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("- or Left Arrow","Advance to next opcode");
    usage.addKeyboardMouseBinding("+ or Right Array","Move to previous opcode");
}




int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates how to use glLogicOp for mixing rendered scene and the frame-buffer.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
   
    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // load the nodes from the commandline arguments.
    osg::Node* loadedModel = osgDB::readNodeFiles(arguments);
    if (!loadedModel)
    {
        return 1;
    }
  
    osg::Group* root = new osg::Group;
    root->addChild(loadedModel);
    
    
    osg::StateSet*  stateset =  new osg::StateSet;
    osg::LogicOp*   logicOp =   new osg::LogicOp(osg::LogicOp::OR_INVERTED);

    stateset->setAttributeAndModes(logicOp,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

    //tell to sort the mesh before displaying it
    stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);


    loadedModel->setStateSet(stateset);




    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    viewer.getEventHandlerList().push_front(new TechniqueEventHandler(logicOp));

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
    
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

 
    
    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(root);
     
    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData( root );
    
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
