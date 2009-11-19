#ifndef FBXRMESH_H
#define FBXRMESH_H

#include <fbxfilesdk/fbxfilesdk_def.h>
#include <osgDB/ReaderWriter>
#include <osg/Material>

osgDB::ReaderWriter::ReadResult readFbxMesh(
    FBXFILESDK_NAMESPACE::KFbxNode* pNode,
    osg::ref_ptr<osgAnimation::AnimationManagerBase>& pAnimationManager,
    const std::vector<osg::ref_ptr<osg::Material>>&,
    const std::vector<osg::ref_ptr<osg::Texture>>&);

#endif
