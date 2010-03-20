#ifndef FBXRNODE_H
#define FBXRNODE_H

#include "fbxMaterialToOsgStateSet.h"
namespace osgAnimation
{
    class AnimationManagerBase;
    class RigGeometry;
}

typedef std::map<std::pair<KFbxNode*, osgAnimation::RigGeometry*>, osg::Matrix> BindMatrixMap;

osgAnimation::Skeleton* getSkeleton(KFbxNode*,
    const std::set<const KFbxNode*>& fbxSkeletons,
    std::map<KFbxNode*, osgAnimation::Skeleton*>&);

osgDB::ReaderWriter::ReadResult readFbxNode(
    FBXFILESDK_NAMESPACE::KFbxSdkManager& pSdkManager,
    FBXFILESDK_NAMESPACE::KFbxNode* pNode,
    osg::ref_ptr<osgAnimation::AnimationManagerBase>& pAnimationManager,
    bool& bIsBone,
    int& nLightCount,
    FbxMaterialToOsgStateSet& fbxMaterialToOsgStateSet,
    std::map<KFbxNode*, osg::Node*>& nodeMap,
    BindMatrixMap& boneBindMatrices,
    const std::set<const KFbxNode*>& fbxSkeletons,
    std::map<KFbxNode*, osgAnimation::Skeleton*>& skeletonMap,
    const osgDB::ReaderWriter::Options* options = NULL);

#endif
