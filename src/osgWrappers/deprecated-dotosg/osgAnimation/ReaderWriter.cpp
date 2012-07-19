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


#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/ReaderWriter>
#include <osg/io_utils>
#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/TimelineAnimationManager>
#include <osgAnimation/VertexInfluence>
#include <osgAnimation/Animation>
#include <osgAnimation/Bone>
#include <osgAnimation/UpdateBone>
#include <osgAnimation/UpdateMatrixTransform>
#include <osgAnimation/Skeleton>
#include <osgAnimation/RigGeometry>
#include <osgAnimation/MorphGeometry>
#include <osgAnimation/StackedTransform>
#include <osgAnimation/StackedTranslateElement>
#include <osgAnimation/StackedRotateAxisElement>
#include <osgAnimation/StackedMatrixElement>
#include <osgAnimation/StackedScaleElement>
#include "Matrix.h"

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osgDB;
using namespace osg;

bool Bone_readLocalData(Object& obj, Input& fr)
{
    osgAnimation::Bone& bone = dynamic_cast<osgAnimation::Bone&>(obj);

    osg::Quat att;
    bool iteratorAdvanced = false;
    if (fr.matchSequence("bindQuaternion %f %f %f %f"))
    {
        fr[1].getFloat(att[0]);
        fr[2].getFloat(att[1]);
        fr[3].getFloat(att[2]);
        fr[4].getFloat(att[3]);

        fr += 5;
        iteratorAdvanced = true;
        osg::notify(osg::WARN) << "Old osgAnimation file format update your data file" << std::endl;
    }

    osg::Vec3d pos(0,0,0);
    if (fr.matchSequence("bindPosition %f %f %f"))
    {
        fr[1].getFloat(pos[0]);
        fr[2].getFloat(pos[1]);
        fr[3].getFloat(pos[2]);

        fr += 4;
        iteratorAdvanced = true;
        osg::notify(osg::WARN) << "Old osgAnimation file format update your data file" << std::endl;
    }

    osg::Vec3d scale(1,1,1);
    if (fr.matchSequence("bindScale %f %f %f"))
    {
        fr[1].getFloat(scale[0]);
        fr[2].getFloat(scale[1]);
        fr[3].getFloat(scale[2]);

        fr += 4;
        iteratorAdvanced = true;
        osg::notify(osg::WARN) << "Old osgAnimation file format update your data file" << std::endl;
    }

    if (fr.matchSequence("InvBindMatrixInSkeletonSpace {"))
    {
        Matrix matrix;
        if (readMatrix(matrix,fr, "InvBindMatrixInSkeletonSpace"))
        {
            bone.setInvBindMatrixInSkeletonSpace(matrix);
            iteratorAdvanced = true;
        }
    }
    if (fr.matchSequence("MatrixInSkeletonSpace {"))
    {
        Matrix matrix;
        if (readMatrix(matrix,fr, "MatrixInSkeletonSpace"))
        {
            bone.setMatrixInSkeletonSpace(matrix);
            iteratorAdvanced = true;
        }
    }
    return iteratorAdvanced;
}

bool Bone_writeLocalData(const Object& obj, Output& fw)
{
    const osgAnimation::Bone& bone = dynamic_cast<const osgAnimation::Bone&>(obj);
    bool res1 = writeMatrix(bone.getInvBindMatrixInSkeletonSpace(), fw, "InvBindMatrixInSkeletonSpace");
    // write this field for debug only
    // because it's recompute at each update
    bool res2 = writeMatrix(bone.getMatrixInSkeletonSpace(), fw, "MatrixInSkeletonSpace");
    return (res1 || res2);
}

RegisterDotOsgWrapperProxy g_BoneProxy
(
    new osgAnimation::Bone,
    "osgAnimation::Bone",
    "Object Node MatrixTransform osgAnimation::Bone Group",
    &Bone_readLocalData,
    &Bone_writeLocalData
    );



bool Skeleton_readLocalData(Object& obj, Input& fr)
{
    return false;
}
bool Skeleton_writeLocalData(const Object& obj, Output& fr)
{
    return true;
}
RegisterDotOsgWrapperProxy g_SkeletonProxy
(
    new osgAnimation::Skeleton,
    "osgAnimation::Skeleton",
    "Object Node MatrixTransform osgAnimation::Skeleton Group",
    &Skeleton_readLocalData,
    &Skeleton_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
    );

// Helper method for reading channels
bool Animation_readChannel(osgAnimation::Channel* pChannel, Input& fr)
{
    bool iteratorAdvanced = false;
    std::string name = "unknown";
    if (fr.matchSequence("name %s"))
    {
        if (fr[1].getStr())
            name = fr[1].getStr();
        fr += 2;
        iteratorAdvanced = true;
    }
    pChannel->setName(name);

    std::string target = "unknown";
    if (fr.matchSequence("target %s"))
    {
        if (fr[1].getStr())
            target = fr[1].getStr();
        fr += 2;
        iteratorAdvanced = true;
    }
    pChannel->setTargetName(target);

// we dont need this info
    float weight = 1.0;
    if (fr.matchSequence("weight %f"))
    {
        fr[1].getFloat(weight);
        fr += 2;
        iteratorAdvanced = true;
    }
//    pChannel->setWeight(weight);
    return iteratorAdvanced;
}

bool Animation_readLocalData(Object& obj, Input& fr)
{
    osgAnimation::Animation& anim = dynamic_cast<osgAnimation::Animation&>(obj);
    bool iteratorAdvanced = false;

    if (fr.matchSequence("playmode %w"))
    {
        if      (fr[1].matchWord("ONCE")) anim.setPlayMode(osgAnimation::Animation::ONCE);
        else if (fr[1].matchWord("STAY")) anim.setPlayMode(osgAnimation::Animation::STAY);
        else if (fr[1].matchWord("LOOP")) anim.setPlayMode(osgAnimation::Animation::LOOP);
        else if (fr[1].matchWord("PPONG")) anim.setPlayMode(osgAnimation::Animation::PPONG);
        fr += 2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("weight %f"))
    {
        float weight;
        fr[1].getFloat(weight);
        fr += 2;
        iteratorAdvanced = true;
        anim.setWeight(weight);
    }

    if (fr.matchSequence("duration %f"))
    {
        float duration;
        fr[1].getFloat(duration);
        fr += 2;
        iteratorAdvanced = true;
        anim.setDuration(duration);
    }

    if (fr.matchSequence("starttime %f"))
    {
        float starttime;
        fr[1].getFloat(starttime);
        fr += 2;
        iteratorAdvanced = true;
        anim.setStartTime(starttime);
    }

    int nbChannels = 0;
    if (fr.matchSequence("num_channels %i"))
    {
        fr[1].getInt(nbChannels);
        fr += 2;
        iteratorAdvanced = true;
    }

    for (int i = 0; i < nbChannels; i++)
    {
        if (fr.matchSequence("DoubleLinearChannel {"))
        {
            fr += 2;

            osgAnimation::DoubleLinearChannel* channel = new osgAnimation::DoubleLinearChannel;

            if (Animation_readChannel(channel, fr))
                iteratorAdvanced = true;

            int nbKeys;
            if (fr.matchSequence("Keyframes %i {"))
            {
                fr[1].getInt(nbKeys);
                fr += 3;
                iteratorAdvanced = true;

                for (int k = 0; k < nbKeys; k++)
                {
                    double v;
                    float time;
                    if (fr.matchSequence("key %f %f"))
                    {
                        fr[1].getFloat(time);
                        fr[2].getFloat(v);
                        fr += 3;
                        channel->getOrCreateSampler()->getOrCreateKeyframeContainer()->push_back(osgAnimation::DoubleKeyframe(time, v));
                        iteratorAdvanced = true;
                    }
                }
                anim.addChannel(channel);

                if (fr.matchSequence("}")) // keyframes
                    fr += 1;
            }
            if (fr.matchSequence("}")) // channel
                fr += 1;
        }
        else if (fr.matchSequence("FloatLinearChannel {"))
        {
            fr += 2;

            osgAnimation::FloatLinearChannel* channel = new osgAnimation::FloatLinearChannel;

            if (Animation_readChannel(channel, fr))
                iteratorAdvanced = true;

            int nbKeys;
            if (fr.matchSequence("Keyframes %i {"))
            {
                fr[1].getInt(nbKeys);
                fr += 3;
                iteratorAdvanced = true;

                for (int k = 0; k < nbKeys; k++)
                {
                    float v;
                    float time;
                    if (fr.matchSequence("key %f %f"))
                    {
                        fr[1].getFloat(time);
                        fr[2].getFloat(v);
                        fr += 3;
                        channel->getOrCreateSampler()->getOrCreateKeyframeContainer()->push_back(osgAnimation::FloatKeyframe(time, v));
                        iteratorAdvanced = true;
                    }
                }
                anim.addChannel(channel);

                if (fr.matchSequence("}")) // keyframes
                    fr += 1;
            }
            if (fr.matchSequence("}")) // channel
                fr += 1;
        }
        else if (fr.matchSequence("Vec2LinearChannel {"))
        {
            fr += 2;

            osgAnimation::Vec2LinearChannel* channel = new osgAnimation::Vec2LinearChannel;

            if (Animation_readChannel(channel, fr))
                iteratorAdvanced = true;

            int nbKeys;
            if (fr.matchSequence("Keyframes %i {"))
            {
                fr[1].getInt(nbKeys);
                fr += 3;
                iteratorAdvanced = true;

                for (int k = 0; k < nbKeys; k++)
                {
                    osg::Vec2 v;
                    float time;
                    if (fr.matchSequence("key %f %f %f"))
                    {
                        fr[1].getFloat(time);
                        fr[2].getFloat(v[0]);
                        fr[3].getFloat(v[1]);
                        fr += 4;
                        channel->getOrCreateSampler()->getOrCreateKeyframeContainer()->push_back(osgAnimation::Vec2Keyframe(time, v));
                        iteratorAdvanced = true;
                    }
                }
                anim.addChannel(channel);

                if (fr.matchSequence("}")) // keyframes
                    fr += 1;
            }
            if (fr.matchSequence("}")) // channel
                fr += 1;
        }
        else if (fr.matchSequence("Vec3LinearChannel {"))
        {
            fr += 2;

            osgAnimation::Vec3LinearChannel* channel = new osgAnimation::Vec3LinearChannel;

            if (Animation_readChannel(channel, fr))
                iteratorAdvanced = true;

            int nbKeys;
            if (fr.matchSequence("Keyframes %i {"))
            {
                fr[1].getInt(nbKeys);
                fr += 3;
                iteratorAdvanced = true;

                for (int k = 0; k < nbKeys; k++)
                {
                    osg::Vec3 v;
                    float time;
                    if (fr.matchSequence("key %f %f %f %f"))
                    {
                        fr[1].getFloat(time);
                        fr[2].getFloat(v[0]);
                        fr[3].getFloat(v[1]);
                        fr[4].getFloat(v[2]);
                        fr += 5;
                        channel->getOrCreateSampler()->getOrCreateKeyframeContainer()->push_back(osgAnimation::Vec3Keyframe(time, v));
                        iteratorAdvanced = true;
                    }
                }
                anim.addChannel(channel);

                if (fr.matchSequence("}")) // keyframes
                    fr += 1;
            }
            if (fr.matchSequence("}")) // channel
                fr += 1;
        }
        else if (fr.matchSequence("Vec4LinearChannel {"))
        {
            fr += 2;

            osgAnimation::Vec4LinearChannel* channel = new osgAnimation::Vec4LinearChannel;

            if (Animation_readChannel(channel, fr))
                iteratorAdvanced = true;

            int nbKeys;
            if (fr.matchSequence("Keyframes %i {"))
            {
                fr[1].getInt(nbKeys);
                fr += 3;
                iteratorAdvanced = true;

                for (int k = 0; k < nbKeys; k++)
                {
                    osg::Vec4 v;
                    float time;
                    if (fr.matchSequence("key %f %f %f %f %f"))
                    {
                        fr[1].getFloat(time);
                        fr[2].getFloat(v[0]);
                        fr[3].getFloat(v[1]);
                        fr[4].getFloat(v[2]);
                        fr[5].getFloat(v[3]);
                        fr += 6;
                        channel->getOrCreateSampler()->getOrCreateKeyframeContainer()->push_back(osgAnimation::Vec4Keyframe(time, v));
                        iteratorAdvanced = true;
                    }
                }
                anim.addChannel(channel);

                if (fr.matchSequence("}")) // keyframes
                    fr += 1;
            }
            if (fr.matchSequence("}")) // channel
                fr += 1;
        }
        else if (fr.matchSequence("QuatSphericalLinearChannel {"))
        {
            fr += 2;

            osgAnimation::QuatSphericalLinearChannel* channel = new osgAnimation::QuatSphericalLinearChannel;

            if (Animation_readChannel(channel, fr))
                iteratorAdvanced = true;

            int nbKeys;
            if (fr.matchSequence("Keyframes %i {"))
            {
                fr[1].getInt(nbKeys);
                fr += 3;
                iteratorAdvanced = true;

                for (int k = 0; k < nbKeys; k++)
                {
                    osg::Quat q;
                    float time;
                    if (fr.matchSequence("key %f %f %f %f %f"))
                    {
                        fr[1].getFloat(time);
                        fr[2].getFloat(q[0]);
                        fr[3].getFloat(q[1]);
                        fr[4].getFloat(q[2]);
                        fr[5].getFloat(q[3]);
                        fr += 6;
                        channel->getOrCreateSampler()->getOrCreateKeyframeContainer()->push_back(osgAnimation::QuatKeyframe(time, q));
                        iteratorAdvanced = true;
                    }
                }
                anim.addChannel(channel);

                if (fr.matchSequence("}")) // keyframes
                    fr += 1;
            }
            if (fr.matchSequence("}")) // channel
                fr += 1;
        }
        // Deprecated
        // Reading of old channel info
        // Kept here for easy conversion of old .osg data to new format
        else if (fr.matchSequence("Channel {"))
        {
            fr += 2;

            std::string name = "unknown";
            if (fr.matchSequence("name %s"))
            {
                if (fr[1].getStr())
                    name = fr[1].getStr();
                fr += 2;
                iteratorAdvanced = true;
            }
            std::string target = "unknown";
            if (fr.matchSequence("target %s"))
            {
                if (fr[1].getStr())
                    target = fr[1].getStr();
                fr += 2;
                iteratorAdvanced = true;
            }

            std::string type = "unknown";
            int nbKeys;
            if (fr.matchSequence("Keyframes %s %i {"))
            {
                if (fr[1].getStr())
                    type = fr[1].getStr();
                fr[2].getInt(nbKeys);
                fr += 4;
                iteratorAdvanced = true;

                osgAnimation::Channel* channel = 0;
                if (type == "Quat")
                {
                    osgAnimation::QuatSphericalLinearChannel* c = new osgAnimation::QuatSphericalLinearChannel;
                    c->getOrCreateSampler()->getOrCreateKeyframeContainer();
                    channel = c;
                }
                else if (type == "Vec3")
                {
                    osgAnimation::Vec3LinearChannel* c = new osgAnimation::Vec3LinearChannel;
                    c->getOrCreateSampler()->getOrCreateKeyframeContainer();
                    channel = c;
                }

                if (channel)
                {
                    for (int k = 0; k < nbKeys; k++)
                    {
                        if (type == "Quat")
                        {
                            osg::Quat q;
                            float time;
                            fr.matchSequence("key %f %f %f %f %f");
                            fr[1].getFloat(time);
                            fr[2].getFloat(q[0]);
                            fr[3].getFloat(q[1]);
                            fr[4].getFloat(q[2]);
                            fr[5].getFloat(q[3]);
                            fr += 6;
                            osgAnimation::QuatSphericalLinearChannel* c = dynamic_cast<osgAnimation::QuatSphericalLinearChannel*>(channel);
                            c->getOrCreateSampler()->getOrCreateKeyframeContainer()->push_back(osgAnimation::QuatKeyframe(time, q));
                            iteratorAdvanced = true;
                        }
                        else if (type == "Vec3")
                        {
                            osg::Vec3 v;
                            float time;
                            fr.matchSequence("key %f %f %f %f");
                            fr[1].getFloat(time);
                            fr[2].getFloat(v[0]);
                            fr[3].getFloat(v[1]);
                            fr[4].getFloat(v[2]);
                            fr += 5;
                            osgAnimation::Vec3LinearChannel* c = dynamic_cast<osgAnimation::Vec3LinearChannel*>(channel);
                            c->getOrCreateSampler()->getOrCreateKeyframeContainer()->push_back(osgAnimation::Vec3Keyframe(time, v));
                            iteratorAdvanced = true;
                        }
                    }
                    channel->setName(name);
                    channel->setTargetName(target);
                    anim.addChannel(channel);
                }
                if (fr.matchSequence("}")) // keyframes
                    fr += 1;

                if (fr.matchSequence("}")) // channel
                    fr += 1;
            }
        }
    }
    return iteratorAdvanced;
}

// Helper method for writing channels
template <typename ChannelType, typename ContainerType>
void Animation_writeChannel(const std::string& channelString, ChannelType* pChannel, Output& fw)
{
    fw.indent() << channelString.c_str() << " {" << std::endl;
    fw.moveIn();
    fw.indent() << "name \"" << pChannel->getName() << "\"" << std::endl;
    fw.indent() << "target \"" << pChannel->getTargetName() << "\"" << std::endl;

//    fw.indent() << "weight " << pChannel->getWeight() << std::endl;

    ContainerType* kfc  = pChannel->getSamplerTyped()->getKeyframeContainerTyped();
    if (kfc)
    {
        fw.indent() << "Keyframes " << kfc->size() << " {" << std::endl;
        fw.moveIn();
        for (unsigned int k = 0; k < kfc->size(); k++)
        {
            fw.indent() << "key " << (*kfc)[k].getTime() << " " <<  (*kfc)[k].getValue() << std::endl;
        }
        fw.moveOut();
        fw.indent() << "}" << std::endl;
        fw.moveOut();
        fw.indent() << "}" << std::endl;
    }
}

bool Animation_writeLocalData(const Object& obj, Output& fw)
{
    const osgAnimation::Animation& anim = dynamic_cast<const osgAnimation::Animation&>(obj);

    switch (anim.getPlayMode())
    {
    case osgAnimation::Animation::ONCE:
        fw.indent() << "playmode ONCE" << std::endl;
        break;
    case osgAnimation::Animation::STAY:
        fw.indent() << "playmode STAY" << std::endl;
        break;
    case osgAnimation::Animation::LOOP:
        fw.indent() << "playmode LOOP" << std::endl;
        break;
    case osgAnimation::Animation::PPONG:
        fw.indent() << "playmode PPONG" << std::endl;
        break;
    default:
        break;
    }

    fw.indent() << "weight " << anim.getWeight() << std::endl;
    fw.indent() << "duration " << anim.getDuration() << std::endl;
    fw.indent() << "starttime " << anim.getStartTime() << std::endl;

    fw.indent() << "num_channels " << anim.getChannels().size()  << std::endl;
    for (unsigned int i = 0; i < anim.getChannels().size(); i++)
    {
        osgAnimation::Channel* pChannel = anim.getChannels()[i].get();

        osgAnimation::DoubleLinearChannel* pDlc = dynamic_cast<osgAnimation::DoubleLinearChannel*>(pChannel);
        if (pDlc)
        {
            Animation_writeChannel<osgAnimation::DoubleLinearChannel, osgAnimation::DoubleKeyframeContainer>("DoubleLinearChannel",  pDlc, fw);
            continue;
        }
        osgAnimation::FloatLinearChannel* pFlc = dynamic_cast<osgAnimation::FloatLinearChannel*>(pChannel);
        if (pFlc)
        {
            Animation_writeChannel<osgAnimation::FloatLinearChannel, osgAnimation::FloatKeyframeContainer>("FloatLinearChannel",  pFlc, fw);
            continue;
        }
        osgAnimation::Vec2LinearChannel* pV2lc = dynamic_cast<osgAnimation::Vec2LinearChannel*>(pChannel);
        if (pV2lc)
        {
            Animation_writeChannel<osgAnimation::Vec2LinearChannel, osgAnimation::Vec2KeyframeContainer>("Vec2LinearChannel",  pV2lc, fw);
            continue;
        }
        osgAnimation::Vec3LinearChannel* pV3lc = dynamic_cast<osgAnimation::Vec3LinearChannel*>(pChannel);
        if (pV3lc)
        {
            Animation_writeChannel<osgAnimation::Vec3LinearChannel, osgAnimation::Vec3KeyframeContainer>("Vec3LinearChannel",  pV3lc, fw);
            continue;
        }
        osgAnimation::Vec4LinearChannel* pV4lc = dynamic_cast<osgAnimation::Vec4LinearChannel*>(pChannel);
        if (pV4lc)
        {
            Animation_writeChannel<osgAnimation::Vec4LinearChannel, osgAnimation::Vec4KeyframeContainer>("Vec4LinearChannel",  pV4lc, fw);
            continue;
        }
        osgAnimation::QuatSphericalLinearChannel* pQslc = dynamic_cast<osgAnimation::QuatSphericalLinearChannel*>(pChannel);
        if (pQslc)
        {
            Animation_writeChannel<osgAnimation::QuatSphericalLinearChannel, osgAnimation::QuatKeyframeContainer>("QuatSphericalLinearChannel",  pQslc, fw);
            continue;
        }
        osgAnimation::FloatCubicBezierChannel* pFcbc = dynamic_cast<osgAnimation::FloatCubicBezierChannel*>(pChannel);
        if (pFcbc)
        {
            Animation_writeChannel<osgAnimation::FloatCubicBezierChannel, osgAnimation::FloatCubicBezierKeyframeContainer>("FloatCubicBezierChannel",  pFcbc, fw);
            continue;
        }
        osgAnimation::DoubleCubicBezierChannel* pDcbc = dynamic_cast<osgAnimation::DoubleCubicBezierChannel*>(pChannel);
        if (pDcbc)
        {
            Animation_writeChannel<osgAnimation::DoubleCubicBezierChannel, osgAnimation::DoubleCubicBezierKeyframeContainer>("DoubleCubicBezierChannel",  pDcbc, fw);
            continue;
        }
        osgAnimation::Vec2CubicBezierChannel* pV2cbc = dynamic_cast<osgAnimation::Vec2CubicBezierChannel*>(pChannel);
        if (pV2cbc)
        {
            Animation_writeChannel<osgAnimation::Vec2CubicBezierChannel, osgAnimation::Vec2CubicBezierKeyframeContainer>("Vec2CubicBezierChannel",  pV2cbc, fw);
            continue;
        }
        osgAnimation::Vec3CubicBezierChannel* pV3cbc = dynamic_cast<osgAnimation::Vec3CubicBezierChannel*>(pChannel);
        if (pV3cbc)
        {
            Animation_writeChannel<osgAnimation::Vec3CubicBezierChannel, osgAnimation::Vec3CubicBezierKeyframeContainer>("Vec3CubicBezierChannel",  pV3cbc, fw);
            continue;
        }
        osgAnimation::Vec4CubicBezierChannel* pV4cbc = dynamic_cast<osgAnimation::Vec4CubicBezierChannel*>(pChannel);
        if (pV4cbc)
        {
            Animation_writeChannel<osgAnimation::Vec4CubicBezierChannel, osgAnimation::Vec4CubicBezierKeyframeContainer>("Vec4CubicBezierChannel",  pV4cbc, fw);
            continue;
        }
    }
    return true;
}
RegisterDotOsgWrapperProxy g_atkAnimationProxy
(
    new osgAnimation::Animation,
    "osgAnimation::Animation",
    "Object osgAnimation::Animation",
    &Animation_readLocalData,
    &Animation_writeLocalData
    );



bool AnimationManagerBase_readLocalData(osgAnimation::AnimationManagerBase& manager, Input& fr)
{
    int nbAnims = 0;
    bool iteratorAdvanced = false;

    if (fr.matchSequence("num_animations %i"))
    {
        fr[1].getInt(nbAnims);
        fr += 2;
        iteratorAdvanced = true;
    }

    for (int i = 0; i < nbAnims; i++)
    {
        Object* o = fr.readObject();
        osgAnimation::Animation* a = dynamic_cast<osgAnimation::Animation*>(o);
        if (a)
        {
            manager.registerAnimation(a);
            iteratorAdvanced = true;
        }
        else
            osg::notify(osg::WARN)<<"Warning: can't read an animation object"<< std::endl;
    }

    return iteratorAdvanced;
}


bool BasicAnimationManager_readLocalData(Object& obj, Input& fr)
{
    osgAnimation::BasicAnimationManager& manager = dynamic_cast<osgAnimation::BasicAnimationManager&>(obj);
    return AnimationManagerBase_readLocalData(manager, fr);
}

bool TimelineAnimationManager_readLocalData(Object& obj, Input& fr)
{
    osgAnimation::TimelineAnimationManager& manager = dynamic_cast<osgAnimation::TimelineAnimationManager&>(obj);
    return AnimationManagerBase_readLocalData(manager, fr);
}


bool AnimationManagerBase_writeLocalData(const osgAnimation::AnimationManagerBase& manager, Output& fw)
{
    const osgAnimation::AnimationList& animList = manager.getAnimationList();

    fw.indent() << "num_animations " << animList.size()  << std::endl;
    for (osgAnimation::AnimationList::const_iterator it = animList.begin(); it != animList.end(); ++it)
    {
        if (!fw.writeObject(**it))
            osg::notify(osg::WARN)<<"Warning: can't write an animation object"<< std::endl;
    }
    return true;
}

bool BasicAnimationManager_writeLocalData(const Object& obj, Output& fw)
{
    const osgAnimation::BasicAnimationManager& manager = dynamic_cast<const osgAnimation::BasicAnimationManager&>(obj);
    return AnimationManagerBase_writeLocalData(manager, fw);
}

bool TimelineAnimationManager_writeLocalData(const Object& obj, Output& fw)
{
    const osgAnimation::TimelineAnimationManager& manager = dynamic_cast<const osgAnimation::TimelineAnimationManager&>(obj);
    return AnimationManagerBase_writeLocalData(manager, fw);
}


RegisterDotOsgWrapperProxy g_BasicAnimationManagerProxy
(
    new osgAnimation::BasicAnimationManager,
    "osgAnimation::BasicAnimationManager",
    "Object NodeCallback osgAnimation::BasicAnimationManager",
    &BasicAnimationManager_readLocalData,
    &BasicAnimationManager_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

RegisterDotOsgWrapperProxy g_TimelineAnimationManagerProxy
(
    new osgAnimation::TimelineAnimationManager,
    "osgAnimation::TimelineAnimationManager",
    "Object NodeCallback osgAnimation::TimelineAnimationManager",
    &TimelineAnimationManager_readLocalData,
    &TimelineAnimationManager_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);


bool RigGeometry_readLocalData(Object& obj, Input& fr)
{
    osgAnimation::RigGeometry& geom = dynamic_cast<osgAnimation::RigGeometry&>(obj);
    osg::ref_ptr<osgAnimation::VertexInfluenceMap> vmap = new osgAnimation::VertexInfluenceMap;

    int nbGroups = 0;
    bool iteratorAdvanced = false;
    if (fr.matchSequence("num_influences %i"))
    {
        fr[1].getInt(nbGroups);
        fr += 2;
        iteratorAdvanced = true;
    }

    for (int i = 0; i < nbGroups; i++)
    {
        int nbVertexes = 0;
        std::string name;
        if (fr.matchSequence("osgAnimation::VertexInfluence %s %i {"))
        {
            name = fr[1].getStr();
            fr[2].getInt(nbVertexes);
            fr += 4;
            iteratorAdvanced = true;
        }

        osgAnimation::VertexInfluence vi;
        vi.setName(name);
        vi.reserve(nbVertexes);
        for (int j = 0; j < nbVertexes; j++)
        {
            int index = -1;
            float weight = 1;
            if (fr.matchSequence("%i %f"))
            {
                fr[0].getInt(index);
                fr[1].getFloat(weight);
                fr += 2;
                iteratorAdvanced = true;
            }
            vi.push_back(osgAnimation::VertexIndexWeight(index, weight));
        }
        if (fr.matchSequence("}"))
        {
            fr+=1;
        }
        (*vmap)[name] = vi;
    }
    if (!vmap->empty())
        geom.setInfluenceMap(vmap.get());

    if (fr.matchSequence("Geometry {"))
    {
        osg::Geometry* source = dynamic_cast<osg::Geometry*>(fr.readObject());
        geom.setSourceGeometry(source);
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool RigGeometry_writeLocalData(const Object& obj, Output& fw)
{
    const osgAnimation::RigGeometry& geom = dynamic_cast<const osgAnimation::RigGeometry&>(obj);
    const osgAnimation::VertexInfluenceMap* vm = geom.getInfluenceMap();
    if (!vm)
        return true;
    fw.indent() << "num_influences "  << vm->size() << std::endl;
    fw.moveIn();
    for (osgAnimation::VertexInfluenceMap::const_iterator it = vm->begin(); it != vm->end(); ++it)
    {
        std::string name = it->first;
        if (name.empty())
            name = "Empty";
        fw.indent() << "osgAnimation::VertexInfluence \""  << name << "\" " << it->second.size() << " {" << std::endl;
        fw.moveIn();
        const osgAnimation::VertexInfluence& vi = it->second;
        for (osgAnimation::VertexInfluence::const_iterator itv = vi.begin(); itv != vi.end(); itv++)
        {
            fw.indent() << itv->first << " " << itv->second << std::endl;
        }
        fw.moveOut();
        fw.indent() << "}" << std::endl;
    }
    fw.moveOut();

    fw.writeObject(*geom.getSourceGeometry());
    return true;
}

RegisterDotOsgWrapperProxy g_atkRigGeometryProxy
(
    new osgAnimation::RigGeometry,
    "osgAnimation::RigGeometry",
    "Object osgAnimation::RigGeometry Drawable Geometry",
    &RigGeometry_readLocalData,
    &RigGeometry_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
    );


bool MorphGeometry_readLocalData(Object& obj, Input& fr)
{
    osgAnimation::MorphGeometry& geom = dynamic_cast<osgAnimation::MorphGeometry&>(obj);

    bool iteratorAdvanced = false;

    if (fr[0].matchWord("method"))
    {
        if (fr[1].matchWord("NORMALIZED"))
        {
            geom.setMethod(osgAnimation::MorphGeometry::NORMALIZED);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if  (fr[1].matchWord("RELATIVE"))
        {
            geom.setMethod(osgAnimation::MorphGeometry::RELATIVE);
            fr+=2;
            iteratorAdvanced = true;
        }
    }

    if (fr[0].matchWord("morphNormals"))
    {
        if (fr[1].matchWord("TRUE"))
        {
            geom.setMorphNormals(true);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("FALSE"))
        {
            geom.setMorphNormals(false);
            fr+=2;
            iteratorAdvanced = true;
        }
    }

    int num_morphTargets = 0;
    if (fr.matchSequence("num_morphTargets %i"))
    {
        fr[1].getInt(num_morphTargets);
        fr += 2;
        iteratorAdvanced = true;
    }

    for (int i = 0; i < num_morphTargets; i++)
    {
        if (fr.matchSequence("MorphTarget {"))
        {
            int entry = fr[0].getNoNestedBrackets();
            fr += 2;
            iteratorAdvanced = true;

            while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
            {

                float weight = 1.0;
                if (fr.matchSequence("weight %f"))
                {
                    fr[1].getFloat(weight);
                    fr += 2;
                }
                osg::Drawable* drawable = NULL;
                drawable = fr.readDrawable();
                osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(drawable);
                if (geometry)
                    geom.addMorphTarget(geometry, weight);
            }
            if (fr.matchSequence("}"))
                fr += 1;
        }
    }

    return iteratorAdvanced;
}

bool MorphGeometry_writeLocalData(const Object& obj, Output& fw)
{
    const osgAnimation::MorphGeometry& geom = dynamic_cast<const osgAnimation::MorphGeometry&>(obj);

    switch(geom.getMethod())
    {
        case(osgAnimation::MorphGeometry::NORMALIZED): fw.indent() << "method NORMALIZED"<<std::endl; break;
    case(osgAnimation::MorphGeometry::RELATIVE): fw.indent() << "method RELATIVE"<<std::endl; break;
    }

    fw.indent() << "morphNormals ";
    if (geom.getMorphNormals())
        fw << "TRUE" << std::endl;
    else
        fw << "FALSE" << std::endl;

    const osgAnimation::MorphGeometry::MorphTargetList& morphTargets = geom.getMorphTargetList();
    fw.indent() << "num_morphTargets " << morphTargets.size() << std::endl;
    for (unsigned int i = 0; i < morphTargets.size(); i++)
    {
        fw.indent() << "MorphTarget {" << std::endl;
        fw.moveIn();
        fw.indent() << "weight " << morphTargets[i].getWeight() <<std::endl;
        fw.writeObject(*morphTargets[i].getGeometry());
        fw.moveOut();
        fw.indent() << "}" << std::endl;
    }
    return true;
}

RegisterDotOsgWrapperProxy g_osgAnimationMorphGeometryProxy
(
    new osgAnimation::MorphGeometry,
    "osgAnimation::MorphGeometry",
    "Object Drawable osgAnimation::MorphGeometry Geometry",
    &MorphGeometry_readLocalData,
    &MorphGeometry_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
    );





bool UpdateBone_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;
    return iteratorAdvanced;
}

bool UpdateBone_writeLocalData(const Object& obj, Output& fw)
{
    return true;
}

RegisterDotOsgWrapperProxy g_UpdateBoneProxy
(
    new osgAnimation::UpdateBone,
    "osgAnimation::UpdateBone",
    "Object NodeCallback osgAnimation::UpdateMatrixTransform osgAnimation::UpdateBone",
    &UpdateBone_readLocalData,
    &UpdateBone_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
    );



bool UpdateSkeleton_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;
    return iteratorAdvanced;
}

bool UpdateSkeleton_writeLocalData(const Object& obj, Output& fw)
{
    return true;
}

RegisterDotOsgWrapperProxy g_UpdateSkeletonProxy
(
    new osgAnimation::Skeleton::UpdateSkeleton,
    "osgAnimation::UpdateSkeleton",
    "Object NodeCallback osgAnimation::UpdateSkeleton",
    &UpdateSkeleton_readLocalData,
    &UpdateSkeleton_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
    );


bool UpdateMorph_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;
    return iteratorAdvanced;
}

bool UpdateMorph_writeLocalData(const Object& obj, Output& fw)
{
    return true;
}

RegisterDotOsgWrapperProxy g_UpdateMorphProxy
(
 new osgAnimation::UpdateMorph,
    "osgAnimation::UpdateMorph",
    "Object NodeCallback osgAnimation::UpdateMorph",
    &UpdateMorph_readLocalData,
    &UpdateMorph_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

