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

using namespace osgAnimation;

Action::Action()
{
    _numberFrame = 25;
    _fps = 25;
    _speed = 1.0;
    _loop = 1;
}
Action::Action(const Action&,const osg::CopyOp&) {}
Action::Callback* Action::getFrameCallback(unsigned int frame)
{
    if (_framesCallback.find(frame) != _framesCallback.end())
    {
        return _framesCallback[frame].get();
    }
    return 0;
}

void Action::removeCallback(Callback* cb)
{
    std::vector<unsigned int> keyToRemove;
    for (FrameCallback::iterator it = _framesCallback.begin(); it != _framesCallback.end(); ++it)
    {
        if (it->second.get())
        {
            if (it->second.get() == cb)
            {
                it->second = it->second->getNestedCallback();
                if (!it->second.valid())
                    keyToRemove.push_back(it->first);
            }
            else
            {
                it->second->removeCallback(cb);
            }
        }
    }
    for (std::vector<unsigned int>::iterator it = keyToRemove.begin(); it != keyToRemove.end(); ++it)
        _framesCallback.erase(*it);
}

Action::Callback* Action::getFrameCallback(double time)
{
    unsigned int frame = static_cast<unsigned int>(floor(time * _fps));
    return getFrameCallback(frame);
}

bool osgAnimation::Action::evaluateFrame(unsigned int frame, unsigned int& resultframe, unsigned int& nbloop )
{
    unsigned int nbFrames = getNumFrames();
    if (!nbFrames) {
        nbFrames = 1;
        osg::notify(osg::WARN) << "osgAnimation::Action::evaluateFrame your action " << getName() << " has 0 frames, it seems like an error in the data" << std::endl;
    }

    nbloop = frame / nbFrames;
    resultframe = frame;

    if (frame > nbFrames-1)
    {
        if (!getLoop())
            resultframe = frame % nbFrames;
        else
        {
            if (nbloop >= getLoop())
                return false;
            else
                resultframe = frame % nbFrames;
        }
    }
    return true;
}
