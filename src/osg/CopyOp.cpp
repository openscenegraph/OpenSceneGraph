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
#include <osg/CopyOp>
#include <osg/Node>
#include <osg/StateSet>
#include <osg/Texture>
#include <osg/Drawable>
#include <osg/Array>
#include <osg/PrimitiveSet>
#include <osg/Shape>
#include <osg/StateAttribute>
#include <osg/Callback>

using namespace osg;

#define COPY_OP( TYPE, FLAG ) \
TYPE* CopyOp::operator() (const TYPE* obj) const \
{ \
    if (obj && _flags&FLAG) \
        return osg::clone(obj, *this); \
    else \
        return const_cast<TYPE*>(obj); \
}

COPY_OP( Object,                   DEEP_COPY_OBJECTS )
COPY_OP( StateSet,                 DEEP_COPY_STATESETS )
COPY_OP( Image,                    DEEP_COPY_IMAGES )
COPY_OP( Uniform,                  DEEP_COPY_UNIFORMS )
COPY_OP( UniformCallback,          DEEP_COPY_CALLBACKS )
COPY_OP( StateAttributeCallback,   DEEP_COPY_CALLBACKS )
COPY_OP( Drawable,                 DEEP_COPY_DRAWABLES )
COPY_OP( Texture,                  DEEP_COPY_TEXTURES )
COPY_OP( Array,                    DEEP_COPY_ARRAYS )
COPY_OP( PrimitiveSet,             DEEP_COPY_PRIMITIVES )
COPY_OP( Shape,                    DEEP_COPY_SHAPES )

Referenced* CopyOp::operator() (const Referenced* ref) const
{
    return const_cast<Referenced*>(ref);
}

Node* CopyOp::operator() (const Node* node) const
{
    if (!node) return 0;

    const Drawable* drawable = node->asDrawable();
    if (drawable) return operator()(drawable);
    else if (_flags&DEEP_COPY_NODES) return osg::clone(node, *this);
    else return const_cast<Node*>(node);
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
            return osg::clone(attr, *this);
        }
    }
    else
        return const_cast<StateAttribute*>(attr);
}

Callback* CopyOp::operator() (const Callback* nc) const
{
    if (nc && _flags&DEEP_COPY_CALLBACKS)
    {
        // deep copy the full chain of callback
        Callback* first = osg::clone(nc, *this);
        if (!first) return 0;

        first->setNestedCallback(0);
        nc = nc->getNestedCallback();
        while (nc)
        {
            Callback* ucb = osg::clone(nc, *this);
            if (ucb)
            {
                ucb->setNestedCallback(0);
                first->addNestedCallback(ucb);
            }
            nc = nc->getNestedCallback();
        }
        return first;
    }
    else
        return const_cast<Callback*>(nc);
}
