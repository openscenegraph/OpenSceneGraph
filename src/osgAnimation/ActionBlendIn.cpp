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

#include <osgAnimation/ActionBlendIn>

using namespace osgAnimation;

ActionBlendIn::ActionBlendIn() : _weight(0) {}
ActionBlendIn::ActionBlendIn(const ActionBlendIn& a, const osg::CopyOp& c) : Action(a,c)
{
    _weight = a._weight;
    _animation = a._animation;
}

ActionBlendIn::ActionBlendIn(Animation* animation, double duration, double weight)
{
    _animation = animation;
    _weight = weight;
    float d = duration * _fps;
    setNumFrames(static_cast<unsigned int>(floor(d)) + 1);
    setName("BlendIn");
}

void ActionBlendIn::computeWeight(unsigned int frame)
{

    // frame + 1 because the start is 0 and we want to start the blend in at the first
    // frame.
    double ratio = ( (frame+1) * 1.0 / (getNumFrames()) );
    double w = _weight * ratio;

    OSG_DEBUG << getName() << " BlendIn frame " << frame  << " weight " << w << std::endl;
    _animation->setWeight(w);
}
