/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgProducer/Viewer>

#include <osg/Material>
#include <osg/Geode>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Projection>
#include <osg/PolygonOffset>
#include <osg/MatrixTransform>
#include <osg/CameraNode>
#include <osg/FrontFace>

#include <osgText/Text>


osg::Node* createRearView(osg::Node* subgraph, const osg::Vec4& clearColour)
{
    osg::CameraNode* camera = new osg::CameraNode;

    // set the viewport
    camera->setViewport(420,800,400,200);

    // set the view matrix
    camera->setCullingActive(false);    
    camera->setReferenceFrame(osg::Transform::RELATIVE_RF);
    camera->setTransformOrder(osg::CameraNode::POST_MULTIPLE);

    camera->setProjectionMatrix(osg::Matrixd::scale(-1.0f,1.0f,1.0f));
    camera->setViewMatrix(osg::Matrixd::rotate(osg::inDegrees(180.0f),0.0f,1.0f,0.0f));

    // set clear the color and depth buffer
    camera->setClearColor(clearColour);
    camera->setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    // draw subgraph after main camera view.
    camera->setRenderOrder(osg::CameraNode::POST_RENDER);

    // add the subgraph to draw.
    camera->addChild(subgraph);
    
    // switch of back face culling as we've swapped over the projection matrix making back faces become front faces.
#if 1
    camera->getOrCreateStateSet()->setAttribute(new osg::FrontFace(osg::FrontFace::CLOCKWISE));
#else
    camera->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
#endif
    
    return camera;
}

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates how to do Head Up Displays.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] [filename] ...");
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
    
    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles(arguments);

    osg::ref_ptr<osg::Group> group  = new osg::Group;
    
    // add the HUD subgraph.    
    if (scene.valid()) group->addChild(scene.get());
    group->addChild(createRearView(scene.get(), viewer.getClearColor()));

    // set the scene to render
    viewer.setSceneData(group.get());

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
