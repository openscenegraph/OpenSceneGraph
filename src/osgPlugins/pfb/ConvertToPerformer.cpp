#include "ConvertToPerformer.h"

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
#include <osg/FileNameUtils>
#include <osg/Registry>

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

extern "C" {

extern pfNode *
pfdLoadFile_osg (char *fileName)
{
    osg::Node* node = osg::loadNodeFile(fileName);
    if (node==NULL) return 0;

    ConvertToPerformer converter;
    return converter.convert(node);
}

};


ConvertToPerformer::ConvertToPerformer()
{
    setTraverseMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);

    _pfParent = NULL;
    _pfRoot = NULL;

    _gsetPrimMap[osg::GeoSet::POINTS] = PFGS_POINTS;
    _gsetPrimMap[osg::GeoSet::LINES] = PFGS_LINES;
    _gsetPrimMap[osg::GeoSet::LINE_STRIP] = PFGS_LINESTRIPS;
    _gsetPrimMap[osg::GeoSet::TRIANGLES] = PFGS_TRIS;
    _gsetPrimMap[osg::GeoSet::QUADS] = PFGS_QUADS;
    _gsetPrimMap[osg::GeoSet::TRIANGLE_STRIP] = PFGS_TRISTRIPS;
    _gsetPrimMap[osg::GeoSet::FLAT_LINE_STRIP] = PFGS_FLAT_LINESTRIPS;
    _gsetPrimMap[osg::GeoSet::FLAT_TRIANGLE_STRIP] = PFGS_FLAT_TRISTRIPS;
    _gsetPrimMap[osg::GeoSet::TRIANGLE_FAN] = PFGS_TRIFANS;
    _gsetPrimMap[osg::GeoSet::FLAT_TRIANGLE_FAN] = PFGS_FLAT_TRIFANS;
    _gsetPrimMap[osg::GeoSet::POLYGON] = PFGS_POLYS;
    _gsetPrimMap[osg::GeoSet::NO_TYPE] = PFGS_NUM_PRIMS;

    _gsetBindMap[osg::GeoSet::BIND_OFF] = PFGS_OFF;
    _gsetBindMap[osg::GeoSet::BIND_OVERALL] = PFGS_OVERALL;
    _gsetBindMap[osg::GeoSet::BIND_PERPRIM] = PFGS_PER_PRIM;
    _gsetBindMap[osg::GeoSet::BIND_PERVERTEX] = PFGS_PER_VERTEX;

              
    _gstateTypeMap[osg::GeoState::TRANSPARENCY] = PFSTATE_TRANSPARENCY;
    _gstateTypeMap[osg::GeoState::ANTIALIAS] = PFSTATE_ANTIALIAS;
    _gstateTypeMap[osg::GeoState::LIGHTING] = PFSTATE_ENLIGHTING;
    _gstateTypeMap[osg::GeoState::TEXTURE] = PFSTATE_ENTEXTURE;
    _gstateTypeMap[osg::GeoState::FOG] = PFSTATE_ENFOG;
    _gstateTypeMap[osg::GeoState::FACE_CULL] = PFSTATE_CULLFACE;
    _gstateTypeMap[osg::GeoState::WIREFRAME] = PFSTATE_ENWIREFRAME;
    _gstateTypeMap[osg::GeoState::TEXGEN] = PFSTATE_ENTEXGEN;
    _gstateTypeMap[osg::GeoState::TEXMAT] = PFSTATE_ENTEXMAT;

}

ConvertToPerformer::~ConvertToPerformer()
{
}

pfNode* ConvertToPerformer::convert(osg::Node* node)
{
    _pfRoot = NULL;
    if (node)
    {
        node->accept(*this);
    }
    return _pfRoot;
}

pfObject* ConvertToPerformer::getPfObject(osg::Object* osgObj)
{
    OsgObjectToPfObjectMap::iterator fitr = _osgToPfMap.find(osgObj);
    if (fitr != _osgToPfMap.end())
    {
        osg::notify(osg::DEBUG) << "Found shared object"<<endl;
        return (*fitr).second;
    }
    else return NULL;
}

void ConvertToPerformer::regisiterOsgObjectForPfObject(osg::Object* osgObj,pfObject* pfObj)
{
    _osgToPfMap[osgObj] = pfObj;
}


void ConvertToPerformer::apply(osg::Node& node)
{
    node.traverse(*this);
}

void ConvertToPerformer::apply(osg::Group& node)
{
    pfGroup* parent = _pfParent;

    pfGroup* pf_group = dynamic_cast<pfGroup*>(getPfObject(&node));
    if (pf_group)
    {
        if (_pfParent) _pfParent->addChild(pf_group);
        return;
    }

    pf_group = new pfGroup;
    if (!_pfRoot) _pfRoot = pf_group;
    if (_pfParent) _pfParent->addChild(pf_group);

    regisiterOsgObjectForPfObject(&node,pf_group);

    if (!node.getName().empty()) pf_group->setName(node.getName().c_str());

    _pfParent = pf_group;

    node.traverse(*this);

    _pfParent = parent;
}

void ConvertToPerformer::apply(osg::DCS& osgDCS)
{
    pfGroup* parent = _pfParent;

    pfDCS* pf_dcs = dynamic_cast<pfDCS*>(getPfObject(&osgDCS));
    if (pf_dcs)
    {
        if (_pfParent) _pfParent->addChild(pf_dcs);
        return;
    }

    pf_dcs = new pfDCS;
    if (!_pfRoot) _pfRoot = pf_dcs;
    if (_pfParent) _pfParent->addChild(pf_dcs);

    regisiterOsgObjectForPfObject(&osgDCS,pf_dcs);

    if (!osgDCS.getName().empty()) pf_dcs->setName(osgDCS.getName().c_str());

    osg::Matrix* matrix = osgDCS.getMatrix();
    
    pfMatrix pf_matrix(matrix->_mat[0][0],matrix->_mat[0][1],matrix->_mat[0][2],matrix->_mat[0][3],
                       matrix->_mat[1][0],matrix->_mat[1][1],matrix->_mat[1][2],matrix->_mat[1][3],
                       matrix->_mat[2][0],matrix->_mat[2][1],matrix->_mat[2][2],matrix->_mat[2][3],
                       matrix->_mat[3][0],matrix->_mat[3][1],matrix->_mat[3][2],matrix->_mat[3][3]);
    
    pf_dcs->setMat(pf_matrix);


    _pfParent = pf_dcs;

    osgDCS.traverse(*this);

    _pfParent = parent;
    
}

void ConvertToPerformer::apply(osg::Switch& node)
{
    pfGroup* parent = _pfParent;

    pfSwitch* pf_switch = dynamic_cast<pfSwitch*>(getPfObject(&node));
    if (pf_switch)
    {
        if (_pfParent) _pfParent->addChild(pf_switch);
        return;
    }

    pf_switch = new pfSwitch;
    if (!_pfRoot) _pfRoot = pf_switch;
    if (_pfParent) _pfParent->addChild(pf_switch);

    regisiterOsgObjectForPfObject(&node,pf_switch);

    if (!node.getName().empty()) pf_switch->setName(node.getName().c_str());

    _pfParent = pf_switch;

    node.traverse(*this);

    _pfParent = parent;
}

void ConvertToPerformer::apply(osg::LOD& node)
{
    pfGroup* parent = _pfParent;

    pfLOD* pf_lod = dynamic_cast<pfLOD*>(getPfObject(&node));
    if (pf_lod)
    {
        if (_pfParent) _pfParent->addChild(pf_lod);
        return;
    }

    pf_lod = new pfLOD;
    if (!_pfRoot) _pfRoot = pf_lod;
    if (_pfParent) _pfParent->addChild(pf_lod);

    regisiterOsgObjectForPfObject(&node,pf_lod);

    if (!node.getName().empty()) pf_lod->setName(node.getName().c_str());

    _pfParent = pf_lod;

    node.traverse(*this);

    _pfParent = parent;
}

void ConvertToPerformer::apply(osg::Scene& scene)
{
    pfGroup* parent = _pfParent;

    pfScene* pf_scene = dynamic_cast<pfScene*>(getPfObject(&scene));
    if (pf_scene)
    {
        if (_pfParent) _pfParent->addChild(pf_scene);
        return;
    }

    pf_scene = new pfScene;
    if (!_pfRoot) _pfRoot = pf_scene;
    if (_pfParent) _pfParent->addChild(pf_scene);

    regisiterOsgObjectForPfObject(&scene,pf_scene);

    if (!scene.getName().empty()) pf_scene->setName(scene.getName().c_str());

    _pfParent = pf_scene;

    scene.traverse(*this);

    _pfParent = parent;
}


void ConvertToPerformer::apply(osg::Billboard& node)
{
    pfGroup* parent = _pfParent;

    pfBillboard* pf_billboard = dynamic_cast<pfBillboard*>(getPfObject(&node));
    if (pf_billboard)
    {
        if (_pfParent) _pfParent->addChild(pf_billboard);
        return;
    }

    pf_billboard = new pfBillboard;
    if (!_pfRoot) _pfRoot = pf_billboard;
    if (_pfParent) _pfParent->addChild(pf_billboard);

    regisiterOsgObjectForPfObject(&node,pf_billboard);

    if (!node.getName().empty()) pf_billboard->setName(node.getName().c_str());

    for(int i=0;i<node.getNumGeosets();++i)
    {
        pfGeoSet* pf_geoset = visitGeoSet(node.getGeoSet(i));
        if (pf_geoset) pf_billboard->addGSet(pf_geoset);
    }

    _pfParent = parent;
}

void ConvertToPerformer::apply(osg::Geode& node)
{
    pfGroup* parent = _pfParent;

    pfGeode* pf_geode = dynamic_cast<pfGeode*>(getPfObject(&node));
    if (pf_geode)
    {
        if (_pfParent) _pfParent->addChild(pf_geode);
        return;
    }

    pf_geode = new pfGeode;
    if (!_pfRoot) _pfRoot = pf_geode;
    if (_pfParent) _pfParent->addChild(pf_geode);

    regisiterOsgObjectForPfObject(&node,pf_geode);

    if (!node.getName().empty()) pf_geode->setName(node.getName().c_str());

    for(int i=0;i<node.getNumGeosets();++i)
    {
        pfGeoSet* pf_geoset = visitGeoSet(node.getGeoSet(i));
        if (pf_geoset) pf_geode->addGSet(pf_geoset);
    }

    _pfParent = parent;
}

pfGeoSet* ConvertToPerformer::visitGeoSet(osg::GeoSet* geoset)
{
    if (geoset==NULL) return NULL;

    void* arena = pfGetSharedArena();

    pfGeoSet* pf_geoset = new pfGeoSet();

    pf_geoset->setGState(visitGeoState(geoset->getGeoState()));

    int i;

    // number of prims
    int np = geoset->getNumPrims();
    int *plen = geoset->getPrimLengths();

    // Number of verticies (may be different than number of coords)
    geoset->computeNumVerts();

    pf_geoset->setPrimType(_gsetPrimMap[geoset->getPrimType()]);
    pf_geoset->setNumPrims(np);

    if (plen)
    {
        //int *pf_plen = new int [np];
        int* pf_plen = (int*) pfMalloc(sizeof(int) * np, arena);
        for(i=0;i<np;++i) pf_plen[i] = plen[i];
        pf_geoset->setPrimLengths(pf_plen);
    }

    osg::Vec3 *coords = geoset->getCoords();
    osg::ushort *ilist = geoset->getCIndex();


    // copy the vertex coordinates across.
    if( coords )
    {

        int cc = geoset->getNumCoords();

        pfVec3* pf_coords = (pfVec3*) pfMalloc(sizeof(pfVec3) * cc, arena);

        for( i = 0; i < cc; i++ )
        {
            pf_coords[i][0] = coords[i][0];
            pf_coords[i][1] = coords[i][1];
            pf_coords[i][2] = coords[i][2];
        }

        if(ilist)
        {
            int ni=geoset->getNumIndices();
            ushort* pf_cindex = (ushort*) pfMalloc(sizeof(ushort) * ni, arena);
            for( i = 0; i < ni; i++ )
            {
                pf_cindex[i] = ilist[i];
            }
            pf_geoset->setAttr( PFGS_COORD3, PFGS_PER_VERTEX, pf_coords, pf_cindex );
        }
        else
        {
            pf_geoset->setAttr( PFGS_COORD3, PFGS_PER_VERTEX, pf_coords, NULL );
        }

    }

    osg::Vec3 *norms = geoset->getNormals();
    ilist = geoset->getNIndex();

    // copy normals
    if(norms)
    {
        int bind = _gsetBindMap[geoset->getNormalBinding()];

        int cc = geoset->getNumNormals();

        pfVec3* pf_norms = (pfVec3*) pfMalloc(sizeof(pfVec3) * cc, arena);

        for( i = 0; i < cc; i++ )
        {
            pf_norms[i][0] = norms[i][0];
            pf_norms[i][1] = norms[i][1];
            pf_norms[i][2] = norms[i][2];
        }

        if(ilist)
        {
            int ni=geoset->getNumNIndices();
            ushort* pf_nindex = (ushort*) pfMalloc(sizeof(ushort) * ni, arena);
            for( i = 0; i < ni; i++ )
            {
                pf_nindex[i] = ilist[i];
            }
            pf_geoset->setAttr(PFGS_NORMAL3, bind, pf_norms,pf_nindex);
        }
        else
        {
            pf_geoset->setAttr(PFGS_NORMAL3, bind, pf_norms,NULL);
        }

    }

    osg::Vec4 *colors = geoset->getColors();
    ilist = geoset->getColIndex();

    // copy colors
    if(colors)
    {
        int bind = _gsetBindMap[geoset->getColorBinding()];

        int cc = geoset->getNumColors();

        pfVec4* pf_colors = (pfVec4*) pfMalloc(sizeof(pfVec4) * cc, arena);

        for( i = 0; i < cc; i++ )
        {
            pf_colors[i][0] = colors[i][0];
            pf_colors[i][1] = colors[i][1];
            pf_colors[i][2] = colors[i][2];
            pf_colors[i][3] = colors[i][3];
        }

        if(ilist)
        {
            int ni=geoset->getNumCIndices();
            ushort* pf_cindex = (ushort*) pfMalloc(sizeof(ushort) * ni, arena);
            for( i = 0; i < ni; i++ )
            {
                pf_cindex[i] = ilist[i];
            }
            pf_geoset->setAttr(PFGS_COLOR4, bind, pf_colors,pf_cindex);
        }
        else
        {
            pf_geoset->setAttr(PFGS_COLOR4, bind, pf_colors,NULL);
        }

    }

// 
//     pfVec2 *tcoords;
//     geoset->getAttrLists( PFGS_TEXCOORD2,  (void **)&tcoords, &ilist );
// 
//     // copy texture coords
//     if(tcoords)
//     {
//         int bind = geoset->getAttrBind( PFGS_TEXCOORD2 );
//         int nn = bind == PFGS_OFF ? 0 :
//                  bind == PFGS_OVERALL ? 1 :
//                  bind == PFGS_PER_PRIM ? geoset->getNumPrims() :
//                  bind == PFGS_PER_VERTEX ? nv : 0;
// 
//         // set the normal binding type.
//         pf_geoset->setTextureBinding(_gsetBindMap[bind]);
// 
//         // calc the maximum num of vertex from the index list.
//         int cc;
//         if (ilist)
//         {
//             cc = 0;
//             for( i = 0; i < nv; i++ )
//                 if( ilist[i] > cc ) cc = ilist[i];
//             cc++;
//         }
//         else
//             cc = nn;
// 
// 
//         osg::Vec2* osg_tcoords = new osg::Vec2 [cc];
//         for( i = 0; i < cc; i++ )
//         {
//             osg_tcoords[i][0] = tcoords[i][0];
//             osg_tcoords[i][1] = tcoords[i][1];
//         }
// 
//         if(ilist)
//         {
//             osg::ushort* osg_cindex = new osg::ushort [nn];
//             for( i = 0; i < nn; i++ )
//             {
//                 osg_cindex[i] = ilist[i];
//             }
//             pf_geoset->setTextureCoords(osg_tcoords, osg_cindex );
//         }
//         else
//         {
//             pf_geoset->setTextureCoords(osg_tcoords);
//         }
// 
//     }
// 
//     pfVec4 *colors;
//     geoset->getAttrLists( PFGS_COLOR4,  (void **)&colors, &ilist );
// 
//     // copy color coords
//     if(colors)
//     {
//         int bind = geoset->getAttrBind( PFGS_COLOR4 );
//         int nn = bind == PFGS_OFF ? 0 :
//                  bind == PFGS_OVERALL ? 1 :
//                  bind == PFGS_PER_PRIM ? geoset->getNumPrims() :
//                  bind == PFGS_PER_VERTEX ? nv-flat_shaded_offset : 0;
// 
//         // set the normal binding type.
//         pf_geoset->setColorBinding(_gsetBindMap[bind]);
// 
//         // calc the maximum num of vertex from the index list.
//         int cc;
//         if (ilist)
//         {
//             cc = 0;
//             for( i = 0; i < nn; i++ )
//                 if( ilist[i] > cc ) cc = ilist[i];
//             cc++;
//         }
//         else
//             cc = nn;
// 
// 
//         osg::Vec4* osg_colors = new osg::Vec4 [cc];
//         for( i = 0; i < cc; i++ )
//         {
//             osg_colors[i][0] = colors[i][0];
//             osg_colors[i][1] = colors[i][1];
//             osg_colors[i][2] = colors[i][2];
//             osg_colors[i][3] = colors[i][3];
//         }
// 
//         if(ilist)
//         {
//             osg::ushort* osg_cindex = new osg::ushort [nn];
//             for( i = 0; i < nn; i++ )
//             {
//                 osg_cindex[i] = ilist[i];
//             }
//             pf_geoset->setColors(osg_colors, osg_cindex );
//         }
//         else
//         {
//             pf_geoset->setColors(osg_colors);
//         }
// 
//     }
// 


    return pf_geoset;
}

pfGeoState* ConvertToPerformer::visitGeoState(osg::GeoState* geostate)
{
    if (geostate==NULL) return NULL;

    pfGeoState* pf_geostate = new pfGeoState();

    switch(geostate->getMode(osg::GeoState::LIGHTING))
    {
        case(osg::GeoState::OVERRIDE_ON): 
        case(osg::GeoState::ON): pf_geostate->setMode(PFSTATE_ENLIGHTING,PF_ON);break;
        case(osg::GeoState::OVERRIDE_OFF): 
        case(osg::GeoState::OFF): pf_geostate->setMode(PFSTATE_ENLIGHTING,PF_OFF);break;
    }

    switch(geostate->getMode(osg::GeoState::TEXTURE))
    {
        case(osg::GeoState::OVERRIDE_ON): 
        case(osg::GeoState::ON): pf_geostate->setMode(PFSTATE_ENTEXTURE,PF_ON);break;
        case(osg::GeoState::OVERRIDE_OFF): 
        case(osg::GeoState::OFF): pf_geostate->setMode(PFSTATE_ENTEXTURE,PF_OFF);break;
    }

    return pf_geostate;
}

pfMaterial* ConvertToPerformer::visitMaterial(osg::Material* material)
{
    if (material==NULL) return NULL;

    pfMaterial* pf_material = new pfMaterial();

    return pf_material;
}

pfTexture* ConvertToPerformer::visitTexture(osg::Texture* tex)
{
    if (tex==NULL) return NULL;

    pfTexture* pf_texture = new pfTexture();

    return pf_texture;
}

