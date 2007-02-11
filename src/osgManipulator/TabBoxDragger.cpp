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
//osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/TabBoxDragger>

#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/Quat>

using namespace osgManipulator;

TabBoxDragger::TabBoxDragger()
{
    for (int i=0; i<6; ++i)
    {
        _planeDraggers.push_back(new TabPlaneDragger());
        addChild(_planeDraggers[i].get());
        addDragger(_planeDraggers[i].get());
    }

    {
        _planeDraggers[0]->setMatrix(osg::Matrix::translate(osg::Vec3(0.0,0.5,0.0)));
    }
    {
        osg::Quat rotation; rotation.makeRotate(osg::Vec3(0.0f, -1.0f, 0.0f), osg::Vec3(0.0f, 1.0f, 0.0f));
        _planeDraggers[1]->setMatrix(osg::Matrix(rotation)*osg::Matrix::translate(osg::Vec3(0.0,-0.5,0.0)));
    }
    {
        osg::Quat rotation; rotation.makeRotate(osg::Vec3(0.0f, 0.0f, 1.0f), osg::Vec3(0.0f, 1.0f, 0.0f));
        _planeDraggers[2]->setMatrix(osg::Matrix(rotation)*osg::Matrix::translate(osg::Vec3(0.0,0.0,-0.5)));
    }

    {
        osg::Quat rotation; rotation.makeRotate(osg::Vec3(0.0f, 1.0f, 0.0f), osg::Vec3(0.0f, 0.0f, 1.0f));
        _planeDraggers[3]->setMatrix(osg::Matrix(rotation)*osg::Matrix::translate(osg::Vec3(0.0,0.0,0.5)));
    }

    {
        osg::Quat rotation; rotation.makeRotate(osg::Vec3(1.0f, 0.0f, 0.0f), osg::Vec3(0.0f, 1.0f, 0.0f));
        _planeDraggers[4]->setMatrix(osg::Matrix(rotation)*osg::Matrix::translate(osg::Vec3(-0.5,0.0,0.0)));
    }

    {
        osg::Quat rotation; rotation.makeRotate(osg::Vec3(0.0f, 1.0f, 0.0f), osg::Vec3(1.0f, 0.0f, 0.0f));
        _planeDraggers[5]->setMatrix(osg::Matrix(rotation)*osg::Matrix::translate(osg::Vec3(0.5,0.0,0.0)));
    }

    setParentDragger(getParentDragger());
}
       
TabBoxDragger::~TabBoxDragger()
{
}

void TabBoxDragger::setupDefaultGeometry()
{
    for (unsigned int i=0; i<_planeDraggers.size(); ++i)
        _planeDraggers[i]->setupDefaultGeometry(false);
}

void TabBoxDragger::setPlaneColor(const osg::Vec4& color)
{
    for (unsigned int i=0; i<_planeDraggers.size(); ++i)
        _planeDraggers[i]->setPlaneColor(color);
}
