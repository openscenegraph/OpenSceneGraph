#ifndef FBXRNODE_H
#define FBXRNODE_H

namespace osgAnimation
{
    class AnimationManagerBase;
}

osgDB::ReaderWriter::ReadResult readFbxNode(
    FBXFILESDK_NAMESPACE::KFbxSdkManager& pSdkManager,
    FBXFILESDK_NAMESPACE::KFbxNode* pNode,
    osg::ref_ptr<osgAnimation::AnimationManagerBase>& pAnimationManager,
    const std::string& dir,
    bool& bNeedSkeleton,
    int& nLightCount);

#endif
