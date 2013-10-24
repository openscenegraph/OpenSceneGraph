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

#include <osgManipulator/TrackballDragger>
#include <osgManipulator/AntiSquish>

#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/PolygonMode>
#include <osg/CullFace>
#include <osg/Quat>
#include <osg/AutoTransform>

using namespace osgManipulator;

namespace
{

    osg::Geometry* createCircleGeometry(float radius, unsigned int numSegments)
    {
        const float angleDelta = 2.0f*osg::PI/(float)numSegments;
        const float r = radius;
        float angle = 0.0f;
        osg::Vec3Array* vertexArray = new osg::Vec3Array(numSegments);
        osg::Vec3Array* normalArray = new osg::Vec3Array(numSegments);
        for(unsigned int i = 0; i < numSegments; ++i,angle+=angleDelta)
        {
            float c = cosf(angle);
            float s = sinf(angle);
            (*vertexArray)[i].set(c*r,s*r,0.0f);
            (*normalArray)[i].set(c,s,0.0f);
        }
        osg::Geometry* geometry = new osg::Geometry();
        geometry->setVertexArray(vertexArray);
        geometry->setNormalArray(normalArray, osg::Array::BIND_PER_VERTEX);
        geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP,0,vertexArray->size()));
        return geometry;
    }

}

TrackballDragger::TrackballDragger(bool useAutoTransform)
{
    if (useAutoTransform)
    {
        float pixelSize = 50.0f;
        osg::MatrixTransform* scaler = new osg::MatrixTransform;
        scaler->setMatrix(osg::Matrix::scale(pixelSize, pixelSize, pixelSize));

        osg::AutoTransform *at = new osg::AutoTransform;
        at->setAutoScaleToScreen(true);
        at->addChild(scaler);

        AntiSquish* as = new AntiSquish;
        as->addChild(at);
        addChild(as);

        _xDragger = new RotateCylinderDragger();
        scaler->addChild(_xDragger.get());
        addDragger(_xDragger.get());

        _yDragger = new RotateCylinderDragger();
        scaler->addChild(_yDragger.get());
        addDragger(_yDragger.get());

        _zDragger = new RotateCylinderDragger();
        scaler->addChild(_zDragger.get());
        addDragger(_zDragger.get());

        _xyzDragger = new RotateSphereDragger();
        scaler->addChild(_xyzDragger.get());
        addDragger(_xyzDragger.get());
    }
    else
    {
        _xDragger = new RotateCylinderDragger();
        addChild(_xDragger.get());
        addDragger(_xDragger.get());

        _yDragger = new RotateCylinderDragger();
        addChild(_yDragger.get());
        addDragger(_yDragger.get());

        _zDragger = new RotateCylinderDragger();
        addChild(_zDragger.get());
        addDragger(_zDragger.get());

        _xyzDragger = new RotateSphereDragger();
        addChild(_xyzDragger.get());
        addDragger(_xyzDragger.get());
    }

    _axisLineWidth = 2.0f;
    _pickCylinderHeight = 0.15f;

    setParentDragger(getParentDragger());
}

TrackballDragger::~TrackballDragger()
{
}

void TrackballDragger::setupDefaultGeometry()
{
    _geode = new osg::Geode;
    {
        osg::TessellationHints* hints = new osg::TessellationHints;
        hints->setCreateTop(false);
        hints->setCreateBottom(false);
        hints->setCreateBackFace(false);

        _cylinder = new osg::Cylinder;
        _cylinder->setHeight(_pickCylinderHeight);
        osg::ShapeDrawable* cylinderDrawable = new osg::ShapeDrawable(_cylinder.get(), hints);
        _geode->addDrawable(cylinderDrawable);
        setDrawableToAlwaysCull(*cylinderDrawable);
        _geode->addDrawable(createCircleGeometry(1.0f, 100));
    }

    // Draw in line mode.
    {
        osg::PolygonMode* polymode = new osg::PolygonMode;
        polymode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
        _geode->getOrCreateStateSet()->setAttributeAndModes(polymode,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
        _lineWidth = new osg::LineWidth(_axisLineWidth);
        _geode->getOrCreateStateSet()->setAttributeAndModes(_lineWidth.get(), osg::StateAttribute::ON);

#if !defined(OSG_GLES2_AVAILABLE)
        _geode->getOrCreateStateSet()->setMode(GL_NORMALIZE, osg::StateAttribute::ON);
#endif

    }

    // Add line to all the individual 1D draggers.
    _xDragger->addChild(_geode.get());
    _yDragger->addChild(_geode.get());
    _zDragger->addChild(_geode.get());


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

    // Add invisible sphere for pick the spherical dragger.
    {
        osg::Drawable* sphereDrawable = new osg::ShapeDrawable(new osg::Sphere());
        setDrawableToAlwaysCull(*sphereDrawable);
        osg::Geode* sphereGeode = new osg::Geode;
        sphereGeode->addDrawable(sphereDrawable);

        _xyzDragger->addChild(sphereGeode);
    }
}

void TrackballDragger::setAxisLineWidth(float linePixelWidth)
{
    _axisLineWidth = linePixelWidth;
    if (_lineWidth.valid())
        _lineWidth->setWidth(linePixelWidth);
}
void TrackballDragger::setPickCylinderHeight(float pickCylinderHeight)
{
    _pickCylinderHeight = pickCylinderHeight;
    if (_cylinder.valid())
        _cylinder->setHeight(pickCylinderHeight);
}
