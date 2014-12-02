/* OpenSceneGraph example, osgblendequation.
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

#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>
#include <osg/BlendEquation>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>

#include <osgViewer/Viewer>

#include <iostream>

const int _eq_nb=8;
const osg::BlendEquation::Equation _equations[_eq_nb]=
{
    osg::BlendEquation::FUNC_ADD,
    osg::BlendEquation::FUNC_SUBTRACT,
    osg::BlendEquation::FUNC_REVERSE_SUBTRACT,
    osg::BlendEquation::RGBA_MIN,
    osg::BlendEquation::RGBA_MAX,
    osg::BlendEquation::ALPHA_MIN,
    osg::BlendEquation::ALPHA_MAX,
    osg::BlendEquation::LOGIC_OP
};

const char* _equations_name[_eq_nb]=
{
    "osg::BlendEquation::FUNC_ADD",
    "osg::BlendEquation::FUNC_SUBTRACT",
    "osg::BlendEquation::FUNC_REVERSE_SUBTRACT",
    "osg::BlendEquation::RGBA_MIN",
    "osg::BlendEquation::RGBA_MAX",
    "osg::BlendEquation::ALPHA_MIN",
    "osg::BlendEquation::ALPHA_MAX",
    "osg::BlendEquation::LOGIC_OP"
};


class TechniqueEventHandler : public osgGA::GUIEventHandler
{
public:

    TechniqueEventHandler(osg::BlendEquation* blendEq) { _blendEq=blendEq; _eq_index=0;}
    TechniqueEventHandler() { std::cerr<<"Error, can't initialize it!";}

    META_Object(osgBlendEquationApp,TechniqueEventHandler);

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);

    virtual void getUsage(osg::ApplicationUsage& usage) const;

protected:

    ~TechniqueEventHandler() {}

    TechniqueEventHandler(const TechniqueEventHandler&,const osg::CopyOp&) {}

    osg::BlendEquation*    _blendEq;

    int         _eq_index;
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
                _eq_index++;
                if (_eq_index>=_eq_nb) _eq_index=0;
                _blendEq->setEquation(_equations[_eq_index]);
                std::cout<<"Equation name = "<<_equations_name[_eq_index]<<std::endl;
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Left ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Left)
            {
                _eq_index--;
                if (_eq_index<0) _eq_index=_eq_nb-1;
                _blendEq->setEquation(_equations[_eq_index]);
                std::cout<<"Equation name = "<<_equations_name[_eq_index]<<std::endl;
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
    usage.addKeyboardMouseBinding("Left Arrow","Advance to next equation");
    usage.addKeyboardMouseBinding("Right Array","Move to previous equation");
}




int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates how to use glBlendEquation for mixing rendered scene and the frame-buffer.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");

    // construct the viewer.
    osgViewer::Viewer viewer;

    // load the nodes from the commandline arguments.
    osg::Node* loadedModel = osgDB::readNodeFiles(arguments);

    // if not loaded assume no arguments passed in, try use default mode instead.
    if (!loadedModel) loadedModel = osgDB::readNodeFile("cessnafire.osgt");

    if (!loadedModel)
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    osg::Group* root = new osg::Group;
    root->addChild(loadedModel);


    osg::StateSet* stateset = new osg::StateSet;
    stateset->setDataVariance(osg::Object::DYNAMIC);

    osg::BlendEquation* blendEquation = new osg::BlendEquation(osg::BlendEquation::FUNC_ADD);
    blendEquation->setDataVariance(osg::Object::DYNAMIC);

    stateset->setAttributeAndModes(blendEquation,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

    //tell to sort the mesh before displaying it
    stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    loadedModel->setStateSet(stateset);

    viewer.addEventHandler(new TechniqueEventHandler(blendEquation));

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData( root );

    return viewer.run();
}
