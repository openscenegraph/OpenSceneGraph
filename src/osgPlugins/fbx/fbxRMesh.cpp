#include <cassert>
#include <sstream>

#include <osg/BlendFunc>
#include <osg/Geode>
#include <osg/Image>
#include <osg/MatrixTransform>
#include <osg/TexMat>
#include <osg/TexGen>
#include <osg/TexEnvCombine>

#include <osgUtil/TriStripVisitor>
#include <osgUtil/Tessellator>

#include <osgDB/ReadFile>

#include <osgAnimation/RigGeometry>
#include <osgAnimation/MorphGeometry>
#include <osgAnimation/BasicAnimationManager>

#if defined(_MSC_VER)
#pragma warning( disable : 4505 )
#pragma warning( default : 4996 )
#endif
#include <fbxsdk.h>

#include "fbxReader.h"

enum GeometryType
{
    GEOMETRY_STATIC,
    GEOMETRY_RIG,
    GEOMETRY_MORPH
};

osg::Vec3d convertVec3(const FbxVector4& v)
{
    return osg::Vec3d(
        v[0],
        v[1],
        v[2]);
}

template <typename T>
bool layerElementValid(const FbxLayerElementTemplate<T>* pLayerElement)
{
    if (!pLayerElement)
        return false;

    switch (pLayerElement->GetMappingMode())
    {
    case FbxLayerElement::eByControlPoint:
    case FbxLayerElement::eByPolygonVertex:
    case FbxLayerElement::eByPolygon:
        break;
    default:
        return false;
    }

    switch (pLayerElement->GetReferenceMode())
    {
    case FbxLayerElement::eDirect:
    case FbxLayerElement::eIndexToDirect:
        return true;
    default:
        break;
    }

    return false;
}

template <typename T>
int getVertexIndex(const FbxLayerElementTemplate<T>* pLayerElement,
    const FbxMesh* fbxMesh,
    int nPolygon, int nPolyVertex, int nMeshVertex)
{
    int index = 0;

    switch (pLayerElement->GetMappingMode())
    {
    case FbxLayerElement::eByControlPoint:
        index = fbxMesh->GetPolygonVertex(nPolygon, nPolyVertex);
        break;
    case FbxLayerElement::eByPolygonVertex:
        index = nMeshVertex;
        break;
    case FbxLayerElement::eByPolygon:
        index = nPolygon;
        break;
    default:
        OSG_WARN << "getVertexIndex: unsupported FBX mapping mode" << std::endl;
    }

    if (pLayerElement->GetReferenceMode() == FbxLayerElement::eDirect)
    {
        return index;
    }

    return pLayerElement->GetIndexArray().GetAt(index);
}

template <typename T>
int getPolygonIndex(const FbxLayerElementTemplate<T>* pLayerElement, int nPolygon)
{
    if (pLayerElement &&
        pLayerElement->GetMappingMode() == FbxLayerElement::eByPolygon)
    {
        switch (pLayerElement->GetReferenceMode())
        {
        case FbxLayerElement::eDirect:
            return nPolygon;
        case FbxLayerElement::eIndexToDirect:
            return pLayerElement->GetIndexArray().GetAt(nPolygon);
        default:
            break;
        }
    }

    return 0;
}

template <typename FbxT>
FbxT getElement(const FbxLayerElementTemplate<FbxT>* pLayerElement,
    const FbxMesh* fbxMesh,
    int nPolygon, int nPolyVertex, int nMeshVertex)
{
    return pLayerElement->GetDirectArray().GetAt(getVertexIndex(
        pLayerElement, fbxMesh, nPolygon, nPolyVertex, nMeshVertex));
}

typedef std::map<unsigned, osg::ref_ptr<osg::Geometry> > GeometryMap;

osg::Array* createVec2Array(bool doublePrecision)
{
    if  (doublePrecision) return new osg::Vec2dArray;
    else return new osg::Vec2Array;
}
osg::Array* createVec3Array(bool doublePrecision)
{
    if  (doublePrecision) return new osg::Vec3dArray;
    else return new osg::Vec3Array;
}
osg::Array* createVec4Array(bool doublePrecision)
{
    if  (doublePrecision) return new osg::Vec4dArray;
    else return new osg::Vec4Array;
}

osg::Geometry* getGeometry(osg::Geode* pGeode, GeometryMap& geometryMap,
    std::vector<StateSetContent>& stateSetList,
    GeometryType gt,
    unsigned int mti,
    bool bNormal,
    bool useDiffuseMap,
    bool useOpacityMap,
    bool useEmissiveMap,
    bool useAmbientMap,
    // more here...
    bool bColor,
    const osgDB::Options& options,
    bool lightmapTextures)
{
    GeometryMap::iterator it = geometryMap.find(mti);

    if (it != geometryMap.end())
    {
        return it->second.get();
    }

    osg::ref_ptr<osg::Geometry> pGeometry;
    if (gt == GEOMETRY_MORPH)
    {
        pGeometry = new osgAnimation::MorphGeometry;
    }
    else
    {
        pGeometry = new osg::Geometry;
    }

    osgDB::Options::PrecisionHint precision = options.getPrecisionHint();

    pGeometry->setVertexArray(createVec3Array((precision & osgDB::Options::DOUBLE_PRECISION_VERTEX) != 0));
    if (bNormal) pGeometry->setNormalArray(createVec3Array((precision & osgDB::Options::DOUBLE_PRECISION_NORMAL) != 0), osg::Array::BIND_PER_VERTEX);

    // create as much textures coordinates as needed...
    if (useDiffuseMap)
        pGeometry->setTexCoordArray(StateSetContent::DIFFUSE_TEXTURE_UNIT, createVec2Array((precision & osgDB::Options::DOUBLE_PRECISION_TEX_COORD) != 0), osg::Array::BIND_PER_VERTEX);
    if (useOpacityMap)
        pGeometry->setTexCoordArray(StateSetContent::OPACITY_TEXTURE_UNIT, createVec2Array((precision & osgDB::Options::DOUBLE_PRECISION_TEX_COORD) != 0), osg::Array::BIND_PER_VERTEX);
    if (useEmissiveMap)
        pGeometry->setTexCoordArray(StateSetContent::EMISSIVE_TEXTURE_UNIT, createVec2Array((precision & osgDB::Options::DOUBLE_PRECISION_TEX_COORD) != 0), osg::Array::BIND_PER_VERTEX);
    if (useAmbientMap)
        pGeometry->setTexCoordArray(StateSetContent::AMBIENT_TEXTURE_UNIT, createVec2Array((precision & osgDB::Options::DOUBLE_PRECISION_TEX_COORD) != 0), osg::Array::BIND_PER_VERTEX);
    // create more textures coordinates here...

    if (bColor) pGeometry->setColorArray(createVec4Array((precision & osgDB::Options::DOUBLE_PRECISION_COLOR) != 0), osg::Array::BIND_PER_VERTEX);

    if (mti < stateSetList.size())
    {
        osg::StateSet* stateSet = pGeometry->getOrCreateStateSet();

        bool transparent = false;
        const StateSetContent& ssc = stateSetList[mti];

        // set material...
        if (osg::Material* pMaterial = ssc.material.get())
        {
            stateSet->setAttributeAndModes(pMaterial);
            transparent = pMaterial->getDiffuse(osg::Material::FRONT).w() < 1.0f;
        }

        // diffuse texture map...
        if (ssc.diffuseTexture)
        {
            stateSet->setTextureAttributeAndModes(StateSetContent::DIFFUSE_TEXTURE_UNIT, ssc.diffuseTexture.get());

            if (ssc.diffuseScaleU != 1.0 || ssc.diffuseScaleV != 1.0)
            {
                // set UV scaling...
                osg::ref_ptr<osg::TexMat> texmat = new osg::TexMat();
                osg::Matrix uvScaling;
                uvScaling.makeScale(osg::Vec3(ssc.diffuseScaleU, ssc.diffuseScaleV, 1.0));
                texmat->setMatrix(uvScaling);
                stateSet->setTextureAttributeAndModes(StateSetContent::DIFFUSE_TEXTURE_UNIT, texmat.get(), osg::StateAttribute::ON);
            }

            if (lightmapTextures)
            {
                double factor = ssc.diffuseFactor;
                osg::ref_ptr<osg::TexEnvCombine> texenv = new osg::TexEnvCombine();
                texenv->setCombine_RGB(osg::TexEnvCombine::INTERPOLATE);
                texenv->setSource0_RGB(osg::TexEnvCombine::TEXTURE);
                texenv->setSource1_RGB(osg::TexEnvCombine::PREVIOUS);
                texenv->setSource2_RGB(osg::TexEnvCombine::CONSTANT);
                texenv->setConstantColor(osg::Vec4(factor, factor, factor, factor));
                stateSet->setTextureAttributeAndModes(StateSetContent::DIFFUSE_TEXTURE_UNIT, texenv.get(), osg::StateAttribute::ON);
            }

            // setup transparency
            if (!transparent && ssc.diffuseTexture->getImage())
                transparent = ssc.diffuseTexture->getImage()->isImageTranslucent();
        }

        // opacity texture map...
        if (ssc.opacityTexture)
        {
            stateSet->setTextureAttributeAndModes(StateSetContent::OPACITY_TEXTURE_UNIT, ssc.opacityTexture.get());

            if (ssc.opacityScaleU != 1.0 || ssc.opacityScaleV != 1.0)
            {
                // set UV scaling...
                osg::ref_ptr<osg::TexMat> texmat = new osg::TexMat();
                osg::Matrix uvScaling;
                uvScaling.makeScale(osg::Vec3(ssc.opacityScaleU, ssc.opacityScaleV, 1.0));
                texmat->setMatrix(uvScaling);
                stateSet->setTextureAttributeAndModes(StateSetContent::OPACITY_TEXTURE_UNIT, texmat.get(), osg::StateAttribute::ON);
            }

            // setup combiner to ignore RGB...
            osg::ref_ptr<osg::TexEnvCombine> texenv = new osg::TexEnvCombine();
            texenv->setCombine_RGB(osg::TexEnvCombine::REPLACE);
            texenv->setSource0_RGB(osg::TexEnvCombine::PREVIOUS);
            stateSet->setTextureAttributeAndModes(StateSetContent::OPACITY_TEXTURE_UNIT, texenv.get(), osg::StateAttribute::ON);

            // setup transparency...
            if (!transparent && ssc.opacityTexture->getImage())
                transparent = ssc.opacityTexture->getImage()->isImageTranslucent();
        }

        // reflection texture map...
        if (ssc.reflectionTexture)
        {
            stateSet->setTextureAttributeAndModes(StateSetContent::REFLECTION_TEXTURE_UNIT, ssc.reflectionTexture.get());

            // setup spherical map...
            osg::ref_ptr<osg::TexGen> texgen = new osg::TexGen();
            texgen->setMode(osg::TexGen::SPHERE_MAP);
            stateSet->setTextureAttributeAndModes(StateSetContent::REFLECTION_TEXTURE_UNIT, texgen.get(), osg::StateAttribute::ON);

            // setup combiner for factor...
            double factor = ssc.reflectionFactor;
            osg::ref_ptr<osg::TexEnvCombine> texenv = new osg::TexEnvCombine();
            texenv->setCombine_RGB(osg::TexEnvCombine::INTERPOLATE);
            texenv->setSource0_RGB(osg::TexEnvCombine::TEXTURE);
            texenv->setSource1_RGB(osg::TexEnvCombine::PREVIOUS);
            texenv->setSource2_RGB(osg::TexEnvCombine::CONSTANT);
            texenv->setConstantColor(osg::Vec4(factor, factor, factor, factor));
            stateSet->setTextureAttributeAndModes(StateSetContent::REFLECTION_TEXTURE_UNIT, texenv.get(), osg::StateAttribute::ON);
        }

        // emissive texture map
        if (ssc.emissiveTexture)
        {
            if (ssc.emissiveScaleU != 1.0 || ssc.emissiveScaleV != 1.0)
            {
                // set UV scaling...
                osg::ref_ptr<osg::TexMat> texmat = new osg::TexMat();
                osg::Matrix uvScaling;
                uvScaling.makeScale(osg::Vec3(ssc.emissiveScaleU, ssc.emissiveScaleV, 1.0));
                texmat->setMatrix(uvScaling);
                stateSet->setTextureAttributeAndModes(StateSetContent::EMISSIVE_TEXTURE_UNIT, texmat.get(), osg::StateAttribute::ON);
            }

            stateSet->setTextureAttributeAndModes(StateSetContent::EMISSIVE_TEXTURE_UNIT, ssc.emissiveTexture.get());
        }

        // ambient texture map
        if (ssc.ambientTexture)
        {
            if (ssc.ambientScaleU != 1.0 || ssc.ambientScaleV != 1.0)
            {
                // set UV scaling...
                osg::ref_ptr<osg::TexMat> texmat = new osg::TexMat();
                osg::Matrix uvScaling;
                uvScaling.makeScale(osg::Vec3(ssc.ambientScaleU, ssc.ambientScaleV, 1.0));
                texmat->setMatrix(uvScaling);
                stateSet->setTextureAttributeAndModes(StateSetContent::AMBIENT_TEXTURE_UNIT, texmat.get(), osg::StateAttribute::ON);
            }

            stateSet->setTextureAttributeAndModes(StateSetContent::AMBIENT_TEXTURE_UNIT, ssc.ambientTexture.get());
        }

        // add more texture maps here...

        if (transparent)
        {
            stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
            stateSet->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        }
    }

    geometryMap.insert(std::pair<unsigned, osg::ref_ptr<osg::Geometry> >(mti, pGeometry));
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
    osg::ref_ptr<osgAnimation::AnimationManagerBase>& pAnimManager,
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

void readAnimation(FbxNode* pNode, FbxScene& fbxScene, const std::string& targetName,
    osg::ref_ptr<osgAnimation::AnimationManagerBase>& pAnimationManager,
    FbxMesh* pMesh, int nBlendShape, int nBlendShapeChannel, int nShape)
{
    for (int i = 0; i < fbxScene.GetSrcObjectCount<FbxAnimStack>(); ++i)
    {
        FbxAnimStack* pAnimStack = FbxCast<FbxAnimStack>(fbxScene.GetSrcObject<FbxAnimStack>(i));

        int nbAnimLayers = pAnimStack->GetMemberCount<FbxAnimLayer>();

        const char* pTakeName = pAnimStack->GetName();

        if (!pTakeName || !*pTakeName)
            continue;

        for (int j = 0; j < nbAnimLayers; j++)
        {
            FbxAnimLayer* pAnimLayer = pAnimStack->GetMember<FbxAnimLayer>(j);

            FbxAnimCurve* pCurve = pMesh->GetShapeChannel(nBlendShape, nBlendShapeChannel, pAnimLayer, false);

            if (!pCurve)
            {
                continue;
            }

            int nKeys = pCurve->KeyGetCount();
            if (!nKeys)
            {
                continue;
            }

            osgAnimation::FloatLinearChannel* pChannel = new osgAnimation::FloatLinearChannel;
            std::vector<osgAnimation::TemplateKeyframe<float> >& keyFrameCntr = *pChannel->getOrCreateSampler()->getOrCreateKeyframeContainer();

            for (int k = 0; k < nKeys; ++k)
            {
                FbxAnimCurveKey key = pCurve->KeyGet(k);
                double fTime = key.GetTime().GetSecondDouble();
                float fValue = static_cast<float>(key.GetValue() * 0.01);
                keyFrameCntr.push_back(osgAnimation::FloatKeyframe(fTime,fValue));
            }

            pChannel->setTargetName(targetName);
            std::stringstream ss;
            ss << nShape;
            pChannel->setName(ss.str());
            addChannel(pChannel, pAnimationManager, pTakeName);
        }
    }
}

void addBindMatrix(
    BindMatrixMap& boneBindMatrices,
    FbxNode* pBone,
    const osg::Matrix& bindMatrix,
    osgAnimation::RigGeometry* pRigGeometry)
{
    boneBindMatrices.insert(BindMatrixMap::value_type(
        BindMatrixMap::key_type(pBone, pRigGeometry), bindMatrix));
}

void addVec2ArrayElement(osg::Array& a, const FbxVector2& v)
{
    if (a.getType() == osg::Array::Vec2dArrayType)
    {
        static_cast<osg::Vec2dArray&>(a).push_back(osg::Vec2d(v[0], v[1]));
    }
    else
    {
        static_cast<osg::Vec2Array&>(a).push_back(osg::Vec2(
            static_cast<float>(v[0]),
            static_cast<float>(v[1])));
    }
}

void addVec3ArrayElement(osg::Array& a, const FbxVector4& v)
{
    if (a.getType() == osg::Array::Vec3dArrayType)
    {
        static_cast<osg::Vec3dArray&>(a).push_back(osg::Vec3d(v[0], v[1], v[2]));
    }
    else
    {
        static_cast<osg::Vec3Array&>(a).push_back(osg::Vec3(
            static_cast<float>(v[0]),
            static_cast<float>(v[1]),
            static_cast<float>(v[2])));
    }
}

void addColorArrayElement(osg::Array& a, const FbxColor& c)
{
    if (a.getType() == osg::Array::Vec4dArrayType)
    {
        static_cast<osg::Vec4dArray&>(a).push_back(osg::Vec4d(c.mRed, c.mGreen, c.mBlue, c.mAlpha));
    }
    else
    {
        static_cast<osg::Vec4Array&>(a).push_back(osg::Vec4(
            static_cast<float>(c.mRed),
            static_cast<float>(c.mGreen),
            static_cast<float>(c.mBlue),
            static_cast<float>(c.mAlpha)));
    }
}

// scans StateSetList looking for the (first) channel name for the specified map type...
std::string getUVChannelForTextureMap(std::vector<StateSetContent>& stateSetList, const char* pName)
{
    // will return the first occurrence in the state set list...
    // TODO: what if more than one channel for the same map type?
    for (unsigned int i = 0; i < stateSetList.size(); i++)
    {
        if (0 == strcmp(pName, FbxSurfaceMaterial::sDiffuse))
            return stateSetList[i].diffuseChannel;
        if (0 == strcmp(pName, FbxSurfaceMaterial::sTransparentColor))
            return stateSetList[i].opacityChannel;
        if (0 == strcmp(pName, FbxSurfaceMaterial::sReflection))
            return stateSetList[i].reflectionChannel;
        if (0 == strcmp(pName, FbxSurfaceMaterial::sEmissive))
            return stateSetList[i].emissiveChannel;
        if (0 == strcmp(pName, FbxSurfaceMaterial::sAmbient))
            return stateSetList[i].ambientChannel;
        // more here...
    }

    return "";
}

// scans mesh layers looking for the UV element corresponding to the specified channel name...
const FbxLayerElementUV* getUVElementForChannel(std::string uvChannelName,
    FbxLayerElement::EType elementType, FbxMesh* pFbxMesh)
{
    // scan layers for specified UV channel...
    for (int cLayerIndex = 0; cLayerIndex < pFbxMesh->GetLayerCount(); cLayerIndex++)
    {
        const FbxLayer* pFbxLayer = pFbxMesh->GetLayer(cLayerIndex);
        if (!pFbxLayer)
            continue;

        if (const FbxLayerElementUV* uv = pFbxLayer->GetUVs())
        {
            if (0 == uvChannelName.compare(uv->GetName()))
                return uv;
        }
    }

    for (int cLayerIndex = 0; cLayerIndex < pFbxMesh->GetLayerCount(); cLayerIndex++)
    {
        const FbxLayer* pFbxLayer = pFbxMesh->GetLayer(cLayerIndex);
        if (!pFbxLayer)
            continue;

        if (const FbxLayerElementUV* uv = pFbxLayer->GetUVs(elementType))
        {
            return uv;
        }
    }

    return 0;
}

typedef std::pair<osg::Geometry*, int> GIPair;
typedef std::multimap<int, GIPair> FbxToOsgVertexMap;
typedef std::map<GIPair, int> OsgToFbxNormalMap;

void readMeshTriangle(const FbxMesh * fbxMesh, int i /*polygonIndex*/,
                      int posInPoly0, int posInPoly1, int posInPoly2,
                      int meshVertex0, int meshVertex1, int meshVertex2,
                      FbxToOsgVertexMap& fbxToOsgVertMap,
                      OsgToFbxNormalMap& osgToFbxNormMap,
                      const FbxVector4* pFbxVertices,
                      const FbxLayerElementNormal* pFbxNormals,
                      const FbxLayerElementUV* pFbxUVs_diffuse,
                      const FbxLayerElementUV* pFbxUVs_opacity,
                      const FbxLayerElementUV* pFbxUVs_emissive,
                      const FbxLayerElementUV* pFbxUVs_ambient,
                      const FbxLayerElementVertexColor* pFbxColors,
                      osg::Geometry* pGeometry,
                      osg::Array* pVertices,
                      osg::Array* pNormals,
                      osg::Array* pTexCoords_diffuse,
                      osg::Array* pTexCoords_opacity,
                      osg::Array* pTexCoords_emissive,
                      osg::Array* pTexCoords_ambient,
                      osg::Array* pColors)
{
    int v0 = fbxMesh->GetPolygonVertex(i, posInPoly0),
        v1 = fbxMesh->GetPolygonVertex(i, posInPoly1),
        v2 = fbxMesh->GetPolygonVertex(i, posInPoly2);

    fbxToOsgVertMap.insert(FbxToOsgVertexMap::value_type(v0, GIPair(pGeometry, pVertices->getNumElements())));
    fbxToOsgVertMap.insert(FbxToOsgVertexMap::value_type(v1, GIPair(pGeometry, pVertices->getNumElements() + 1)));
    fbxToOsgVertMap.insert(FbxToOsgVertexMap::value_type(v2, GIPair(pGeometry, pVertices->getNumElements() + 2)));

    addVec3ArrayElement(*pVertices, pFbxVertices[v0]);
    addVec3ArrayElement(*pVertices, pFbxVertices[v1]);
    addVec3ArrayElement(*pVertices, pFbxVertices[v2]);

    if (pNormals)
    {
        int n0 = getVertexIndex(pFbxNormals, fbxMesh, i, posInPoly0, meshVertex0);
        int n1 = getVertexIndex(pFbxNormals, fbxMesh, i, posInPoly1, meshVertex1);
        int n2 = getVertexIndex(pFbxNormals, fbxMesh, i, posInPoly2, meshVertex2);

        osgToFbxNormMap.insert(OsgToFbxNormalMap::value_type(GIPair(pGeometry, pNormals->getNumElements()), n0));
        osgToFbxNormMap.insert(OsgToFbxNormalMap::value_type(GIPair(pGeometry, pNormals->getNumElements() + 1), n1));
        osgToFbxNormMap.insert(OsgToFbxNormalMap::value_type(GIPair(pGeometry, pNormals->getNumElements() + 2), n2));

        addVec3ArrayElement(*pNormals, pFbxNormals->GetDirectArray().GetAt(n0));
        addVec3ArrayElement(*pNormals, pFbxNormals->GetDirectArray().GetAt(n1));
        addVec3ArrayElement(*pNormals, pFbxNormals->GetDirectArray().GetAt(n2));
    }

    // add texture maps data (avoid duplicates)...
    if (pTexCoords_diffuse)
    {
        addVec2ArrayElement(*pTexCoords_diffuse, getElement(pFbxUVs_diffuse, fbxMesh, i, posInPoly0, meshVertex0));
        addVec2ArrayElement(*pTexCoords_diffuse, getElement(pFbxUVs_diffuse, fbxMesh, i, posInPoly1, meshVertex1));
        addVec2ArrayElement(*pTexCoords_diffuse, getElement(pFbxUVs_diffuse, fbxMesh, i, posInPoly2, meshVertex2));
    }
    if (pTexCoords_opacity && (pTexCoords_opacity != pTexCoords_diffuse))
    {
        addVec2ArrayElement(*pTexCoords_opacity, getElement(pFbxUVs_opacity, fbxMesh, i, posInPoly0, meshVertex0));
        addVec2ArrayElement(*pTexCoords_opacity, getElement(pFbxUVs_opacity, fbxMesh, i, posInPoly1, meshVertex1));
        addVec2ArrayElement(*pTexCoords_opacity, getElement(pFbxUVs_opacity, fbxMesh, i, posInPoly2, meshVertex2));
    }

    // Only spherical reflection maps are supported (so do not add coordinates for the reflection map)

    if (pTexCoords_emissive && (pTexCoords_emissive != pTexCoords_opacity) && (pTexCoords_emissive != pTexCoords_diffuse))
    {
        addVec2ArrayElement(*pTexCoords_emissive, getElement(pFbxUVs_emissive, fbxMesh, i, posInPoly0, meshVertex0));
        addVec2ArrayElement(*pTexCoords_emissive, getElement(pFbxUVs_emissive, fbxMesh, i, posInPoly1, meshVertex1));
        addVec2ArrayElement(*pTexCoords_emissive, getElement(pFbxUVs_emissive, fbxMesh, i, posInPoly2, meshVertex2));
    }
    if (pTexCoords_ambient && (pTexCoords_ambient != pTexCoords_opacity) && (pTexCoords_ambient != pTexCoords_diffuse) && (pTexCoords_ambient != pTexCoords_emissive))
    {
        addVec2ArrayElement(*pTexCoords_ambient, getElement(pFbxUVs_ambient, fbxMesh, i, posInPoly0, meshVertex0));
        addVec2ArrayElement(*pTexCoords_ambient, getElement(pFbxUVs_ambient, fbxMesh, i, posInPoly1, meshVertex1));
        addVec2ArrayElement(*pTexCoords_ambient, getElement(pFbxUVs_ambient, fbxMesh, i, posInPoly2, meshVertex2));
    }
    // add more texture maps here...

    if (pColors)
    {
        addColorArrayElement(*pColors, getElement(pFbxColors, fbxMesh, i, posInPoly0, meshVertex0));
        addColorArrayElement(*pColors, getElement(pFbxColors, fbxMesh, i, posInPoly1, meshVertex1));
        addColorArrayElement(*pColors, getElement(pFbxColors, fbxMesh, i, posInPoly2, meshVertex2));
    }
}

/// Says if a quad should be split using vertices 02 (or else 13)
bool quadSplit02(const FbxMesh * fbxMesh, int i /*polygonIndex*/,
                 int posInPoly0, int posInPoly1, int posInPoly2, int posInPoly3,
                 const FbxVector4* pFbxVertices)
{
    // Algorithm may be a bit dumb. If you got a faster one, feel free to change.
    // Here we test each of the 4 triangles and see if there is one in the opposite direction.
    //        Triangles: 012, 023, 013, 123
    // For this, we do a cross product to get normals. We say here the first triangle is the reference, and do a dot product to see the direction.
    //        Normals: na (= (p1-p0)^(p2-p1)), nb, na, nd
    //        Dot products: rb (na.nb), rc, rd
    // Results:
    //        if r*>0 => convex (02 and 13 are ok, so choose 02)
    //        if rb only <0, or r*<0 => concave, split on 13
    //        if rc only <0, or rd<0 => concave, split on 02
    //        else unhandled (crossed polygon?) => choose 02
    //    In short:
    //        if rb only <0, or r*<0 => return false
    //        else return true

    int v0 = fbxMesh->GetPolygonVertex(i, posInPoly0);
    int v1 = fbxMesh->GetPolygonVertex(i, posInPoly1);
    int v2 = fbxMesh->GetPolygonVertex(i, posInPoly2);
    int v3 = fbxMesh->GetPolygonVertex(i, posInPoly3);

    osg::Vec3d p0(pFbxVertices[v0][0], pFbxVertices[v0][1], pFbxVertices[v0][2]);
    osg::Vec3d p1(pFbxVertices[v1][0], pFbxVertices[v1][1], pFbxVertices[v1][2]);
    osg::Vec3d p2(pFbxVertices[v2][0], pFbxVertices[v2][1], pFbxVertices[v2][2]);
    osg::Vec3d p3(pFbxVertices[v3][0], pFbxVertices[v3][1], pFbxVertices[v3][2]);

    osg::Vec3d na((p1 - p0) ^ (p2 - p1));
    osg::Vec3d nb((p2 - p0) ^ (p3 - p2));

    double rb(na * nb);
    if (rb >= 0) return true;        // Split at 02

    osg::Vec3d nc((p1 - p0) ^ (p3 - p1));
    osg::Vec3d nd((p2 - p1) ^ (p3 - p2));
    double rc(na * nc);
    double rd(na * nd);
    return (rc >= 0 || rd >= 0);
}

struct PolygonRef
{
    PolygonRef(osg::Geometry* pGeometry, int numPoly, int nVertex)
        : pGeometry(pGeometry), numPoly(numPoly), nVertex(nVertex)
    {}
    osg::Geometry* pGeometry;
    int numPoly;
    int nVertex;
};
typedef std::vector<PolygonRef> PolygonRefList;

osgDB::ReaderWriter::ReadResult OsgFbxReader::readMesh(
    FbxNode* pNode,
    FbxMesh* fbxMesh,
    std::vector<StateSetContent>& stateSetList,
    const char* szName)
{
    GeometryMap geometryMap;

    osg::Geode* pGeode = new osg::Geode;
    pGeode->setName(szName);

    const FbxLayerElementNormal* pFbxNormals = 0;
    const FbxLayerElementVertexColor* pFbxColors = 0;
    const FbxLayerElementMaterial* pFbxMaterials = 0;

    const FbxVector4* pFbxVertices = fbxMesh->GetControlPoints();

    // scan layers for Normals, Colors and Materials elements (this will get the first available elements)...
    for (int cLayerIndex = 0; cLayerIndex < fbxMesh->GetLayerCount(); cLayerIndex++)
    {
        const FbxLayer* pFbxLayer = fbxMesh->GetLayer(cLayerIndex);
        if (!pFbxLayer)
            continue;

        // get normals, colors and materials...
        if (!pFbxNormals)
            pFbxNormals = pFbxLayer->GetNormals();
        if (!pFbxColors)
            pFbxColors = pFbxLayer->GetVertexColors();
        if (!pFbxMaterials)
            pFbxMaterials = pFbxLayer->GetMaterials();
    }

    // look for UV elements (diffuse, opacity, reflection, emissive, ...) and get their channels names...
    std::string diffuseChannel = getUVChannelForTextureMap(stateSetList, FbxSurfaceMaterial::sDiffuse);
    std::string opacityChannel = getUVChannelForTextureMap(stateSetList, FbxSurfaceMaterial::sTransparentColor);
    std::string emissiveChannel = getUVChannelForTextureMap(stateSetList, FbxSurfaceMaterial::sEmissive);
    std::string ambientChannel = getUVChannelForTextureMap(stateSetList, FbxSurfaceMaterial::sAmbient);
    // look for more UV elements here...

    // UV elements...
    const FbxLayerElementUV* pFbxUVs_diffuse = getUVElementForChannel(diffuseChannel, FbxLayerElement::eTextureDiffuse, fbxMesh);
    const FbxLayerElementUV* pFbxUVs_opacity = getUVElementForChannel(opacityChannel, FbxLayerElement::eTextureTransparency, fbxMesh);
    const FbxLayerElementUV* pFbxUVs_emissive = getUVElementForChannel(emissiveChannel, FbxLayerElement::eTextureEmissive, fbxMesh);
    const FbxLayerElementUV* pFbxUVs_ambient = getUVElementForChannel(ambientChannel, FbxLayerElement::eTextureAmbient, fbxMesh);
    // more UV elements here...

    // check elements validity...
    if (!layerElementValid(pFbxNormals)) pFbxNormals = 0;
    if (!layerElementValid(pFbxColors)) pFbxColors = 0;

    if (!layerElementValid(pFbxUVs_diffuse)) pFbxUVs_diffuse = 0;
    if (!layerElementValid(pFbxUVs_opacity)) pFbxUVs_opacity = 0;
    if (!layerElementValid(pFbxUVs_emissive)) pFbxUVs_emissive = 0;
    if (!layerElementValid(pFbxUVs_ambient)) pFbxUVs_ambient = 0;
    // more here...

    int nPolys = fbxMesh->GetPolygonCount();

    int nDeformerCount = fbxMesh->GetDeformerCount(FbxDeformer::eSkin);
    int nDeformerBlendShapeCount = fbxMesh->GetDeformerCount(FbxDeformer::eBlendShape);

    GeometryType geomType = GEOMETRY_STATIC;

    //determine the type of geometry
    if (nDeformerCount)
    {
        geomType = GEOMETRY_RIG;
    }
    else if (nDeformerBlendShapeCount)
    {
        geomType = GEOMETRY_MORPH;
    }

    FbxToOsgVertexMap fbxToOsgVertMap;
    OsgToFbxNormalMap osgToFbxNormMap;

    // First add only triangles and quads (easy to split into triangles without
    // more processing)
    // This is the reason we store polygons references:

    PolygonRefList polygonRefList;

    for (int i = 0, nVertex = 0; i < nPolys; ++i)
    {
        int lPolygonSize = fbxMesh->GetPolygonSize(i);

        int materialIndex = getPolygonIndex(pFbxMaterials, i);

        osg::Geometry* pGeometry = getGeometry(pGeode, geometryMap,
            stateSetList, geomType, materialIndex,
            pFbxNormals != 0,
            pFbxUVs_diffuse != 0,
            pFbxUVs_opacity != 0,
            pFbxUVs_emissive != 0,
            pFbxUVs_ambient != 0,
            // more UV elements here...
            pFbxColors != 0,
            options,
            lightmapTextures);

        osg::Array* pVertices = pGeometry->getVertexArray();
        osg::Array* pNormals = pGeometry->getNormalArray();

        // get texture coordinates...
        osg::Array* pTexCoords_diffuse = pGeometry->getTexCoordArray(StateSetContent::DIFFUSE_TEXTURE_UNIT);
        osg::Array* pTexCoords_opacity = pGeometry->getTexCoordArray(StateSetContent::OPACITY_TEXTURE_UNIT);
        osg::Array* pTexCoords_emissive = pGeometry->getTexCoordArray(StateSetContent::EMISSIVE_TEXTURE_UNIT);
        osg::Array* pTexCoords_ambient = pGeometry->getTexCoordArray(StateSetContent::AMBIENT_TEXTURE_UNIT);
        // more texture coordinates here...

        osg::Array* pColors = pGeometry->getColorArray();

        if (lPolygonSize == 3)
        {
            // Triangle
            readMeshTriangle(fbxMesh, i,
                0, 1, 2,
                nVertex, nVertex+1, nVertex+2,
                fbxToOsgVertMap, osgToFbxNormMap,
                pFbxVertices, pFbxNormals, pFbxUVs_diffuse, pFbxUVs_opacity, pFbxUVs_emissive, pFbxUVs_ambient, pFbxColors,
                pGeometry,
                pVertices, pNormals, pTexCoords_diffuse, pTexCoords_opacity, pTexCoords_emissive, pTexCoords_ambient, pColors);
            nVertex += 3;
        }
        else if (lPolygonSize == 4)
        {
            // Quad - Convert to triangles
            // Use some fast specialized code to see how the should be decomposed
            // Two cases : Split at '02' (012 and 023), or split at '13 (013 and 123)
            bool split02 = quadSplit02(fbxMesh, i, 0, 1, 2, 3, pFbxVertices);
            int p02 = split02 ? 2 : 3; // Triangle 0, point 2
            int p10 = split02 ? 0 : 1; // Triangle 1, point 0
            readMeshTriangle(fbxMesh, i,
                0, 1, p02,
                nVertex, nVertex+1, nVertex+p02,
                fbxToOsgVertMap, osgToFbxNormMap,
                pFbxVertices, pFbxNormals, pFbxUVs_diffuse, pFbxUVs_opacity, pFbxUVs_emissive, pFbxUVs_ambient, pFbxColors,
                pGeometry,
                pVertices, pNormals, pTexCoords_diffuse, pTexCoords_opacity, pTexCoords_emissive, pTexCoords_ambient, pColors);
            readMeshTriangle(fbxMesh, i,
                p10, 2, 3,
                nVertex+p10, nVertex+2, nVertex+3,
                fbxToOsgVertMap, osgToFbxNormMap,
                pFbxVertices, pFbxNormals, pFbxUVs_diffuse, pFbxUVs_opacity, pFbxUVs_emissive, pFbxUVs_ambient, pFbxColors,
                pGeometry,
                pVertices, pNormals, pTexCoords_diffuse, pTexCoords_opacity, pTexCoords_emissive, pTexCoords_ambient, pColors);
            nVertex += 4;
        }
        else if (tessellatePolygons)
        {
            // Polygons - Store to add after triangles
            polygonRefList.push_back(PolygonRef(pGeometry, i, nVertex));
            nVertex += lPolygonSize;
        }
        else
        {
            int nVertex0 = nVertex;
            nVertex += (std::min)(2, lPolygonSize);

            for (int j = 2; j < lPolygonSize; ++j, ++nVertex)
            {
                readMeshTriangle(fbxMesh, i,
                    0, j - 1, j,
                    nVertex0, nVertex - 1, nVertex,
                    fbxToOsgVertMap, osgToFbxNormMap,
                    pFbxVertices, pFbxNormals, pFbxUVs_diffuse, pFbxUVs_opacity, pFbxUVs_emissive, pFbxUVs_ambient, pFbxColors,
                    pGeometry,
                    pVertices, pNormals, pTexCoords_diffuse, pTexCoords_opacity, pTexCoords_emissive, pTexCoords_ambient, pColors);
            }
        }
    }

    for (unsigned i = 0; i < pGeode->getNumDrawables(); ++i)
    {
        osg::Geometry* pGeometry = pGeode->getDrawable(i)->asGeometry();
        osg::DrawArrays* pDrawArrays = new osg::DrawArrays(
            GL_TRIANGLES, 0, pGeometry->getVertexArray()->getNumElements());
        pGeometry->addPrimitiveSet(pDrawArrays);
    }

    // Now add polygons - Convert to triangles
    // We put vertices in their own PrimitiveSet with Mode=POLYGON; then run the
    // Tessellator on the Geometry which should tessellate the polygons
    // automagically.
    for (PolygonRefList::iterator it = polygonRefList.begin(), itEnd=polygonRefList.end();
        it != itEnd; ++it)
    {
        int i = it->numPoly;
        int lPolygonSize = fbxMesh->GetPolygonSize(i);
        //int materialIndex = getPolygonIndex(pFbxMaterials, i);
        osg::Geometry* pGeometry = it->pGeometry;

        osg::Array* pVertices = pGeometry->getVertexArray();
        osg::Array* pNormals = pGeometry->getNormalArray();
        osg::Array* pTexCoords_diffuse = pGeometry->getTexCoordArray(StateSetContent::DIFFUSE_TEXTURE_UNIT);
        osg::Array* pTexCoords_opacity = pGeometry->getTexCoordArray(StateSetContent::OPACITY_TEXTURE_UNIT);
        osg::Array* pTexCoords_emissive = pGeometry->getTexCoordArray(StateSetContent::EMISSIVE_TEXTURE_UNIT);
        osg::Array* pTexCoords_ambient = pGeometry->getTexCoordArray(StateSetContent::AMBIENT_TEXTURE_UNIT);
        osg::Array* pColors = pGeometry->getColorArray();
        // Index of the 1st vertex of the polygon in the geometry
        int osgVertex0 = pVertices->getNumElements();

        for (int j = 0, nVertex = it->nVertex; j<lPolygonSize; ++j, ++nVertex)
        {
            int v0 = fbxMesh->GetPolygonVertex(i, j);
            fbxToOsgVertMap.insert(FbxToOsgVertexMap::value_type(v0, GIPair(pGeometry, pVertices->getNumElements())));
            addVec3ArrayElement(*pVertices, pFbxVertices[v0]);
            if (pNormals)
            {
                int n0 = getVertexIndex(pFbxNormals, fbxMesh, i, j, nVertex);
                osgToFbxNormMap.insert(OsgToFbxNormalMap::value_type(GIPair(pGeometry, pNormals->getNumElements()), n0));
                addVec3ArrayElement(*pNormals, pFbxNormals->GetDirectArray().GetAt(n0));
            }

            // add texture maps data (avoid duplicates)...
            if (pTexCoords_diffuse)
            {
                addVec2ArrayElement(*pTexCoords_diffuse, getElement(pFbxUVs_diffuse, fbxMesh, i, j, nVertex));
            }
            if (pTexCoords_opacity && (pTexCoords_opacity != pTexCoords_diffuse))
            {
                addVec2ArrayElement(*pTexCoords_opacity, getElement(pFbxUVs_opacity, fbxMesh, i, j, nVertex));
            }

            // Only spherical reflection maps are supported (so do not add coordinates for the reflection map)

            if (pTexCoords_emissive && (pTexCoords_emissive != pTexCoords_opacity) && (pTexCoords_emissive != pTexCoords_diffuse))
            {
                addVec2ArrayElement(*pTexCoords_emissive, getElement(pFbxUVs_emissive, fbxMesh, i, j, nVertex));
            }
            if (pTexCoords_ambient && (pTexCoords_ambient != pTexCoords_opacity) && (pTexCoords_ambient != pTexCoords_diffuse))
            {
                addVec2ArrayElement(*pTexCoords_ambient, getElement(pFbxUVs_ambient, fbxMesh, i, j, nVertex));
            }
            // add more texture maps here...

            if (pColors)
            {
                addColorArrayElement(*pColors, getElement(pFbxColors, fbxMesh, i, j, nVertex));
            }
        }

        osg::DrawArrays* pDrawArrays = new osg::DrawArrays(
            GL_POLYGON, osgVertex0, pGeometry->getVertexArray()->getNumElements() - osgVertex0);
        pGeometry->addPrimitiveSet(pDrawArrays);
    }

    for (unsigned i = 0; i < pGeode->getNumDrawables(); ++i)
    {
        osg::Geometry* pGeometry = pGeode->getDrawable(i)->asGeometry();

        // Now split polygons if necessary
        osgUtil::Tessellator tessellator;
        tessellator.retessellatePolygons(*pGeometry);

        if (pGeode->getNumDrawables() > 1)
        {
            std::stringstream ss;
            ss << pGeode->getName() << " " << i + 1;
            pGeometry->setName(ss.str());
        }
        else
        {
            pGeometry->setName(pGeode->getName());
        }
    }

    if (geomType == GEOMETRY_RIG)
    {
        typedef std::map<osg::ref_ptr<osg::Geometry>,
            osg::ref_ptr<osgAnimation::RigGeometry> > GeometryRigGeometryMap;
        GeometryRigGeometryMap old2newGeometryMap;

        for (unsigned i = 0; i < pGeode->getNumDrawables(); ++i)
        {
            osg::Geometry* pGeometry = pGeode->getDrawable(i)->asGeometry();

            osgAnimation::RigGeometry* pRig = new osgAnimation::RigGeometry;
            pRig->setSourceGeometry(pGeometry);
            pRig->copyFrom(*pGeometry);
            old2newGeometryMap.insert(GeometryRigGeometryMap::value_type(
                pGeometry, pRig));
            pRig->setDataVariance(osg::Object::DYNAMIC);
            pRig->setUseDisplayList( false );
            pGeode->setDrawable(i, pRig);

            pRig->setInfluenceMap(new osgAnimation::VertexInfluenceMap);
            pGeometry = pRig;
        }

        for (int i = 0; i < nDeformerCount; ++i)
        {
            FbxSkin* pSkin = (FbxSkin*)fbxMesh->GetDeformer(i, FbxDeformer::eSkin);
            int nClusters = pSkin->GetClusterCount();
            for (int j = 0; j < nClusters; ++j)
            {
                FbxCluster* pCluster = pSkin->GetCluster(j);
                //assert(KFbxCluster::eNORMALIZE == pCluster->GetLinkMode());
                FbxNode* pBone = pCluster->GetLink();

                FbxAMatrix transformLink;
                pCluster->GetTransformLinkMatrix(transformLink);
                FbxAMatrix transformLinkInverse = transformLink.Inverse();
                const double* pTransformLinkInverse = transformLinkInverse;
                osg::Matrix bindMatrix(pTransformLinkInverse);

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
                        osgAnimation::RigGeometry& rig =
                            dynamic_cast<osgAnimation::RigGeometry&>(
                            *old2newGeometryMap[gi.first]);
                        addBindMatrix(boneBindMatrices, pBone, bindMatrix, &rig);
                        osgAnimation::VertexInfluenceMap& vim =
                            *rig.getInfluenceMap();
                        osgAnimation::VertexInfluence& vi =
                            getVertexInfluence(vim, pBone->GetName());
                        vi.push_back(osgAnimation::VertexIndexWeight(
                            gi.second, weight));
                    }
                }
            }
        }
    }
    else if (geomType == GEOMETRY_MORPH)
    {
        for (unsigned i = 0; i < pGeode->getNumDrawables(); ++i)
        {
            osg::Geometry* pGeometry = pGeode->getDrawable(i)->asGeometry();

            osgAnimation::MorphGeometry& morph = dynamic_cast<osgAnimation::MorphGeometry&>(*pGeometry);

            pGeode->addUpdateCallback(new osgAnimation::UpdateMorph(morph.getName()));

            //read morph geometry
            for (int nBlendShape = 0; nBlendShape < nDeformerBlendShapeCount; ++nBlendShape)
            {
                FbxBlendShape* pBlendShape = FbxCast<FbxBlendShape>(fbxMesh->GetDeformer(nBlendShape, FbxDeformer::eBlendShape));
                const int nBlendShapeChannelCount = pBlendShape->GetBlendShapeChannelCount();

                for (int nBlendShapeChannel = 0; nBlendShapeChannel < nBlendShapeChannelCount; ++nBlendShapeChannel)
                {
                    FbxBlendShapeChannel* pBlendShapeChannel = pBlendShape->GetBlendShapeChannel(nBlendShapeChannel);
                    if (!pBlendShapeChannel->GetTargetShapeCount()) continue;

                    //Assume one shape
                    if (pBlendShapeChannel->GetTargetShapeCount() > 1)
                    {
                        OSG_WARN << "Multiple FBX Target Shapes, only the first will be used" << std::endl;
                    }
                    const FbxGeometryBase* pMorphShape = pBlendShapeChannel->GetTargetShape(0);

                    const FbxLayerElementNormal* pFbxShapeNormals = 0;
                    if (const FbxLayer* pFbxShapeLayer = pMorphShape->GetLayer(0))
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
                    pMorphTarget->setName(pMorphShape->GetName());
                    morph.addMorphTarget(pMorphTarget, 0.0f);

                    readAnimation(pNode, fbxScene, morph.getName(), pAnimationManager, fbxMesh,
                        nBlendShape, nBlendShapeChannel, (int)morph.getMorphTargetList().size() - 1);
                }
            }
        }

        int nMorphTarget = 0;
        for (int nBlendShape = 0; nBlendShape < nDeformerBlendShapeCount; ++nBlendShape)
        {
            FbxBlendShape* pBlendShape = FbxCast<FbxBlendShape>(fbxMesh->GetDeformer(nBlendShape, FbxDeformer::eBlendShape));
            const int nBlendShapeChannelCount = pBlendShape->GetBlendShapeChannelCount();

            for (int nBlendShapeChannel = 0; nBlendShapeChannel < nBlendShapeChannelCount; ++nBlendShapeChannel)
            {
                FbxBlendShapeChannel* pBlendShapeChannel = pBlendShape->GetBlendShapeChannel(nBlendShapeChannel);
                if (!pBlendShapeChannel->GetTargetShapeCount()) continue;

                //Assume one shape again
                const FbxGeometryBase* pMorphShape = pBlendShapeChannel->GetTargetShape(0);

                const FbxLayerElementNormal* pFbxShapeNormals = 0;
                if (const FbxLayer* pFbxShapeLayer = pMorphShape->GetLayer(0))
                {
                    pFbxShapeNormals = pFbxShapeLayer->GetNormals();
                    if (!layerElementValid(pFbxShapeNormals)) pFbxShapeNormals = 0;
                }

                const FbxVector4* pControlPoints = pMorphShape->GetControlPoints();
                int nControlPoints = pMorphShape->GetControlPointsCount();
                for (int fbxIndex = 0; fbxIndex < nControlPoints; ++fbxIndex)
                {
                    osg::Vec3d vPos = convertVec3(pControlPoints[fbxIndex]);
                    for (FbxToOsgVertexMap::const_iterator it =
                        fbxToOsgVertMap.find(fbxIndex);
                        it != fbxToOsgVertMap.end() &&
                        it->first == fbxIndex; ++it)
                    {
                        GIPair gi = it->second;
                        osgAnimation::MorphGeometry& morphGeom =
                            dynamic_cast<osgAnimation::MorphGeometry&>(*gi.first);
                        osg::Geometry* pGeometry = morphGeom.getMorphTarget(nMorphTarget).getGeometry();

                        if (pGeometry->getVertexArray()->getType() == osg::Array::Vec3dArrayType)
                        {
                            osg::Vec3dArray* pVertices = static_cast<osg::Vec3dArray*>(pGeometry->getVertexArray());
                            (*pVertices)[gi.second] = vPos;
                        }
                        else
                        {
                            osg::Vec3Array* pVertices = static_cast<osg::Vec3Array*>(pGeometry->getVertexArray());
                            (*pVertices)[gi.second] = vPos;
                        }

                        if (pFbxShapeNormals && pGeometry->getNormalArray())
                        {
                            if (pGeometry->getNormalArray()->getType() == osg::Array::Vec3dArrayType)
                            {
                                osg::Vec3dArray* pNormals = static_cast<osg::Vec3dArray*>(pGeometry->getNormalArray());
                                (*pNormals)[gi.second] = convertVec3(
                                    pFbxShapeNormals->GetDirectArray().GetAt(osgToFbxNormMap[gi]));
                            }
                            else
                            {
                                osg::Vec3Array* pNormals = static_cast<osg::Vec3Array*>(pGeometry->getNormalArray());
                                (*pNormals)[gi.second] = convertVec3(
                                    pFbxShapeNormals->GetDirectArray().GetAt(osgToFbxNormMap[gi]));
                            }
                        }
                    }
                }

                //don't put this in the for loop as we don't want to do it if the loop continues early
                ++nMorphTarget;
            }
        }
    }

    FbxAMatrix fbxGeometricTransform;
    fbxGeometricTransform.SetTRS(
        pNode->GeometricTranslation.Get(),
        pNode->GeometricRotation.Get(),
        pNode->GeometricScaling.Get());
    const double* pGeometricMat = fbxGeometricTransform;
    osg::Matrix osgGeometricTransform(pGeometricMat);

    if (geomType == GEOMETRY_RIG)
    {
        FbxSkin* pSkin = (FbxSkin*)fbxMesh->GetDeformer(0, FbxDeformer::eSkin);
        if (pSkin->GetClusterCount())
        {
            FbxAMatrix fbxTransformMatrix;
            pSkin->GetCluster(0)->GetTransformMatrix(fbxTransformMatrix);
            const double* pTransformMatrix = fbxTransformMatrix;
            osgGeometricTransform.postMult(osg::Matrix(pTransformMatrix));
        }
    }

    osg::Node* pResult = pGeode;

    if (!osgGeometricTransform.isIdentity())
    {
        osg::MatrixTransform* pMatTrans = new osg::MatrixTransform(osgGeometricTransform);
        pMatTrans->addChild(pGeode);
        pResult = pMatTrans;
    }

    if (geomType == GEOMETRY_RIG)
    {
        //Add the geometry to the skeleton ancestor of one of the bones.
        FbxSkin* pSkin = (FbxSkin*)fbxMesh->GetDeformer(0, FbxDeformer::eSkin);
        if (pSkin->GetClusterCount())
        {
            osgAnimation::Skeleton* pSkeleton = getSkeleton(
                pSkin->GetCluster(0)->GetLink(), fbxSkeletons, skeletonMap);
            pSkeleton->addChild(pResult);
            return osgDB::ReaderWriter::ReadResult::FILE_LOADED;
        }
    }

    return osgDB::ReaderWriter::ReadResult(pResult);
}

osgDB::ReaderWriter::ReadResult OsgFbxReader::readFbxMesh(FbxNode* pNode,
    std::vector<StateSetContent>& stateSetList)
{
    FbxMesh* lMesh = FbxCast<FbxMesh>(pNode->GetNodeAttribute());

    if (!lMesh)
    {
        return osgDB::ReaderWriter::ReadResult::ERROR_IN_READING_FILE;
    }

    return readMesh(pNode, lMesh, stateSetList,
        pNode->GetName());
}
