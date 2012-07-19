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

#include <osgAnimation/StatsVisitor>
#include <osgAnimation/Timeline>
#include <osgAnimation/ActionBlendIn>
#include <osgAnimation/ActionBlendOut>
#include <osgAnimation/ActionStripAnimation>
#include <osgAnimation/ActionAnimation>

using namespace osgAnimation;

StatsActionVisitor::StatsActionVisitor() {}
void StatsActionVisitor::reset() { _channels.clear(); }

StatsActionVisitor::StatsActionVisitor(osg::Stats* stats,unsigned int frame)
{
    _frame = frame;
    _stats = stats;
}

void StatsActionVisitor::apply(Timeline& tm)
{
    _stats->setAttribute(_frame,"Timeline", tm.getCurrentTime());
    tm.traverse(*this);
}

void StatsActionVisitor::apply(Action& action)
{
    if (isActive(action))
    {
        _channels.push_back(action.getName());
        _stats->setAttribute(_frame,action.getName(),1);
    }
}

void StatsActionVisitor::apply(ActionBlendIn& action)
{
    if (isActive(action))
    {
        _channels.push_back(action.getName());
        _stats->setAttribute(_frame,action.getName(), action.getWeight());
    }
}

void StatsActionVisitor::apply(ActionBlendOut& action)
{
    if (isActive(action))
    {
        _channels.push_back(action.getName());
        _stats->setAttribute(_frame,action.getName(), action.getWeight());
    }
}

void StatsActionVisitor::apply(ActionAnimation& action)
{
    if (isActive(action))
    {
        _channels.push_back(action.getName());
        _stats->setAttribute(_frame,action.getName(), action.getAnimation()->getWeight());
    }
}

void StatsActionVisitor::apply(ActionStripAnimation& action)
{
    if (isActive(action))
    {
        _channels.push_back(action.getName());
        double value;
        std::string name = action.getName();
        if (_stats->getAttribute(_frame, name, value))
            name += "+";
        _stats->setAttribute(_frame, action.getName(), action.getAnimation()->getAnimation()->getWeight());
    }
}
