// -*-c++-*-

#include <osg/Group>
#include <osg/Sequence>
#include <osg/MatrixTransform>

#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>

#include <osgGLUT/Viewer>
#include <osgGLUT/glut>


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

        if (ea.getEventType() == osgGA::GUIEventAdapter::KEYBOARD)
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

void write_usage(std::ostream& out, const std::string& name)
{
    out << std::endl;
    out <<"usage:"<< std::endl;
    out <<"    "<<name<<" [options] infile1 [infile2 ...]"<< std::endl;
    out << std::endl;
    out <<"options:"<< std::endl;
    out <<"    -l libraryName   - load plugin of name libraryName"<< std::endl;
    out <<"                       i.e. -l osgdb_pfb"<< std::endl;
    out <<"                       Useful for loading reader/writers which can load"<< std::endl;
    out <<"                       other file formats in addition to its extension."<< std::endl;
    out <<"    -e extensionName - load reader/wrter plugin for file extension"<< std::endl;
    out <<"                       i.e. -e pfb"<< std::endl;
    out <<"                       Useful short hand for specifying full library name as"<< std::endl;
    out <<"                       done with -l above, as it automatically expands to"<< std::endl;
    out <<"                       the full library name appropriate for each platform."<< std::endl;
    out<<std::endl;
}

osg::Sequence* generateSeq(osg::Sequence::LoopMode mode,
                           float speed, int nreps,
                           std::vector<osg::Node*>& model)
{
    osg::Sequence* seqNode = osgNew osg::Sequence;

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
    // initialize the GLUT
    glutInit( &argc, argv );

    if (argc < 2)
    {
        write_usage(osg::notify(osg::NOTICE), argv[0]);
        return 0;
    }

    // create commandline args
    std::vector<std::string> commandLine;
    for (int ia = 1; ia < argc; ia++)
        commandLine.push_back(argv[ia]);

    // initialize the viewer
    osgGLUT::Viewer viewer;
    viewer.setWindowTitle(argv[0]);

    // configure the viewer from the commandline arguments, and eat any
    // parameters that have been matched.
    viewer.readCommandLine(commandLine);
    
    // assumes any remaining parameters are models
    std::vector<osg::Node*> model;
    unsigned int i;
    for (i = 0; i < commandLine.size(); i++) {
        std::cerr << "Loading " << commandLine[i] << std::endl;
        osg::Node* node = osgDB::readNodeFile(commandLine[i]);
        if (node)
            model.push_back(node);
    }
    if (model.empty()) {
        write_usage(osg::notify(osg::NOTICE),argv[0]);
        return -1;
    }

    // root
    osg::Group* rootNode = osgNew osg::Group;

    // create sequences
    std::vector<osg::Sequence*> seq;

    const osg::Sequence::LoopMode mode[] = { osg::Sequence::LOOP,
                                             osg::Sequence::SWING,
                                             osg::Sequence::LOOP };
    const float speed[] = { 0.5f, 1.0f, 1.5f };
    const int nreps[] = { -1, 5, 1 };

    float x = 0.0f;
    for (i = 0; i < (sizeof(speed) / sizeof(speed[0])); i++) {
        osg::Sequence* seqNode = generateSeq(mode[i], speed[i], nreps[i],
                                             model);
        if (!seqNode)
            continue;
        seq.push_back(seqNode);

        // position sequence
        osg::Matrix matrix;
        matrix.makeTranslate(x, 0.0, 0.0);

        osg::MatrixTransform* xform = osgNew osg::MatrixTransform;
        xform->setMatrix(matrix);
        xform->addChild(seqNode);

        rootNode->addChild(xform);

        x += seqNode->getBound()._radius * 1.5f;
    }

    // add model to viewer.
    viewer.addViewport(rootNode);

    // register additional event handler
    viewer.prependEventHandler(osgNew MyEventHandler(&seq), 0);

    // register trackball, flight and drive.
    viewer.registerCameraManipulator(new osgGA::TrackballManipulator);

    viewer.open();
    viewer.run();
    
    return 0;
}
