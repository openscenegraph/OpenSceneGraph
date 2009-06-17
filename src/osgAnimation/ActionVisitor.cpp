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

#include <osgAnimation/Action>
#include <osgAnimation/ActionVisitor>
#include <osgAnimation/Timeline>

void osgAnimation::ActionVisitor::pushFrameActionOnStack(FrameAction& fa) { _stackFrameAction.push_back(fa); }
void osgAnimation::ActionVisitor::popFrameAction() { _stackFrameAction.pop_back(); }
void osgAnimation::ActionVisitor::pushTimelineOnStack(Timeline* tm) { _stackTimeline.push_back(tm); }
void osgAnimation::ActionVisitor::popTimeline() { _stackTimeline.pop_back(); }
void osgAnimation::ActionVisitor::apply(Action& action) { traverse(action); }
void osgAnimation::ActionVisitor::apply(Timeline& tm) { tm.traverse(*this); }
void osgAnimation::ActionVisitor::apply(BlendIn& action) { apply(static_cast<Action&>(action));}
void osgAnimation::ActionVisitor::apply(BlendOut& action) { apply(static_cast<Action&>(action)); }
void osgAnimation::ActionVisitor::apply(ActionAnimation& action) { apply(static_cast<Action&>(action)); }
void osgAnimation::ActionVisitor::apply(StripAnimation& action) { apply(static_cast<Action&>(action)); }
void osgAnimation::ActionVisitor::traverse(Action& action)
{
    action.traverse(*this);
}

osgAnimation::Timeline* osgAnimation::ActionVisitor::getCurrentTimeline()
{
    if (_stackTimeline.empty())
        return 0;
    return _stackTimeline.back();
}

osgAnimation::UpdateActionVisitor::UpdateActionVisitor() { _frame = 0; }


void osgAnimation::UpdateActionVisitor::apply(Timeline& tm)
{
    tm.setEvaluating(true);

    tm.traverse(*this);

    tm.setEvaluating(false);

    tm.setLastFrameEvaluated(_frame);
}

bool osgAnimation::UpdateActionVisitor::isActive() const
{
    FrameAction fa = _stackFrameAction.back();
    if (_frame < fa.first)
        return false;
    if (!fa.second.valid())
        return false;
    if (getLocalFrame() >= fa.second->getNumFrames())
        return false;
    return true;
}
unsigned int osgAnimation::UpdateActionVisitor::getLocalFrame() const
{
    return _frame - _stackFrameAction.back().first;
}

void osgAnimation::UpdateActionVisitor::apply(Action& action)
{
    if (isActive())
    {
        unsigned int frame = getLocalFrame();

        unsigned int frameInAction;
        unsigned int loopDone;
        bool result = action.evaluateFrame(frame, frameInAction, loopDone);
        if (!result)
        {
            osg::notify(osg::DEBUG_INFO) << action.getName() << " Action frame " << frameInAction  << " finished" << std::endl;
            return;
        }
        osg::notify(osg::DEBUG_INFO) << action.getName() << " Action frame " << frame  << " relative to loop " << frameInAction  << " no loop " << loopDone<< std::endl;

        frame = frameInAction;
        Action::Callback* cb = action.getFrameCallback(frame);
        while (cb)
        {
            osg::notify(osg::DEBUG_INFO) << action.getName() << " evaluate callback " << cb->getName() << " at " << frame << std::endl;
            (*cb)(&action, this);
            cb = cb->getNestedCallback();
        }
    }
}

void osgAnimation::UpdateActionVisitor::apply(BlendIn& action)
{
    if (isActive()) 
    {
        unsigned int frame = getLocalFrame();
        apply(static_cast<Action&>(action));
        action.computeWeight(frame);
    }
}

void osgAnimation::UpdateActionVisitor::apply(BlendOut& action)
{
    if (isActive()) 
    {
        unsigned int frame = getLocalFrame();
        apply(static_cast<Action&>(action));
        action.computeWeight(frame);
    }
}

void osgAnimation::UpdateActionVisitor::apply(ActionAnimation& action)
{
    if (isActive()) 
    {
        unsigned int frame = getLocalFrame();
        apply(static_cast<Action&>(action));
        action.updateAnimation(frame);
    }
}

void osgAnimation::UpdateActionVisitor::apply(StripAnimation& action)
{
    if (isActive())
    {
        unsigned int frame = getLocalFrame();
        apply(static_cast<Action&>(action));
        action.computeWeightAndUpdateAnimation(frame);
    }
}



osgAnimation::ClearActionVisitor::ClearActionVisitor(ClearType type) : _clearType(type)
{
}

void osgAnimation::ClearActionVisitor::apply(Timeline& tm)
{
    _remove.clear();
    tm.traverse(*this);
    for (int i = 0; i < (int)_remove.size(); i++)
        tm.removeAction(_remove[i].get());
}
void osgAnimation::ClearActionVisitor::apply(Action& action)
{
    FrameAction fa = _stackFrameAction.back();
    switch( _clearType) {
    case BEFORE_FRAME:
        if (_frame > fa.first)
            _remove.push_back(&action);
        break;
    case AFTER_FRAME:
        if (_frame - fa.first > action.getNumFrames())
            _remove.push_back(&action);
        break;
    }
}
