#ifndef FBXRNODE_H
#define FBXRNODE_H

#include "fbxMaterialToOsgStateSet.h"
namespace osgAnimation
{
    class AnimationManagerBase;
}

osgDB::ReaderWriter::ReadResult readFbxNode(
    FBXFILESDK_NAMESPACE::KFbxSdkManager& pSdkManager,
    FBXFILESDK_NAMESPACE::KFbxNode* pNode,
    osg::ref_ptr<osgAnimation::AnimationManagerBase>& pAnimationManager,
    bool& bNeedSkeleton,
    int& nLightCount,
    FbxMaterialToOsgStateSet& fbxMaterialToOsgStateSet,
    const osgDB::Options* options = NULL);

#endif
