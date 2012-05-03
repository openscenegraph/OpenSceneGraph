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

#include <osgManipulator/ScaleAxisDragger>

#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/Quat>

using namespace osgManipulator;

ScaleAxisDragger::ScaleAxisDragger()
{
    _xDragger = new osgManipulator::Scale1DDragger();
    addChild(_xDragger.get());
    addDragger(_xDragger.get());

    _yDragger = new osgManipulator::Scale1DDragger();
    addChild(_yDragger.get());
    addDragger(_yDragger.get());

    _zDragger = new osgManipulator::Scale1DDragger();
    addChild(_zDragger.get());
    addDragger(_zDragger.get());

    setParentDragger(getParentDragger());
}

ScaleAxisDragger::~ScaleAxisDragger()
{
}

void ScaleAxisDragger::setupDefaultGeometry()
{
    // Create a line.
    osg::Geode* lineGeode = new osg::Geode;
    {
        osg::Geometry* geometry = new osg::Geometry();

        osg::Vec3Array* vertices = new osg::Vec3Array(2);
        (*vertices)[0] = osg::Vec3(0.0f,0.0f,0.0f);
        (*vertices)[1] = osg::Vec3(1.0f,0.0f,0.0f);

        geometry->setVertexArray(vertices);
        geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,2));

        lineGeode->addDrawable(geometry);
    }

    // Turn of lighting for line and set line width.
    {
        osg::LineWidth* linewidth = new osg::LineWidth();
        linewidth->setWidth(2.0f);
        lineGeode->getOrCreateStateSet()->setAttributeAndModes(linewidth, osg::StateAttribute::ON);
        lineGeode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    }

    // Add line to all the individual 1D draggers.
    _xDragger->addChild(lineGeode);
    _yDragger->addChild(lineGeode);
    _zDragger->addChild(lineGeode);

    osg::Geode* geode = new osg::Geode;

    // Create a box.
    geode->addDrawable(new osg::ShapeDrawable(new osg::Box(osg::Vec3(1.0f,0.0f,0.0f), 0.05)));

    // Add geode to all 1D draggers.
    _xDragger->addChild(geode);
    _yDragger->addChild(geode);
    _zDragger->addChild(geode);

    // Rotate Z-axis dragger appropriately.
    {
        osg::Quat rotation; rotation.makeRotate(osg::Vec3(1.0f, 0.0f, 0.0f), osg::Vec3(0.0f, 0.0f, 1.0f));
        _zDragger->setMatrix(osg::Matrix(rotation));
    }

    // Rotate Y-axis dragger appropriately.
    {
        osg::Quat rotation; rotation.makeRotate(osg::Vec3(1.0f, 0.0f, 0.0f), osg::Vec3(0.0f, 1.0f, 0.0f));
        _yDragger->setMatrix(osg::Matrix(rotation));
    }

    // Send different colors for each dragger.
    _xDragger->setColor(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
    _yDragger->setColor(osg::Vec4(0.0f,1.0f,0.0f,1.0f));
    _zDragger->setColor(osg::Vec4(0.0f,0.0f,1.0f,1.0f));
}
