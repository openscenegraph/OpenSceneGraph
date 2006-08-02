// -*-c++-*-

/*
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial
 * applications,as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osgText/Text>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Projection>
#include <osg/Sequence>
#include <osg/MatrixTransform>

#include <osgDB/ReadFile>

#include <osgProducer/Viewer>


// create text drawable at 'pos'
osg::Geode* createText(const std::string& str, const osg::Vec3& pos)
{
    // text drawable
    osgText::Text* text = new osgText::Text;
    text->setFont(std::string("fonts/arial.ttf"));
    text->setPosition(pos);
    text->setText(str);

    // geode
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(text);

    return geode;
}

osg::Node* createTextGroup(const char** text)
{
    osg::Group* group = new osg::Group;

    osg::Vec3 pos(120.0f, 800.0f, 0.0f);
    const osg::Vec3 delta(0.0f, -60.0f, 0.0f);

    // header
    const char** t = text;
    group->addChild(createText(*t++, pos));
    pos += delta;

    // remainder of text under sequence
    osg::Sequence* seq = new osg::Sequence;
    group->addChild(seq);
    while (*t) {
        seq->addChild(createText(*t++, pos));
        seq->setTime(seq->getNumChildren()-1, 2.0f);
        pos += delta;
    }

    // loop through all children
    seq->setInterval(osg::Sequence::LOOP, 0,-1);

    // real-time playback, repeat indefinitively
    seq->setDuration(1.0f, -1);

    // must be started explicitly
    seq->setMode(osg::Sequence::START);

    return group;
}

osg::Node* createHUD(osg::Node* node)
{
    // absolute transform
    osg::MatrixTransform* modelview_abs = new osg::MatrixTransform;
    modelview_abs->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    modelview_abs->setMatrix(osg::Matrix::identity());
    modelview_abs->addChild(node);

    // 2D projection node
    osg::Projection* projection = new osg::Projection;
    projection->setMatrix(osg::Matrix::ortho2D(0,1280,0,1024));
    projection->addChild(modelview_abs);

    // turn off lighting and depth test
    osg::StateSet* state = modelview_abs->getOrCreateStateSet();
    state->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    state->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

    return projection;
}

osg::Node* createScaledNode(osg::Node* node, float targetScale)
{
    // create scale matrix
    osg::MatrixTransform* transform = new osg::MatrixTransform;

    const osg::BoundingSphere& bsphere = node->getBound();
    float scale = targetScale / bsphere._radius;
    transform->setMatrix(osg::Matrix::scale(scale,scale,scale));
    transform->setDataVariance(osg::Object::STATIC);
    transform->addChild(node);

    // rescale normals
    osg::StateSet* state = transform->getOrCreateStateSet();
    state->setMode(GL_NORMALIZE, osg::StateAttribute::ON);

    return transform;
}

osg::Sequence* createSequence(osg::ArgumentParser& arguments)
{
    // assumes any remaining parameters are models
    osg::Sequence* seq = new osg::Sequence;
    for (int i = 1; i < arguments.argc(); ++i)
    {
        // load model
        osg::Node* node = osgDB::readNodeFile(arguments[i]);
        if (!node) {
            continue;
        }
        seq->addChild(createScaledNode(node, 100.0f));
        seq->setTime(seq->getNumChildren()-1, 1.0f);
    }

    // loop through all children
    seq->setInterval(osg::Sequence::LOOP, 0,-1);

    // real-time playback, repeat indefinitively
    seq->setDuration(1.0f, -1);

    return seq;
}

// event handler to control sequence
class SequenceEventHandler : public osgGA::GUIEventHandler
{
public:
    SequenceEventHandler(osg::Sequence* seq)
    {
        _seq = seq;
    }

    // handle keydown events
    virtual bool handle(const osgGA::GUIEventAdapter& ea,
                        osgGA::GUIActionAdapter&)
    {
        if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN) {
            switch (ea.getKey()) {
            case 'S':
                {
                    osg::Sequence::SequenceMode mode = _seq->getMode();
                    if (mode == osg::Sequence::STOP) {
                        mode = osg::Sequence::START;
                        std::cerr << "Start" << std::endl;
                    }
                    else if (mode == osg::Sequence::PAUSE) {
                        mode = osg::Sequence::RESUME;
                        std::cerr << "Resume" << std::endl;
                    }
                    else {
                        mode = osg::Sequence::PAUSE;
                        std::cerr << "Pause" << std::endl;
                    }
                    _seq->setMode(mode);
                }
                break;
            case 'L':
                {
                    osg::Sequence::LoopMode mode;
                    int begin, end;
                    _seq->getInterval(mode, begin, end);
                    if (mode == osg::Sequence::LOOP) {
                        mode = osg::Sequence::SWING;
                        std::cerr << "Swing" << std::endl;
                    }
                    else {
                        mode = osg::Sequence::LOOP;
                        std::cerr << "Loop" << std::endl;
                    }
                    _seq->setInterval(mode, begin, end);
                }
                break;
            default:
                break;
            }
        }

        return false;
    }

    // accept visits
    virtual void accept(osgGA::GUIEventHandlerVisitor&) {}

private:
    osg::ref_ptr<osg::Sequence> _seq;
};


int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use of osg::Sequence.");
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
    
    // root
    osg::Group* rootNode = new osg::Group;

    // create info display
    const char* text[] = {
        "osg::Sequence Mini-Howto",
        "- can be used for simple flip-book-style animation",
        "- is subclassed from osg::Switch",
        "- assigns a display duration to each child",
        "- can loop or swing through an interval of it's children",
        "- can repeat the interval a number of times or indefinitively",
        "- press 'Shift-S' to start/pause/resume",
        "- press 'Shift-L' to toggle loop/swing mode",
        NULL
    };
    rootNode->addChild(createHUD(createTextGroup(text)));

    // add sequence of models from command line
    osg::Sequence* seq = createSequence(arguments);
    rootNode->addChild(seq);

    // add model to viewer.
    viewer.setSceneData(rootNode);

    // add event handler to control sequence
    viewer.getEventHandlerList().push_front(new SequenceEventHandler(seq));

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
