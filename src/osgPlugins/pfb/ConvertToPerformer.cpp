#include "ConvertToPerformer.h"

#include <osg/Group>
#include <osg/LOD>
#include <osg/Switch>
#include <osg/Geode>
#include <osg/Billboard>
#include <osg/Texture>
#include <osg/Image>
#include <osg/Notify>

#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>

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

extern "C"
{

    extern pfNode *
        pfdLoadFile_osg (char *fileName)
    {
        osg::Node* node = osgDB::readNodeFile(fileName);
        if (node==NULL) return 0;

        ConvertToPerformer converter;
        return converter.convert(node);
    }

};

ConvertToPerformer::ConvertToPerformer()
{
    setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);

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
}


ConvertToPerformer::~ConvertToPerformer()
{
}


pfNode* ConvertToPerformer::convert(const osg::Node* node)
{
    _pfRoot = NULL;
    if (node)
    {
        // a hack to get round current limitation of node visitor which
        // only handles non const operations.
        osg::Node* non_const_node = const_cast<osg::Node*>(node);
        non_const_node->accept(*this);
    }
    return _pfRoot;
}


pfObject* ConvertToPerformer::getPfObject(osg::Object* osgObj)
{
    OsgObjectToPfObjectMap::iterator fitr = _osgToPfMap.find(osgObj);
    if (fitr != _osgToPfMap.end())
    {
        osg::notify(osg::DEBUG_INFO) << "Found shared object"<<std::endl;
        return (*fitr).second;
    }
    else return NULL;
}


void ConvertToPerformer::registerOsgObjectForPfObject(osg::Object* osgObj,pfObject* pfObj)
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

    registerOsgObjectForPfObject(&node,pf_group);

    if (!node.getName().empty()) pf_group->setName(node.getName().c_str());

    _pfParent = pf_group;

    node.traverse(*this);

    _pfParent = parent;
}


void ConvertToPerformer::apply(osg::MatrixTransform& osgTransform)
{
    pfGroup* parent = _pfParent;

    pfDCS* pf_dcs = dynamic_cast<pfDCS*>(getPfObject(&osgTransform));
    if (pf_dcs)
    {
        if (_pfParent) _pfParent->addChild(pf_dcs);
        return;
    }

    pf_dcs = new pfDCS;
    if (!_pfRoot) _pfRoot = pf_dcs;
    if (_pfParent) _pfParent->addChild(pf_dcs);

    registerOsgObjectForPfObject(&osgTransform,pf_dcs);

    if (!osgTransform.getName().empty()) pf_dcs->setName(osgTransform.getName().c_str());

    const osg::Matrix& matrix = osgTransform.getMatrix();

    pfMatrix pf_matrix(matrix(0,0),matrix(0,1),matrix(0,2),matrix(0,3),
                       matrix(1,0),matrix(1,1),matrix(1,2),matrix(1,3),
                       matrix(2,0),matrix(2,1),matrix(2,2),matrix(2,3),
                       matrix(3,0),matrix(3,1),matrix(3,2),matrix(3,3));

    pf_dcs->setMat(pf_matrix);

    _pfParent = pf_dcs;

    osgTransform.traverse(*this);

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

    registerOsgObjectForPfObject(&node,pf_switch);

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

    registerOsgObjectForPfObject(&node,pf_lod);

    if (!node.getName().empty()) pf_lod->setName(node.getName().c_str());

    _pfParent = pf_lod;

    node.traverse(*this);

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

    registerOsgObjectForPfObject(&node,pf_billboard);

    if (!node.getName().empty()) pf_billboard->setName(node.getName().c_str());

    for(unsigned int i=0;i<node.getNumDrawables();++i)
    {
        osg::GeoSet* osg_gset = dynamic_cast<osg::GeoSet*>(node.getDrawable(i));
        if (osg_gset)
        {
            pfGeoSet* pf_geoset = visitGeoSet(osg_gset);
            if (pf_geoset) pf_billboard->addGSet(pf_geoset);
        }
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

    registerOsgObjectForPfObject(&node,pf_geode);

    if (!node.getName().empty()) pf_geode->setName(node.getName().c_str());

    for(unsigned int i=0;i<node.getNumDrawables();++i)
    {
        osg::GeoSet* osg_gset = dynamic_cast<osg::GeoSet*>(node.getDrawable(i));
        if (osg_gset)
        {
            pfGeoSet* pf_geoset = visitGeoSet(osg_gset);
            if (pf_geoset) pf_geode->addGSet(pf_geoset);
        }
    }

    _pfParent = parent;
}


pfGeoSet* ConvertToPerformer::visitGeoSet(osg::GeoSet* geoset)
{
    if (geoset==NULL) return NULL;

    osg::GeoSet::IndexPointer& cindex = geoset->getCoordIndices();
    if (cindex.valid() && !cindex._is_ushort) 
    {
        osg::notify(osg::WARN)<<"Warning: Cannot convert osg::GeoSet to pfGeoSet due to uint coord index."<<std::endl;
        return NULL;
    }

    osg::GeoSet::IndexPointer& nindex = geoset->getNormalIndices();
    if (nindex.valid() && !nindex._is_ushort) 
    {
        osg::notify(osg::WARN)<<"Warning: Cannot convert osg::GeoSet to pfGeoSet due to uint normal index."<<std::endl;
        return NULL;
    }

    osg::GeoSet::IndexPointer& colindex = geoset->getColorIndices();
    if (colindex.valid() && !colindex._is_ushort) 
    {
        osg::notify(osg::WARN)<<"Warning: Cannot convert osg::GeoSet to pfGeoSet due to uint color index."<<std::endl;
        return NULL;
    }

    void* arena = pfGetSharedArena();

    pfGeoSet* pf_geoset = new pfGeoSet();

    pf_geoset->setGState(visitStateSet(geoset->getStateSet()));

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
    GLushort *ilist = cindex._ptr._ushort;

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
            int ni=geoset->getNumCoordIndices();
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
    ilist = nindex._ptr._ushort;

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
            int ni=geoset->getNumNormalIndices();
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
    ilist = colindex._ptr._ushort;

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
            int ni=geoset->getNumColorIndices();
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

    // Copy texture coords

    osg::Vec2 *tcoords = geoset->getTextureCoords();
    ilist = geoset->getTextureIndices()._ptr._ushort;
    if( tcoords )
    {
	int bind = _gsetBindMap[geoset->getTextureBinding()];
	int ct = geoset->getNumTextureCoords();
	pfVec2 *pf_tcoords = (pfVec2 *)pfMalloc( sizeof( pfVec2 ) * ct, arena );
	for( i = 0; i < ct; i++ )
	{
	    pf_tcoords[i].set( tcoords[i][0], tcoords[i][1] );
	}
	if( ilist )
	{
	    int nt = geoset->getNumTextureIndices();
	    ushort *pf_tindex = (ushort *)pfMalloc( sizeof( ushort ) * nt, arena );
	    for( i = 0; i < nt; i++ )
		pf_tindex[i] = ilist[i];
	
	    pf_geoset->setAttr( PFGS_TEXCOORD2, bind, pf_tcoords, pf_tindex );
	}
	else
	    pf_geoset->setAttr( PFGS_TEXCOORD2, bind, pf_tcoords, NULL );
    }


    return pf_geoset;
}


pfGeoState* ConvertToPerformer::visitStateSet(osg::StateSet* stateset)
{
    if (stateset==NULL) return NULL;

    pfGeoState* pf_geostate = new pfGeoState();

    switch(stateset->getMode(GL_LIGHTING))
    {
        case(osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON):
        case(osg::StateAttribute::ON): pf_geostate->setMode(PFSTATE_ENLIGHTING,PF_ON);break;
        case(osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF):
        case(osg::StateAttribute::OFF): pf_geostate->setMode(PFSTATE_ENLIGHTING,PF_OFF);break;
        // pfGeostate value as default inherit.
        case(osg::StateAttribute::INHERIT): break;
    }

    switch(stateset->getMode(GL_TEXTURE_2D))
    {
        case(osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON):
        case(osg::StateAttribute::ON): pf_geostate->setMode(PFSTATE_ENTEXTURE,PF_ON);break;
        case(osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF):
        case(osg::StateAttribute::OFF): pf_geostate->setMode(PFSTATE_ENTEXTURE,PF_OFF);break;
        // pfGeostate value as default inherit.
        case(osg::StateAttribute::INHERIT): break;
    }

    const osg::Texture *tex = dynamic_cast<const osg::Texture *>(stateset->getAttribute( osg::StateAttribute::TEXTURE));

    if( tex != NULL )
    {
	pfTexture *pf_tex = new pfTexture;
	const osg::Image *img = tex->getImage();
	int ns = img->s();
	int nt = img->t();
	int ncomp = 
		img->getPixelFormat() == GL_LUMINANCE  ? 1 :
		img->getPixelFormat() == GL_LUMINANCE_ALPHA  ? 2 :
		img->getPixelFormat() == GL_RGB  ? 3 :
		img->getPixelFormat() == GL_RGBA ? 4 : 3;

  	uint *uim = (uint *)pfMalloc( ns * nt * ncomp, pfGetSharedArena() );

	memcpy( uim, img->data(), ns * nt * ncomp );

    	pf_tex->setImage( uim, ncomp, ns, nt, 0 );

	pf_geostate->setAttr( PFSTATE_TEXTURE, pf_tex );
	pf_geostate->setAttr( PFSTATE_TEXENV, new pfTexEnv );
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
