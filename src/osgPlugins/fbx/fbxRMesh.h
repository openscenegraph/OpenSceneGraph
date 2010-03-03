#ifndef FBXRMESH_H
#define FBXRMESH_H

#include <fbxfilesdk/fbxfilesdk_def.h>
#include <osgDB/ReaderWriter>
#include <osg/Material>
#include "fbxMaterialToOsgStateSet.h"
osgDB::ReaderWriter::ReadResult readFbxMesh(
    FBXFILESDK_NAMESPACE::KFbxSdkManager& pSdkManager,
    FBXFILESDK_NAMESPACE::KFbxNode* pNode,
    osg::ref_ptr<osgAnimation::AnimationManagerBase>& pAnimationManager,
    std::vector<StateSetContent>&,
    std::map<KFbxNode*, osg::Matrix>& boneBindMatrices,
    std::map<KFbxNode*, osgAnimation::Skeleton*>& skeletonMap);

#endif
