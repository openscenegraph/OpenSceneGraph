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
#include <osg/ShapeDrawable>
#include <osg/Quat>
#include <osg/ProxyNode>
#include <osg/io_utils>

#include <osgSim/MultiSwitch>
#include <osgSim/DOFTransform>
#include <osgSim/LightPointNode>
#include <osgSim/LightPointSystem>
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
#include "LightPointSystemRecord.h"
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
#include "FindExternalModelVisitor.h"

static int dprint = 0 ;
#define DPRINT if(dprint)fprintf

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
    static int numVerts = 0;

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

        case LIGHT_PT_ANIMATION_PALETTE_OP:
            visitLtPtAnimationPalette(osgPrimary, (LtPtAnimationPaletteRecord*)child);
            break;

        case VERTEX_PALETTE_OP:
            visitVertexPalette(osgPrimary, (VertexPaletteRecord*)child);
            break;

        case VERTEX_C_OP:
            visitVertex(osgPrimary, (VertexRecord*)child);
            numVerts++;
            break;

        case VERTEX_CN_OP:
            visitNormalVertex(osgPrimary, (NormalVertexRecord*)child);
            numVerts++;
            break;

        case VERTEX_CNT_OP:
            visitNormalTextureVertex(osgPrimary, (NormalTextureVertexRecord*)child);
            numVerts++;
            break;

        case VERTEX_CT_OP:
            visitTextureVertex(osgPrimary, (TextureVertexRecord*)child);
            numVerts++;
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
    GeoSetBuilder   billboardBuilderPoint;

    // Visit
    for(int i=0; i < rec->getNumChildren(); i++)
    {
        Record* child = rec->getChild(i);

        if (child && child->isPrimaryNode())
        {
            DPRINT(stderr, "**************************************\nvisitPrimaryNode: Got child opcode %d\n", child->getOpcode()) ;
            
            switch (child->getOpcode())
            {
            case MESH_OP:

                if( ((MeshRecord*)child)->getData()->swTemplateTrans == 2)  //Axis type rotate
                    visitMesh(osgParent, &billboardBuilder, (MeshRecord*)child);
                else if( ((MeshRecord*)child)->getData()->swTemplateTrans == 4)  //Point type rotate
                    visitMesh(osgParent, &billboardBuilderPoint, (MeshRecord*)child);
                else
                    visitMesh(osgParent, &geoSetBuilder, (MeshRecord*)child);
                break;

            case FACE_OP:
            {
                FaceRecord* fr = (FaceRecord*)child;
                if( fr->getData()->swTemplateTrans == 2)  //Axis type rotate
                    visitFace(&billboardBuilder, osgParent, fr);
                else if( fr->getData()->swTemplateTrans == 4)  //Point type rotate
                    visitFace(&billboardBuilderPoint, osgParent, fr);
                else
                    visitFace(&geoSetBuilder, osgParent, fr);
            }
            break;
            case LIGHT_POINT_OP:
                visitLightPoint(osgParent, (LightPointRecord*)child);
                break;
            case INDEXED_LIGHT_PT_OP:
                visitLightPointIndex(osgParent, (LightPointIndexRecord*)child);
                break;
            case LIGHT_PT_SYSTEM_OP:
                osgPrim = visitLightPointSystem(osgParent, (LightPointSystemRecord*)child);
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
            case ROAD_SEGMENT_OP:
                // treat road segment record as a group record for now
                osgPrim = visitRoadSegment(osgParent, (GroupRecord*)child);
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

    if( !billboardBuilderPoint.empty() )
    {
        osg::Billboard* billboard = new osg::Billboard;
        billboard->setMode( osg::Billboard::POINT_ROT_WORLD );
        billboardBuilderPoint.createOsgGeoSets(billboard );
        
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

    if ( flightVersion > 13 )
    {
        SColorPalette* pCol = (SColorPalette*)rec->getData();
        int colors = (flightVersion >= 1500) ? 1024 : 512;
    
        // *******************************************************************    
        // GTHACK (Gordon Tomlinson)
        // 
        // Some older files or converted file come through with less than
        // the normal color palettes entries, which was cause an array over
        // over run at times and thus a crash , chnaged the copy code to now  
        // figure out how many color entries there are and just grab those  
        // and fill any extras empty entries with a default white 
        // 
        // *******************************************************************    
        unsigned int datalen =   pCol->RecHeader.length();

        int colorLen = ( datalen -( sizeof(char)*128)) /sizeof(pCol->Colors[0])-1;
        
        //
        // Quick sanity check on the size
        //
        if( colorLen > colors ) 
            colorLen = colors;
        
        int i;
        for (i = 0; i < colorLen ; i++)
        {
            osg::Vec4 color( pCol->Colors[i].get());

            //
            //  Force alpha to one
            // 
            color[3] = 1.0f;     

            pColorPool->addColor(i, color);
        }
        // 
        // Fill any remainder of the palette with white
        // 
        for (i = colorLen; i < colors ; i++)
        {
            osg::Vec4 color( 1.0f, 1.0f, 1.0f, 1.0f );
            pColorPool->addColor( i, color );
        }
                          
    } //  ( flightVersion > 13 ) 
        
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


void ConvertFromFLT::visitLtPtAppearancePalette(osg::Group& /*osgParent*/, LtPtAppearancePaletteRecord* rec)
{
    SLightPointAppearancePalette* ltPtApp = (SLightPointAppearancePalette*)rec->getData();
    LtPtAppearancePool* pool = rec->getFltFile()->getLtPtAppearancePool();
    assert( pool );
    if (ltPtApp && pool)
    {
        LtPtAppearancePool::PoolLtPtAppearance* entry = new LtPtAppearancePool::PoolLtPtAppearance;

        entry->_iBackColorIdx = ltPtApp->backColor;
        entry->_bIntensity = ltPtApp->intensity;
        entry->_sfMinPixelSize = ltPtApp->minPixelSize;
        entry->_sfMaxPixelSize = ltPtApp->maxPixelSize;
        entry->_sfActualSize = ltPtApp->actualSize;
        entry->_iDirectionality = ltPtApp->directionality;
        entry->_sfHLobeAngle = ltPtApp->horizLobeAngle;
        entry->_sfVLobeAngle = ltPtApp->vertLobeAngle;
        entry->_sfLobeRollAngle = ltPtApp->lobeRollAngle;

        pool->add(ltPtApp->index, entry);
    }
}

void ConvertFromFLT::visitLtPtAnimationPalette(osg::Group& /*osgParent*/, LtPtAnimationPaletteRecord* rec)
{
    SLightPointAnimationPalette* ltPtAnim = (SLightPointAnimationPalette*)rec->getData();
    LtPtAnimationPool* pool = rec->getFltFile()->getLtPtAnimationPool();
    assert( pool );
    if (ltPtAnim && pool)
    {
        osg::ref_ptr<LtPtAnimationPool::PoolLtPtAnimation> entry = new LtPtAnimationPool::PoolLtPtAnimation;

        entry->_name = std::string( ltPtAnim->name );

        // Support sequenced animations
        if ( (ltPtAnim->animType == LtPtAnimationPaletteRecord::SEQ_TYPE) &&
             (ltPtAnim->numSequences > 0) )
        {
            osg::ref_ptr<osgSim::BlinkSequence> b = new osgSim::BlinkSequence;
            for (int idx=0; idx<ltPtAnim->numSequences; idx++)
            {
                SLightPointAnimationSequence* seq = rec->sequence( idx );
                osg::Vec4 color( 0.f, 0.f, 0.f, 0.f );
                if (seq->seqState != LtPtAnimationPaletteRecord::SEQ_OFF)
                {
                    // Sequence state is On or Color Change, so set the color to non-black
                    ColorPool* pColorPool = rec->getFltFile()->getColorPool();
                    color = pColorPool->getColor( seq->seqColor );
                }
                b->addPulse( seq->duration, color );
            }
            entry->_blink = b;
        }
        // Support strobe animations
        else if (ltPtAnim->animType == LtPtAnimationPaletteRecord::STROBE_TYPE)
        {
            osg::ref_ptr<osgSim::BlinkSequence> b = new osgSim::BlinkSequence;
            const float duration = .5f / ltPtAnim->period;
            b->addPulse( duration, osg::Vec4( 0.f, 0.f, 0.f, 0.f ) );
            b->addPulse( duration, osg::Vec4( 1.f, 1.f, 1.f, 1.f ) );
            entry->_blink = b;
        }

        pool->add( ltPtAnim->index, entry.get() );
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

    // Check for forward animation (sequence)
    bool forwardAnim = (currentGroup->dwFlags & GroupRecord::FORWARD_ANIM) != 0;

    // For versions prior to 15.8, the swing bit can be set independently
    // of the animation bit.  This implies forward animation (with swing)
    if ((fltVer < 1580) && (currentGroup->dwFlags & GroupRecord::SWING_ANIM))
        forwardAnim = true;
    
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

osg::Group* ConvertFromFLT::visitRoadSegment(osg::Group& osgParent, GroupRecord* rec)
{
    osg::Group* group = new osg::Group;

    group->setName(rec->getData()->szIdent);
    //cout<<"Converted a road segment node of ID "<<group->getName()<<" to group node."<<endl;
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

    // default DOF animation state
    transform->setAnimationOn(rec->getFltFile()->getDefaultDOFAnimationState());

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
        // I don't think we need to create a group node for each mask any more
        // create a osg group node
        //osg::ref_ptr<osg::Group> group = new osg::Group;
        //osgSwitch->addChild( group.get() );

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

    if ( pSObject->dwFlags & 0xFC000000) // some of the 6 defined flag bits are set
    {
        std::string desc("flt object flags: 0x");
        char cflags[33];

        sprintf( cflags, "%X", (unsigned int)pSObject->dwFlags );
        desc = desc + cflags;

        object->getDescriptions().push_back( desc );
    }

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
                dgset->setColorBinding( /*osg::Geometry::BIND_OVERALL*/ osg::Geometry::BIND_PER_PRIMITIVE );
                break;

            case FaceRecord::VERTEX_COLOR:
                // Use vertex colors, not illuminated
                osgStateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
                dgset->setColorBinding( osg::Geometry::BIND_PER_VERTEX );
                break;

            case FaceRecord::FACE_COLOR_LIGHTING:
                // Use face color and vertex normal
                osgStateSet->setMode( GL_LIGHTING, osg::StateAttribute::ON );
                dgset->setColorBinding( /*osg::Geometry::BIND_OVERALL*/ osg::Geometry::BIND_PER_PRIMITIVE);
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

            // Copy the current set of options
            osg::ref_ptr<osgDB::ReaderWriter::Options> versionOptions = 
                (osgDB::ReaderWriter::Options*)rec->getFltFile()->
                getOptions()->clone(osg::CopyOp(osg::CopyOp::SHALLOW_COPY));

            // Create a new set of ReaderWriter options, and prepend the 
            // OpenFlight version option ("FLT_VER") and number to the current
            // option string.  This will inform the ATTR reader what version 
            // of OpenFlight file it's dealing with, so it knows how much data
            // to read.  (Unfortunately, this seems to be the only way to 
            // communicate with the ATTR reader from here).
            char versionStr[30];
            sprintf(versionStr, "FLT_VER %d ", rec->getFlightVersion());
            std::string newString(versionStr);
            newString.append(versionOptions->getOptionString());
            versionOptions->setOptionString(newString);

            // Finally, get the texture from the texture pool
            flt::AttrData *textureAttrData = 
                pTexturePool->getTexture(nIndex, versionOptions.get());

            osg::ref_ptr<osg::StateSet> textureStateSet;
            if (textureAttrData)
              textureStateSet = textureAttrData->stateset;

            if (textureStateSet.valid())
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
                 detailTextureAttrData = 
                     pTexturePool->getTexture(nIndex2,versionOptions.get());
                 if (detailTextureAttrData && detailTextureAttrData->stateset.valid()) {
                     osg::Texture2D *detTexture = dynamic_cast<osg::Texture2D*>(detailTextureAttrData->stateset->getTextureAttribute( 0, osg::StateAttribute::TEXTURE));
                     textureStateSet->setTextureAttributeAndModes(1,detTexture,osg::StateAttribute::ON);                    
                     osg::TexEnvCombine *tec1 = new osg::TexEnvCombine;
                     float scale = (detailTextureAttrData->modulateDetail==0)?2.0f:4.0f;                     
                     tec1->setScale_RGB(scale);
                     tec1->setScale_Alpha(scale);
                     textureStateSet->setTextureAttribute( 1, tec1,osg::StateAttribute::ON );                                            
                 }                 
                }

                // If a detail texture structure exists, set the texture
                // coordinate scalars on the current DynGeoSet, so the correct
                // detail texture coordinates get generated later.
                if (pSFace->iDetailTexturePattern != -1 && 
                    detailTextureAttrData && 
                    detailTextureAttrData->stateset.valid())
                {
                   // Set the texture coordinate scalars
                   dgset->setDetailTexCoords(detailTextureAttrData->txDetail_m,
                                             detailTextureAttrData->txDetail_n);

                   // Make sure detail texturing is on
                   dgset->enableDetailTexture();
                }
                else 
                {
                   // Make sure detail texturing is off
                   dgset->disableDetailTexture();
                }

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

    // Copy the current set of options
    osg::ref_ptr<osgDB::ReaderWriter::Options> versionOptions =
        (osgDB::ReaderWriter::Options*)mtr->getFltFile()->
        getOptions()->clone(osg::CopyOp(osg::CopyOp::SHALLOW_COPY));

    // Create a new set of ReaderWriter options, and prepend the
    // OpenFlight version option ("FLT_VER") and number to the current
    // option string.  This will inform the ATTR reader what version
    // of OpenFlight file it's dealing with, so it knows how much data
    // to read.  (Unfortunately, this seems to be the only way to
    // communicate with the ATTR reader from here).
    char versionStr[30];
    sprintf(versionStr, "FLT_VER %d ", mtr->getFlightVersion());
    std::string newString(versionStr);
    newString.append(versionOptions->getOptionString());
    versionOptions->setOptionString(newString);

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
            int mtDataTexture = mt->data[l].texture;
            //int mtDataEffect  = mt->data[l].effect;
            //int mtDataMapping = mt->data[l].mapping;
            //int mtDataData    = mt->data[l].data;
            mt->data[l].endian();

            TexturePool* pTexturePool = mtr->getFltFile()->getTexturePool();
            
            assert( pTexturePool );
            if (!pTexturePool)
            {
                osg::notify(osg::WARN)<<"ConvertFromFLT::addMultiTexture(DynGeoSet*, MultiTextureRecord*) pTexturePool invalid."<<std::endl;
                return;
            }

            // Get the texture attribute data from the texture pool
            flt::AttrData *textureAttrData = 
                dynamic_cast<flt::AttrData *>(pTexturePool->
                    getTexture(mtDataTexture,versionOptions.get()));

            CERR << "pTexturePool->getTexture(mtDataTexture): " << pTexturePool->getTexture(mtDataTexture,versionOptions.get()) << "\n";
            if (!textureAttrData)
            {
                CERR << "unable to set up multi-texture layer." << std::endl;
                return;
            }

            // Get the texture state set from the attribute data structure
            osg::ref_ptr<osg::StateSet> textureStateSet = textureAttrData->stateset;
            CERR << "textureStateSet: " << textureStateSet.get() << "\n";

            if (!textureStateSet.valid())
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

            texture_stateset->setTextureAttributeAndModes(i, texture,osg::StateAttribute::ON);
   
            osg::StateAttribute* texenv_0 = textureStateSet->getTextureAttribute( 0, osg::StateAttribute::TEXENV );
            if (texenv_0)
            {
                texture_stateset->setTextureAttribute( i, texenv_0);
            }
            else
            {
                osg::TexEnv* osgTexEnv = new osg::TexEnv;
                osgTexEnv->setMode(osg::TexEnv::MODULATE);
                texture_stateset->setTextureAttribute( i, osgTexEnv );
            }
            

            CERR << "geom: " << geom << "\n";
            CERR << ", referenceCount: "
                 << geom->referenceCount() << "\n";

            // Get the state set from the current geometry
            osg::StateSet* geom_stateset = geom->getStateSet();
            
            CERR << "geom_stateset: " << geom_stateset << "\n";
                
            // See if we need to merge or set the texture state set on the
            // geometry
            if ( geom_stateset )
            {
                geom_stateset->merge( *texture_stateset );
                CERR << "Merging layer " << i << "\n";
            } else {
                geom->setStateSet( texture_stateset );
                CERR << "Setting layer " << i << "\n";
            }

            // Set the texture binding on the current texture unit to
            // per-vertex
            dgset->setTextureBinding(i, osg::Geometry::BIND_PER_VERTEX);

            l++;
        }
    }
}

void
ConvertFromFLT::addUVList( DynGeoSet* dgset, UVListRecord* uvr )
{
    if (!dgset || !uvr || !uvr->isAncillaryRecord())
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
            CERR << "Has layer " << i << "\n";

            // Assume we are working with vertex lists for now
            for ( int v = l*num_coords; v < (l+1)*num_coords; v++ )
            {
                uvl->coords.vertex[v].endian();
                CERR << "( u: " << uvl->coords.vertex[v].coords[1] << ", "
                     << "v: " << uvl->coords.vertex[v].coords[0] << ")\n";

                /// FIXME: should be (x,y) instead of (y,x) - ENDIAN problem???
                // Add the texture coordinates to the current DynGeoSet
                dgset->addTCoord(i, 
                                 osg::Vec2(uvl->coords.vertex[v].coords[1],
                                           uvl->coords.vertex[v].coords[0]));
            }

            l++;
        }
    }
}

void ConvertFromFLT::visitFace(GeoSetBuilder* pBuilder, osg::Group& osgParent, FaceRecord* rec)
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
    addVertices(pBuilder, osgParent, rec);

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
            addMultiTexture( dgset, mtr );
            
            // addMultiTexture( pBuilder->getDynGeoSet(), mtr );
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

    // Add face to builder pool
    pBuilder->addPrimitive();

    // Look for subfaces
    {
        _nSubfaceLevel++;
        int n;
        for(n=0; n<rec->getNumChildren(); n++)
        {
            Record* child = rec->getChild(n);

            if (child && child->isOfType(FACE_OP))
                visitFace(pBuilder, osgParent, (FaceRecord*)child);
        }
        _nSubfaceLevel--;
    }
}

/* C.Holtz:  These global variables are to support the REPLICATE ancillary record
   used for lightpoint strings in 15.7 (it really should support replication of any
   node, but I really only needed lightpoint strings and it's such a hack, I don't
   want to propagate it anywhere is doesn't really need to be :) */
static osg::Matrix theMatrix ;
static osg::Matrix theGeneralMatrix ;
static osg::Vec3 from, delta ;
static int num_replicate ;
static int got_gm, got_m, got_t, got_replicate ;

// Return number of vertices added to builder.
int ConvertFromFLT::addVertices(GeoSetBuilder* pBuilder, osg::Group& osgParent, PrimNodeRecord* primRec)
{
    int i;
    int vertices=0;
    DynGeoSet* dgset = pBuilder->getDynGeoSet();

    /* Clear the replicate stuff each time through */
    got_gm = got_m = got_t = got_replicate = 0 ;

    DPRINT(stderr, ">>> addVerticies...%d children\n", primRec->getNumChildren()) ;
    for(i=0; i < primRec->getNumChildren(); i++)
    {
        Record* child = primRec->getChild(i);
        if (child == NULL) break;

        DPRINT(stderr, "     child opcode = %d\n", child->getOpcode()) ;
        
        switch (child->getOpcode())
        {
        case VERTEX_LIST_OP:
            vertices += visitVertexList(pBuilder, (VertexListRecord*)child);
            break;
        
        case MORPH_VERTEX_LIST_OP:
            vertices += visitMorphVertexList(pBuilder, (MorphVertexListRecord*)child);
            break;
        
        case LOCAL_VERTEX_POOL_OP:
            vertices += visitLocalVertexPool(pBuilder, (LocalVertexPoolRecord *)child);
            break;

        case TRANSLATE_OP:
           if (1) {
           // This will be for replicated verticies
           STranslate *pSTranslate = ((TranslateRecord *)child)->getData() ;
           // scale position.
           from.set(pSTranslate->From[0],pSTranslate->From[1],pSTranslate->From[2]) ;
           from *= _unitScale ;
           delta.set(pSTranslate->Delta[0],pSTranslate->Delta[1],pSTranslate->Delta[2]) ;
           delta *= _unitScale ;
           DPRINT(stderr, "   ** addVerticies: Got Translate: F=%lf, %lf, %lf / D=%lf, %lf, %lf\n",
              from[0], from[1], from[2], delta[0], delta[1], delta[2]) ;
           got_t = 1 ;
           }
           break ;
        
        case MATRIX_OP:
           {
           // This will be for replicated verticies
           SMatrix *pSMatrix = ((MatrixRecord *)child)->getData() ;
           for(int i=0;i<4;++i)
           {
               for(int j=0;j<4;++j)
               {
                   theMatrix(i,j) = pSMatrix->sfMat[i][j];
               }
           }
           // scale position.
           osg::Vec3 pos = theMatrix.getTrans();
           theMatrix *= osg::Matrix::translate(-pos);
           pos *= (float)_unitScale;
           theMatrix *= osg::Matrix::translate(pos);
           if(dprint)std::cout << "   ** addVerticies: Got Matrix: " << theMatrix << std::endl ;
           got_m = 1 ;
           }
           break ;
        
        case GENERAL_MATRIX_OP:
           {
           // This will be for replicated verticies
           SGeneralMatrix *pSMatrix = ((GeneralMatrixRecord *)child)->getData() ;
           for(int i=0;i<4;++i)
           {
               for(int j=0;j<4;++j)
               {
                   theGeneralMatrix(i,j) = pSMatrix->sfMat[i][j];
               }
           }
           // scale position.
           osg::Vec3 pos = theGeneralMatrix.getTrans();
           theGeneralMatrix *= osg::Matrix::translate(-pos);
           pos *= (float)_unitScale;
           theGeneralMatrix *= osg::Matrix::translate(pos);
           if(dprint)std::cout << "   ** addVerticies: Got GeneralMatrix: " << theGeneralMatrix << std::endl ;
           got_gm = 1 ;
           }
           break ;
        
        case REPLICATE_OP:
           {
           // This will be for replicated verticies
           SReplicate *pSReplicate = (SReplicate *)(child->getData())  ;
#if 1
           int16 temp = pSReplicate->iNumber;
           ENDIAN(temp);
           num_replicate = temp ;
#else
           ENDIAN(pSReplicate->iNumber) ;
           num_replicate = pSReplicate->iNumber ;
#endif
           DPRINT(stderr, "   ** addVerticies: Got Replicate: %d times\n", num_replicate) ;
           got_replicate = 1 ;
           }
           break ;
        
        case LIGHT_POINT_OP:
           {
            /* Apparently, lightpoints are allowed to be clildren
               of faces in older versions (<15.4?) */
           DPRINT(stderr, "   ** addVerticies: Got LIGHT_POINT_OP\n") ;
           visitLightPoint(osgParent, (LightPointRecord*)child) ;
           }
           break ;
        
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

    DPRINT(stderr, ">>> visitVertexList...%d vertices\n", vertices) ;
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


int ConvertFromFLT::visitMorphVertexList(GeoSetBuilder* pBuilder, MorphVertexListRecord* rec)
{
    DynGeoSet* dgset = pBuilder->getDynGeoSet();
    int vertices = rec->numberOfVertices();

    DPRINT(stderr, ">>> visitVertexList...%d vertices\n", vertices) ;
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
    
    int i ;
    
    DPRINT(stderr, ">>> addVertex...") ;
    
    switch(rec->getOpcode())
    {
    case VERTEX_C_OP:
        DPRINT(stderr, "VERTEX_C_OP\n") ;
        {
            SVertex* pVert = (SVertex*)rec->getData();
            osg::Vec3 coord(pVert->Coord.x(), pVert->Coord.y(), pVert->Coord.z());
            coord *= (float)_unitScale;
            dgset->addCoord(coord);
            if ( got_replicate ) {
               /* Handle vertex replication */
               DPRINT(stderr, "      ### addVertex: Replicating (%f,%f,%f) %d times...\n", 
                  coord[0], coord[1], coord[2], num_replicate) ;
               for ( i = 0 ; i < num_replicate ; i++ ) {
                  if ( got_t ) {
                     coord += delta ;
                     DPRINT(stderr, "          >> Replicated vertex as (%f,%f,%f)\n", 
                        coord[0], coord[1], coord[2]) ;
                     dgset->addCoord(coord) ;
                  } else {
                     /* If we didn't get a translate record, try to get the delta from the matrix */
                     delta = theMatrix.getTrans() ;
                     coord += delta ;
                     DPRINT(stderr, "          >> Replicated vertex as (%f,%f,%f)\n", 
                        coord[0], coord[1], coord[2]) ;
                     dgset->addCoord(coord) ;
                  }
               }
            }
            if (dgset->getColorBinding() == osg::Geometry::BIND_PER_VERTEX)
                ADD_VERTEX_COLOR(dgset, pVert, rec->getFltFile()->getColorPool())

        }
        break;

    case VERTEX_CN_OP:
        DPRINT(stderr, "VERTEX_CN_OP\n") ;
        {
            SNormalVertex* pVert = (SNormalVertex*)rec->getData();
            osg::Vec3 coord(pVert->Coord.x(), pVert->Coord.y(), pVert->Coord.z());
            coord *= (float)_unitScale;
            dgset->addCoord(coord);
            if ( got_replicate ) {
               /* Handle vertex replication */
               DPRINT(stderr, "      ### addVertex: Replicating (%f,%f,%f) %d times...\n", 
                  coord[0], coord[1], coord[2], num_replicate) ;
               for ( i = 0 ; i < num_replicate ; i++ ) {
                  if ( got_t ) {
                     coord += delta ;
                     DPRINT(stderr, "          >> Replicated vertex as (%f,%f,%f)\n", 
                        coord[0], coord[1], coord[2]) ;
                     dgset->addCoord(coord) ;
                  } else {
                     /* If we didn't get a translate record, try to get the delta from the matrix */
                     delta = theMatrix.getTrans() ;
                     coord += delta ;
                     DPRINT(stderr, "          >> Replicated vertex as (%f,%f,%f)\n", 
                        coord[0], coord[1], coord[2]) ;
                     dgset->addCoord(coord) ;
                  }
               }
            }
            if (dgset->getNormalBinding() == osg::Geometry::BIND_PER_VERTEX)
                ADD_NORMAL(dgset, pVert)
            if (dgset->getColorBinding() == osg::Geometry::BIND_PER_VERTEX)
                ADD_VERTEX_COLOR(dgset, pVert, rec->getFltFile()->getColorPool())
        }
        break;

    case VERTEX_CNT_OP:
        DPRINT(stderr, "VERTEX_CNT_OP\n") ;
        {
            SNormalTextureVertex* pVert = (SNormalTextureVertex*)rec->getData();
            osg::Vec3 coord(pVert->Coord.x(), pVert->Coord.y(), pVert->Coord.z());
            coord *= (float)_unitScale;
            dgset->addCoord(coord);
            if ( got_replicate ) {
               /* Handle vertex replication */
               DPRINT(stderr, "      ### addVertex: Replicating (%f,%f,%f) %d times...\n", 
                  coord[0], coord[1], coord[2], num_replicate) ;
               for ( i = 0 ; i < num_replicate ; i++ ) {
                  if ( got_t ) {
                     coord += delta ;
                     DPRINT(stderr, "          >> Replicated vertex as (%f,%f,%f)\n", 
                        coord[0], coord[1], coord[2]) ;
                     dgset->addCoord(coord) ;
                  } else {
                     /* If we didn't get a translate record, try to get the delta from the matrix */
                     delta = theMatrix.getTrans() ;
                     coord += delta ;
                     DPRINT(stderr, "          >> Replicated vertex as (%f,%f,%f)\n", 
                        coord[0], coord[1], coord[2]) ;
                     dgset->addCoord(coord) ;
                  }
               }
            }
            if (dgset->getNormalBinding() == osg::Geometry::BIND_PER_VERTEX)
                ADD_NORMAL(dgset, pVert)
            if (dgset->getTextureBinding() == osg::Geometry::BIND_PER_VERTEX)
                ADD_TCOORD(dgset, pVert)
            if (dgset->getColorBinding() == osg::Geometry::BIND_PER_VERTEX)
                ADD_VERTEX_COLOR(dgset, pVert, rec->getFltFile()->getColorPool())
        }
        break;

    case VERTEX_CT_OP:
        DPRINT(stderr, "VERTEX_CT_OP\n") ;
        {
            STextureVertex* pVert = (STextureVertex*)rec->getData();
            osg::Vec3 coord(pVert->Coord.x(), pVert->Coord.y(), pVert->Coord.z());
            coord *= (float)_unitScale;
            dgset->addCoord(coord);
            if ( got_replicate ) {
               /* Handle vertex replication */
               DPRINT(stderr, "      ### addVertex: Replicating (%f,%f,%f) %d times...\n", 
                  coord[0], coord[1], coord[2], num_replicate) ;
               for ( i = 0 ; i < num_replicate ; i++ ) {
                  if ( got_t ) {
                     coord += delta ;
                     DPRINT(stderr, "          >> Replicated vertex as (%f,%f,%f)\n", 
                        coord[0], coord[1], coord[2]) ;
                     dgset->addCoord(coord) ;
                  } else {
                     /* If we didn't get a translate record, try to get the delta from the matrix */
                     delta = theMatrix.getTrans() ;
                     coord += delta ;
                     DPRINT(stderr, "          >> Replicated vertex as (%f,%f,%f)\n", 
                        coord[0], coord[1], coord[2]) ;
                     dgset->addCoord(coord) ;
                  }
               }
            }
            if (dgset->getTextureBinding() == osg::Geometry::BIND_PER_VERTEX)
                ADD_TCOORD(dgset, pVert)
            if (dgset->getColorBinding() == osg::Geometry::BIND_PER_VERTEX)
                ADD_VERTEX_COLOR(dgset, pVert, rec->getFltFile()->getColorPool())
        }
        break;

    case OLD_VERTEX_OP:
        DPRINT(stderr, "OLD_VERTEX_OP\n") ;
        {
            SOldVertex* pVert = (SOldVertex*)rec->getData();
            osg::Vec3 coord(pVert->v[0], pVert->v[1], pVert->v[2]);
            coord *= (float)_unitScale;
            dgset->addCoord(coord);
            if ( got_replicate ) {
               /* Handle vertex replication */
               DPRINT(stderr, "      ### addVertex: Replicating (%f,%f,%f) %d times...\n", 
                  coord[0], coord[1], coord[2], num_replicate) ;
               for ( i = 0 ; i < num_replicate ; i++ ) {
                  if ( got_t ) {
                     coord += delta ;
                     DPRINT(stderr, "          >> Replicated vertex as (%f,%f,%f)\n", 
                        coord[0], coord[1], coord[2]) ;
                     dgset->addCoord(coord) ;
                  } else {
                     /* If we didn't get a translate record, try to get the delta from the matrix */
                     delta = theMatrix.getTrans() ;
                     coord += delta ;
                     DPRINT(stderr, "          >> Replicated vertex as (%f,%f,%f)\n", 
                        coord[0], coord[1], coord[2]) ;
                     dgset->addCoord(coord) ;
                  }
               }
            }
            if ((dgset->getTextureBinding() == osg::Geometry::BIND_PER_VERTEX)
            &&  (rec->getSize() >= sizeof(SOldVertex)))
                ADD_OLD_TCOORD(dgset, pVert)
        }
        break;

    case OLD_VERTEX_COLOR_OP:
        DPRINT(stderr, "OLD_VERTEX_COLOR_OP\n") ;
        {
            SOldVertexColor* pVert = (SOldVertexColor*)rec->getData();
            osg::Vec3 coord(pVert->v[0], pVert->v[1], pVert->v[2]);
            coord *= (float)_unitScale;
            dgset->addCoord(coord);
            if ( got_replicate ) {
               /* Handle vertex replication */
               DPRINT(stderr, "      ### addVertex: Replicating (%f,%f,%f) %d times...\n", 
                  coord[0], coord[1], coord[2], num_replicate) ;
               for ( i = 0 ; i < num_replicate ; i++ ) {
                  if ( got_t ) {
                     coord += delta ;
                     DPRINT(stderr, "          >> Replicated vertex as (%f,%f,%f)\n", 
                        coord[0], coord[1], coord[2]) ;
                     dgset->addCoord(coord) ;
                  } else {
                     /* If we didn't get a translate record, try to get the delta from the matrix */
                     delta = theMatrix.getTrans() ;
                     coord += delta ;
                     DPRINT(stderr, "          >> Replicated vertex as (%f,%f,%f)\n", 
                        coord[0], coord[1], coord[2]) ;
                     dgset->addCoord(coord) ;
                  }
               }
            }
            if (dgset->getColorBinding() == osg::Geometry::BIND_PER_VERTEX)
                ADD_OLD_COLOR(dgset, pVert, rec->getFltFile()->getColorPool())
            if ((dgset->getTextureBinding() == osg::Geometry::BIND_PER_VERTEX)
            &&  (rec->getSize() >= sizeof(SOldVertexColor)))
                ADD_OLD_TCOORD(dgset, pVert)
        }
        break;

    case OLD_VERTEX_COLOR_NORMAL_OP:
        DPRINT(stderr, "OLD_VERTEX_COLOR_NORMAL_OP\n") ;
        {
            SOldVertexColorNormal* pVert = (SOldVertexColorNormal*)rec->getData();
            osg::Vec3 coord(pVert->v[0], pVert->v[1], pVert->v[2]);
            coord *= (float)_unitScale;
            dgset->addCoord(coord);
            if ( got_replicate ) {
               /* Handle vertex replication */
               DPRINT(stderr, "      ### addVertex: Replicating (%f,%f,%f) %d times...\n", 
                  coord[0], coord[1], coord[2], num_replicate) ;
               for ( i = 0 ; i < num_replicate ; i++ ) {
                  if ( got_t ) {
                     coord += delta ;
                     DPRINT(stderr, "          >> Replicated vertex as (%f,%f,%f)\n", 
                        coord[0], coord[1], coord[2]) ;
                     dgset->addCoord(coord) ;
                  } else {
                     /* If we didn't get a translate record, try to get the delta from the matrix */
                     delta = theMatrix.getTrans() ;
                     coord += delta ;
                     DPRINT(stderr, "          >> Replicated vertex as (%f,%f,%f)\n", 
                        coord[0], coord[1], coord[2]) ;
                     dgset->addCoord(coord) ;
                  }
               }
            }
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
        DPRINT(stderr, "*** UNKNOWN (%d)***\n", rec->getOpcode()) ;
        return 0;
    }
    
    /* Clear out the replicate stuff so that it doesn't get reused if we
       return to this function from somewhere other than addVerticies() */
    got_gm = got_m = got_t = got_replicate = 0 ;

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
    FltFile* pFile = rec->getExternal();
    osg::Group* external = NULL;
    if (pFile)
    {
        //Path for Nested external references
        osgDB::ReaderWriter::Options *options = pFile->getOptions();

        if(options && (options->getObjectCacheHint() & osgDB::ReaderWriter::Options::CACHE_NODES)!=0)
        {
            external = dynamic_cast<osg::Group*> (osgDB::Registry::instance()->getFromObjectCache(rec->getFilename()));
            if(external)
            {
                osg::ProxyNode *proxynode = new osg::ProxyNode;
                proxynode->setCenterMode(osg::ProxyNode::USE_BOUNDING_SPHERE_CENTER);
                proxynode->addChild(external, rec->getFilename());
                osg::Group *tempParent = visitAncillary(osgParent, *proxynode, rec);
                tempParent->addChild(proxynode);
                return external;
            }
        }

        osgDB::FilePathList& fpl = options->getDatabasePathList();
        const std::string& filePath = osgDB::getFilePath(rec->getFilename());
        std::string pushAndPopPath;
        //If absolute path
        if( (filePath.length()>0 && filePath.find_first_of("/\\")==0) ||
            (filePath.length()>2 && filePath.substr(1,1)==":" && filePath.find_first_of("/\\")==2) )
        {
            pushAndPopPath = filePath;
        }
        else
        {
            pushAndPopPath = (fpl.empty() | fpl.back().empty() ? "." : fpl.back()) + "/" + filePath;
        }
        fpl.push_back(pushAndPopPath);
        

        pFile->setDesiredUnits( rec->getFltFile()->getDesiredUnits() );
        external = pFile->convert();
        if (external)
        {
          // In the situation in which only one model is required from an
          // externally referenced file, it would be more efficient to only
          // convert that one model from the FltFile records.  (This would be
          // the preferred method if this loader is rewritten.)  However, 
          // since this situation is fairly rare and it is currently much 
          // more straight forward to work with the OSG structure, we will 
          // just pull out the part of the OSG tree that is needed at this 
          // point.

          // If a model name was specified, find and add the node with
          // that name.  Otherwise, add the entire tree.

          std::string modelName = rec->getModelName();
          if ( modelName.empty() )
          {
               // Add the entire externally referenced file
                osg::ProxyNode *proxynode = new osg::ProxyNode;
                proxynode->setCenterMode(osg::ProxyNode::USE_BOUNDING_SPHERE_CENTER);
                proxynode->addChild(external, rec->getFilename());
                osg::Group *tempParent = visitAncillary(osgParent, *proxynode, rec);
                tempParent->addChild(proxynode);

                if(options && (options->getObjectCacheHint() & osgDB::ReaderWriter::Options::CACHE_NODES)!=0)
                    osgDB::Registry::instance()->addEntryToObjectCache(rec->getFilename(), external);
          }
          else
          {
            // Find the specified model
            FindExternalModelVisitor findExternalModelVisitor;
            findExternalModelVisitor.setModelName( modelName );
            external->accept( findExternalModelVisitor );
            osg::Node *model = findExternalModelVisitor.getModel();
            if (model)
            {
                osg::ProxyNode *proxynode = new osg::ProxyNode;
                proxynode->setCenterMode(osg::ProxyNode::USE_BOUNDING_SPHERE_CENTER);
                proxynode->addChild(model, rec->getFilename());
                osg::Group *tempParent = visitAncillary(osgParent, *proxynode, rec);
                tempParent->addChild(proxynode);

                if(options && (options->getObjectCacheHint() & osgDB::ReaderWriter::Options::CACHE_NODES)!=0)
                    osgDB::Registry::instance()->addEntryToObjectCache(rec->getFilename(), model);
            }
            else
            {
              osg::notify(osg::WARN) << "In ConvertFromFLT::visitExternal,"
                        << " the requested model " << modelName 
                        << " was not found in external file " 
                        << rec->getFilename() << std::endl;
            }
          }
        }

        fpl.pop_back();
    }
    return external;
}

void ConvertFromFLT::visitLightPoint(GeoSetBuilder* pBuilder,osg::Group& osgParent, LightPointRecord* rec)
{
    SLightPoint *pSLightPoint = (SLightPoint*)rec->getData();
    if (!pSLightPoint) return;

    DynGeoSet* dgset = pBuilder->getDynGeoSet();
    osg::StateSet* stateSet = dgset->getStateSet();

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
    addVertices(pBuilder, osgParent, rec);
    pBuilder->addPrimitive();
}


void ConvertFromFLT::visitLightPoint(osg::Group& osgParent, LightPointRecord* rec)
{
    SLightPoint *pSLightPoint = (SLightPoint*)rec->getData();
    if (!pSLightPoint) return;

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

    DPRINT(stderr, "visitLightPoint: visiting node '%s'...(%d children)\n", pSLightPoint->szIdent, rec->getNumChildren()) ;
    lpNode->setName(pSLightPoint->szIdent) ;
    
    lpNode->setMinPixelSize( pSLightPoint->sfMinPixelSize);
    lpNode->setMaxPixelSize( pSLightPoint->sfMaxPixelSize);
    DPRINT(stderr, "   MinPixelSize = %f\n", pSLightPoint->sfMinPixelSize) ;
    DPRINT(stderr, "   MaxPixelSize = %f\n", pSLightPoint->sfMaxPixelSize) ;

    addVertices(&pBuilder, osgParent, rec);

    const DynGeoSet::CoordList& coords = dgset->getCoordList();
    const DynGeoSet::ColorList& colors = dgset->getColorList();
    const DynGeoSet::NormalList& norms = dgset->getNormalList();

    DPRINT(stderr, "   Num Coords=%d, Num Colors=%d, Num Norms=%d\n",
        static_cast<int>(coords.size()),
        static_cast<int>(colors.size()),
        static_cast<int>(norms.size())) ;
    
    bool directional = false;
    int numInternalLightPoints = 0; // Number of osgSim::LightPoint objects to add per OpenFlight light point vertex
    switch (pSLightPoint->diDirection)
    {
    case 0: // Omnidirectional;
        DPRINT(stderr, "   OMNIDIRECTIONAL\n") ;
        directional = false;
        numInternalLightPoints = 1;
        break;
    case 1: // Unidirectional;
        DPRINT(stderr, "   UNIDIRECTIONAL\n") ;
        directional = true;
        numInternalLightPoints = 1;
        break;
    case 2: // Bidirectional;
        DPRINT(stderr, "   BIDIRECTIONAL\n") ;
        directional = true;
        numInternalLightPoints = 2;
        break;
    }

    float lobeVert=0.f, lobeHorz=0.f, lobeRoll=0.f;
    if ( directional)
    {
        lobeVert = osg::DegreesToRadians( pSLightPoint->sfLobeVert );
        lobeHorz = osg::DegreesToRadians( pSLightPoint->sfLobeHoriz );
        lobeRoll = osg::DegreesToRadians( pSLightPoint->sfLobeRoll );
    }
    float pointRadius =  pSLightPoint->afActualPixelSize * _unitScale;

    DPRINT(stderr, "   Vertical Lobe Angle = %f\n", osg::RadiansToDegrees(lobeVert)) ;
    DPRINT(stderr, "   Horizontal Lobe Angle = %f\n", osg::RadiansToDegrees(lobeHorz)) ;
    DPRINT(stderr, "   Lobe Roll Angle = %f\n", osg::RadiansToDegrees(lobeRoll)) ;
    DPRINT(stderr, "   Point Radius = %f\n", pointRadius) ;
    
    /* From my experience during all this testing, I think it's safe to assume that
       each light point in a single light point node should share the same color
       and normal.  Even if multiple normals are found, they seem to be wrong for some
       reason */
    osg::Vec4 color( 1.0f, 1.0f, 1.0f, 1.0f);
    osg::Vec3 normal( 1.0f, 0.0f, 0.0f);
    
    for ( unsigned int nl = 0; nl < coords.size(); nl++)
    {
       //if( nl < colors.size())  color = colors[nl];
       if( colors.size()>0)  color = colors[0];
       DPRINT(stderr, "   Color = %f, %f, %f, %f\n", color.x(), color.y(), color.z(), color.w()) ;

       osgSim::LightPoint lp( true, coords[ nl], color, pSLightPoint->sfIntensityFront, pointRadius);

       if( pSLightPoint->diDirection )
       {
            DPRINT(stderr, "   LP is directional...\n") ;
            if ( !pSLightPoint->diDirectionalMode ) {
               DPRINT(stderr, "%%%%%%%% WARNING: diDirection is set, but diDirectionalMode is off!!!\n") ;
            }
            
            // calc elevation angles
            //if( nl < norms.size())  normal = norms[nl];
            if( norms.size()>0)  normal = norms[0];
            DPRINT(stderr, "   Normal = %f, %f, %f\n", normal.x(), normal.y(), normal.z()) ;
            
            // Verify normal.  If normal is 0,0,0, then LP isn't really directional
            if ( (fabsf(normal.x()) < 0.0001) && (fabsf(normal.y()) < 0.0001) && (fabsf(normal.z()) < 0.0001) ) {
               DPRINT(stderr, "%%%%%%%% WARNING: diDirection is set, but normal is not set!!!\n") ;
               DPRINT(stderr, "   ADDING LIGHTPOINT\n") ;
               lpNode->addLightPoint( lp);
               continue ;
            }
            if ( normal.isNaN() ) {
               DPRINT(stderr, "%%%%%%%% WARNING: diDirection is set, but normal is NaN!!!\n") ;
               DPRINT(stderr, "   ADDING LIGHTPOINT\n") ;
               lpNode->addLightPoint( lp);
               continue ;
            }
            

            lp._sector = new osgSim::DirectionalSector( normal, lobeHorz, lobeVert, lobeRoll);
            
            if( pSLightPoint->diDirection == 2 )
            {
                 DPRINT(stderr, "   ** LP is BIdirectional...\n") ;
                 
                 // pSLightPoint->dwBackColor is not a color, it is handle
                 // Get the color from the ColorPool
                 // Nick
                 // osg::Vec4 backcolor = pSLightPoint->dwBackColor.get() ;
                 ColorPool* pColorPool = rec->getFltFile()->getColorPool();
                 osg::Vec4 backcolor = pColorPool->getColor(pSLightPoint->dwBackColor);

                 if ( backcolor.w() == 0.0 ) backcolor[3] = 1.0 ;
                 osgSim::LightPoint lp2( true, coords[ nl], backcolor, 1.0f, pointRadius);
                 DPRINT(stderr, "   Backface Color = %f, %f, %f, %f\n", backcolor.x(), backcolor.y(), backcolor.z(), backcolor.w()) ;
                 // calc elevation angles
                 osg::Vec3 backnormal = - normal ;
                 DPRINT(stderr, "   Normal = %f, %f, %f\n", backnormal.x(), backnormal.y(), backnormal.z()) ;
                 
                 lp2._sector = new osgSim::DirectionalSector( backnormal, lobeHorz, lobeVert, lobeRoll);
                 
                 DPRINT(stderr, "   ADDING BACKFACING LIGHTPOINT\n") ;
                 lpNode->addLightPoint(lp2);
               }
            }

        DPRINT(stderr, "   ADDING LIGHTPOINT\n") ;
        lpNode->addLightPoint( lp);
        
    }

    DPRINT (stderr, "lpNode has %d children\n", lpNode->getNumLightPoints()) ;
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
        return; // Appearance index out of range
    LtPtAnimationPool* animPool = rec->getFltFile()->getLtPtAnimationPool();
    LtPtAnimationPool::PoolLtPtAnimation* ltPtAnim = NULL;
    if (ltPtIdx->iAnimationIndex >= 0)
    {
        ltPtAnim = animPool->get( ltPtIdx->iAnimationIndex );
        if (!ltPtAnim)
            return; // Animation index out of range
    }

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

    addVertices(&pBuilder, osgParent, rec);

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

    float lobeVert=0.f, lobeHorz=0.f, lobeRoll=0.f;
    if ( directional)
    {
        lobeVert = osg::DegreesToRadians( ltPtApp->_sfVLobeAngle );
        lobeHorz = osg::DegreesToRadians( ltPtApp->_sfHLobeAngle );
        lobeRoll = osg::DegreesToRadians( ltPtApp->_sfLobeRollAngle );
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

            osgSim::BlinkSequence* blink = NULL;
            if (ltPtAnim && ltPtAnim->_blink.valid())
                blink = ltPtAnim->_blink.get();

            // note in corbin's code the ltPtApp->_bIntensity was set to 1.0, however,
            // I have left the original setting in place.
            osgSim::LightPoint lp( true, coords[nl], color, ltPtApp->_bIntensity, pointRadius,
                0, blink );

            if (directional)
            {
                // calc elevation angles
                osg::Vec3 normal( 1.0f, 0.0f, 0.0f);
                if (nl < norms.size())
                    normal = norms[nl];
                if (i==1)
                    // Negate the normal for the back facing internal light point
                    normal = -normal;

                lp._sector = new osgSim::DirectionalSector( normal, lobeHorz, lobeVert, lobeRoll);
            }

            lpNode->addLightPoint(lp);
        }
    }

    osgParent.addChild(lpNode);
}


// OpenFlight 15.8 (1580)
// Light point systems allow an application to control intensity, animation
//   state, and on/off state of all child light point nodes from a single node.
// On/off state implemented with an osgSim::MultiSwitch. Set 0 turns all children
//   off, set 1 turns them on. Applications can define other sets if desired, or
//   redefine these sets.
// Children LightPointNodes all have a reference to a common LightPointState object
//   An application controls intensity and animation state parameters by finding
//   the first child of the Light Point System MultiSwitch, calling
//   getLightPointState(), and setting intensity and animation state accordingly.
osg::Group* ConvertFromFLT::visitLightPointSystem(osg::Group& osgParent, LightPointSystemRecord* rec)
{
    SLightPointSystem *ltPtSys = (SLightPointSystem*)rec->getData();

    osgSim::MultiSwitch* system = new osgSim::MultiSwitch;
    osg::ref_ptr<osgSim::LightPointSystem> lightState = new osgSim::LightPointSystem;

    // Attach children
    visitAncillary( osgParent, *system, rec )->addChild( system );
    visitPrimaryNode( *system, rec );

    system->setName( ltPtSys->ident );

    // Set default sets: 0 for all off, 1 for all on
    system->setAllChildrenOn( 1 );
    system->setAllChildrenOff( 0 );

    // Set initial on/off state
    unsigned int initialSet = ( (ltPtSys->flags & 0x80000000) != 0 ) ? 1 : 0;
    system->setActiveSwitchSet( initialSet );


    lightState->setIntensity( ltPtSys->intensity );
    switch( ltPtSys->animationState )
    {
        // Note that OpenFlight 15.8 spec says 0 means on and 1 means off.
        //   However, if animation is set on in Creator, it stores a 1, and
        //   a zero is stored for off! So, for now, we ignore the spec...
    case 0:
        lightState->setAnimationState( osgSim::LightPointSystem::ANIMATION_OFF );
        break;
    default:
    case 1:
        lightState->setAnimationState( osgSim::LightPointSystem::ANIMATION_ON );
        break;
    case 2:
        lightState->setAnimationState( osgSim::LightPointSystem::ANIMATION_RANDOM );
        break;
    }

    // Set light point state in all children
    int errorChildren = 0;
    for ( unsigned int i=0; i<system->getNumChildren(); i++)
    {
        osg::Node* child = system->getChild( i );
        if (osgSim::LightPointNode* lpn = dynamic_cast<osgSim::LightPointNode*>(child))
            lpn->setLightPointSystem (lightState.get() );
        else
            // Should never have a non-LightPointNode child
            errorChildren++;
    }
    if (errorChildren > 0)
        osg::notify( osg::WARN ) << "ConvertFromFLT::visitLightPointSystem found " << errorChildren << " non-LightPointNode child(ren)." << std::endl;

    return ( (osg::Group*) system );
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
    addVertices ( pBuilder, parent, rec );

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


int ConvertFromFLT::addMeshPrimitives ( osg::Group &parent, GeoSetBuilder *pBuilder, MeshRecord *rec )
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
            visitMeshPrimitive ( parent, pBuilder, (MeshPrimitiveRecord *) child );
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


void ConvertFromFLT::visitMeshPrimitive ( osg::Group &parent, GeoSetBuilder *pBuilder, MeshPrimitiveRecord *mesh )
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
        osg::notify(osg::NOTICE)<<"Warning:ConvertFromFLT::visitMeshPrimitive () unknown MeshPrimitiveRecord type."<<std::endl;
        return;
    }

    // Add the vertex properties.
    setMeshCoordinates    ( mesh->getNumVertices(), pool, mesh, geometry );
    setMeshNormals        ( mesh->getNumVertices(), pool, mesh, geometry );
    setMeshColors         ( mesh->getNumVertices(), pool, mesh, geometry );
    setMeshTexCoordinates ( mesh->getNumVertices(), pool, mesh, geometry );

    DynGeoSet* dgset = pBuilder->getDynGeoSet();
    osg::StateSet* osgStateSet = dgset->getStateSet();
    geometry->setStateSet( osgStateSet );

    // Add the geometry to the geode.
    geode->addDrawable ( geometry );

    // Add the geode to the parent.
    parent.addChild ( geode );
}


uint32 ConvertFromFLT::setMeshCoordinates ( const uint32 &numVerts, const LocalVertexPoolRecord *pool, MeshPrimitiveRecord *mesh, osg::Geometry *geometry )
{
    if (!pool || !mesh || !geometry)
    {
        osg::notify(osg::WARN)<<"OpenFlight loader detected error:: ConvertFromFLT::setMeshCoordinates passed null objects."<<std::endl;
    }

    // If there aren't any coordinates...
    if ( false == pool->hasAttribute ( LocalVertexPoolRecord::POSITION ) )
        return 0;

    // Allocate the vertices.
    osg::Vec3Array *coords = new osg::Vec3Array(numVerts);
    if ( NULL == coords )
    {
        osg::notify(osg::WARN)<<"OpenFlight loader detected error:: ConvertFromFLT::setMeshCoordinates out of memory."<<std::endl;
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
            osg::notify(osg::WARN)<<"OpenFlight loader detected error:: ConvertFromFLT::setMeshCoordinates out of bounds."<<std::endl;
            return 0;
        }

        // Get the coordinate (using "index").
        if ( !pool->getPosition ( index, px, py, pz ) )
        {
            osg::notify(osg::WARN)<<"OpenFlight loader detected error:: ConvertFromFLT::setMeshCoordinates out of bounds."<<std::endl;
            return 0;
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
    if (!pool || !mesh || !geometry)
    {
        osg::notify(osg::WARN)<<"OpenFlight loader detected error:: ConvertFromFLT::setMeshNormals passed null objects."<<std::endl;
    }

    // If there aren't any coordinates...
    if ( false == pool->hasAttribute ( LocalVertexPoolRecord::NORMAL ) )
        return 0;

    // Allocate the normals.
    osg::Vec3Array *normals = new osg::Vec3Array(numVerts);
    if ( NULL == normals )
    {
        osg::notify(osg::WARN)<<"OpenFlight loader detected error:: ConvertFromFLT::setMeshNormals out of memory."<<std::endl;
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
            osg::notify(osg::WARN)<<"OpenFlight loader detected error:: ConvertFromFLT::setMeshNormals out of bounds."<<std::endl;
            return 0;
        }

        // Get the normal (using "index").
        if ( !pool->getNormal ( index, x, y, z ) )
        {
            osg::notify(osg::WARN)<<"OpenFlight loader detected error:: ConvertFromFLT::setMeshNormals out of bounds."<<std::endl;
            return 0;
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
    if (!pool || !mesh || !geometry)
    {
        osg::notify(osg::WARN)<<"OpenFlight loader detected error:: ConvertFromFLT::setMeshColors passed null objects."<<std::endl;
    }

    // If there aren't any colors...
    if ( false == pool->hasAttribute ( LocalVertexPoolRecord::RGB_COLOR ) )
        return 0;

    // Allocate the normals.
    osg::Vec4Array *colors = new osg::Vec4Array(numVerts);
    if ( NULL == colors )
    {
        osg::notify(osg::WARN)<<"OpenFlight loader detected error:: ConvertFromFLT::setMeshColors out of memory."<<std::endl;
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
            osg::notify(osg::WARN)<<"OpenFlight loader detected error:: ConvertFromFLT::setMeshColors out of bounds."<<std::endl;
            return 0;
        }

        // Get the color (using "index").
        if ( false == pool->getColorRGBA ( index, red, green, blue, alpha ) )
        {
            osg::notify(osg::WARN)<<"OpenFlight loader detected error:: ConvertFromFLT::setMeshColors out of bounds."<<std::endl;
            return 0;
        }

        // Add the coordinate.
        (*colors)[i].set ( red, green, blue, alpha );
    }

    // Set the mesh coordinates.
    geometry->setColorArray ( colors );
    geometry->setColorBinding( osg::Geometry::BIND_PER_VERTEX );

    // Return the number of colors added.
    return i;
}

void ConvertFromFLT::setMeshTexCoordinates ( const uint32 &numVerts, const LocalVertexPoolRecord *pool, MeshPrimitiveRecord *mesh, osg::Geometry *geometry )
{
    if (!pool || !mesh || !geometry)
    {
        osg::notify(osg::WARN)<<"OpenFlight loader detected error:: ConvertFromFLT::setMeshTexCoordinates passed null objects."<<std::endl;
    }

    // Don't know the best way to do this in C++ without breaking
    // data encapsulation rules or ending up with silly cut-and-paste
    // code
    //
    std::vector<LocalVertexPoolRecord::AttributeMask>  lAttrList( 8 );
    lAttrList[0] = LocalVertexPoolRecord::BASE_UV;
    lAttrList[1] = LocalVertexPoolRecord::UV_1;
    lAttrList[2] = LocalVertexPoolRecord::UV_2;
    lAttrList[3] = LocalVertexPoolRecord::UV_3;
    lAttrList[4] = LocalVertexPoolRecord::UV_4;
    lAttrList[5] = LocalVertexPoolRecord::UV_5;
    lAttrList[6] = LocalVertexPoolRecord::UV_6;
    lAttrList[7] = LocalVertexPoolRecord::UV_7;

    osg::notify(osg::INFO) << "flt2osg::setMeshTexCoordinates() "
                           << "Attribute masks in list."
                           << std::endl;

    // Check for texture coordinates for each possible texture
    //
    unsigned int   lAttrIdx;
    for (lAttrIdx = 0; lAttrIdx < lAttrList.size(); ++lAttrIdx)
    {
       osg::notify(osg::INFO) << "flt2osg::setMeshTexCoordinates() "
                              << "Checking texture "
                              << lAttrIdx
                              << std::endl;

        // If there aren't any coordinates for this texture, skip to next
        //
        if (!pool->hasAttribute ( lAttrList[lAttrIdx] ) )
            continue;

        // Allocate the vertices.
        osg::Vec2Array *coords = new osg::Vec2Array(numVerts);
        if ( NULL == coords )
        {
            osg::notify(osg::WARN)<<"OpenFlight loader detected error:: ConvertFromFLT::setMeshTexCoordinates out of memory."<<std::endl;
            return;
        }

        osg::notify(osg::INFO) << "flt2osg::setMeshTexCoordinates() "
                               << "Getting coords"
                               << std::endl;

        // Declare outside of loop.
        uint32 i ( 0 ), index ( 0 );
        float32 pu, pv;

        // Loop through all the vertices.
        for ( i = 0; i < numVerts; ++i )
        {
            // Get the i'th index into the vertex pool.
            if ( !mesh->getVertexIndex ( i, index ) )
            {
                osg::notify(osg::WARN)<<"OpenFlight loader detected error:: ConvertFromFLT::setMeshTexCoordinates out of bounds."<<std::endl;
                return;
            }

            // Get the coordinate (using "index").
            if ( !pool->getUV ( index, lAttrList[lAttrIdx], pu, pv ) )
            {
                osg::notify(osg::WARN)<<"OpenFlight loader detected error:: ConvertFromFLT::setMeshTexCoordinates out of bounds."<<std::endl;
                return;
            }

            // Add the coordinate.
            (*coords)[i].set ( (float) pu, (float) pv );
        }

        osg::notify(osg::INFO) << "flt2osg::setMeshTexCoordinates() "
                               << "Adding coords to texture unit "
                               << lAttrIdx
                               << std::endl;

        // Set the mesh coordinates for this texture layer... use
        // the attribute index as the texture unit.
        //
        geometry->setTexCoordArray ( lAttrIdx, coords );

    }  // check each possible texture

    return;
}


