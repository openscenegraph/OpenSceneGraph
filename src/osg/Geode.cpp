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
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Notify>

#include <stdio.h>
#include <math.h>

#define square(x)   ((x)*(x))

using namespace osg;

Geode::Geode()
{
}

Geode::Geode(const Geode& geode,const CopyOp& copyop):
    Group(geode,copyop)
{
}

Geode::~Geode()
{
}

bool Geode::addDrawable( Drawable* drawable )
{
    return addChild(drawable);
}


bool Geode::removeDrawable( Drawable* drawable )
{
    return removeDrawables(getDrawableIndex(drawable),1);
}

bool Geode::removeDrawables(unsigned int pos,unsigned int numDrawablesToRemove)
{
    return removeChildren(pos, numDrawablesToRemove);
}

bool Geode::replaceDrawable( Drawable* origDrawable, Drawable* newDrawable )
{
    return replaceChild(origDrawable, newDrawable);
}

bool Geode::setDrawable( unsigned  int i, Drawable* newDrawable )
{
    return setChild(i, newDrawable);
}


BoundingSphere Geode::computeBound() const
{
    BoundingSphere bsphere;

    _bbox.init();

    for(NodeList::const_iterator itr = _children.begin();
        itr!=_children.end();
        ++itr)
    {
        if (itr->valid())
        {
            const osg::Drawable* drawable = (*itr)->asDrawable();
            if (drawable)
            {
                _bbox.expandBy(drawable->getBoundingBox());
            }
            else
            {
                _bbox.expandBy((*itr)->getBound());
            }
        }
    }

    if (_bbox.valid())
    {
        bsphere.expandBy(_bbox);
    }
    return bsphere;
}

void Geode::compileDrawables(RenderInfo& renderInfo)
{
    for(NodeList::iterator itr = _children.begin();
        itr!=_children.end();
        ++itr)
    {
        osg::Drawable* drawable = itr->valid() ? (*itr)->asDrawable() : 0;
        if (drawable) drawable->compileGLObjects(renderInfo);
    }
}
