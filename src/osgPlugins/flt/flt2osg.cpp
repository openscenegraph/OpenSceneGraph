#include <stdio.h>
#include <string.h>
#include <osg/GL>

#include <osg/Group>
#include <osg/LOD>
#include <osg/Transform>
#include <osg/Switch>
#include <osg/Geode>
#include <osg/GeoSet>
#include <osg/StateSet>
#include <osg/CullFace>
#include <osg/TexEnv>
#include <osg/TexGen>
#include <osg/AlphaFunc>
#include <osg/Transparency>
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
#include "OldVertexRecords.h"
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
#include "InstanceRecords.h"



using namespace flt;


ConvertFromFLT::ConvertFromFLT() :
    _faceColor(1,1,1,1)
{
    _diOpenFlightVersion = 0;
    _diCurrentOffset = 0;
    _wObjTransparency = 0;
    _nSubfaceLevel = 0;
    _unitScale = 1.0;
    _bHdrRgbMode = false;
}


ConvertFromFLT::~ConvertFromFLT()
{
}


osg::Group* ConvertFromFLT::convert(HeaderRecord* rec)
{
    if (rec==NULL) return NULL;
    return visitHeader(rec);
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


osg::Group* ConvertFromFLT::visitInstanceDefinition(osg::Group& osgParent,InstanceDefinitionRecord* rec)
{
    osg::Group* group = new osg::Group;
    InstancePool* pInstancePool = rec->getFltFile()->getInstancePool();

    visitAncillary(osgParent, *group, rec);

    pInstancePool->addInstance((int)rec->getData()->iInstDefNumber,group);
    visitPrimaryNode(*group, (PrimNodeRecord*)rec);

    return group;
}

osg::Group* ConvertFromFLT::visitInstanceReference(osg::Group& osgParent,InstanceReferenceRecord* rec)
{
    osg::Group* group;
    InstancePool* pInstancePool = rec->getFltFile()->getInstancePool();

    group = pInstancePool->getInstance((int)rec->getData()->iInstDefNumber);
    if (group)
        osgParent.addChild( group );
    else
        osg::notify(osg::INFO) << "Warning: cannot find the instance definition in flt file."<<std::endl;
    return group;
}


osg::Group* ConvertFromFLT::visitAncillary(osg::Group& osgParent, osg::Group& osgPrimary, PrimNodeRecord* rec)
{
    osg::Group* parent = &osgParent;
    // Visit ancillary records
    for(int i=0; i < rec->getNumChildren(); i++)
    {
        Record* child = rec->getChild(i);
        if (!child->isAncillaryRecord())
            break;

        switch (child->getOpcode())
        {
        case LONG_ID_OP:
            visitLongID(osgPrimary, (LongIDRecord*)child);
            break;

        case MATRIX_OP:
            // Note: Ancillary record creates osg node
            parent = visitMatrix(*parent, osgPrimary, (MatrixRecord*)child);
            break;

        case COMMENT_OP:
//              visitComment(osgPrimary, (CommentRecord*)child);
            break;

        case COLOR_PALETTE_OP:
            visitColorPalette(osgPrimary, (ColorPaletteRecord*)child);
            break;

        case MATERIAL_PALETTE_OP:
            visitMaterialPalette(osgPrimary, (MaterialPaletteRecord*)child);
            break;

        case OLD_MATERIAL_PALETTE_OP:
            visitOldMaterialPalette(osgPrimary, (OldMaterialPaletteRecord*)child);
            break;

        case TEXTURE_PALETTE_OP:
            visitTexturePalette(osgPrimary, (TexturePaletteRecord*)child);
            break;

        case VERTEX_PALETTE_OP:
            visitVertexPalette(osgPrimary, (VertexPaletteRecord*)child);
            break;

        case VERTEX_C_OP:
            visitVertex(osgPrimary, (VertexRecord*)child);
            break;

        case VERTEX_CN_OP:
            visitNormalVertex(osgPrimary, (NormalVertexRecord*)child);
            break;

        case VERTEX_CNT_OP:
            visitNormalTextureVertex(osgPrimary, (NormalTextureVertexRecord*)child);
            break;

        case VERTEX_CT_OP:
            visitTextureVertex(osgPrimary, (TextureVertexRecord*)child);
            break;
        }
    }
    return parent;
}


osg::Group* ConvertFromFLT::visitPrimaryNode(osg::Group& osgParent, PrimNodeRecord* rec)
{
    osg::Group* osgPrim = NULL;
    GeoSetBuilder   geoSetBuilder;
    GeoSetBuilder   billboardBuilder;

    // Visit
    for(int i=0; i < rec->getNumChildren(); i++)
    {
        Record* child = rec->getChild(i);

        if (child && child->isPrimaryNode())
        {
            switch (child->getOpcode())
            {
            case FACE_OP:
            {
                FaceRecord* fr = (FaceRecord*)child;
                if( fr->getData()->swTemplateTrans == 2)  //Axis type rotate
                    visitFace(&billboardBuilder, fr);
                else
                    visitFace(&geoSetBuilder, fr);
            }
            break;
            case LIGHT_POINT_OP:
                visitLightPoint(&geoSetBuilder, (LightPointRecord*)child);
                break;
            case GROUP_OP:
                osgPrim = visitGroup(osgParent, (GroupRecord*)child);
                break;
            case LOD_OP:
                osgPrim = visitLOD(osgParent, (LodRecord*)child);
                break;
            case OLD_LOD_OP:
                osgPrim = visitOldLOD(osgParent, (OldLodRecord*)child);
                break;
            case DOF_OP:
                osgPrim = visitDOF(osgParent, (DofRecord*)child);
                break;
            case SWITCH_OP:
                osgPrim = visitSwitch(osgParent, (SwitchRecord*)child);
                break;
            case OBJECT_OP:
                osgPrim = visitObject(osgParent, (ObjectRecord*)child);
                break;
            case INSTANCE_REFERENCE_OP:
                osgPrim = visitInstanceReference(osgParent, (InstanceReferenceRecord*)child);
                break;
            case INSTANCE_DEFINITION_OP:
                osgPrim = visitInstanceDefinition(osgParent, (InstanceDefinitionRecord*)child);
                break;
            case EXTERNAL_REFERENCE_OP:
                osgPrim = visitExternal(osgParent, (ExternalRecord*)child);
                break;
            }
        }
    }

    if( !geoSetBuilder.empty() )
    {
        osg::Geode* geode = new osg::Geode;
        geoSetBuilder.createOsgGeoSets(geode );
    
        if (geode->getNumDrawables() > 0)
            osgParent.addChild( geode );
    }

    if( !billboardBuilder.empty() )
    {
        osg::Billboard* billboard = new osg::Billboard;
        billboardBuilder.createOsgGeoSets(billboard );
        
        if (billboard->getNumDrawables() > 0)
            osgParent.addChild( billboard );
    }

    return osgPrim;
}


void ConvertFromFLT::visitLongID(osg::Group& osgParent, LongIDRecord* rec)
{
    SLongID *pSLongID = (SLongID*)rec->getData();

    // these cout's are here for double checking whether handlng of the longID
    // string is being managed corectly. 
    // std::cout << "ConvertFromFLT::visitLongID '"<<std::string(pSLongID->szIdent,pSLongID->RecHeader.length()-4)<<"'"<<std::endl;
    // std::cout << "ConvertFromFLT::visitLongID cstyle string '"<<pSLongID->szIdent<<"'"<<std::endl;

    osgParent.setName(std::string(pSLongID->szIdent,rec->getBodyLength()));
}


osg::Group* ConvertFromFLT::visitHeader(HeaderRecord* rec)
{
    SHeader *pSHeader = (SHeader*)rec->getData();

    // Version
    _diOpenFlightVersion = pSHeader->diFormatRevLev;
    osg::notify(osg::INFO) << "Version " << _diOpenFlightVersion << std::endl;

    // Unit scale
    switch (pSHeader->swVertexCoordUnit)
    {
    case HeaderRecord::METERS:
        _unitScale = 1.0;
        break;
    case HeaderRecord::KILOMETERS:
        _unitScale = 1000.0;
        break;
    case HeaderRecord::FEET:
        _unitScale = 0.3048;
        break;
    case HeaderRecord::INCHES:
        _unitScale = 0.02540;
        break;
    case HeaderRecord::NAUTICAL_MILES:
        _unitScale = 1852.0;
        break;
    default:
        _unitScale = 1.0;
    }

    // Flight v.11 & v.12 use integer coordinates
    if (rec->getFlightVersion() < 13)
    {
        if (pSHeader->iMultDivUnit >= 0)
            _unitScale *= (double)pSHeader->iMultDivUnit;
        else
            _unitScale /= (double)(-pSHeader->iMultDivUnit);
    }

    _bHdrRgbMode = (pSHeader->dwFlags & 0x40000000) ? true : false;    // RGB space (=packed color)

    // Create root group node
    osg::Group* group = new osg::Group;
    group->setName(pSHeader->szIdent);
    visitAncillary(*group, *group, rec);
    visitPrimaryNode(*group, rec);

    return group;
}


                                 /*osgParent*/
void ConvertFromFLT::visitColorPalette(osg::Group& , ColorPaletteRecord* rec)
{
    if (!rec->getFltFile()->useInternalColorPalette()) return;

    ColorPool* pColorPool = rec->getFltFile()->getColorPool();
    int flightVersion = rec->getFlightVersion();

    if (flightVersion > 13)
    {
        SColorPalette* pCol = (SColorPalette*)rec->getData();
        int colors = (flightVersion >= 1500) ? 1024 : 512;

        for (int i=0; i < colors; i++)
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
}

                                 /*osgParent*/
void ConvertFromFLT::visitMaterialPalette(osg::Group&, MaterialPaletteRecord* rec)
{
    if (!rec->getFltFile()->useInternalMaterialPalette()) return;

    SMaterial* pSMaterial = (SMaterial*)rec->getData();
    MaterialPool* pMaterialPool = rec->getFltFile()->getMaterialPool();
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
}

                                 /*osgParent*/
void ConvertFromFLT::visitOldMaterialPalette(osg::Group& , OldMaterialPaletteRecord* rec)
{
    if (!rec->getFltFile()->useInternalMaterialPalette()) return;

    SOldMaterial* pSMaterial = (SOldMaterial*)rec->getData();
    MaterialPool* pMaterialPool = rec->getFltFile()->getMaterialPool();

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
}

                                 /*osgParent*/
void ConvertFromFLT::visitTexturePalette(osg::Group& , TexturePaletteRecord* rec)
{
    int nIndex;
    char* pFilename;

    if (!rec->getFltFile()->useInternalTexturePalette()) return;

    if (rec->getFlightVersion() > 13)
    {
        STexturePalette* pTexture = (STexturePalette*)rec->getData();
        pFilename = pTexture->szFilename;
        nIndex = pTexture->diIndex;
    }
    else // version 11, 12 & 13
    {
        SOldTexturePalette* pOldTexture = (SOldTexturePalette*)rec->getData();
        pFilename = pOldTexture->szFilename;
        nIndex = pOldTexture->diIndex;
    }

    TexturePool* pTexturePool = rec->getFltFile()->getTexturePool();
    if (pTexturePool == NULL) return;

    // Get StateSet containing texture from registry pool.
    osg::StateSet *osgStateSet = Registry::instance()->getTexture(pFilename);

    if (osgStateSet)
    {
        // Add texture to local pool to be able to get by index.
        pTexturePool->addTexture(nIndex, osgStateSet);
        return;     // Texture already loaded
    }

    // Read texture and attribute file
    osg::ref_ptr<osg::Image> image = osgDB::readImageFile(pFilename);
    if (image.valid())
    {
        std::string attrName(pFilename);
        attrName += ".attr";

        // Read attribute file
        char options[256];
        sprintf(options,"FLT_VER %d",rec->getFlightVersion());

        osgDB::Registry::instance()->setOptions(new osgDB::ReaderWriter::Options(options));
        osg::StateSet* osgStateSet =
            dynamic_cast<osg::StateSet*>(osgDB::readObjectFile(attrName));
        osgDB::Registry::instance()->setOptions(NULL);      // Delete options

        // if not found create default StateSet
        if (osgStateSet == NULL)
        {
            osgStateSet = new osg::StateSet;

            osg::Texture* osgTexture = new osg::Texture;
            osgTexture->setWrap(osg::Texture::WRAP_S,osg::Texture::REPEAT);
            osgTexture->setWrap(osg::Texture::WRAP_T,osg::Texture::REPEAT);
            osgStateSet->setAttributeAndModes(osgTexture,osg::StateAttribute::ON);

            osg::TexEnv* osgTexEnv = new osg::TexEnv;
            osgTexEnv->setMode(osg::TexEnv::MODULATE);
            osgStateSet->setAttribute( osgTexEnv );
        }

        osg::Texture *osgTexture = dynamic_cast<osg::Texture*>(osgStateSet->getAttribute( osg::StateAttribute::TEXTURE));
        if (osgTexture == NULL)
        {
            osgTexture = new osg::Texture;
            osgStateSet->setAttributeAndModes(osgTexture,osg::StateAttribute::ON);
        }

        osgTexture->setImage(image.get());

        // Add new texture to registry pool
        Registry::instance()->addTexture(pFilename, osgStateSet);

        // Also add to local pool to be able to get texture by index.
        pTexturePool->addTexture(nIndex, osgStateSet);
    }
}

                                 /*osgParent*/
void ConvertFromFLT::visitVertexPalette(osg::Group& , VertexPaletteRecord* rec)
{
    _diCurrentOffset = rec->getSize();
}

                                 /*osgParent*/
void ConvertFromFLT::visitVertex(osg::Group& , VertexRecord* rec)
{
    regisiterVertex(_diCurrentOffset, rec);
    _diCurrentOffset += rec->getSize();
}

                                 /*osgParent*/
void ConvertFromFLT::visitNormalVertex(osg::Group& , NormalVertexRecord* rec)
{
    regisiterVertex(_diCurrentOffset, rec);
    _diCurrentOffset += rec->getSize();
}

                                 /*osgParent*/
void ConvertFromFLT::visitTextureVertex(osg::Group& , TextureVertexRecord* rec)
{
    regisiterVertex(_diCurrentOffset, rec);
    _diCurrentOffset += rec->getSize();
}

                                 /*osgParent*/
void ConvertFromFLT::visitNormalTextureVertex(osg::Group& , NormalTextureVertexRecord* rec)
{
    regisiterVertex(_diCurrentOffset, rec);
    _diCurrentOffset += rec->getSize();
}


osg::Group* ConvertFromFLT::visitGroup(osg::Group& osgParent, GroupRecord* rec)
{
    osg::Group* group = new osg::Group;

    group->setName(rec->getData()->szIdent);
    visitAncillary(osgParent, *group, rec)->addChild( group );
    visitPrimaryNode(*group, rec);
    return group;
}


osg::Group* ConvertFromFLT::visitLOD(osg::Group& osgParent, LodRecord* rec)
{
    SLevelOfDetail* pSLOD = rec->getData();
    osg::LOD* lod = new osg::LOD;

    float64x3* pCenter = &pSLOD->Center;
    lod->setCenter(osg::Vec3(pCenter->x(), pCenter->y(), pCenter->z())*_unitScale);
    lod->setRange(0, pSLOD->dfSwitchOutDist*_unitScale);
    lod->setRange(1, pSLOD->dfSwitchInDist*_unitScale);
    lod->setName(pSLOD->szIdent);
    visitAncillary(osgParent, *lod, rec)->addChild( lod );

    osg::Group* group = new osg::Group;
    lod->addChild(group);
    visitPrimaryNode(*group, rec);

    return lod;
}


osg::Group* ConvertFromFLT::visitOldLOD(osg::Group& osgParent, OldLodRecord* rec)
{
    SOldLOD* pSLOD = (SOldLOD*)rec->getData();
    osg::LOD* lod = new osg::LOD;

    lod->setCenter(osg::Vec3(
        (float)pSLOD->Center[0],
        (float)pSLOD->Center[1],
        (float)pSLOD->Center[2])*_unitScale);
    lod->setRange(0, ((float)pSLOD->dwSwitchOutDist)*_unitScale);
    lod->setRange(1, ((float)pSLOD->dwSwitchInDist)*_unitScale);
    lod->setName(pSLOD->szIdent);
    visitAncillary(osgParent, *lod, rec)->addChild( lod );

    osg::Group* group = new osg::Group;
    lod->addChild(group);
    visitPrimaryNode(*group, rec);

    return lod;
}


// TODO: DOF node implemented as Group.
// Converted DOF to use transform - jtracy@ist.ucf.edu
osg::Group* ConvertFromFLT::visitDOF(osg::Group& osgParent, DofRecord* rec)
{
    osg::Transform* transform = new osg::Transform;

    transform->setName(rec->getData()->szIdent);
    transform->setDataVariance(osg::Object::DYNAMIC);
    visitAncillary(osgParent, *transform, rec)->addChild( transform );
    visitPrimaryNode(*transform, (PrimNodeRecord*)rec);
        
    // note for Judd (and others) shouldn't there be code in here to set up the transform matrix?
    // as a transform with an identity matrix is effectively only a
    // a Group... I will leave for other more familiar with the
    // DofRecord to create the matrix as I don't have any Open Flight
    // documentation.  RO August 2001.
        

    return transform;
}


osg::Group* ConvertFromFLT::visitSwitch(osg::Group& osgParent, SwitchRecord* rec)
{
    osg::Switch* switc = new osg::Switch;

    switc->setName(rec->getData()->szIdent);
    switc->setValue(rec->getData()->dwCurrentMask);
    visitAncillary(osgParent, *switc, rec)->addChild( switc );
    visitPrimaryNode(*switc, (PrimNodeRecord*)rec);

    /*
            TODO:
            mask_bit = 1 << (child_num % 32)
            mask_word = mask_words [mask_num * num_words + child_num / 32]
            child_selected = mask_word & mask_bit
    */

    return switc;
}


osg::Group* ConvertFromFLT::visitObject(osg::Group& osgParent, ObjectRecord* rec)
{
    SObject *pSObject = (SObject*)rec->getData();
    osg::Group* object = new osg::Group;

    object->setName(pSObject->szIdent);
    visitAncillary(osgParent, *object, rec)->addChild( object );

    unsigned short  wPrevTransparency = _wObjTransparency;
    _wObjTransparency = pSObject->wTransparency;
    visitPrimaryNode(*object, (PrimNodeRecord*)rec);
    _wObjTransparency = wPrevTransparency;


    return object;
}


void ConvertFromFLT::visitFace(GeoSetBuilder* pBuilder, FaceRecord* rec)
{
    DynGeoSet* dgset = pBuilder->getDynGeoSet();
    osg::StateSet* osgStateSet = dgset->getStateSet();
    SFace *pSFace = (SFace*)rec->getData();
    bool bBlend = false;

    if (rec->getFlightVersion() > 13)
    {
        if (pSFace->dwFlags & FaceRecord::HIDDEN_BIT)
            return;
    }

    //
    // Cull face & wireframe
    //

    int drawMode = pSFace->swDrawFlag & (BIT0 | BIT1);
    switch(drawMode)
    {
        case FaceRecord::SOLID_BACKFACED:
            // Enable backface culling
            {
                osg::CullFace* cullface = new osg::CullFace;
                cullface->setMode(osg::CullFace::BACK);
                osgStateSet->setAttributeAndModes(cullface, osg::StateAttribute::ON);
            }
            break;

        case FaceRecord::SOLID_NO_BACKFACE:
            // Disable backface culling
            osgStateSet->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
            break;

        case FaceRecord::WIREFRAME_NOT_CLOSED:
            dgset->setPrimType(osg::GeoSet::LINE_STRIP);
            break;

        case FaceRecord::WIREFRAME_CLOSED:
            dgset->setPrimType(osg::GeoSet::LINE_LOOP);
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

    //
    // Lighting and color binding
    //

    if (rec->getFlightVersion() > 13)
    {
        switch(pSFace->swLightMode)
        {
            case FaceRecord::FACE_COLOR:
                // Use face color, not illuminated
                osgStateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
                dgset->setColorBinding( osg::GeoSet::BIND_OVERALL /*BIND_PERPRIM*/ );
                break;

            case FaceRecord::VERTEX_COLOR:
                // Use vertex colors, not illuminated
                osgStateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
                dgset->setColorBinding( osg::GeoSet::BIND_PERVERTEX );
                break;

            case FaceRecord::FACE_COLOR_LIGHTING:
                // Use face color and vertex normal
                osgStateSet->setMode( GL_LIGHTING, osg::StateAttribute::ON );
                dgset->setColorBinding( osg::GeoSet::BIND_OVERALL );
                dgset->setNormalBinding(osg::GeoSet::BIND_PERVERTEX);
                break;

            case FaceRecord::VERTEX_COLOR_LIGHTING:
                // Use vertex color and vertex normal
                osgStateSet->setMode( GL_LIGHTING, osg::StateAttribute::ON );
                dgset->setColorBinding( osg::GeoSet::BIND_PERVERTEX );
                dgset->setNormalBinding(osg::GeoSet::BIND_PERVERTEX);
                break;

            default :
                osgStateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
                dgset->setColorBinding( osg::GeoSet::BIND_OVERALL );
                break;
        }
    }
    else // Version 11, 12 & 13
    {
        osgStateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
        dgset->setColorBinding( osg::GeoSet::BIND_OVERALL /*BIND_PERPRIM*/ );
    }


    //
    // Face Color
    //

    if (pSFace->swTexWhite && (pSFace->iTexturePattern != -1))
    {
        // Render textured polygons white
        _faceColor.set(1, 1, 1, 1);
    }
    else
    {
        ColorPool* pColorPool = rec->getFltFile()->getColorPool();
        _faceColor.set(1, 1, 1, 1);

        if (rec->getFlightVersion() > 13)
        {
            if (!(pSFace->dwFlags & FaceRecord::NO_COLOR_BIT))
            {
                bool bPackedColor =
                        _bHdrRgbMode ||
                        (pSFace->dwFlags & FaceRecord::PACKED_COLOR_BIT) ||
                        (pColorPool == NULL);

                if (bPackedColor)
                    _faceColor = pSFace->PrimaryPackedColor.get();
                else
                    _faceColor = pColorPool->getColor(pSFace->dwPrimaryColorIndex);
            }
        }
        else // Version 11, 12 & 13
        {
            bool bPackedColor = _bHdrRgbMode || (pColorPool == NULL);

           if (bPackedColor)
                _faceColor = pSFace->PrimaryPackedColor.get();
            else
                _faceColor = pColorPool->getColor(pSFace->wPrimaryNameIndex);
        }

    }

    // Face color alpha
    _faceColor[3] = 1.0f - ((float)pSFace->wTransparency / 65535.0f);
    if (pSFace->wTransparency > 0) bBlend = true;

    if ((dgset->getColorBinding() == osg::GeoSet::BIND_OVERALL)
    ||  (dgset->getColorBinding() == osg::GeoSet::BIND_PERPRIM))
        dgset->addColor(_faceColor);

    //
    // Material
    //

    MaterialPool* pMaterialPool = rec->getFltFile()->getMaterialPool();
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

            // In contrast to the OpenFlight Specification this works!
            alpha = pSMaterial->sfAlpha * 
                (1.0f - ((float)pSFace->wTransparency / 65535.0f)) *
                (1.0f - ((float)_wObjTransparency / 65535.0f));

            ambient[0] = pSMaterial->Ambient[0] * _faceColor[0];
            ambient[1] = pSMaterial->Ambient[1] * _faceColor[1];
            ambient[2] = pSMaterial->Ambient[2] * _faceColor[2];
            ambient[3] = alpha;

            diffuse[0] = pSMaterial->Diffuse[0] * _faceColor[0];
            diffuse[1] = pSMaterial->Diffuse[1] * _faceColor[1];
            diffuse[2] = pSMaterial->Diffuse[2] * _faceColor[2];
            diffuse[3] = alpha;

            specular[0] = pSMaterial->Specular[0];
            specular[1] = pSMaterial->Specular[1];
            specular[2] = pSMaterial->Specular[2];
            specular[3] = alpha;

            emissiv[0] = pSMaterial->Emissive[0];
            emissiv[1] = pSMaterial->Emissive[1];
            emissiv[2] = pSMaterial->Emissive[2];
            emissiv[3] = alpha;

            osgMaterial->setColorMode(osg::Material::OFF);
            osgMaterial->setAmbient(osg::Material::FRONT_AND_BACK, ambient);
            osgMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, diffuse);
            osgMaterial->setSpecular(osg::Material::FRONT_AND_BACK, specular);
            osgMaterial->setEmission(osg::Material::FRONT_AND_BACK, emissiv);
            osgMaterial->setAlpha(osg::Material::FRONT_AND_BACK, alpha);
            osgMaterial->setShininess(osg::Material::FRONT_AND_BACK, pSMaterial->sfShininess/128.0f);
            osgStateSet->setAttribute(osgMaterial);

            if (alpha < 1.0f) bBlend = true;
        }
    }

    //
    // Subface
    //

    if (rec->getParent()->isOfType(FACE_OP))
    {
        if (_nSubfaceLevel > 0)
        {
            osg::PolygonOffset* polyoffset = new osg::PolygonOffset;
            if (polyoffset)
            {
                polyoffset->setFactor(-1.0f*_nSubfaceLevel);
                polyoffset->setUnits(-20.0f*_nSubfaceLevel);
                osgStateSet->setAttributeAndModes(polyoffset,osg::StateAttribute::ON);
            }
        }
    }

    //
    // Texture
    //

    if (pSFace->iTexturePattern != -1)
    {
        TexturePool* pTexturePool = rec->getFltFile()->getTexturePool();
        if (pTexturePool)
        {
            osg::StateSet *textureStateSet = dynamic_cast<osg::StateSet *>
                (pTexturePool->getTexture((int)pSFace->iTexturePattern));

            if (textureStateSet)
            {
                // Merge face stateset with texture stateset
                osgStateSet->merge(*textureStateSet);

                // Alpha channel in texture?
                osg::Texture *osgTexture = dynamic_cast<osg::Texture*>(textureStateSet->getAttribute( osg::StateAttribute::TEXTURE));
                if (osgTexture)
                {
                    osg::Image* osgImage = osgTexture->getImage();
                    switch (osgImage->pixelFormat())
                    {
                    case GL_LUMINANCE_ALPHA:
                    case GL_RGBA:
                        bBlend = true;
                        break;
                    }
                }

#if 0           // Started to experiment with OpenFlight texture mapping modes
                if (pSFace->iTextureMapIndex > -1)
                {
                    osg::TexGen* osgTexGen = new osg::TexGen;
                    osgTexGen->setMode(osg::TexGen::SPHERE_MAP);
                    osgStateSet->setAttributeAndModes(osgTexGen,osg::StateAttribute::ON);
                }
#endif

                dgset->setTextureBinding(osg::GeoSet::BIND_PERVERTEX);
            }
        }
    }


    //
    // Transparency
    //

    if (bBlend)
    {
        osg::Transparency* osgTransparency = new osg::Transparency();
        osgTransparency->setFunction(osg::Transparency::SRC_ALPHA, osg::Transparency::ONE_MINUS_SRC_ALPHA);
        osgStateSet->setAttribute(osgTransparency);
        osgStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
        osgStateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    }

    //
    // Vertices
    //

    addVertices(pBuilder, rec);

    //
    // Add face to builder GeoSet pool
    //

    pBuilder->addPrimitive();

    //
    // Look for subfaces
    //

    {
        _nSubfaceLevel++;
        int n;
        for(n=0; n<rec->getNumChildren(); n++)
        {
            Record* child = rec->getChild(n);

            if (child && child->isOfType(FACE_OP))
                visitFace(pBuilder, (FaceRecord*)child);
        }
        _nSubfaceLevel--;
    }
}


// Return number of vertices added to builder.
int ConvertFromFLT::addVertices(GeoSetBuilder* pBuilder, PrimNodeRecord* primRec)
{
    int i;
    int vertices=0;
    DynGeoSet* dgset = pBuilder->getDynGeoSet();

    for(i=0; i < primRec->getNumChildren(); i++)
    {
        Record* child = primRec->getChild(i);
        if (child == NULL) break;

        switch (child->getOpcode())
        {
        case VERTEX_LIST_OP:
            vertices += visitVertexList(pBuilder, (VertexListRecord*)child);
            break;
        
        default :
            vertices += addVertex(pBuilder, child);
            break;
        }
    }

    if (vertices > 0)
    {
        if (dgset->getPrimType() == osg::GeoSet::POINTS)
        {
            for (i=0; i < vertices; i++)
                dgset->addPrimLen(1);
        }
        else
        {
            dgset->addPrimLen(vertices);
        }
    }

    return vertices;
}

int ConvertFromFLT::visitVertexList(GeoSetBuilder* pBuilder, VertexListRecord* rec)
{
    //DynGeoSet* dgset = pBuilder->getDynGeoSet();
    int vertices = rec->numberOfVertices();

    // Add vertices to GeoSetBuilder
    for (int i=0; i < vertices; i++)
    {
        Record* vertex = getVertexFromPool(rec->getVertexPoolOffset(i));
        if (vertex)
            addVertex(pBuilder, vertex);
    }

    return vertices;
}





// Return 1 if record is a known vertex record else return 0.
int ConvertFromFLT::addVertex(DynGeoSet* dgset, Record* rec)
{
    switch(rec->getOpcode())
    {
    case VERTEX_C_OP:
        {
            SVertex* pVert = (SVertex*)rec->getData();
            osg::Vec3 coord(pVert->Coord.x(), pVert->Coord.y(), pVert->Coord.z());
            coord *= (float)_unitScale;
            dgset->addCoord(coord);
            if (dgset->getColorBinding() == osg::GeoSet::BIND_PERVERTEX)
                ADD_VERTEX_COLOR(dgset, pVert, rec->getFltFile()->getColorPool())

        }
        break;

    case VERTEX_CN_OP:
        {
            SNormalVertex* pVert = (SNormalVertex*)rec->getData();
            osg::Vec3 coord(pVert->Coord.x(), pVert->Coord.y(), pVert->Coord.z());
            coord *= (float)_unitScale;
            dgset->addCoord(coord);
            if (dgset->getNormalBinding() == osg::GeoSet::BIND_PERVERTEX)
                ADD_NORMAL(dgset, pVert)
            if (dgset->getColorBinding() == osg::GeoSet::BIND_PERVERTEX)
                ADD_VERTEX_COLOR(dgset, pVert, rec->getFltFile()->getColorPool())
        }
        break;

    case VERTEX_CNT_OP:
        {
            SNormalTextureVertex* pVert = (SNormalTextureVertex*)rec->getData();
            osg::Vec3 coord(pVert->Coord.x(), pVert->Coord.y(), pVert->Coord.z());
            coord *= (float)_unitScale;
            dgset->addCoord(coord);
            if (dgset->getNormalBinding() == osg::GeoSet::BIND_PERVERTEX)
                ADD_NORMAL(dgset, pVert)
            if (dgset->getTextureBinding() == osg::GeoSet::BIND_PERVERTEX)
                ADD_TCOORD(dgset, pVert)
            if (dgset->getColorBinding() == osg::GeoSet::BIND_PERVERTEX)
                ADD_VERTEX_COLOR(dgset, pVert, rec->getFltFile()->getColorPool())
        }
        break;

    case VERTEX_CT_OP:
        {
            STextureVertex* pVert = (STextureVertex*)rec->getData();
            osg::Vec3 coord(pVert->Coord.x(), pVert->Coord.y(), pVert->Coord.z());
            coord *= (float)_unitScale;
            dgset->addCoord(coord);
            if (dgset->getTextureBinding() == osg::GeoSet::BIND_PERVERTEX)
                ADD_TCOORD(dgset, pVert)
            if (dgset->getColorBinding() == osg::GeoSet::BIND_PERVERTEX)
                ADD_VERTEX_COLOR(dgset, pVert, rec->getFltFile()->getColorPool())
        }
        break;

    case OLD_VERTEX_OP:
        {
            SOldVertex* pVert = (SOldVertex*)rec->getData();
            osg::Vec3 coord(pVert->v[0], pVert->v[1], pVert->v[2]);
            coord *= (float)_unitScale;
            dgset->addCoord(coord);
            if ((dgset->getTextureBinding() == osg::GeoSet::BIND_PERVERTEX)
            &&  (rec->getSize() >= sizeof(SOldVertex)))
                ADD_OLD_TCOORD(dgset, pVert)
        }
        break;

    case OLD_VERTEX_COLOR_OP:
        {
            SOldVertexColor* pVert = (SOldVertexColor*)rec->getData();
            osg::Vec3 coord(pVert->v[0], pVert->v[1], pVert->v[2]);
            coord *= (float)_unitScale;
            dgset->addCoord(coord);
            if (dgset->getColorBinding() == osg::GeoSet::BIND_PERVERTEX)
                ADD_OLD_COLOR(dgset, pVert, rec->getFltFile()->getColorPool())
            if ((dgset->getTextureBinding() == osg::GeoSet::BIND_PERVERTEX)
            &&  (rec->getSize() >= sizeof(SOldVertexColor)))
                ADD_OLD_TCOORD(dgset, pVert)
        }
        break;

    case OLD_VERTEX_COLOR_NORMAL_OP:
        {
            SOldVertexColorNormal* pVert = (SOldVertexColorNormal*)rec->getData();
            osg::Vec3 coord(pVert->v[0], pVert->v[1], pVert->v[2]);
            coord *= (float)_unitScale;
            dgset->addCoord(coord);
            if (dgset->getNormalBinding() == osg::GeoSet::BIND_PERVERTEX)
            {
                osg::Vec3 normal(pVert->n[0], pVert->n[1], pVert->n[2]);
                normal /= (float)(1L<<30);
                dgset->addNormal(normal);
            }
            if (dgset->getColorBinding() == osg::GeoSet::BIND_PERVERTEX)
                ADD_OLD_COLOR(dgset, pVert, rec->getFltFile()->getColorPool())
            if ((dgset->getTextureBinding() == osg::GeoSet::BIND_PERVERTEX)
            &&  (rec->getSize() >= sizeof(SOldVertexColorNormal)))
                ADD_OLD_TCOORD(dgset, pVert)
        }
        break;

    default :
        return 0;
    }

    return 1;
}


// matrix record
osg::Group* ConvertFromFLT::visitMatrix(osg::Group& osgParent, const osg::Group& /*osgPrimary*/, MatrixRecord* rec)
{
    SMatrix* pSMatrix = (SMatrix*)rec->getData();
    osg::Transform* transform = new osg::Transform;

    osg::Matrix m;
    for(int i=0;i<4;++i)
    {
        for(int j=0;j<4;++j)
        {
            m(i,j) = pSMatrix->sfMat[i][j];
        }
    }

    // scale position.
    osg::Vec3 pos = m.getTrans();
    m *= osg::Matrix::translate(-pos);
    pos *= (float)_unitScale;
    m *= osg::Matrix::translate(pos);

    transform->setDataVariance(osg::Object::STATIC);
    transform->setMatrix(m);
    
    osgParent.addChild(transform);

    return transform;
}


osg::Group* ConvertFromFLT::visitExternal(osg::Group& osgParent, ExternalRecord* rec)
{
    // SExternalReference *pSExternal = (SExternalReference*)rec->getData();

    FltFile* pFile = rec->getExternal();
    osg::Group* external = NULL;
    if (pFile)
    {
        external = pFile->convert();
        if (external)
            visitAncillary(osgParent, *external, rec)->addChild(external);
    }

    return external;
}


void ConvertFromFLT::visitLightPoint(GeoSetBuilder* pBuilder, LightPointRecord* rec)
{
    DynGeoSet* dgset = pBuilder->getDynGeoSet();
    osg::StateSet* stateSet = dgset->getStateSet();
    SLightPoint *pSLightPoint = (SLightPoint*)rec->getData();

    dgset->setPrimType(osg::GeoSet::POINTS);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    stateSet->setMode(GL_POINT_SMOOTH, osg::StateAttribute::ON);
    dgset->setColorBinding(osg::GeoSet::BIND_PERVERTEX);

    osg::Point* point = new osg::Point;
    if (point)
    {
        point->setSize(pSLightPoint->sfSize);
        stateSet->setAttributeAndModes( point, osg::StateAttribute::ON );
//      point->setFadeThresholdSize(const float fadeThresholdSize);
//      point->setDistanceAttenuation(const Vec3& distanceAttenuation);
//      point->setStateSetModes(*stateSet, osg::StateAttribute::ON); // GL_POINT_SMOOTH

    }

    // Visit vertices
    addVertices(pBuilder, rec);
    pBuilder->addPrimitive();
}
