// flt2osg.cpp

#include <string.h>
#include "osg/GL"

#include <osg/Group>
#include <osg/LOD>
#include <osg/Transform>
#include <osg/Switch>
#include <osg/Geode>
#include <osg/GeoSet>
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
    GeoSetBuilder   geoSetBuilder(_pFltFile);

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
    ColorPool* pColorPool = _pFltFile->getColorPool();
    if (pColorPool == NULL)
    {
        _pFltFile->useLocalColorPool();
        pColorPool = _pFltFile->getColorPool();
    }

    if (rec->getSize() > sizeof(SOldColorPalette))
    {
        SColorPalette* pCol = (SColorPalette*)rec->getData();
        for (int i=0; i < 1024; i++)
            pColorPool->addColor(i, pCol->Colors[i].get());
    }
    else    // version 11, 12 & 13
    {
        SOldColorPalette* pSColor = (SOldColorPalette*)rec->getData();
        unsigned int i;
        for (i=0; i < sizeof(pSColor->Colors)/sizeof(pSColor->Colors[0]); i++)
        {
            pColorPool->addColor(i, pSColor->Colors[i].get());
        }

        for (i=0; i < sizeof(pSColor->FixedColors)/sizeof(pSColor->FixedColors[0]); i++)
        {
            pColorPool->addColor(i+4096, pSColor->FixedColors[i].get());
        }
    }

    return NULL;
}


                                 /*osgParent*/
osg::Node* ConvertFromFLT::visitMaterialPalette(osg::Group* , MaterialPaletteRecord* rec)
{
    SMaterial* pSMaterial = (SMaterial*)rec->getData();
    MaterialPool* pMaterialPool = _pFltFile->getMaterialPool();

    if (pSMaterial && pMaterialPool )
    {
        PoolMaterial* pPoolMat = new PoolMaterial;

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
    SOldMaterial* pSMaterial = (SOldMaterial*)rec->getData();
    MaterialPool* pMaterialPool = _pFltFile->getMaterialPool();

    if (pSMaterial && pMaterialPool )
    {
        for (int i=0; i < 64; i++)
        {
            PoolMaterial* pPoolMat = new PoolMaterial;

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
    STexturePalette* pTexture = (STexturePalette*)rec->getData();

    if (pTexture)
    {
        osg::ref_ptr<osg::Image> image = osgDB::readImageFile(pTexture->szFilename);
        if (image.valid())
        {
            osg::Texture *osgTexture = new osg::Texture;
            TexturePool* pTexturePool = _pFltFile->getTexturePool();
            if (osgTexture && pTexturePool)
            {
                osgTexture->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
                osgTexture->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
                osgTexture->setImage(image.get());
                pTexturePool->addTexture((int)pTexture->diIndex, osgTexture);
            }
        }
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
    SFace *pSFace = (SFace*)rec->getData();
    osg::Vec4 color(1,1,1,1);

    // Cull face & wireframe
    int drawMode = pSFace->swDrawFlag & (BIT0 | BIT1);
    switch(drawMode)
    {
        case FaceRecord::SOLID_BACKFACED:
            pBuilder->setCullface(true);
            break;
        case FaceRecord::SOLID_NO_BACKFACE:
            pBuilder->setCullface(false);
            break;
        case FaceRecord::WIREFRAME_NOT_CLOSED:
            pBuilder->setPrimType(osg::GeoSet::LINE_STRIP);
            break;
        case FaceRecord::WIREFRAME_CLOSED:
            pBuilder->setPrimType(osg::GeoSet::LINE_LOOP);
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

    // Lighting
    if (_diOpenFlightVersion > 13)
    {
        switch(pSFace->swLightMode)
        {
            case FaceRecord::FACE_COLOR:
            case FaceRecord::VERTEX_COLOR:
                pBuilder->setLighting(false);
                break;
            case FaceRecord::FACE_COLOR_LIGHTING:
            case FaceRecord::VERTEX_COLOR_LIGHTING:
                pBuilder->setLighting(true);
                break;
        }
    }

    // Color
    {
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

                if (alpha < 1.0f)
                    pBuilder->setTransparency(true);

                pBuilder->setFaceColor(color);

                switch(pSFace->swLightMode)
                {
                    case FaceRecord::FACE_COLOR:
                    case FaceRecord::FACE_COLOR_LIGHTING:
                        pBuilder->setColorBinding(osg::GeoSet::BIND_OVERALL);
                        break;

                    case FaceRecord::VERTEX_COLOR:
                    case FaceRecord::VERTEX_COLOR_LIGHTING:
                        pBuilder->setColorBinding(osg::GeoSet::BIND_PERVERTEX);
                        break;
                }
            }
        }
        else // Version 11, 12 & 13
        {
            float alpha;
            bool bPackedColor = _bHdrRgbMode || (pColorPool == NULL);

           if (bPackedColor)
                color = pSFace->PrimaryPackedColor.get();
            else
                color = pColorPool->getColor(pSFace->wPrimaryNameIndex);

            alpha = 1.0f - (float)pSFace->wTransparency / 65535.0f;
            color[3] = alpha;

            if (alpha < 1.0f)
                pBuilder->setTransparency(true);

            pBuilder->setColorBinding(osg::GeoSet::BIND_OVERALL);
            pBuilder->setFaceColor(color);
        }
    }

    // Material
    MaterialPool* pMaterialPool = _pFltFile->getMaterialPool();
    if (pMaterialPool)
    {
        PoolMaterial* pSMaterial = pMaterialPool->getMaterial((int)pSFace->iMaterial);

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

            osgMaterial->setAmbient(osg::Material::FRONT_AND_BACK, ambient);
            osgMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, diffuse);
            osgMaterial->setSpecular(osg::Material::FRONT_AND_BACK, specular);
            osgMaterial->setEmission(osg::Material::FRONT_AND_BACK, emissiv);
            osgMaterial->setAlpha(osg::Material::FRONT_AND_BACK, alpha);
            osgMaterial->setShininess(osg::Material::FRONT_AND_BACK, pSMaterial->sfShininess/128.0f);

            if (alpha < 1.0f)
                pBuilder->setTransparency(true);

            switch (pSFace->swLightMode)
            {
            case FaceRecord::VERTEX_COLOR:
            case FaceRecord::VERTEX_COLOR_LIGHTING:
                osgMaterial->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);
                break;

            default:
                osgMaterial->setColorMode(osg::Material::OFF);
            }

            pBuilder->setMaterial(osgMaterial);
        }
    }

    // Subface
    if (rec->getParent()->isOfType(FACE_OP))
    {
        pBuilder->setSubface(_nSubfaceLevel);
    }

    // Texture
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
                        pBuilder->setTransparency(true);
                        break;
                }
            }
            pBuilder->setTexture(osgTexture);
        }
    }

    // Visit vertices
    int i;
    for(i=0; i < rec->getNumChildren(); i++)
    {
        Record* child = rec->getChild(i);

        if (child)
        {
            int op = child->getOpcode();
            switch (op)
            {
                case VERTEX_LIST_OP:
                    visitVertexList(pBuilder, (VertexListRecord*)child);
                    break;

                case OLD_VERTEX_OP:
                    pBuilder->addVertex(child);
                    break;

                case OLD_VERTEX_COLOR_OP:
                    pBuilder->setColorBinding(osg::GeoSet::BIND_PERVERTEX);
                    pBuilder->addVertex(child);
                    break;

                case OLD_VERTEX_COLOR_NORMAL_OP:
                    pBuilder->setColorBinding(osg::GeoSet::BIND_PERVERTEX);
                    pBuilder->addVertex(child);
                    pBuilder->setLighting(true);
                    break;
            }
        }
    }

    // Add primitive to GeoSet and prepare for next.
    pBuilder->addPrimitive();

    // Look for subfaces
    _nSubfaceLevel++;
    for(i=0; i < rec->getNumChildren(); i++)
    {
        Record* child = rec->getChild(i);

        if (child && child->isOfType(FACE_OP))
            visitFace(pBuilder, (FaceRecord*)child);
    }
    _nSubfaceLevel--;
}


void ConvertFromFLT::visitVertexList(GeoSetBuilder* pBuilder, VertexListRecord* rec)
{
    int vertices = rec->numberOfVertices();

    // Add vertices to GeoSetBuilder
    if (pBuilder->getPrimType() == osg::GeoSet::POINTS)
    {
        for (int i=0; i < vertices; i++)
        {
            Record* vertex = getVertexFromPool(rec->getVertexPoolOffset(i));
            if (vertex)
            {
                pBuilder->setPrimType(osg::GeoSet::POINTS);
                pBuilder->setColorBinding(osg::GeoSet::BIND_PERVERTEX);
                pBuilder->addVertex(vertex);
                pBuilder->addPrimitive();
            }
        }
    }
    else
    {
        for (int i=0; i < vertices; i++)
        {
            Record* vertex = getVertexFromPool(rec->getVertexPoolOffset(i));
            if (vertex)
                pBuilder->addVertex(vertex);
        }
    }
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
                m._mat[i][j] = pSMatrix->sfMat[i][j];
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
    //    SExternalReference *pSExternal = (SExternalReference*)rec->getData();

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
    //    SLightPoint *pSLightPoint = (SLightPoint*)rec->getData();

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
                    pBuilder->setPrimType(osg::GeoSet::POINTS);
                    visitVertexList(pBuilder, (VertexListRecord*)child);
                    break;

                case OLD_VERTEX_OP:
                    pBuilder->addVertex(child);
                    pBuilder->addPrimitive();
                    break;

                case OLD_VERTEX_COLOR_OP:
                case OLD_VERTEX_COLOR_NORMAL_OP:
                    pBuilder->setColorBinding(osg::GeoSet::BIND_PERVERTEX);
                    pBuilder->addVertex(child);
                    pBuilder->addPrimitive();
                    break;
            }
        }
    }
}
