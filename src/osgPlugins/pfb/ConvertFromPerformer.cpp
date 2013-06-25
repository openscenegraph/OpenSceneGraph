// -*-c++-*-

#include "ConvertFromPerformer.h"

#include <osg/Group>
#include <osg/MatrixTransform>
#include <osg/LOD>
#include <osg/Switch>
#include <osg/Geode>
#include <osg/Billboard>
#include <osg/Texture2D>
#include <osg/Image>
#include <osg/CullFace>
#include <osg/TexGen>
#include <osg/TexEnv>
#include <osg/TexMat>
#include <osg/Material>
#include <osg/Notify>
#include <osg/Geometry>
#include <osg/Sequence>
#include <osg/ShadeModel>
#include <osg/Depth>
#include <osg/AlphaFunc>

#include <osgDB/FileNameUtils>
#include <osgDB/Registry>
#include <osgDB/WriteFile>

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

#ifdef WIN32
#define snprintf _snprintf
#endif

// Need to undefine these because Performer defines them and it causes a
// compiler error in ConvertFromPerformer::visitBillboard.
#ifdef   AXIAL_ROT
#undef   AXIAL_ROT
#endif

#ifdef   POINT_ROT_EYE
#undef   POINT_ROT_EYE
#endif

#ifdef   POINT_ROT_WORLD
#undef   POINT_ROT_WORLD
#endif

extern "C"
{

    extern int
        pfdStoreFile_osg (pfNode* root, char *fileName)
    {
        ConvertFromPerformer converter;
        osg::Node* node =  converter.convert(root);

        if (node==NULL) return 0;
        if (osgDB::writeNodeFile(*node,fileName)) return 1;
        else return 0;
    }

}


ConvertFromPerformer::ConvertFromPerformer()
{
    _osgRoot = NULL;

    _gsetBindMap[PFGS_OFF] = deprecated_osg::Geometry::BIND_OFF;
    _gsetBindMap[PFGS_OVERALL] = deprecated_osg::Geometry::BIND_OVERALL;
    _gsetBindMap[PFGS_PER_PRIM] = deprecated_osg::Geometry::BIND_PER_PRIMITIVE;
    _gsetBindMap[PFGS_PER_VERTEX] = deprecated_osg::Geometry::BIND_PER_VERTEX;

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
        //         OSG_DEBUG << "Found shared object"<<std::endl;
        return (*fitr).second;
    }
    else return NULL;
}


void ConvertFromPerformer::registerPfObjectForOsgObject(pfObject* pfObj,osg::Object* osgObj)
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
    osg::Group* osgScene = dynamic_cast<osg::Group*>(getOsgObject(scene));
    if (osgScene)
    {
        if (osgParent) osgParent->addChild(osgScene);
        return osgScene;
    }

    osgScene = new osg::Group;
    if (osgParent) osgParent->addChild(osgScene);

    registerPfObjectForOsgObject(scene,osgScene);

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

    registerPfObjectForOsgObject(group,osgGroup);

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

    registerPfObjectForOsgObject(lod,osgLOD);

    const char* name = lod->getName();
    if (name) osgLOD->setName(name);

    pfVec3 center;
    lod->getCenter(center);
    osg::Vec3 osgCenter(center[0],center[1],center[2]);
    osgLOD->setCenter(osgCenter);

    int i;
    for(i=0;i<lod->getNumRanges()-1;++i)
    {
        osgLOD->setRange(i,lod->getRange(i),lod->getRange(i+1));
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
    osgSwitch->setAllChildrenOff();
    if (osgParent) osgParent->addChild(osgSwitch);

    registerPfObjectForOsgObject(switchNode,osgSwitch);

    const char* name = switchNode->getName();
    if (name) osgSwitch->setName(name);

    float val = switchNode->getVal();
    if (val==PFSWITCH_ON)
    {
        osgSwitch->setAllChildrenOn();
    }
    else if (val==PFSWITCH_OFF)
    {
        osgSwitch->setAllChildrenOff();
    }
    else
    {
        osgSwitch->setSingleChildOn((unsigned int)val);
    }

    for(int i=0;i<switchNode->getNumChildren();++i)
    {
        visitNode(osgSwitch,switchNode->getChild(i));
    }
    return (osg::Node*)osgSwitch;
}


osg::Node* ConvertFromPerformer::visitSequence(osg::Group* osgParent,
                                               pfSequence* sequence)
{

    //OSG_WARN<<"Warning : cannot convert pfSequence as no osg::Sequence exists, using osg::Switch instead."<<std::endl;

    osg::Sequence* osgSequence = dynamic_cast<osg::Sequence*>(getOsgObject(sequence));
    if (osgSequence)
    {
        if (osgParent) osgParent->addChild(osgSequence);
        return osgSequence;
    }

    osgSequence = new osg::Sequence;
    if (osgParent) osgParent->addChild(osgSequence);

    registerPfObjectForOsgObject(sequence,osgSequence);

#if 0
    if (sequence->getNumChildren()>0)
    {
        // set switch to first child as a 'hack' to prevent all
        // children being traversed during rendering.  Note,
        // once osg::Sequence has been implemented this can all
        // be removed.
        osgSequence->setValue(0);
    }
#endif

    // add children
    for(int i=0;i<sequence->getNumChildren();++i)
    {
        //OSG_WARN << "child " << i << " time " << sequence->getTime(i) << std::endl;
        osgSequence->setTime(i, sequence->getTime(i));
        visitNode(osgSequence,sequence->getChild(i));
    }

    // interval
    int mode, begin, end;
    sequence->getInterval(&mode, &begin, &end);

    //OSG_WARN << "loop " << mode << std::endl;
    osg::Sequence::LoopMode loopMode = osg::Sequence::LOOP;
    if (mode == PFSEQ_SWING)
        loopMode = osg::Sequence::SWING;
    osgSequence->setInterval(loopMode, begin, end);

    // duration
    float speed;
    int repeat;
    sequence->getDuration(&speed, &repeat);
    osgSequence->setDuration(speed, repeat);

    // mode
    mode = sequence->getMode();

    osg::Sequence::SequenceMode seqMode = osg::Sequence::START;
    switch (mode) {
    case PFSEQ_STOP:
        seqMode = osg::Sequence::STOP;
        break;
    case PFSEQ_PAUSE:
        seqMode = osg::Sequence::PAUSE;
        break;
    }
    osgSequence->setMode(seqMode);

    return (osg::Node*)osgSequence;
}


osg::Node* ConvertFromPerformer::visitDCS(osg::Group* osgParent,pfDCS* dcs)
{

    osg::MatrixTransform* osgTransform = dynamic_cast<osg::MatrixTransform*>(getOsgObject(dcs));
    if (osgTransform)
    {
        if (osgParent) osgParent->addChild(osgTransform);
        return osgTransform;
    }

    osgTransform = new osg::MatrixTransform;
    if (osgParent) osgParent->addChild(osgTransform);

    registerPfObjectForOsgObject(dcs,osgTransform);

    const char* name = dcs->getName();
    if (name) osgTransform->setName(name);

    pfMatrix matrix;
    dcs->getMat(matrix);

    osg::Matrix osgMatrix(matrix[0][0],matrix[0][1],matrix[0][2],matrix[0][3],
        matrix[1][0],matrix[1][1],matrix[1][2],matrix[1][3],
        matrix[2][0],matrix[2][1],matrix[2][2],matrix[2][3],
        matrix[3][0],matrix[3][1],matrix[3][2],matrix[3][3]);

    osgTransform->setMatrix(osgMatrix);

    for(int i=0;i<dcs->getNumChildren();++i)
    {
        visitNode(osgTransform,dcs->getChild(i));
    }
    return (osg::Node*)osgTransform;
}


osg::Node* ConvertFromPerformer::visitSCS(osg::Group* osgParent,pfSCS* scs)
{
    // note the OSG does not currently have a SCS, so use DCS instead.
    osg::MatrixTransform* osgTransform = dynamic_cast<osg::MatrixTransform*>(getOsgObject(scs));
    if (osgTransform)
    {
        if (osgParent) osgParent->addChild(osgTransform);
        return osgTransform;
    }

    osgTransform = new osg::MatrixTransform;
    if (osgParent) osgParent->addChild(osgTransform);

    osgTransform->setDataVariance(osg::Object::STATIC);

    registerPfObjectForOsgObject(scs,osgTransform);

    const char* name = scs->getName();
    if (name) osgTransform->setName(name);

    pfMatrix matrix;
    scs->getMat(matrix);
    osg::Matrix osgMatrix(matrix[0][0],matrix[0][1],matrix[0][2],matrix[0][3],
        matrix[1][0],matrix[1][1],matrix[1][2],matrix[1][3],
        matrix[2][0],matrix[2][1],matrix[2][2],matrix[2][3],
        matrix[3][0],matrix[3][1],matrix[3][2],matrix[3][3]);

    osgTransform->setMatrix(osgMatrix);

    for(int i=0;i<scs->getNumChildren();++i)
    {
        visitNode(osgTransform,scs->getChild(i));
    }
    return (osg::Node*)osgTransform;
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

    registerPfObjectForOsgObject(geode,osgGeode);

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

    registerPfObjectForOsgObject(billboard,osgBillboard);

    const char* name = billboard->getName();
    if (name) osgBillboard->setName(name);

    pfVec3 axis;
    billboard->getAxis(axis);
    osgBillboard->setAxis(osg::Vec3(axis[0],axis[1],axis[2]));

    switch( billboard->getMode( PFBB_ROT ) )
    {
    case PFBB_AXIAL_ROT:
       osgBillboard->setMode( osg::Billboard::AXIAL_ROT );
       break;
    case PFBB_POINT_ROT_EYE:
       osgBillboard->setMode( osg::Billboard::POINT_ROT_EYE );
       break;
    case PFBB_POINT_ROT_WORLD:
       osgBillboard->setMode( osg::Billboard::POINT_ROT_WORLD );
       break;
    }

    for(int i=0;i<billboard->getNumGSets();++i)
    {
                                 /* osg::GeoSet* osggset = */
        visitGeoSet(osgBillboard,billboard->getGSet(i));
        pfVec3 pos;
        billboard->getPos(i,pos);
        osgBillboard->setPosition(i,osg::Vec3(pos[0],pos[1],pos[2]));
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
        case PFGS_TRIFANS :
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


osg::Drawable* ConvertFromPerformer::visitGeoSet(osg::Geode* osgGeode,pfGeoSet* geoset)
{
    if (geoset==NULL) return NULL;

    osg::Drawable* osgDrawable = dynamic_cast<osg::Drawable*>(getOsgObject(geoset));
    if (osgDrawable)
    {
        if (osgGeode) osgGeode->addDrawable(osgDrawable);
        return osgDrawable;
    }

    // we'll make it easy to convert by using the Performer style osg::GeoSet,
    // and then convert back to a deprecated_osg::Geometry afterwards.
    //osg::ref_ptr<osg::GeoSet> geom = new osg::GeoSet;
    deprecated_osg::Geometry* geom = new deprecated_osg::Geometry;

    int i;

    // number of prims
    int np = geoset->getNumPrims();
    int *plen = geoset->getPrimLengths();

    // Number of verticies (may be different than number of coords)
    int nv = getNumVerts( geoset );

    int prim = geoset->getPrimType();
    int flat_shaded_skip_per_primitive=0;
    if (prim == PFGS_FLAT_LINESTRIPS)       flat_shaded_skip_per_primitive=1;
    else if (prim == PFGS_FLAT_TRISTRIPS)   flat_shaded_skip_per_primitive=2;
    else if (prim == PFGS_FLAT_TRIFANS)     flat_shaded_skip_per_primitive=2;

    int flat_shaded_skip_all_primitives=flat_shaded_skip_per_primitive*np;

    // create the primitive sets.
    switch( geoset->getPrimType() )
    {
        case PFGS_POINTS :
            geom->addPrimitiveSet(new osg::DrawArrays(GL_POINTS,0,np));
            break;

        case PFGS_LINES :
            geom->addPrimitiveSet(new osg::DrawArrays(GL_LINES,0,2*np));
            break;

        case PFGS_TRIS :
            geom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES,0,3*np));
            break;

        case PFGS_QUADS :
            geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4*np));
            break;

        case PFGS_TRISTRIPS :
            geom->addPrimitiveSet(new osg::DrawArrayLengths(GL_TRIANGLE_STRIP,0,np,plen));
            break;

        case PFGS_TRIFANS :
            geom->addPrimitiveSet(new osg::DrawArrayLengths(GL_TRIANGLE_FAN,0,np,plen));
            break;

        case PFGS_FLAT_TRISTRIPS :
            geom->addPrimitiveSet(new osg::DrawArrayLengths(GL_TRIANGLE_STRIP,0,np,plen));
            break;

        case PFGS_POLYS :
            geom->addPrimitiveSet(new osg::DrawArrayLengths(GL_POLYGON,0,np,plen));
            break;

        case PFGS_LINESTRIPS :
            geom->addPrimitiveSet(new osg::DrawArrayLengths(GL_LINE_STRIP,0,np,plen));
            break;

        case PFGS_FLAT_LINESTRIPS :
            geom->addPrimitiveSet(new osg::DrawArrayLengths(GL_LINE_STRIP,0,np,plen));
            break;

        default:
           OSG_INFO << "ConvertFromPerformer::visitGeoSet: Osg can't convert primitive "
                     << geoset->getPrimType()
                     << std::endl;
            break;

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

        osg::Vec3Array* osg_coords = new osg::Vec3Array(cc);
        for( i = 0; i < cc; i++ )
        {
            (*osg_coords)[i][0] = coords[i][0];
            (*osg_coords)[i][1] = coords[i][1];
            (*osg_coords)[i][2] = coords[i][2];
        }

        geom->setVertexArray(osg_coords);

        if(ilist)
        {
            geom->setVertexIndices(new osg::UShortArray(nv,ilist));
        }

    }


    pfVec2 *tcoords;
    geoset->getAttrLists( PFGS_TEXCOORD2,  (void **)&tcoords, &ilist );

    // copy texture coords
    if(tcoords)
    {
        int bind = geoset->getAttrBind( PFGS_TEXCOORD2 );


        if (bind==PFGS_PER_VERTEX && bind != PFGS_OFF)
        {
            int nn = bind == PFGS_OFF ? 0 :
                     bind == PFGS_OVERALL ? 1 :
                     bind == PFGS_PER_PRIM ? geoset->getNumPrims() :
                     bind == PFGS_PER_VERTEX ? nv : 0;

            // set the normal binding type.
            //geom->setTextureBinding(_gsetBindMap[bind]);

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

            osg::Vec2Array* osg_tcoords = new osg::Vec2Array(cc);
            for( i = 0; i < cc; i++ )
            {
                (*osg_tcoords)[i][0] = tcoords[i][0];
                (*osg_tcoords)[i][1] = tcoords[i][1];
            }

            geom->setTexCoordArray(0,osg_tcoords);

            if(ilist)
            {
                geom->setTexCoordIndices(0,new osg::UShortArray(nn,ilist));
            }
        }
        else
        {
            OSG_INFO<<"OSG can't handle non PER_VERTEX tex coord binding at present"<<std::endl;
        }

    }

    pfVec3 *norms;
    geoset->getAttrLists( PFGS_NORMAL3,  (void **)&norms, &ilist );

    // copy normals
    if(norms)
    {
        int bind = geoset->getAttrBind( PFGS_NORMAL3 );
        if (bind==PFGS_PER_VERTEX && flat_shaded_skip_all_primitives!=0)
        {
            // handle flat shaded skip of normals.
            int nn = nv-flat_shaded_skip_all_primitives;


            if (ilist)
            {
                // calc the maximum num of vertex from the index list.
                int cc = 0;
                for( i = 0; i < nn; i++ )
                    if( ilist[i] > cc ) cc = ilist[i];
                cc++;

                // straight forward mapping of normals across.
                osg::Vec3Array* osg_norms = new osg::Vec3Array(cc);
                for( i = 0; i < cc; i++ )
                {
                    (*osg_norms)[i][0] = norms[i][0];
                    (*osg_norms)[i][1] = norms[i][1];
                    (*osg_norms)[i][2] = norms[i][2];
                }
                geom->setNormalArray(osg_norms);

                // set the normal binding type.
                geom->setNormalBinding(_gsetBindMap[bind]);

                osg::UShortArray* osg_indices = new osg::UShortArray;
                osg_indices->reserve(nv);

                int ni=0;
                for( i = 0; i < np; ++i)
                {
                    for(int si=0;si<flat_shaded_skip_per_primitive;++si)
                    {
                        osg_indices->push_back(ilist[ni]);
                    }
                    for( int pi=0; pi < plen[i]-flat_shaded_skip_per_primitive; ++pi)
                    {
                        osg_indices->push_back(ilist[ni++]);
                    }
                }

                if (ni!=nn)
                {
                    OSG_INFO << "1 ni!=nn"<<std::endl;
                }

                geom->setNormalIndices(osg_indices);


            }
            else
            {
                osg::Vec3Array* osg_norms = new osg::Vec3Array;
                osg_norms->reserve(nv);

                int ni=0;
                for( i = 0; i < np; ++i)
                {
                    for(int si=0;si<flat_shaded_skip_per_primitive;++si)
                    {
                        osg_norms->push_back(osg::Vec3(norms[ni][0],norms[ni][1],norms[ni][2]));
                    }
                    for( int pi=0; pi < plen[i]-flat_shaded_skip_per_primitive; ++pi)
                    {
                        osg_norms->push_back(osg::Vec3(norms[ni][0],norms[ni][1],norms[ni][2]));
                        ni++;
                    }
                }

                geom->setNormalArray(osg_norms);
            }

        }
        else
        {
            int nn = bind == PFGS_OFF ? 0 :
                     bind == PFGS_OVERALL ? 1 :
                     bind == PFGS_PER_PRIM ? geoset->getNumPrims() :
                     bind == PFGS_PER_VERTEX ? nv : 0;

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

            osg::Vec3Array* osg_norms = new osg::Vec3Array(cc);
            for( i = 0; i < cc; i++ )
            {
                (*osg_norms)[i][0] = norms[i][0];
                (*osg_norms)[i][1] = norms[i][1];
                (*osg_norms)[i][2] = norms[i][2];
            }
            geom->setNormalArray(osg_norms);

            // set the normal binding type.
            geom->setNormalBinding(_gsetBindMap[bind]);

            if(ilist)
            {
                geom->setNormalIndices(new osg::UShortArray(nn,ilist));
            }

        }
    }

    pfVec4 *colors;
    geoset->getAttrLists( PFGS_COLOR4,  (void **)&colors, &ilist );

    // copy color coords
    if(colors)
    {
        int bind = geoset->getAttrBind( PFGS_COLOR4 );
        if (bind==PFGS_PER_VERTEX && flat_shaded_skip_all_primitives!=0)
        {
            // handle flat shaded skip of colours.
            // handle flat shaded skip of normals.
            int nn = nv-flat_shaded_skip_all_primitives;

            if (ilist)
            {
                // calc the maximum num of vertex from the index list.
                int cc = 0;
                for( i = 0; i < nn; i++ )
                    if( ilist[i] > cc ) cc = ilist[i];
                cc++;

                // straight forward mapping of normals across.
                osg::Vec4Array* osg_colors = new osg::Vec4Array(cc);
                for( i = 0; i < cc; i++ )
                {
                    (*osg_colors)[i][0] = colors[i][0];
                    (*osg_colors)[i][1] = colors[i][1];
                    (*osg_colors)[i][2] = colors[i][2];
                    (*osg_colors)[i][3] = colors[i][3];
                }
                geom->setColorArray(osg_colors);

                // set the color binding type.
                geom->setColorBinding(_gsetBindMap[bind]);


                osg::UShortArray* osg_indices = new osg::UShortArray;
                osg_indices->reserve(nv);

                int ni=0;
                for( i = 0; i < np; ++i)
                {
                    for(int si=0;si<flat_shaded_skip_per_primitive;++si)
                    {
                        osg_indices->push_back(ilist[ni]);
                    }
                    for( int pi=0; pi < plen[i]-flat_shaded_skip_per_primitive; ++pi)
                    {
                        osg_indices->push_back(ilist[ni++]);
                    }
                }

                if (ni!=nn)
                {
                    OSG_INFO << "1 ni!=nn"<<std::endl;
                }

                geom->setColorIndices(osg_indices);


            }
            else
            {
                osg::Vec4Array* osg_colors = new osg::Vec4Array;
                osg_colors->reserve(nv);

                int ni=0;
                for( i = 0; i < np; ++i)
                {
                    for(int si=0;si<flat_shaded_skip_per_primitive;++si)
                    {
                        osg_colors->push_back(osg::Vec4(colors[ni][0],colors[ni][1],colors[ni][2],colors[ni][3]));
                    }
                    for( int pi=0; pi < plen[i]-flat_shaded_skip_per_primitive; ++pi)
                    {
                        osg_colors->push_back(osg::Vec4(colors[ni][0],colors[ni][1],colors[ni][2],colors[ni][3]));
                        ni++;
                    }
                }

                geom->setColorArray(osg_colors);

                // set the color binding type.
                geom->setColorBinding(_gsetBindMap[bind]);
            }
        }
        else
        {
            int nn = bind == PFGS_OFF ? 0 :
                     bind == PFGS_OVERALL ? 1 :
                     bind == PFGS_PER_PRIM ? geoset->getNumPrims() :
                     bind == PFGS_PER_VERTEX ? nv : 0;

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

            osg::Vec4Array* osg_colors = new osg::Vec4Array(cc);
            for( i = 0; i < cc; i++ )
            {
                (*osg_colors)[i][0] = colors[i][0];
                (*osg_colors)[i][1] = colors[i][1];
                (*osg_colors)[i][2] = colors[i][2];
                (*osg_colors)[i][3] = colors[i][3];
            }

            geom->setColorArray(osg_colors);

            // set the color binding type.
            geom->setColorBinding(_gsetBindMap[bind]);


            if(ilist)
            {
                geom->setColorIndices(new osg::UShortArray(nn,ilist));
            }

        }
    }
    else
    {
    }



    visitGeoState(geom,geoset->getGState());

    if (flat_shaded_skip_per_primitive)
    {
        osg::StateSet* stateset = geom->getOrCreateStateSet();
        osg::ShadeModel* shademodel = dynamic_cast<osg::ShadeModel*>(stateset->getAttribute(osg::StateAttribute::SHADEMODEL));
        if (!shademodel)
        {
            shademodel = new osg::ShadeModel;
            stateset->setAttribute(shademodel);
        }
        shademodel->setMode( osg::ShadeModel::FLAT );
    }

    osgGeode->addDrawable(geom);
    registerPfObjectForOsgObject(geoset,geom);

    return geom;
}


osg::StateSet* ConvertFromPerformer::visitGeoState(osg::Drawable* osgDrawable,pfGeoState* geostate)
{
    if (geostate==NULL) return NULL;

    osg::StateSet* osgStateSet = dynamic_cast<osg::StateSet*>(getOsgObject(geostate));
    if (osgStateSet)
    {
        if (osgDrawable) osgDrawable->setStateSet(osgStateSet);
        return osgStateSet;
    }

    osgStateSet = new osg::StateSet;
    if (osgDrawable) osgDrawable->setStateSet(osgStateSet);

    registerPfObjectForOsgObject(geostate,osgStateSet);

    // Don could you fill in some of these blanks???
    unsigned int inherit = geostate->getInherit();
    //     OSG_DEBUG << endl << "Inherit = "<<inherit<<std::endl;
    //     if (inherit & PFSTATE_TRANSPARENCY) OSG_DEBUG << "Inherit PFSTATE_TRANSPARENCY"<<std::endl;
    //     else OSG_DEBUG << "Define PFSTATE_TRANSPARENCY"<<std::endl;
    //     if (inherit & PFSTATE_ENTEXTURE) OSG_DEBUG << "Inherit PFSTATE_ENTEXTURE"<<std::endl;
    //     else OSG_DEBUG << "Define PFSTATE_ENTEXTURE"<<std::endl;
    //     if (inherit & PFSTATE_CULLFACE) OSG_DEBUG << "Inherit PFSTATE_CULLFACE"<<std::endl;
    //     else OSG_DEBUG << "Define PFSTATE_CULLFACE"<<std::endl;
    //     if (inherit & PFSTATE_ENLIGHTING) OSG_DEBUG << "Inherit PFSTATE_ENLIGHTING"<<std::endl;
    //     else OSG_DEBUG << "Define PFSTATE_ENLIGHTING"<<std::endl;
    //     if (inherit & PFSTATE_ENFOG) OSG_DEBUG << "Inherit PFSTATE_ENFOG"<<std::endl;
    //     else OSG_DEBUG << "Define PFSTATE_ENFOG"<<std::endl;
    //     if (inherit & PFSTATE_ENWIREFRAME) OSG_DEBUG << "Inherit PFSTATE_ENWIREFRAME"<<std::endl;
    //     else OSG_DEBUG << "Define PFSTATE_ENWIREFRAME"<<std::endl;
    //     if (inherit & PFSTATE_ENTEXMAT) OSG_DEBUG << "Inherit PFSTATE_ENTEXMAT"<<std::endl;
    //     else OSG_DEBUG << "Define PFSTATE_ENTEXMAT"<<std::endl;
    //     if (inherit & PFSTATE_ENTEXGEN) OSG_DEBUG << "Inherit PFSTATE_ENTEXGEN"<<std::endl;
    //     else OSG_DEBUG << "Define PFSTATE_ENTEXGEN"<<std::endl;
    //
    //     if (inherit & PFSTATE_ANTIALIAS) OSG_DEBUG << "Inherit PFSTATE_ANTIALIAS"<<std::endl;
    //     else OSG_DEBUG << "Define PFSTATE_ANTIALIAS"<<std::endl;
    //     if (inherit & PFSTATE_DECAL) OSG_DEBUG << "Inherit PFSTATE_DECAL"<<std::endl;
    //     else OSG_DEBUG << "Define PFSTATE_DECAL"<<std::endl;
    //     if (inherit & PFSTATE_ALPHAFUNC) OSG_DEBUG << "Inherit PFSTATE_ALPHAFUNC"<<std::endl;
    //     else OSG_DEBUG << "Define PFSTATE_ALPHAFUNC"<<std::endl;
    //     if (inherit & PFSTATE_ENCOLORTABLE) OSG_DEBUG << "Inherit PFSTATE_ENCOLORTABLE"<<std::endl;
    //     else OSG_DEBUG << "Define PFSTATE_ENCOLORTABLE"<<std::endl;
    //     if (inherit & PFSTATE_ENHIGHLIGHTING) OSG_DEBUG << "Inherit PFSTATE_ENHIGHLIGHTING"<<std::endl;
    //     else OSG_DEBUG << "Define PFSTATE_ENHIGHLIGHTING"<<std::endl;
    //     if (inherit & PFSTATE_ENLPOINTSTATE) OSG_DEBUG << "Inherit PFSTATE_ENLPOINTSTATE"<<std::endl;
    //     else OSG_DEBUG << "Define PFSTATE_ENLPOINTSTATE"<<std::endl;
    //     if (inherit & PFSTATE_ENTEXLOD) OSG_DEBUG << "Inherit PFSTATE_ENTEXLOD"<<std::endl;
    //     else OSG_DEBUG << "Define PFSTATE_ENTEXLOD"<<std::endl;

    if (!(inherit & PFSTATE_ALPHAFUNC)) {
        int mode = geostate->getMode(PFSTATE_ALPHAFUNC);
        float value = geostate->getVal(PFSTATE_ALPHAREF);
        osg::AlphaFunc* aFunc = new osg::AlphaFunc();
        switch (mode) {
            case PFAF_ALWAYS:
                aFunc->setFunction(osg::AlphaFunc::ALWAYS); //==PFAF_OFF in performer 311
                break;
            case PFAF_EQUAL:
                aFunc->setFunction(osg::AlphaFunc::EQUAL);
                break;
            case PFAF_GEQUAL:
                aFunc->setFunction(osg::AlphaFunc::GEQUAL);
                break;
            case PFAF_GREATER:
                aFunc->setFunction(osg::AlphaFunc::GREATER);
                break;
            case PFAF_LEQUAL:
                aFunc->setFunction(osg::AlphaFunc::LEQUAL);
                break;
            case PFAF_LESS:
                aFunc->setFunction(osg::AlphaFunc::LESS);
                break;
            case PFAF_NEVER:
                aFunc->setFunction(osg::AlphaFunc::NEVER);
                break;
            case PFAF_NOTEQUAL:
                aFunc->setFunction(osg::AlphaFunc::NOTEQUAL);
                break;
//            case PFAF_OFF: //==PFAF_ALWAYS
//                break;
            default: //warn PFAF not recognized?
                break;
        }
        aFunc->setReferenceValue(value);
        if (mode != PFAF_OFF) {
            osgStateSet->setAttributeAndModes(aFunc);
        } else {
            osgStateSet->setAttributeAndModes(aFunc,osg::StateAttribute::OFF);
        }
    }

    if (inherit & PFSTATE_TRANSPARENCY) osgStateSet->setMode(GL_BLEND,osg::StateAttribute::INHERIT);
    else
    {
        int mode = geostate->getMode(PFSTATE_TRANSPARENCY);
        if (mode & PFTR_NO_OCCLUDE) {
            mode &= ~PFTR_NO_OCCLUDE;//remove NO_OCCLUDE bit
            osg::Depth* depth = new osg::Depth(osg::Depth::LESS,0.0f,1.0f,false);
            osgStateSet->setAttributeAndModes(depth);
        }
        switch(mode)
        {
            case(PFTR_FAST):
            case(PFTR_HIGH_QUALITY):
            case(PFTR_BLEND_ALPHA):
            case(PFTR_MS_ALPHA):
            case(PFTR_MS_ALPHA_MASK):
//            case(PFTR_NO_OCCLUDE): //this is a bit flag.
            case(PFTR_ON):
                osgStateSet->setMode(GL_BLEND,osg::StateAttribute::ON);
                osgStateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
                break;
            case(PFTR_OFF): osgStateSet->setMode(GL_BLEND,osg::StateAttribute::OFF);break;
            default:        osgStateSet->setMode(GL_BLEND,osg::StateAttribute::INHERIT);break;
        }
    }

    if (inherit & PFSTATE_ENTEXTURE) osgStateSet->setTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::INHERIT);
    else
    {
        int mode = geostate->getMode(PFSTATE_ENTEXTURE);
        switch(mode)
        {
            case(PF_ON):    osgStateSet->setTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::ON);break;
            case(PF_OFF):
            default:        osgStateSet->setTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::OFF);break;
        }
    }

    if (inherit & PFSTATE_CULLFACE) osgStateSet->setMode(GL_CULL_FACE,osg::StateAttribute::INHERIT);
    else
    {
        int mode = geostate->getMode(PFSTATE_CULLFACE);
        switch(mode)
        {
            case(PFCF_BACK):
            {
                osgStateSet->setMode(GL_CULL_FACE,osg::StateAttribute::ON);
                osg::CullFace *cf = new osg::CullFace;
                cf->setMode(osg::CullFace::BACK);
                osgStateSet->setAttribute(cf);
            }
            break;

            case(PFCF_FRONT):
            {
                osgStateSet->setMode(GL_CULL_FACE,osg::StateAttribute::ON);
                osg::CullFace *cf = new osg::CullFace;
                cf->setMode(osg::CullFace::FRONT);
                osgStateSet->setAttribute(cf);
            }
            break;
            case(PFCF_BOTH):
            {
                osgStateSet->setMode(GL_CULL_FACE,osg::StateAttribute::ON);
                osg::CullFace *cf = new osg::CullFace;
                cf->setMode(osg::CullFace::FRONT_AND_BACK);
                osgStateSet->setAttribute(cf);
            }
            break;
            case(PFCF_OFF):
            default:        osgStateSet->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);break;
        }
    }

    if (inherit & PFSTATE_ENLIGHTING) osgStateSet->setMode(GL_LIGHTING,osg::StateAttribute::INHERIT);
    else
    {
        int mode = geostate->getMode(PFSTATE_ENLIGHTING);
        switch(mode)
        {
            case(PF_ON):    osgStateSet->setMode(GL_LIGHTING,osg::StateAttribute::ON);break;
            case(PF_OFF):
            default:        osgStateSet->setMode(GL_LIGHTING,osg::StateAttribute::OFF);break;
        }
    }

    if (inherit & PFSTATE_ENFOG) osgStateSet->setMode(GL_FOG,osg::StateAttribute::INHERIT);
    else
    {
        int mode = geostate->getMode(PFSTATE_ENFOG);
        switch(mode)
        {
            case(PF_ON):    osgStateSet->setMode(GL_FOG,osg::StateAttribute::ON);break;
            case(PF_OFF):
            default:        osgStateSet->setMode(GL_FOG,osg::StateAttribute::OFF);break;
        }
    }

    // not currently supported by OSG
    //     if (inherit & PFSTATE_ENWIREFRAME)  osgStateSet->setMode(osg::StateSet::WIREFRAME,osg::StateAttribute::INHERIT);
    //     else
    //     {
    //         int mode = geostate->getMode(PFSTATE_ENWIREFRAME);
    //         switch(mode)
    //         {
    //         case(PF_ON):    osgStateSet->setMode(osg::StateSet::WIREFRAME,osg::StateAttribute::ON);break;
    //         case(PF_OFF):
    //         default:        osgStateSet->setMode(osg::StateSet::WIREFRAME,osg::StateAttribute::OFF);break;
    //         }
    //     }

    // redundent in OSG's implementation of texmat mode
    //     if (inherit & PFSTATE_ENTEXMAT)  osgStateSet->setMode(osg::StateSet::TEXMAT,osg::StateAttribute::INHERIT);
    //     else
    //     {
    //         int mode = geostate->getMode(PFSTATE_ENTEXMAT);
    //         switch(mode)
    //         {
    //         case(PF_ON):    osgStateSet->setMode(osg::StateSet::TEXMAT,osg::StateAttribute::ON);break;
    //         case(PF_OFF):
    //         default:        osgStateSet->setMode(osg::StateSet::TEXMAT,osg::StateAttribute::OFF);break;
    //         }
    //     }


    // commenting out the following block since the TexGen should be set
    // appropriately by the osg::TexGen block below.
    //     if (inherit & PFSTATE_ENTEXGEN)  osgStateSet->setMode(osg::StateSet::TEXGEN,osg::StateAttribute::INHERIT);
    //     else
    //     {
    //         int mode = geostate->getMode(PFSTATE_ENTEXGEN);
    //         switch(mode)
    //         {
    //             case(PF_ON):    osgStateSet->setMode(osg::StateSet::TEXGEN,osg::StateAttribute::ON);break;
    //             case(PF_OFF):
    //             default:        osgStateSet->setMode(osg::StateSet::TEXGEN,osg::StateAttribute::OFF);break;
    //         }
    //     }
    //


    pfMaterial* front_mat = (pfMaterial*)geostate->getAttr(PFSTATE_FRONTMTL);
    pfMaterial* back_mat = (pfMaterial*)geostate->getAttr(PFSTATE_BACKMTL);
    visitMaterial(osgStateSet,front_mat,back_mat);

    pfTexture* tex = (pfTexture*)geostate->getAttr(PFSTATE_TEXTURE);
    visitTexture(osgStateSet,tex);

    pfTexEnv* texenv = (pfTexEnv*)geostate->getAttr(PFSTATE_TEXENV);

    if(texenv)
    {
      osg::TexEnv* osgTexEnv = new osg::TexEnv();
      int mode = texenv->getMode();

      float r,g,b,a;
      texenv->getBlendColor(&r, &g, &b, &a);

      switch(mode)
      {
          case(PFTE_MODULATE) :
              osgTexEnv->setMode(osg::TexEnv::MODULATE);
              osgTexEnv->setColor(osg::Vec4(r,g,b,a));
              break;
          case(PFTE_DECAL) :
              osgTexEnv->setMode(osg::TexEnv::DECAL);
              osgTexEnv->setColor(osg::Vec4(r,g,b,a));
              break;
          case(PFTE_BLEND) :
              osgTexEnv->setMode(osg::TexEnv::BLEND);
              osgTexEnv->setColor(osg::Vec4(r,g,b,a));
              break;
          case(PFTE_REPLACE) :
              osgTexEnv->setMode(osg::TexEnv::REPLACE);
              osgTexEnv->setColor(osg::Vec4(r,g,b,a));
              break;
          case(PFTE_ADD) :
              osgTexEnv->setMode(osg::TexEnv::ADD);
              osgTexEnv->setColor(osg::Vec4(r,g,b,a));
              break;
          default:
              OSG_WARN << "TexEnv Mode "<<mode<<" not currently supported by the OSG."<<std::endl;
              break;
      }
      osgStateSet->setTextureAttribute(0,osgTexEnv);
    }

    pfTexGen* texgen = (pfTexGen*)geostate->getAttr(PFSTATE_TEXGEN);

    if (texgen)
    {
        osg::TexGen* osgTexGen = new osg::TexGen();
        int mode = texgen->getMode(PF_S);

        // should this follow setPlane block be within the following switch?
        float x, y, z, d;
        texgen->getPlane(PF_S, &x, &y, &z, &d);
        osgTexGen->setPlane(osg::TexGen::S, osg::Vec4(x,y,z,d));
        texgen->getPlane(PF_T, &x, &y, &z, &d);
        osgTexGen->setPlane(osg::TexGen::T, osg::Vec4(x,y,z,d));

        switch(mode)
        {
            case(PFTG_OBJECT_LINEAR) :
                osgTexGen->setMode(osg::TexGen::OBJECT_LINEAR);
                osgStateSet->setTextureAttribute(0,osgTexGen);
                osgStateSet->setTextureMode(0,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
                osgStateSet->setTextureMode(0,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
                break;
            case(PFTG_EYE_LINEAR_IDENT) :
                OSG_WARN << "TexGen Mode PFTG_EYE_LINEAR_IDENT not currently supported by the OSG,"<<std::endl;
                OSG_WARN << "       assuming osg::TexGen::EYE_LINEAR."<<std::endl;
            case(PFTG_EYE_LINEAR) :
                osgTexGen->setMode(osg::TexGen::EYE_LINEAR);
                osgStateSet->setTextureAttribute(0,osgTexGen);
                osgStateSet->setTextureMode(0,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
                osgStateSet->setTextureMode(0,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
                break;
            case(PFTG_SPHERE_MAP) :
                osgTexGen->setMode(osg::TexGen::SPHERE_MAP);
                osgStateSet->setTextureAttribute(0,osgTexGen);
                osgStateSet->setTextureMode(0,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
                osgStateSet->setTextureMode(0,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
                break;
            case(PFTG_OFF) :
                osgStateSet->setAssociatedTextureModes(0,osgTexGen,osg::StateAttribute::OFF);
                break;
            case(PFTG_OBJECT_DISTANCE_TO_LINE) :
                OSG_WARN << "TexGen Mode PFTG_OBJECT_DISTANCE_TO_LINE not currently supported by the OSG."<<std::endl;
                osgStateSet->setAssociatedTextureModes(0,osgTexGen,osg::StateAttribute::OFF);
                break;
            case(PFTG_EYE_DISTANCE_TO_LINE) :
                OSG_WARN << "TexGen Mode PFTG_EYE_DISTANCE_TO_LINE not currently supported by the OSG."<<std::endl;
                osgStateSet->setAssociatedTextureModes(0,osgTexGen,osg::StateAttribute::OFF);
                break;
            default:
                OSG_WARN << "TexGen Mode "<<mode<<" not currently supported by the OSG."<<std::endl;
                osgStateSet->setAssociatedTextureModes(0,osgTexGen,osg::StateAttribute::OFF);
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
        osgTexMat->setMatrix(osgMatrix);
        osgStateSet->setTextureAttribute(0,osgTexMat);
    }

    return osgStateSet;
}


osg::Material* ConvertFromPerformer::visitMaterial(osg::StateSet* osgStateSet,pfMaterial* front_mat,pfMaterial* back_mat)
{
    if (front_mat==NULL && back_mat==NULL) return NULL;

    osg::Material* osgMaterial = new osg::Material;
    if (osgStateSet) osgStateSet->setAttribute(osgMaterial);

    pfMaterial* material = NULL;
    if (front_mat==back_mat) material = front_mat;
    else if (back_mat==NULL) material = front_mat;
    else if (front_mat==NULL) material = back_mat;

    if (material)                // single materials for front and back.
    {

        int colorMode = material->getColorMode(material->getSide());

        switch(colorMode)
        {
            case(PFMTL_CMODE_AMBIENT_AND_DIFFUSE): osgMaterial->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE); break;
            case(PFMTL_CMODE_AMBIENT): osgMaterial->setColorMode(osg::Material::AMBIENT); break;
            case(PFMTL_CMODE_DIFFUSE): osgMaterial->setColorMode(osg::Material::DIFFUSE); break;
            case(PFMTL_CMODE_EMISSION): osgMaterial->setColorMode(osg::Material::EMISSION); break;
            case(PFMTL_CMODE_SPECULAR): osgMaterial->setColorMode(osg::Material::SPECULAR); break;
            case(PFMTL_CMODE_OFF): osgMaterial->setColorMode(osg::Material::OFF); break;
        }

        osgMaterial->setShininess(osg::Material::FRONT_AND_BACK,material->getShininess());

        float a = material->getAlpha();
        float r,g,b;

        material->getColor(PFMTL_AMBIENT,&r,&g,&b);
        osgMaterial->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4(r,g,b,a));

        material->getColor(PFMTL_DIFFUSE,&r,&g,&b);
        osgMaterial->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(r,g,b,a));

        material->getColor(PFMTL_EMISSION,&r,&g,&b);
        osgMaterial->setEmission(osg::Material::FRONT_AND_BACK,osg::Vec4(r,g,b,a));

        material->getColor(PFMTL_SPECULAR,&r,&g,&b);
        osgMaterial->setSpecular(osg::Material::FRONT_AND_BACK,osg::Vec4(r,g,b,a));
    }
    else                         // separate materials for front and back.
    {

        int colorMode = front_mat->getColorMode(front_mat->getSide());

        switch(colorMode)
        {
            case(PFMTL_CMODE_AMBIENT_AND_DIFFUSE): osgMaterial->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE); break;
            case(PFMTL_CMODE_AMBIENT): osgMaterial->setColorMode(osg::Material::AMBIENT); break;
            case(PFMTL_CMODE_DIFFUSE): osgMaterial->setColorMode(osg::Material::DIFFUSE); break;
            case(PFMTL_CMODE_EMISSION): osgMaterial->setColorMode(osg::Material::EMISSION); break;
            case(PFMTL_CMODE_SPECULAR): osgMaterial->setColorMode(osg::Material::SPECULAR); break;
            case(PFMTL_CMODE_OFF): osgMaterial->setColorMode(osg::Material::OFF); break;
        }

        float a;
        float r,g,b;

        // front material
        osgMaterial->setShininess(osg::Material::FRONT,front_mat->getShininess());

        a = front_mat->getAlpha();

        front_mat->getColor(PFMTL_AMBIENT,&r,&g,&b);
        osgMaterial->setAmbient(osg::Material::FRONT,osg::Vec4(r,g,b,a));

        front_mat->getColor(PFMTL_DIFFUSE,&r,&g,&b);
        osgMaterial->setDiffuse(osg::Material::FRONT,osg::Vec4(r,g,b,a));

        front_mat->getColor(PFMTL_EMISSION,&r,&g,&b);
        osgMaterial->setEmission(osg::Material::FRONT,osg::Vec4(r,g,b,a));

        front_mat->getColor(PFMTL_SPECULAR,&r,&g,&b);
        osgMaterial->setSpecular(osg::Material::FRONT,osg::Vec4(r,g,b,a));

        // back material
        osgMaterial->setShininess(osg::Material::BACK,back_mat->getShininess());

        a = back_mat->getAlpha();

        back_mat->getColor(PFMTL_AMBIENT,&r,&g,&b);
        osgMaterial->setAmbient(osg::Material::BACK,osg::Vec4(r,g,b,a));

        back_mat->getColor(PFMTL_DIFFUSE,&r,&g,&b);
        osgMaterial->setDiffuse(osg::Material::BACK,osg::Vec4(r,g,b,a));

        back_mat->getColor(PFMTL_EMISSION,&r,&g,&b);
        osgMaterial->setEmission(osg::Material::BACK,osg::Vec4(r,g,b,a));

        back_mat->getColor(PFMTL_SPECULAR,&r,&g,&b);
        osgMaterial->setSpecular(osg::Material::BACK,osg::Vec4(r,g,b,a));

    }

    return osgMaterial;
}


static osg::Texture2D::FilterMode getTexfilter(int filter, int pftype)
{
    if (filter == PFTEX_MINFILTER)
    {

        if (pftype & PFTEX_LINEAR)
            return osg::Texture2D::NEAREST_MIPMAP_LINEAR;
        else if (pftype & PFTEX_BILINEAR)
            return osg::Texture2D::LINEAR_MIPMAP_NEAREST;
        else if (pftype & PFTEX_TRILINEAR)
            return osg::Texture2D::LINEAR_MIPMAP_LINEAR;

        return osg::Texture2D::NEAREST_MIPMAP_LINEAR;

    }
    else
    {
        // MAGFILTER

        // not quite sure what is supposed to be interpret the Peformer
        // filter modes here so will simple go with OpenGL default.

        return osg::Texture2D::LINEAR;
    }
}


osg::Texture2D* ConvertFromPerformer::visitTexture(osg::StateSet* osgStateSet,pfTexture* tex)
{
    if (tex==NULL) return NULL;

    osg::Texture2D* osgTexture = dynamic_cast<osg::Texture2D*>(getOsgObject(tex));
    if (osgTexture) {
        if (osgStateSet) osgStateSet->setTextureAttribute(0,osgTexture);
        return osgTexture;
    }

    osgTexture = new osg::Texture2D;
    registerPfObjectForOsgObject(tex, osgTexture);

    if (osgStateSet) osgStateSet->setTextureAttribute(0,osgTexture);

    int repeat_r = tex->getRepeat(PFTEX_WRAP_R);
    int repeat_s = tex->getRepeat(PFTEX_WRAP_S);
    int repeat_t = tex->getRepeat(PFTEX_WRAP_T);

    if (repeat_r==PFTEX_CLAMP)
        osgTexture->setWrap(osg::Texture2D::WRAP_R,osg::Texture2D::CLAMP);
    else
        osgTexture->setWrap(osg::Texture2D::WRAP_R,osg::Texture2D::REPEAT);

    if (repeat_s==PFTEX_CLAMP)
        osgTexture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP);
    else
        osgTexture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::REPEAT);

    if (repeat_t==PFTEX_CLAMP)
        osgTexture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP);
    else
        osgTexture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::REPEAT);

    // filter
#if 1
    osgTexture->setFilter(osg::Texture2D::MIN_FILTER,
                          getTexfilter(PFTEX_MINFILTER,
                                       tex->getFilter(PFTEX_MINFILTER)));
    osgTexture->setFilter(osg::Texture2D::MAG_FILTER,
                          getTexfilter(PFTEX_MAGFILTER,
                                       tex->getFilter(PFTEX_MAGFILTER)));
#endif

    // image
    std::string texName = tex->getName();

    if (_saveImagesAsRGB) {
        std::string strippedName = osgDB::getStrippedName(texName);

        pfList* imgList = tex->getList();
        if (imgList) {
            // save image list
            char buf[8];
            for (int i = 0; i < imgList->getNum(); i++) {
                pfTexture* t = (pfTexture*) imgList->get(i);
                if (t) {
                    snprintf(buf, sizeof(buf)-1, "_%04d", i);
                    texName = _saveImageDirectory+strippedName+buf+".rgb";
                    t->saveFile(texName.c_str());
                }
            }
        }
        else {
            // save single image
            texName = _saveImageDirectory+strippedName+".rgb";
            tex->saveFile(texName.c_str());
        }
    }

    if (!_saveAbsoluteImagePath) texName = osgDB::getSimpleFileName(texName);

    int s=0;
    int t=0;
    int r=0;
    int comp=0;
    unsigned int* imageData = NULL;

    tex->getImage(&imageData,&comp,&s,&t,&r);

    int internalFormat = comp;

    unsigned int pixelFormat =
        comp == 1 ? GL_LUMINANCE :
        comp == 2 ? GL_LUMINANCE_ALPHA :
        comp == 3 ? GL_RGB :
        comp == 4 ? GL_RGBA : (GLenum)-1;

    unsigned int dataType = GL_UNSIGNED_BYTE;

    // copy image data
    int size = s * t * r * comp;
    unsigned char* data = (unsigned char*) malloc(size);
    memcpy(data, imageData, size);

    osg::Image* image = new osg::Image;
    image->setFileName(texName.c_str());
    image->setImage(s,t,r,
                    internalFormat,
                    pixelFormat,
                    dataType,data,
                    osg::Image::USE_MALLOC_FREE);

    osgTexture->setImage(image);

    return osgTexture;
}
