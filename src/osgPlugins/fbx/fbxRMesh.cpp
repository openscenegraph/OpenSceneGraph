#include <osg/Geode>
#include <osg/Image>
#include <osg/MatrixTransform>

#include <osgUtil/TriStripVisitor>

#include <osgDB/ReadFile>

#include <osgAnimation/RigGeometry>
#include <osgAnimation/MorphGeometry>
#include <osgAnimation/BasicAnimationManager>

#if defined(_MSC_VER)
#pragma warning( disable : 4505 )
#endif
#include <fbxsdk.h>

#include "fbxRMesh.h"

enum GeometryType
{
    GEOMETRY_STATIC,
    GEOMETRY_RIG,
    GEOMETRY_MORPH
};

osg::Vec3 convertVec3(const KFbxVector4& v)
{
    return osg::Vec3(
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]));
}

osg::Vec2 convertVec2(const KFbxVector2& v)
{
    return osg::Vec2(
        static_cast<float>(v[0]),
        static_cast<float>(v[1]));
}

osg::Vec4 convertColor(const KFbxColor& color)
{
    return osg::Vec4(
        static_cast<float>(color.mRed),
        static_cast<float>(color.mGreen),
        static_cast<float>(color.mBlue),
        static_cast<float>(color.mAlpha));
}

template <typename T>
bool layerElementValid(const KFbxLayerElementTemplate<T>* pLayerElement)
{
    if (!pLayerElement)
        return false;

    switch (pLayerElement->GetMappingMode())
    {
    case KFbxLayerElement::eBY_CONTROL_POINT:
    case KFbxLayerElement::eBY_POLYGON_VERTEX:
    case KFbxLayerElement::eBY_POLYGON:
        break;
    default:
        return false;
    }

    switch (pLayerElement->GetReferenceMode())
    {
    case KFbxLayerElement::eDIRECT:
    case KFbxLayerElement::eINDEX_TO_DIRECT:
        return true;
    }

    return false;
}

template <typename T>
int getVertexIndex(const KFbxLayerElementTemplate<T>* pLayerElement,
    KFbxMesh* fbxMesh,
    int nPolygon, int nPolyVertex, int nMeshVertex)
{
    int index = 0;

    switch (pLayerElement->GetMappingMode())
    {
    case KFbxLayerElement::eBY_CONTROL_POINT:
        index = fbxMesh->GetPolygonVertex(nPolygon, nPolyVertex);
        break;
    case KFbxLayerElement::eBY_POLYGON_VERTEX:
        index = nMeshVertex;
        break;
    case KFbxLayerElement::eBY_POLYGON:
        index = nPolygon;
        break;
    }

    if (pLayerElement->GetReferenceMode() == KFbxLayerElement::eDIRECT)
    {
        return index;
    }

    return pLayerElement->GetIndexArray().GetAt(index);
}

template <typename T>
int getPolygonIndex(const KFbxLayerElementTemplate<T>* pLayerElement, int nPolygon)
{
    if (pLayerElement &&
        pLayerElement->GetMappingMode() == KFbxLayerElement::eBY_POLYGON)
    {
        switch (pLayerElement->GetReferenceMode())
        {
        case KFbxLayerElement::eDIRECT:
            return nPolygon;
        case KFbxLayerElement::eINDEX_TO_DIRECT:
            return pLayerElement->GetIndexArray().GetAt(nPolygon);
        }
    }

    return 0;
}

template <typename FbxT>
FbxT getElement(const KFbxLayerElementTemplate<FbxT>* pLayerElement,
    KFbxMesh* fbxMesh,
    int nPolygon, int nPolyVertex, int nMeshVertex)
{
    return pLayerElement->GetDirectArray().GetAt(getVertexIndex(
        pLayerElement, fbxMesh, nPolygon, nPolyVertex, nMeshVertex));
}

typedef std::map<unsigned, osg::ref_ptr<osg::Geometry> > GeometryMap;

osg::Geometry* getGeometry(osg::Geode* pGeode, GeometryMap& geometryMap,
    const std::vector<osg::ref_ptr<osg::Material>>& materialList,
    const std::vector<osg::ref_ptr<osg::Texture>>& textureList,
    GeometryType gt, unsigned mti, bool bNormal, bool bTexCoord, bool bColor)
{
    GeometryMap::iterator it = geometryMap.find(mti);

    if (it != geometryMap.end())
    {
        return it->second.get();
    }

    osg::ref_ptr<osg::Geometry> pGeometry;
    if (gt == GEOMETRY_RIG)
    {
        osgAnimation::RigGeometry* pRig = new osgAnimation::RigGeometry;
        pRig->setInfluenceMap(new osgAnimation::VertexInfluenceMap);
        pGeometry = pRig;
    }
    else if (gt == GEOMETRY_MORPH)
    {
        pGeometry = new osgAnimation::MorphGeometry;
    }
    else
    {
        pGeometry = new osg::Geometry;
    }

    pGeometry->setVertexData(osg::Geometry::ArrayData(new osg::Vec3Array, osg::Geometry::BIND_PER_VERTEX));
    if (bNormal) pGeometry->setNormalData(osg::Geometry::ArrayData(new osg::Vec3Array, osg::Geometry::BIND_PER_VERTEX));
    if (bTexCoord) pGeometry->setTexCoordData(0, osg::Geometry::ArrayData(new osg::Vec2Array, osg::Geometry::BIND_PER_VERTEX));
    if (bColor) pGeometry->setColorData(osg::Geometry::ArrayData(new osg::Vec4Array, osg::Geometry::BIND_PER_VERTEX));

    if (mti < materialList.size())
    {
        pGeometry->getOrCreateStateSet()->setAttributeAndModes(materialList[mti].get());
    }

    if (mti < textureList.size())
    {
        pGeometry->getOrCreateStateSet()->setTextureAttributeAndModes(0, textureList[mti].get());
    }

    geometryMap.insert(std::pair<unsigned, osg::ref_ptr<osg::Geometry>>(mti, pGeometry));
    pGeode->addDrawable(pGeometry.get());

    return pGeometry.get();
}

osgAnimation::VertexInfluence& getVertexInfluence(
    osgAnimation::VertexInfluenceMap& vim, const std::string& name)
{
    osgAnimation::VertexInfluenceMap::iterator it = vim.lower_bound(name);
    if (it == vim.end() || name != it->first)
    {
        it = vim.insert(it, osgAnimation::VertexInfluenceMap::value_type(
            name, osgAnimation::VertexInfluence()));
        it->second.setName(name);
    }
    return it->second;
}

void addChannel(
    osgAnimation::Channel* pChannel,
    osg::ref_ptr<osgAnimation::AnimationManagerBase> &pAnimManager,
    const char* pTakeName)
{
    if (!pChannel)
    {
        return;
    }

    if (!pAnimManager) pAnimManager = new osgAnimation::BasicAnimationManager;

    osgAnimation::Animation* pAnimation = 0;
    const osgAnimation::AnimationList& anims = pAnimManager->getAnimationList();
    for (size_t i = 0; i < anims.size(); ++i)
    {
        if (anims[i]->getName() == pTakeName)
        {
            pAnimation = anims[i].get();
        }
    }

    if (!pAnimation)
    {
        pAnimation = new osgAnimation::Animation;
        pAnimation->setName(pTakeName);
        pAnimManager->registerAnimation(pAnimation);
    }

    pAnimation->addChannel(pChannel);
}

void readAnimation(KFbxNode* pNode, osg::Geode* pGeode,
    osg::ref_ptr<osgAnimation::AnimationManagerBase>& pAnimationManager,
    KFbxMesh* pMesh, int nShape)
{
    for (int i = 1; i < pNode->GetTakeNodeCount(); ++i)
    {
        const char* pTakeName = pNode->GetTakeNodeName(i);

        KFCurve* pCurve = pMesh->GetShapeChannel(nShape, true, pTakeName);

        osgAnimation::FloatLinearChannel* pChannel = new osgAnimation::FloatLinearChannel;
        std::vector<osgAnimation::TemplateKeyframe<float> >& keyFrameCntr = *pChannel->getOrCreateSampler()->getOrCreateKeyframeContainer();

        int nKeys = pCurve->KeyGetCount();
        if (!nKeys)
        {
            float fValue = static_cast<float>(pCurve->GetValue() * 0.01);
            keyFrameCntr.push_back(osgAnimation::FloatKeyframe(0.0f,fValue));
        }

        for (int k = 0; k < nKeys; ++k)
        {
            KFCurveKey key = pCurve->KeyGet(k);
            float fTime = static_cast<float>(key.GetTime().GetSecondDouble());
            float fValue = static_cast<float>(key.GetValue() * 0.01);
            keyFrameCntr.push_back(osgAnimation::FloatKeyframe(fTime,fValue));
        }

        pChannel->setTargetName(pGeode->getName());
        pChannel->setName(pMesh->GetShapeName(nShape));
        addChannel(pChannel, pAnimationManager, pTakeName);
    }
}

osgDB::ReaderWriter::ReadResult readMesh(KFbxNode* pNode, KFbxMesh* fbxMesh,
    osg::ref_ptr<osgAnimation::AnimationManagerBase>& pAnimationManager,
    const std::vector<osg::ref_ptr<osg::Material>>& materialList,
    const std::vector<osg::ref_ptr<osg::Texture>>& textureList,
    const char* szName)
{
    GeometryMap geometryMap;

    osg::Geode* pGeode = new osg::Geode;
    pGeode->setName(szName);

    const KFbxLayer* pFbxLayer = 0;
    const KFbxLayerElementNormal* pFbxNormals = 0;
    const KFbxLayerElementUV* pFbxUVs = 0;
    const KFbxLayerElementVertexColor* pFbxColors = 0;
    const KFbxLayerElementMaterial* pFbxMaterials = 0;

    const KFbxVector4* pFbxVertices = fbxMesh->GetControlPoints();

    if (pFbxLayer = fbxMesh->GetLayer(0))
    {
        pFbxNormals = pFbxLayer->GetNormals();
        pFbxColors = pFbxLayer->GetVertexColors();
        pFbxUVs = pFbxLayer->GetUVs();
        pFbxMaterials = pFbxLayer->GetMaterials();

        if (!layerElementValid(pFbxNormals)) pFbxNormals = 0;
        if (!layerElementValid(pFbxColors)) pFbxColors = 0;
        if (!layerElementValid(pFbxUVs)) pFbxUVs = 0;
    }

    int nPolys = fbxMesh->GetPolygonCount();

    int nDeformerCount = fbxMesh->GetDeformerCount(KFbxDeformer::eSKIN);
    int nMorphShapeCount = 0;

    GeometryType geomType = GEOMETRY_STATIC;

    //determine the type of geometry
    if (nDeformerCount)
    {
        geomType = GEOMETRY_RIG;
    }
    else if (nMorphShapeCount = fbxMesh->GetShapeCount())
    {
        geomType = GEOMETRY_MORPH;
    }

    typedef std::pair<osg::Geometry*, int> GIPair;
    typedef std::multimap<int, GIPair> FbxToOsgVertexMap;
    typedef std::map<GIPair, int> OsgToFbxNormalMap;
    FbxToOsgVertexMap fbxToOsgVertMap;
    OsgToFbxNormalMap osgToFbxNormMap;

    for (int i = 0, nVertex = 0; i < nPolys; ++i)
    {
        int lPolygonSize = fbxMesh->GetPolygonSize(i);

        int materialIndex = getPolygonIndex(pFbxMaterials, i);

        osg::Geometry* pGeometry = getGeometry(pGeode, geometryMap,
            materialList, textureList, geomType, materialIndex,
            pFbxNormals != 0, pFbxUVs != 0, pFbxColors != 0);

        osg::Vec3Array* pVertices = static_cast<osg::Vec3Array*>(
            pGeometry->getVertexArray());
        osg::Vec3Array* pNormals = static_cast<osg::Vec3Array*>(
            pGeometry->getNormalArray());
        osg::Vec2Array* pTexCoords = static_cast<osg::Vec2Array*>(
            pGeometry->getTexCoordArray(0));
        osg::Vec4Array* pColors = static_cast<osg::Vec4Array*>(
            pGeometry->getColorArray());

        int nVertex0 = nVertex;
        nVertex += (std::min)(2, lPolygonSize);

        //convert polygon to triangles
        for (int j = 2; j < lPolygonSize; ++j, ++nVertex)
        {
            int v0 = fbxMesh->GetPolygonVertex(i, 0),
                v1 = fbxMesh->GetPolygonVertex(i, j - 1),
                v2 = fbxMesh->GetPolygonVertex(i, j);

            fbxToOsgVertMap.insert(FbxToOsgVertexMap::value_type(v0, GIPair(pGeometry, pVertices->size())));
            fbxToOsgVertMap.insert(FbxToOsgVertexMap::value_type(v1, GIPair(pGeometry, pVertices->size() + 1)));
            fbxToOsgVertMap.insert(FbxToOsgVertexMap::value_type(v2, GIPair(pGeometry, pVertices->size() + 2)));


            pVertices->push_back(convertVec3(pFbxVertices[v0]));
            pVertices->push_back(convertVec3(pFbxVertices[v1]));
            pVertices->push_back(convertVec3(pFbxVertices[v2]));

            if (pNormals)
            {
                int n0 = getVertexIndex(pFbxNormals, fbxMesh, i, 0, nVertex0);
                int n1 = getVertexIndex(pFbxNormals, fbxMesh, i, j - 1, nVertex - 1);
                int n2 = getVertexIndex(pFbxNormals, fbxMesh, i, j, nVertex);

                osgToFbxNormMap.insert(OsgToFbxNormalMap::value_type(GIPair(pGeometry, pNormals->size()), n0));
                osgToFbxNormMap.insert(OsgToFbxNormalMap::value_type(GIPair(pGeometry, pNormals->size() + 1), n1));
                osgToFbxNormMap.insert(OsgToFbxNormalMap::value_type(GIPair(pGeometry, pNormals->size() + 2), n2));

                pNormals->push_back(convertVec3(pFbxNormals->GetDirectArray().GetAt(n0)));
                pNormals->push_back(convertVec3(pFbxNormals->GetDirectArray().GetAt(n1)));
                pNormals->push_back(convertVec3(pFbxNormals->GetDirectArray().GetAt(n2)));
            }

            if (pTexCoords)
            {
                pTexCoords->push_back(convertVec2(getElement(pFbxUVs, fbxMesh, i, 0, nVertex0)));
                pTexCoords->push_back(convertVec2(getElement(pFbxUVs, fbxMesh, i, j - 1, nVertex - 1)));
                pTexCoords->push_back(convertVec2(getElement(pFbxUVs, fbxMesh, i, j, nVertex)));
            }

            if (pColors)
            {
                pColors->push_back(convertColor(getElement(pFbxColors, fbxMesh, i, 0, nVertex0)));
                pColors->push_back(convertColor(getElement(pFbxColors, fbxMesh, i, j - 1, nVertex - 1)));
                pColors->push_back(convertColor(getElement(pFbxColors, fbxMesh, i, j, nVertex)));
            }
        }
    }

    for (int i = 0; i < pGeode->getNumDrawables(); ++i)
    {
        osg::Geometry* pGeometry = pGeode->getDrawable(i)->asGeometry();
        pGeometry->setName(pGeode->getName());

        osg::DrawArrays* pDrawArrays = new osg::DrawArrays(
            GL_TRIANGLES, 0, pGeometry->getVertexArray()->getNumElements());
        pGeometry->addPrimitiveSet(pDrawArrays);
    }

    if (geomType == GEOMETRY_RIG)
    {
        for (int i = 0; i < nDeformerCount; ++i)
        {
            KFbxSkin* pSkin = (KFbxSkin*)fbxMesh->GetDeformer(i, KFbxDeformer::eSKIN);
            int nClusters = pSkin->GetClusterCount();
            for (int j = 0; j < nClusters; ++j)
            {
                KFbxCluster* pCluster = (KFbxCluster*)pSkin->GetCluster(j);
                KFbxNode* pBone = pCluster->GetLink();

                int nIndices = pCluster->GetControlPointIndicesCount();
                int* pIndices = pCluster->GetControlPointIndices();
                double* pWeights = pCluster->GetControlPointWeights();

                for (int k = 0; k < nIndices; ++k)
                {
                    int fbxIndex = pIndices[k];
                    float weight = static_cast<float>(pWeights[k]);

                    for (FbxToOsgVertexMap::const_iterator it =
                        fbxToOsgVertMap.find(fbxIndex);
                        it != fbxToOsgVertMap.end() &&
                        it->first == fbxIndex; ++it)
                    {
                        GIPair gi = it->second;
                        osgAnimation::RigGeometry& rig = dynamic_cast<osgAnimation::RigGeometry&>(*gi.first);
                        osgAnimation::VertexInfluenceMap& vim = *rig.getInfluenceMap();
                        osgAnimation::VertexInfluence& vi = getVertexInfluence(vim, pBone->GetName());
                        vi.push_back(osgAnimation::VertexIndexWeight(
                            gi.second, weight));
                    }
                }
            }
        }
    }
    else if (geomType == GEOMETRY_MORPH)
    {
        pGeode->addUpdateCallback(new osgAnimation::UpdateMorph(pGeode->getName()));


        for (int i = 0; i < pGeode->getNumDrawables(); ++i)
        {
            osg::Geometry* pGeometry = pGeode->getDrawable(i)->asGeometry();

            osgAnimation::MorphGeometry& morph = dynamic_cast<osgAnimation::MorphGeometry&>(*pGeometry);

            //read morph geometry
            for (int j = 0; j < nMorphShapeCount; ++j)
            {
                const KFbxGeometryBase* pMorphShape = fbxMesh->GetShape(i);

                const KFbxLayerElementNormal* pFbxShapeNormals = 0;
                if (const KFbxLayer* pFbxShapeLayer = pMorphShape->GetLayer(0))
                {
                    pFbxShapeNormals = pFbxShapeLayer->GetNormals();
                    if (!layerElementValid(pFbxShapeNormals)) pFbxShapeNormals = 0;
                }

                osg::Geometry* pMorphTarget = new osg::Geometry(morph);
                pMorphTarget->setVertexArray(static_cast<osg::Array*>(
                    pMorphTarget->getVertexArray()->clone(osg::CopyOp::DEEP_COPY_ARRAYS)));
                if (pFbxShapeNormals)
                {
                    if (osg::Array* pNormals = pMorphTarget->getNormalArray())
                    {
                        pMorphTarget->setNormalArray(static_cast<osg::Array*>(
                            pNormals->clone(osg::CopyOp::DEEP_COPY_ARRAYS)));
                    }
                }
                pMorphTarget->setName(fbxMesh->GetShapeName(j));
                KFCurve* pCurve = fbxMesh->GetShapeChannel(j);
                double defaultWeight = pCurve->GetValue() * 0.01;
                morph.addMorphTarget(pMorphTarget, static_cast<float>(defaultWeight));

                readAnimation(pNode, pGeode, pAnimationManager, fbxMesh, j);
            }
        }

        for (int i = 0; i < nMorphShapeCount; ++i)
        {
            const KFbxGeometryBase* pMorphShape = fbxMesh->GetShape(i);

            const KFbxLayerElementNormal* pFbxShapeNormals = 0;
            if (const KFbxLayer* pFbxShapeLayer = pMorphShape->GetLayer(0))
            {
                pFbxShapeNormals = pFbxShapeLayer->GetNormals();
                if (!layerElementValid(pFbxShapeNormals)) pFbxShapeNormals = 0;
            }

            const KFbxVector4* pControlPoints = pMorphShape->GetControlPoints();
            int nControlPoints = pMorphShape->GetControlPointsCount();
            for (int fbxIndex = 0; fbxIndex < nControlPoints; ++fbxIndex)
            {
                osg::Vec3 vPos = convertVec3(pControlPoints[fbxIndex]);
                for (FbxToOsgVertexMap::const_iterator it =
                    fbxToOsgVertMap.find(fbxIndex);
                    it != fbxToOsgVertMap.end() &&
                    it->first == fbxIndex; ++it)
                {
                    GIPair gi = it->second;
                    osgAnimation::MorphGeometry& morphGeom =
                        dynamic_cast<osgAnimation::MorphGeometry&>(*gi.first);
                    osg::Geometry* pGeometry = morphGeom.getMorphTarget(i).getGeometry();
                    osg::Vec3Array* pVertices = static_cast<osg::Vec3Array*>(pGeometry->getVertexArray());
                    (*pVertices)[gi.second] = vPos;

                    if (pFbxShapeNormals)
                    {
                        if (osg::Vec3Array* pNormals = static_cast<osg::Vec3Array*>(pGeometry->getNormalArray()))
                        {
                            (*pNormals)[gi.second] = convertVec3(
                                pFbxShapeNormals->GetDirectArray().GetAt(osgToFbxNormMap[gi]));
                        }
                    }
                }
            }
        }
    }

    KFbxXMatrix fbxVertexTransform;
    fbxVertexTransform.SetTRS(
        pNode->GetGeometricTranslation(KFbxNode::eSOURCE_SET),
        pNode->GetGeometricRotation(KFbxNode::eSOURCE_SET),
        pNode->GetGeometricScaling(KFbxNode::eSOURCE_SET));
    const double* pVertexMat = fbxVertexTransform;
    osg::Matrix vertexMat(pVertexMat);

    if (vertexMat.isIdentity())
    {
        return osgDB::ReaderWriter::ReadResult(pGeode);
    }
    else
    {
        osg::MatrixTransform* pMatTrans = new osg::MatrixTransform(vertexMat);
        pMatTrans->addChild(pGeode);
        return osgDB::ReaderWriter::ReadResult(pMatTrans);
    }
}

osgDB::ReaderWriter::ReadResult readFbxMesh(KFbxNode* pNode,
    osg::ref_ptr<osgAnimation::AnimationManagerBase>& pAnimationManager,
    const std::vector<osg::ref_ptr<osg::Material>>& materialList,
    const std::vector<osg::ref_ptr<osg::Texture>>& textureList)
{
    KFbxMesh* lMesh = dynamic_cast<KFbxMesh*>(pNode->GetNodeAttribute());

    if (!lMesh)
    {
        return osgDB::ReaderWriter::ReadResult::ERROR_IN_READING_FILE;
    }

    return readMesh(pNode, lMesh, pAnimationManager, materialList, textureList, pNode->GetName());
}
