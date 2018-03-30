/* OpenSceneGraph example, osgclip.
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

#include <osg/MatrixTransform>
#include <osg/ClipNode>
#include <osg/Billboard>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>
#include <osg/Material>
#include <osg/PolygonOffset>
#include <osg/PolygonMode>
#include <osg/LineStipple>
#include <osg/AnimationPath>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgViewer/Viewer>

#include <osgUtil/Optimizer>
#include <osgText/Text>


osg::ref_ptr<osg::Node> decorate_with_clip_node(const osg::ref_ptr<osg::Node>& subgraph)
{
    osg::ref_ptr<osg::Group> rootnode = new osg::Group;

    // create wireframe view of the model so the user can see
    // what parts are being culled away.
    osg::StateSet* stateset = new osg::StateSet;
    //osg::Material* material = new osg::Material;
    osg::PolygonMode* polymode = new osg::PolygonMode;
    polymode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
    stateset->setAttributeAndModes(polymode,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

    osg::Group* wireframe_subgraph = new osg::Group;
    wireframe_subgraph->setStateSet(stateset);
    wireframe_subgraph->addChild(subgraph);
    rootnode->addChild(wireframe_subgraph);

    // more complex approach to managing ClipNode, allowing
    // ClipNode node to be transformed independently from the subgraph
    // that it is clipping.

    osg::MatrixTransform* transform= new osg::MatrixTransform;

    osg::NodeCallback* nc = new osg::AnimationPathCallback(subgraph->getBound().center(),osg::Vec3(0.0f,0.0f,1.0f),osg::inDegrees(45.0f));
    transform->setUpdateCallback(nc);

    osg::ClipNode* clipnode = new osg::ClipNode;
    osg::BoundingSphere bs = subgraph->getBound();
    bs.radius()*= 0.4f;

    osg::BoundingBox bb;
    bb.expandBy(bs);

    clipnode->createClipBox(bb);
    clipnode->setCullingActive(false);

    transform->addChild(clipnode);
    rootnode->addChild(transform);


    // create clipped part.
    osg::Group* clipped_subgraph = new osg::Group;

    clipped_subgraph->setStateSet(clipnode->getStateSet());
    clipped_subgraph->addChild(subgraph);
    rootnode->addChild(clipped_subgraph);

    return rootnode;
}


osg::ref_ptr<osg::Node> simple_decorate_with_clip_node(const osg::ref_ptr<osg::Node>& subgraph)
{
    osg::ref_ptr<osg::Group> rootnode = new osg::Group;


    // more complex approach to managing ClipNode, allowing
    // ClipNode node to be transformed independently from the subgraph
    // that it is clipping.

    osg::MatrixTransform* transform= new osg::MatrixTransform;

    osg::NodeCallback* nc = new osg::AnimationPathCallback(subgraph->getBound().center(),osg::Vec3(0.0f,0.0f,1.0f),osg::inDegrees(45.0f));
    transform->setUpdateCallback(nc);

    osg::ClipNode* clipnode = new osg::ClipNode;
    osg::BoundingSphere bs = subgraph->getBound();
    bs.radius()*= 0.4f;

    osg::BoundingBox bb;
    bb.expandBy(bs);

    clipnode->createClipBox(bb);
    clipnode->setCullingActive(false);

    transform->addChild(clipnode);
    rootnode->addChild(transform);


    // create clipped part.
    osg::Group* clipped_subgraph = new osg::Group;

    clipped_subgraph->setStateSet(clipnode->getStateSet());
    clipped_subgraph->addChild(subgraph);
    rootnode->addChild(clipped_subgraph);

    return rootnode;
}

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    osgViewer::Viewer viewer(arguments);

    // load the nodes from the commandline arguments.
    osg::ref_ptr<osg::Group> scene = new osg::Group;

    std::string textString;
    while(arguments.read("--text", textString))
    {
        osg::ref_ptr<osgText::Text> text = new osgText::Text;
        text->setFont("fonts/times.ttf");
        text->setAxisAlignment(osgText::Text::XZ_PLANE);
        text->setDrawMode(osgText::Text::TEXT|osgText::Text::ALIGNMENT|osgText::Text::BOUNDINGBOX|osgText::Text::FILLEDBOUNDINGBOX);
        text->setText(textString);
        scene->addChild(text);
    }


    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles(arguments);
    if (loadedModel.valid())
    {
        scene->addChild(scene.get());
    }


    // if not loaded assume no arguments passed in, try use default mode instead.
    if (scene->getNumChildren()==0)
    {
        loadedModel = osgDB::readRefNodeFile("cow.osgt");
        scene->addChild(loadedModel.get());
    }


    if (scene->getNumChildren()==0)
    {
        osg::notify(osg::NOTICE)<<"Please specify a filename on the command line"<<std::endl;
        return 1;
    }

    // decorate the scenegraph with a clip node.
    osg::ref_ptr<osg::Node> rootnode;


    if (arguments.read("--simple"))
    {
        rootnode = simple_decorate_with_clip_node(scene);
    }
    else
    {
        rootnode = decorate_with_clip_node(scene);
    }

    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);

    // set the scene to render
    viewer.setSceneData(rootnode);

    return viewer.run();
}
