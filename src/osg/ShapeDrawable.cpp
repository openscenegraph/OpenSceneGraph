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
#include <osg/ShapeDrawable>
#include <osg/GL>
#include <osg/Notify>
#include <osg/KdTree>

using namespace osg;

ShapeDrawable::ShapeDrawable():
    _color(1.0f, 1.0f, 1.0f, 1.0f)
{
}

ShapeDrawable::ShapeDrawable(Shape* shape,TessellationHints* hints):
    _color(1.0f, 1.0f, 1.0f, 1.0f),
    _tessellationHints(hints)
{
    setShape(shape);
}

ShapeDrawable::ShapeDrawable(const ShapeDrawable& sd,const CopyOp& copyop):
    Geometry(sd,copyop),
    _color(sd._color),
    _tessellationHints(sd._tessellationHints)
{
}

ShapeDrawable::~ShapeDrawable()
{
}

void ShapeDrawable::setShape(Shape* shape)
{
    if (_shape==shape) return;

    _shape = shape;

    build();
}


void ShapeDrawable::setColor(const Vec4& color)
{
    _color = color;

    Vec4Array* colors = dynamic_cast<Vec4Array*>(_colorArray.get());
    if (!colors || colors->empty() || colors->getBinding()!=Array::BIND_OVERALL)
    {
        _colorArray = colors = new Vec4Array(Array::BIND_OVERALL, 1);
    }

    (*colors)[0] = color;
    colors->dirty();

    dirtyGLObjects();
}

void ShapeDrawable::setTessellationHints(TessellationHints* hints)
{
    if (_tessellationHints!=hints)
    {
        _tessellationHints = hints;
        build();
    }
}

void ShapeDrawable::build()
{
    // we can't create a tessellation for a KdTree
    if (dynamic_cast<KdTree*>(_shape.get())!=0) return;

    // reset all the properties.
    setVertexArray(0);
    setNormalArray(0);
    setColorArray(0);
    setSecondaryColorArray(0);
    setFogCoordArray(0);
    getTexCoordArrayList().clear();
    getVertexAttribArrayList().clear();
    getPrimitiveSetList().clear();

    if (_shape)
    {
        BuildShapeGeometryVisitor dsv(this, _tessellationHints.get());
        _shape->accept(dsv);
    }

    setColor(_color);
}
