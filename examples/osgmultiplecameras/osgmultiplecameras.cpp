/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
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
#include <osgViewer/Viewer>

#include <osg/Material>
#include <osg/Geode>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Projection>
#include <osg/PolygonOffset>
#include <osg/MatrixTransform>
#include <osg/Camera>
#include <osg/FrontFace>

#include <osgText/Text>


osg::Node* createRearView(osg::Node* subgraph, const osg::Vec4& clearColour)
{
    osg::Camera* camera = new osg::Camera;

    // set the viewport
    camera->setViewport(10,10,400,200);

    // set the view matrix
    camera->setCullingActive(false);    
    camera->setReferenceFrame(osg::Transform::RELATIVE_RF);
    camera->setTransformOrder(osg::Camera::POST_MULTIPLY);

    camera->setProjectionMatrix(osg::Matrixd::scale(-1.0f,1.0f,1.0f));
    camera->setViewMatrix(osg::Matrixd::rotate(osg::inDegrees(180.0f),0.0f,1.0f,0.0f));

    // set clear the color and depth buffer
    camera->setClearColor(clearColour);
    camera->setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    // draw subgraph after main camera view.
    camera->setRenderOrder(osg::Camera::POST_RENDER);

    // add the subgraph to draw.
    camera->addChild(subgraph);
    
    // switch of back face culling as we've swapped over the projection matrix making back faces become front faces.
    camera->getOrCreateStateSet()->setAttribute(new osg::FrontFace(osg::FrontFace::CLOCKWISE));
    
    return camera;
}

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // construct the viewer.
    osgViewer::Viewer viewer;
    
    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles(arguments);

    osg::ref_ptr<osg::Group> group  = new osg::Group;
    
    // add the HUD subgraph.    
    if (scene.valid()) group->addChild(scene.get());

    osg::Vec4 colour =  viewer.getCamera()->getClearColor();
    colour.r() *= 0.5f;
    colour.g() *= 0.5f;
    colour.b() *= 0.5f;

    // note tone down the normal back ground colour to make it obvious that there is a seperate camera inserted.
    group->addChild(createRearView(scene.get(), colour));

    // set the scene to render
    viewer.setSceneData(group.get());

    return viewer.run();
}
