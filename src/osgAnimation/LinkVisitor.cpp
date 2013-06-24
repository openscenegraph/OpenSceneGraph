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

#include <osgAnimation/LinkVisitor>
#include <osgAnimation/AnimationUpdateCallback>
#include <osg/Notify>
#include <osg/Geode>

using namespace osgAnimation;

LinkVisitor::LinkVisitor() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
{
    _nbLinkedTarget = 0;
}

void LinkVisitor::reset()
{
    _nbLinkedTarget = 0;
}

AnimationList& LinkVisitor::getAnimationList()
{
    return _animations;
}
void LinkVisitor::link(AnimationUpdateCallbackBase* cb)
{
    int result = 0;
    for (int i = 0; i < (int)_animations.size(); i++)
    {
        result += cb->link(_animations[i].get());
        _nbLinkedTarget += result;
    }
    OSG_NOTICE << "LinkVisitor links " << result << " for \"" << cb->getName() << '"' << std::endl;
}

void LinkVisitor::handle_stateset(osg::StateSet* stateset)
{
    if (!stateset)
        return;
    const osg::StateSet::AttributeList& attr = stateset->getAttributeList();
    for (osg::StateSet::AttributeList::const_iterator it = attr.begin(); it != attr.end(); ++it)
    {
        osg::StateAttribute* sattr = it->second.first.get();
        osgAnimation::AnimationUpdateCallbackBase* cb = dynamic_cast<osgAnimation::AnimationUpdateCallbackBase*>(sattr->getUpdateCallback());
        if (cb)
            link(cb);
    }
}

void LinkVisitor::apply(osg::Node& node)
{
    osg::StateSet* st = node.getStateSet();
    if (st)
        handle_stateset(st);

    osg::NodeCallback* cb = node.getUpdateCallback();
    while (cb)
    {
        osgAnimation::AnimationUpdateCallbackBase* cba = dynamic_cast<osgAnimation::AnimationUpdateCallbackBase*>(cb);
        if (cba)
            link(cba);
        cb = cb->getNestedCallback();
    }
    traverse(node);
}

void LinkVisitor::apply(osg::Geode& node)
{
    for (unsigned int i = 0; i < node.getNumDrawables(); i++)
    {
        osg::Drawable* drawable = node.getDrawable(i);
        if (drawable && drawable->getStateSet())
            handle_stateset(drawable->getStateSet());
    }
    apply(static_cast<osg::Node&>(node));
}
