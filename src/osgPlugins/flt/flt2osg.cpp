// Modify ConvertFromFLT::setTexture to create a new osg::TexEnvCombiner in texture stateset to handle 
// detail texture
// Julian Ortiz, June 18th 2003.


#include <stdio.h>
#include <string.h>
#include <osg/GL>

#include <osg/Group>
#include <osg/LOD>
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/StateSet>
#include <osg/CullFace>
#include <osg/TexEnv>
#include <osg/TexEnvCombine>
#include <osg/TexGen>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/Point>
#include <osg/Material>
#include <osg/PolygonOffset>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Billboard>
#include <osg/Texture2D>
#include <osg/LightSource>
#include <osg/Image>
#include <osg/Notify>
#include <osg/Sequence>

#include <osgSim/MultiSwitch>
#include <osgSim/DOFTransform>
#include <osgSim/LightPointNode>
#include <osgSim/Sector>
#include <osgSim/BlinkSequence>


#include <osgDB/FileUtils>
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
#include "LightPointPaletteRecords.h"
#include "VertexPoolRecords.h"
#include "OldVertexRecords.h"
#include "GroupRecord.h"
#include "LodRecord.h"
#include "DofRecord.h"
#include "SwitchRecord.h"
#include "ObjectRecord.h"
#include "FaceRecord.h"
#include "MeshRecord.h"
#include "MeshPrimitiveRecord.h"
#include "TransformationRecords.h"
#include "ExternalRecord.h"
#include "LightPointRecord.h"
#include "Input.h"
#include "GeoSetBuilder.h"
#include "LongIDRecord.h"
#include "CommentRecord.h"
#include "InstanceRecords.h"
#include "LocalVertexPoolRecord.h"
#include "MultiTextureRecord.h"
#include "UVListRecord.h"
#include "LightSourceRecord.h"
#include "LightSourcePaletteRecord.h"
#include "AttrData.h"
#include "BSPRecord.h"



using namespace flt;

unsigned int mystrnlen(char *s, unsigned int maxLen)
{
    for (unsigned int i = 0; i < maxLen; i++)
    {
        if (!s[i]) return i;
    }
    return maxLen;
}

ConvertFromFLT::ConvertFromFLT() :
    _faceColor(1,1,1,1)
{
    _diOpenFlightVersion = 0;
    _diCurrentOffset = 0;
    _wObjTransparency = 0;
    _nSubfaceLevel = 0;
    _unitScale = 1.0;
    _useTextureAlphaForTranspancyBinning = true;
    _bHdrRgbMode = false;
    _currentLocalVertexPool = 0;
    _doUnitsConversion = true;
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
    // Note: There are databases that contains nodes with both Matrix and GeneralMatrix
    // ancillary records. We need just one of these to put into the scenegraph
    // Nick.
    bool mxFound  = false;

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

        case GENERAL_MATRIX_OP:
            // Note: Ancillary record creates osg node
            if (!mxFound)
            {
                parent = visitGeneralMatrix(*parent, osgPrimary, (GeneralMatrixRecord*)child);
                mxFound = true;
            }
            break;

        case MATRIX_OP:
            // Note: Ancillary record creates osg node
            if (!mxFound)
            {
                parent = visitMatrix(*parent, osgPrimary, (MatrixRecord*)child);
                mxFound = true;
            }
            break;

        case COMMENT_OP:
              visitComment(osgPrimary, (CommentRecord*)child);
            break;

        case COLOR_PALETTE_OP:
            visitColorPalette(osgPrimary, (ColorPaletteRecord*)child);
            break;

        case LIGHT_SOURCE_PALETTE_OP:
            visitLightSourcePalette(osgPrimary, (LightSourcePaletteRecord*)child);
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

        case LIGHT_PT_APPEARANCE_PALETTE_OP:
            visitLtPtAppearancePalette(osgPrimary, (LtPtAppearancePaletteRecord*)child);
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

        default:

        #ifdef _DEBUG

            osg::notify( osg::INFO ) << "flt::ConvertFromFLT::visitAncillary: "
            << "Unknown opcode: " << child->getOpcode() << "\n";
            
        #endif

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
            case MESH_OP:

                if( ((MeshRecord*)child)->getData()->swTemplateTrans == 2)  //Axis type rotate
                    visitMesh(osgParent, &billboardBuilder, (MeshRecord*)child);
                else
                    visitMesh(osgParent, &geoSetBuilder, (MeshRecord*)child);
                break;

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
#ifdef USE_DEPRECATED_LIGHTPOINT
                visitLightPoint(&geoSetBuilder, (LightPointRecord*)child);
#else
                visitLightPoint(osgParent, (LightPointRecord*)child);
#endif
                break;
            case INDEXED_LIGHT_PT_OP:
                visitLightPointIndex(osgParent, (LightPointIndexRecord*)child);
                break;
            case GROUP_OP:
                osgPrim = visitGroup(osgParent, (GroupRecord*)child);
                break;
            case LIGHT_SOURCE_OP:
                osgPrim = visitLightSource(osgParent, (LightSourceRecord*)child);
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
            case BSP_OP:
                osgPrim = visitBSP(osgParent, (BSPRecord*)child);
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
            case ROAD_CONSTRUCTION_OP:
                // treat road construction record as a group record for now
                osgPrim = visitRoadConstruction(osgParent, (GroupRecord*)child);
                break;
        
            default:

            #ifdef _DEBUG
            
                osg::notify(osg::INFO) << "In ConvertFromFLT::visitPrimaryNode(), unknown opcode: " << child->getOpcode() << std::endl;

            #endif

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

    unsigned int stringLength = mystrnlen(pSLongID->szIdent,rec->getBodyLength());
    osgParent.setName(std::string(pSLongID->szIdent,stringLength));
}


void ConvertFromFLT::visitComment(osg::Node& osgParent, CommentRecord* rec)
{
    SComment *pSComment = (SComment*)rec->getData();

    //std::cout << "ConvertFromFLT::visitComment '"<<std::string(pSComment->szComment,pSComment->RecHeader.length()-4)<<"'"<<std::endl;
    //std::cout << "ConvertFromFLT::visitComment cstyle string '"<<pSComment->szComment<<"'"<<std::endl;

    unsigned int stringLength = mystrnlen(pSComment->szComment,rec->getBodyLength());
    std::string commentfield(pSComment->szComment,stringLength);
    unsigned int front_of_line = 0;
    unsigned int end_of_line = 0;
    while (end_of_line<commentfield.size())
    {
        if (commentfield[end_of_line]=='\r')
        {
            osgParent.addDescription( std::string( commentfield, front_of_line, end_of_line-front_of_line) );
        
            if (end_of_line+1<commentfield.size() &&
                commentfield[end_of_line+1]=='\n') ++end_of_line;

            ++end_of_line;
            front_of_line = end_of_line;
        }
        else if (commentfield[end_of_line]=='\n')
        {
            osgParent.addDescription( std::string( commentfield, front_of_line, end_of_line-front_of_line) );
            ++end_of_line;
            front_of_line = end_of_line;
        }
        else ++end_of_line;
    }
    if (front_of_line<end_of_line)
    {
        osgParent.addDescription( std::string( commentfield, front_of_line, end_of_line-front_of_line) );
    }
    
}


osg::Group* ConvertFromFLT::visitHeader(HeaderRecord* rec)
{
    SHeader *pSHeader = (SHeader*)rec->getData();

    // Version
    _diOpenFlightVersion = pSHeader->diFormatRevLev;
    osg::notify(osg::INFO) << "Version " << _diOpenFlightVersion << std::endl;

    // Unit scale
    if ( _doUnitsConversion ) 
    {
        switch (rec->getFltFile()->getDesiredUnits())
        {
        case FltFile::ConvertToMeters:
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
            break;

        case FltFile::ConvertToKilometers:
            switch (pSHeader->swVertexCoordUnit)
            {
            case HeaderRecord::METERS:
                _unitScale = 0.001;
                break;
            case HeaderRecord::KILOMETERS:
                _unitScale = 1.0;
                break;
            case HeaderRecord::FEET:
                _unitScale = 0.0003048;
                break;
            case HeaderRecord::INCHES:
                _unitScale = 0.0000254;
                break;
            case HeaderRecord::NAUTICAL_MILES:
                _unitScale = 1.852;
                break;
            default:
                _unitScale = 1.0;
            }
            break;

        case FltFile::ConvertToFeet:
            switch (pSHeader->swVertexCoordUnit)
            {
            case HeaderRecord::METERS:
                _unitScale = 3.2808399;
                break;
            case HeaderRecord::KILOMETERS:
                _unitScale = 3280.839895;
                break;
            case HeaderRecord::FEET:
                _unitScale = 1.0;
                break;
            case HeaderRecord::INCHES:
                _unitScale = 0.0833333;
                break;
            case HeaderRecord::NAUTICAL_MILES:
                _unitScale = 6076.1154856 ;
                break;
            default:
                _unitScale = 1.0;
            }
            break;

        case FltFile::ConvertToInches:
            switch (pSHeader->swVertexCoordUnit)
            {
            case HeaderRecord::METERS:
                _unitScale = 39.3700787;
                break;
            case HeaderRecord::KILOMETERS:
                _unitScale = 39370.0787402;
                break;
            case HeaderRecord::FEET:
                _unitScale = 12.0;
                break;
            case HeaderRecord::INCHES:
                _unitScale = 1.0;
                break;
            case HeaderRecord::NAUTICAL_MILES:
                _unitScale = 72913.3858268;
                break;
            default:
                _unitScale = 1.0;
            }
            break;

        case FltFile::ConvertToNauticalMiles:
            switch (pSHeader->swVertexCoordUnit)
            {
            case HeaderRecord::METERS:
                _unitScale = 0.0005399568;
                break;
            case HeaderRecord::KILOMETERS:
                _unitScale = 0.5399568;
                break;
            case HeaderRecord::FEET:
                _unitScale = 0.0001646;
                break;
            case HeaderRecord::INCHES:
                _unitScale = 0.0000137;
                break;
            case HeaderRecord::NAUTICAL_MILES:
                _unitScale = 1.0;
                break;
            default:
                _unitScale = 1.0;
            }
            break;

        default:
            _unitScale = 1.0;
            break;
        }
    }
    else
    {    
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
            pColorPool->addColor(i+(4096>>7), color);
        }
    }
}

                                 /*osgParent*/
void ConvertFromFLT::visitLightSourcePalette(osg::Group& , LightSourcePaletteRecord* rec)
{

    SLightSourcePalette* pLight = (SLightSourcePalette*)rec->getData();

    osg::Light* light = new osg::Light();
    
    light->setAmbient( osg::Vec4(
                pLight->sfAmbientRGBA[0], pLight->sfAmbientRGBA[1],
                pLight->sfAmbientRGBA[2], pLight->sfAmbientRGBA[3] ) );
    light->setDiffuse( osg::Vec4(
                pLight->sfDiffuseRGBA[0], pLight->sfDiffuseRGBA[1],
                pLight->sfDiffuseRGBA[2], pLight->sfDiffuseRGBA[3] ) );
    light->setSpecular( osg::Vec4(
                pLight->sfSpecularRGBA[0], pLight->sfSpecularRGBA[1],
                pLight->sfSpecularRGBA[2], pLight->sfSpecularRGBA[3] ) );
    light->setConstantAttenuation( pLight->sfConstantAttuenation );
    light->setLinearAttenuation( pLight->sfLinearAttuenation );
    light->setQuadraticAttenuation( pLight->sfQuadraticAttuenation );
    //light->setSpotExponent( pLight->sfDropoff );
    //light->setSpotCutoff( pLight->sfCutoff );

    LightPool* pLightPool = rec->getFltFile()->getLightPool();
    pLightPool->addLight( pLight->diIndex, light );
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


    std::string textureName(pFilename);
    pTexturePool->addTextureName(nIndex, textureName);

    CERR<<"pTexturePool->addTextureName("<<nIndex<<", "<<textureName<<")"<<std::endl;
}


void ConvertFromFLT::visitLtPtAppearancePalette(osg::Group& osgParent, LtPtAppearancePaletteRecord* rec)
{
    SLightPointAppearancePalette* ltPtApp = (SLightPointAppearancePalette*)rec->getData();
    LtPtAppearancePool* pool = rec->getFltFile()->getLtPtAppearancePool();
    assert( pool );
    if (ltPtApp && pool)
    {
        LtPtAppearancePool::PoolLtPtAppearance* entry = new LtPtAppearancePool::PoolLtPtAppearance;

        entry->_iBackColorIdx = ltPtApp->backColor;
        entry->_sfIntensity = ltPtApp->intensity;
        entry->_sfMinPixelSize = ltPtApp->minPixelSize;
        entry->_sfMaxPixelSize = ltPtApp->maxPixelSize;
        entry->_sfActualSize = ltPtApp->actualSize;
        entry->_iDirectionality = ltPtApp->directionality;
        entry->_sfHLobeAngle = ltPtApp->horizLobeAngle;
        entry->_sfVLobeAngle = ltPtApp->vertLobeAngle;

        pool->add(ltPtApp->index, entry);
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


osg::Group* ConvertFromFLT::visitBSP(osg::Group& osgParent, BSPRecord* rec)
{
    // create group node for the time being
        osg::Group* group = new osg::Group;
        group->setName(rec->getData()->szIdent);

        visitAncillary(osgParent, *group, rec)->addChild( group );
        visitPrimaryNode(*group, rec);

        return group;
}

osg::Group* ConvertFromFLT::visitGroup(osg::Group& osgParent, GroupRecord* rec)
{
	const int fltVer = rec->getFltFile()->getFlightVersion();

    SGroup* currentGroup = (SGroup*) rec->getData();

    const bool forwardAnim = (currentGroup->dwFlags & GroupRecord::FORWARD_ANIM)!=0;
	// OpenFlight 15.8 adds backwards animations
    const bool backwardAnim = ( (fltVer >= 1580) &&
		((currentGroup->dwFlags & GroupRecord::BACKWARD_ANIM) != 0) );
	// Regardless of forwards or backwards, animation could have swing bit set
	const osg::Sequence::LoopMode loopMode = ( (currentGroup->dwFlags & GroupRecord::SWING_ANIM) == 0 ) ?
		osg::Sequence::LOOP : osg::Sequence::SWING;
     
    if( forwardAnim || backwardAnim)
    {
        osg::Sequence* animSeq = new osg::Sequence;
        
        visitAncillary(osgParent, *animSeq, rec)->addChild( animSeq );
        visitPrimaryNode(*animSeq, rec);

		const int numReps = (fltVer >= 1580) ?
			currentGroup->iLoopCount : 1000000;
		const float frameDuration = (fltVer >= 1580) ?
			currentGroup->fLoopDuration / (float)animSeq->getNumChildren() : 0.f;
        animSeq->setDuration( frameDuration, numReps );

        if ( forwardAnim )
            animSeq->setInterval( loopMode, 0, -1 );
        else // Backwards animation
            animSeq->setInterval( loopMode, -1, 0 );

        animSeq->setMode( osg::Sequence::START );

        // Only set the name from the normal record ID if the visitAncillary()
        // call didn't find a Long ID record on this group
        if (animSeq->getName().length() == 0)
            animSeq->setName(rec->getData()->szIdent);
        
        return animSeq;
    }
     
    else
    {
        osg::Group* group = new osg::Group;
        group->setName(rec->getData()->szIdent);
        visitAncillary(osgParent, *group, rec)->addChild( group );
        visitPrimaryNode(*group, rec);
        return group;
    }
}

osg::Group* ConvertFromFLT::visitLightSource(osg::Group& osgParent, LightSourceRecord* rec)
{

    static int lightnum = 0;

    LightPool* pLightPool = rec->getFltFile()->getLightPool();
    SLightSource* pLSource = (SLightSource*) rec->getData();

    /*
    if ( !(pLSource->dwFlags & 1) ) { // enabled
        return NULL;
    }
    */

    osg::LightSource* lightSource = new osg::LightSource();

    osg::Light* pLight = pLightPool->getLight( pLSource->diIndex );
    osg::Light* light = new osg::Light( *pLight );

    light->setPosition( osg::Vec4(
                pLSource->Coord.x(), pLSource->Coord.y(),
                pLSource->Coord.z(), 1 ) );
    light->setLightNum( lightnum );

    lightSource->setLight( light );
    lightSource->setLocalStateSetModes( osg::StateAttribute::ON );
    osg::Node* node = &osgParent;
    if ( 1 ) { //pLSource->dwFlags & 2 ) { // global
        while ( !node->getParents().empty() ) {
            node = *(node->getParents().begin());
        }
    }
    lightSource->setStateSetModes( *(node->getOrCreateStateSet()),
            osg::StateAttribute::ON );

    lightnum++;

    osgParent.addChild( lightSource );
    visitPrimaryNode(*lightSource, rec);

    return lightSource;
}

osg::Group* ConvertFromFLT::visitRoadConstruction(osg::Group& osgParent, GroupRecord* rec)
{
    osg::Group* group = new osg::Group;

    group->setName(rec->getData()->szIdent);
    //cout<<"Converted a road construction node of ID "<<group->getName()<<" to group node."<<endl;
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
    
    lod->setRange(0, pSLOD->dfSwitchOutDist*_unitScale,
                     pSLOD->dfSwitchInDist*_unitScale);
                     
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
        
    lod->setRange(0, ((float)pSLOD->dwSwitchOutDist)*_unitScale,
                     ((float)pSLOD->dwSwitchInDist)*_unitScale);
                     
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
#define USE_DOFTransform

#if defined(USE_DOFTransform)

    osgSim::DOFTransform* transform = new osgSim::DOFTransform;
    transform->setName(rec->getData()->szIdent);
    transform->setDataVariance(osg::Object::DYNAMIC);
    visitAncillary(osgParent, *transform, rec)->addChild( transform );
    visitPrimaryNode(*transform, (PrimNodeRecord*)rec);

    SDegreeOfFreedom* p_data = rec->getData();

    //now fill up members:

    //tranlsations:
    transform->setMinTranslate(osg::Vec3(_unitScale*p_data->dfX._dfMin,
                      _unitScale*p_data->dfY._dfMin,
                      _unitScale*p_data->dfZ._dfMin));

    transform->setMaxTranslate(osg::Vec3(_unitScale*p_data->dfX._dfMax,
                      _unitScale*p_data->dfY._dfMax,
                      _unitScale*p_data->dfZ._dfMax));

    transform->setCurrentTranslate(osg::Vec3(_unitScale*p_data->dfX._dfCurrent,
                      _unitScale*p_data->dfY._dfCurrent,
                      _unitScale*p_data->dfZ._dfCurrent));

    transform->setIncrementTranslate(osg::Vec3(_unitScale*p_data->dfX._dfIncrement,
                      _unitScale*p_data->dfY._dfIncrement,
                      _unitScale*p_data->dfZ._dfIncrement));

    //rotations:
    transform->setMinHPR(osg::Vec3(osg::inDegrees(p_data->dfYaw._dfMin),
                        osg::inDegrees(p_data->dfPitch._dfMin),
                        osg::inDegrees(p_data->dfRoll._dfMin)));

    transform->setMaxHPR(osg::Vec3(osg::inDegrees(p_data->dfYaw._dfMax),
                        osg::inDegrees(p_data->dfPitch._dfMax),
                        osg::inDegrees(p_data->dfRoll._dfMax)));

    transform->setCurrentHPR(osg::Vec3(osg::inDegrees(p_data->dfYaw._dfCurrent),
                        osg::inDegrees(p_data->dfPitch._dfCurrent),
                        osg::inDegrees(p_data->dfRoll._dfCurrent)));

    transform->setIncrementHPR(osg::Vec3(osg::inDegrees(p_data->dfYaw._dfIncrement),
                        osg::inDegrees(p_data->dfPitch._dfIncrement),
                        osg::inDegrees(p_data->dfRoll._dfIncrement)));

    //scales:
    transform->setMinScale(osg::Vec3(p_data->dfXscale._dfMin,
                        p_data->dfYscale._dfMin,
                        p_data->dfZscale._dfMin));

    transform->setMaxScale(osg::Vec3(p_data->dfXscale._dfMax,
                        p_data->dfYscale._dfMax,
                        p_data->dfZscale._dfMax));

    transform->setCurrentScale(osg::Vec3(p_data->dfXscale._dfCurrent,
                        p_data->dfYscale._dfCurrent,
                        p_data->dfZscale._dfCurrent));

    transform->setIncrementScale(osg::Vec3(p_data->dfXscale._dfIncrement,
                        p_data->dfYscale._dfIncrement,
                        p_data->dfZscale._dfIncrement));

    // compute the put matrix.
    osg::Vec3 O ( p_data->OriginLocalDOF.x(),
                  p_data->OriginLocalDOF.y(),
                  p_data->OriginLocalDOF.z());

    osg::Vec3 xAxis ( p_data->PointOnXaxis.x(),
                      p_data->PointOnXaxis.y(),
                      p_data->PointOnXaxis.z());
    xAxis = xAxis - O;
    xAxis.normalize();

    osg::Vec3 xyPlane ( p_data->PointInXYplane.x(),
                        p_data->PointInXYplane.y(),
                        p_data->PointInXYplane.z());
    xyPlane = xyPlane - O;
    xyPlane.normalize();
    
    osg::Vec3 normalz = xAxis ^ xyPlane;
    normalz.normalize();

    // get X, Y, Z axis of the DOF in terms of global coordinates
    osg::Vec3 Rz = normalz;
    if (Rz == osg::Vec3(0,0,0)) Rz[2] = 1;
    osg::Vec3 Rx = xAxis;
    if (Rx == osg::Vec3(0,0,0)) Rx[0] = 1;
    osg::Vec3 Ry = Rz ^ Rx;


    O *= _unitScale;

    // create the putmatrix
    osg::Matrix inv_putmat( Rx.x(), Rx.y(), Rx.z(), 0, 
                            Ry.x(), Ry.y(), Ry.z(), 0,
                            Rz.x(), Rz.y(), Rz.z(), 0,
                            O.x(), O.y(), O.z(), 1);

    transform->setInversePutMatrix(inv_putmat);
    transform->setPutMatrix(osg::Matrix::inverse(inv_putmat));


/*
    //and now do a little ENDIAN to put flags in ordewr as described in OpenFlight spec
    unsigned long flags = p_data->dwFlags;
    ENDIAN(flags);

    //and setup limitation flags:
//    transform->setLimitationFlags(flags);
*/
    transform->setLimitationFlags(p_data->dwFlags);

    return transform;

#else

    osg::MatrixTransform* transform = new osg::MatrixTransform;

    transform->setName(rec->getData()->szIdent);
    transform->setDataVariance(osg::Object::DYNAMIC);
    visitAncillary(osgParent, *transform, rec)->addChild( transform );
    visitPrimaryNode(*transform, (PrimNodeRecord*)rec);
        
    SDegreeOfFreedom* p_data = rec->getData();

    // get transformation to O_world
    osg::Vec3 O ( p_data->OriginLocalDOF.x(),
                  p_data->OriginLocalDOF.y(),
                  p_data->OriginLocalDOF.z());

    osg::Vec3 xAxis ( p_data->PointOnXaxis.x(),
                      p_data->PointOnXaxis.y(),
                      p_data->PointOnXaxis.z());
    xAxis = xAxis - O;
    xAxis.normalize();

    osg::Vec3 xyPlane ( p_data->PointInXYplane.x(),
                        p_data->PointInXYplane.y(),
                        p_data->PointInXYplane.z());
    xyPlane = xyPlane - O;
    xyPlane.normalize();
    
    osg::Vec3 normalz = xAxis ^ xyPlane;
    normalz.normalize();

    // get X, Y, Z axis of the DOF in terms of global coordinates
    osg::Vec3 Rz = normalz;
    if (Rz == osg::Vec3(0,0,0)) Rz[2] = 1;
    osg::Vec3 Rx = xAxis;
    if (Rx == osg::Vec3(0,0,0)) Rx[0] = 1;
    osg::Vec3 Ry = Rz ^ Rx;

    // create the putmatrix
    osg::Matrix putmat( Rx.x(), Rx.y(), Rx.z(), 0, 
                        Ry.x(), Ry.y(), Ry.z(), 0,
                        Rz.x(), Rz.y(), Rz.z(), 0,
                        O.x(), O.y(), O.z(), 1);

    // apply DOF transformation
    osg::Vec3 trans(_unitScale*p_data->dfX._dfCurrent,
                    _unitScale*p_data->dfY._dfCurrent,
                    _unitScale*p_data->dfZ._dfCurrent);

    float roll_rad = osg::inDegrees(p_data->dfRoll._dfCurrent);
    float pitch_rad = osg::inDegrees(p_data->dfPitch._dfCurrent);
    float yaw_rad = osg::inDegrees(p_data->dfYaw._dfCurrent);

    float sx = rec->getData()->dfXscale.current();
    float sy = rec->getData()->dfYscale.current();
    float sz = rec->getData()->dfZscale.current();

    // this is the local DOF transformation
    osg::Matrix dof_matrix = osg::Matrix::scale(sx, sy, sz)*
                             osg::Matrix::rotate(yaw_rad,  0.0f,0.0f,1.0f)*
                             osg::Matrix::rotate(roll_rad,  0.0f,1.0f,0.0f)*
                             osg::Matrix::rotate(pitch_rad, 1.0f,0.0f,0.0f)*
                             osg::Matrix::translate(trans);

    // transforming local into global
    dof_matrix.preMult(osg::Matrix::inverse(putmat));

    cout << "putmat "<< putmat<<endl;

    // transforming global into local
    dof_matrix.postMult(putmat);

    transform->setMatrix(dof_matrix);
    return transform;        
    
#endif

}


osg::Group* ConvertFromFLT::visitSwitch(osg::Group& osgParent, SwitchRecord* rec)
{
    SSwitch *pSSwitch = (SSwitch*)rec->getData();
    osgSim::MultiSwitch* osgSwitch = new osgSim::MultiSwitch;

    osgSwitch->setName(pSSwitch->szIdent);
    visitAncillary(osgParent, *osgSwitch, rec)->addChild( osgSwitch );
    visitPrimaryNode(*osgSwitch, (PrimNodeRecord*)rec);

    unsigned int totalNumChildren = (unsigned int)rec->getNumChildren();
    if (totalNumChildren!=osgSwitch->getNumChildren())
    {
        // only convert the children we agree on,
        // however, there could be a chance that ordering of children might
        // be different if there a children that hanvn't mapped across...
        if (totalNumChildren>osgSwitch->getNumChildren()) totalNumChildren=osgSwitch->getNumChildren();        
        osg::notify(osg::INFO)<<"Warning::OpenFlight loader has come across an incorrectly handled switch."<<std::endl;
        osg::notify(osg::INFO)<<"         The number of OpenFlight children ("<<rec->getNumChildren()<<") "<<std::endl;
        osg::notify(osg::INFO)<<"         exceeds the number converted to OSG ("<<osgSwitch->getNumChildren()<<")"<<std::endl;
    }

    // for each mask in the FLT Switch node
    for (int itMask=0; itMask<pSSwitch->nMasks; ++itMask) 
    {
        // create a osg group node
        osg::ref_ptr<osg::Group> group = new osg::Group;
        osgSwitch->addChild( group.get() );
        // for each child in the FLT Switch node
        for (unsigned int itChild=0; itChild<totalNumChildren; ++itChild)
        {
            // test if this child is active in the current mask (itMask)
            unsigned int nMaskBit = itChild % 32;
            unsigned int nMaskWord = itMask * pSSwitch->nWordsInMask + itChild / 32;
            osgSwitch->setValue(itMask, itChild, (pSSwitch->aMask[nMaskWord] & (uint32(1) << nMaskBit))!=0 );
        }
        // now the group contain all the childrens that are active in the current mask (itMask)
    }
    osgSwitch->setActiveSwitchSet(pSSwitch->nCurrentMask);   
    return osgSwitch;
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


void ConvertFromFLT::setCullFaceAndWireframe ( const SFace *pSFace, osg::StateSet *osgStateSet, DynGeoSet *dgset )
{
    //
    // Cull face & wireframe
    //

    switch(pSFace->swDrawFlag)
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
            dgset->setPrimType(osg::PrimitiveSet::LINE_STRIP);
            break;

        case FaceRecord::WIREFRAME_CLOSED:
            dgset->setPrimType(osg::PrimitiveSet::LINE_LOOP);
            break;

        case FaceRecord::OMNIDIRECTIONAL_LIGHT:
        case FaceRecord::UNIDIRECTIONAL_LIGHT:
        case FaceRecord::BIDIRECTIONAL_LIGHT:
            dgset->setPrimType(osg::PrimitiveSet::POINTS);
            break;
    }
}


void ConvertFromFLT::setDirectionalLight()
{
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
}


void ConvertFromFLT::setLightingAndColorBinding ( const FaceRecord *rec, const SFace *pSFace, osg::StateSet *osgStateSet, DynGeoSet *dgset )
{
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
                dgset->setColorBinding( osg::Geometry::BIND_OVERALL /*BIND_PERPRIM*/ );
                break;

            case FaceRecord::VERTEX_COLOR:
                // Use vertex colors, not illuminated
                osgStateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
                dgset->setColorBinding( osg::Geometry::BIND_PER_VERTEX );
                break;

            case FaceRecord::FACE_COLOR_LIGHTING:
                // Use face color and vertex normal
                osgStateSet->setMode( GL_LIGHTING, osg::StateAttribute::ON );
                dgset->setColorBinding( osg::Geometry::BIND_OVERALL );
                dgset->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
                break;

            case FaceRecord::VERTEX_COLOR_LIGHTING:
                // Use vertex color and vertex normal
                osgStateSet->setMode( GL_LIGHTING, osg::StateAttribute::ON );
                dgset->setColorBinding( osg::Geometry::BIND_PER_VERTEX );
                dgset->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
                break;

            default :
                osgStateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
                dgset->setColorBinding( osg::Geometry::BIND_OVERALL );
                break;
        }
    }
    else // Version 11, 12 & 13
    {
        osgStateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
        dgset->setColorBinding( osg::Geometry::BIND_OVERALL /*BIND_PERPRIM*/ );
    }
}


void ConvertFromFLT::setColor ( FaceRecord *rec, SFace *pSFace, DynGeoSet *dgset, bool &bBlend )
{
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
                {
                    if (rec->getFlightVersion() >= 1540)
                    {
                        _faceColor = 
                            pColorPool->getColor(pSFace->dwPrimaryColorIndex);
                    }
                    else  // Version 14.2, 15.0
                    {
                        // The field now called wPrimaryNameIndex was
                        // originally the primary color/intensity code
                        // for OpenFlight v14.2 and v15.0 files
                        _faceColor = 
                            pColorPool->getColor(pSFace->wPrimaryNameIndex);
                    }
                }
            }
        }
        else // Version 11, 12 & 13
        {
            bool bPackedColor = _bHdrRgbMode || (pColorPool == NULL);

           if (bPackedColor)
                _faceColor = pSFace->PrimaryPackedColor.get();
            else
                _faceColor = pColorPool->getOldColor(pSFace->wPrimaryNameIndex);
        }

    }

    // Face color alpha
    _faceColor[3] = 1.0f - ((float)pSFace->wTransparency / 65535.0f);
    if (pSFace->wTransparency > 0) bBlend = true;

    if ((dgset->getColorBinding() == osg::Geometry::BIND_OVERALL)
    ||  (dgset->getColorBinding() == osg::Geometry::BIND_PER_PRIMITIVE))
        dgset->addColor(_faceColor);
}


void ConvertFromFLT::setMaterial ( FaceRecord *rec, SFace *pSFace, osg::StateSet *osgStateSet, bool &bBlend )
{
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
            osgMaterial->setShininess(osg::Material::FRONT_AND_BACK, pSMaterial->sfShininess);
            osgStateSet->setAttribute(osgMaterial);

            if (alpha < 1.0f) bBlend = true;
        }
    }
}


void ConvertFromFLT::setTexture ( FaceRecord *rec, SFace *pSFace, osg::StateSet *osgStateSet, DynGeoSet *dgset, bool &bBlend )
{
    //
    // Texture
    //

    if (pSFace->iTexturePattern != -1)
    {
        TexturePool* pTexturePool = rec->getFltFile()->getTexturePool();
        if (pTexturePool)
        {
            int nIndex = (int)pSFace->iTexturePattern;
            flt::AttrData *textureAttrData = pTexturePool->getTexture(nIndex,rec->getFlightVersion());

            osg::StateSet *textureStateSet;
            if (textureAttrData)
              textureStateSet = textureAttrData->stateset;
            else
              textureStateSet = NULL;

            if (textureStateSet)
            {
                //We got detail texture, so we got detailTexture stateset and add a TexEnvCombine attribute
                // To add simple detail texture we just use texture unit 1 to store detail
                //As Creators help says, if Mag. Filter General is set to Modulate Detail we just
                //add the detail, but if it set to Add Detail then we got a lighter image, so we
                //use scale_rgb and scale_alpha of osg::TexEnvCombine to make this effect
                // Julian Ortiz, June 18th 2003.

                flt::AttrData *detailTextureAttrData = NULL;                                
                if (pSFace->iDetailTexturePattern != -1) {                 
                 int nIndex2 = (int)pSFace->iDetailTexturePattern;
                 detailTextureAttrData = pTexturePool->getTexture(nIndex2,rec->getFlightVersion());
                 if (detailTextureAttrData && detailTextureAttrData->stateset) {
                     osg::Texture2D *detTexture = dynamic_cast<osg::Texture2D*>(detailTextureAttrData->stateset->getTextureAttribute( 0, osg::StateAttribute::TEXTURE));
                     textureStateSet->setTextureAttributeAndModes(1,detTexture,osg::StateAttribute::ON);                    
                     osg::TexEnvCombine *tec1 = new osg::TexEnvCombine;
                     float scale = (detailTextureAttrData->modulateDetail==0)?2.0f:4.0f;                     
                     tec1->setScale_RGB(scale);
                     tec1->setScale_Alpha(scale);
                     textureStateSet->setTextureAttribute( 1, tec1,osg::StateAttribute::ON );                                            
                 }                 
                }

                //Now, an ugly thing,... we have detected that in Creator we defined that a texture will we used as
                //detail texture, and we load it as it using texture unit 1,... but we also need to create texture 
                //coordinates to map this detail texture, I found that texture coordinates assigment is made in
                //DynGeoSet::addToGeometry and the easy way I found to create those new coordinates is to add a method
                //to DynGeoSet class named setDetailTextureStatus that pass detail Texture AttrData class, so when 
                //DynGeoSet::addToGeometry runs it reads this class and create new texture coordinates if we got a valid
                //AttrData object. I now this is not a good way to do it, and I expect someone with more osg knowledge
                //could make it in a better way.
                // Julian Ortiz, June 18th 2003.                     
                if (pSFace->iDetailTexturePattern != -1 && detailTextureAttrData && detailTextureAttrData->stateset)
                 dgset->setDetailTextureAttrData(detailTextureAttrData);
                else 
                 dgset->setDetailTextureAttrData(NULL);

                // Merge face stateset with texture stateset
                osgStateSet->merge(*textureStateSet);

                // Alpha channel in texture?
                osg::Texture2D *osgTexture = dynamic_cast<osg::Texture2D*>(textureStateSet->getTextureAttribute( 0, osg::StateAttribute::TEXTURE));
                if (osgTexture)
                {
                    osg::Image* osgImage = osgTexture->getImage();
                    if (getUseTextureAlphaForTransparancyBinning() && osgImage->isImageTranslucent()) bBlend = true;
                    
                }

#if 0           // Started to experiment with OpenFlight texture mapping modes
                if (pSFace->iTextureMapIndex > -1)
                {
                    osg::TexGen* osgTexGen = new osg::TexGen;
                    osgTexGen->setMode(osg::TexGen::SPHERE_MAP);
                    osgStateSet->setTextueAttributeAndModes( 0, osgTexGen,osg::StateAttribute::ON);
                }
#endif

                dgset->setTextureBinding(osg::Geometry::BIND_PER_VERTEX);
            }
        }
    }
}


void ConvertFromFLT::setTransparency ( osg::StateSet *osgStateSet, bool &bBlend )
{
    //
    // Transparency
    //

    if (bBlend)
    {
        osg::BlendFunc* osgBlendFunc = new osg::BlendFunc();
        osgBlendFunc->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
        osgStateSet->setAttribute(osgBlendFunc);
        osgStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
        osgStateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    }
}

void
ConvertFromFLT::addMultiTexture( DynGeoSet* dgset, MultiTextureRecord* mtr )
{
    osg::Geometry* geom = dgset->getGeometry();
    
    if (geom==0 || mtr==0 || !mtr->isAncillaryRecord())
    {
        osg::notify(osg::WARN)<<"ConvertFromFLT::addMultiTexture(DynGeoSet*, MultiTextureRecord*) has been passed invalid paramters."<<std::endl;
        return;
    }
    
    SMultiTexture* mt = reinterpret_cast<SMultiTexture*>(mtr->getData());
    if (mt==0)
    {
        osg::notify(osg::WARN)<<"ConvertFromFLT::addMultiTexture(DynGeoSet*, MultiTextureRecord*) mtr->getData() not valid SMultiTexture*."<<std::endl;
        return;
    }


    CERR << "ConvertFromFLT::addMultiTexture\n";
    int l = 0;
    for ( int i = 0; i < 8; i++ )
    {
        if ( (1 << (32-i)) & mt->layers )
        {
            CERR << "Has layer " << i << "\n";
            mt->data[l].endian();
            CERR << "texture: " << mt->data[l].texture << "\n";
            CERR << "effect: " << mt->data[l].effect << "\n";
            CERR << "mapping: " << mt->data[l].mapping << "\n";
            CERR << "data: " << mt->data[l].data << "\n";

            TexturePool* pTexturePool = mtr->getFltFile()->getTexturePool();
            
            assert( pTexturePool );
            if (!pTexturePool)
            {
                osg::notify(osg::WARN)<<"ConvertFromFLT::addMultiTexture(DynGeoSet*, MultiTextureRecord*) pTexturePool invalid."<<std::endl;
                return;
            }
            
            osg::StateSet *textureStateSet = dynamic_cast<osg::StateSet *> (pTexturePool->getTexture((int)mt->data[l].texture,mtr->getFlightVersion()));

            CERR << "pTexturePool->getTexture((int)mt->data[l].texture): " << pTexturePool->getTexture((int)mt->data[l].texture,mtr->getFlightVersion()) << "\n";
            CERR << "textureStateSet: " << textureStateSet << "\n";

            if (!textureStateSet)
            {
                CERR << "unable to set up multi-texture layer." << std::endl;
                return;
            }

            osg::Texture2D *texture = dynamic_cast<osg::Texture2D*>(textureStateSet->getTextureAttribute(0, osg::StateAttribute::TEXTURE));
            CERR << "texture: " << texture << "\n";

            osg::StateSet* texture_stateset = new osg::StateSet;
            CERR << "texture_stateset: " << texture_stateset << "\n";

            if (!texture)
            {
                osg::notify(osg::WARN)<<"ConvertFromFLT::addMultiTexture(DynGeoSet*, MultiTextureRecord*) texture invalid."<<std::endl;
                return;
            }


            texture_stateset->setTextureAttributeAndModes(
            i, texture,osg::StateAttribute::ON);

            osg::TexEnv* osgTexEnv = new osg::TexEnv;

            CERR << "osgTexEnv: " << osgTexEnv << "\n";

            osgTexEnv->setMode(osg::TexEnv::MODULATE);
            texture_stateset->setTextureAttribute( i, osgTexEnv );

            CERR << "geom: " << geom << "\n";
            CERR << ", referenceCount: "
                 << geom->referenceCount() << "\n";

            osg::StateSet* geom_stateset = geom->getStateSet();
            
            CERR << "geom_stateset: " << geom_stateset << "\n";
                
            if ( geom_stateset )
            {
                geom_stateset->merge( *texture_stateset );
                CERR << "Merging layer " << i << "\n";
            } else {
                geom->setStateSet( texture_stateset );
                CERR << "Setting layer " << i << "\n";
            }

            l++;
        }
    }
}

void
ConvertFromFLT::addUVList( DynGeoSet* dgset, UVListRecord* uvr )
{
    osg::Geometry* geom = dgset->getGeometry();

    if (!geom || !uvr || !uvr->isAncillaryRecord())
    {
        osg::notify(osg::WARN)<<"ConvertFromFLT::addUVList( DynGeoSet*, UVListRecord*) has been passed invalid paramters."<<std::endl;
        return;
    }

    SUVList* uvl = reinterpret_cast<SUVList*>(uvr->getData());
    if (!uvl)
    {
        osg::notify(osg::WARN)<<"ConvertFromFLT::addUVList( DynGeoSet*, UVListRecord*) uvr->getData() is invalid."<<std::endl;
        return;
    }
    
    CERR << "ConvertFromFLT::addUVList\n";
    int l = 0;
    int num_coords = dgset->coordListSize();
    for ( int i = 0; i < 8; i++ )
    {
        if ( (1 << (32-i)) & uvl->layers )
        {
            osg::Vec2Array* tcoords = new osg::Vec2Array;
            CERR << "Has layer " << i << "\n";
            // Assume we are working with vertex lists for now
            for ( int v = l*num_coords; v < (l+1)*num_coords; v++ )
            {
                uvl->coords.vertex[v].endian();
                CERR << "( u: " << uvl->coords.vertex[v].coords[1] << ", "
                            << "v: " << uvl->coords.vertex[v].coords[0] << ")\n";
                        /// FIXME: should be (x,y) instead of (y,x) - ENDIAN problem???
                tcoords->push_back( osg::Vec2( uvl->coords.vertex[v].coords[1],
                        uvl->coords.vertex[v].coords[0] ) );
            }
            if ( !tcoords->empty() )
            {
                CERR << "Setting tcoords " << i << ": " << tcoords << "\n";
                geom->setTexCoordArray( i, tcoords );
            }

            l++;
        }
    }
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

    // Various properties.
    setCullFaceAndWireframe ( pSFace, osgStateSet, dgset );
    setDirectionalLight();
    setLightingAndColorBinding ( rec, pSFace, osgStateSet, dgset );
    setColor ( rec, pSFace, dgset, bBlend );
    setMaterial ( rec, pSFace, osgStateSet, bBlend );

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
                osgStateSet->setAttributeAndModes(polyoffset,osg::StateAttribute::ON);
            }
        }
    }

    // Texture.
    setTexture ( rec, pSFace, osgStateSet, dgset, bBlend );

    // Transparency
    setTransparency ( osgStateSet, bBlend );

    // Vertices
    addVertices(pBuilder, rec);

    // Add face to builder pool
    pBuilder->addPrimitive();

    // Visit ancillary records
    for(int i=0; i < rec->getNumChildren(); i++)
    {
        Record* child = rec->getChild(i);
        if (!child->isAncillaryRecord())
            break;

    switch (child->getOpcode())
    {
        case MULTI_TEXTURE_OP:
        {
            MultiTextureRecord* mtr =
            dynamic_cast<MultiTextureRecord*>(child);
            if (!mtr)
            {
                osg::notify( osg::WARN ) << "flt::ConvertFromFLT::visitFace(GeoSetBuilder*, FaceRecord*) found invalid MultiTextureRecord*"<<std::endl;
                return;
            }
            
            // original code, but causes crash becayse addPrimitive can invalidate teh dgset pointer.
            // addMultiTexture( dgset, mtr );
            
            addMultiTexture( pBuilder->getDynGeoSet(), mtr );
        }
        break;

        default:

        #ifdef _DEBUG

        osg::notify( osg::NOTICE) << "flt::ConvertFromFLT::visitFace: "
            << "Unhandled opcode: " << child->getOpcode() << "\n";
        #endif

        break;
    }
    }

    // Look for subfaces
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
        
        case LOCAL_VERTEX_POOL_OP:
            vertices += visitLocalVertexPool(pBuilder, (LocalVertexPoolRecord *)child);
            break;

        default :
            vertices += addVertex(pBuilder, child);
            break;
        }
    }

    if (vertices > 0)
    {
        if (dgset->getPrimType() == osg::PrimitiveSet::POINTS)
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
    DynGeoSet* dgset = pBuilder->getDynGeoSet();
    int vertices = rec->numberOfVertices();

    // Add vertices to GeoSetBuilder
    for (int j=0; j < vertices; j++)
    {
        Record* vertex = getVertexFromPool(rec->getVertexPoolOffset(j));
        if (vertex)
            addVertex(pBuilder, vertex);
    }

    // Visit ancillary records
    for(int i=0; i < rec->getNumChildren(); i++)
    {
        Record* child = rec->getChild(i);
    CERR << "OPCODE: " << child->getOpcode() << "\n";
        if (!child->isAncillaryRecord())
            break;

    switch (child->getOpcode())
    {
        case UV_LIST_OP:
        {
            UVListRecord* uvr =
            dynamic_cast<UVListRecord*>(child);
            assert( uvr );
            addUVList( dgset, uvr );
        }
        break;
        case MULTI_TEXTURE_OP:
        {
            CERR2 << "MULTI_TEXTURE_OP in visitVertexList\n";
            MultiTextureRecord* mtr =
            dynamic_cast<MultiTextureRecord*>(child);
            assert( mtr );
            addMultiTexture( dgset, mtr );
        }
        break;
        default:

        #ifdef _DEBUG

        osg::notify( osg::NOTICE )
            << "flt::ConvertFromFLT::visitVertexList: "
            << "Unhandled opcode: " << child->getOpcode() << "\n";

        #endif

        break;
    }
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
            if (dgset->getColorBinding() == osg::Geometry::BIND_PER_VERTEX)
                ADD_VERTEX_COLOR(dgset, pVert, rec->getFltFile()->getColorPool())

        }
        break;

    case VERTEX_CN_OP:
        {
            SNormalVertex* pVert = (SNormalVertex*)rec->getData();
            osg::Vec3 coord(pVert->Coord.x(), pVert->Coord.y(), pVert->Coord.z());
            coord *= (float)_unitScale;
            dgset->addCoord(coord);
            if (dgset->getNormalBinding() == osg::Geometry::BIND_PER_VERTEX)
                ADD_NORMAL(dgset, pVert)
            if (dgset->getColorBinding() == osg::Geometry::BIND_PER_VERTEX)
                ADD_VERTEX_COLOR(dgset, pVert, rec->getFltFile()->getColorPool())
        }
        break;

    case VERTEX_CNT_OP:
        {
            SNormalTextureVertex* pVert = (SNormalTextureVertex*)rec->getData();
            osg::Vec3 coord(pVert->Coord.x(), pVert->Coord.y(), pVert->Coord.z());
            coord *= (float)_unitScale;
            dgset->addCoord(coord);
            if (dgset->getNormalBinding() == osg::Geometry::BIND_PER_VERTEX)
                ADD_NORMAL(dgset, pVert)
            if (dgset->getTextureBinding() == osg::Geometry::BIND_PER_VERTEX)
                ADD_TCOORD(dgset, pVert)
            if (dgset->getColorBinding() == osg::Geometry::BIND_PER_VERTEX)
                ADD_VERTEX_COLOR(dgset, pVert, rec->getFltFile()->getColorPool())
        }
        break;

    case VERTEX_CT_OP:
        {
            STextureVertex* pVert = (STextureVertex*)rec->getData();
            osg::Vec3 coord(pVert->Coord.x(), pVert->Coord.y(), pVert->Coord.z());
            coord *= (float)_unitScale;
            dgset->addCoord(coord);
            if (dgset->getTextureBinding() == osg::Geometry::BIND_PER_VERTEX)
                ADD_TCOORD(dgset, pVert)
            if (dgset->getColorBinding() == osg::Geometry::BIND_PER_VERTEX)
                ADD_VERTEX_COLOR(dgset, pVert, rec->getFltFile()->getColorPool())
        }
        break;

    case OLD_VERTEX_OP:
        {
            SOldVertex* pVert = (SOldVertex*)rec->getData();
            osg::Vec3 coord(pVert->v[0], pVert->v[1], pVert->v[2]);
            coord *= (float)_unitScale;
            dgset->addCoord(coord);
            if ((dgset->getTextureBinding() == osg::Geometry::BIND_PER_VERTEX)
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
            if (dgset->getColorBinding() == osg::Geometry::BIND_PER_VERTEX)
                ADD_OLD_COLOR(dgset, pVert, rec->getFltFile()->getColorPool())
            if ((dgset->getTextureBinding() == osg::Geometry::BIND_PER_VERTEX)
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
            if (dgset->getNormalBinding() == osg::Geometry::BIND_PER_VERTEX)
            {
                osg::Vec3 normal(pVert->n[0], pVert->n[1], pVert->n[2]);
                normal /= (float)(1L<<30);
                dgset->addNormal(normal);
            }
            if (dgset->getColorBinding() == osg::Geometry::BIND_PER_VERTEX)
                ADD_OLD_COLOR(dgset, pVert, rec->getFltFile()->getColorPool())
            if ((dgset->getTextureBinding() == osg::Geometry::BIND_PER_VERTEX)
            &&  (rec->getSize() >= sizeof(SOldVertexColorNormal)))
                ADD_OLD_TCOORD(dgset, pVert)
        }
        break;

    default :
        return 0;
    }

    return 1;
}

// general matrix
osg::Group* ConvertFromFLT::visitGeneralMatrix(osg::Group& osgParent, const osg::Group& /*osgPrimary*/, GeneralMatrixRecord* rec)
{
    SGeneralMatrix* pSMatrix = (SGeneralMatrix*)rec->getData();
    osg::MatrixTransform* transform = new osg::MatrixTransform;

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

// matrix record
osg::Group* ConvertFromFLT::visitMatrix(osg::Group& osgParent, const osg::Group& /*osgPrimary*/, MatrixRecord* rec)
{
    SMatrix* pSMatrix = (SMatrix*)rec->getData();
    osg::MatrixTransform* transform = new osg::MatrixTransform;

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

    std::string filePath = osgDB::getFilePath(rec->getFilename());
    std::string pushAndPopPath;
    //If absolute path
    if( (filePath.length()>0 && filePath.find_first_of("/\\")==0) ||
        (filePath.length()>2 && filePath.substr(1,1)==":" && filePath.find_first_of("/\\")==2) )
    {
        pushAndPopPath = filePath;
    }
    else
    {
        osgDB::FilePathList fpl = osgDB::getDataFilePathList();
        pushAndPopPath = fpl.empty() ? "." : fpl.front();
        if(pushAndPopPath.empty()) pushAndPopPath = ".";
        pushAndPopPath += "/" + filePath;
    }

    osgDB::PushAndPopDataPath tmpfile(pushAndPopPath);
    //osgDB::PushAndPopDataPath tmpfile(osgDB::getFilePath(rec->getFilename()));

    
    FltFile* pFile = rec->getExternal();
    osg::Group* external = NULL;
    if (pFile)
    {
        pFile->setDesiredUnits( rec->getFltFile()->getDesiredUnits() );
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

    dgset->setPrimType(osg::PrimitiveSet::POINTS);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    stateSet->setMode(GL_POINT_SMOOTH, osg::StateAttribute::ON);
    dgset->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

    osg::Point* point = new osg::Point;
    if (point)
    {
        /*
        point->setSize(pSLightPoint->sfSize);
        stateSet->setAttributeAndModes( point, osg::StateAttribute::ON );
//      point->setFadeThresholdSize(const float fadeThresholdSize);
//      point->setDistanceAttenuation(const Vec3& distanceAttenuation);
//      point->setStateSetModes(*stateSet, osg::StateAttribute::ON); // GL_POINT_SMOOTH
    */
        //change to:
        point->setSize(pSLightPoint->afActualPixelSize);
        point->setFadeThresholdSize(pSLightPoint->sfTranspFalloff);
        //numbers that are going to appear are "experimental"
        point->setDistanceAttenuation(osg::Vec3(0.0001, 0.0005, 0.00000025));

        point->setMinSize(pSLightPoint->sfMinPixelSize);
        point->setMaxSize(pSLightPoint->sfMaxPixelSize);

        stateSet->setAttributeAndModes( point, osg::StateAttribute::ON );
        stateSet->setMode(GL_POINT_SMOOTH, osg::StateAttribute::ON);
        stateSet->setAttributeAndModes(new osg::BlendFunc, osg::StateAttribute::ON);

    }

    // Visit vertices
    addVertices(pBuilder, rec);
    pBuilder->addPrimitive();
}


void ConvertFromFLT::visitLightPoint(osg::Group& osgParent, LightPointRecord* rec)
{
    SLightPoint *pSLightPoint = (SLightPoint*)rec->getData();

    GeoSetBuilder pBuilder;
    DynGeoSet* dgset = pBuilder.getDynGeoSet();
    dgset->setPrimType(osg::PrimitiveSet::POINTS);
    dgset->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    dgset->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

    osgSim::LightPointNode *lpNode = new osgSim::LightPointNode();

    for(int i=0; i < rec->getNumChildren(); i++)
    {
        Record* child = rec->getChild(i);
        if( child->classOpcode() == COMMENT_OP) visitComment(*lpNode, (CommentRecord*)child);
    }

    lpNode->setMinPixelSize( pSLightPoint->sfMinPixelSize);
    lpNode->setMaxPixelSize( pSLightPoint->sfMaxPixelSize);

    addVertices(&pBuilder, rec);

    const DynGeoSet::CoordList& coords = dgset->getCoordList();
    const DynGeoSet::ColorList& colors = dgset->getColorList();
    const DynGeoSet::NormalList& norms = dgset->getNormalList();

    float lobeVert = osg::DegreesToRadians( pSLightPoint->sfLobeVert );
    float lobeHorz = osg::DegreesToRadians( pSLightPoint->sfLobeHoriz );
    float pointRadius =  pSLightPoint->afActualPixelSize * _unitScale;

    for ( unsigned int nl = 0; nl < coords.size(); nl++)
    {
       osg::Vec4 color( 1.0f, 1.0f, 1.0f, 1.0f);
       if( nl < colors.size())  color = colors[nl];

       osgSim::LightPoint lp( true, coords[ nl], color, pSLightPoint->sfIntensityFront, pointRadius);

       if( pSLightPoint->diDirection )
       {
            // calc elevation angles
            osg::Vec3 normal( 1.0f, 0.0f, 0.0f);
            if( nl < norms.size())  normal = norms[nl];

            float elevAngle = osg::PI_2 - acos( normal.z() );
            if( normal.z() < 0.0f) elevAngle = -elevAngle;
            float minElevation = elevAngle - lobeVert/2.0f;
            float maxElevation = elevAngle + lobeVert/2.0f;

            // calc azimuth angles
            osg::Vec2 pNormal( normal.x(), normal.y() );
            float lng = pNormal.normalize();
            float azimAngle = 0.0f;
            if( lng > 0.0000001)
            {
                azimAngle = acos( pNormal.y() );
                if( pNormal.y() > 0.0f ) azimAngle = - azimAngle;

                float minAzimuth = azimAngle - lobeHorz/2.0f;
                float maxAzimuth = azimAngle + lobeHorz/2.0f;

                float fadeRange = 0.0f;
                lp._sector = new osgSim::AzimElevationSector( minAzimuth, maxAzimuth, minElevation, maxElevation, fadeRange);
            }
        }

        lpNode->addLightPoint( lp);
    }

    osgParent.addChild( lpNode);
}


// OpenFlight 15.8 (1580)
// Light point records contain indices into appearance and animation palettes.
//   Need to look up the palette entries to determine how the light points
//   look (and behave).
void ConvertFromFLT::visitLightPointIndex(osg::Group& osgParent, LightPointIndexRecord* rec)
{
    SLightPointIndex *ltPtIdx = (SLightPointIndex*)rec->getData();
    LtPtAppearancePool* appPool = rec->getFltFile()->getLtPtAppearancePool();
    LtPtAppearancePool::PoolLtPtAppearance* ltPtApp = appPool->get( ltPtIdx->iAppearanceIndex );
    if (!ltPtApp)
        // Appearance index out of range
        return;

    // TBD also get ltPtAnim record.
    // LightPointAnimation not currently implemented

    GeoSetBuilder pBuilder;
    DynGeoSet* dgset = pBuilder.getDynGeoSet();
    dgset->setPrimType(osg::PrimitiveSet::POINTS);
    dgset->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    dgset->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

    osgSim::LightPointNode *lpNode = new osgSim::LightPointNode();

    for (int i=0; i < rec->getNumChildren(); i++)
    {
        Record* child = rec->getChild(i);
        if( child->classOpcode() == COMMENT_OP) visitComment(*lpNode, (CommentRecord*)child);
    }

    lpNode->setMinPixelSize( ltPtApp->_sfMinPixelSize );
    lpNode->setMaxPixelSize( ltPtApp->_sfMaxPixelSize );

    addVertices(&pBuilder, rec);

    const DynGeoSet::CoordList& coords = dgset->getCoordList();
    const DynGeoSet::ColorList& colors = dgset->getColorList();
    const DynGeoSet::NormalList& norms = dgset->getNormalList();

    bool directional = false;
    int numInternalLightPoints = 0; // Number of osgSim::LightPoint objects to add per OpenFlight light point vertex
    switch (ltPtApp->_iDirectionality)
    {
    case 0: // Omnidirectional;
        directional = false;
        numInternalLightPoints = 1;
        break;
    case 1: // Unidirectional;
        directional = true;
        numInternalLightPoints = 1;
        break;
    case 2: // Bidirectional;
        directional = true;
        numInternalLightPoints = 2;
        break;
    }

    float lobeVert=0.f, lobeHorz=0.f;
    if ( directional)
    {
        lobeVert = osg::DegreesToRadians( ltPtApp->_sfVLobeAngle );
        lobeHorz = osg::DegreesToRadians( ltPtApp->_sfHLobeAngle );
    }
    float pointRadius =  ltPtApp->_sfActualSize * _unitScale;

    for (unsigned int nl = 0; nl < coords.size(); nl++)
    {
        // Could add 1 or 2 internal light points, 2 for bidirectional
        for (int i=0; i<numInternalLightPoints; i++)
        {
            osg::Vec4 color( 1.0f, 1.0f, 1.0f, 1.0f);
            if ( (i==0) && (nl < colors.size()) )
                color = colors[nl];
            else if (i==1)
            {
                // Get back color
                ColorPool* pColorPool = rec->getFltFile()->getColorPool();
                color = pColorPool->getColor( ltPtApp->_iBackColorIdx );
            }

            osgSim::LightPoint lp( true, coords[nl], color, ltPtApp->_sfIntensity, pointRadius);

            if (directional)
            {
                // calc elevation angles
                osg::Vec3 normal( 1.0f, 0.0f, 0.0f);
                if (nl < norms.size())
                    normal = norms[nl];
                if (i==1)
                    // Negate the normal for the back facing internal light point
                    normal = -normal;

                float elevAngle = osg::PI_2 - acos( normal.z() );
                if (normal.z() < 0.0f)
                    elevAngle = -elevAngle;
                float minElevation = elevAngle - lobeVert/2.0f;
                float maxElevation = elevAngle + lobeVert/2.0f;

                // calc azimuth angles
                osg::Vec2 pNormal( normal.x(), normal.y() );
                float lng = pNormal.normalize();
                float azimAngle = 0.0f;
                if( lng > 0.0000001)
                {
                    azimAngle = acos( pNormal.y() );
                    if (pNormal.x() < 0.0f)
                        azimAngle = -azimAngle;

                    float minAzimuth = azimAngle - lobeHorz/2.0f;
                    float maxAzimuth = azimAngle + lobeHorz/2.0f;

                    float fadeRange = 0.0f;
                    lp._sector = new osgSim::AzimElevationSector( minAzimuth, maxAzimuth, minElevation, maxElevation, fadeRange);
                }
            }

            lpNode->addLightPoint(lp);
        }
    }

    osgParent.addChild(lpNode);
}


void ConvertFromFLT::visitMesh ( osg::Group &parent, GeoSetBuilder *pBuilder, MeshRecord *rec )
{
    DynGeoSet* dgset = pBuilder->getDynGeoSet();
    osg::StateSet *osgStateSet = dgset->getStateSet();
    SFace *pSFace = (SFace *) rec->getData();
    bool bBlend = false;

    // See if it's hidden.
    if ( rec->getFlightVersion() > 13 && flt::hasBits ( pSFace->dwFlags, (uint32) MeshRecord::HIDDEN_BIT ) )
        return;

    // Set the various properties.
    setCullFaceAndWireframe ( pSFace, osgStateSet, dgset );
    setDirectionalLight();
    setLightingAndColorBinding ( rec, pSFace, osgStateSet, dgset );
    setColor ( rec, pSFace, dgset, bBlend );
    setMaterial ( rec, pSFace, osgStateSet, bBlend );
    setTexture ( rec, pSFace, osgStateSet, dgset, bBlend );
    setTransparency ( osgStateSet, bBlend );

    // Add the vertices.
    addVertices ( pBuilder, rec );

    // Add the mesh primitives.
    addMeshPrimitives ( parent, pBuilder, rec );

    // Visit ancillary records
    for(int i=0; i < rec->getNumChildren(); i++)
    {
        Record* child = rec->getChild(i);
        if (!child->isAncillaryRecord())
            break;

    switch (child->getOpcode())
    {
        case MULTI_TEXTURE_OP:
        {
            CERR2 << "MULTI_TEXTURE_OP in visitMesh\n";
            MultiTextureRecord* mtr =
            dynamic_cast<MultiTextureRecord*>(child);
            assert( mtr );
            addMultiTexture( dgset, mtr );
        }
        break;
        default:

        #ifdef _DEBUG

        osg::notify( osg::NOTICE ) << "flt::ConvertFromFLT::visitMesh: "
            << "Unhandled opcode: " << child->getOpcode() << "\n";

        #endif

        break;
    }
    }

}


int ConvertFromFLT::addMeshPrimitives ( osg::Group &parent, GeoSetBuilder *, MeshRecord *rec )
{
    // The count of the mesh primitives added.
    int count = 0;

    // Loop through all the children.
    for ( int i = 0; i < rec->getNumChildren(); ++i )
    {
        // Get the i'th child.
        Record *child = rec->getChild ( i );

        // If it is a mesh primitive...
        if ( MESH_PRIMITIVE_OP == child->getOpcode() )
        {
            // Visit this mesh primitive.
            visitMeshPrimitive ( parent, (MeshPrimitiveRecord *) child );
            ++count;
        }
    }

    // Return the number of mesh primitives added.
    return count;
}


int ConvertFromFLT::visitLocalVertexPool ( GeoSetBuilder *, LocalVertexPoolRecord *rec )
{
    // Make the given instance the current one.
    _currentLocalVertexPool = rec;

    // We didn't add any vertices.
    return 0;
}


void ConvertFromFLT::visitMeshPrimitive ( osg::Group &parent, MeshPrimitiveRecord *mesh )
{
    if ( !mesh )
    {
        osg::notify(osg::NOTICE)<<"Warning:ConvertFromFLT::visitMeshPrimitive () mesh is 0, unable to process."<<std::endl;
        return;
    }

    osg::Geode *geode = new osg::Geode();
    osg::Geometry *geometry = new osg::Geometry();
    LocalVertexPoolRecord *pool = _currentLocalVertexPool;
    
    if (!pool)
    {
        osg::notify(osg::NOTICE)<<"Warning:ConvertFromFLT::visitMeshPrimitive () pool is 0, unable to process."<<std::endl;
        return;
    }
    
    assert ( pool );

    // Set the correct primitive type.
    switch ( mesh->getData()->primitiveType )
    {
    case MeshPrimitiveRecord::TRIANGLE_STRIP:
        geometry->addPrimitiveSet ( new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP,0,mesh->getNumVertices()) );
        break;
    case MeshPrimitiveRecord::TRIANGLE_FAN:
        geometry->addPrimitiveSet ( new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN,0,mesh->getNumVertices()) );
        break;
    case MeshPrimitiveRecord::QUADRILATERAL_STRIP:
        geometry->addPrimitiveSet ( new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP,0,mesh->getNumVertices()) );
        break;
    case MeshPrimitiveRecord::INDEXED_POLYGON:
        geometry->addPrimitiveSet ( new osg::DrawArrays(osg::PrimitiveSet::POLYGON,0,mesh->getNumVertices()) );
        break;
    default:
        assert ( 0 ); // What type is this?
        return;
    }

    // Add the vertex properties.
    setMeshCoordinates ( mesh->getNumVertices(), pool, mesh, geometry );
    setMeshNormals     ( mesh->getNumVertices(), pool, mesh, geometry );
    setMeshColors      ( mesh->getNumVertices(), pool, mesh, geometry );

    // Add the geometry to the geode.
    geode->addDrawable ( geometry );

    // Add the geode to the parent.
    parent.addChild ( geode );
}


uint32 ConvertFromFLT::setMeshCoordinates ( const uint32 &numVerts, const LocalVertexPoolRecord *pool, MeshPrimitiveRecord *mesh, osg::Geometry *geometry )
{
    assert ( pool );
    assert ( mesh );
    assert ( geometry );

    // If there aren't any coordinates...
    if ( false == pool->hasAttribute ( LocalVertexPoolRecord::POSITION ) )
        return 0;

    // Allocate the vertices.
    osg::Vec3Array *coords = new osg::Vec3Array(numVerts);
    if ( NULL == coords )
    {
        assert ( 0 );
        return 0;
    }

    // Declare outside of loop.
    uint32 i ( 0 ), index ( 0 );
    float64 px, py, pz;

    // Loop through all the vertices.
    for ( i = 0; i < numVerts; ++i )
    {
        // Get the i'th index into the vertex pool.
        if ( !mesh->getVertexIndex ( i, index ) )
        {
            assert ( 0 ); // We stepped out of bounds.
            break;
        }

        // Get the coordinate (using "index").
        if ( !pool->getPosition ( index, px, py, pz ) )
        {
            assert ( 0 ); // We stepped out of bounds.
            break;
        }

        // Add the coordinate.
        (*coords)[i].set ( (float) px, (float) py, (float) pz );
    }

    // Set the mesh coordinates.
    geometry->setVertexArray ( coords );

    // Return the number of coordinates added.
    return i;
}


uint32 ConvertFromFLT::setMeshNormals ( const uint32 &numVerts, const LocalVertexPoolRecord *pool, MeshPrimitiveRecord *mesh, osg::Geometry *geometry )
{
    assert ( pool );
    assert ( mesh );
    assert ( geometry );

    // If there aren't any coordinates...
    if ( false == pool->hasAttribute ( LocalVertexPoolRecord::NORMAL ) )
        return 0;

    // Allocate the normals.
    osg::Vec3Array *normals = new osg::Vec3Array(numVerts);
    if ( NULL == normals )
    {
        assert ( 0 );
        return 0;
    }

    // Declare outside of loop.
    uint32 i ( 0 ), index ( 0 );
    float32 x, y, z;

    // Loop through all the vertices.
    for ( i = 0; i < numVerts; ++i )
    {
        // Get the i'th index into the vertex pool.
        if ( !mesh->getVertexIndex ( i, index ) )
        {
            assert ( 0 ); // We stepped out of bounds.
            break;
        }

        // Get the normal (using "index").
        if ( !pool->getNormal ( index, x, y, z ) )
        {
            assert ( 0 ); // We stepped out of bounds.
            break;
        }

        // Add the normal.
        (*normals)[i].set ( x, y, z );
    }

    // Set the mesh normals.
    geometry->setNormalArray( normals );
    geometry->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );

    // Return the number of normals added.
    return i;
}


uint32 ConvertFromFLT::setMeshColors ( const uint32 &numVerts, const LocalVertexPoolRecord *pool, MeshPrimitiveRecord *mesh, osg::Geometry *geometry )
{
    assert ( pool );
    assert ( mesh );
    assert ( geometry );

    // If there aren't any colors...
    if ( false == pool->hasAttribute ( LocalVertexPoolRecord::RGB_COLOR ) )
        return 0;

    // Allocate the normals.
    osg::Vec4Array *colors = new osg::Vec4Array(numVerts);
    if ( NULL == colors )
    {
        assert ( 0 );
        return 0;
    }

    // Declare outside of loop.
    uint32 i ( 0 ), index ( 0 );
    float32 red, green, blue, alpha;

    // Loop through all the vertices.
    for ( i = 0; i < numVerts; ++i )
    {
        // Get the i'th index into the vertex pool.
        if ( false == mesh->getVertexIndex ( i, index ) )
        {
            assert ( 0 ); // We stepped out of bounds.
            break;
        }

        // Get the color (using "index").
        if ( false == pool->getColorRGBA ( index, red, green, blue, alpha ) )
        {
            assert ( 0 ); // We stepped out of bounds.
            break;
        }

        // Add the coordinate.
        (*colors)[i].set ( red, green, blue, alpha );
    }

    // Set the mesh coordinates.
    geometry->setColorArray ( colors );

    // Return the number of colors added.
    return i;
}
