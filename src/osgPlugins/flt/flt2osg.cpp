#include <stdio.h>
#include <string.h>
#include <osg/GL>

#include <osg/Group>
#include <osg/LOD>
#include <osg/MatrixTransform>
#include <osg/Switch>
#include <osg/Geode>
#include <osg/StateSet>
#include <osg/CullFace>
#include <osg/TexEnv>
#include <osg/TexGen>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/Point>
#include <osg/Material>
#include <osg/PolygonOffset>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Billboard>
#include <osg/Texture>
#include <osg/Image>
#include <osg/Notify>

#include <osg/DOFTransform>
#include <osg/Sequence>

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
#include "MeshRecord.h"
#include "MeshPrimitiveRecord.h"
#include "TransformationRecords.h"
#include "ExternalRecord.h"
#include "LightPointRecord.h"
#include "Input.h"
#include "GeoSetBuilder.h"
#include "LongIDRecord.h"
#include "InstanceRecords.h"
#include "LocalVertexPoolRecord.h"



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

            #ifdef _DEBUG
            
            default:
              osg::notify(osg::INFO) << "In ConvertFromFLT::visitPrimaryNode(), unknown opcode: " << child->getOpcode() << std::endl;
              break;

            #endif
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
            pColorPool->addColor(i+(4096>>7), color);
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
    
    
    unsigned int unit = 0;

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
            osgStateSet->setTextureAttributeAndModes( unit, osgTexture,osg::StateAttribute::ON);

            osg::TexEnv* osgTexEnv = new osg::TexEnv;
            osgTexEnv->setMode(osg::TexEnv::MODULATE);
            osgStateSet->setTextureAttribute( unit, osgTexEnv );
        }

        osg::Texture *osgTexture = dynamic_cast<osg::Texture*>(osgStateSet->getTextureAttribute( unit, osg::StateAttribute::TEXTURE));
        if (osgTexture == NULL)
        {
            osgTexture = new osg::Texture;
            osgStateSet->setTextureAttributeAndModes( unit, osgTexture,osg::StateAttribute::ON);
        }

        osgTexture->setImage(image.get());

        // Add new texture to registry pool
        // ( umm... should this have reference to the texture unit? RO. July2002)
        Registry::instance()->addTexture(pFilename, osgStateSet);

        // Also add to local pool to be able to get texture by index.
        // ( umm... should this have reference to the texture unit? RO. July2002)
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

    SGroup* currentGroup = (SGroup*) rec->getData();
    bool forwardAnim , swingAnim;

    // OpenFlight 15.7 has two animation flags, forward and swing 
    if ( (currentGroup->dwFlags    & GroupRecord::FORWARD_ANIM)== GroupRecord::FORWARD_ANIM) forwardAnim    =    true; else forwardAnim    =    false;
    if ( (currentGroup->dwFlags    & GroupRecord::SWING_ANIM)    == GroupRecord::SWING_ANIM) swingAnim       =    true; else swingAnim    =    false;
     
    if( forwardAnim || swingAnim )
    {
        osg::Sequence* animSeq = new osg::Sequence;
        
        if ( forwardAnim )
            animSeq->setInterval(osg::Sequence::LOOP, 0, -1);
        else 
            animSeq->setInterval(osg::Sequence::SWING, 0, -1);
        
        animSeq->setName(rec->getData()->szIdent);
        
        visitAncillary(osgParent, *animSeq, rec)->addChild( animSeq );
        visitPrimaryNode(*animSeq, rec);
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
#define USE_DOFTransform

#if defined(USE_DOFTransform)

    osg::DOFTransform* transform = new osg::DOFTransform;
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
    osg::Group* group = new osg::Group;

    group->setName(pSSwitch->szIdent);
    visitAncillary(osgParent, *group, rec)->addChild( group );
    visitPrimaryNode(*group, (PrimNodeRecord*)rec);

    for(unsigned int nChild=0; nChild<(unsigned int)rec->getNumChildren(); nChild++)
    {
        unsigned int nMaskBit = nChild % 32;
        unsigned int nMaskWord = pSSwitch->nCurrentMask * pSSwitch->nWordsInMask + nChild / 32;

        if (!(pSSwitch->aMask[nMaskWord] & (uint32(1) << nMaskBit)))
        {
            if (nChild<group->getNumChildren())
            {
                osg::Node* node = group->getChild(nChild);
                if (node)
                    node->setNodeMask(0);
            }
            else
            {
                osg::notify(osg::WARN)<<"Warning::OpenFlight loader has come across an incorrectly handled switch."<<std::endl;
                osg::notify(osg::WARN)<<"         The number of OpenFlight children ("<<rec->getNumChildren()<<") "<<std::endl;
                osg::notify(osg::WARN)<<"         exceeds the number converted to OSG ("<<group->getNumChildren()<<")"<<std::endl;
            }
             
             
        }
    }

    return group;
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
            dgset->setPrimType(osg::Primitive::LINE_STRIP);
            break;

        case FaceRecord::WIREFRAME_CLOSED:
            dgset->setPrimType(osg::Primitive::LINE_LOOP);
            break;

        case FaceRecord::OMNIDIRECTIONAL_LIGHT:
        case FaceRecord::UNIDIRECTIONAL_LIGHT:
        case FaceRecord::BIDIRECTIONAL_LIGHT:
            dgset->setPrimType(osg::Primitive::POINTS);
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
                    _faceColor = pColorPool->getColor(pSFace->dwPrimaryColorIndex);
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
            osg::StateSet *textureStateSet = dynamic_cast<osg::StateSet *>
                (pTexturePool->getTexture((int)pSFace->iTexturePattern));

            if (textureStateSet)
            {
                // Merge face stateset with texture stateset
                osgStateSet->merge(*textureStateSet);

                // Alpha channel in texture?
                osg::Texture *osgTexture = dynamic_cast<osg::Texture*>(textureStateSet->getTextureAttribute( 0, osg::StateAttribute::TEXTURE));
                if (osgTexture)
                {
                    osg::Image* osgImage = osgTexture->getImage();
                    switch (osgImage->getPixelFormat())
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
        if (dgset->getPrimType() == osg::Primitive::POINTS)
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

    dgset->setPrimType(osg::Primitive::POINTS);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    stateSet->setMode(GL_POINT_SMOOTH, osg::StateAttribute::ON);
    dgset->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

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
    rec->makeCurrent();

    // We didn't add any vertices.
    return 0;
}


void ConvertFromFLT::visitMeshPrimitive ( osg::Group &parent, MeshPrimitiveRecord *mesh )
{
    assert ( mesh );

    osg::Geode *geode = new osg::Geode();
    osg::Geometry *geometry = new osg::Geometry();
    LocalVertexPoolRecord *pool = LocalVertexPoolRecord::getCurrent();
    assert ( pool );

    // Set the correct primitive type.
    switch ( mesh->getData()->primitiveType )
    {
    case MeshPrimitiveRecord::TRIANGLE_STRIP:
        geometry->addPrimitive ( new osg::DrawArrays(osg::Primitive::TRIANGLE_STRIP,0,mesh->getNumVertices()) );
        break;
    case MeshPrimitiveRecord::TRIANGLE_FAN:
        geometry->addPrimitive ( new osg::DrawArrays(osg::Primitive::TRIANGLE_FAN,0,mesh->getNumVertices()) );
        break;
    case MeshPrimitiveRecord::QUADRILATERAL_STRIP:
        geometry->addPrimitive ( new osg::DrawArrays(osg::Primitive::QUAD_STRIP,0,mesh->getNumVertices()) );
        break;
    case MeshPrimitiveRecord::INDEXED_POLYGON:
        geometry->addPrimitive ( new osg::DrawArrays(osg::Primitive::POLYGON,0,mesh->getNumVertices()) );
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
