// -*-c++-*-

#include <osg/Group>
#include <osg/Sequence>
#include <osg/MatrixTransform>

#include <osgDB/ReadFile>

#include <osgProducer/Viewer>


//
// A simple demo demonstrating usage of osg::Sequence.
//

// simple event handler to start/stop sequences
class MyEventHandler : public osgGA::GUIEventHandler {
public:
    /// Constructor.
    MyEventHandler(std::vector<osg::Sequence*>* seq) 
    {
        _seq = seq;
    }

    /// Handle events.
    virtual bool handle(const osgGA::GUIEventAdapter& ea,
                        osgGA::GUIActionAdapter&)
    {
        bool handled = false;

        if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
        {
            const char keys[] = "!@#$%^&*()";
            for (unsigned int i = 0; i < (sizeof(keys) / sizeof(keys[0])); i++) {
                if (i < _seq->size() && ea.getKey() == keys[i])
                {
                    // toggle sequence
                    osg::Sequence* seq = (*_seq)[i];
                    osg::Sequence::SequenceMode mode = seq->getMode();
                    switch (mode) {
                    case osg::Sequence::START:
                        seq->setMode(osg::Sequence::PAUSE);
                        break;
                    case osg::Sequence::STOP:
                        seq->setMode(osg::Sequence::START);
                        break;
                    case osg::Sequence::PAUSE:
                        seq->setMode(osg::Sequence::RESUME);
                        break;
                    default:
                        break;
                    }
                    std::cerr << "Toggled sequence " << i << std::endl;
                    handled = true;
                }
            }
        }

        return handled;
    }

    /// accept visits.
    virtual void accept(osgGA::GUIEventHandlerVisitor&) {}

private:
    std::vector<osg::Sequence*>* _seq;
};

osg::Sequence* generateSeq(osg::Sequence::LoopMode mode,
                           float speed, int nreps,
                           std::vector<osg::Node*>& model)
{
    osg::Sequence* seqNode = new osg::Sequence;

    // add children, show each child for 1.0 seconds
    for (unsigned int i = 0; i < model.size(); i++) {
        seqNode->addChild(model[i]);
        seqNode->setTime(i, 1.0f);
    }

    // interval
    seqNode->setInterval(mode, 0, -1);

    // speed-up factor and number of repeats for entire sequence
    seqNode->setDuration(speed, nreps);

    // stopped
    seqNode->setMode(osg::Sequence::STOP);

    return seqNode;
}

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
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
    
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }
    
    // assumes any remaining parameters are models
    std::vector<osg::Node*> model;
    for (int i = 1; i < arguments.argc(); i++)
    {
        std::cerr << "Loading " << arguments[i] << std::endl;
        osg::Node* node = osgDB::readNodeFile(arguments[i]);
        if (node)
            model.push_back(node);
    }
    if (model.empty()) {
        return -1;
    }

    // root
    osg::Group* rootNode = new osg::Group;

    // create sequences
    std::vector<osg::Sequence*> seq;

    const osg::Sequence::LoopMode mode[] = { osg::Sequence::LOOP,
                                             osg::Sequence::SWING,
                                             osg::Sequence::LOOP };
    const float speed[] = { 0.5f, 1.0f, 1.5f };
    const int nreps[] = { -1, 5, 1 };

    float x = 0.0f;
    for (unsigned int j = 0; j < (sizeof(speed) / sizeof(speed[0])); j++) {
        osg::Sequence* seqNode = generateSeq(mode[j], speed[j], nreps[j],
                                             model);
        if (!seqNode)
            continue;
        seq.push_back(seqNode);

        // position sequence
        osg::Matrix matrix;
        matrix.makeTranslate(x, 0.0, 0.0);

        osg::MatrixTransform* xform = new osg::MatrixTransform;
        xform->setMatrix(matrix);
        xform->addChild(seqNode);

        rootNode->addChild(xform);

        x += seqNode->getBound()._radius * 1.5f;
    }


    // add model to viewer.
    viewer.setSceneData(rootNode);

    // register additional event handler
    viewer.getEventHandlerList().push_front(new MyEventHandler(&seq));

    // create the windows and run the threads.
    viewer.realize(Producer::CameraGroup::ThreadPerCamera);

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
