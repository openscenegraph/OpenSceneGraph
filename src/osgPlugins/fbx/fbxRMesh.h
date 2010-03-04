#ifndef FBXRMESH_H
#define FBXRMESH_H

#include <fbxfilesdk/fbxfilesdk_def.h>
#include <osgDB/ReaderWriter>
#include <osg/Material>
#include "fbxRNode.h"
osgDB::ReaderWriter::ReadResult readFbxMesh(
    FBXFILESDK_NAMESPACE::KFbxSdkManager& pSdkManager,
    FBXFILESDK_NAMESPACE::KFbxNode* pNode,
    osg::ref_ptr<osgAnimation::AnimationManagerBase>& pAnimationManager,
    std::vector<StateSetContent>&,
    BindMatrixMap& boneBindMatrices,
    std::map<KFbxNode*, osgAnimation::Skeleton*>& skeletonMap);

#endif
