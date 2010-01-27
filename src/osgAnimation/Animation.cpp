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

#include <osgAnimation/Animation>

using namespace osgAnimation;

Animation::Animation(const osgAnimation::Animation& anim, const osg::CopyOp& copyop): osg::Object(anim, copyop),
    _duration(anim._duration),
    _originalDuration(anim._originalDuration),
    _weight(anim._weight),
    _startTime(anim._startTime),
    _playmode(anim._playmode)
{
    const ChannelList& cl = anim.getChannels();
    for (ChannelList::const_iterator it = cl.begin(); it != cl.end(); ++it)
    {
        addChannel(it->get()->clone());
    }
}


void Animation::addChannel(Channel* pChannel)
{
    _channels.push_back(pChannel);
    if (!_duration)
        computeDuration();
    else
        _originalDuration = computeDurationFromChannels();
}

double Animation::computeDurationFromChannels() const
{
    double tmin = 1e5;
    double tmax = -1e5;
    ChannelList::const_iterator chan;
    for( chan=_channels.begin(); chan!=_channels.end(); chan++ )
    {
        float min = (*chan)->getStartTime();
        if (min < tmin)
            tmin = min;
        float max = (*chan)->getEndTime();
        if (max > tmax)
            tmax = max;
    }
    return tmax-tmin;
}

void Animation::computeDuration()
{
    _duration = computeDurationFromChannels();
    _originalDuration = _duration;
}

osgAnimation::ChannelList& Animation::getChannels()
{
    return _channels;
}

const osgAnimation::ChannelList& Animation::getChannels() const
{
    return _channels;
}


void Animation::setDuration(double duration)
{
    _originalDuration = computeDurationFromChannels();
    _duration = duration;
}

float Animation::getDuration() const
{
    return _duration;
}

float Animation::getWeight () const
{
    return _weight;
}

void Animation::setWeight (float weight)
{
    _weight = weight;
}

bool Animation::update (float time, int priority)
{
    if (!_duration) // if not initialized then do it
        computeDuration();

    double ratio = _originalDuration / _duration;

    float t = (time - _startTime) * ratio;
    switch (_playmode) 
    {
    case ONCE:
        if (t > _duration)
            return false;
        break;
    case STAY:
        if (t > _duration)
            t = _duration;
        break;
    case LOOP:
        if (!_duration)
            t = _startTime;
        else if (t > _duration)
            t = fmod(t, (float)_duration);
        //      std::cout << "t " << t << " duration " << _duration << std::endl;
        break;
    case PPONG: 
        if (!_duration)
            t = _startTime;
        else 
        {
            int tt = (int) (t / _duration);
            t = fmod(t, (float)_duration);
            if (tt%2)
                t = _duration - t;
        }
        break;
    }

    ChannelList::const_iterator chan;
    for( chan=_channels.begin(); chan!=_channels.end(); ++chan)
    {
        (*chan)->update(t, _weight, priority);
    }
    return true;
}

void Animation::resetTargets()
{
    ChannelList::const_iterator chan;
    for( chan=_channels.begin(); chan!=_channels.end(); ++chan)
        (*chan)->reset();
}
