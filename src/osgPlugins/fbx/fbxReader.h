#ifndef FBXRANIMATION_H
#define FBXRANIMATION_H

#include <fbxfilesdk/fbxfilesdk_def.h>
#include <osgDB/ReaderWriter>
#include <osgAnimation/BasicAnimationManager>
#include "fbxMaterialToOsgStateSet.h"

namespace osgAnimation
{
    class AnimationManagerBase;
    class RigGeometry;
    class Skeleton;
}

typedef std::map<std::pair<KFbxNode*, osgAnimation::RigGeometry*>, osg::Matrix> BindMatrixMap;

class OsgFbxReader
{
public:
    KFbxSdkManager& pSdkManager;
    KFbxScene& fbxScene;
    osg::ref_ptr<osgAnimation::AnimationManagerBase> pAnimationManager;
    FbxMaterialToOsgStateSet& fbxMaterialToOsgStateSet;
    std::map<KFbxNode*, osg::Node*> nodeMap;
    BindMatrixMap boneBindMatrices;
    const std::set<const KFbxNode*>& fbxSkeletons;
    std::map<KFbxNode*, osgAnimation::Skeleton*> skeletonMap;
    const osgDB::Options& options;
    bool lightmapTextures, tessellatePolygons;

    enum AuthoringTool
    {
        UNKNOWN,
        OPENSCENEGRAPH,
        AUTODESK_3DSTUDIO_MAX
    } authoringTool;

    OsgFbxReader(
        KFbxSdkManager& pSdkManager1,
        KFbxScene& fbxScene1,
        FbxMaterialToOsgStateSet& fbxMaterialToOsgStateSet1,
        const std::set<const KFbxNode*>& fbxSkeletons1,
        const osgDB::Options& options1,
        AuthoringTool authoringTool1,
        bool lightmapTextures1,
        bool tessellatePolygons1)
        : pSdkManager(pSdkManager1),
        fbxScene(fbxScene1),
        fbxMaterialToOsgStateSet(fbxMaterialToOsgStateSet1),
        fbxSkeletons(fbxSkeletons1),
        options(options1),
        lightmapTextures(lightmapTextures1),
        tessellatePolygons(tessellatePolygons1),
        authoringTool(authoringTool1)
    {}

    osgDB::ReaderWriter::ReadResult readFbxNode(
        KFbxNode*, bool& bIsBone, int& nLightCount);

    std::string readFbxAnimation(
        KFbxNode*, const char* targetName);

    osgDB::ReaderWriter::ReadResult readFbxCamera(
        KFbxNode* pNode);

    osgDB::ReaderWriter::ReadResult readFbxLight(
        KFbxNode* pNode, int& nLightCount);

    osgDB::ReaderWriter::ReadResult readMesh(
        KFbxNode* pNode, KFbxMesh* fbxMesh,
        std::vector<StateSetContent>& stateSetList,
        const char* szName);

    osgDB::ReaderWriter::ReadResult readFbxMesh(
        KFbxNode* pNode,
        std::vector<StateSetContent>&);
};

osgAnimation::Skeleton* getSkeleton(KFbxNode*,
    const std::set<const KFbxNode*>& fbxSkeletons,
    std::map<KFbxNode*, osgAnimation::Skeleton*>&);

#endif
