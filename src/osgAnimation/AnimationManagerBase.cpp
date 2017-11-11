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

#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/LinkVisitor>
#include <algorithm>

using namespace osgAnimation;

AnimationManagerBase::~AnimationManagerBase() {}

AnimationManagerBase::AnimationManagerBase()
{
    _needToLink = false;
    _automaticLink = true;
}

void AnimationManagerBase::clearTargets()
{
    for (TargetSet::iterator it = _targets.begin(); it != _targets.end(); ++it)
        (*it).get()->reset();
}

void AnimationManagerBase::dirty()
{
    _needToLink = true;
}

void AnimationManagerBase::setAutomaticLink(bool state) { _automaticLink = state; }
bool AnimationManagerBase::getAutomaticLink() const { return _automaticLink; }

void AnimationManagerBase::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    if (nv && nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
    {
        if (needToLink())
        {
            /** manager need to link, it means that an animation has been added
                so we need to relink all item animated with all animations.
                We apply the linker visitor on the manager node to affect
                all its children.
                But it should not be done here, it should be done in the
                update of AnimationManager
            */
            link(node);
        }
        const osg::FrameStamp* fs = nv->getFrameStamp();
        update(fs->getSimulationTime());
    }
    traverse(node,nv);
}


AnimationManagerBase::AnimationManagerBase(const AnimationManagerBase& b, const osg::CopyOp& copyop) :
    osg::Object(b, copyop),
    osg::Callback(b, copyop),
    osg::NodeCallback(b,copyop) // TODO check this
{
    const AnimationList& animationList = b.getAnimationList();
    for (AnimationList::const_iterator it = animationList.begin();
         it != animationList.end();
         ++it)
    {
        Animation* animation = dynamic_cast<osgAnimation::Animation*>(it->get()->clone(copyop));
        _animations.push_back(animation);
    }
    _needToLink = true;
    _automaticLink = b._automaticLink;
    buildTargetReference();
}

void AnimationManagerBase::buildTargetReference()
{
    _targets.clear();
    for( AnimationList::iterator iterAnim = _animations.begin(); iterAnim != _animations.end(); ++iterAnim )
    {
        Animation* anim = (*iterAnim).get();
        for (ChannelList::iterator it = anim->getChannels().begin();
             it != anim->getChannels().end();
             ++it)
            _targets.insert((*it)->getTarget());
    }
}


void AnimationManagerBase::registerAnimation (Animation* animation)
{
    _needToLink = true;
    _animations.push_back(animation);
    buildTargetReference();
}

void AnimationManagerBase::removeRegisteredAnimation(Animation* animation)
{
	unregisterAnimation(animation);
}

void AnimationManagerBase::unregisterAnimation(Animation* animation)
{
    AnimationList::iterator it = std::find(_animations.begin(), _animations.end(), animation);
    if (it != _animations.end())
    {
        _animations.erase(it);
    }
    buildTargetReference();
}

bool AnimationManagerBase::needToLink() const { return _needToLink && getAutomaticLink(); }


void AnimationManagerBase::setLinkVisitor(LinkVisitor* visitor)
{
    _linker = visitor;
}

LinkVisitor* AnimationManagerBase::getOrCreateLinkVisitor()
{
    if (!_linker.valid())
        _linker = new LinkVisitor;
    return _linker.get();
}

void AnimationManagerBase::link(osg::Node* subgraph)
{
    LinkVisitor* linker = getOrCreateLinkVisitor();
    linker->getAnimationList().clear();
    linker->getAnimationList() = _animations;

    subgraph->accept(*linker);
    _needToLink = false;
    buildTargetReference();
}
