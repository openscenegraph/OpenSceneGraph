/* OpenSceneGraph example, osglogicop.
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
#include <osg/LogicOp>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>

#include <osgUtil/Optimizer>

#include <iostream>

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

    // load the nodes from the commandline arguments.
    osg::Node* loadedModel = osgDB::readNodeFiles(arguments);
    
    // if not loaded assume no arguments passed in, try use default mode instead.
    if (!loadedModel) loadedModel = osgDB::readNodeFile("glider.osg");
    
    if (!loadedModel)
    {
        osg::notify(osg::NOTICE)<<"Please specify model filename on the command line."<<std::endl;
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

    // construct the viewer.
    osgViewer::Viewer viewer;

    viewer.addEventHandler(new TechniqueEventHandler(logicOp));
    
    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(root);
     
    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData( root );
    
    return viewer.run();
}
