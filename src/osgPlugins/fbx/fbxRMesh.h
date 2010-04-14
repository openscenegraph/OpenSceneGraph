#ifndef FBXRMESH_H
#define FBXRMESH_H

#include <fbxfilesdk/fbxfilesdk_def.h>
#include <osgDB/ReaderWriter>
#include <osg/Material>
#include "fbxRNode.h"
osgDB::ReaderWriter::ReadResult readFbxMesh(
    KFbxSdkManager& pSdkManager,
    KFbxScene& fbxScene,
    KFbxNode* pNode,
    osg::ref_ptr<osgAnimation::AnimationManagerBase>& pAnimationManager,
    std::vector<StateSetContent>&,
    BindMatrixMap& boneBindMatrices,
    const std::set<const KFbxNode*>& fbxSkeletons,
    std::map<KFbxNode*, osgAnimation::Skeleton*>& skeletonMap,
    const osgDB::Options&);

#endif
