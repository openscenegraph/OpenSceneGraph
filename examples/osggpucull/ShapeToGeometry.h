/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2014 Robert Osfield
 *  Copyright (C) 2014 Pawel Ksiezopolski
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
*/
#ifndef SHAPE_TO_GEOMETRY
#define SHAPE_TO_GEOMETRY 1
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/NodeVisitor>

// arbitrary minima for rows & segments ( from shapedrawable.cpp )
const unsigned int MIN_NUM_ROWS = 3;
const unsigned int MIN_NUM_SEGMENTS = 5;

// osg::GLBeginEndAdapter descendant that stores data for osg::Geometry creation
class FakeGLBeginEndAdapter : public osg::GLBeginEndAdapter
{
public:
    FakeGLBeginEndAdapter();

    void PushMatrix();
    void MultMatrixd(const GLdouble* m);
    void Translated(GLdouble x, GLdouble y, GLdouble z);
    void Scaled(GLdouble x, GLdouble y, GLdouble z);
    void Rotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
    void End();

   osg::ref_ptr<osg::Geometry> geometry;
};

class ShapeToGeometryVisitor : public osg::ConstShapeVisitor
{
public:

    ShapeToGeometryVisitor(const osg::TessellationHints* hints)
        : osg::ConstShapeVisitor(), _hints(hints)
    {
    }

    virtual void apply(const osg::Sphere&);
    virtual void apply(const osg::Box&);
    virtual void apply(const osg::Cone&);
    virtual void apply(const osg::Cylinder&);
    virtual void apply(const osg::Capsule&);
    virtual void apply(const osg::InfinitePlane&);

    virtual void apply(const osg::TriangleMesh&);
    virtual void apply(const osg::ConvexHull&);
    virtual void apply(const osg::HeightField&);

    virtual void apply(const osg::CompositeShape&);

    osg::Geometry* getGeometry() { return gl.geometry.get(); }


    const osg::TessellationHints* _hints;
    FakeGLBeginEndAdapter gl;
protected:

    ShapeToGeometryVisitor& operator = (const ShapeToGeometryVisitor&) { return *this; }
    enum SphereHalf { SphereTopHalf, SphereBottomHalf };

    // helpers for apply( Cylinder | Sphere | Capsule )
    void drawCylinderBody(unsigned int numSegments, float radius, float height);
    void drawHalfSphere(unsigned int numSegments, unsigned int numRows, float radius, SphereHalf which, float zOffset = 0.0f);
};

osg::Geometry* convertShapeToGeometry(const osg::Shape& shape, const osg::TessellationHints* hints);

osg::Geometry* convertShapeToGeometry(const osg::Shape& shape, const osg::TessellationHints* hints, const osg::Vec4& color);

osg::Geode* convertShapeToGeode(const osg::Shape& shape, const osg::TessellationHints* hints);

osg::Geode* convertShapeToGeode(const osg::Shape& shape, const osg::TessellationHints* hints, const osg::Vec4& color);


// example : how to use convertShapeToGeometry()
//     osg::ref_ptr<osg::Capsule> shape = new osg::Capsule( osg::Vec3( 0.0, 0.0, 0.0 ), radius, height );
//     osg::ref_ptr<osg::TessellationHints> tessHints = new osg::TessellationHints;
//     tessHints->setDetailRatio(0.5f);
//     tessHints->setCreateTextureCoords(true);
//     osg::ref_ptr<osg::Geometry> capsuleGeometry = convertShapeToGeometry(*shape.get(), tessHints.get());
//     osg::ref_ptr<osg::Geometry> redCapsuleGeometry = convertShapeToGeometry(*shape.get(), tessHints.get(), osg::Vec4(1.0,0.0,0.0,1.0) );


#endif
