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
#include <osgAnimation/ActionBlendIn>
#include <osgAnimation/ActionBlendOut>
#include <osgAnimation/ActionStripAnimation>
#include <osgAnimation/ActionAnimation>
#include <osgAnimation/ActionVisitor>
#include <osgAnimation/Timeline>

using namespace osgAnimation;

ActionVisitor::ActionVisitor()
{
    _currentLayer = 0;
}
void ActionVisitor::pushFrameActionOnStack(const FrameAction& fa) { _stackFrameAction.push_back(fa); }
void ActionVisitor::popFrameAction() { _stackFrameAction.pop_back(); }
void ActionVisitor::pushTimelineOnStack(Timeline* tm) { _stackTimeline.push_back(tm); }
void ActionVisitor::popTimeline() { _stackTimeline.pop_back(); }
void ActionVisitor::apply(Action& action) { traverse(action); }
void ActionVisitor::apply(Timeline& tm) { tm.traverse(*this); }
void ActionVisitor::apply(ActionBlendIn& action) { apply(static_cast<Action&>(action));}
void ActionVisitor::apply(ActionBlendOut& action) { apply(static_cast<Action&>(action)); }
void ActionVisitor::apply(ActionAnimation& action) { apply(static_cast<Action&>(action)); }
void ActionVisitor::apply(ActionStripAnimation& action) { apply(static_cast<Action&>(action)); }
void ActionVisitor::traverse(Action& action)
{
    action.traverse(*this);
}

Timeline* ActionVisitor::getCurrentTimeline()
{
    if (_stackTimeline.empty())
        return 0;
    return _stackTimeline.back();
}

UpdateActionVisitor::UpdateActionVisitor() 
{
    _frame = 0; 
    _currentAnimationPriority = 0;
}


void UpdateActionVisitor::apply(Timeline& tm)
{
    _currentAnimationPriority = 0;

    tm.setEvaluating(true);

    tm.traverse(*this);

    tm.setEvaluating(false);

    tm.setLastFrameEvaluated(_frame);
}

bool UpdateActionVisitor::isActive(Action& action) const
{
    FrameAction fa = _stackFrameAction.back();
    if (_frame < fa.first)
        return false;
    if (!fa.second.valid())
        return false;

    unsigned int f = getLocalFrame();
    unsigned int frameInAction;
    unsigned int loopDone;
    return action.evaluateFrame(f, frameInAction, loopDone);
}

unsigned int UpdateActionVisitor::getLocalFrame() const
{
    return _frame - _stackFrameAction.back().first;
}

void UpdateActionVisitor::apply(Action& action)
{
    if (isActive(action))
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

void UpdateActionVisitor::apply(ActionBlendIn& action)
{
    if (isActive(action)) 
    {
        unsigned int frame = getLocalFrame();
        apply(static_cast<Action&>(action));
        action.computeWeight(frame);
    }
}

void UpdateActionVisitor::apply(ActionBlendOut& action)
{
    if (isActive(action)) 
    {
        unsigned int frame = getLocalFrame();
        apply(static_cast<Action&>(action));
        action.computeWeight(frame);
    }
}

void UpdateActionVisitor::apply(ActionAnimation& action)
{
    if (isActive(action))
    {
        unsigned int frame = getLocalFrame();
        apply(static_cast<Action&>(action));
		int pri = static_cast<int>(_currentAnimationPriority);
		_currentAnimationPriority++;
        action.updateAnimation(frame, -pri);
    }
}

void UpdateActionVisitor::apply(ActionStripAnimation& action)
{
    if (isActive(action))
    {
        apply(static_cast<Action&>(action));
        action.traverse(*this);
    }
}



ClearActionVisitor::ClearActionVisitor(ClearType type) : _clearType(type)
{
}

void ClearActionVisitor::apply(Timeline& tm)
{
    _remove.clear();
    tm.traverse(*this);
    for (int i = 0; i < (int)_remove.size(); i++)
        tm.removeAction(_remove[i].get());
}
void ClearActionVisitor::apply(Action& action)
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
