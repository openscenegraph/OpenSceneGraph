/*
 * osgDB::wrl - a VRML 1.0 loader for OpenSceneGraph
 * Copyright (C) 2002 Ruben Lopez <ryu@gpul.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef WIN32
#  pragma warning (disable:4786)
#  pragma warning (disable:4541)
#endif
#include <iostream>
#include "osgvisitor.h"
#include "material.h"
#include "coordinate3.h"
#include "separator.h"
#include "matrixtransform.h"
#include "indexedfaceset.h"
#include "texturecoordinate.h"
#include "texture2.h"
#include "transform.h"

#include <osg/Geode>
#include <osg/GeoSet>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Texture>
#include <osg/Transform>
#include <osg/Transparency>
#include <osg/CullFace>
#include <osg/FrontFace>
#include <osg/PolygonMode>

#include <osgDB/ReadFile>
#include <osgUtil/SmoothingVisitor> ///< Para calcular normals
#include <osgUtil/TriStripVisitor> ///< Para convertir a tristrip


#include "atrvec.h"
#include "atrfloat.h"
#include "atrstring.h"

#include "ltstr.h"
#include "normals.h"

#define CREASE_ANGLE 2//3.14159265359 * 2.0 / 3.0

class CacheObjetos {
    typedef std::map<osg::ref_ptr<Material>, osg::ref_ptr<osg::Material> > MaterialMap;
    typedef std::map<const char*, osg::ref_ptr<osg::Texture>, ltstr> TextureMap;
    typedef std::map<osg::ref_ptr<MyNode>, osg::ref_ptr<osg::Node> > NodeMap;

    static MaterialMap materiales;
    static TextureMap textures;
    static NodeMap nodos;
public:
    static osg::Node* getMyNode(MyNode*) {
        return 0;
    }

    static osg::Material* getMaterial(Material* _material) {
	if (materiales.find(_material) == materiales.end()) {
	    AtrVec *ambient=(AtrVec*)_material->getAttribute("ambientColor");
	    AtrVec *diffuse=(AtrVec*)_material->getAttribute("diffuseColor");
	    AtrVec *specular=(AtrVec*)_material->getAttribute("specularColor");
	    AtrFloat *shininess=(AtrFloat*)_material->getAttribute("shininess");
	    AtrFloat *transparency=(AtrFloat*)_material->getAttribute("transparency");
	    osg::Material *material=new osg::Material();
	    if (ambient) material->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4(ambient->getValCut(0),ambient->getValCut(1),ambient->getValCut(2),1.0f));
	    if (diffuse) material->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(diffuse->getValCut(0),diffuse->getValCut(1),diffuse->getValCut(2),1.0f));
	    if (specular) material->setSpecular(osg::Material::FRONT_AND_BACK,osg::Vec4(specular->getValCut(0),specular->getValCut(1),specular->getValCut(2),1.0f));
	    if (shininess) material->setShininess(osg::Material::FRONT_AND_BACK,shininess->getValue());
	    if (transparency) material->setTransparency(osg::Material::FRONT_AND_BACK,transparency->getValue());
            materiales[_material]=material;
	}
        return materiales[_material].get();
    }

    static osg::Texture* getTextura(const char* _texture) {
	if (textures.find(_texture) == textures.end()) {
	    osg::Texture *texture=new osg::Texture();
	    texture->setImage(osgDB::readImageFile(_texture));
	    texture->setWrap(osg::Texture::WRAP_S,osg::Texture::REPEAT);
	    texture->setWrap(osg::Texture::WRAP_T,osg::Texture::REPEAT);
	    std::cout << "Loading texture " << _texture << std::endl;
	    textures[_texture]=texture;
	} 
	return textures[_texture].get();
    }
};

CacheObjetos::MaterialMap CacheObjetos::materiales;
CacheObjetos::TextureMap CacheObjetos::textures;
CacheObjetos::NodeMap CacheObjetos::nodos;

static void makeTransform(MatrixTransform *matriz_active, osg::Transform *nodo) {
     // Original
     osg::Matrix m(matriz_active->getElem(0),matriz_active->getElem(1),matriz_active->getElem(2),matriz_active->getElem(3),
		  matriz_active->getElem(4),matriz_active->getElem(5),matriz_active->getElem(6),matriz_active->getElem(7),
		  matriz_active->getElem(8),matriz_active->getElem(9),matriz_active->getElem(10),matriz_active->getElem(11),
		  matriz_active->getElem(12),matriz_active->getElem(13),matriz_active->getElem(14),matriz_active->getElem(15));
    nodo->setMatrix(m);
}

OSGVisitor::OSGVisitor(MyNode *nodo) {
    root=0;
    parent=0;
    coord3_active=0;
    tcoord_active=0;
    texture_active=0;
    material_active=0;
    total_vert=0;
    two_sided=false;
    nodo->accept(this);
    std::cout << "Model of " << total_vert << " vertices" << std::endl;
}

void OSGVisitor::applyMyNode(MyNode *) {
}

void OSGVisitor::applyMaterial(Material *material) {
    material_active=material;
}

void OSGVisitor::applyCoordinate3(Coordinate3 *coord) {
    coord3_active=coord;
}

void OSGVisitor::applyMatrixTransform(MatrixTransform *tr) {
    makeTransform(tr,parent);
}

void OSGVisitor::applySeparator(Separator *sep) {
    osg::Transform *group=new osg::Transform();
    if (root==0) {
	root=group;
    }
    if (parent!=0) {
	parent->addChild(group);
    }
    two_sided=false;
    MyNode::MyNodeList hijos=sep->getChildren();
    for (MyNode::MyNodeList::iterator iter=hijos.begin();iter!=hijos.end();iter++) {
	osg::ref_ptr<MyNode> hijo=*iter;
	parent=group;
        hijo->accept(this);
    }
}

void OSGVisitor::applyIndexedFaceSet(IndexedFaceSet *ifs) {
    unsigned i,j;
    if (coord3_active == 0) {
	std::cerr << "ERROR: IndexedFaceSet without previous Coordinate3!" << std::endl;
        throw -1;
    }
    osg::Geode *geode=new osg::Geode();
    osg::GeoSet *objeto=new osg::GeoSet();
    osg::StateSet *state=new osg::StateSet();
    osg::FrontFace *frontface=new osg::FrontFace();
    frontface->setMode(osg::FrontFace::CLOCKWISE);
    state->setAttributeAndModes(frontface,osg::StateAttribute::ON);
    osg::CullFace *cull=new osg::CullFace();
    cull->setMode(osg::CullFace::BACK);
    if (ifs->getTwoSided() == false) {
	state->setAttributeAndModes(cull,osg::StateAttribute::ON);
    } else {
	std::cout <<  "Deactivating culling for this object" << std::endl;
	state->setAttributeAndModes(cull,osg::StateAttribute::OFF);
	osg::Transparency    *transp=new osg::Transparency();
	transp->setFunction(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	state->setAttribute(transp);
	state->setMode(GL_BLEND,osg::StateAttribute::ON);
	state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    }
    geode->setStateSet(state);
    /** Applying the material */
    if (material_active!=0) {
    osg::Material *material = CacheObjetos::getMaterial(material_active);
	state->setAttributeAndModes(material,osg::StateAttribute::ON);
    }
    /** Applying the texture */
    if (texture_active!=0) {
        AtrString *filename=(AtrString*)texture_active->getAttribute("filename");
	if (filename) {
	    osg::Texture *texture=CacheObjetos::getTextura(filename->getValue());
	    state->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
	}

    }
    VertexList vertices=coord3_active->getVertices();
    PolygonList polys=ifs->getPolygons();
    TextureCoordList tcoord;
    if (tcoord_active) tcoord=tcoord_active->getTextureCoords();
    unsigned nPoly=polys.size();
    unsigned nvert_total=0;

    objeto->setPrimType(osg::GeoSet::POLYGON);
    objeto->setNumPrims(nPoly);

    /** Calculating length of primitives */
    int *long_primitivas=new int[nPoly];
    for (j=0;j<nPoly;j++) {
	long_primitivas[j]=polys[j]->size();
        nvert_total+=long_primitivas[j];
    }
    objeto->setPrimLengths(long_primitivas);
    this->total_vert+=nvert_total;

    /** We change from the ad-hoc scenegraph to the OSG one */
    unsigned posCoords=0;
    osg::Vec3 *coords = new osg::Vec3[nvert_total];
    osg::Vec2 *tcoords = new osg::Vec2[nvert_total];
    osg::Vec3 *normalsFlat = new osg::Vec3[nvert_total];
    bool hasTextureIndices=ifs->hasTextureIndices();
    PolygonList textureIndices = ifs->getTextureIndices();
    for (j=0; j < nPoly; j++) {
	VertexIndexList vindex=*polys[j];
	VertexIndexList texindex;
	if (hasTextureIndices) texindex=*textureIndices[j];
	unsigned nVert=vindex.size();
	osg::Vec3 normal;
	if (nVert > 2) {
	    normal = calcNormal(vertices[vindex[0]],vertices[vindex[1]],vertices[vindex[2]]);
	} else {
	    normal = osg::Vec3(0,0,0);
	}
	for (i=0; i < nVert; i++) {
	    int vert = vindex[i];
	    coords[posCoords].set(vertices[vert][0],vertices[vert][1],vertices[vert][2]);
            normalsFlat[posCoords].set(normal[0],normal[1],normal[2]);
	    if (tcoord_active) {
		if (hasTextureIndices) {
                    int coord=texindex[i];
		    tcoords[posCoords].set(tcoord[coord].first,tcoord[coord].second);
		} else {
		    tcoords[posCoords].set(tcoord[vert].first,tcoord[vert].second);
		}
	    }
	    posCoords++;
	}
    }

    /** Establishing parameters of the geoset */
    objeto->setCoords(coords);
    if (tcoord_active) {
	objeto->setTextureCoords(tcoords);
	objeto->setTextureBinding(osg::GeoSet::BIND_PERVERTEX);
    }
    //osg::Vec3 *normals=calcNormals(vertices,polys,nvert_total);
    //objeto->setNormals(normals);
    
    geode->addDrawable(objeto);
    parent->addChild(geode);
}

void OSGVisitor::applyTextureCoordinate(TextureCoordinate *texc) {
    tcoord_active=texc;
}

void OSGVisitor::applyTexture2(Texture2 *tex) {
    texture_active=tex;
}

void OSGVisitor::applyTransform(Transform *trans) {
    osg::Matrix scale;
    osg::Matrix rotate;
    osg::Matrix translate;
    AtrVec *_scale=(AtrVec*)trans->getAttribute("scaleFactor");
    AtrVec *_rotate=(AtrVec*)trans->getAttribute("rotation");
    AtrVec *_translate=(AtrVec*)trans->getAttribute("translation");
    if (_scale) {
	scale.makeScale(_scale->getVal(0),_scale->getVal(1),_scale->getVal(2));
    }
    if (_rotate) {
	rotate.makeRotate(_rotate->getVal(3),_rotate->getVal(0),_rotate->getVal(1),_rotate->getVal(2));
    }
    if (_translate) {
	translate.makeTranslate(_translate->getVal(0),_translate->getVal(1),_translate->getVal(2));
    }
    parent->setMatrix(scale*rotate*translate);
}


osg::Node* OSGVisitor::getRoot() {
    return root;
}

