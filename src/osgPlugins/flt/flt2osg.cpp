// flt2osg.cpp

#include <string.h>
#include "osg/GL"

#include <osg/Group>
#include <osg/LOD>
#include <osg/Transform>
#include <osg/Switch>
#include <osg/Geode>
#include <osg/GeoSet>
#include <osg/StateSet>
#include <osg/CullFace>
#include <osg/TexEnv>
#include <osg/Point>
#include <osg/Material>
#include <osg/PolygonOffset>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Billboard>
#include <osg/Texture>
#include <osg/Image>
#include <osg/Notify>

#include <osgDB/FileNameUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgDB/Registry>

#include "opcodes.h"
#include "flt.h"
#include "flt2osg.h"
#include "FltFile.h"
#include "Registry.h"
#include "Record.h"
#include "HeaderRecord.h"
#include "ColorPaletteRecord.h"
#include "MaterialPaletteRecord.h"
#include "OldMaterialPaletteRecord.h"
#include "TexturePaletteRecord.h"
#include "VertexPoolRecords.h"
#include "GroupRecord.h"
#include "LodRecord.h"
#include "DofRecord.h"
#include "SwitchRecord.h"
#include "ObjectRecord.h"
#include "FaceRecord.h"
#include "TransformationRecords.h"
#include "ExternalRecord.h"
#include "LightPointRecord.h"
#include "Input.h"
#include "GeoSetBuilder.h"
#include "LongIDRecord.h"

using namespace flt;

ConvertFromFLT::ConvertFromFLT(FltFile* pFltFile)
{
    _pFltFile = pFltFile;
    _diOpenFlightVersion = 0;
    _diCurrentOffset = 0;
    _wObjTransparency = 0;
    _nSubfaceLevel = 0;
    _sfHdrUnitScale = 1;
    _bHdrRgbMode = false;
}


ConvertFromFLT::~ConvertFromFLT()
{
}


osg::Node* ConvertFromFLT::convert(Record* rec)
{
    if (rec==NULL) return NULL;
    return visitNode(NULL, rec);
}


////////////////////////////////////////////////////////////////////

Record* ConvertFromFLT::getVertexFromPool(int nOffset)
{
    VertexPaletteOffsetMap::iterator fitr = _VertexPaletteOffsetMap.find(nOffset);
    if (fitr != _VertexPaletteOffsetMap.end())
    {
        return (*fitr).second;
    }
    else return NULL;
}


void ConvertFromFLT::regisiterVertex(int nOffset, Record* pRec)
{
    _VertexPaletteOffsetMap[nOffset] = pRec;
}


////////////////////////////////////////////////////////////////////

osg::Node* ConvertFromFLT::visitNode(osg::Group* osgParent, Record* rec)
{
    if (rec==NULL) return NULL;

    if      (rec->isOfType(HEADER_OP))              return visitHeader(osgParent, (HeaderRecord*)rec);
    else if (rec->isOfType(COLOR_PALETTE_OP))       return visitColorPalette(osgParent, (ColorPaletteRecord*)rec);
    else if (rec->isOfType(MATERIAL_PALETTE_OP))    return visitMaterialPalette(osgParent, (MaterialPaletteRecord*)rec);
    else if (rec->isOfType(OLD_MATERIAL_PALETTE_OP))return visitOldMaterialPalette(osgParent, (OldMaterialPaletteRecord*)rec);
    else if (rec->isOfType(TEXTURE_PALETTE_OP))     return visitTexturePalette(osgParent, (TexturePaletteRecord*)rec);
    else if (rec->isOfType(VERTEX_PALETTE_OP))      return visitVertexPalette(osgParent, (VertexPaletteRecord*)rec);
    else if (rec->isOfType(VERTEX_C_OP))            return visitVertex(osgParent, (VertexRecord*)rec);
    else if (rec->isOfType(VERTEX_CN_OP))           return visitNormalVertex(osgParent, (NormalVertexRecord*)rec);
    else if (rec->isOfType(VERTEX_CNT_OP))          return visitNormalTextureVertex(osgParent, (NormalTextureVertexRecord*)rec);
    else if (rec->isOfType(VERTEX_CT_OP))           return visitTextureVertex(osgParent, (TextureVertexRecord*)rec);
    else if (rec->isOfType(GROUP_OP))               return visitGroup(osgParent, (GroupRecord*)rec);
    else if (rec->isOfType(LOD_OP))                 return visitLOD(osgParent, (LodRecord*)rec);
    else if (rec->isOfType(OLD_LOD_OP))             return visitOldLOD(osgParent, (OldLodRecord*)rec);
    else if (rec->isOfType(DOF_OP))                 return visitDOF(osgParent, (DofRecord*)rec);
    else if (rec->isOfType(SWITCH_OP))              return visitSwitch(osgParent, (SwitchRecord*)rec);
    else if (rec->isOfType(OBJECT_OP))              return visitObject(osgParent, (ObjectRecord*)rec);
    else if (rec->isOfType(EXTERNAL_REFERENCE_OP))  return visitExternal(osgParent, (ExternalRecord*)rec);
    else if (rec->isOfType(MATRIX_OP))              return visitMatrix(osgParent, (MatrixRecord*)rec);
    else if (rec->isOfType(LONG_ID_OP))             return visitLongID(osgParent, (LongIDRecord*)rec);

    return NULL;
}


osg::Node* ConvertFromFLT::visitAncillary(osg::Group* osgParent, PrimNodeRecord* rec)
{
    osg::Node* node = NULL;

    // Visit
    for(int i=0; i < rec->getNumChildren(); i++)
    {
        Record* child = rec->getChild(i);

        if (child && child->isAncillaryRecord())
        {
            node = visitNode(osgParent, child);
            if (node) osgParent = (osg::Group*)node;
        }
    }

    return osgParent;
}


osg::Node* ConvertFromFLT::visitPrimaryNode(osg::Group* osgParent, PrimNodeRecord* rec)
{
    osg::Node* node = NULL;
    GeoSetBuilder   geoSetBuilder(_pFltFile.get());

    // Visit
    for(int i=0; i < rec->getNumChildren(); i++)
    {
        Record* child = rec->getChild(i);

        if (child && child->isPrimaryNode())
        {
            if (child->isOfType(FACE_OP))
                visitFace(&geoSetBuilder, (FaceRecord*)child);
            else if (child->isOfType(LIGHT_POINT_OP))
                visitLightPoint(&geoSetBuilder, (LightPointRecord*)child);
            else
                node = visitNode(osgParent, child);
        }
    }

    osg::Geode* geode = geoSetBuilder.createOsgGeoSets();
    if (osgParent && geode)
        osgParent->addChild( geode );

    return node;
}


osg::Node* ConvertFromFLT::visitLongID(osg::Group* osgParent, LongIDRecord* rec)
{
    SLongID *pSLongID = (SLongID*)rec->getData();

    osgParent->setName(pSLongID->szIdent);

    return NULL;
}


osg::Node* ConvertFromFLT::visitHeader(osg::Group* osgParent, HeaderRecord* rec)
{
    SHeader *pSHeader = (SHeader*)rec->getData();

    // Version
    _diOpenFlightVersion = pSHeader->diFormatRevLev;
    osg::notify(osg::INFO) << "Version " << _diOpenFlightVersion << endl;

    // Unit scale
    if (pSHeader->iMultDivUnit < 0)
        _sfHdrUnitScale = 1.f / -pSHeader->iMultDivUnit;
    else if (pSHeader->iMultDivUnit > 0)
        _sfHdrUnitScale = pSHeader->iMultDivUnit;

    _bHdrRgbMode = (pSHeader->dwFlags & 0x40000000) ? true : false;    // RGB space (=packed color)

    // Create root group node
    osg::Group* group = new osg::Group;
    if (group)
    {
        visitAncillary(osgParent, rec);

        visitPrimaryNode(group, (PrimNodeRecord*)rec);
        group->setName(pSHeader->szIdent);
        if (osgParent) osgParent->addChild(group);
    }

    return (osg::Node*)group;
}


                                 /*osgParent*/
osg::Node* ConvertFromFLT::visitColorPalette(osg::Group* , ColorPaletteRecord* rec)
{
    if (!_pFltFile->useColorPalette()) return NULL;

    ColorPool* pColorPool = _pFltFile->getColorPool();

    if (_diOpenFlightVersion > 13)
    {
        SColorPalette* pCol = (SColorPalette*)rec->getData();
        for (int i=0; i < 1024; i++)
        {
            osg::Vec4 color(pCol->Colors[i].get());
            color[3] = 1.0f;    // Force alpha to one
            pColorPool->addColor(i, color);
        }
    }
    else    // version 11, 12 & 13
    {
        SOldColorPalette* pCol = (SOldColorPalette*)rec->getData();
        unsigned int i;
        for (i=0; i < sizeof(pCol->Colors)/sizeof(pCol->Colors[0]); i++)
        {
            osg::Vec4 color(pCol->Colors[i].get());
            color[3] = 1.0f;    // Force alpha to one
            pColorPool->addColor(i, color);
        }

        for (i=0; i < sizeof(pCol->FixedColors)/sizeof(pCol->FixedColors[0]); i++)
        {
            osg::Vec4 color(pCol->FixedColors[i].get());
            color[3] = 1.0f;    // Force alpha to one
            pColorPool->addColor(i+4096, color);
        }
    }

    return NULL;
}


                                 /*osgParent*/
osg::Node* ConvertFromFLT::visitMaterialPalette(osg::Group* , MaterialPaletteRecord* rec)
{
    if (!_pFltFile->useMaterialPalette()) return NULL;

    SMaterial* pSMaterial = (SMaterial*)rec->getData();
    MaterialPool* pMaterialPool = _pFltFile->getMaterialPool();
    if (pSMaterial && pMaterialPool)
    {
        MaterialPool::PoolMaterial* pPoolMat = new MaterialPool::PoolMaterial;

        pPoolMat->Ambient   = pSMaterial->Ambient;
        pPoolMat->Diffuse   = pSMaterial->Diffuse;
        pPoolMat->Specular  = pSMaterial->Specular;
        pPoolMat->Emissive  = pSMaterial->Emissive;
        pPoolMat->sfShininess = pSMaterial->sfShininess;
        pPoolMat->sfAlpha   = pSMaterial->sfAlpha;

        pMaterialPool->addMaterial((int)pSMaterial->diIndex, pPoolMat);
    }
    return NULL;
}

                                 /*osgParent*/
osg::Node* ConvertFromFLT::visitOldMaterialPalette(osg::Group* , OldMaterialPaletteRecord* rec)
{
    if (!_pFltFile->useMaterialPalette()) return NULL;

    SOldMaterial* pSMaterial = (SOldMaterial*)rec->getData();
    MaterialPool* pMaterialPool = _pFltFile->getMaterialPool();

    if (pSMaterial && pMaterialPool )
    {
        for (int i=0; i < 64; i++)
        {
            MaterialPool::PoolMaterial* pPoolMat = new MaterialPool::PoolMaterial;

            pPoolMat->Ambient   = pSMaterial->mat[i].Ambient;
            pPoolMat->Diffuse   = pSMaterial->mat[i].Diffuse;
            pPoolMat->Specular  = pSMaterial->mat[i].Specular;
            pPoolMat->Emissive  = pSMaterial->mat[i].Emissive;
            pPoolMat->sfShininess = pSMaterial->mat[i].sfShininess;
            pPoolMat->sfAlpha   = pSMaterial->mat[i].sfAlpha;
            
            pMaterialPool->addMaterial(i, pPoolMat);
        }
    }
    return NULL;
}

                                 /*osgParent*/
osg::Node* ConvertFromFLT::visitTexturePalette(osg::Group* , TexturePaletteRecord* rec)
{
    if (!_pFltFile->useTexturePalette()) return NULL;

    STexturePalette* pTexture = (STexturePalette*)rec->getData();
    TexturePool* pTexturePool = _pFltFile->getTexturePool();

    if (pTexture && pTexturePool)
    {
        osg::Texture *osgTexture;

        osgTexture = Registry::instance()->getTexture(pTexture->szFilename);
        if (osgTexture == NULL)
        {
            osg::ref_ptr<osg::Image> image = osgDB::readImageFile(pTexture->szFilename);
            if (image.valid())
            {
                osgTexture = new osg::Texture;
                osgTexture->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
                osgTexture->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
                osgTexture->setImage(image.get());

                // set up trilinear filtering.
                osgTexture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_LINEAR);
                osgTexture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);

                // Add new texture to registry
                Registry::instance()->addTexture(pTexture->szFilename, osgTexture);
            }
        }

        if (osgTexture)
            pTexturePool->addTexture((int)pTexture->diIndex, osgTexture);
    }

    return NULL;
}

                                 /*osgParent*/
osg::Node* ConvertFromFLT::visitVertexPalette(osg::Group* , VertexPaletteRecord* rec)
{
    _diCurrentOffset = rec->getSize();
    return NULL;
}


                                 /*osgParent*/
osg::Node* ConvertFromFLT::visitVertex(osg::Group* , VertexRecord* rec)
{
    regisiterVertex(_diCurrentOffset, rec);
    _diCurrentOffset += rec->getSize();
    return NULL;
}


                                 /*osgParent*/
osg::Node* ConvertFromFLT::visitNormalVertex(osg::Group* , NormalVertexRecord* rec)
{
    regisiterVertex(_diCurrentOffset, rec);
    _diCurrentOffset += rec->getSize();
    return NULL;
}


                                 /*osgParent*/
osg::Node* ConvertFromFLT::visitTextureVertex(osg::Group* , TextureVertexRecord* rec)
{
    regisiterVertex(_diCurrentOffset, rec);
    _diCurrentOffset += rec->getSize();
    return NULL;
}


                                 /*osgParent*/
osg::Node* ConvertFromFLT::visitNormalTextureVertex(osg::Group* , NormalTextureVertexRecord* rec)
{
    regisiterVertex(_diCurrentOffset, rec);
    _diCurrentOffset += rec->getSize();
    return NULL;
}


osg::Node* ConvertFromFLT::visitGroup(osg::Group* osgParent, GroupRecord* rec)
{
    osg::Group* group = new osg::Group;
    if (group)
    {
        osg::Node* node = visitAncillary(osgParent, rec);
        if (node) osgParent = (osg::Group*)node;

        group->setName(rec->getData()->szIdent);
        osgParent->addChild( group );
        visitPrimaryNode(group, (PrimNodeRecord*)rec);
    }

    return (osg::Node*)group;
}


osg::Node* ConvertFromFLT::visitLOD(osg::Group* osgParent, LodRecord* rec)
{
    SLevelOfDetail* pSLOD = rec->getData();
    osg::LOD* lod = new osg::LOD;
    osg::Group* group = new osg::Group;
    if (lod && group)
    {
        osg::Node* node = visitAncillary(osgParent, rec);
        if (node) osgParent = (osg::Group*)node;

        visitPrimaryNode(group, (PrimNodeRecord*)rec);
        float64x3* pCenter = &pSLOD->Center;
        lod->addChild(group);
        lod->setCenter(osg::Vec3(pCenter->x(), pCenter->y(), pCenter->z()));
        lod->setRange(0, pSLOD->dfSwitchOutDist);
        lod->setRange(1, pSLOD->dfSwitchInDist);
        lod->setName(pSLOD->szIdent);
        osgParent->addChild( lod );
    }

    return (osg::Node*)lod;
}


osg::Node* ConvertFromFLT::visitOldLOD(osg::Group* osgParent, OldLodRecord* rec)
{
    SOldLOD* pSLOD = (SOldLOD*)rec->getData();
    osg::LOD* lod = new osg::LOD;
    osg::Group* group = new osg::Group;
    if (lod && group)
    {
        osg::Node* node = visitAncillary(osgParent, rec);
        if (node) osgParent = (osg::Group*)node;

        visitPrimaryNode(group, (PrimNodeRecord*)rec);
        lod->addChild(group);
        lod->setCenter(osg::Vec3(
            (float)pSLOD->Center[0],
            (float)pSLOD->Center[1],
            (float)pSLOD->Center[2]));
        lod->setRange(0, (float)pSLOD->dwSwitchOutDist);
        lod->setRange(1, (float)pSLOD->dwSwitchInDist);
        lod->setName(pSLOD->szIdent);
        osgParent->addChild( lod );
    }

    return (osg::Node*)lod;
}


// TODO: DOF node implemented as Group.
// Converted DOF to use transform - jtracy@ist.ucf.edu
osg::Node* ConvertFromFLT::visitDOF(osg::Group* osgParent, DofRecord* rec)
{
    osg::Transform* transform = new osg::Transform;

    if (transform)
    {
        osg::Node* node = visitAncillary(osgParent, rec);
        if (node) osgParent = (osg::Group*)node;

        visitPrimaryNode(transform, (PrimNodeRecord*)rec);
        transform->setName(rec->getData()->szIdent);
        
        // note for Judd (and others) shouldn't there be code in here to set up the transform matrix?
        // as a transform with an identity matrix is effectively only a
        // a Group... I will leave for other more familiar with the
        // DofRecord to create the matrix as I don't have any Open Flight
        // documentation.  RO August 2001.
        
        osgParent->addChild( transform );
    }

    return (osg::Node*)transform;
}


osg::Node* ConvertFromFLT::visitSwitch(osg::Group* osgParent, SwitchRecord* rec)
{
    osg::Switch* switc = new osg::Switch;

    if (switc)
    {
        osg::Node* node = visitAncillary(osgParent, rec);
        if (node) osgParent = (osg::Group*)node;

        visitPrimaryNode(switc, (PrimNodeRecord*)rec);
        switc->setName(rec->getData()->szIdent);
        switc->setValue(rec->getData()->dwCurrentMask);
        osgParent->addChild( switc );

        /*
                TODO:
                mask_bit = 1 << (child_num % 32)
                mask_word = mask_words [mask_num * num_words + child_num / 32]
                child_selected = mask_word & mask_bit
        */
    }

    return (osg::Node*)switc;
}


osg::Node* ConvertFromFLT::visitObject(osg::Group* osgParent, ObjectRecord* rec)
{
    osg::Group* group = new osg::Group;

    if (group)
    {
        osg::Node* node = visitAncillary(osgParent, rec);
        if (node) osgParent = (osg::Group*)node;

        unsigned short  wPrevTransparency = _wObjTransparency;
        _wObjTransparency = rec->getData()->wTransparency;
        visitPrimaryNode(group, (PrimNodeRecord*)rec);
        _wObjTransparency = wPrevTransparency;

        group->setName(rec->getData()->szIdent);
        osgParent->addChild( group );
    }

    return (osg::Node*)group;
}


void ConvertFromFLT::visitFace(GeoSetBuilder* pBuilder, FaceRecord* rec)
{
    osg::GeoSet* geoSet = pBuilder->getGeoSet();
    osg::StateSet* stateSet = geoSet->getStateSet();
    SFace *pSFace = (SFace*)rec->getData();
    osg::Vec4 color(1,1,1,1);

    // Cull face & wireframe
    int drawMode = pSFace->swDrawFlag & (BIT0 | BIT1);
    switch(drawMode)
    {
        case FaceRecord::SOLID_BACKFACED:
            // Enable backface culling
            {
                osg::CullFace* cullface = new osg::CullFace;
                cullface->setMode(osg::CullFace::BACK);
                stateSet->setAttributeAndModes(cullface, osg::StateAttribute::ON);
            }
            break;

        case FaceRecord::SOLID_NO_BACKFACE:
            // Disable backface culling
            stateSet->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
            break;

        case FaceRecord::WIREFRAME_NOT_CLOSED:
            geoSet->setPrimType(osg::GeoSet::LINE_STRIP);
            break;

        case FaceRecord::WIREFRAME_CLOSED:
            geoSet->setPrimType(osg::GeoSet::LINE_LOOP);
            break;
    }
    /*
        TODO:

        int directionalLight = pSFace->swDrawFlag & (BIT3 | BIT4);
        switch(directionalLight)
        {
        case FaceRecord::OMNIDIRECTIONAL_LIGHT:
            break;
        case FaceRecord::UNIDIRECTIONAL_LIGHT:
            break;
        case FaceRecord::BIDIRECTIONAL_LIGHT:
            break;
        }
    */


    // Lighting and color binding
    if (_diOpenFlightVersion > 13)
    {
        switch(pSFace->swLightMode)
        {
            case FaceRecord::FACE_COLOR:
                // Use face color, not illuminated
                stateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
                geoSet->setColorBinding( osg::GeoSet::BIND_OVERALL );
                break;

            case FaceRecord::VERTEX_COLOR:
                // Use vertex colors, not illuminated
                stateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
                geoSet->setColorBinding( osg::GeoSet::BIND_PERVERTEX );
                break;

            case FaceRecord::FACE_COLOR_LIGHTING:
                // Use face color and vertex normal
                stateSet->setMode( GL_LIGHTING, osg::StateAttribute::ON );
                geoSet->setColorBinding( osg::GeoSet::BIND_OVERALL );
                geoSet->setNormalBinding(osg::GeoSet::BIND_PERVERTEX);
                break;

            case FaceRecord::VERTEX_COLOR_LIGHTING:
                // Use vertex color and vertex normal
                stateSet->setMode( GL_LIGHTING, osg::StateAttribute::ON );
                geoSet->setColorBinding( osg::GeoSet::BIND_PERVERTEX );
                geoSet->setNormalBinding(osg::GeoSet::BIND_PERVERTEX);
                break;
        }
    }
    else // Version 11, 12 & 13
    {
        geoSet->setColorBinding( osg::GeoSet::BIND_OVERALL );
    }


    // Face Color
    {
        float alpha = 1.0f;
        ColorPool* pColorPool = _pFltFile->getColorPool();

        if (_diOpenFlightVersion > 13)
        {
            if (!(pSFace->dwFlags & FaceRecord::NO_COLOR_BIT))
            {
                float alpha;
                bool bPackedColor =
                        _bHdrRgbMode ||
                        (pSFace->dwFlags & FaceRecord::PACKED_COLOR_BIT) ||
                        (pColorPool == NULL);

                if (bPackedColor)
                    color = pSFace->PrimaryPackedColor.get();
                else
                    color = pColorPool->getColor(pSFace->dwPrimaryColorIndex);

                alpha = 1.0f - (float)pSFace->wTransparency / 65535.0f;
                color[3] = alpha;
            }
        }
        else // Version 11, 12 & 13
        {
            bool bPackedColor = _bHdrRgbMode || (pColorPool == NULL);

           if (bPackedColor)
                color = pSFace->PrimaryPackedColor.get();
            else
                color = pColorPool->getColor(pSFace->wPrimaryNameIndex);

            alpha = 1.0f - (float)pSFace->wTransparency / 65535.0f;
            color[3] = alpha;
        }

        // Transparency
        if (alpha < 1.0f)
        {
            stateSet->setMode(GL_BLEND,osg::StateAttribute::ON);
            stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        }

        if (geoSet->getColorBinding() == osg::GeoSet::BIND_OVERALL)
            geoSet->setColors( new osg::Vec4(color) );
    }

    // Material
    MaterialPool* pMaterialPool = _pFltFile->getMaterialPool();
    if (pMaterialPool)
    {
        MaterialPool::PoolMaterial* pSMaterial = pMaterialPool->getMaterial((int)pSFace->iMaterial);

        if (pSMaterial)
        {
            osg::Material* osgMaterial = new osg::Material;
            osg::Vec4 ambient;
            osg::Vec4 diffuse;
            osg::Vec4 specular;
            osg::Vec4 emissiv;
            float alpha;

            alpha = pSMaterial->sfAlpha * (1.0f - (
                ((float)pSFace->wTransparency / 65535.0f) * ((float)_wObjTransparency / 65535.0f) ));

            ambient[0] = pSMaterial->Ambient[0] * color[0];
            ambient[1] = pSMaterial->Ambient[1] * color[1];
            ambient[2] = pSMaterial->Ambient[2] * color[2];
            ambient[3] = alpha;

            diffuse[0] = pSMaterial->Diffuse[0] * color[0];
            diffuse[1] = pSMaterial->Diffuse[1] * color[1];
            diffuse[2] = pSMaterial->Diffuse[2] * color[2];
            diffuse[3] = alpha;

            specular[0] = pSMaterial->Specular[0];
            specular[1] = pSMaterial->Specular[1];
            specular[2] = pSMaterial->Specular[2];
            specular[3] = alpha;

            emissiv[0] = pSMaterial->Emissive[0];
            emissiv[1] = pSMaterial->Emissive[1];
            emissiv[2] = pSMaterial->Emissive[2];
            emissiv[3] = alpha;

//          osgMaterial->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);
            osgMaterial->setAmbient(osg::Material::FRONT_AND_BACK, ambient);
            osgMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, diffuse);
            osgMaterial->setSpecular(osg::Material::FRONT_AND_BACK, specular);
            osgMaterial->setEmission(osg::Material::FRONT_AND_BACK, emissiv);
            osgMaterial->setAlpha(osg::Material::FRONT_AND_BACK, alpha);
            osgMaterial->setShininess(osg::Material::FRONT_AND_BACK, pSMaterial->sfShininess/128.0f);

            if (alpha < 1.0f)
            {
                stateSet->setMode(GL_BLEND,osg::StateAttribute::ON);
                stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
            }

            stateSet->setAttribute(osgMaterial);
        }
    }

    // Subface
    if (rec->getParent()->isOfType(FACE_OP))
    {
        if (_nSubfaceLevel > 0)
        {
            osg::PolygonOffset* polyoffset = new osg::PolygonOffset;
            if (polyoffset)
            {
                polyoffset->setFactor(-1.0f*_nSubfaceLevel);
                polyoffset->setUnits(-20.0f*_nSubfaceLevel);
                stateSet->setAttributeAndModes(polyoffset,osg::StateAttribute::ON);
            }
        }
    }

    // Texture
    if (pSFace->iTexturePattern != -1)
    {
        TexturePool* pTexturePool = _pFltFile->getTexturePool();
        if (pTexturePool)
        {
            osg::Texture* osgTexture = pTexturePool->getTexture((int)pSFace->iTexturePattern);
            if (osgTexture)
            {
                osg::Image* image = osgTexture->getImage();
                if (image)
                {
                    switch (image->pixelFormat())
                    {
                        case GL_LUMINANCE_ALPHA:
                        case GL_RGBA:
                            stateSet->setMode(GL_BLEND,osg::StateAttribute::ON);
                            stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
                            break;
                    }
                }

                geoSet->setTextureBinding(osg::GeoSet::BIND_PERVERTEX);

                // TODO: Crrect when .attr loader implemented
                osg::TexEnv* osgTexEnv = new osg::TexEnv;
                osgTexEnv->setMode(osg::TexEnv::MODULATE);
                stateSet->setAttribute( osgTexEnv );
                stateSet->setAttributeAndModes( osgTexture, osg::StateAttribute::ON );
            }
        }
    }

    // Visit vertices
    if (_diOpenFlightVersion > 13)
    {
        int i;
        for(i=0; i < rec->getNumChildren(); i++)
        {
            Record* child = rec->getChild(i);
            if (child == NULL) break;

            switch (child->getOpcode())
            {
            case VERTEX_LIST_OP:
                pBuilder->addPrimLen(
                    visitVertexList(pBuilder, (VertexListRecord*)child));
                break;
            }
        }
    }
    else
    {
        int i;
        int vertices=0;
        for(i=0; i < rec->getNumChildren(); i++)
        {
            Record* child = rec->getChild(i);
            if (child == NULL) break;

            switch (child->getOpcode())
            {
            case OLD_VERTEX_OP:
                pBuilder->addVertex(child);
                vertices++;
                break;

            case OLD_VERTEX_COLOR_OP:
                geoSet->setColorBinding( osg::GeoSet::BIND_PERVERTEX );
                pBuilder->addVertex(child);
                vertices++;
                break;

            case OLD_VERTEX_COLOR_NORMAL_OP:
                geoSet->setColorBinding( osg::GeoSet::BIND_PERVERTEX );
                pBuilder->addVertex(child);
                vertices++;
                break;
            }
        }
        pBuilder->addPrimLen(vertices);
    }

    // Add primitives to GeoSet and prepare for next.
    pBuilder->addPrimitive();

    // Look for subfaces
    {
        int n;

        _nSubfaceLevel++;
        for(n=0; n<rec->getNumChildren(); n++)
        {
            Record* child = rec->getChild(n);

            if (child && child->isOfType(FACE_OP))
                visitFace(pBuilder, (FaceRecord*)child);
        }
        _nSubfaceLevel--;
    }
}


int ConvertFromFLT::visitVertexList(GeoSetBuilder* pBuilder, VertexListRecord* rec)
{
    osg::GeoSet* geoSet = pBuilder->getGeoSet();
    int vertices = rec->numberOfVertices();

    // Add vertices to GeoSetBuilder
    for (int i=0; i < vertices; i++)
    {
        Record* vertex = getVertexFromPool(rec->getVertexPoolOffset(i));
        if (vertex)
            pBuilder->addVertex(vertex);
    }
    return vertices;
}


osg::Node* ConvertFromFLT::visitMatrix(osg::Group* osgParent, MatrixRecord* rec)
{
    SMatrix* pSMatrix = (SMatrix*)rec->getData();

    osg::Transform* dcs = new osg::Transform;
    if (dcs)
    {
        osg::Matrix m;
        for(int i=0;i<4;++i)
        {
            for(int j=0;j<4;++j)
            {
                m(i,j) = pSMatrix->sfMat[i][j];
            }
        }
        dcs->setMatrix(m);
        osgParent->addChild(dcs);
        return (osg::Node*)dcs;
    }

    return NULL;
}


osg::Node* ConvertFromFLT::visitExternal(osg::Group* osgParent, ExternalRecord* rec)
{
    // SExternalReference *pSExternal = (SExternalReference*)rec->getData();
    if (osgParent)
    {
        osg::Node* node = visitAncillary(osgParent, rec);
        if (node) osgParent = (osg::Group*)node;

        FltFile* pFile = rec->getExternal();
        if (pFile)
        {
            node = pFile->convert();
            if (node)
            {
                osgParent->addChild(node);
                return node;
            }
        }
    }

    return NULL;
}


void ConvertFromFLT::visitLightPoint(GeoSetBuilder* pBuilder, LightPointRecord* rec)
{
    osg::GeoSet* geoSet = pBuilder->getGeoSet();
    osg::StateSet* stateSet = geoSet->getStateSet();
    SLightPoint *pSLightPoint = (SLightPoint*)rec->getData();

    geoSet->setPrimType(osg::GeoSet::POINTS);
    geoSet->setColorBinding(osg::GeoSet::BIND_PERVERTEX);

    osg::Point* point = new osg::Point;
    if (point)
    {
        point->setSize( pSLightPoint->sfSize );
        stateSet->setAttributeAndModes( point, osg::StateAttribute::ON );
//      point->setFadeThresholdSize(const float fadeThresholdSize);
//      point->setDistanceAttenuation(const Vec3& distanceAttenuation);
//      point->setStateSetModes(*stateSet,const GLModeValue value); // GL_POINT_SMOOTH

    }

    // Visit vertices
    for(int i=0; i < rec->getNumChildren(); i++)
    {
        Record* child = rec->getChild(i);

        if (child)
        {
            int op = child->getOpcode();
            switch (op)
            {
                case VERTEX_LIST_OP:
                    {
                    int vertices = visitVertexList(pBuilder, (VertexListRecord*)child);
                    for (int v=0; v<vertices; v++)
                        pBuilder->addPrimLen(1);
                    }
                    break;

                case OLD_VERTEX_OP:
                case OLD_VERTEX_COLOR_OP:
                case OLD_VERTEX_COLOR_NORMAL_OP:
                    pBuilder->addVertex(child);
                    pBuilder->addPrimLen(1);
                    break;
            }
        }
    }

    pBuilder->addPrimitive();
}
