// flt2osg.cpp

#include <string.h>
#include "osg/GL"

#include <osg/Scene>
#include <osg/Group>
#include <osg/DCS>
#include <osg/LOD>
#include <osg/DCS>
#include <osg/Switch>
#include <osg/Sequence>
#include <osg/Geode>
#include <osg/GeoSet>
#include <osg/GeoState>
#include <osg/Material>
#include <osg/PolygonOffset>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Billboard>
#include <osg/Texture>
#include <osg/Image>
#include <osg/FileNameUtils>
#include <osg/Registry>
#include <osg/Notify>
#include <osg/FileNameUtils>

#include "opcodes.h"
#include "flt.h"
#include "flt2osg.h"
#include "FltFile.h"
#include "Record.h"
#include "HeaderRecord.h"
#include "ColorPaletteRecord.h"
#include "MaterialPaletteRecord.h"
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

    osg::Group* group = new osg::Group;

    _diOpenFlightVersion = pSHeader->diFormatRevLev;

    osg::notify(osg::INFO) << "Version " << _diOpenFlightVersion << endl;

    if (group)
    {
        visitAncillary(osgParent, rec);

        visitPrimaryNode(group, (PrimNodeRecord*)rec);
        group->setName(pSHeader->szIdent);
        if (osgParent) osgParent->addChild(group);
    }

    return (osg::Node*)group;
}


osg::Node* ConvertFromFLT::visitColorPalette(osg::Group* osgParent, ColorPaletteRecord* rec)
{
    SColorPalette* pCol = (SColorPalette*)rec->getData();

    ColorPool* pColorPool = _pFltFile->getColorPool();
    if (pCol && pColorPool)
    {
        for (int n=0; n<1024; n++)
            pColorPool->regisiterColor(n, pCol->Colors[n].get());
    }
    return NULL;
}


osg::Node* ConvertFromFLT::visitMaterialPalette(osg::Group* osgParent, MaterialPaletteRecord* rec)
{
    SMaterial* pSMaterial = (SMaterial*)rec->getData();

    MaterialPool* pMaterialPool = _pFltFile->getMaterialPool();
    if (pSMaterial && pMaterialPool /* && (pMaterial->diFlags & BIT0) */)
        pMaterialPool->regisiterMaterial((int)pSMaterial->diIndex, pSMaterial);
    return NULL;
}


osg::Node* ConvertFromFLT::visitTexturePalette(osg::Group* osgParent, TexturePaletteRecord* rec)
{
    STexturePalette* pTexture = (STexturePalette*)rec->getData();

    if (pTexture)
    {
        osg::notify(osg::INFO) << "Texture" << pTexture->diIndex << " = " << pTexture->szFilename << endl;

        osg::ref_ptr<osg::Image> image = osg::Registry::instance()->readImage(pTexture->szFilename);
        if (image.valid())
        {
            osg::Texture *osgTexture = new osg::Texture;
            TexturePool* pTexturePool = _pFltFile->getTexturePool();
            if (osgTexture && pTexturePool)
            {
                osgTexture->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
                osgTexture->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
                osgTexture->setImage(image.get());
                pTexturePool->regisiterTexture((int)pTexture->diIndex, osgTexture);
            }
        }
    }
    return NULL;
}


osg::Node* ConvertFromFLT::visitVertexPalette(osg::Group* osgParent, VertexPaletteRecord* rec)
{
    _diCurrentOffset = rec->getSize();
    return NULL;
}


osg::Node* ConvertFromFLT::visitVertex(osg::Group* osgParent, VertexRecord* rec)
{
    regisiterVertex(_diCurrentOffset, rec);
    _diCurrentOffset += rec->getSize();
    return NULL;
}


osg::Node* ConvertFromFLT::visitNormalVertex(osg::Group* osgParent, NormalVertexRecord* rec)
{
    regisiterVertex(_diCurrentOffset, rec);
    _diCurrentOffset += rec->getSize();
    return NULL;
}


osg::Node* ConvertFromFLT::visitTextureVertex(osg::Group* osgParent, TextureVertexRecord* rec)
{
    regisiterVertex(_diCurrentOffset, rec);
    _diCurrentOffset += rec->getSize();
    return NULL;
}


osg::Node* ConvertFromFLT::visitNormalTextureVertex(osg::Group* osgParent, NormalTextureVertexRecord* rec)
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

        visitPrimaryNode(group, (PrimNodeRecord*)rec);
        group->setName(rec->getData()->szIdent);
        osgParent->addChild( group );
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
    osg::Group* group = new osg::Group;
    if (group)
    {
        osg::Node* node = visitAncillary(osgParent, rec);
        if (node) osgParent = (osg::Group*)node;

        visitPrimaryNode(group, (PrimNodeRecord*)rec);
        group->setName(rec->getData()->szIdent);
        osgParent->addChild( group );
    }

    return (osg::Node*)group;
}


// TODO: DOF node implemented as Group.
osg::Node* ConvertFromFLT::visitDOF(osg::Group* osgParent, DofRecord* rec)
{
    osg::Group* group = new osg::Group;

    if (group)
    {
        osg::Node* node = visitAncillary(osgParent, rec);
        if (node) osgParent = (osg::Group*)node;

        visitPrimaryNode(group, (PrimNodeRecord*)rec);
        group->setName(rec->getData()->szIdent);
        osgParent->addChild( group );
    }

    return (osg::Node*)group;
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
        switc->setVal(rec->getData()->dwCurrentMask);
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
    int drawMode  = pSFace->swDrawFlag & (BIT0 | BIT1);
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

    // Color
    switch(pSFace->swLightMode)
    {
    case FaceRecord::FACE_COLOR:
    case FaceRecord::FACE_COLOR_LIGHTING:
    case FaceRecord::VERTEX_COLOR:
    case FaceRecord::VERTEX_COLOR_LIGHTING:
        if (!(pSFace->diFlags & BIT1))              // face color bit
        {
            float alpha;

            if (pSFace->diFlags & BIT3)             // Packed color bit
                color = pSFace->PrimaryPackedColor.get();
            else
            {
                ColorPool* pColorPool = _pFltFile->getColorPool();
                if (pColorPool)
                    color = pColorPool->getColor(pSFace->dwPrimaryColorIndex);
            }

            alpha = 1.0f - (float)pSFace->wTransparency / 65535.0f;
            color[3] = alpha;

            if (alpha < 1.0f)
                pBuilder->setTransparency(true);

            pBuilder->setColorBinding(osg::GeoSet::BIND_OVERALL);
            pBuilder->setColor(color);
        }
        break;
    }

    // Material
    MaterialPool* pMaterialPool = _pFltFile->getMaterialPool();
    if (pMaterialPool)
    {
        SMaterial* pSMaterial = pMaterialPool->getMaterial((int)pSFace->iMaterial);

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

            osgMaterial->setAmbient(osg::Material::FACE_FRONT_AND_BACK, ambient);
            osgMaterial->setDiffuse(osg::Material::FACE_FRONT_AND_BACK, diffuse);
            osgMaterial->setSpecular(osg::Material::FACE_FRONT_AND_BACK, specular);
            osgMaterial->setEmission(osg::Material::FACE_FRONT_AND_BACK, emissiv);
            osgMaterial->setShininess(osg::Material::FACE_FRONT_AND_BACK, pSMaterial->sfShininess/128.0f);
            osgMaterial->setColorMode(osg::Material::OFF);

            if (alpha < 1.0f)
                pBuilder->setTransparency(true);

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
    for(int i=0; i < rec->getNumChildren(); i++)
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
            case OLD_VERTEX_COLOR_OP:
            case OLD_VERTEX_COLOR_NORMAL_OP:
                pBuilder->addVertex(child);
                break;

            }
        }
    }

    // Add primitive to GeoSet and prepare for next.
    pBuilder->addPrimitive();

    // Look for sufbfaces
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

    osg::DCS* dcs = new osg::DCS;
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
    SExternalReference *pSExternal = (SExternalReference*)rec->getData();

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
    SLightPoint *pSLightPoint = (SLightPoint*)rec->getData();

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



