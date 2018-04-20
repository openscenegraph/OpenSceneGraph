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

#include <osgManipulator/TranslateAxisDragger>

#include <osg/Quat>
#include <osg/Geode>

using namespace osgManipulator;

TranslateAxisDragger::TranslateAxisDragger()
{
    _xDragger = new Translate1DDragger(osg::Vec3(0.0,0.0,0.0), osg::Vec3(0.0,0.0,1.0));
    addChild(_xDragger.get());
    addDragger(_xDragger.get());

    _yDragger = new Translate1DDragger(osg::Vec3(0.0,0.0,0.0), osg::Vec3(0.0,0.0,1.0));
    addChild(_yDragger.get());
    addDragger(_yDragger.get());

    _zDragger = new Translate1DDragger(osg::Vec3(0.0,0.0,0.0), osg::Vec3(0.0,0.0,1.0));
    addChild(_zDragger.get());
    addDragger(_zDragger.get());

    _axisLineWidth = 2.0f;
    _pickCylinderRadius = 0.015f;
    _coneHeight = 0.1f;

    setParentDragger(getParentDragger());
}

TranslateAxisDragger::~TranslateAxisDragger()
{
}

void TranslateAxisDragger::setupDefaultGeometry()
{
    // Create a line.
    _lineGeode = new osg::Geode;
    {
        osg::Geometry* geometry = new osg::Geometry();

        osg::Vec3Array* vertices = new osg::Vec3Array(2);
        (*vertices)[0] = osg::Vec3(0.0f,0.0f,0.0f);
        (*vertices)[1] = osg::Vec3(0.0f,0.0f,1.0f);

        geometry->setVertexArray(vertices);
        geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,2));

        _lineGeode->addDrawable(geometry);
    }

    // Turn of lighting for line and set line width.
    {
        _lineWidth = new osg::LineWidth();
        _lineWidth->setWidth(_axisLineWidth);
        _lineGeode->getOrCreateStateSet()->setAttributeAndModes(_lineWidth.get(), osg::StateAttribute::ON);
        _lineGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    }

    // Add line to all the individual 1D draggers.
    _xDragger->addChild(_lineGeode.get());
    _yDragger->addChild(_lineGeode.get());
    _zDragger->addChild(_lineGeode.get());

    osg::Geode* geode = new osg::Geode;

    // Create a cone.
    {
        _cone = new osg::Cone (osg::Vec3(0.0f, 0.0f, 1.0f), _coneHeight * 0.25f, _coneHeight);
        osg::ShapeDrawable* coneDrawable = new osg::ShapeDrawable(_cone.get());
        // coneDrawable->setColor(osg::Vec4(0.0f,0.0f,1.0f,1.0f));
        geode->addDrawable(coneDrawable);

        // This ensures correct lighting for scaled draggers.
#if !defined(OSG_GLES2_AVAILABLE)
        geode->getOrCreateStateSet()->setMode(GL_NORMALIZE, osg::StateAttribute::ON);
#endif
    }

    // Create an invisible cylinder for picking the line.
    {
        _cylinder = new osg::Cylinder (osg::Vec3(0.0f,0.0f,0.5f), _pickCylinderRadius, 1.0f);
        osg::Drawable* geometry = new osg::ShapeDrawable(_cylinder.get());
        setDrawableToAlwaysCull(*geometry);
        geode->addDrawable(geometry);
    }

    // Add geode to all 1D draggers.
    _xDragger->addChild(geode);
    _yDragger->addChild(geode);
    _zDragger->addChild(geode);

    // Rotate X-axis dragger appropriately.
    {
        osg::Quat rotation; rotation.makeRotate(osg::Vec3(0.0f, 0.0f, 1.0f), osg::Vec3(1.0f, 0.0f, 0.0f));
        _xDragger->setMatrix(osg::Matrix(rotation));
    }

    // Rotate Y-axis dragger appropriately.
    {
        osg::Quat rotation; rotation.makeRotate(osg::Vec3(0.0f, 0.0f, 1.0f), osg::Vec3(0.0f, 1.0f, 0.0f));
        _yDragger->setMatrix(osg::Matrix(rotation));
    }

    // Send different colors for each dragger.
    _xDragger->setColor(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
    _yDragger->setColor(osg::Vec4(0.0f,1.0f,0.0f,1.0f));
    _zDragger->setColor(osg::Vec4(0.0f,0.0f,1.0f,1.0f));
}

void TranslateAxisDragger::setAxisLineWidth(float linePixelWidth)
{
    _axisLineWidth = linePixelWidth;
    if (_lineWidth.valid())
        _lineWidth->setWidth(linePixelWidth);
}
void TranslateAxisDragger::setPickCylinderRadius(float pickCylinderRadius)
{
    _pickCylinderRadius = pickCylinderRadius;
    if (_cylinder.valid())
        _cylinder->setRadius(pickCylinderRadius);
}

void TranslateAxisDragger::setConeHeight(float height)
{
    _coneHeight = height;
    if (_cone.valid())
    {
        _cone->setRadius(0.25f * height);
        _cone->setHeight(height);
    }
}
