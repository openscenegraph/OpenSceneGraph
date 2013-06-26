// -*-c++-*-

/*
 * FBX writer for Open Scene Graph
 *
 * Copyright (C) 2009
 *
 * Writing support added 2009 by Thibault Caporal and Sukender (Benoit Neil - http://sukender.free.fr)
 *
 * The Open Scene Graph (OSG) is a cross platform C++/OpenGL library for
 * real-time rendering of large 3D photo-realistic models.
 * The OSG homepage is http://www.openscenegraph.org/
 */

#include <climits>                     // required for UINT_MAX
#include <cassert>
#include <osg/CullFace>
#include <osg/MatrixTransform>
#include <osg/NodeVisitor>
#include <osg/PrimitiveSet>
#include <osgDB/FileUtils>
#include <osgDB/WriteFile>
#include "WriterNodeVisitor.h"


// Use namespace qualification to avoid static-link symbol collisions
// from multiply defined symbols.
namespace pluginfbx
{


/** writes all primitives of a primitive-set out to a stream, decomposes quads to triangles, line-strips to lines etc */
class PrimitiveIndexWriter : public osg::PrimitiveIndexFunctor
{
public:
    PrimitiveIndexWriter(const osg::Geometry* geo,
                         ListTriangle&        listTriangles,
                         unsigned int         drawable_n,
                         unsigned int         material) :
        _drawable_n(drawable_n),
        _listTriangles(listTriangles),
        _modeCache(0),
        _hasNormalCoords(geo->getNormalArray() != NULL),
        _hasTexCoords(geo->getTexCoordArray(0) != NULL),
        _geo(geo),
        _lastFaceIndex(0),
        _material(material),
        _curNormalIndex(0),
        _normalBinding(osg::Geometry::BIND_OFF),
        _mesh(0)
    {
        _normalBinding = geo->getNormalBinding();
        if (!geo->getNormalArray() || geo->getNormalArray()->getNumElements()==0)
        {
            _normalBinding = osg::Geometry::BIND_OFF;        // Turn off binding if there is no normal data
        }
        reset();
    }

    void reset() { _curNormalIndex = 0; }

    unsigned int getNextFaceIndex() { return _lastFaceIndex; }

    virtual void setVertexArray(unsigned int, const osg::Vec2*) {}

    virtual void setVertexArray(unsigned int, const osg::Vec3*) {}

    virtual void setVertexArray(unsigned int, const osg::Vec4*) {}

    virtual void setVertexArray(unsigned int, const osg::Vec2d*) {}

    virtual void setVertexArray(unsigned int, const osg::Vec3d*) {}

    virtual void setVertexArray(unsigned int, const osg::Vec4d*) {}

    // operator for triangles
    void writeTriangle(unsigned int i1, unsigned int i2, unsigned int i3)
    {
        Triangle triangle;
        triangle.t1 = i1;
        triangle.t2 = i2;
        triangle.t3 = i3;
        if (_normalBinding == osg::Geometry::BIND_PER_VERTEX){
            triangle.normalIndex1 = i1;
            triangle.normalIndex2 = i2;
            triangle.normalIndex3 = i3;
        }
        else{
            triangle.normalIndex1 = _curNormalIndex;
            triangle.normalIndex2 = _curNormalIndex;
            triangle.normalIndex3 = _curNormalIndex;
        }
        triangle.material = _material;
        _listTriangles.push_back(std::make_pair(triangle, _drawable_n));
    }

    virtual void begin(GLenum mode)
    {
        _modeCache = mode;
        _indexCache.clear();
    }

    virtual void vertex(unsigned int vert)
    {
        _indexCache.push_back(vert);
    }

    virtual void end()
    {
        if (!_indexCache.empty())
        {
            drawElements(_modeCache, _indexCache.size(), &_indexCache.front());
        }
    }

    virtual void drawArrays(GLenum mode, GLint first, GLsizei count);

    virtual void drawElements(GLenum mode, GLsizei count, const GLubyte* indices)
    {
        drawElementsImplementation<GLubyte>(mode, count, indices);
    }

    virtual void drawElements(GLenum mode, GLsizei count, const GLushort* indices)
    {
        drawElementsImplementation<GLushort>(mode, count, indices);
    }

    virtual void drawElements(GLenum mode, GLsizei count, const GLuint* indices)
    {
        drawElementsImplementation<GLuint>(mode, count, indices);
    }

protected:
    template <typename T> void drawElementsImplementation(GLenum mode, GLsizei count, const T* indices)
    {
        if (indices==0 || count==0) return;

        typedef const T* IndexPointer;

        switch (mode)
        {
        case GL_TRIANGLES:
            {
                IndexPointer ilast = indices + count;
                for (IndexPointer iptr = indices; iptr < ilast; iptr+=3)
                {
                    writeTriangle(iptr[0], iptr[1], iptr[2]);
                }
                break;
            }
        case GL_TRIANGLE_STRIP:
            {
                IndexPointer iptr = indices;
                for (GLsizei i = 2; i < count; ++i, ++iptr)
                {
                    if (i & 1) writeTriangle(iptr[0], iptr[2], iptr[1]);
                    else       writeTriangle(iptr[0], iptr[1], iptr[2]);
                }
                break;
            }
        case GL_QUADS:
            {
                IndexPointer iptr = indices;
                for (GLsizei i = 3; i < count; i += 4, iptr += 4)
                {
                    writeTriangle(iptr[0], iptr[1], iptr[2]);
                    writeTriangle(iptr[0], iptr[2], iptr[3]);
                }
                break;
            }
        case GL_QUAD_STRIP:
            {
                IndexPointer iptr = indices;
                for (GLsizei i = 3; i < count; i += 2, iptr += 2)
                {
                    writeTriangle(iptr[0], iptr[1], iptr[2]);
                    writeTriangle(iptr[1], iptr[3], iptr[2]);
                }
                break;
            }
        case GL_POLYGON: // treat polygons as GL_TRIANGLE_FAN
        case GL_TRIANGLE_FAN:
            {
                IndexPointer iptr = indices;
                unsigned int first = *iptr;
                ++iptr;
                for (GLsizei i = 2; i < count; ++i, ++iptr)
                {
                    writeTriangle(first, iptr[0], iptr[1]);
                }
                break;
            }
        case GL_POINTS:
        case GL_LINES:
        case GL_LINE_STRIP:
        case GL_LINE_LOOP:
            // Not handled
            break;
        default:
            // uhm should never come to this point :)
            break;
        }
        if (_normalBinding == osg::Geometry::BIND_PER_PRIMITIVE_SET) ++_curNormalIndex;
    }

private:
    PrimitiveIndexWriter& operator = (const PrimitiveIndexWriter&); // { return *this; }

    unsigned int         _drawable_n;
    ListTriangle&        _listTriangles;
    GLenum               _modeCache;
    std::vector<GLuint>  _indexCache;
    bool                 _hasNormalCoords, _hasTexCoords;
    const osg::Geometry* _geo;
    unsigned int         _lastFaceIndex;
    int                  _material;
    unsigned int         _curNormalIndex;
    osg::Geometry::AttributeBinding _normalBinding;
    FbxMesh*            _mesh;
};

void PrimitiveIndexWriter::drawArrays(GLenum mode,GLint first,GLsizei count)
{
    unsigned int pos=first;
    switch (mode)
    {
    case GL_TRIANGLES:
        for (GLsizei i = 2; i < count; i += 3, pos += 3)
        {
            writeTriangle(pos, pos + 1, pos + 2);
        }
        break;
    case GL_TRIANGLE_STRIP:
        for (GLsizei i = 2; i < count; ++i, ++pos)
        {
            if (i & 1) writeTriangle(pos, pos + 2, pos + 1);
            else       writeTriangle(pos, pos + 1, pos + 2);
        }
        break;
    case GL_QUADS:
        for (GLsizei i = 3; i < count; i += 4, pos += 4)
        {
            writeTriangle(pos, pos + 1, pos + 2);
            writeTriangle(pos, pos + 2, pos + 3);
        }
        break;
    case GL_QUAD_STRIP:
        for (GLsizei i = 3; i < count; i += 2, pos += 2)
        {
            writeTriangle(pos, pos + 1, pos + 2);
            writeTriangle(pos + 1, pos + 3, pos + 2);
        }
        break;
    case GL_POLYGON: // treat polygons as GL_TRIANGLE_FAN
    case GL_TRIANGLE_FAN:
        pos = first + 1;
        for (GLsizei i = 2; i < count; ++i, ++pos)
        {
            writeTriangle(first, pos, pos+1);
        }
        break;
    case GL_POINTS:
    case GL_LINES:
    case GL_LINE_STRIP:
    case GL_LINE_LOOP:
    default:
        OSG_WARN << "WriterNodeVisitor :: can't handle mode " << mode << std::endl;
        break;
    }
    if (_normalBinding == osg::Geometry::BIND_PER_PRIMITIVE_SET) ++_curNormalIndex;
}

WriterNodeVisitor::Material::Material(WriterNodeVisitor& writerNodeVisitor,
                                      osgDB::ExternalFileWriter & externalWriter,
                                      const osg::StateSet* stateset,
                                      const osg::Material* mat,
                                      const osg::Texture* tex,
                                      FbxManager* pSdkManager,
                                      const osgDB::ReaderWriter::Options * options,
                                      int index) :
    _fbxMaterial(NULL),
    _fbxTexture(NULL),
    _index(index),
    _osgImage(NULL)
{
    osg::Vec4 diffuse(1,1,1,1),
              ambient(0.2,0.2,0.2,1),
              specular(0,0,0,1),
              emission(1,1,1,1);

    float shininess(0);
    float transparency(0);

    if (mat)
    {
        assert(stateset);
        diffuse = mat->getDiffuse(osg::Material::FRONT);
        ambient = mat->getAmbient(osg::Material::FRONT);
        specular = mat->getSpecular(osg::Material::FRONT);
        shininess = mat->getShininess(osg::Material::FRONT);
        emission = mat->getEmission(osg::Material::FRONT);
        transparency = 1 - diffuse.w();

        const osg::StateAttribute* attribute = stateset->getAttribute(osg::StateAttribute::CULLFACE);
        if (attribute)
        {
            assert(dynamic_cast<const osg::CullFace*>(attribute));
            osg::CullFace::Mode mode = static_cast<const osg::CullFace*>(attribute)->getMode();
            if (mode == osg::CullFace::FRONT)
            {
                OSG_WARN << "FBX Writer: Reversed face (culled FRONT) not supported yet." << std::endl;
            }
            else if (mode != osg::CullFace::BACK)
            {
                assert(mode == osg::CullFace::FRONT_AND_BACK);
                OSG_WARN << "FBX Writer: Invisible face (culled FRONT_AND_BACK) not supported yet." << std::endl;
            }
        }

        _fbxMaterial = FbxSurfacePhong::Create(pSdkManager, mat->getName().c_str());
        if (_fbxMaterial)
        {
            _fbxMaterial->DiffuseFactor.Set(1);
            _fbxMaterial->Diffuse.Set(FbxDouble3(
                diffuse.x(),
                diffuse.y(),
                diffuse.z()));

            _fbxMaterial->TransparencyFactor.Set(transparency);

            _fbxMaterial->Ambient.Set(FbxDouble3(
                ambient.x(),
                ambient.y(),
                ambient.z()));

            _fbxMaterial->Emissive.Set(FbxDouble3(
                emission.x(),
                emission.y(),
                emission.z()));

            _fbxMaterial->Specular.Set(FbxDouble3(
                specular.x(),
                specular.y(),
                specular.z()));

            _fbxMaterial->Shininess.Set(shininess);
        }
    }
    if (tex && tex->getImage(0))
    {
        _osgImage = tex->getImage(0);

        std::string relativePath;
        externalWriter.write(*_osgImage, options, NULL, &relativePath);

        _fbxTexture = FbxFileTexture::Create(pSdkManager, relativePath.c_str());
        _fbxTexture->SetFileName(relativePath.c_str());
        // Create a FBX material if needed
        if (!_fbxMaterial)
        {
            _fbxMaterial = FbxSurfacePhong::Create(pSdkManager, relativePath.c_str());
        }
        // Connect texture to material's diffuse
        // Note there should be no reason FbxSurfacePhong::Create() would return NULL, but as previous code made this secirity test, here we keep the same way.
        if (_fbxMaterial)
        {
            _fbxMaterial->Diffuse.ConnectSrcObject(_fbxTexture);
        }
    }
}

int WriterNodeVisitor::processStateSet(const osg::StateSet* ss)
{
    //OSG_ALWAYS << "Trying Adding " << ss->getAttribute(osg::StateAttribute::MATERIAL)->getName() << std::endl;
    MaterialMap::iterator itr = _materialMap.find(MaterialMap::key_type(ss));
    if (itr != _materialMap.end())
    {
        if (itr->second.getIndex() < 0)
            itr->second.setIndex(_lastMaterialIndex++);
        return itr->second.getIndex();
    }

    const osg::Material* mat = dynamic_cast<const osg::Material*>(ss->getAttribute(osg::StateAttribute::MATERIAL));
    const osg::Texture* tex = dynamic_cast<const osg::Texture*>(ss->getTextureAttribute(0, osg::StateAttribute::TEXTURE));

    if (mat || tex)
    {
        int matNum = _lastMaterialIndex;
        _materialMap.insert(MaterialMap::value_type(MaterialMap::key_type(ss),
            Material(*this, _externalWriter, ss, mat, tex, _pSdkManager, _options, matNum)));
        ++_lastMaterialIndex;
        return matNum;
    }
    return -1;
}

unsigned int addPolygon(MapIndices & index_vert, unsigned int vertIndex, unsigned int normIndex, unsigned int drawableNum)
{
    VertexIndex vert(vertIndex, drawableNum, normIndex);
    MapIndices::iterator itIndex = index_vert.find(vert);
    if (itIndex == index_vert.end())
    {
        unsigned int indexMesh = index_vert.size();
        index_vert.insert(std::make_pair(vert, indexMesh));
        return indexMesh;
    }
    return itIndex->second;
}

void addPolygon(FbxMesh * mesh, MapIndices & index_vert, const Triangle & tri, unsigned int drawableNum)
{
    mesh->AddPolygon(addPolygon(index_vert, tri.t1, tri.normalIndex1, drawableNum));
    mesh->AddPolygon(addPolygon(index_vert, tri.t2, tri.normalIndex2, drawableNum));
    mesh->AddPolygon(addPolygon(index_vert, tri.t3, tri.normalIndex3, drawableNum));
}


void
WriterNodeVisitor::setLayerTextureAndMaterial(FbxMesh* mesh)
{
    FbxLayerElementTexture* lTextureDiffuseLayer = FbxLayerElementTexture::Create(mesh, "Diffuse");
    lTextureDiffuseLayer->SetMappingMode(FbxLayerElement::eByPolygon);
    lTextureDiffuseLayer->SetReferenceMode(FbxLayerElement::eIndexToDirect);

    FbxLayerElementMaterial* lMaterialLayer = FbxLayerElementMaterial::Create(mesh, "materialLayer");
    lMaterialLayer->SetMappingMode(FbxLayerElement::eByPolygon);
    lMaterialLayer->SetReferenceMode(FbxLayerElement::eIndexToDirect);

    lTextureDiffuseLayer->GetDirectArray().SetCount(_lastMaterialIndex);
    lMaterialLayer->mDirectArray->SetCount(_lastMaterialIndex);

    for (MaterialMap::iterator it = _materialMap.begin(); it != _materialMap.end(); ++it)
    {
        if (it->second.getIndex() != -1)
        {
            FbxSurfaceMaterial* lMaterial = it->second.getFbxMaterial();
            FbxFileTexture* lTexture = it->second.getFbxTexture();
            lTextureDiffuseLayer->GetDirectArray().SetAt(it->second.getIndex(), lTexture);
            lMaterialLayer->mDirectArray->SetAt(it->second.getIndex(), lMaterial);
        }
    }
    mesh->GetLayer(0)->SetMaterials(lMaterialLayer);
    mesh->GetLayer(0)->SetTextures(FbxLayerElement::eTextureDiffuse, lTextureDiffuseLayer);
}

void
WriterNodeVisitor::setControlPointAndNormalsAndUV(const osg::Geode& geo,
                                                  MapIndices&       index_vert,
                                                  bool              texcoords,
                                                  FbxMesh*         mesh)
{
    mesh->InitControlPoints(index_vert.size());
    FbxLayerElementNormal* lLayerElementNormal= FbxLayerElementNormal::Create(mesh, "");
    // For now, FBX writer only supports normals bound per vertices
    lLayerElementNormal->SetMappingMode(FbxLayerElement::eByControlPoint);
    lLayerElementNormal->SetReferenceMode(FbxLayerElement::eDirect);
    lLayerElementNormal->GetDirectArray().SetCount(index_vert.size());
    mesh->GetLayer(0)->SetNormals(lLayerElementNormal);
    FbxLayerElementUV* lUVDiffuseLayer = FbxLayerElementUV::Create(mesh, "DiffuseUV");

    if (texcoords)
    {
        lUVDiffuseLayer->SetMappingMode(FbxLayerElement::eByControlPoint);
        lUVDiffuseLayer->SetReferenceMode(FbxLayerElement::eDirect);
        lUVDiffuseLayer->GetDirectArray().SetCount(index_vert.size());
        mesh->GetLayer(0)->SetUVs(lUVDiffuseLayer, FbxLayerElement::eTextureDiffuse);
    }

    for (MapIndices::iterator it = index_vert.begin(); it != index_vert.end(); ++it)
    {
        const osg::Geometry* pGeometry = geo.getDrawable(it->first.drawableIndex)->asGeometry();
        unsigned int vertexIndex = it->first.vertexIndex;
        unsigned int normalIndex = it->first.normalIndex;

        const osg::Array * basevecs = pGeometry->getVertexArray();
        assert(basevecs);
        if (!basevecs || basevecs->getNumElements()==0)
        {
            //OSG_NOTIFY()
            continue;
        }
        FbxVector4 vertex;
        if (basevecs->getType() == osg::Array::Vec3ArrayType)
        {
            const osg::Vec3  & vec = (*static_cast<const osg::Vec3Array  *>(basevecs))[vertexIndex];
            vertex.Set(vec.x(), vec.y(), vec.z());
        }
        else if (basevecs->getType() == osg::Array::Vec3dArrayType)
        {
            const osg::Vec3d & vec = (*static_cast<const osg::Vec3dArray *>(basevecs))[vertexIndex];
            vertex.Set(vec.x(), vec.y(), vec.z());
        }
        else
        {
            OSG_NOTIFY(osg::FATAL) << "Vertex array is not Vec3 or Vec3d. Not implemented" << std::endl;
            throw "Vertex array is not Vec3 or Vec3d. Not implemented";
            //_succeeded = false;
            //return;
        }
        mesh->SetControlPointAt(vertex, it->second);


        const osg::Array * basenormals = pGeometry->getNormalArray();

        if (basenormals && basenormals->getNumElements()>0)
        {
            FbxVector4 normal;
            if (basenormals->getType() == osg::Array::Vec3ArrayType)
            {
                const osg::Vec3  & vec = (*static_cast<const osg::Vec3Array  *>(basenormals))[normalIndex];
                normal.Set(vec.x(), vec.y(), vec.z(), 0);
            }
            else if (basenormals->getType() == osg::Array::Vec3dArrayType)
            {
                const osg::Vec3d & vec = (*static_cast<const osg::Vec3dArray *>(basenormals))[normalIndex];
                normal.Set(vec.x(), vec.y(), vec.z(), 0);
            }
            else
            {
                OSG_NOTIFY(osg::FATAL) << "Normal array is not Vec3 or Vec3d. Not implemented" << std::endl;
                throw "Normal array is not Vec3 or Vec3d. Not implemented";
                //_succeeded = false;
                //return;
            }

            //switch (pGeometry->getNormalBinding())
            //{
            //case osg::Geometry::BIND_PER_PRIMITIVE_SET:
            //case osg::Geometry::BIND_PER_VERTEX:
            //    break;
            //}
            lLayerElementNormal->GetDirectArray().SetAt(it->second, normal);
        }

        if (texcoords)
        {
            const osg::Array * basetexcoords = pGeometry->getTexCoordArray(0);
            if (basetexcoords && basetexcoords->getNumElements()>0)
            {
                FbxVector2 texcoord;
                if (basetexcoords->getType() == osg::Array::Vec2ArrayType)
                {
                    const osg::Vec2 & vec = (*static_cast<const osg::Vec2Array *>(basetexcoords))[vertexIndex];
                    texcoord.Set(vec.x(), vec.y());
                }
                else if (basetexcoords->getType() == osg::Array::Vec2dArrayType)
                {
                    const osg::Vec2d & vec = (*static_cast<const osg::Vec2dArray *>(basetexcoords))[vertexIndex];
                    texcoord.Set(vec.x(), vec.y());
                }
                else
                {
                    OSG_NOTIFY(osg::FATAL) << "Texture coords array is not Vec2 or Vec2d. Not implemented" << std::endl;
                    throw "Texture coords array is not Vec2 or Vec2d. Not implemented";
                    //_succeeded = false;
                    //return;
                }

                lUVDiffuseLayer->GetDirectArray().SetAt(it->second, texcoord);
            }
        }
    }
}

void WriterNodeVisitor::buildFaces(const osg::Geode& geo,
                                   ListTriangle&     listTriangles,
                                   bool              texcoords)
{
    MapIndices index_vert;
    FbxMesh* mesh = FbxMesh::Create(_pSdkManager, geo.getName().c_str());
    _curFbxNode->AddNodeAttribute(mesh);
    _curFbxNode->SetShadingMode(FbxNode::eTextureShading);
    FbxLayer* lLayer = mesh->GetLayer(0);
    if (lLayer == NULL)
    {
        mesh->CreateLayer();
        lLayer = mesh->GetLayer(0);
    }
    setLayerTextureAndMaterial(mesh);
    lLayer->GetTextures(FbxLayerElement::eTextureDiffuse)->GetIndexArray().SetCount(listTriangles.size());
    lLayer->GetMaterials()->GetIndexArray().SetCount(listTriangles.size());

    unsigned int i = 0;
    for (ListTriangle::iterator it = listTriangles.begin(); it != listTriangles.end(); ++it, ++i) //Go through the triangle list to define meshs
    {
        if (it->first.material == -1)
        {
            mesh->BeginPolygon();
        }
        else
        {
            mesh->BeginPolygon(i);
            lLayer->GetTextures(FbxLayerElement::eTextureDiffuse)->GetIndexArray().SetAt(i, it->first.material);
            lLayer->GetMaterials()->GetIndexArray().SetAt(i, it->first.material);
        }
        addPolygon(mesh, index_vert, it->first, it->second);
        mesh->EndPolygon();
    }
    setControlPointAndNormalsAndUV(geo, index_vert, texcoords, mesh);
}

void WriterNodeVisitor::createListTriangle(const osg::Geometry* geo,
                                           ListTriangle&        listTriangles,
                                           bool&                texcoords,
                                           unsigned int&        drawable_n)
{
    unsigned int nbVertices = 0;
    {
        const osg::Array * vecs = geo->getVertexArray();
        if (vecs)
        {
            nbVertices = vecs->getNumElements();

            // Texture coords
            const osg::Array * texvec = geo->getTexCoordArray(0);
            if (texvec)
            {
                unsigned int nb = texvec->getNumElements();
                if (nb == nbVertices) texcoords = true;
                else
                {
                    OSG_WARN << "There are more/less texture coords than vertices! Ignoring texture coords.";
                }
            }
        }
    }

    if (nbVertices==0) return;

    int material = processStateSet(_currentStateSet.get());

    PrimitiveIndexWriter pif(geo, listTriangles, drawable_n, material);
    for (unsigned int iPrimSet = 0; iPrimSet < geo->getNumPrimitiveSets(); ++iPrimSet) //Fill the Triangle List
    {
        const osg::PrimitiveSet* ps = geo->getPrimitiveSet(iPrimSet);
        const_cast<osg::PrimitiveSet*>(ps)->accept(pif);
    }
}

void WriterNodeVisitor::apply(osg::Geode& node)
{
    FbxNode* parent = _curFbxNode;
    FbxNode* nodeFBX = FbxNode::Create(_pSdkManager, node.getName().empty() ? "DefaultName" : node.getName().c_str());
    _curFbxNode->AddChild(nodeFBX);
    _curFbxNode = nodeFBX;
    unsigned int count = node.getNumDrawables();
    ListTriangle listTriangles;
    bool texcoords = false;
    for (MaterialMap::iterator it = _materialMap.begin(); it != _materialMap.end(); ++it)
        it->second.setIndex(-1);
    _lastMaterialIndex = 0;
    if(node.getStateSet()){
        pushStateSet(node.getStateSet());
    }
    for (unsigned int i = 0; i < count; ++i)
    {
        const osg::Geometry* g = node.getDrawable(i)->asGeometry();
        if (g != NULL)
        {
            pushStateSet(g->getStateSet());
            createListTriangle(g, listTriangles, texcoords, i);
            popStateSet(g->getStateSet());
        }
    }
    if(node.getStateSet()){
        popStateSet(node.getStateSet());
    }
    if (count > 0)
    {
        buildFaces(node, listTriangles, texcoords);
    }
    if (succeedLastApply())
        traverse(node);
    _curFbxNode = parent;
}

void WriterNodeVisitor::apply(osg::Group& node)
{
    FbxNode* parent = _curFbxNode;

    FbxNode* nodeFBX = FbxNode::Create(_pSdkManager, node.getName().empty() ? "DefaultName" : node.getName().c_str());
    _curFbxNode->AddChild(nodeFBX);
    _curFbxNode = nodeFBX;
    traverse(node);
    _curFbxNode = parent;
}

void WriterNodeVisitor::apply(osg::MatrixTransform& node)
{
    FbxNode* parent = _curFbxNode;
    _curFbxNode = FbxNode::Create(_pSdkManager, node.getName().empty() ? "DefaultName" : node.getName().c_str());
    parent->AddChild(_curFbxNode);

    const osg::Matrix& matrix = node.getMatrix();
    osg::Vec3d pos, scl;
    osg::Quat rot, so;

    matrix.decompose(pos, rot, scl, so);
    _curFbxNode->LclTranslation.Set(FbxDouble3(pos.x(), pos.y(), pos.z()));
    _curFbxNode->LclScaling.Set(FbxDouble3(scl.x(), scl.y(), scl.z()));

    FbxAMatrix mat;

    FbxQuaternion q(rot.x(), rot.y(), rot.z(), rot.w());
    mat.SetQ(q);
    FbxVector4 vec4 = mat.GetR();

    _curFbxNode->LclRotation.Set(FbxDouble3(vec4[0], vec4[1], vec4[2]));

    traverse(node);
    _curFbxNode = parent;
}

// end namespace pluginfbx
}
