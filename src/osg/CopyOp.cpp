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
#include <osg/CopyOp>
#include <osg/Node>
#include <osg/StateSet>
#include <osg/Texture>
#include <osg/Drawable>
#include <osg/Array>
#include <osg/PrimitiveSet>
#include <osg/Shape>

using namespace osg;

Referenced* CopyOp::operator() (const Referenced* ref) const
{
    return const_cast<Referenced*>(ref);
}

Object* CopyOp::operator() (const Object* obj) const
{
    if (obj && _flags&DEEP_COPY_OBJECTS)
        return obj->clone(*this);
    else return const_cast<Object*>(obj);
}

Node* CopyOp::operator() (const Node* node) const
{
    if (node && _flags&DEEP_COPY_NODES)
        return dynamic_cast<Node*>(node->clone(*this));
    else
        return const_cast<Node*>(node);
}

Drawable* CopyOp::operator() (const Drawable* drawable) const
{
    if (drawable && _flags&DEEP_COPY_DRAWABLES)
        return dynamic_cast<Drawable*>(drawable->clone(*this));
    else
        return const_cast<Drawable*>(drawable);
}

StateSet* CopyOp::operator() (const StateSet* stateset) const
{
    if (stateset && _flags&DEEP_COPY_STATESETS)
        return dynamic_cast<StateSet*>(stateset->clone(*this));
    else
        return const_cast<StateSet*>(stateset);
}

StateAttribute* CopyOp::operator() (const StateAttribute* attr) const
{
    if (attr && _flags&DEEP_COPY_STATEATTRIBUTES)
    {
        const Texture* textbase = dynamic_cast<const Texture*>(attr);
        if (textbase)
        {
            return operator()(textbase);
        }
        else 
        {
            return dynamic_cast<StateAttribute*>(attr->clone(*this));
        }
    }
    else
        return const_cast<StateAttribute*>(attr);
}

Texture* CopyOp::operator() (const Texture* text) const
{
    if (text && _flags&DEEP_COPY_TEXTURES)
        return dynamic_cast<Texture*>(text->clone(*this));
    else
        return const_cast<Texture*>(text);
}

Image* CopyOp::operator() (const Image* image) const
{
    if (image && _flags&DEEP_COPY_IMAGES)
        return dynamic_cast<Image*>(image->clone(*this));
    else return const_cast<Image*>(image);
}

Array* CopyOp::operator() (const Array* array) const
{
    if (array && _flags&DEEP_COPY_ARRAYS)
        return dynamic_cast<Array*>(array->clone(*this));
    else
        return const_cast<Array*>(array);
}

PrimitiveSet* CopyOp::operator() (const PrimitiveSet* primitive) const
{
    if (primitive && _flags&DEEP_COPY_PRIMITIVES)
        return dynamic_cast<PrimitiveSet*>(primitive->clone(*this));
    else
        return const_cast<PrimitiveSet*>(primitive);
}

Shape* CopyOp::operator() (const Shape* shape) const
{
    if (shape && _flags&DEEP_COPY_SHAPES)
        return dynamic_cast<Shape*>(shape->clone(*this));
    else
        return const_cast<Shape*>(shape);
}

