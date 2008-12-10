/*  -*-c++-*- 
 *  Copyright (C) 2008 Cedric Pinson <mornifle@plopbyte.net>
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
 *
 * Authors:
 * Cedric Pinson <mornifle@plopbyte.net>
 * jeremy Moles <jeremy@emperorlinux.com>
*/

#include "AnimtkViewerKeyHandler"
#include "AnimtkViewerGUI"

#include <iostream>
#include <osg/io_utils>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgWidget/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgDB/ReadFile>
#include <osgAnimation/AnimationManagerBase>

const int WIDTH  = 1440;
const int HEIGHT = 900;


osg::Geode* createAxis() 
{
    osg::Geode*     geode    = new osg::Geode();  
    osg::Geometry*  geometry = new osg::Geometry();
    osg::Vec3Array* vertices = new osg::Vec3Array();
    osg::Vec4Array* colors   = new osg::Vec4Array();
	
    vertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    vertices->push_back(osg::Vec3(1.0f, 0.0f, 0.0f));
    vertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    vertices->push_back(osg::Vec3(0.0f, 1.0f, 0.0f));
    vertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    vertices->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));

    colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
    colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
    colors->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
    colors->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
    colors->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f));
    colors->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f));
	
    geometry->setVertexArray(vertices);
    geometry->setColorArray(colors);
    geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);    
    geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, 6));
    geometry->getOrCreateStateSet()->setMode(GL_LIGHTING, false);

    geode->addDrawable(geometry);

    return geode;
}

int main(int argc, char** argv) 
{
    osg::ArgumentParser psr(&argc, argv);

    if(argc < 2) 
    {
        std::cerr << "usage: AnimtkViewer <file.osg>" << std::endl;
        return 1;
    }

    osgViewer::Viewer viewer(psr);
    osg::ref_ptr<osg::Group> group = new osg::Group();

    osg::Group* node = dynamic_cast<osg::Group*>(osgDB::readNodeFile(psr[1])); //dynamic_cast<osgAnimation::AnimationManager*>(osgDB::readNodeFile(psr[1]));
    if(!node) 
    {
        std::cerr << "Can't read file " << psr[1]<< std::endl;
        return 1;
    }

    // Set our Singleton's model.
    osgAnimation::AnimationManagerBase* base = dynamic_cast<osgAnimation::AnimationManagerBase*>(node->getUpdateCallback());
    osgAnimation::BasicAnimationManager* manager = new osgAnimation::BasicAnimationManager(*base);
    node->setUpdateCallback(manager);
    AnimtkViewerModelController::setModel(manager);

    node->addChild(createAxis());

    AnimtkViewerGUI* gui    = new AnimtkViewerGUI(&viewer, WIDTH, HEIGHT, 0x1234);
    osg::Camera*     camera = gui->createParentOrthoCamera();
	
    node->setNodeMask(0x0001);

    group->addChild(node);
    group->addChild(camera);

    viewer.addEventHandler(new AnimtkKeyEventHandler());
    viewer.addEventHandler(new osgViewer::StatsHandler());
    viewer.addEventHandler(new osgViewer::WindowSizeHandler());
    viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));
    viewer.addEventHandler(new osgWidget::MouseHandler(gui));
    viewer.addEventHandler(new osgWidget::KeyboardHandler(gui));
    viewer.addEventHandler(new osgWidget::ResizeHandler(gui, camera));
    viewer.setSceneData(group.get());

    viewer.setUpViewInWindow(40, 40, WIDTH, HEIGHT);

    return viewer.run();
}
