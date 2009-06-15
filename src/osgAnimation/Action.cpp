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

osgAnimation::Action::Action()
{
    _numberFrame = 25;
    _fps = 25;
    _speed = 1.0;
    _loop = 1;
}
osgAnimation::Action::Action(const Action&,const osg::CopyOp&) {}
osgAnimation::Action::Callback* osgAnimation::Action::getFrameCallback(unsigned int frame)
{
    if (_framesCallback.find(frame) != _framesCallback.end())
    {
        return _framesCallback[frame];
    }
    return 0;
}

osgAnimation::Action::Callback* osgAnimation::Action::getFrameCallback(double time)
{
    unsigned int frame = static_cast<unsigned int>(floor(time * _fps));
    return getFrameCallback(frame);
}

bool osgAnimation::Action::evaluateFrame(unsigned int frame, unsigned int& resultframe, unsigned int& nbloop )
{
    nbloop = frame / getNumFrames();
    resultframe = frame;

    if (frame > getNumFrames()-1)
    {
        if (!getLoop())
            resultframe = frame % getNumFrames();
        else
        {
            if (nbloop >= getLoop())
                return false;
            else
                resultframe = frame % getNumFrames();
        }
    }
    return true;
}

#if 0
void osgAnimation::Action::evaluate(unsigned int frame)
{
    unsigned int frameInAction;
    unsigned int loopDone;
    bool result = evaluateFrame(frame, frameInAction, loopDone);
    if (!result)
    {
        osg::notify(osg::DEBUG_INFO) << getName() << " Action frame " << frameInAction  << " finished" << std::endl;
        return;
    }
    osg::notify(osg::DEBUG_INFO) << getName() << " Action frame " << frame  << " relative to loop " << frameInAction  << " no loop " << loopDone<< std::endl;

    frame = frameInAction;
    if (_framesCallback.find(frame) != _framesCallback.end())
    {
        osg::notify(osg::DEBUG_INFO) << getName() << " evaluate callback " << _framesCallback[frame]->getName() << " at " << frame << std::endl;
        (*_framesCallback[frame])(this, visitor);
    }
}
#endif


osgAnimation::BlendIn::BlendIn(Animation* animation, double duration, double weight)
{
    _animation = animation;
    _weight = weight;
    float d = duration * _fps;
    setNumFrames(static_cast<unsigned int>(floor(d)) + 1);
    setName("BlendIn");
}

void osgAnimation::BlendIn::computeWeight(unsigned int frame)
{
    // frame + 1 because the start is 0 and we want to start the blend in at the first
    // frame.
    double ratio = ( (frame+1) * 1.0 / (getNumFrames()) );
    double w = _weight * ratio;
    _animation->setWeight(w);
}

#if 0
void osgAnimation::BlendIn::evaluate(unsigned int frame)
{
    Action::evaluate(frame);
    // frame + 1 because the start is 0 and we want to start the blend in at the first
    // frame.
    double ratio = ( (frame+1) * 1.0 / (getNumFrames()) );
    double w = _weight;
    if (frame < getNumFrames() -1 ) // the last frame we set the target weight the user asked
        w = _weight * ratio;
    _animation->setWeight(w);
}
#endif


osgAnimation::BlendOut::BlendOut(Animation* animation, double duration)
{
    _animation = animation;
    float d = duration * _fps;
    setNumFrames(static_cast<unsigned int>(floor(d) + 1));
    _weight = 1.0;
    setName("BlendOut");
}

void osgAnimation::BlendOut::computeWeight(unsigned int frame)
{
    double ratio = ( (frame+1) * 1.0 / (getNumFrames()) );
    double w = _weight * (1.0-ratio);
    _animation->setWeight(w);
}

#if 0
void osgAnimation::BlendOut::evaluate(unsigned int frame)
{
    Action::evaluate(frame);
    // frame + 1 because the start is 0 and we want to start the blend in at the first
    // frame.
    double ratio = ( (frame+1) * 1.0 / (getNumFrames()) );
    double w = 0.0;
    if (frame < getNumFrames() -1 ) // the last frame we set the target weight the user asked
        w = _weight * (1.0-ratio);
    _animation->setWeight(w);
}
#endif


osgAnimation::ActionAnimation::ActionAnimation(Animation* animation) : _animation(animation)
{
    Action::setDuration(animation->getDuration());
    setName(animation->getName());
}
void osgAnimation::ActionAnimation::updateAnimation(unsigned int frame)
{
    _animation->update(frame * 1.0/_fps);
}





osgAnimation::StripAnimation::StripAnimation(const StripAnimation& a, const osg::CopyOp& c) : Action(a,c) 
{
    _animation = a._animation;
    _blendIn = a._blendIn;
    _blendOut = a._blendOut;
}

osgAnimation::StripAnimation::StripAnimation(Animation* animation, double blendInDuration, double blendOutDuration, double blendInWeightTarget)
{
    _blendIn = new BlendIn(animation, blendInDuration, blendInWeightTarget);
    _animation = new ActionAnimation(animation);
    unsigned int start = static_cast<unsigned int>(floor((_animation->getDuration() - blendOutDuration) * _fps));
    _blendOut = FrameAction(start, new BlendOut(animation, blendOutDuration));
    setName(animation->getName() + "_Strip");
    _blendIn->setName(_animation->getName() + "_" + _blendIn->getName());
    _blendOut.second->setName(_animation->getName() + "_" + _blendOut.second->getName());
    setDuration(animation->getDuration());
}


void osgAnimation::StripAnimation::setLoop(unsigned int loop)
{
    _animation->setLoop(loop);
    if (!loop)
        setDuration(-1);
    else
        setDuration(loop * _animation->getDuration());

    // duration changed re evaluate the blendout duration
    unsigned int start = static_cast<unsigned int>(floor((getDuration() - _blendOut.second->getDuration()) * _fps));
    _blendOut = FrameAction(start, _blendOut.second);
}

void osgAnimation::StripAnimation::computeWeightAndUpdateAnimation(unsigned int frame)
{
    if (frame < _blendIn->getNumFrames())
        _blendIn->computeWeight(frame);
    if (frame >= _blendOut.first)
        _blendOut.second->computeWeight(frame - _blendOut.first);
    _animation->updateAnimation(frame);
}
