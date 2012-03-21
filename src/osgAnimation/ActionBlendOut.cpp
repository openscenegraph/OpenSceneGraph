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

#include <osgAnimation/ActionBlendOut>

using namespace osgAnimation;

ActionBlendOut::ActionBlendOut() : _weight(0) {}
ActionBlendOut::ActionBlendOut(const ActionBlendOut& a, const osg::CopyOp& c) : Action(a,c)
{
    _weight = a._weight;
    _animation = a._animation;
}

ActionBlendOut::ActionBlendOut(Animation* animation, double duration)
{
    _animation = animation;
    float d = duration * _fps;
    setNumFrames(static_cast<unsigned int>(floor(d) + 1));
    _weight = 1.0;
    setName("BlendOut");
}

void ActionBlendOut::computeWeight(unsigned int frame)
{
    double ratio = ( (frame+1) * 1.0 / (getNumFrames()) );
    double w = _weight * (1.0-ratio);
    OSG_DEBUG << getName() << " BlendOut frame " << frame  << " weight " << w << std::endl;
    _animation->setWeight(w);
}
