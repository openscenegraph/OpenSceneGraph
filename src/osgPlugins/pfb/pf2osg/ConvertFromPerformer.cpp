#include "ConvertFromPerformer.h"

#include <osg/Scene>
#include <osg/Group>
#include <osg/DCS>
#include <osg/LOD>
#include <osg/Switch>
#include <osg/Sequence>
#include <osg/Geode>
#include <osg/Billboard>
#include <osg/Texture>
#include <osg/Image>
#include <osg/CullFace>
#include <osg/FileNameUtils>
#include <osg/Notify>

#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfBillboard.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfSCS.h>
#include <Performer/pf/pfLOD.h>
#include <Performer/pf/pfSwitch.h>
#include <Performer/pf/pfSequence.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfMaterial.h>
#include <Performer/pr/pfTexture.h>

#ifdef OSG_USE_IO_DOT_H
#include <iostream.h>
#else
#include <iostream>
using namespace std;
#endif

ConvertFromPerformer::ConvertFromPerformer()
{
    _osgRoot = NULL;

    _gsetPrimMap[PFGS_POINTS] = osg::GeoSet::POINTS;
    _gsetPrimMap[PFGS_LINES] = osg::GeoSet::LINES;
    _gsetPrimMap[PFGS_LINESTRIPS] = osg::GeoSet::LINE_STRIP;
    _gsetPrimMap[PFGS_TRIS] = osg::GeoSet::TRIANGLES;
    _gsetPrimMap[PFGS_QUADS] = osg::GeoSet::QUADS;
    _gsetPrimMap[PFGS_TRISTRIPS] = osg::GeoSet::TRIANGLE_STRIP;
    _gsetPrimMap[PFGS_FLAT_LINESTRIPS] = osg::GeoSet::FLAT_LINE_STRIP;
    _gsetPrimMap[PFGS_FLAT_TRISTRIPS] = osg::GeoSet::FLAT_TRIANGLE_STRIP;
    _gsetPrimMap[PFGS_TRIFANS] = osg::GeoSet::TRIANGLE_FAN;
    _gsetPrimMap[PFGS_FLAT_TRIFANS] = osg::GeoSet::FLAT_TRIANGLE_FAN;
    _gsetPrimMap[PFGS_POLYS] = osg::GeoSet::POLYGON;
    _gsetPrimMap[PFGS_NUM_PRIMS] = osg::GeoSet::NO_TYPE;

    _gsetBindMap[PFGS_OFF] = osg::GeoSet::BIND_OFF;
    _gsetBindMap[PFGS_OVERALL] = osg::GeoSet::BIND_OVERALL;
    _gsetBindMap[PFGS_PER_PRIM] = osg::GeoSet::BIND_PERPRIM;
    _gsetBindMap[PFGS_PER_VERTEX] = osg::GeoSet::BIND_PERVERTEX;

              
    _gstateTypeMap[PFSTATE_TRANSPARENCY] = osg::GeoState::TRANSPARENCY;
    _gstateTypeMap[PFSTATE_ANTIALIAS] = osg::GeoState::ANTIALIAS;
    _gstateTypeMap[PFSTATE_ENLIGHTING] = osg::GeoState::LIGHTING;
    _gstateTypeMap[PFSTATE_ENTEXTURE] = osg::GeoState::TEXTURE;
    _gstateTypeMap[PFSTATE_ENFOG] = osg::GeoState::FOG;
    _gstateTypeMap[PFSTATE_CULLFACE] = osg::GeoState::FACE_CULL;
    _gstateTypeMap[PFSTATE_ENWIREFRAME] = osg::GeoState::WIREFRAME;
    _gstateTypeMap[PFSTATE_ENTEXGEN] = osg::GeoState::TEXGEN;
    _gstateTypeMap[PFSTATE_ENTEXMAT] = osg::GeoState::TEXMAT;

//  not currently supported under the OSG.
//     _gstateTypeMap[PFSTATE_DECAL] = ;
//     _gstateTypeMap[PFSTATE_ENTEXLOD] = ;
//     _gstateTypeMap[PFSTATE_ALPHAFUNC] = ;
//     _gstateTypeMap[PFSTATE_ENCOLORTABLE] = ;
//     _gstateTypeMap[PFSTATE_ENHIGHLIGHTING] = ;
//     _gstateTypeMap[PFSTATE_ENLPOINTSTATE] = ;

    _saveImagesAsRGB = false;
    _saveAbsoluteImagePath = false;

}

ConvertFromPerformer::~ConvertFromPerformer()
{
}


osg::Node* ConvertFromPerformer::convert(pfNode* node)
{
    if (node==NULL) return NULL;
    return visitNode(NULL,node);
}


osg::Object* ConvertFromPerformer::getOsgObject(pfObject* pfObj)
{
    PfObjectToOsgObjectMap::iterator fitr = _pfToOsgMap.find(pfObj);
    if (fitr != _pfToOsgMap.end())
    {
//         osg::notify(DEBUG) << "Found shared object"<<endl;
        return (*fitr).second;
    }
    else return NULL;
}

void ConvertFromPerformer::regisiterPfObjectForOsgObject(pfObject* pfObj,osg::Object* osgObj)
{
    _pfToOsgMap[pfObj] = osgObj;
}

osg::Node* ConvertFromPerformer::visitNode(osg::Group* osgParent,pfNode* node)
{
    if (node==NULL) return NULL;

    if      (node->getType()->isDerivedFrom( pfBillboard::getClassType()))  return visitBillboard(osgParent,(pfBillboard*)node);
    else if (node->getType()->isDerivedFrom( pfGeode::getClassType()))      return visitGeode(osgParent,(pfGeode*)node);
    else if (node->getType()->isDerivedFrom( pfScene::getClassType()))      return visitScene(osgParent,(pfScene*)node);
    else if (node->getType()->isDerivedFrom( pfDCS::getClassType()))        return visitDCS(osgParent,(pfDCS*)node);
    else if (node->getType()->isDerivedFrom( pfSCS::getClassType()))        return visitSCS(osgParent,(pfSCS*)node);
    else if (node->getType()->isDerivedFrom( pfLOD::getClassType()))        return visitLOD(osgParent,(pfLOD*)node);
    else if (node->getType()->isDerivedFrom( pfSequence::getClassType()))   return visitSequence(osgParent,(pfSequence*)node);
    else if (node->getType()->isDerivedFrom( pfSwitch::getClassType()))     return visitSwitch(osgParent,(pfSwitch*)node);
    else if (node->getType()->isDerivedFrom( pfGroup::getClassType()))      return visitGroup(osgParent,(pfGroup*)node);

    return NULL;
}

osg::Node* ConvertFromPerformer::visitScene(osg::Group* osgParent,pfScene* scene)
{
    osg::Scene* osgScene = dynamic_cast<osg::Scene*>(getOsgObject(scene));
    if (osgScene)
    {
        if (osgParent) osgParent->addChild(osgScene);
        return osgScene;
    }

    osgScene = new osg::Scene;
    if (osgParent) osgParent->addChild(osgScene);

    regisiterPfObjectForOsgObject(scene,osgScene);

    const char* name = scene->getName();
    if (name) osgScene->setName(name);

    for(int i=0;i<scene->getNumChildren();++i)
    {
        visitNode(osgScene,scene->getChild(i));
    }
    return (osg::Node*)osgScene;
}

osg::Node* ConvertFromPerformer::visitGroup(osg::Group* osgParent,pfGroup* group)
{
    osg::Group* osgGroup = dynamic_cast<osg::Group*>(getOsgObject(group));
    if (osgGroup)
    {
        if (osgParent) osgParent->addChild(osgGroup);
        return osgGroup;
    }

    osgGroup = new osg::Group;
    if (osgParent) osgParent->addChild(osgGroup);

    regisiterPfObjectForOsgObject(group,osgGroup);

    const char* name = group->getName();
    if (name) osgGroup->setName(name);

    for(int i=0;i<group->getNumChildren();++i)
    {
        visitNode(osgGroup,group->getChild(i));
    }
    return (osg::Node*)osgGroup;
}

osg::Node* ConvertFromPerformer::visitLOD(osg::Group* osgParent,pfLOD* lod)
{
    osg::LOD* osgLOD = dynamic_cast<osg::LOD*>(getOsgObject(lod));
    if (osgLOD)
    {
        if (osgParent) osgParent->addChild(osgLOD);
        return osgLOD;
    }

    osgLOD = new osg::LOD;
    if (osgParent) osgParent->addChild(osgLOD);

    regisiterPfObjectForOsgObject(lod,osgLOD);

    const char* name = lod->getName();
    if (name) osgLOD->setName(name);

    pfVec3 center;
    lod->getCenter(center);
    osg::Vec3 osgCenter(center[0],center[1],center[2]);
    osgLOD->setCenter(osgCenter);

    int i;
    for(i=0;i<lod->getNumRanges();++i)
    {
        osgLOD->setRange(i,lod->getRange(i));
    }

    for(i=0;i<lod->getNumChildren();++i)
    {
        visitNode(osgLOD,lod->getChild(i));
    }
    return (osg::Node*)osgLOD;

}

osg::Node* ConvertFromPerformer::visitSwitch(osg::Group* osgParent,pfSwitch* switchNode)
{
    osg::Switch* osgSwitch = dynamic_cast<osg::Switch*>(getOsgObject(switchNode));
    if (osgSwitch)
    {
        if (osgParent) osgParent->addChild(osgSwitch);
        return osgSwitch;
    }

    osgSwitch = new osg::Switch;
    if (osgParent) osgParent->addChild(osgSwitch);

    regisiterPfObjectForOsgObject(switchNode,osgSwitch);

    const char* name = switchNode->getName();
    if (name) osgSwitch->setName(name);

    float val = switchNode->getVal();
    if (val==PFSWITCH_ON)
    {
        osgSwitch->setVal(osg::Switch::ALL_CHILDREN_ON);
    }
    else if (val==PFSWITCH_OFF)
    {
        osgSwitch->setVal(osg::Switch::ALL_CHILDREN_OFF);
    }
    else
    {
        osgSwitch->setVal((int)val);
    }

    for(int i=0;i<switchNode->getNumChildren();++i)
    {
        visitNode(osgSwitch,switchNode->getChild(i));
    }
    return (osg::Node*)osgSwitch;
}

osg::Node* ConvertFromPerformer::visitSequence(osg::Group* osgParent,pfSequence* sequence)
{
    osg::Sequence* osgSequence = dynamic_cast<osg::Sequence*>(getOsgObject(sequence));
    if (osgSequence)
    {
        if (osgParent) osgParent->addChild(osgSequence);
        return osgSequence;
    }

    osgSequence = new osg::Sequence;
    if (osgParent) osgParent->addChild(osgSequence);

    regisiterPfObjectForOsgObject(sequence,osgSequence);

    for(int i=0;i<sequence->getNumChildren();++i)
    {
        visitNode(osgSequence,sequence->getChild(i));
    }
    return (osg::Node*)osgSequence;
}

osg::Node* ConvertFromPerformer::visitDCS(osg::Group* osgParent,pfDCS* dcs)
{

    osg::DCS* osgDCS = dynamic_cast<osg::DCS*>(getOsgObject(dcs));
    if (osgDCS)
    {
        if (osgParent) osgParent->addChild(osgDCS);
        return osgDCS;
    }

    osgDCS = new osg::DCS;
    if (osgParent) osgParent->addChild(osgDCS);

    regisiterPfObjectForOsgObject(dcs,osgDCS);

    const char* name = dcs->getName();
    if (name) osgDCS->setName(name);

    pfMatrix matrix;
    dcs->getMat(matrix);

    osg::Matrix osgMatrix(matrix[0][0],matrix[0][1],matrix[0][2],matrix[0][3],
                          matrix[1][0],matrix[1][1],matrix[1][2],matrix[1][3],
                          matrix[2][0],matrix[2][1],matrix[2][2],matrix[2][3],
                          matrix[3][0],matrix[3][1],matrix[3][2],matrix[3][3]);

    osgDCS->setMatrix(osgMatrix);

    for(int i=0;i<dcs->getNumChildren();++i)
    {
        visitNode(osgDCS,dcs->getChild(i));
    }
    return (osg::Node*)osgDCS;
}

osg::Node* ConvertFromPerformer::visitSCS(osg::Group* osgParent,pfSCS* scs)
{
    // note the OSG does not currently have a SCS, so use DCS instead.
    osg::DCS* osgDCS = dynamic_cast<osg::DCS*>(getOsgObject(scs));
    if (osgDCS)
    {
        if (osgParent) osgParent->addChild(osgDCS);
        return osgDCS;
    }

    osgDCS = new osg::DCS;
    if (osgParent) osgParent->addChild(osgDCS);

    regisiterPfObjectForOsgObject(scs,osgDCS);

    const char* name = scs->getName();
    if (name) osgDCS->setName(name);

    pfMatrix matrix;
    scs->getMat(matrix);
    osg::Matrix osgMatrix(matrix[0][0],matrix[0][1],matrix[0][2],matrix[0][3],
                          matrix[1][0],matrix[1][1],matrix[1][2],matrix[1][3],
                          matrix[2][0],matrix[2][1],matrix[2][2],matrix[2][3],
                          matrix[3][0],matrix[3][1],matrix[3][2],matrix[3][3]);

    osgDCS->setMatrix(osgMatrix);

    for(int i=0;i<scs->getNumChildren();++i)
    {
        visitNode(osgDCS,scs->getChild(i));
    }
    return (osg::Node*)osgDCS;
}

osg::Node* ConvertFromPerformer::visitGeode(osg::Group* osgParent,pfGeode* geode)
{
    osg::Geode* osgGeode = dynamic_cast<osg::Geode*>(getOsgObject(geode));
    if (osgGeode)
    {
        if (osgParent) osgParent->addChild(osgGeode);
        return osgGeode;
    }

    osgGeode = new osg::Geode;
    if (osgParent) osgParent->addChild(osgGeode);

    regisiterPfObjectForOsgObject(geode,osgGeode);

    const char* name = geode->getName();
    if (name) osgGeode->setName(name);

    for(int i=0;i<geode->getNumGSets();++i)
    {
        visitGeoSet(osgGeode,geode->getGSet(i));
    }

    return (osg::Node*)osgGeode;
}

osg::Node* ConvertFromPerformer::visitBillboard(osg::Group* osgParent,pfBillboard* billboard)
{
//    return NULL;

    osg::Billboard* osgBillboard = dynamic_cast<osg::Billboard*>(getOsgObject(billboard));
    if (osgBillboard)
    {
        if (osgParent) osgParent->addChild(osgBillboard);
        return osgBillboard;
    }

    osgBillboard = new osg::Billboard;
    if (osgParent) osgParent->addChild(osgBillboard);

    regisiterPfObjectForOsgObject(billboard,osgBillboard);

    const char* name = billboard->getName();
    if (name) osgBillboard->setName(name);

    pfVec3 axis;
    billboard->getAxis(axis);
    osgBillboard->setAxis(osg::Vec3(axis[0],axis[1],axis[2]));

    for(int i=0;i<billboard->getNumGSets();++i)
    {
        /* osg::GeoSet* osggset = */visitGeoSet(osgBillboard,billboard->getGSet(i));
        pfVec3 pos;
        billboard->getPos(i,pos);
        osgBillboard->setPos(i,osg::Vec3(pos[0],pos[1],pos[2]));
    }

    return (osg::Node*)osgBillboard;
}

int ConvertFromPerformer::getNumVerts(pfGeoSet *gset)
{
    int nv;
    int np;
    int *lens;
    int i;

    np = gset->getNumPrims();
    nv = 0;

    switch( gset->getPrimType() )
    {
        case PFGS_POINTS :
            nv = np;
            break;

        case PFGS_LINES :
            nv = 2 * np;
            break;

        case PFGS_TRIS :
            nv = 3 * np;
            break;

        case PFGS_QUADS :
            nv = 4 * np;
            break;

        case PFGS_TRISTRIPS :
        case PFGS_FLAT_TRISTRIPS :
        case PFGS_POLYS :
        case PFGS_LINESTRIPS :
        case PFGS_FLAT_LINESTRIPS :

            lens = gset->getPrimLengths();
            for( i = 0; i < np; i++ )
                nv += lens[i];
            break;

    }


    return nv;
}

osg::GeoSet* ConvertFromPerformer::visitGeoSet(osg::Geode* osgGeode,pfGeoSet* geoset)
{
    if (geoset==NULL) return NULL;

    osg::GeoSet* osgGeoSet = dynamic_cast<osg::GeoSet*>(getOsgObject(geoset));
    if (osgGeoSet)
    {
        if (osgGeode) osgGeode->addGeoSet(osgGeoSet);
        return osgGeoSet;
    }

    osgGeoSet = new osg::GeoSet;
    if (osgGeode) osgGeode->addGeoSet(osgGeoSet);

    regisiterPfObjectForOsgObject(geoset,osgGeoSet);

    visitGeoState(osgGeoSet,geoset->getGState());

    int i;

    // number of prims
    int np = geoset->getNumPrims();
    int *plen = geoset->getPrimLengths();

    // Number of verticies (may be different than number of coords)
    int nv = getNumVerts( geoset );

    int prim = geoset->getPrimType();
    int flat_shaded_offset=0;
    if (prim == PFGS_FLAT_LINESTRIPS)       flat_shaded_offset=np;
    else if (prim == PFGS_FLAT_TRISTRIPS)   flat_shaded_offset=2*np;
    else if (prim == PFGS_FLAT_TRIFANS)     flat_shaded_offset=2*np;

    osgGeoSet->setPrimType(_gsetPrimMap[geoset->getPrimType()]);
    osgGeoSet->setNumPrims(np);

    if (plen)
    {
        int *osg_plen = new int [np];
        for(i=0;i<np;++i) osg_plen[i] = plen[i];
        osgGeoSet->setPrimLengths(osg_plen);
    }

    pfVec3 *coords;
    ushort *ilist;
    geoset->getAttrLists( PFGS_COORD3,  (void **)&coords, &ilist );

    // copy the vertex coordinates across.
    if( coords )
    {
        // calc the maximum num of vertex from the index list.
        int cc;
        if (ilist)
        {
            cc = 0;
            for( i = 0; i < nv; i++ )
                if( ilist[i] > cc ) cc = ilist[i];
            cc++;
        }
        else
            cc = nv;


        osg::Vec3* osg_coords = new osg::Vec3 [cc];
        for( i = 0; i < cc; i++ )
        {
            osg_coords[i][0] = coords[i][0];
            osg_coords[i][1] = coords[i][1];
            osg_coords[i][2] = coords[i][2];
        }

        if(ilist)
        {
            osg::ushort* osg_cindex = new osg::ushort [nv];
            for( i = 0; i < nv; i++ )
            {
                osg_cindex[i] = ilist[i];
            }
            osgGeoSet->setCoords(osg_coords, osg_cindex );
        }
        else
        {
            osgGeoSet->setCoords(osg_coords);
        }

    }

    pfVec3 *norms;
    geoset->getAttrLists( PFGS_NORMAL3,  (void **)&norms, &ilist );

    // copy normals
    if(norms)
    {
        int bind = geoset->getAttrBind( PFGS_NORMAL3 );
        int nn = bind == PFGS_OFF ? 0 :
                 bind == PFGS_OVERALL ? 1 :
                 bind == PFGS_PER_PRIM ? geoset->getNumPrims() :
                 bind == PFGS_PER_VERTEX ? nv-flat_shaded_offset : 0;
                 
        // set the normal binding type.
        osgGeoSet->setNormalBinding(_gsetBindMap[bind]);

        // calc the maximum num of vertex from the index list.
        int cc;
        if (ilist)
        {
            cc = 0;
            for( i = 0; i < nn; i++ )
                if( ilist[i] > cc ) cc = ilist[i];
            cc++;
        }
        else
            cc = nn;


        osg::Vec3* osg_norms = new osg::Vec3 [cc];
        for( i = 0; i < cc; i++ )
        {
            osg_norms[i][0] = norms[i][0];
            osg_norms[i][1] = norms[i][1];
            osg_norms[i][2] = norms[i][2];
        }

        if(ilist)
        {
            osg::ushort* osg_cindex = new osg::ushort [nn];
            for( i = 0; i < nn; i++ )
            {
                osg_cindex[i] = ilist[i];
            }
            osgGeoSet->setNormals(osg_norms, osg_cindex );
        }
        else
        {
            osgGeoSet->setNormals(osg_norms);
        }

    }

    pfVec2 *tcoords;
    geoset->getAttrLists( PFGS_TEXCOORD2,  (void **)&tcoords, &ilist );

    // copy texture coords
    if(tcoords)
    {
        int bind = geoset->getAttrBind( PFGS_TEXCOORD2 );
        int nn = bind == PFGS_OFF ? 0 :
                 bind == PFGS_OVERALL ? 1 :
                 bind == PFGS_PER_PRIM ? geoset->getNumPrims() :
                 bind == PFGS_PER_VERTEX ? nv : 0;

        // set the normal binding type.
        osgGeoSet->setTextureBinding(_gsetBindMap[bind]);

        // calc the maximum num of vertex from the index list.
        int cc;
        if (ilist)
        {
            cc = 0;
            for( i = 0; i < nv; i++ )
                if( ilist[i] > cc ) cc = ilist[i];
            cc++;
        }
        else
            cc = nn;


        osg::Vec2* osg_tcoords = new osg::Vec2 [cc];
        for( i = 0; i < cc; i++ )
        {
            osg_tcoords[i][0] = tcoords[i][0];
            osg_tcoords[i][1] = tcoords[i][1];
        }

        if(ilist)
        {
            osg::ushort* osg_cindex = new osg::ushort [nn];
            for( i = 0; i < nn; i++ )
            {
                osg_cindex[i] = ilist[i];
            }
            osgGeoSet->setTextureCoords(osg_tcoords, osg_cindex );
        }
        else
        {
            osgGeoSet->setTextureCoords(osg_tcoords);
        }

    }

    pfVec4 *colors;
    geoset->getAttrLists( PFGS_COLOR4,  (void **)&colors, &ilist );

    // copy color coords
    if(colors)
    {
        int bind = geoset->getAttrBind( PFGS_COLOR4 );
        int nn = bind == PFGS_OFF ? 0 :
                 bind == PFGS_OVERALL ? 1 :
                 bind == PFGS_PER_PRIM ? geoset->getNumPrims() :
                 bind == PFGS_PER_VERTEX ? nv-flat_shaded_offset : 0;

        // set the normal binding type.
        osgGeoSet->setColorBinding(_gsetBindMap[bind]);

        // calc the maximum num of vertex from the index list.
        int cc;
        if (ilist)
        {
            cc = 0;
            for( i = 0; i < nn; i++ )
                if( ilist[i] > cc ) cc = ilist[i];
            cc++;
        }
        else
            cc = nn;


        osg::Vec4* osg_colors = new osg::Vec4 [cc];
        for( i = 0; i < cc; i++ )
        {
            osg_colors[i][0] = colors[i][0];
            osg_colors[i][1] = colors[i][1];
            osg_colors[i][2] = colors[i][2];
            osg_colors[i][3] = colors[i][3];
        }

        if(ilist)
        {
            osg::ushort* osg_cindex = new osg::ushort [nn];
            for( i = 0; i < nn; i++ )
            {
                osg_cindex[i] = ilist[i];
            }
            osgGeoSet->setColors(osg_colors, osg_cindex );
        }
        else
        {
            osgGeoSet->setColors(osg_colors);
        }

    }

    return osgGeoSet;
}

osg::GeoState* ConvertFromPerformer::visitGeoState(osg::GeoSet* osgGeoSet,pfGeoState* geostate)
{
    if (geostate==NULL) return NULL;

    osg::GeoState* osgGeoState = dynamic_cast<osg::GeoState*>(getOsgObject(geostate));
    if (osgGeoState)
    {
        if (osgGeoSet) osgGeoSet->setGeoState(osgGeoState);
        return osgGeoState;
    }

    osgGeoState = new osg::GeoState;
    if (osgGeoSet) osgGeoSet->setGeoState(osgGeoState);

    regisiterPfObjectForOsgObject(geostate,osgGeoState);


    // Don could you fill in some of these blanks???
    unsigned int inherit = geostate->getInherit();
//     osg::notify(DEBUG) << endl << "Inherit = "<<inherit<<endl;
//     if (inherit & PFSTATE_TRANSPARENCY) osg::notify(DEBUG) << "Inherit PFSTATE_TRANSPARENCY"<<endl;
//     else osg::notify(DEBUG) << "Define PFSTATE_TRANSPARENCY"<<endl;
//     if (inherit & PFSTATE_ENTEXTURE) osg::notify(DEBUG) << "Inherit PFSTATE_ENTEXTURE"<<endl;
//     else osg::notify(DEBUG) << "Define PFSTATE_ENTEXTURE"<<endl;
//     if (inherit & PFSTATE_CULLFACE) osg::notify(DEBUG) << "Inherit PFSTATE_CULLFACE"<<endl;
//     else osg::notify(DEBUG) << "Define PFSTATE_CULLFACE"<<endl;
//     if (inherit & PFSTATE_ENLIGHTING) osg::notify(DEBUG) << "Inherit PFSTATE_ENLIGHTING"<<endl;
//     else osg::notify(DEBUG) << "Define PFSTATE_ENLIGHTING"<<endl;
//     if (inherit & PFSTATE_ENFOG) osg::notify(DEBUG) << "Inherit PFSTATE_ENFOG"<<endl;
//     else osg::notify(DEBUG) << "Define PFSTATE_ENFOG"<<endl;
//     if (inherit & PFSTATE_ENWIREFRAME) osg::notify(DEBUG) << "Inherit PFSTATE_ENWIREFRAME"<<endl;
//     else osg::notify(DEBUG) << "Define PFSTATE_ENWIREFRAME"<<endl;
//     if (inherit & PFSTATE_ENTEXMAT) osg::notify(DEBUG) << "Inherit PFSTATE_ENTEXMAT"<<endl;
//     else osg::notify(DEBUG) << "Define PFSTATE_ENTEXMAT"<<endl;
//     if (inherit & PFSTATE_ENTEXGEN) osg::notify(DEBUG) << "Inherit PFSTATE_ENTEXGEN"<<endl;
//     else osg::notify(DEBUG) << "Define PFSTATE_ENTEXGEN"<<endl;
// 
//     if (inherit & PFSTATE_ANTIALIAS) osg::notify(DEBUG) << "Inherit PFSTATE_ANTIALIAS"<<endl;
//     else osg::notify(DEBUG) << "Define PFSTATE_ANTIALIAS"<<endl;
//     if (inherit & PFSTATE_DECAL) osg::notify(DEBUG) << "Inherit PFSTATE_DECAL"<<endl;
//     else osg::notify(DEBUG) << "Define PFSTATE_DECAL"<<endl;
//     if (inherit & PFSTATE_ALPHAFUNC) osg::notify(DEBUG) << "Inherit PFSTATE_ALPHAFUNC"<<endl;
//     else osg::notify(DEBUG) << "Define PFSTATE_ALPHAFUNC"<<endl;
//     if (inherit & PFSTATE_ENCOLORTABLE) osg::notify(DEBUG) << "Inherit PFSTATE_ENCOLORTABLE"<<endl;
//     else osg::notify(DEBUG) << "Define PFSTATE_ENCOLORTABLE"<<endl;
//     if (inherit & PFSTATE_ENHIGHLIGHTING) osg::notify(DEBUG) << "Inherit PFSTATE_ENHIGHLIGHTING"<<endl;
//     else osg::notify(DEBUG) << "Define PFSTATE_ENHIGHLIGHTING"<<endl;
//     if (inherit & PFSTATE_ENLPOINTSTATE) osg::notify(DEBUG) << "Inherit PFSTATE_ENLPOINTSTATE"<<endl;
//     else osg::notify(DEBUG) << "Define PFSTATE_ENLPOINTSTATE"<<endl;
//     if (inherit & PFSTATE_ENTEXLOD) osg::notify(DEBUG) << "Inherit PFSTATE_ENTEXLOD"<<endl;
//     else osg::notify(DEBUG) << "Define PFSTATE_ENTEXLOD"<<endl;


    if (inherit & PFSTATE_TRANSPARENCY) osgGeoState->setMode(osg::GeoState::TRANSPARENCY,osg::GeoState::INHERIT);
    else 
    {
        int mode = geostate->getMode(PFSTATE_TRANSPARENCY);
        switch(mode)
        {
        case(PFTR_FAST):
        case(PFTR_HIGH_QUALITY):
        case(PFTR_BLEND_ALPHA):
        case(PFTR_MS_ALPHA):
        case(PFTR_MS_ALPHA_MASK):
        case(PFTR_NO_OCCLUDE):
        case(PFTR_ON):  osgGeoState->setMode(osg::GeoState::TRANSPARENCY,osg::GeoState::ON);break;
        case(PFTR_OFF): osgGeoState->setMode(osg::GeoState::TRANSPARENCY,osg::GeoState::OFF);break;
        default:        osgGeoState->setMode(osg::GeoState::TRANSPARENCY,osg::GeoState::INHERIT);break;
        }
    }

    if (inherit & PFSTATE_ENTEXTURE) osgGeoState->setMode(osg::GeoState::TEXTURE,osg::GeoState::INHERIT);
    else
    {
        int mode = geostate->getMode(PFSTATE_ENTEXTURE);
        switch(mode)
        {
        case(PF_ON):    osgGeoState->setMode(osg::GeoState::TEXTURE,osg::GeoState::ON);break;
        case(PF_OFF): 
        default:        osgGeoState->setMode(osg::GeoState::TEXTURE,osg::GeoState::OFF);break;
        }
    }

    if (inherit & PFSTATE_CULLFACE) osgGeoState->setMode(osg::GeoState::FACE_CULL,osg::GeoState::INHERIT);
    else 
    {
        int mode = geostate->getMode(PFSTATE_CULLFACE);
        switch(mode)
        {
        case(PFCF_BACK):
            {
                osgGeoState->setMode(osg::GeoState::FACE_CULL,osg::GeoState::ON);
                osg::CullFace *cf = new osg::CullFace;
                cf->setMode(osg::CullFace::BACK);
                osgGeoState->setAttribute(osg::GeoState::FACE_CULL,cf);
            }
            break;
            
        case(PFCF_FRONT):
            {
                osgGeoState->setMode(osg::GeoState::FACE_CULL,osg::GeoState::ON);
                osg::CullFace *cf = new osg::CullFace;
                cf->setMode(osg::CullFace::FRONT);
                osgGeoState->setAttribute(osg::GeoState::FACE_CULL,cf);
            }
            break;
        case(PFCF_BOTH):
            {
                osgGeoState->setMode(osg::GeoState::FACE_CULL,osg::GeoState::ON);
                osg::CullFace *cf = new osg::CullFace;
                cf->setMode(osg::CullFace::FRONT_AND_BACK);
                osgGeoState->setAttribute(osg::GeoState::FACE_CULL,cf);
            }
            break;
        case(PFCF_OFF): 
        default:        osgGeoState->setMode(osg::GeoState::FACE_CULL,osg::GeoState::OFF);break;
        }
    }

    if (inherit & PFSTATE_ENLIGHTING) osgGeoState->setMode(osg::GeoState::LIGHTING,osg::GeoState::INHERIT);
    else 
    {
        int mode = geostate->getMode(PFSTATE_ENLIGHTING);
        switch(mode)
        {
        case(PF_ON):    osgGeoState->setMode(osg::GeoState::LIGHTING,osg::GeoState::ON);break;
        case(PF_OFF): 
        default:        osgGeoState->setMode(osg::GeoState::LIGHTING,osg::GeoState::OFF);break;
        }
    }

    if (inherit & PFSTATE_ENFOG) osgGeoState->setMode(osg::GeoState::FOG,osg::GeoState::INHERIT);
    else 
    {
        int mode = geostate->getMode(PFSTATE_ENFOG);
        switch(mode)
        {
        case(PF_ON):    osgGeoState->setMode(osg::GeoState::FOG,osg::GeoState::ON);break;
        case(PF_OFF): 
        default:        osgGeoState->setMode(osg::GeoState::FOG,osg::GeoState::OFF);break;
        }
    }

// not currently supported by OSG
//     if (inherit & PFSTATE_ENWIREFRAME)  osgGeoState->setMode(osg::GeoState::WIREFRAME,osg::GeoState::INHERIT);
//     else 
//     {
//         int mode = geostate->getMode(PFSTATE_ENWIREFRAME);
//         switch(mode)
//         {
//         case(PF_ON):    osgGeoState->setMode(osg::GeoState::WIREFRAME,osg::GeoState::ON);break;
//         case(PF_OFF): 
//         default:        osgGeoState->setMode(osg::GeoState::WIREFRAME,osg::GeoState::OFF);break;
//         }
//     }


// redundent in OSG's implementation of texmat mode
//     if (inherit & PFSTATE_ENTEXMAT)  osgGeoState->setMode(osg::GeoState::TEXMAT,osg::GeoState::INHERIT);
//     else 
//     {
//         int mode = geostate->getMode(PFSTATE_ENTEXMAT);
//         switch(mode)
//         {
//         case(PF_ON):    osgGeoState->setMode(osg::GeoState::TEXMAT,osg::GeoState::ON);break;
//         case(PF_OFF): 
//         default:        osgGeoState->setMode(osg::GeoState::TEXMAT,osg::GeoState::OFF);break;
//         }
//     }

    if (inherit & PFSTATE_ENTEXGEN)  osgGeoState->setMode(osg::GeoState::TEXGEN,osg::GeoState::INHERIT);
    else 
    {
        int mode = geostate->getMode(PFSTATE_ENTEXGEN);
        switch(mode)
        {
        case(PF_ON):    osgGeoState->setMode(osg::GeoState::TEXGEN,osg::GeoState::ON);break;
        case(PF_OFF): 
        default:        osgGeoState->setMode(osg::GeoState::TEXGEN,osg::GeoState::OFF);break;
        }
    }

    pfMaterial* mat = (pfMaterial*)geostate->getAttr(PFSTATE_FRONTMTL);
    visitMaterial(osgGeoState,mat);
    
    pfTexture* tex = (pfTexture*)geostate->getAttr(PFSTATE_TEXTURE);
    visitTexture(osgGeoState,tex);

    pfTexGen* texgen = (pfTexGen*)geostate->getAttr(PFSTATE_TEXGEN);
    if (texgen)
    {
        osg::TexGen* osgTexGen = new osg::TexGen();
        int mode = texgen->getMode(PF_S);
        switch(mode)
        {
        case(PFTG_OBJECT_LINEAR) : 
            osgTexGen->setMode(osg::TexGen::OBJECT_LINEAR);
            osgGeoState->setAttribute(osg::GeoState::TEXGEN,osgTexGen);
            break;
        case(PFTG_EYE_LINEAR_IDENT) : 
            cerr << "TexGen Mode PFTG_EYE_LINEAR_IDENT not currently supported by the OSG,"<<endl;
            cerr << "       assuming osg::TexGen::EYE_LINEAR."<<endl;
        case(PFTG_EYE_LINEAR) : 
           osgTexGen->setMode(osg::TexGen::EYE_LINEAR);
            osgGeoState->setAttribute(osg::GeoState::TEXGEN,osgTexGen);
            break;
        case(PFTG_SPHERE_MAP) : 
            osgTexGen->setMode(osg::TexGen::SPHERE_MAP); 
            osgGeoState->setAttribute(osg::GeoState::TEXGEN,osgTexGen);
            break;
        case(PFTG_OFF) : 
            osgGeoState->setMode(osg::GeoState::TEXGEN,osg::GeoState::OFF);
            break;
        case(PFTG_OBJECT_DISTANCE_TO_LINE) : 
            cerr << "TexGen Mode PFTG_OBJECT_DISTANCE_TO_LINE not currently supported by the OSG."<<endl;
            osgGeoState->setMode(osg::GeoState::TEXGEN,osg::GeoState::OFF);
            break;
        case(PFTG_EYE_DISTANCE_TO_LINE) : 
            cerr << "TexGen Mode PFTG_EYE_DISTANCE_TO_LINE not currently supported by the OSG."<<endl;
            osgGeoState->setMode(osg::GeoState::TEXGEN,osg::GeoState::OFF);
            break;
        default: 
            cerr << "TexGen Mode "<<mode<<" not currently supported by the OSG."<<endl;
            osgGeoState->setMode(osg::GeoState::TEXGEN,osg::GeoState::OFF);
            break;
        }
        
    }

    pfMatrix* texmat = (pfMatrix*)geostate->getAttr(PFSTATE_TEXMAT);
    if (texmat)
    {
        osg::Matrix osgMatrix((*texmat)[0][0],(*texmat)[0][1],(*texmat)[0][2],(*texmat)[0][3],
                              (*texmat)[1][0],(*texmat)[1][1],(*texmat)[1][2],(*texmat)[1][3],
                              (*texmat)[2][0],(*texmat)[2][1],(*texmat)[2][2],(*texmat)[2][3],
                              (*texmat)[3][0],(*texmat)[3][1],(*texmat)[3][2],(*texmat)[3][3]);

        osg::TexMat* osgTexMat = new osg::TexMat();
        osgTexMat->copy(osgMatrix);
        osgGeoState->setAttribute(osg::GeoState::TEXMAT,osgTexMat);
    }


    return osgGeoState;
}

osg::Material* ConvertFromPerformer::visitMaterial(osg::GeoState* osgGeoState,pfMaterial* material)
{
    if (material==NULL) return NULL;

    osg::Material* osgMaterial = new osg::Material;
    if (osgGeoState) osgGeoState->setAttribute(osg::GeoState::MATERIAL,osgMaterial);

    float s = material->getShininess();
    osgMaterial->setShininess(osg::Material::FACE_FRONT_AND_BACK,s);

    float a = material->getAlpha();
    float r,g,b;

    material->getColor(PFMTL_AMBIENT,&r,&g,&b);
    osgMaterial->setAmbient(osg::Material::FACE_FRONT_AND_BACK,osg::Vec4(r,g,b,a));

    material->getColor(PFMTL_DIFFUSE,&r,&g,&b);
    osgMaterial->setDiffuse(osg::Material::FACE_FRONT_AND_BACK,osg::Vec4(r,g,b,a));

    material->getColor(PFMTL_EMISSION,&r,&g,&b);
    osgMaterial->setEmission(osg::Material::FACE_FRONT_AND_BACK,osg::Vec4(r,g,b,a));

    material->getColor(PFMTL_SPECULAR,&r,&g,&b);
    osgMaterial->setSpecular(osg::Material::FACE_FRONT_AND_BACK,osg::Vec4(r,g,b,a));

    return osgMaterial;
}

osg::Texture* ConvertFromPerformer::visitTexture(osg::GeoState* osgGeoState,pfTexture* tex)
{
    if (tex==NULL) return NULL;

    osg::Texture* osgTexture = new osg::Texture;
    _pfToOsgMap[tex] = osgTexture;

    if (osgGeoState) osgGeoState->setAttribute(osg::GeoState::TEXTURE,osgTexture);


    int repeat_r = tex->getRepeat(PFTEX_WRAP_R);
    int repeat_s = tex->getRepeat(PFTEX_WRAP_S);
    int repeat_t = tex->getRepeat(PFTEX_WRAP_T);
    
    if (repeat_r==PFTEX_CLAMP) osgTexture->setWrap(osg::Texture::WRAP_R,osg::Texture::CLAMP);
    else osgTexture->setWrap(osg::Texture::WRAP_R,osg::Texture::REPEAT);

    if (repeat_s==PFTEX_CLAMP) osgTexture->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP);
    else osgTexture->setWrap(osg::Texture::WRAP_S,osg::Texture::REPEAT);

    if (repeat_t==PFTEX_CLAMP) osgTexture->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP);
    else osgTexture->setWrap(osg::Texture::WRAP_T,osg::Texture::REPEAT);

    std::string texName = tex->getName();

    if (_saveImagesAsRGB)
    {
        std::string strippedName = osg::getStrippedName(texName);
        texName = _saveImageDirectory+strippedName+".rgb";
        tex->saveFile(texName.c_str());
    }

    if (!_saveAbsoluteImagePath) texName = osg::getSimpleFileName(texName);

    osg::Image* image = new osg::Image;
    image->setFileName(texName.c_str());
    
    osgTexture->setImage(image);

    return osgTexture;
}
