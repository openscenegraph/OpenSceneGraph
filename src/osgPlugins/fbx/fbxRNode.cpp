#include <cassert>
#include <memory>
#include <sstream>

#include <osg/io_utils>
#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/Material>
#include <osg/Texture2D>

#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>

#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/Bone>
#include <osgAnimation/Skeleton>
#include <osgAnimation/StackedMatrixElement>
#include <osgAnimation/StackedQuaternionElement>
#include <osgAnimation/StackedScaleElement>
#include <osgAnimation/StackedTranslateElement>
#include <osgAnimation/UpdateBone>

#if defined(_MSC_VER)
    #pragma warning( disable : 4505 )
#endif
#include <fbxsdk.h>

#include "fbxRAnimation.h"
#include "fbxRCamera.h"
#include "fbxRLight.h"
#include "fbxRMesh.h"
#include "fbxRNode.h"
#include "fbxMaterialToOsgStateSet.h"

osg::Quat makeQuat(const fbxDouble3& degrees, ERotationOrder fbxRotOrder)
{
    double radiansX = osg::DegreesToRadians(degrees[0]);
    double radiansY = osg::DegreesToRadians(degrees[1]);
    double radiansZ = osg::DegreesToRadians(degrees[2]);

    switch (fbxRotOrder)
    {
    case eEULER_XYZ:
        return osg::Quat(
            radiansX, osg::Vec3d(1,0,0),
            radiansY, osg::Vec3d(0,1,0),
            radiansZ, osg::Vec3d(0,0,1));
    case eEULER_XZY:
        return osg::Quat(
            radiansX, osg::Vec3d(1,0,0),
            radiansZ, osg::Vec3d(0,0,1),
            radiansY, osg::Vec3d(0,1,0));
    case eEULER_YZX:
        return osg::Quat(
            radiansY, osg::Vec3d(0,1,0),
            radiansZ, osg::Vec3d(0,0,1),
            radiansX, osg::Vec3d(1,0,0));
    case eEULER_YXZ:
        return osg::Quat(
            radiansY, osg::Vec3d(0,1,0),
            radiansX, osg::Vec3d(1,0,0),
            radiansZ, osg::Vec3d(0,0,1));
    case eEULER_ZXY:
        return osg::Quat(
            radiansZ, osg::Vec3d(0,0,1),
            radiansX, osg::Vec3d(1,0,0),
            radiansY, osg::Vec3d(0,1,0));
    case eEULER_ZYX:
        return osg::Quat(
            radiansZ, osg::Vec3d(0,0,1),
            radiansY, osg::Vec3d(0,1,0),
            radiansX, osg::Vec3d(1,0,0));
    case eSPHERIC_XYZ:
        {
            //I don't know what eSPHERIC_XYZ means, so this is a complete guess.
            osg::Quat quat;
            quat.makeRotate(osg::Vec3d(1.0, 0.0, 0.0), osg::Vec3d(degrees[0], degrees[1], degrees[2]));
            return quat;
        }
    default:
        osg::notify(osg::WARN) << "Invalid FBX rotation mode." << std::endl;
        return osg::Quat();
    }
}

void makeLocalMatrix(const KFbxNode* pNode, osg::Matrix& m)
{
    /*From http://area.autodesk.com/forum/autodesk-fbx/fbx-sdk/the-makeup-of-the-local-matrix-of-an-kfbxnode/

    Local Matrix = LclTranslation * RotationOffset * RotationPivot *
      PreRotation * LclRotation * PostRotation * RotationPivotInverse *
      ScalingOffset * ScalingPivot * LclScaling * ScalingPivotInverse

    LocalTranslation : translate (xform -query -translation)
    RotationOffset: translation compensates for the change in the rotate pivot point (xform -q -rotateTranslation)
    RotationPivot: current rotate pivot position (xform -q -rotatePivot)
    PreRotation : joint orientation(pre rotation)
    LocalRotation: rotate transform (xform -q -rotation & xform -q -rotateOrder)
    PostRotation : rotate axis (xform -q -rotateAxis)
    RotationPivotInverse: inverse of RotationPivot
    ScalingOffset: translation compensates for the change in the scale pivot point (xform -q -scaleTranslation)
    ScalingPivot: current scale pivot position (xform -q -scalePivot)
    LocalScaling: scale transform (xform -q -scale)
    ScalingPivotInverse: inverse of ScalingPivot
    */

    // When this flag is set to false, the RotationOrder, the Pre/Post rotation
    // values and the rotation limits should be ignored.
    bool rotationActive = pNode->RotationActive.Get();

    ERotationOrder fbxRotOrder = rotationActive ? pNode->RotationOrder.Get() : eEULER_XYZ;

    fbxDouble3 fbxLclPos = pNode->LclTranslation.Get();
    fbxDouble3 fbxRotOff = pNode->RotationOffset.Get();
    fbxDouble3 fbxRotPiv = pNode->RotationPivot.Get();
    fbxDouble3 fbxPreRot = pNode->PreRotation.Get();
    fbxDouble3 fbxLclRot = pNode->LclRotation.Get();
    fbxDouble3 fbxPostRot = pNode->PostRotation.Get();
    fbxDouble3 fbxSclOff = pNode->ScalingOffset.Get();
    fbxDouble3 fbxSclPiv = pNode->ScalingPivot.Get();
    fbxDouble3 fbxLclScl = pNode->LclScaling.Get();

    m.makeTranslate(osg::Vec3d(
        fbxLclPos[0] + fbxRotOff[0] + fbxRotPiv[0],
        fbxLclPos[1] + fbxRotOff[1] + fbxRotPiv[1],
        fbxLclPos[2] + fbxRotOff[2] + fbxRotPiv[2]));
    if (rotationActive)
    {
        m.preMultRotate(
            makeQuat(fbxPostRot, fbxRotOrder) *
            makeQuat(fbxLclRot, fbxRotOrder) *
            makeQuat(fbxPreRot, fbxRotOrder));
    }
    else
    {
        m.preMultRotate(makeQuat(fbxLclRot, fbxRotOrder));
    }
    m.preMultTranslate(osg::Vec3d(
        fbxSclOff[0] + fbxSclPiv[0] - fbxRotPiv[0],
        fbxSclOff[1] + fbxSclPiv[1] - fbxRotPiv[1],
        fbxSclOff[2] + fbxSclPiv[2] - fbxRotPiv[2]));
    m.preMultScale(osg::Vec3d(fbxLclScl[0], fbxLclScl[1], fbxLclScl[2]));
    m.preMultTranslate(osg::Vec3d(
        -fbxSclPiv[0],
        -fbxSclPiv[1],
        -fbxSclPiv[2]));
}

void readTranslationElement(KFbxTypedProperty<fbxDouble3>& prop,
                            osgAnimation::UpdateMatrixTransform* pUpdate,
                            osg::Matrix& staticTransform)
{
    fbxDouble3 fbxPropValue = prop.Get();
    osg::Vec3d val(
        fbxPropValue[0],
        fbxPropValue[1],
        fbxPropValue[2]);

    if (prop.GetKFCurve(KFCURVENODE_T_X) ||
        prop.GetKFCurve(KFCURVENODE_T_Y) ||
        prop.GetKFCurve(KFCURVENODE_T_Z))
    {
        if (!staticTransform.isIdentity())
        {
            pUpdate->getStackedTransforms().push_back(new osgAnimation::StackedMatrixElement(staticTransform));
            staticTransform.makeIdentity();
        }
        pUpdate->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate", val));
    }
    else
    {
        staticTransform.preMultTranslate(val);
    }
}

void readRotationElement(KFbxTypedProperty<fbxDouble3>& prop,
                         ERotationOrder fbxRotOrder,
                         osgAnimation::UpdateMatrixTransform* pUpdate,
                         osg::Matrix& staticTransform)
{
    osg::Quat quat = makeQuat(prop.Get(), fbxRotOrder);

    if (prop.GetKFCurve(KFCURVENODE_R_X) ||
        prop.GetKFCurve(KFCURVENODE_R_Y) ||
        prop.GetKFCurve(KFCURVENODE_R_Z))
    {
        if (!staticTransform.isIdentity())
        {
            pUpdate->getStackedTransforms().push_back(new osgAnimation::StackedMatrixElement(staticTransform));
            staticTransform.makeIdentity();
        }
        pUpdate->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("quaternion", quat));
    }
    else
    {
        staticTransform.preMultRotate(quat);
    }
}

void readScaleElement(KFbxTypedProperty<fbxDouble3>& prop,
                      osgAnimation::UpdateMatrixTransform* pUpdate,
                      osg::Matrix& staticTransform)
{
    fbxDouble3 fbxPropValue = prop.Get();
    osg::Vec3d val(
        fbxPropValue[0],
        fbxPropValue[1],
        fbxPropValue[2]);

    if (prop.GetKFCurve(KFCURVENODE_S_X) ||
        prop.GetKFCurve(KFCURVENODE_S_Y) ||
        prop.GetKFCurve(KFCURVENODE_S_Z))
    {
        if (!staticTransform.isIdentity())
        {
            pUpdate->getStackedTransforms().push_back(new osgAnimation::StackedMatrixElement(staticTransform));
            staticTransform.makeIdentity();
        }
        pUpdate->getStackedTransforms().push_back(new osgAnimation::StackedScaleElement("scale", val));
    }
    else
    {
        staticTransform.preMultScale(val);
    }
}

void readUpdateMatrixTransform(osgAnimation::UpdateMatrixTransform* pUpdate, KFbxNode* pNode)
{
    osg::Matrix staticTransform;

    readTranslationElement(pNode->LclTranslation, pUpdate, staticTransform);

    fbxDouble3 fbxRotOffset = pNode->RotationOffset.Get();
    fbxDouble3 fbxRotPiv = pNode->RotationPivot.Get();
    staticTransform.preMultTranslate(osg::Vec3d(
        fbxRotPiv[0] + fbxRotOffset[0],
        fbxRotPiv[1] + fbxRotOffset[1],
        fbxRotPiv[2] + fbxRotOffset[2]));

    // When this flag is set to false, the RotationOrder, the Pre/Post rotation
    // values and the rotation limits should be ignored.
    bool rotationActive = pNode->RotationActive.Get();

    ERotationOrder fbxRotOrder = (rotationActive && pNode->RotationOrder.IsValid()) ?
        pNode->RotationOrder.Get() : eEULER_XYZ;

    if (rotationActive)
    {
        staticTransform.preMultRotate(makeQuat(pNode->PreRotation.Get(), fbxRotOrder));
    }

    readRotationElement(pNode->LclRotation, fbxRotOrder, pUpdate, staticTransform);

    if (rotationActive)
    {
        staticTransform.preMultRotate(makeQuat(pNode->PostRotation.Get(), fbxRotOrder));
    }

    fbxDouble3 fbxSclOffset = pNode->ScalingOffset.Get();
    fbxDouble3 fbxSclPiv = pNode->ScalingPivot.Get();
    staticTransform.preMultTranslate(osg::Vec3d(
        fbxSclOffset[0] + fbxSclPiv[0] - fbxRotPiv[0],
        fbxSclOffset[1] + fbxSclPiv[1] - fbxRotPiv[1],
        fbxSclOffset[2] + fbxSclPiv[2] - fbxRotPiv[2]));

    readScaleElement(pNode->LclScaling, pUpdate, staticTransform);

    staticTransform.preMultTranslate(osg::Vec3d(
        -fbxSclPiv[0],
        -fbxSclPiv[1],
        -fbxSclPiv[2]));

    if (!staticTransform.isIdentity())
    {
        pUpdate->getStackedTransforms().push_back(new osgAnimation::StackedMatrixElement(staticTransform));
    }
}

osg::Group* createGroupNode(KFbxSdkManager& pSdkManager, KFbxNode* pNode,
    const std::string& animName, const osg::Matrix& localMatrix, bool bNeedSkeleton,
    std::map<KFbxNode*, osg::Node*>& nodeMap)
{
    if (bNeedSkeleton)
    {
        osgAnimation::Bone* osgBone = new osgAnimation::Bone;
        osgBone->setDataVariance(osg::Object::DYNAMIC);
        osgBone->setName(pNode->GetName());
        osgAnimation::UpdateBone* pUpdate = new osgAnimation::UpdateBone(animName);
        readUpdateMatrixTransform(pUpdate, pNode);
        osgBone->setUpdateCallback(pUpdate);

        nodeMap.insert(std::pair<KFbxNode*, osg::Node*>(pNode, osgBone));

        return osgBone;
    }
    else
    {
        bool bAnimated = !animName.empty();
        if (!bAnimated && localMatrix.isIdentity())
        {
            osg::Group* pGroup = new osg::Group;
            pGroup->setName(pNode->GetName());
            return pGroup;
        }

        osg::MatrixTransform* pTransform = new osg::MatrixTransform(localMatrix);
        pTransform->setName(pNode->GetName());

        if (bAnimated)
        {
            osgAnimation::UpdateMatrixTransform* pUpdate = new osgAnimation::UpdateMatrixTransform(animName);
            readUpdateMatrixTransform(pUpdate, pNode);
            pTransform->setUpdateCallback(pUpdate);
        }

        return pTransform;
    }
}

osgDB::ReaderWriter::ReadResult readFbxNode(
    KFbxSdkManager& pSdkManager, KFbxNode* pNode,
    osg::ref_ptr<osgAnimation::AnimationManagerBase>& pAnimationManager,
    bool& bIsBone, int& nLightCount,
    FbxMaterialToOsgStateSet& fbxMaterialToOsgStateSet,
    std::map<KFbxNode*, osg::Node*>& nodeMap,
    BindMatrixMap& boneBindMatrices,
    const std::set<const KFbxNode*>& fbxSkeletons,
    std::map<KFbxNode*, osgAnimation::Skeleton*>& skeletonMap,
    const osgDB::ReaderWriter::Options* options)
{
    if (KFbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute())
    {
        if (lNodeAttribute->GetAttributeType() == KFbxNodeAttribute::eNURB ||
            lNodeAttribute->GetAttributeType() == KFbxNodeAttribute::ePATCH)
        {
            KFbxGeometryConverter lConverter(&pSdkManager);
            lConverter.TriangulateInPlace(pNode);
        }
    }

    bIsBone = false;
    bool bCreateSkeleton = false;

    KFbxNodeAttribute::EAttributeType lAttributeType = KFbxNodeAttribute::eUNIDENTIFIED;
    if (pNode->GetNodeAttribute())
    {
        lAttributeType = pNode->GetNodeAttribute()->GetAttributeType();
        if (lAttributeType == KFbxNodeAttribute::eSKELETON)
        {
            bIsBone = true;
        }
    }

    if (!bIsBone && fbxSkeletons.find(pNode) != fbxSkeletons.end())
    {
        bIsBone = true;
    }

    unsigned nMaterials = pNode->GetMaterialCount();
    std::vector<StateSetContent > stateSetList;

    for (unsigned i = 0; i < nMaterials; ++i)
    {
        KFbxSurfaceMaterial* fbxMaterial = pNode->GetMaterial(i);
        assert(fbxMaterial);
        stateSetList.push_back(fbxMaterialToOsgStateSet.convert(fbxMaterial));
    }

    osg::NodeList skeletal, children;

    int nChildCount = pNode->GetChildCount();
    for (int i = 0; i < nChildCount; ++i)
    {
        KFbxNode* pChildNode = pNode->GetChild(i);

        if (pChildNode->GetParent() != pNode)
        {
            //workaround for bug that occurs in some files exported from Blender
            continue;
        }

        bool bChildIsBone = false;
        osgDB::ReaderWriter::ReadResult childResult = readFbxNode(
            pSdkManager, pChildNode, pAnimationManager,
            bChildIsBone, nLightCount, fbxMaterialToOsgStateSet, nodeMap,
            boneBindMatrices, fbxSkeletons, skeletonMap, options);
        if (childResult.error())
        {
            return childResult;
        }
        else if (osg::Node* osgChild = childResult.getNode())
        {
            if (bChildIsBone)
            {
                if (!bIsBone) bCreateSkeleton = true;
                skeletal.push_back(osgChild);
            }
            else
            {
                children.push_back(osgChild);
            }
        }
    }

    std::string animName = readFbxAnimation(pNode, pAnimationManager, pNode->GetName());

    osg::Matrix localMatrix;
    makeLocalMatrix(pNode, localMatrix);
    bool bLocalMatrixIdentity = localMatrix.isIdentity();

    osg::ref_ptr<osg::Group> osgGroup;

    bool bEmpty = children.empty() && !bIsBone;

    switch (lAttributeType)
    {
    case KFbxNodeAttribute::eUNIDENTIFIED:
        if (bLocalMatrixIdentity && children.size() + skeletal.size() == 1)
        {
            if (children.size() == 1)
            {
                return osgDB::ReaderWriter::ReadResult(children.front().get());
            }
            else
            {
                return osgDB::ReaderWriter::ReadResult(skeletal.front().get());
            }
        }
        break;
    case KFbxNodeAttribute::eMESH:
        {
            size_t bindMatrixCount = boneBindMatrices.size();
            osgDB::ReaderWriter::ReadResult meshRes = readFbxMesh(pSdkManager,
                pNode, pAnimationManager, stateSetList, boneBindMatrices,
                fbxSkeletons, skeletonMap);
            if (meshRes.error())
            {
                return meshRes;
            }
            else if (osg::Node* node = meshRes.getNode())
            {
                bEmpty = false;

                if (bindMatrixCount != boneBindMatrices.size())
                {
                    //The mesh is skinned therefore the bind matrix will handle all transformations.
                    localMatrix.makeIdentity();
                    bLocalMatrixIdentity = true;
                }

                if (animName.empty() &&
                    children.empty() &&
                    skeletal.empty() &&
                    bLocalMatrixIdentity)
                {
                    return osgDB::ReaderWriter::ReadResult(node);
                }

                children.insert(children.begin(), node);
            }
        }
        break;
    case KFbxNodeAttribute::eCAMERA:
    case KFbxNodeAttribute::eLIGHT:
        {
            osgDB::ReaderWriter::ReadResult res =
                lAttributeType == KFbxNodeAttribute::eCAMERA ?
                readFbxCamera(pNode) : readFbxLight(pNode, nLightCount);
            if (res.error())
            {
                return res;
            }
            else if (osg::Group* resGroup = dynamic_cast<osg::Group*>(res.getObject()))
            {
                bEmpty = false;
                if (animName.empty() &&
                    bLocalMatrixIdentity)
                {
                    osgGroup = resGroup;
                }
                else
                {
                    children.insert(children.begin(), resGroup);
                }
            }
        }
        break;
    }

    if (bEmpty)
    {
        osgDB::ReaderWriter::ReadResult(0);
    }

    if (!osgGroup) osgGroup = createGroupNode(pSdkManager, pNode, animName, localMatrix, bIsBone, nodeMap);

    osg::Group* pAddChildrenTo = osgGroup.get();
    if (bCreateSkeleton)
    {
        osgAnimation::Skeleton* osgSkeleton = getSkeleton(pNode, fbxSkeletons, skeletonMap);
        osgSkeleton->setDefaultUpdateCallback();
        pAddChildrenTo->addChild(osgSkeleton);
        pAddChildrenTo = osgSkeleton;
    }

    for (osg::NodeList::iterator it = skeletal.begin(); it != skeletal.end(); ++it)
    {
        pAddChildrenTo->addChild(it->get());
    }
    for (osg::NodeList::iterator it = children.begin(); it != children.end(); ++it)
    {
        pAddChildrenTo->addChild(it->get());
    }


    return osgDB::ReaderWriter::ReadResult(osgGroup.get());
}

osgAnimation::Skeleton* getSkeleton(KFbxNode* fbxNode,
    const std::set<const KFbxNode*>& fbxSkeletons,
    std::map<KFbxNode*, osgAnimation::Skeleton*>& skeletonMap)
{
    //Find the first non-skeleton ancestor of the node.
    while (fbxNode &&
        ((fbxNode->GetNodeAttribute() &&
        fbxNode->GetNodeAttribute()->GetAttributeType() == KFbxNodeAttribute::eSKELETON) ||
        fbxSkeletons.find(fbxNode) != fbxSkeletons.end()))
    {
        fbxNode = fbxNode->GetParent();
    }

    std::map<KFbxNode*, osgAnimation::Skeleton*>::const_iterator it = skeletonMap.find(fbxNode);
    if (it == skeletonMap.end())
    {
        osgAnimation::Skeleton* skel = new osgAnimation::Skeleton;
        skel->setDefaultUpdateCallback();
        skeletonMap.insert(std::pair<KFbxNode*, osgAnimation::Skeleton*>(fbxNode, skel));
        return skel;
    }
    else
    {
        return it->second;
    }
}
