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

osgAnimation::StatsActionVisitor::StatsActionVisitor() {}
void osgAnimation::StatsActionVisitor::reset() { _channels.clear(); }

osgAnimation::StatsActionVisitor::StatsActionVisitor(osg::Stats* stats,unsigned int frame)
{
    _frame = frame;
    _stats = stats;
}

void osgAnimation::StatsActionVisitor::apply(Timeline& tm)
{
    _stats->setAttribute(_frame,"Timeline", tm.getCurrentTime());
    tm.traverse(*this);
}

void osgAnimation::StatsActionVisitor::apply(Action& action)
{
    if (isActive(action))
    {
        _channels.push_back(action.getName());
        _stats->setAttribute(_frame,action.getName(),1);
    }
}

void osgAnimation::StatsActionVisitor::apply(BlendIn& action)
{
    if (isActive(action)) 
    {
        _channels.push_back(action.getName());
        _stats->setAttribute(_frame,action.getName(), action.getWeight());
    }
}

void osgAnimation::StatsActionVisitor::apply(BlendOut& action)
{
    if (isActive(action)) 
    {
        _channels.push_back(action.getName());
        _stats->setAttribute(_frame,action.getName(), action.getWeight());
    }
}

void osgAnimation::StatsActionVisitor::apply(ActionAnimation& action)
{
    if (isActive(action)) 
    {
        _channels.push_back(action.getName());
        _stats->setAttribute(_frame,action.getName(), action.getAnimation()->getWeight());
    }
}

void osgAnimation::StatsActionVisitor::apply(StripAnimation& action)
{
    if (isActive(action))
    {
        _channels.push_back(action.getName());
        _stats->setAttribute(_frame,action.getName(), action.getActionAnimation()->getAnimation()->getWeight());
    }
}
