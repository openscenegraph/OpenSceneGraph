/*  -*-c++-*-
 *  Copyright (C) 2009 Cedric Pinson <cedric.pinson@plopbyte.net>
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

#include <osgAnimation/StackedTransform>

using namespace osgAnimation;

StackedTransform::StackedTransform() {}

StackedTransform::StackedTransform(const StackedTransform& rhs, const osg::CopyOp& co)
{
    reserve(rhs.size());
    for (StackedTransform::const_iterator it = rhs.begin(); it != rhs.end(); ++it)
    {
        const StackedTransformElement* element = it->get();
        if (element)
            push_back(osg::clone(element,co));
    }
}


void StackedTransform::update(float t)
{
    bool dirty = false;
    for (StackedTransform::iterator it = begin(); it != end(); ++it)
    {
        StackedTransformElement* element = it->get();
        if (!element)
            continue;
        // update and check if there are changes
        element->update(t);
        if (!dirty && !element->isIdentity()){
            dirty = true;
        }
    }

    if (!dirty)
        return;


    // dirty update matrix
    _matrix.makeIdentity();
    for (StackedTransform::iterator it = begin(); it != end(); ++it)
    {
        StackedTransformElement* element = it->get();
        if (!element || element->isIdentity())
            continue;
        element->applyToMatrix(_matrix);
    }
}

const osg::Matrix& StackedTransform::getMatrix() const
{
    return _matrix;
}
