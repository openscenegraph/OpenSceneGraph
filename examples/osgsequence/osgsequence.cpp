/* OpenSceneGraph example, osgsequence.
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

#include <osgText/Text>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Projection>
#include <osg/Sequence>
#include <osg/MatrixTransform>

#include <osgDB/ReadFile>

#include <osgViewer/Viewer>

#include <iostream>

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

    typedef std::vector<std::string> Filenames;
    Filenames filenames;
    
    if (arguments.argc() > 1)
    {
        for (int i = 1; i < arguments.argc(); ++i)
        {
            filenames.push_back(arguments[i]);
        }
    }
    else
    {
        filenames.push_back("cow.osg");
        filenames.push_back("dumptruck.osg");
        filenames.push_back("cessna.osg");
        filenames.push_back("glider.osg");
    }
    
    for(Filenames::iterator itr = filenames.begin();
        itr != filenames.end();
        ++itr)        
    {
        // load model
        osg::Node* node = osgDB::readNodeFile(*itr);

        if (node)
        {
            seq->addChild(createScaledNode(node, 100.0f));
            seq->setTime(seq->getNumChildren()-1, 1.0f);
        }
    }

    // loop through all children
    seq->setInterval(osg::Sequence::LOOP, 0,-1);

    // real-time playback, repeat indefinitively
    seq->setDuration(1.0f, -1);

    seq->setMode(osg::Sequence::START);

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
            case 's':
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
            case 'l':
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

private:
    osg::ref_ptr<osg::Sequence> _seq;
};


int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
   
    // construct the viewer.
    osgViewer::Viewer viewer;
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
        "- press 's' to start/pause/resume",
        "- press 'l' to toggle loop/swing mode",
        NULL
    };
    rootNode->addChild(createHUD(createTextGroup(text)));

    // add sequence of models from command line
    osg::Sequence* seq = createSequence(arguments);
    rootNode->addChild(seq);

    // add model to viewer.
    viewer.setSceneData(rootNode);

    // add event handler to control sequence
    viewer.addEventHandler(new SequenceEventHandler(seq));

    return viewer.run();
}
