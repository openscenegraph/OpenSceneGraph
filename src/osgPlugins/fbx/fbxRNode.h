#ifndef FBXRNODE_H
#define FBXRNODE_H

#include "fbxMaterialToOsgStateSet.h"
namespace osgAnimation
{
    class AnimationManagerBase;
}

osgAnimation::Skeleton* getSkeleton(KFbxNode*, std::map<KFbxNode*, osgAnimation::Skeleton*>&);

osgDB::ReaderWriter::ReadResult readFbxNode(
    FBXFILESDK_NAMESPACE::KFbxSdkManager& pSdkManager,
    FBXFILESDK_NAMESPACE::KFbxNode* pNode,
    osg::ref_ptr<osgAnimation::AnimationManagerBase>& pAnimationManager,
    bool& bIsBone,
    int& nLightCount,
    FbxMaterialToOsgStateSet& fbxMaterialToOsgStateSet,
	std::map<KFbxNode*, osg::Node*>& nodeMap,
	std::map<KFbxNode*, osg::Matrix>& boneBindMatrices,
	std::map<KFbxNode*, osgAnimation::Skeleton*>& skeletonMap,
    const osgDB::Options* options = NULL);

#endif
