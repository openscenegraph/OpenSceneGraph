/*  -*-c++-*- 
 *  Copyright (C) 2008 Cedric Pinson <mornifle@plopbyte.net>
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

Animation::Animation(const osgAnimation::Animation& anim, const osg::CopyOp& c)
{
    _duration = anim._duration;
    _ratio = anim._ratio;
    _weight = anim._weight;
    _startTime = anim._startTime;
    _playmode = anim._playmode;
}


void Animation::addChannel(Channel* pChannel)
{
    _channels.push_back(pChannel);
    computeDuration();
}

void Animation::computeDuration() 
{
    float tmin = 1e5;
    float tmax = -1e5;
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
    _duration = tmax-tmin;
}

osgAnimation::ChannelList& Animation::getChannels()
{
    return _channels;
}

const osgAnimation::ChannelList& Animation::getChannels() const
{
    return _channels;
}

float Animation::getDuration() const
{
    return _duration;
}

float Animation::getRatio() const
{
    return _ratio;
}

void Animation::setRatio(float ratio)
{
    _ratio = ratio;
}

float Animation::getWeight () const
{
    return _weight;
}

void Animation::setWeight (float weight)
{
    _weight = weight;
}

bool Animation::update (float time)
{
    float t = (time - _startTime) * _ratio;
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
            t = fmodf(t, _duration);
        //      std::cout << "t " << t << " duration " << _duration << std::endl;
        break;
    case PPONG: 
        if (!_duration)
            t = _startTime;
        else 
        {
            int tt = (int) (t / _duration);
            t = fmodf(t, _duration);
            if (tt%2)
                t = _duration - t;
        }
        break;
    }

    //  std::cout << "t " << t << " / " << _duration << std::endl;

    ChannelList::const_iterator chan;
    for( chan=_channels.begin(); chan!=_channels.end(); ++chan) 
    {
        (*chan)->setWeight(_weight);
        (*chan)->update(t);
    }
    return true;
}

void Animation::resetTargets()
{
    ChannelList::const_iterator chan;
    for( chan=_channels.begin(); chan!=_channels.end(); ++chan)
        (*chan)->reset();
}
