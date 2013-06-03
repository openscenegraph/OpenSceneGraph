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
#include <osgAnimation/StackedRotateAxisElement>
#include <osgAnimation/StackedScaleElement>
#include <osgAnimation/StackedTranslateElement>
#include <osgAnimation/UpdateBone>

#if defined(_MSC_VER)
    #pragma warning( disable : 4505 )
    #pragma warning( default : 4996 )
#endif
#include <fbxsdk.h>

#include "fbxReader.h"

bool isAnimated(FbxProperty& prop, FbxScene& fbxScene)
{
    for (int i = 0; i < fbxScene.GetSrcObjectCount<FbxAnimStack>(); ++i)
    {
        FbxAnimStack* pAnimStack = FbxCast<FbxAnimStack>(fbxScene.GetSrcObject<FbxAnimStack>(i));

        const int nbAnimLayers = pAnimStack->GetMemberCount<FbxAnimLayer>();
        for (int j = 0; j < nbAnimLayers; j++)
        {
            FbxAnimLayer* pAnimLayer = pAnimStack->GetMember<FbxAnimLayer>(j);
            if (prop.GetCurveNode(pAnimLayer, false))
            {
                return true;
            }
        }
    }
    return false;
}

bool isAnimated(FbxProperty& prop, const char* channel, FbxScene& fbxScene)
{
    for (int i = 0; i < fbxScene.GetSrcObjectCount<FbxAnimStack>(); ++i)
    {
        FbxAnimStack* pAnimStack = FbxCast<FbxAnimStack>(fbxScene.GetSrcObject<FbxAnimStack>(i));

        const int nbAnimLayers = pAnimStack->GetMemberCount<FbxAnimLayer>();
        for (int j = 0; j < nbAnimLayers; j++)
        {
            FbxAnimLayer* pAnimLayer = pAnimStack->GetMember<FbxAnimLayer>(j);
            if (prop.GetCurve(pAnimLayer, channel, false))
            {
                return true;
            }
        }
    }
    return false;
}

osg::Quat makeQuat(const FbxDouble3& degrees, EFbxRotationOrder fbxRotOrder)
{
    double radiansX = osg::DegreesToRadians(degrees[0]);
    double radiansY = osg::DegreesToRadians(degrees[1]);
    double radiansZ = osg::DegreesToRadians(degrees[2]);

    switch (fbxRotOrder)
    {
    case eEulerXYZ:
        return osg::Quat(
            radiansX, osg::Vec3d(1,0,0),
            radiansY, osg::Vec3d(0,1,0),
            radiansZ, osg::Vec3d(0,0,1));
    case eEulerXZY:
        return osg::Quat(
            radiansX, osg::Vec3d(1,0,0),
            radiansZ, osg::Vec3d(0,0,1),
            radiansY, osg::Vec3d(0,1,0));
    case eEulerYZX:
        return osg::Quat(
            radiansY, osg::Vec3d(0,1,0),
            radiansZ, osg::Vec3d(0,0,1),
            radiansX, osg::Vec3d(1,0,0));
    case eEulerYXZ:
        return osg::Quat(
            radiansY, osg::Vec3d(0,1,0),
            radiansX, osg::Vec3d(1,0,0),
            radiansZ, osg::Vec3d(0,0,1));
    case eEulerZXY:
        return osg::Quat(
            radiansZ, osg::Vec3d(0,0,1),
            radiansX, osg::Vec3d(1,0,0),
            radiansY, osg::Vec3d(0,1,0));
    case eEulerZYX:
        return osg::Quat(
            radiansZ, osg::Vec3d(0,0,1),
            radiansY, osg::Vec3d(0,1,0),
            radiansX, osg::Vec3d(1,0,0));
    case eSphericXYZ:
        {
            //I don't know what eSPHERIC_XYZ means, so this is a complete guess.
            osg::Quat quat;
            quat.makeRotate(osg::Vec3d(1.0, 0.0, 0.0), osg::Vec3d(degrees[0], degrees[1], degrees[2]));
            return quat;
        }
    default:
        OSG_WARN << "Invalid FBX rotation mode." << std::endl;
        return osg::Quat();
    }
}

void makeLocalMatrix(const FbxNode* pNode, osg::Matrix& m)
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

    EFbxRotationOrder fbxRotOrder = rotationActive ? pNode->RotationOrder.Get() : eEulerXYZ;

    FbxDouble3 fbxLclPos = pNode->LclTranslation.Get();
    FbxDouble3 fbxRotOff = pNode->RotationOffset.Get();
    FbxDouble3 fbxRotPiv = pNode->RotationPivot.Get();
    FbxDouble3 fbxPreRot = pNode->PreRotation.Get();
    FbxDouble3 fbxLclRot = pNode->LclRotation.Get();
    FbxDouble3 fbxPostRot = pNode->PostRotation.Get();
    FbxDouble3 fbxSclOff = pNode->ScalingOffset.Get();
    FbxDouble3 fbxSclPiv = pNode->ScalingPivot.Get();
    FbxDouble3 fbxLclScl = pNode->LclScaling.Get();

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

void readTranslationElement(FbxPropertyT<FbxDouble3>& prop,
                            osgAnimation::UpdateMatrixTransform* pUpdate,
                            osg::Matrix& staticTransform,
                            FbxScene& fbxScene)
{
    FbxDouble3 fbxPropValue = prop.Get();
    osg::Vec3d val(
        fbxPropValue[0],
        fbxPropValue[1],
        fbxPropValue[2]);

    if (isAnimated(prop, fbxScene))
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

void getRotationOrder(EFbxRotationOrder fbxRotOrder, int order[/*3*/])
{
    switch (fbxRotOrder)
    {
    case eEulerXZY:
        order[0] = 0; order[1] = 2; order[2] = 1;
        break;
    case eEulerYZX:
        order[0] = 1; order[1] = 2; order[2] = 0;
        break;
    case eEulerYXZ:
        order[0] = 1; order[1] = 0; order[2] = 2;
        break;
    case eEulerZXY:
        order[0] = 2; order[1] = 0; order[2] = 1;
        break;
    case eEulerZYX:
        order[0] = 2; order[1] = 1; order[2] = 0;
        break;
    default:
        order[0] = 0; order[1] = 1; order[2] = 2;
    }
}

void readRotationElement(FbxPropertyT<FbxDouble3>& prop,
                         EFbxRotationOrder fbxRotOrder,
                         bool quatInterpolate,
                         osgAnimation::UpdateMatrixTransform* pUpdate,
                         osg::Matrix& staticTransform,
                         FbxScene& fbxScene)
{
    if (isAnimated(prop, fbxScene))
    {
        if (quatInterpolate)
        {
            if (!staticTransform.isIdentity())
            {
                pUpdate->getStackedTransforms().push_back(
                    new osgAnimation::StackedMatrixElement(staticTransform));
                staticTransform.makeIdentity();
            }
            pUpdate->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement(
                "quaternion", makeQuat(prop.Get(), fbxRotOrder)));
        }
        else
        {
            const char* curveNames[3] = {FBXSDK_CURVENODE_COMPONENT_X, FBXSDK_CURVENODE_COMPONENT_Y, FBXSDK_CURVENODE_COMPONENT_Z};
            osg::Vec3 axes[3] = {osg::Vec3(1,0,0), osg::Vec3(0,1,0), osg::Vec3(0,0,1)};

            FbxDouble3 fbxPropValue = prop.Get();
            fbxPropValue[0] = osg::DegreesToRadians(fbxPropValue[0]);
            fbxPropValue[1] = osg::DegreesToRadians(fbxPropValue[1]);
            fbxPropValue[2] = osg::DegreesToRadians(fbxPropValue[2]);

            int order[3] = {0, 1, 2};
            getRotationOrder(fbxRotOrder, order);

            for (int i = 0; i < 3; ++i)
            {
                int j = order[2-i];
                if (isAnimated(prop, curveNames[j], fbxScene))
                {
                    if (!staticTransform.isIdentity())
                    {
                        pUpdate->getStackedTransforms().push_back(new osgAnimation::StackedMatrixElement(staticTransform));
                        staticTransform.makeIdentity();
                    }

                    pUpdate->getStackedTransforms().push_back(new osgAnimation::StackedRotateAxisElement(
                        std::string("rotate") + curveNames[j], axes[j], fbxPropValue[j]));
                }
                else
                {
                    staticTransform.preMultRotate(osg::Quat(fbxPropValue[j], axes[j]));
                }
            }
        }
    }
    else
    {
        staticTransform.preMultRotate(makeQuat(prop.Get(), fbxRotOrder));
    }
}

void readScaleElement(FbxPropertyT<FbxDouble3>& prop,
                      osgAnimation::UpdateMatrixTransform* pUpdate,
                      osg::Matrix& staticTransform,
                      FbxScene& fbxScene)
{
    FbxDouble3 fbxPropValue = prop.Get();
    osg::Vec3d val(
        fbxPropValue[0],
        fbxPropValue[1],
        fbxPropValue[2]);

    if (isAnimated(prop, fbxScene))
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

void readUpdateMatrixTransform(osgAnimation::UpdateMatrixTransform* pUpdate, FbxNode* pNode, FbxScene& fbxScene)
{
    osg::Matrix staticTransform;

    readTranslationElement(pNode->LclTranslation, pUpdate, staticTransform, fbxScene);

    FbxDouble3 fbxRotOffset = pNode->RotationOffset.Get();
    FbxDouble3 fbxRotPiv = pNode->RotationPivot.Get();
    staticTransform.preMultTranslate(osg::Vec3d(
        fbxRotPiv[0] + fbxRotOffset[0],
        fbxRotPiv[1] + fbxRotOffset[1],
        fbxRotPiv[2] + fbxRotOffset[2]));

    // When this flag is set to false, the RotationOrder, the Pre/Post rotation
    // values and the rotation limits should be ignored.
    bool rotationActive = pNode->RotationActive.Get();

    EFbxRotationOrder fbxRotOrder = (rotationActive && pNode->RotationOrder.IsValid()) ?
        pNode->RotationOrder.Get() : eEulerXYZ;

    if (rotationActive)
    {
        staticTransform.preMultRotate(makeQuat(pNode->PreRotation.Get(), fbxRotOrder));
    }

    readRotationElement(pNode->LclRotation, fbxRotOrder,
        pNode->QuaternionInterpolate.IsValid() && pNode->QuaternionInterpolate.Get(),
        pUpdate, staticTransform, fbxScene);

    if (rotationActive)
    {
        staticTransform.preMultRotate(makeQuat(pNode->PostRotation.Get(), fbxRotOrder));
    }

    FbxDouble3 fbxSclOffset = pNode->ScalingOffset.Get();
    FbxDouble3 fbxSclPiv = pNode->ScalingPivot.Get();
    staticTransform.preMultTranslate(osg::Vec3d(
        fbxSclOffset[0] + fbxSclPiv[0] - fbxRotPiv[0],
        fbxSclOffset[1] + fbxSclPiv[1] - fbxRotPiv[1],
        fbxSclOffset[2] + fbxSclPiv[2] - fbxRotPiv[2]));

    readScaleElement(pNode->LclScaling, pUpdate, staticTransform, fbxScene);

    staticTransform.preMultTranslate(osg::Vec3d(
        -fbxSclPiv[0],
        -fbxSclPiv[1],
        -fbxSclPiv[2]));

    if (!staticTransform.isIdentity())
    {
        pUpdate->getStackedTransforms().push_back(new osgAnimation::StackedMatrixElement(staticTransform));
    }
}

osg::Group* createGroupNode(FbxManager& pSdkManager, FbxNode* pNode,
    const std::string& animName, const osg::Matrix& localMatrix, bool bNeedSkeleton,
    std::map<FbxNode*, osg::Node*>& nodeMap, FbxScene& fbxScene)
{
    if (bNeedSkeleton)
    {
        osgAnimation::Bone* osgBone = new osgAnimation::Bone;
        osgBone->setDataVariance(osg::Object::DYNAMIC);
        osgBone->setName(pNode->GetName());
        osgAnimation::UpdateBone* pUpdate = new osgAnimation::UpdateBone(animName);
        readUpdateMatrixTransform(pUpdate, pNode, fbxScene);
        osgBone->setUpdateCallback(pUpdate);

        nodeMap.insert(std::pair<FbxNode*, osg::Node*>(pNode, osgBone));

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
            readUpdateMatrixTransform(pUpdate, pNode, fbxScene);
            pTransform->setUpdateCallback(pUpdate);
        }

        return pTransform;
    }
}

osgDB::ReaderWriter::ReadResult OsgFbxReader::readFbxNode(
    FbxNode* pNode,
    bool& bIsBone, int& nLightCount)
{
    if (FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute())
    {
        FbxNodeAttribute::EType attrType = lNodeAttribute->GetAttributeType();
        switch (attrType)
        {
        case FbxNodeAttribute::eNurbs:
        case FbxNodeAttribute::ePatch:
        case FbxNodeAttribute::eNurbsCurve:
        case FbxNodeAttribute::eNurbsSurface:
            {
                FbxGeometryConverter lConverter(&pSdkManager);
#if FBXSDK_VERSION_MAJOR < 2014
                if (!lConverter.TriangulateInPlace(pNode))
#else
                if (!lConverter.Triangulate(lNodeAttribute,true,false))
#endif
                {
                    OSG_WARN << "Unable to triangulate FBX NURBS " << pNode->GetName() << std::endl;
                }
            }
            break;
        default:
            break;
        }
    }

    bIsBone = false;
    bool bCreateSkeleton = false;

    FbxNodeAttribute::EType lAttributeType = FbxNodeAttribute::eUnknown;
    if (pNode->GetNodeAttribute())
    {
        lAttributeType = pNode->GetNodeAttribute()->GetAttributeType();
        if (lAttributeType == FbxNodeAttribute::eSkeleton)
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
        FbxSurfaceMaterial* fbxMaterial = pNode->GetMaterial(i);
        assert(fbxMaterial);
        stateSetList.push_back(fbxMaterialToOsgStateSet.convert(fbxMaterial));
    }

    osg::NodeList skeletal, children;

    int nChildCount = pNode->GetChildCount();
    for (int i = 0; i < nChildCount; ++i)
    {
        FbxNode* pChildNode = pNode->GetChild(i);

        if (pChildNode->GetParent() != pNode)
        {
            //workaround for bug that occurs in some files exported from Blender
            continue;
        }

        bool bChildIsBone = false;
        osgDB::ReaderWriter::ReadResult childResult = readFbxNode(
            pChildNode, bChildIsBone, nLightCount);
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

    std::string animName = readFbxAnimation(pNode, pNode->GetName());

    osg::Matrix localMatrix;
    makeLocalMatrix(pNode, localMatrix);
    bool bLocalMatrixIdentity = localMatrix.isIdentity();

    osg::ref_ptr<osg::Group> osgGroup;

    bool bEmpty = children.empty() && !bIsBone;

    switch (lAttributeType)
    {
    case FbxNodeAttribute::eMesh:
        {
            size_t bindMatrixCount = boneBindMatrices.size();
            osgDB::ReaderWriter::ReadResult meshRes = readFbxMesh(pNode, stateSetList);
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
    case FbxNodeAttribute::eCamera:
    case FbxNodeAttribute::eLight:
        {
            osgDB::ReaderWriter::ReadResult res =
                lAttributeType == FbxNodeAttribute::eCamera ?
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
    default:
        break;
    }

    if (bEmpty)
    {
        osgDB::ReaderWriter::ReadResult(0);
    }

    if (!osgGroup) osgGroup = createGroupNode(pSdkManager, pNode, animName, localMatrix, bIsBone, nodeMap, fbxScene);

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

osgAnimation::Skeleton* getSkeleton(FbxNode* fbxNode,
    const std::set<const FbxNode*>& fbxSkeletons,
    std::map<FbxNode*, osgAnimation::Skeleton*>& skeletonMap)
{
    //Find the first non-skeleton ancestor of the node.
    while (fbxNode &&
        ((fbxNode->GetNodeAttribute() &&
        fbxNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton) ||
        fbxSkeletons.find(fbxNode) != fbxSkeletons.end()))
    {
        fbxNode = fbxNode->GetParent();
    }

    std::map<FbxNode*, osgAnimation::Skeleton*>::const_iterator it = skeletonMap.find(fbxNode);
    if (it == skeletonMap.end())
    {
        osgAnimation::Skeleton* skel = new osgAnimation::Skeleton;
        skel->setDefaultUpdateCallback();
        skeletonMap.insert(std::pair<FbxNode*, osgAnimation::Skeleton*>(fbxNode, skel));
        return skel;
    }
    else
    {
        return it->second;
    }
}
