#include <memory>

#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/Material>
#include <osg/Texture2D>

#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>

#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/Skeleton>
#include <osgAnimation/UpdateCallback>

#if defined(_MSC_VER)
    #pragma warning( disable : 4505 )
#endif
#include <fbxsdk.h>

#include "fbxRAnimation.h"
#include "fbxRCamera.h"
#include "fbxRLight.h"
#include "fbxRMesh.h"

template <typename FbxT, typename OsgT, typename ConvertFunc>
class FbxToOsgMap
{
    std::map<const FbxT*, osg::ref_ptr<OsgT>> m_map;
public:
    ConvertFunc m_convertFunc;

    FbxToOsgMap(ConvertFunc convertFunc) : m_convertFunc(convertFunc) {}

    osg::ref_ptr<OsgT> Get(const FbxT* fbx)
    {
        if (!fbx)
            return 0;
        std::map<const FbxT*, osg::ref_ptr<OsgT>>::iterator it = m_map.find(fbx);
        if (it != m_map.end())
        {
            return it->second;
        }
        osg::ref_ptr<OsgT> osgObj = m_convertFunc(fbx);
        m_map.insert(std::pair<const FbxT*, osg::ref_ptr<OsgT>>(fbx, osgObj));
        return osgObj;
    }
};

struct GetOsgTexture
{
    const std::string& m_dir;

    GetOsgTexture(const std::string& dir) : m_dir(dir) {}

    static osg::Texture::WrapMode convertWrap(KFbxTexture::EWrapMode wrap)
    {
        return wrap == KFbxTexture::eREPEAT ?
            osg::Texture2D::REPEAT : osg::Texture2D::CLAMP_TO_EDGE;
    }

    osg::ref_ptr<osg::Texture2D> operator () (const KFbxTexture* fbx)
    {
        osg::Image* pImage;
        if ((pImage = osgDB::readImageFile(osgDB::concatPaths(m_dir, fbx->GetRelativeFileName()))) ||
            (pImage = osgDB::readImageFile(osgDB::concatPaths(m_dir, fbx->GetFileName()))))
        {
            osg::ref_ptr<osg::Texture2D> pOsgTex = new osg::Texture2D;

            pOsgTex->setImage(pImage);
            pOsgTex->setWrap(osg::Texture2D::WRAP_S, convertWrap(fbx->GetWrapModeU()));
            pOsgTex->setWrap(osg::Texture2D::WRAP_T, convertWrap(fbx->GetWrapModeV()));

            return pOsgTex;
        }
        else
        {
            return 0;
        }
    }
};

struct GetOsgMaterial
{
    typedef FbxToOsgMap<KFbxTexture, osg::Texture2D, GetOsgTexture> TextureMap;
    TextureMap m_textureMap;

public:
    osg::ref_ptr<osg::Texture2D> m_pTexture;

    GetOsgMaterial(const std::string& dir) : m_textureMap(GetOsgTexture(dir)){}

    osg::ref_ptr<osg::Material> operator () (const KFbxSurfaceMaterial* pFbxMat)
    {
        osg::ref_ptr<osg::Material> pOsgMat = new osg::Material;

        const KFbxSurfaceLambert* pFbxLambert = dynamic_cast<const KFbxSurfaceLambert*>(pFbxMat);

        const KFbxProperty lProperty = pFbxMat->FindProperty(KFbxSurfaceMaterial::sDiffuse);
        if(lProperty.IsValid()){
            int lNbTex = lProperty.GetSrcObjectCount(KFbxTexture::ClassId);
            for (int lTextureIndex = 0; lTextureIndex < lNbTex; lTextureIndex++)
            {
                const KFbxTexture* lTexture = KFbxCast<KFbxTexture>(lProperty.GetSrcObject(KFbxTexture::ClassId, lTextureIndex)); 
                if(lTexture)
                {
                    m_pTexture = m_textureMap.Get(lTexture);
                }

                //For now only allow 1 texture
                break;
            }
        }

        if (pFbxLambert)
        {
            fbxDouble3 color = pFbxLambert->GetDiffuseColor().Get();
            double factor = pFbxLambert->GetDiffuseFactor().Get();
            pOsgMat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(
                static_cast<float>(color[0] * factor),
                static_cast<float>(color[1] * factor),
                static_cast<float>(color[2] * factor),
                static_cast<float>(1.0 - pFbxLambert->GetTransparencyFactor().Get())));

            color = pFbxLambert->GetAmbientColor().Get();
            factor = pFbxLambert->GetAmbientFactor().Get();
            pOsgMat->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(
                static_cast<float>(color[0] * factor),
                static_cast<float>(color[1] * factor),
                static_cast<float>(color[2] * factor),
                1.0f));

            color = pFbxLambert->GetEmissiveColor().Get();
            factor = pFbxLambert->GetEmissiveFactor().Get();
            pOsgMat->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(
                static_cast<float>(color[0] * factor),
                static_cast<float>(color[1] * factor),
                static_cast<float>(color[2] * factor),
                1.0f));

            if (const KFbxSurfacePhong* pFbxPhong = dynamic_cast<const KFbxSurfacePhong*>(pFbxLambert))
            {
                color = pFbxPhong->GetSpecularColor().Get();
                factor = pFbxPhong->GetSpecularFactor().Get();
                pOsgMat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(
                    static_cast<float>(color[0] * factor),
                    static_cast<float>(color[1] * factor),
                    static_cast<float>(color[2] * factor),
                    1.0f));

                pOsgMat->setShininess(osg::Material::FRONT_AND_BACK,
                    static_cast<float>(pFbxPhong->GetShininess().Get()));
            }
        }

        return pOsgMat;
    }
};

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
            radiansY, osg::Vec3d(0,0,1),
            radiansZ, osg::Vec3d(0,1,0));
    case eEULER_YZX:
        return osg::Quat(
            radiansX, osg::Vec3d(0,1,0),
            radiansY, osg::Vec3d(0,0,1),
            radiansZ, osg::Vec3d(1,0,0));
    case eEULER_YXZ:
        return osg::Quat(
            radiansX, osg::Vec3d(0,1,0),
            radiansY, osg::Vec3d(1,0,0),
            radiansZ, osg::Vec3d(0,0,1));
    case eEULER_ZXY:
        return osg::Quat(
            radiansX, osg::Vec3d(0,0,1),
            radiansY, osg::Vec3d(1,0,0),
            radiansZ, osg::Vec3d(0,1,0));
    case eEULER_ZYX:
        return osg::Quat(
            radiansX, osg::Vec3d(0,0,1),
            radiansY, osg::Vec3d(0,1,0),
            radiansZ, osg::Vec3d(1,0,0));
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

    ERotationOrder fbxRotOrder = pNode->RotationOrder.Get();

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
    m.preMultRotate(
        makeQuat(fbxPostRot, fbxRotOrder) *
        makeQuat(fbxLclRot, fbxRotOrder) *
        makeQuat(fbxPreRot, fbxRotOrder));
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

void getApproximateTransform(const KFbxNode* pNode, osg::Vec3& trans, osg::Quat& quat, osg::Vec3& scale)
{
    ERotationOrder fbxRotOrder = pNode->RotationOrder.Get();

    fbxDouble3 fbxLclPos = pNode->LclTranslation.Get();
    //fbxDouble3 fbxRotOff = pNode->RotationOffset.Get();
    //fbxDouble3 fbxRotPiv = pNode->RotationPivot.Get();
    fbxDouble3 fbxPreRot = pNode->PreRotation.Get();
    fbxDouble3 fbxLclRot = pNode->LclRotation.Get();
    fbxDouble3 fbxPostRot = pNode->PostRotation.Get();
    //fbxDouble3 fbxSclOff = pNode->ScalingOffset.Get();
    //fbxDouble3 fbxSclPiv = pNode->ScalingPivot.Get();
    fbxDouble3 fbxLclScl = pNode->LclScaling.Get();

    trans.set(
        static_cast<float>(fbxLclPos[0]),
        static_cast<float>(fbxLclPos[1]),
        static_cast<float>(fbxLclPos[2]));

    quat =
        makeQuat(fbxPostRot, fbxRotOrder) *
        makeQuat(fbxLclRot, fbxRotOrder) *
        makeQuat(fbxPreRot, fbxRotOrder);

    scale.set(
        static_cast<float>(fbxLclScl[0]),
        static_cast<float>(fbxLclScl[1]),
        static_cast<float>(fbxLclScl[2]));
}

void getApproximateTransform(const KFbxNode* pNode, osg::Vec3& trans, osg::Vec3& euler, osg::Vec3& scale)
{
    //ERotationOrder fbxRotOrder = pNode->RotationOrder.Get();

    fbxDouble3 fbxLclPos = pNode->LclTranslation.Get();
    //fbxDouble3 fbxRotOff = pNode->RotationOffset.Get();
    //fbxDouble3 fbxRotPiv = pNode->RotationPivot.Get();
    fbxDouble3 fbxPreRot = pNode->PreRotation.Get();
    fbxDouble3 fbxLclRot = pNode->LclRotation.Get();
    fbxDouble3 fbxPostRot = pNode->PostRotation.Get();
    //fbxDouble3 fbxSclOff = pNode->ScalingOffset.Get();
    //fbxDouble3 fbxSclPiv = pNode->ScalingPivot.Get();
    fbxDouble3 fbxLclScl = pNode->LclScaling.Get();

    trans.set(
        static_cast<float>(fbxLclPos[0]),
        static_cast<float>(fbxLclPos[1]),
        static_cast<float>(fbxLclPos[2]));

    //TODO: Convert each rotation to a quaternion, concatenate them and extract euler from that.
    euler.set(
        osg::DegreesToRadians(static_cast<float>(fbxPreRot[0] + fbxLclRot[0] + fbxPostRot[0])),
        osg::DegreesToRadians(static_cast<float>(fbxPreRot[1] + fbxLclRot[1] + fbxPostRot[1])),
        osg::DegreesToRadians(static_cast<float>(fbxPreRot[2] + fbxLclRot[2] + fbxPostRot[2])));

    scale.set(
        static_cast<float>(fbxLclScl[0]),
        static_cast<float>(fbxLclScl[1]),
        static_cast<float>(fbxLclScl[2]));
}

bool readBindPose(KFbxSdkManager& pManager, KFbxNode* pNode,
    osgAnimation::Bone* osgBone)
{
    KArrayTemplate<KFbxPose*> pPoseList;
    KArrayTemplate<int> pIndex;
    if (!pNode || !KFbxPose::GetBindPoseContaining(
        pManager, pNode, pPoseList, pIndex))
    {
        return false;
    }

    const double* pMat = pPoseList[0]->GetMatrix(pIndex[0]);
    osgBone->setBindMatrixInBoneSpace(osg::Matrix(pMat));
    return true;
}

osg::Group* createGroupNode(KFbxSdkManager& pSdkManager, KFbxNode* pNode,
    const std::string& animName, const osg::Matrix& localMatrix, bool bNeedSkeleton)
{
    if (bNeedSkeleton)
    {
        osgAnimation::Bone* osgBone = new osgAnimation::Bone;
        osgBone->setDataVariance(osg::Object::DYNAMIC);
        osgBone->setName(pNode->GetName());
        osgBone->setDefaultUpdateCallback(animName);

        readBindPose(pSdkManager, pNode, osgBone);

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
            osgAnimation::UpdateTransform* pUpdate = new osgAnimation::UpdateTransform(animName);

            osg::Vec3 trans, rot, scale;
            getApproximateTransform(pNode, trans, rot, scale);

            pUpdate->getPosition()->setValue(trans);
            pUpdate->getEuler()->setValue(rot);
            pUpdate->getScale()->setValue(scale);
            pTransform->setUpdateCallback(pUpdate);
        }
        return pTransform;
    }
}

osgDB::ReaderWriter::ReadResult readFbxNode(
    KFbxSdkManager& pSdkManager, KFbxNode* pNode,
    osg::ref_ptr<osgAnimation::AnimationManagerBase>& pAnimationManager,
    const std::string& dir, bool& bNeedSkeleton, int& nLightCount)
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

    KFbxNodeAttribute::EAttributeType lAttributeType = KFbxNodeAttribute::eUNIDENTIFIED;
    if (pNode->GetNodeAttribute())
    {
        lAttributeType = pNode->GetNodeAttribute()->GetAttributeType();
        if (lAttributeType == KFbxNodeAttribute::eSKELETON)
        {
            bNeedSkeleton = true;
        }
    }

    unsigned nMaterials = pNode->GetMaterialCount();
    std::vector<osg::ref_ptr<osg::Material>> materialList;
    std::vector<osg::ref_ptr<osg::Texture>> textureList;
    materialList.reserve(nMaterials);

    typedef FbxToOsgMap<KFbxSurfaceMaterial, osg::Material, GetOsgMaterial> MaterialMap;
    MaterialMap materialMap(dir);

    for (unsigned i = 0; i < nMaterials; ++i)
    {
        materialList.push_back(materialMap.Get(pNode->GetMaterial(i)));
        textureList.push_back(materialMap.m_convertFunc.m_pTexture);
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

        bool bChildNeedSkeleton = false;
        osgDB::ReaderWriter::ReadResult childResult = readFbxNode(
            pSdkManager, pChildNode, pAnimationManager, dir,
            bChildNeedSkeleton, nLightCount);
        if (childResult.error())
        {
            return childResult;
        }
        else if (osg::Node* osgChild = childResult.getNode())
        {
            if (bChildNeedSkeleton)
            {
                bNeedSkeleton = true;
                skeletal.push_back(osgChild);
            }
            else
            {
                children.push_back(osgChild);
            }
        }
    }

    std::string animName;
    
    if (bNeedSkeleton)
    {
        animName = readFbxBoneAnimation(pNode, pAnimationManager,
            pNode->GetName());
    }
    else
    {
        animName = readFbxAnimation(pNode, pAnimationManager, pNode->GetName());
    }

    osg::Matrix localMatrix;
    makeLocalMatrix(pNode, localMatrix);
    bool bLocalMatrixIdentity = localMatrix.isIdentity();

    osg::ref_ptr<osg::Group> osgGroup;

    bool bEmpty = children.empty() && !bNeedSkeleton;

    switch (lAttributeType)
    {
    case KFbxNodeAttribute::eUNIDENTIFIED:
        if (children.size() + skeletal.size() == 1)
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
            osgDB::ReaderWriter::ReadResult meshRes = readFbxMesh(pNode,
                pAnimationManager, materialList, textureList);
            if (meshRes.error())
            {
                return meshRes;
            }
            else if (osg::Node* node = meshRes.getNode())
            {
                bEmpty = false;
                if (animName.empty() &&
                    children.empty() &&
                    skeletal.empty() &&
                    bLocalMatrixIdentity)
                {
                    return osgDB::ReaderWriter::ReadResult(node);
                }
                osgGroup = createGroupNode(pSdkManager, pNode, animName, localMatrix, bNeedSkeleton);
                osgGroup->getOrCreateStateSet()->setMode(GL_RESCALE_NORMAL,osg::StateAttribute::ON);

                osgGroup->addChild(node);
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
                    osgGroup = createGroupNode(pSdkManager, pNode, animName,
                        localMatrix, bNeedSkeleton);
                    osgGroup->addChild(resGroup);
                }
            }
        }
        break;
    }

    if (bEmpty)
    {
        osgDB::ReaderWriter::ReadResult(0);
    }

    if (!osgGroup) osgGroup = createGroupNode(pSdkManager, pNode, animName, localMatrix, bNeedSkeleton);
    for (osg::NodeList::iterator it = skeletal.begin(); it != skeletal.end(); ++it)
    {
        osgGroup->addChild(it->get());
    }
    for (osg::NodeList::iterator it = children.begin(); it != children.end(); ++it)
    {
        osgGroup->addChild(it->get());
    }

    return osgDB::ReaderWriter::ReadResult(osgGroup.get());
}
