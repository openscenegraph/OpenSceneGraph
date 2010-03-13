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


#include <osgAnimation/ActionStripAnimation>

using namespace osgAnimation;

ActionAnimation* ActionStripAnimation::getAnimation() { return _animation.get(); }
ActionBlendIn* ActionStripAnimation::getBlendIn() { return _blendIn.get(); }
ActionBlendOut* ActionStripAnimation::getBlendOut() { return _blendOut.second.get(); }
const ActionAnimation* ActionStripAnimation::getAnimation() const { return _animation.get(); }
const ActionBlendIn* ActionStripAnimation::getBlendIn() const { return _blendIn.get(); }
const ActionBlendOut* ActionStripAnimation::getBlendOut() const { return _blendOut.second.get(); }
unsigned int ActionStripAnimation::getBlendOutStartFrame() const { return _blendOut.first; }
        
unsigned int ActionStripAnimation::getLoop() const { return _animation->getLoop(); }


ActionStripAnimation::ActionStripAnimation(const ActionStripAnimation& a, const osg::CopyOp& c) : Action(a,c) 
{
    _animation = a._animation;
    _blendIn = a._blendIn;
    _blendOut = a._blendOut;
}

ActionStripAnimation::ActionStripAnimation(Animation* animation, double blendInDuration, double blendOutDuration, double blendInWeightTarget)
{
    _blendIn = new ActionBlendIn(animation, blendInDuration, blendInWeightTarget);
    _animation = new ActionAnimation(animation);
    unsigned int start = static_cast<unsigned int>(floor((_animation->getDuration() - blendOutDuration) * _fps));
    _blendOut = FrameBlendOut(start, new ActionBlendOut(animation, blendOutDuration));
    setName(animation->getName() + "_Strip");
    _blendIn->setName(_animation->getName() + "_" + _blendIn->getName());
    _blendOut.second->setName(_animation->getName() + "_" + _blendOut.second->getName());
    setDuration(animation->getDuration());
}


void ActionStripAnimation::setLoop(unsigned int loop)
{
    _animation->setLoop(loop);
    if (!loop)
        setDuration(-1);
    else
        setDuration(loop * _animation->getDuration());

    // duration changed re evaluate the blendout duration
    unsigned int start = static_cast<unsigned int>(floor((getDuration() - _blendOut.second->getDuration()) * _fps));
    _blendOut = FrameBlendOut(start, _blendOut.second);
}

void ActionStripAnimation::traverse(ActionVisitor& visitor)
{
    if (_blendIn.valid())
    {
        unsigned int f = visitor.getStackedFrameAction().back().first;
        visitor.pushFrameActionOnStack(FrameAction(f,_blendIn.get()));
        _blendIn->accept(visitor);
        visitor.popFrameAction();
    }
    if (_blendOut.second.valid())
    {
        unsigned int f = visitor.getStackedFrameAction().back().first;
        visitor.pushFrameActionOnStack(FrameAction(f + _blendOut.first,_blendOut.second.get()));
        _blendOut.second.get()->accept(visitor);
        visitor.popFrameAction();
    }

    if (_animation.valid())
    {
        unsigned int f = visitor.getStackedFrameAction().back().first;
        visitor.pushFrameActionOnStack(FrameAction(f,_animation.get()));
        _animation->accept(visitor);
        visitor.popFrameAction();
    }
}
