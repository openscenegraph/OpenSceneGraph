/*  -*-c++-*-
 *  Copyright (C) 2008 Cedric Pinson <cedric.pinson@plopbyte.net>
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

#include <osgAnimation/Timeline>
#include <osgAnimation/StatsVisitor>
#include <limits.h>

using namespace osgAnimation;


Timeline::Timeline()
{
    _lastUpdate = 0;
    _currentFrame = 0;
    _fps = 25;
    _speed = 1.0;
    _state = Stop;
    _initFirstFrame = false;
    _previousFrameEvaluated = 0;
    _evaluating = 0;
    _numberFrame = UINT_MAX; // something like infinity
    _collectStats = false;
    _stats = new osg::Stats("Timeline");
    setName("Timeline");
}

Timeline::Timeline(const Timeline& nc,const osg::CopyOp& op)
    : Action(nc, op),
      _actions(nc._actions)
{
    _lastUpdate = 0;
    _currentFrame = 0;
    _fps = 25;
    _speed = 1.0;
    _state = Stop;
    _initFirstFrame = false;
    _previousFrameEvaluated = 0;
    _evaluating = 0;
    _numberFrame = UINT_MAX; // something like infinity
    _collectStats = false;
    _stats = new osg::Stats("Timeline");
    setName("Timeline");
}

void Timeline::setAnimationManager(AnimationManagerBase* manager)
{
    _animationManager = manager;
}

void Timeline::traverse(ActionVisitor& visitor)
{
    int layer = visitor.getCurrentLayer();
    visitor.pushTimelineOnStack(this);
    // update from high priority to low priority
    for( ActionLayers::reverse_iterator iterAnim = _actions.rbegin(); iterAnim != _actions.rend(); ++iterAnim )
    {
        visitor.setCurrentLayer(iterAnim->first);
        ActionList& list = iterAnim->second;
        for (unsigned int i = 0; i < list.size(); i++)
        {
            visitor.pushFrameActionOnStack(list[i]);
            if (list[i].second) list[i].second->accept(visitor);
            visitor.popFrameAction();
        }
    }
    visitor.popTimeline();
    visitor.setCurrentLayer(layer);
}


void Timeline::setStats(osg::Stats* stats) { _stats = stats;}
osg::Stats* Timeline::getStats() { return _stats.get();}
void Timeline::collectStats(bool state) { _collectStats = state;}
StatsActionVisitor* Timeline::getStatsVisitor() { return _statsVisitor.get(); }

void Timeline::clearActions()
{
    _actions.clear();
    _addActionOperations.clear();
    _removeActionOperations.clear();
}

void Timeline::update(double simulationTime)
{
    // first time we call update we generate one frame
    UpdateActionVisitor updateTimeline;
    if (!_initFirstFrame)
    {
        _lastUpdate = simulationTime;
        _initFirstFrame = true;


        _animationManager->clearTargets();
        updateTimeline.setFrame(_currentFrame);
        accept(updateTimeline);

        if (_collectStats)
        {
            if (!_statsVisitor)
                _statsVisitor = new StatsActionVisitor();
            _statsVisitor->setStats(_stats.get());
            _statsVisitor->setFrame(_currentFrame);
            _statsVisitor->reset();
            accept(*_statsVisitor);
        }

        processPendingOperation();
    }

    // find the number of frame pass since the last update
    double delta = (simulationTime - _lastUpdate);
    double nbframes = delta * _fps * _speed;
    unsigned int nb = static_cast<unsigned int>(floor(nbframes));

    for (unsigned int i = 0; i < nb; i++)
    {
        if (_state == Play)
            _currentFrame++;

        _animationManager->clearTargets();
        updateTimeline.setFrame(_currentFrame);
        accept(updateTimeline);
        if (_collectStats)
        {
            if (!_statsVisitor)
                _statsVisitor = new StatsActionVisitor;
            _statsVisitor->setStats(_stats.get());
            _statsVisitor->setFrame(_currentFrame);
            _statsVisitor->reset();
            accept(*_statsVisitor);
        }

        processPendingOperation();
    }
    if (nb)
    {
        _lastUpdate += ((double)nb) / _fps;
    }
}

void Timeline::removeAction(Action* action)
{
    if (getEvaluating())
        _removeActionOperations.push_back(FrameAction(0, action));
    else
        internalRemoveAction(action);
}

void Timeline::addActionAt(unsigned int frame, Action* action, int priority)
{
    // skip if this action has already been added this frame
    for (CommandList::iterator it = _addActionOperations.begin(); it != _addActionOperations.end(); ++it)
    {
        Command& command = *it;
        if (command._action.second.get() == action) {
            OSG_INFO << "Timeline::addActionAt command " << action->getName() << " already added this frame, declined" << std::endl;
            return;
        }
    }

    if (isActive(action))
    {
        OSG_INFO << "Timeline::addActionAt command " << action->getName() << " already active, remove the old" << std::endl;
        removeAction(action);
    }

    if (getEvaluating())
        _addActionOperations.push_back(Command(priority,FrameAction(frame, action)));
    else
        internalAddAction(priority, FrameAction(frame, action));
}
void Timeline::addActionAt(double t, Action* action, int priority)
{
    unsigned int frame = static_cast<unsigned int>(floor(t * _fps));
    addActionAt(frame, action, priority);
}

void Timeline::addActionNow(Action* action, int priority)
{
    addActionAt(getCurrentFrame()+1, action, priority);
}

void Timeline::processPendingOperation()
{
    // process all pending add action operation
    while( !_addActionOperations.empty())
    {
        internalAddAction(_addActionOperations.back()._priority, _addActionOperations.back()._action);
        _addActionOperations.pop_back();
    }

    // process all pending remove action operation
    while( !_removeActionOperations.empty())
    {
        internalRemoveAction(_removeActionOperations.back().second.get());
        _removeActionOperations.pop_back();
    }
}

void Timeline::internalRemoveAction(Action* action)
{
    for (ActionLayers::iterator it = _actions.begin(); it != _actions.end(); ++it)
    {
        ActionList& fa = it->second;
        for (unsigned int i = 0; i < fa.size(); i++)
            if (fa[i].second.get() == action)
            {
                fa.erase(fa.begin() + i);
                return;
            }
    }
}

void Timeline::internalAddAction(int priority, const FrameAction& ftl)
{
    _actions[priority].insert(_actions[priority].begin(), ftl);
}

bool Timeline::isActive(Action* activeAction)
{
    // update from high priority to low priority
    for( ActionLayers::iterator iterAnim = _actions.begin(); iterAnim != _actions.end(); ++iterAnim )
    {
        // update all animation
        ActionList& list = iterAnim->second;
        for (unsigned int i = 0; i < list.size(); i++)
        {
            Action* action = list[i].second.get();
            if (action == activeAction)
            {
                unsigned int firstFrame = list[i].first;
                // check if current frame of timeline hit an action interval
                if (_currentFrame >= firstFrame &&
                    _currentFrame < (firstFrame + action->getNumFrames()) )
                    return true;
            }
        }
    }
    return false;
}
