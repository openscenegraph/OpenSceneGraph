// GeoSetBuilder.cpp

#ifdef WIN32
#pragma warning( disable : 4786 )
#endif

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
#include <osg/Transparency>
#include <osg/Geode>
#include <osg/GeoSet>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Texture>
#include <osg/TexEnv>
#include <osg/CullFace>
#include <osg/PolygonOffset>
#include <osg/Point>
#include <osg/Notify>

#include <map>
#include <algorithm>

using namespace flt;


////////////////////////////////////////////////////////////////////
//
//                       TmpGeoSet
//
////////////////////////////////////////////////////////////////////

// GeoSet with dynamic vertex size.

TmpGeoSet::TmpGeoSet(FltFile* pFltFile)
{
    _geoSet = new osg::GeoSet;
    _geoSet->setStateSet( new osg::StateSet );

    _colorPool      = pFltFile->getColorPool();
    _texturePool    = pFltFile->getTexturePool();
    _materialPool   = pFltFile->getMaterialPool();
}


osg::GeoSet* TmpGeoSet::createOsgGeoSet()
{
    int prims = _primLenList.size();
    int indices = _vertexRecList.size();

    if (prims==0 || indices==0)
        return NULL;

    osg::GeoSet* gset = getGeoSet();

    gset->setNumPrims(prims);

    // prim lengths
    switch( gset->getPrimType() )
    {
    case osg::GeoSet::QUAD_STRIP :
    case osg::GeoSet::FLAT_TRIANGLE_FAN :
    case osg::GeoSet::TRIANGLE_FAN :
    case osg::GeoSet::LINE_LOOP :
    case osg::GeoSet::LINE_STRIP :
    case osg::GeoSet::FLAT_LINE_STRIP :
    case osg::GeoSet::TRIANGLE_STRIP :
    case osg::GeoSet::FLAT_TRIANGLE_STRIP :
    case osg::GeoSet::POLYGON :
        {
            int *lens = new int[prims];
            gset->setPrimLengths( lens );
            for (int n=0; n < prims; n++)
                lens[n] = _primLenList[n];
        }
        break;
    }

    // create osg compatible buffers
    gset->setCoords(new osg::Vec3[indices]);

    if (gset->getColorBinding() == osg::GeoSet::BIND_PERVERTEX)
        gset->setColors(new osg::Vec4[indices]);

    if (gset->getNormalBinding() == osg::GeoSet::BIND_PERVERTEX)
        gset->setNormals(new osg::Vec3[indices]);

    if (gset->getTextureBinding() == osg::GeoSet::BIND_PERVERTEX)
        gset->setTextureCoords(new osg::Vec2[indices]);

    // Copy vertices across
    {
        int index;
        VertexRecList::iterator itr;
        for(index=0, itr=_vertexRecList.begin();
            itr!=_vertexRecList.end();
            ++index, ++itr)
        {
            setVertex(gset, index, itr->get());
        }
    }

    return gset;
}


void TmpGeoSet::setVertex(osg::GeoSet* gset, int index, Record* vertex)
{
    osg::Vec3* coords  = gset->getCoords();
    osg::Vec4* colors  = gset->getColors();
    osg::Vec3* normals = gset->getNormals();
    osg::Vec2* texuv   = gset->getTextureCoords();

    switch(vertex->getOpcode())
    {
        case VERTEX_C_OP:
        {
            SVertex* pVert = (SVertex*)vertex->getData();

            coords[index].set(
                (float)pVert->Coord.x(),
                (float)pVert->Coord.y(),
                (float)pVert->Coord.z());

            if (gset->getColorBinding() == osg::GeoSet::BIND_PERVERTEX)
            {
                if (pVert->swFlags & V_NO_COLOR_BIT)
                    colors[index] = osg::Vec4(1,1,1,1);
                else
                {
                    if (pVert->swFlags & V_PACKED_COLOR_BIT)
                        colors[index] = pVert->PackedColor.get();
                    else
                    {
                        ColorPool* pColorPool = _colorPool.get();
                        colors[index] = pColorPool->getColor(pVert->dwVertexColorIndex);
                    }
                }
            }
        }
        break;

        case VERTEX_CN_OP:
        {
            SNormalVertex* pVert = (SNormalVertex*)vertex->getData();

            coords[index].set(
                (float)pVert->Coord.x(),
                (float)pVert->Coord.y(),
                (float)pVert->Coord.z());

            if (gset->getNormalBinding() == osg::GeoSet::BIND_PERVERTEX)
            {
                normals[index].set(
                    (float)pVert->Normal.x(),
                    (float)pVert->Normal.y(),
                    (float)pVert->Normal.z());
                normals[index].normalize();
            }

            if (gset->getColorBinding() == osg::GeoSet::BIND_PERVERTEX)
            {
                if (pVert->swFlags & V_NO_COLOR_BIT)
                    colors[index] = osg::Vec4(1,1,1,1);
                else
                {
                    if (pVert->swFlags & V_PACKED_COLOR_BIT)
                        colors[index] = pVert->PackedColor.get();
                    else
                    {
                        ColorPool* pColorPool = _colorPool.get();
                        colors[index] = pColorPool->getColor(pVert->dwVertexColorIndex);
                    }
                }
            }
        }
        break;

        case VERTEX_CNT_OP:
        {
            SNormalTextureVertex* pVert = (SNormalTextureVertex*)vertex->getData();

            coords[index].set(
                (float)pVert->Coord.x(),
                (float)pVert->Coord.y(),
                (float)pVert->Coord.z());

            if (gset->getNormalBinding() == osg::GeoSet::BIND_PERVERTEX)
            {
                normals[index].set(
                    (float)pVert->Normal.x(),
                    (float)pVert->Normal.y(),
                    (float)pVert->Normal.z());
                normals[index].normalize();
            }

            if (gset->getTextureBinding() == osg::GeoSet::BIND_PERVERTEX)
            {
                texuv[index].set(
                    (float)pVert->Texture.x(),
                    (float)pVert->Texture.y());
            }

            if (gset->getColorBinding() == osg::GeoSet::BIND_PERVERTEX)
            {
                if (pVert->swFlags & V_NO_COLOR_BIT)
                    colors[index] = osg::Vec4(1,1,1,1);
                else
                {
                    if (pVert->swFlags & V_PACKED_COLOR_BIT)
                        colors[index] = pVert->PackedColor.get();
                    else
                    {
                        ColorPool* pColorPool = _colorPool.get();
                        colors[index] = pColorPool->getColor(pVert->dwVertexColorIndex);
                    }
                }
            }
        }
        break;

        case VERTEX_CT_OP:
        {
            STextureVertex* pVert = (STextureVertex*)vertex->getData();

            coords[index].set(
                (float)pVert->Coord.x(),
                (float)pVert->Coord.y(),
                (float)pVert->Coord.z());

            if (gset->getTextureBinding() == osg::GeoSet::BIND_PERVERTEX)
            {
                texuv[index].set(
                    (float)pVert->Texture.x(),
                    (float)pVert->Texture.y());
            }

            if (gset->getColorBinding() == osg::GeoSet::BIND_PERVERTEX)
            {
                osg::Vec4* colors = gset->getColors();

                if (pVert->swFlags & V_NO_COLOR_BIT)
                    colors[index] = osg::Vec4(1,1,1,1);
                else
                {
                    if (pVert->swFlags & V_PACKED_COLOR_BIT)
                        colors[index] = pVert->PackedColor.get();
                    else
                    {
                        ColorPool* pColorPool = _colorPool.get();
                        colors[index] = pColorPool->getColor(pVert->dwVertexColorIndex);
                    }
                }
            }
        }
        break;

        case OLD_VERTEX_OP:
        {
            SOldVertex* pVert = (SOldVertex*)vertex->getData();

            coords[index].set(
                (float)pVert->v[0],
                (float)pVert->v[1],
                (float)pVert->v[2]);

            if ((gset->getTextureBinding() == osg::GeoSet::BIND_PERVERTEX)
            &&  (vertex->getSize() >= sizeof(SOldVertex)))
            {
                texuv[index].set(
                    (float)pVert->t[0],
                    (float)pVert->t[1]);
            }
        }
        break;

        case OLD_VERTEX_COLOR_OP:
        {
            SOldVertexColor* pVert = (SOldVertexColor*)vertex->getData();

            coords[index].set(
                (float)pVert->v[0],
                (float)pVert->v[1],
                (float)pVert->v[2]);

            if (gset->getColorBinding() == osg::GeoSet::BIND_PERVERTEX)
            {
                osg::Vec4* colors = gset->getColors();
                ColorPool* pColorPool = _colorPool.get();
                if (pColorPool)
                    colors[index] = pColorPool->getColor(pVert->color_index);
                else
                    colors[index] = osg::Vec4(1,1,1,1);
            }

            if ((gset->getTextureBinding() == osg::GeoSet::BIND_PERVERTEX)
            &&  (vertex->getSize() >= sizeof(SOldVertexColor)))
            {
                texuv[index].set(
                    (float)pVert->t[0],
                    (float)pVert->t[1]);
            }
        }
        break;

        case OLD_VERTEX_COLOR_NORMAL_OP:
        {
            SOldVertexColorNormal* pVert = (SOldVertexColorNormal*)vertex->getData();

            coords[index].set(
                (float)pVert->v[0],
                (float)pVert->v[1],
                (float)pVert->v[2]);

            if (gset->getNormalBinding() == osg::GeoSet::BIND_PERVERTEX)
            {
                normals[index].set(
                    (float)pVert->n[0] / (1<<30),    // =pow(2,30)
                    (float)pVert->n[1] / (1<<30),
                    (float)pVert->n[2] / (1<<30));
                normals[index].normalize();
            }

            if (gset->getColorBinding() == osg::GeoSet::BIND_PERVERTEX)
            {
                osg::Vec4* colors = gset->getColors();
                ColorPool* pColorPool = _colorPool.get();
                if (pColorPool)
                    colors[index] = pColorPool->getColor(pVert->color_index);
                else
                    colors[index] = osg::Vec4(1,1,1,1);
            }

            if ((gset->getTextureBinding() == osg::GeoSet::BIND_PERVERTEX)
            &&  (vertex->getSize() >= sizeof(SOldVertexColorNormal)))
            {
                texuv[index].set(
                    (float)pVert->t[0],
                    (float)pVert->t[1]);
            }
        }
        break;
    }
}



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



void GeoSetBuilder::initPrimData()
{
    _tmpGeoSet = new TmpGeoSet(_pFltFile.get());
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

    for(TmpGeoSetList::iterator itr=_tmpGeoSetList.begin();
        itr!=_tmpGeoSetList.end();
        ++itr)
    {
        osg::GeoSet* gset = (*itr)->createOsgGeoSet();
        if (gset)
            geode->addDrawable(gset);
    }

    if (bInternalGeodeAllocation && (geode->getNumDrawables() == 0))
    {
        geode->unref();
        return NULL;
    }

    return geode;
}


bool GeoSetBuilder::addPrimitive()
{
    osg::GeoSet* geoset = getGeoSet();

    if (geoset->getPrimType() == osg::GeoSet::NO_TYPE)
        geoset->setPrimType(findPrimType(numberOfVertices()));

    // Still no primitive type?
    if (geoset->getPrimType() == osg::GeoSet::NO_TYPE)
        return false;

    TmpGeoSet* match = findMatchingGeoSet();
    if (match)
        // append vertices and prim length to match
        match->addVertices( _tmpGeoSet.get() );
    else
        // add new GeoSet+StateSet compination
        _tmpGeoSetList.push_back(_tmpGeoSet.get());

    initPrimData();     // initialize _tmpGeoSet

    return true;
}


TmpGeoSet* GeoSetBuilder::findMatchingGeoSet()
{
    osg::GeoSet* geoSet = getGeoSet();
    osg::StateSet* stateSet = geoSet->getStateSet();

    for(TmpGeoSetList::iterator itr=_tmpGeoSetList.begin();
        itr!=_tmpGeoSetList.end();
        ++itr)
    {
        TmpGeoSet* tmpgeoset = itr->get();
        osg::GeoSet* gset = tmpgeoset->getGeoSet();
        osg::StateSet* sset = gset->getStateSet();

        // Do we have a match?
        if ((geoSet->getPrimType() == gset->getPrimType())
        &&  (geoSet->getColorBinding() == gset->getColorBinding())
        &&  (geoSet->getNormalBinding() == gset->getNormalBinding())
        &&  (geoSet->getTextureBinding() == gset->getTextureBinding())
        &&  (stateSet->compare(*sset, true) == 0))
        {
            if (geoSet->getColorBinding() == osg::GeoSet::BIND_OVERALL)
            {
                osg::Vec4* col1 = geoSet->getColors();
                osg::Vec4* col2 = gset->getColors();
                if (*col1 != *col2)
                    return NULL;
            }

            return tmpgeoset;
        }
    }

    return NULL;
}


osg::GeoSet::PrimitiveType GeoSetBuilder::findPrimType(const int nVertices)
{
    osg::GeoSet::PrimitiveType primtype = osg::GeoSet::NO_TYPE;

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


