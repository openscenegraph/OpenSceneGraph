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

#include <osgManipulator/TranslatePlaneDragger>

#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/Quat>
#include <osg/PolygonMode>
#include <osg/CullFace>
#include <osg/AutoTransform>

using namespace osgManipulator;

TranslatePlaneDragger::TranslatePlaneDragger() : _usingTranslate1DDragger(false)
{
    _translate2DDragger = new Translate2DDragger();
    _translate2DDragger->setColor(osg::Vec4(0.7f, 0.7f, 0.7f, 1.0f));
    addChild(_translate2DDragger.get());
    addDragger(_translate2DDragger.get());

    _translate1DDragger = new Translate1DDragger(osg::Vec3(0.0f,0.0f,0.0f),osg::Vec3(0.0f,1.0f,0.0f));
    _translate1DDragger->setCheckForNodeInNodePath(false);
    addChild(_translate1DDragger.get());
    addDragger(_translate1DDragger.get());

    setParentDragger(getParentDragger());
}

TranslatePlaneDragger::~TranslatePlaneDragger()
{
}

bool TranslatePlaneDragger::handle(const PointerInfo& pointer, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    // Check if the dragger node is in the nodepath.
    if (!pointer.contains(this)) return false;

    if ((ea.getButtonMask() & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON) &&
        ea.getEventType() == osgGA::GUIEventAdapter::PUSH)
        _usingTranslate1DDragger = true;

    bool handled = false;
    if (_usingTranslate1DDragger)
    {
        if (_translate1DDragger->handle(pointer, ea, aa))
            handled = true;
    }
    else
    {
        if (_translate2DDragger->handle(pointer, ea, aa))
            handled = true;
    }

    if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE)
        _usingTranslate1DDragger = false;

    return handled;
}

void TranslatePlaneDragger::setupDefaultGeometry()
{
    // Create a polygon.
    {
        osg::Geode* geode = new osg::Geode;
        osg::Geometry* geometry = new osg::Geometry();

        osg::Vec3Array* vertices = new osg::Vec3Array(4);
        (*vertices)[0] = osg::Vec3(-0.5,0.0,0.5);
        (*vertices)[1] = osg::Vec3(-0.5,0.0,-0.5);
        (*vertices)[2] = osg::Vec3(0.5,0.0,-0.5);
        (*vertices)[3] = osg::Vec3(0.5,0.0,0.5);

        geometry->setVertexArray(vertices);
        geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,vertices->size()));

        osg::Vec3Array* normals = new osg::Vec3Array;
        normals->push_back(osg::Vec3(0.0,1.0,0.0));
        geometry->setNormalArray(normals, osg::Array::BIND_OVERALL);

        geode->addDrawable(geometry);

        osg::PolygonMode* polymode = new osg::PolygonMode;
        polymode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
        geode->getOrCreateStateSet()->setAttributeAndModes(polymode,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

        geode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

        _translate2DDragger->addChild(geode);
    }
}
