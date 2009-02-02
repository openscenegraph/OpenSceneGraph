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


#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/ReaderWriter>

#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/TimelineAnimationManager>
#include <osgAnimation/VertexInfluence>
#include <osgAnimation/Animation>
#include <osgAnimation/Bone>
#include <osgAnimation/Skeleton>
#include <osgAnimation/RigGeometry>
#include <osgAnimation/UpdateCallback>

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
    }

    osg::Vec3d pos(0,0,0);
    if (fr.matchSequence("bindPosition %f %f %f")) 
    {
        fr[1].getFloat(pos[0]);
        fr[2].getFloat(pos[1]);
        fr[3].getFloat(pos[2]);
        
        fr += 4;
        iteratorAdvanced = true;
    }

    osg::Vec3d scale(1,1,1);
    if (fr.matchSequence("bindScale %f %f %f")) 
    {
        fr[1].getFloat(scale[0]);
        fr[2].getFloat(scale[1]);
        fr[3].getFloat(scale[2]);
        
        fr += 4;
        iteratorAdvanced = true;
    }

    bone.setBindMatrixInBoneSpace( osg::Matrix(att) * osg::Matrix::translate(pos));
    return iteratorAdvanced;
}

bool Bone_writeLocalData(const Object& obj, Output& fw) 
{
    const osgAnimation::Bone& bone = dynamic_cast<const osgAnimation::Bone&>(obj);
    osg::Vec3 t;
    osg::Quat r;
    osg::Vec3 s;
    osg::Quat rs;
    bone.getBindMatrixInBoneSpace().decompose(t,r,s,rs);
    fw.indent() << "bindQuaternion "  << r << std::endl;
    fw.indent() << "bindPosition "  << t << std::endl;
    fw.indent() << "bindScale "  << s << std::endl;
    return true;
}

RegisterDotOsgWrapperProxy g_atkBoneProxy
(
    new osgAnimation::Bone,
    "osgAnimation::Bone",
    "Object Node Transform osgAnimation::Bone Group",
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
RegisterDotOsgWrapperProxy g_atkRootSkeletonProxy
(
    new osgAnimation::Skeleton,
    "osgAnimation::Skeleton",
    "Object Node  Transform osgAnimation::Bone osgAnimation::Skeleton Group",
    &Skeleton_readLocalData,
    &Skeleton_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
    );




bool Animation_readLocalData(Object& obj, Input& fr) 
{
    osgAnimation::Animation& anim = dynamic_cast<osgAnimation::Animation&>(obj);
    bool iteratorAdvanced = false;
    int nbChannels = 0;
    if (fr.matchSequence("num_channels %i")) 
    {
        fr[1].getInt(nbChannels);
        fr += 2;
        iteratorAdvanced = true;
    }

    for (int i = 0; i < nbChannels; i++) 
    {
        if (fr.matchSequence("Channel {")) 
        {
            fr += 2;

            std::string name = "unknown";
            if (fr.matchSequence("name %s")) 
            {
                name = fr[1].getStr();
                fr += 2;
                iteratorAdvanced = true;
            }
            std::string target = "unknown";
            if (fr.matchSequence("target %s")) 
            {
                target = fr[1].getStr();
                fr += 2;
                iteratorAdvanced = true;
            }

            std::string type;
            int nbKeys;
            if (fr.matchSequence("Keyframes %s %i {")) 
            {
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
                    channel = new osgAnimation::Vec3LinearChannel;
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

bool Animation_writeLocalData(const Object& obj, Output& fw)
{
    const osgAnimation::Animation& anim = dynamic_cast<const osgAnimation::Animation&>(obj);

    fw.indent() << "num_channels " << anim.getChannels().size()  << std::endl;
    for (unsigned int i = 0; i < anim.getChannels().size(); i++) 
    {
        fw.indent() << "Channel {" << std::endl;
        fw.moveIn();
        fw.indent() << "name \"" << anim.getChannels()[i]->getName() << "\"" << std::endl;
        fw.indent() << "target \"" << anim.getChannels()[i]->getTargetName() << "\"" << std::endl;

        std::string type = "unknown";
        if (anim.getChannels()[i]->getName() == std::string("quaternion")) 
        {
            type = "Quat";
        }
        else if (anim.getChannels()[i]->getName() == std::string("rotation")) 
        {
            type = "Quat";
        }
        else if (anim.getChannels()[i]->getName() == std::string("euler")) 
        {
            type = "Vec3";
        }
        else if (anim.getChannels()[i]->getName() == std::string("scale")) 
        {
            type = "Vec3";
        }
        else if (anim.getChannels()[i]->getName() == std::string("position")) 
        {
            type = "Vec3";
        }

        osgAnimation::KeyframeContainer* kf = anim.getChannels()[i]->getSampler()->getKeyframeContainer();
        fw.indent() << "Keyframes \"" << type << "\" " << kf->size() << " {" << std::endl;
        fw.moveIn();
        for (unsigned int k = 0; k < kf->size(); k++) 
        {
            if (type == "Vec3") 
            {
                osgAnimation::Vec3KeyframeContainer* kk = dynamic_cast<osgAnimation::Vec3KeyframeContainer*>(kf);
                fw.indent() << "key " << (*kk)[k].getTime() << " " <<  (*kk)[k].getValue() << std::endl;
            }
            else if ( type == "Quat") 
            {
                osgAnimation::QuatKeyframeContainer* kk = dynamic_cast<osgAnimation::QuatKeyframeContainer*>(kf);
                fw.indent() << "key " << (*kk)[k].getTime() << " " <<  (*kk)[k].getValue() << std::endl;
            }
        }
        fw.moveOut();
        fw.indent() << "}" << std::endl;
        fw.moveOut();
        fw.indent() << "}" << std::endl;
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
    for (osgAnimation::AnimationList::const_iterator it = animList.begin(); it != animList.end(); it++)
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
    for (osgAnimation::VertexInfluenceMap::const_iterator it = vm->begin(); it != vm->end(); it++) 
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
    return true;
}

RegisterDotOsgWrapperProxy g_atkRigGeometryProxy
(
    new osgAnimation::RigGeometry,
    "osgAnimation::RigGeometry",
    "Object Drawable osgAnimation::RigGeometry Geometry",
    &RigGeometry_readLocalData,
    &RigGeometry_writeLocalData,
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

RegisterDotOsgWrapperProxy g_atkUpdateBoneProxy
(
    new osgAnimation::Bone::UpdateBone,
    "osgAnimation::UpdateBone",
    "Object NodeCallback osgAnimation::UpdateBone",
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

RegisterDotOsgWrapperProxy g_atkUpdateSkeletonProxy
(
    new osgAnimation::Skeleton::UpdateSkeleton,
    "osgAnimation::UpdateSkeleton",
    "Object NodeCallback osgAnimation::UpdateSkeleton",
    &UpdateSkeleton_readLocalData,
    &UpdateSkeleton_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
    );



bool UpdateTransform_readLocalData(Object& obj, Input& fr) 
{
    bool iteratorAdvanced = false;
    return iteratorAdvanced;
}

bool UpdateTransform_writeLocalData(const Object& obj, Output& fw)
{
    return true;
}

RegisterDotOsgWrapperProxy g_atkUpdateTransformProxy
(
    new osgAnimation::UpdateTransform,
    "osgAnimation::UpdateTransform",
    "Object NodeCallback osgAnimation::UpdateTransform",
    &UpdateTransform_readLocalData,
    &UpdateTransform_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

