// GeoSetBuilder.cpp

#include "flt.h"
#include "FltFile.h"
#include "Pool.h"
#include "opcodes.h"
#include "VertexPoolRecords.h"
#include "OldVertexRecords.h"
#include "MaterialPaletteRecord.h"
#include "GeoSetBuilder.h"

#include <osg/Object>
#include <osg/LOD>
#include <osg/GeoState>
#include <osg/Transparency>
#include <osg/GeoSet>
#include <osg/GeoState>
#include <osg/Geode>
#include <osg/Material>
#include <osg/Texture>
#include <osg/TexEnv>
#include <osg/Notify>

#include <algorithm>
#include <map>

using namespace flt;


////////////////////////////////////////////////////////////////////
//
//                       GeoSetBuilder
//
////////////////////////////////////////////////////////////////////

// OpenFlight don't save data in GeoSets.  This class tries to find
// existing GeoSets with matching state before creating a new GeoSet.

GeoSetBuilder::GeoSetBuilder(FltFile* pFltFile)
{
    _pFltFile = pFltFile;
    initPrimData();
}


// virtual
GeoSetBuilder::~GeoSetBuilder()
{
    for(GeoSetList::iterator itr=_aGeoSet.begin();
        itr!=_aGeoSet.end();
        ++itr)
    {
        delete *itr;
    }
}


void GeoSetBuilder::initPrimData()
{
    _appearance.init();
    _aVertex.erase(_aVertex.begin(), _aVertex.end());
}


// Convert flt::TmpGeoSet's to osg::GeoSet's and add to osg::Geode.
// If geode parameter is NULL create new.
// If geode created inside this function and no osg::GeoSet's
// added free geode.
osg::Geode* GeoSetBuilder::createOsgGeoSets(osg::Geode* geode)
{
    bool bInternalGeodeAllocation = false;

    if (geode == NULL)
    {
        geode = new osg::Geode;
        bInternalGeodeAllocation = true;
    }

    for(GeoSetList::iterator itr=_aGeoSet.begin();
        itr!=_aGeoSet.end();
        ++itr)
    {
        osg::GeoSet* gset = (*itr)->createOsgGeoSet();
        if (gset)
            geode->addGeoSet(gset);
    }

    if (bInternalGeodeAllocation && (geode->getNumGeosets() == 0))
    {
        geode->unref();
        return NULL;
    }

    return geode;
}


void GeoSetBuilder::addVertex(Record* vertex)
{
    _aVertex.push_back(vertex);
    _appearance.setVertexOp(vertex->getOpcode());
}


bool GeoSetBuilder::addPrimitive()
{
    int nVertices = _aVertex.size();

    if (nVertices == 0) return false;

    if (_appearance.getPrimType() == osg::GeoSet::NO_TYPE)
        _appearance.setPrimType(findPrimType(nVertices));

    TmpGeoSet* gset = findMatchingGeoSet();
    if (gset)
    {
        addTo(gset);
        if (_appearance.getMaterial())
        {
            _appearance.getMaterial()->unref();
            _appearance.setMaterial(NULL);
        }
    }
    else
        addToNew();

    initPrimData();

    return true;
}


//////////////////////// protected /////////////////////////////////


TmpGeoSet* GeoSetBuilder::findMatchingGeoSet()
{

    for(std::vector<TmpGeoSet*>::iterator itr=_aGeoSet.begin();
        itr!=_aGeoSet.end();
        ++itr)
    {
        if (_appearance == (*itr)->_appearance)
            return *itr;
    }
    
    return NULL;
}


void GeoSetBuilder::addTo(TmpGeoSet* gset)
{
    int nVertices = _aVertex.size();
    gset->addPrimLen(nVertices);

    for(std::vector<Record*>::iterator itr=_aVertex.begin();
        itr!=_aVertex.end();
        ++itr)
    {
        gset->addVertex(*itr);
    }
}


void GeoSetBuilder::addToNew()
{
    TmpGeoSet* gset = new TmpGeoSet(_pFltFile);
    if (gset == NULL) return;

    // Transfer data to TmpGeoSet
    gset->_appearance = _appearance;
    addTo(gset);

    _aGeoSet.push_back(gset);
}


PrimitiveType GeoSetBuilder::findPrimType( int nVertices)
{
    PrimitiveType primtype = osg::GeoSet::NO_TYPE;

    switch (nVertices)
    {
    case 1:
        primtype = osg::GeoSet::POINTS;
        break;
    case 2:
        primtype = osg::GeoSet::LINES;
        break;
    case 3:
        primtype = osg::GeoSet::TRIANGLES;
        break;
    case 4:
        primtype = osg::GeoSet::QUADS;
        break;
    default:
        if (nVertices >= 5) primtype = osg::GeoSet::POLYGON;
        break;
    }

    return primtype;
}


////////////////////////////////////////////////////////////////////
//
//                       TmpGeoSet
//
////////////////////////////////////////////////////////////////////

// GeoSet with dynamic size.  Used by GeoSetBuilder as a temp. buffer.


TmpGeoSet::TmpGeoSet(FltFile* pFltFile)
{
    _pFltFile = pFltFile;
}


TmpGeoSet::~TmpGeoSet()
{
}


void TmpGeoSet::addVertex(Record* vertex)
{
    _appearance.setVertexOp(vertex->getOpcode());
    _aVertex.push_back(vertex);
}


void TmpGeoSet::addPrimLen(int len)
{
    _aPrimLen.push_back(len);
}


osg::GeoSet* TmpGeoSet::createOsgGeoSet()
{
    int prims = _aPrimLen.size();
    int indices = _aVertex.size();

    if (prims==0 || indices==0)
        return NULL;

    osg::GeoSet* gset = new osg::GeoSet;

    gset->setNumPrims(prims);
    gset->setPrimType(_appearance.getPrimType());

    osg::GeoState* gstate = new osg::GeoState;
    gset->setGeoState(gstate);

    // Material
    osg::Material* osgMaterial = _appearance.getMaterial();
    if (osgMaterial)
    {
        gstate->setAttribute(osg::GeoState::MATERIAL, osgMaterial);
    }

    // Color BIND_OVERALL
    if (_appearance.getColorBinding() == osg::GeoSet::BIND_OVERALL)
    {
        osg::Vec4* color = new osg::Vec4[1];
        *color = _appearance.getColor();
        gset->setColorBinding(_appearance.getColorBinding());
        gset->setColors(color);
    }

    // Color BIND_PERVERTEX
    if (_appearance.getColorBinding() == osg::GeoSet::BIND_PERVERTEX)
    {
        gset->setColorBinding(_appearance.getColorBinding());
        gset->setColors(new osg::Vec4[indices]);
    }

    // Transparency
    if (_appearance.getTransparency())
    {
        gstate->setMode(osg::GeoState::TRANSPARENCY, osg::GeoState::ON);
        gstate->setAttribute(osg::GeoState::TRANSPARENCY, new osg::Transparency);
    }

    // Cull face
    if (_appearance.getCullface())
    {
        osg::CullFace* cullface = new osg::CullFace;
        if (cullface)
        {
            gstate->setMode(osg::GeoState::FACE_CULL, osg::GeoState::ON);
            cullface->setMode(osg::CullFace::BACK);
            gstate->setAttribute(osg::GeoState::FACE_CULL, cullface);
        }
    }
    else
    {
        gstate->setMode(osg::GeoState::FACE_CULL, osg::GeoState::OFF);
    }

    // Texture
    if (_appearance.getTexture())
    {
        gstate->setMode(      osg::GeoState::TEXTURE, osg::GeoState::ON );
        gstate->setAttribute( osg::GeoState::TEXTURE, _appearance.getTexture() );
        gstate->setAttribute( osg::GeoState::TEXENV, new osg::TexEnv );
    }

    // Lighting
    if (_appearance.getLighting())
        gstate->setMode( osg::GeoState::LIGHTING, osg::GeoState::ON );
    else
        gstate->setMode( osg::GeoState::LIGHTING, osg::GeoState::OFF );

    // Subface
    if (_appearance.getSubface() > 0)
    {
        osg::PolygonOffset* polyoffset = new osg::PolygonOffset;
        if (polyoffset)
        {
            int level = _appearance.getSubface();
            gstate->setMode(osg::GeoState::POLYGON_OFFSET,osg::GeoState::ON);
            polyoffset->setOffset(-1*level, -20*level);
            gstate->setAttribute(osg::GeoState::POLYGON_OFFSET, polyoffset);
        }
    }

    // Point
    if (_appearance.getPrimType() == osg::GeoSet::POINTS)
    {
        osg::Point* point = new osg::Point;
        if (point)
        {
            gstate->setMode(osg::GeoState::POINT,osg::GeoState::ON);
            point->setSize(8);
            point->enableSmooth();
            gstate->setAttribute(osg::GeoState::POINT, point);
        }
    }

    // PrimLengths

    int nPrimLenSize = 1;
    if (_appearance.getPrimType() == osg::GeoSet::POLYGON)
        nPrimLenSize = prims;
    int *lens = new int[nPrimLenSize];
    gset->setPrimLengths( lens );
    for (int n=0; n < nPrimLenSize; n++)
        lens[n] = _aPrimLen[n];

    // Vertices
    switch(_appearance.getVertexOp())
    {
    case VERTEX_C_OP:
        gset->setCoords(new osg::Vec3[indices]);
       break;
    case VERTEX_CN_OP:
        gset->setNormalBinding(osg::GeoSet::BIND_PERVERTEX);
        gset->setCoords(new osg::Vec3[indices]);
        gset->setNormals(new osg::Vec3[indices]);
        break;
    case VERTEX_CT_OP:
        gset->setTextureBinding(osg::GeoSet::BIND_PERVERTEX);
        gset->setCoords(new osg::Vec3[indices]);
        gset->setTextureCoords(new osg::Vec2[indices]);
        break;
    case VERTEX_CNT_OP:
        gset->setNormalBinding(osg::GeoSet::BIND_PERVERTEX);
        gset->setTextureBinding(osg::GeoSet::BIND_PERVERTEX);
        gset->setCoords(new osg::Vec3[indices]);
        gset->setNormals(new osg::Vec3[indices]);
        gset->setTextureCoords(new osg::Vec2[indices]);
        break;
    case OLD_VERTEX_OP:
        gset->setCoords(new osg::Vec3[indices]);
       break;
    case OLD_VERTEX_COLOR_OP:
        gset->setCoords(new osg::Vec3[indices]);
       break;
    case OLD_VERTEX_COLOR_NORMAL_OP:
        gset->setCoords(new osg::Vec3[indices]);
//      gset->setNormals(new osg::Vec3[indices]);
       break;

    }

    // Copy vertices
    {
        int index;
        std::vector<Record*>::iterator itr;
        for(index=0, itr=_aVertex.begin();
            itr!=_aVertex.end();
            ++index, ++itr)
        {
            setVertex(gset, index, *itr);
        }
    }

    return gset;
}


///////////////////////// private //////////////////////////////////


void TmpGeoSet::setVertex(osg::GeoSet* gset, int index, Record* vertex)
{
    bool bColorBindPerVertex = _appearance.getColorBinding() == osg::GeoSet::BIND_PERVERTEX;

    switch(_appearance.getVertexOp())
    {
    case VERTEX_C_OP:
        {
            SVertex* pVert = (SVertex*)vertex->getData();
            osg::Vec3* coords = gset->getCoords();

            coords[index].set(
                (float)pVert->Coord.x(),
                (float)pVert->Coord.y(),
                (float)pVert->Coord.z());
            if (bColorBindPerVertex)
            {
                osg::Vec4* colors = gset->getColors();

                if (pVert->swFlags & BIT3)
                     colors[index] = pVert->PackedColor.get();
                else
                {
                    ColorPool* pColorPool = _pFltFile->getColorPool();
                    if (pColorPool)
                        colors[index] = pColorPool->getColor(pVert->dwVertexColorIndex);
                }
            }
        }
        break;

    case VERTEX_CN_OP:
        {
            SNormalVertex* pVert = (SNormalVertex*)vertex->getData();
            osg::Vec3* coords = gset->getCoords();
            osg::Vec3* normals = gset->getNormals();

            coords[index].set(
                (float)pVert->Coord.x(),
                (float)pVert->Coord.y(),
                (float)pVert->Coord.z());
            normals[index].set(
                (float)pVert->Normal.x(),
                (float)pVert->Normal.y(),
                (float)pVert->Normal.z());
            if (bColorBindPerVertex)
            {
                osg::Vec4* colors = gset->getColors();

                if (pVert->swFlags & BIT3)
                     colors[index] = pVert->PackedColor.get();
                else
                {
                    ColorPool* pColorPool = _pFltFile->getColorPool();
                    if (pColorPool)
                        colors[index] = pColorPool->getColor(pVert->dwVertexColorIndex);
                }
            }
        }
        break;

    case VERTEX_CNT_OP:
        {
            SNormalTextureVertex* pVert = (SNormalTextureVertex*)vertex->getData();
            osg::Vec3* coords = gset->getCoords();
            osg::Vec3* normals = gset->getNormals();
            osg::Vec2* texuv = gset->getTCoords();

            coords[index].set(
                (float)pVert->Coord.x(),
                (float)pVert->Coord.y(),
                (float)pVert->Coord.z());
            normals[index].set(
                (float)pVert->Normal.x(),
                (float)pVert->Normal.y(),
                (float)pVert->Normal.z());
            texuv[index].set(
                (float)pVert->Texture.x(),
                (float)pVert->Texture.y());
            if (bColorBindPerVertex)
            {
                osg::Vec4* colors = gset->getColors();

                if (pVert->swFlags & BIT3)
                     colors[index] = pVert->PackedColor.get();
                else
                {
                    ColorPool* pColorPool = _pFltFile->getColorPool();
                    if (pColorPool)
                        colors[index] = pColorPool->getColor(pVert->dwVertexColorIndex);
                }
            }
        }
        break;

    case VERTEX_CT_OP:
        {
            STextureVertex* pVert = (STextureVertex*)vertex->getData();
            osg::Vec3* coords = gset->getCoords();
            osg::Vec2* texuv = gset->getTCoords();

            coords[index].set(
                (float)pVert->Coord.x(),
                (float)pVert->Coord.y(),
                (float)pVert->Coord.z());
            texuv[index].set(
                (float)pVert->Texture.x(),
                (float)pVert->Texture.y());
            if (bColorBindPerVertex)
            {
                osg::Vec4* colors = gset->getColors();

                if (pVert->swFlags & BIT3)
                     colors[index] = pVert->PackedColor.get();
                else
                {
                    ColorPool* pColorPool = _pFltFile->getColorPool();
                    if (pColorPool)
                        colors[index] = pColorPool->getColor(pVert->dwVertexColorIndex);
                }
            }
        }
        break;

    case OLD_VERTEX_OP:
        {
            SOldVertex* pVert = (SOldVertex*)vertex->getData();
            osg::Vec3* coords = gset->getCoords();

            coords[index].set(
                (float)pVert->v[0],
                (float)pVert->v[1],
                (float)pVert->v[2]);
        }
        break;

    case OLD_VERTEX_COLOR_OP:
        {
            SOldVertexColor* pVert = (SOldVertexColor*)vertex->getData();
            osg::Vec3* coords = gset->getCoords();

            coords[index].set(
                (float)pVert->v[0],
                (float)pVert->v[1],
                (float)pVert->v[2]);
        }
        break;

    case OLD_VERTEX_COLOR_NORMAL_OP:
        {
            SOldVertexColorNormal* pVert = (SOldVertexColorNormal*)vertex->getData();
            osg::Vec3* coords = gset->getCoords();
//          osg::Vec3* normals = gset->getNormals();

            coords[index].set(
                (float)pVert->Coord[0],
                (float)pVert->Coord[1],
                (float)pVert->Coord[2]);
/*
            normals[index].set(
                (float)pVert->Normal[0],
                (float)pVert->Normal[1],
                (float)pVert->Normal[2]);
*/
        }
        break;
    }
}


