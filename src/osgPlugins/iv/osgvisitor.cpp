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

#include <osg/Notify>

#include <iostream>
#include "osgvisitor.h"
#include "material.h"
#include "coordinate3.h"
#include "separator.h"
#include "matrixtransform.h"
#include "indexedfaceset.h"
#include "indexedtristripset.h"
#include "texturecoordinate.h"
#include "texture2.h"
#include "transform.h"

#include <osg/Geode>
#include <osg/GeoSet>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osg/BlendFunc>
#include <osg/CullFace>
#include <osg/FrontFace>
#include <osg/PolygonMode>

#include <osgDB/ReadFile>
#include <osgUtil/SmoothingVisitor> ///< Para calcular normals
#include <osgUtil/TriStripVisitor> ///< Para convertir a tristrip


#include "atrvec.h"
#include "atrfloat.h"
#include "atrstring.h"
#include "atrvec3list.h"

#include "ltstr.h"
#include "normals.h"

#define CREASE_ANGLE 2//3.14159265359 * 2.0 / 3.0

class ObjectCache {
    typedef std::map<osg::ref_ptr<Material>, osg::ref_ptr<osg::Material> > MaterialMap;
    typedef std::map<const char*, osg::ref_ptr<osg::Texture2D>, ltstr> TextureMap;
    typedef std::map<osg::ref_ptr<MyNode>, osg::ref_ptr<osg::Node> > NodeMap;

    static MaterialMap materials;
    static TextureMap textures;
    static NodeMap nodos;
public:
    static osg::Node* getMyNode(MyNode*) {
        return 0;
    }

    static osg::Material* getMaterial(Material* _material) {
	if (materials.find(_material) == materials.end()) {
	    AtrVec *ambient=dynamic_cast<AtrVec*>(_material->getAttribute("ambientColor"));
	    AtrVec *diffuse=dynamic_cast<AtrVec*>(_material->getAttribute("diffuseColor"));
	    AtrVec *specular=dynamic_cast<AtrVec*>(_material->getAttribute("specularColor"));
	    AtrFloat *shininess=dynamic_cast<AtrFloat*>(_material->getAttribute("shininess"));
	    AtrFloat *transparency=dynamic_cast<AtrFloat*>(_material->getAttribute("transparency"));
	    osg::Material *material=new osg::Material();
	    if (ambient) material->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4(ambient->getValCut(0),ambient->getValCut(1),ambient->getValCut(2),1.0f));
	    if (diffuse) material->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(diffuse->getValCut(0),diffuse->getValCut(1),diffuse->getValCut(2),1.0f));
	    if (specular) material->setSpecular(osg::Material::FRONT_AND_BACK,osg::Vec4(specular->getValCut(0),specular->getValCut(1),specular->getValCut(2),1.0f));
	    if (shininess) material->setShininess(osg::Material::FRONT_AND_BACK,shininess->getValue());
	    if (transparency) material->setTransparency(osg::Material::FRONT_AND_BACK,transparency->getValue());
            if (ambient || diffuse || specular || transparency)
		materials[_material]=material;
            else return 0;
	}
        return materials[_material].get();
    }

    static osg::Texture2D* getTextura(const char* _texture) {
	if (textures.find(_texture) == textures.end()) {
	    osg::Texture2D *texture=new osg::Texture2D();
	    texture->setImage(osgDB::readImageFile(_texture));
	    texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::REPEAT);
	    texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::REPEAT);
	    osg::notify(osg::INFO) << "Loading texture " << _texture << std::endl;
	    textures[_texture]=texture;
	} 
	return textures[_texture].get();
    }
};

ObjectCache::MaterialMap ObjectCache::materials;
ObjectCache::TextureMap ObjectCache::textures;
ObjectCache::NodeMap ObjectCache::nodos;

static void makeTransform(MatrixTransform *matriz_active, osg::MatrixTransform *nodo) {
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
    osg::notify(osg::INFO) << "Model of " << total_vert << " vertices" << std::endl;
}

void OSGVisitor::applyMyNode(MyNode *) {
}

void OSGVisitor::applyMaterial(Material *material) {
    material_active=material;
}

void OSGVisitor::applyCoordinate3(Coordinate3 *coord) {
    coord3_active=coord;
}

void OSGVisitor::applyMatrixTransform(MatrixTransform *tr)
{
    makeTransform(tr,parent);
}

void OSGVisitor::applySeparator(Separator *sep)
{
    osg::MatrixTransform *group=new osg::MatrixTransform();
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

osg::PrimitiveSet *generatePrimitive(PolygonList &polys, unsigned primsize, unsigned numvertices) {
    unsigned i,j;
    osg::PrimitiveSet *p=0;
    // Fisrt of all count the number of polygons
    unsigned count=0;
    for (i=0;i<polys.size();i++) {
	VertexIndexList vindex=*polys[i];
	if (vindex.size() == primsize) {
            count++;
	}
    }
    if (count==0) return 0; ///If no polys, no primitive :)

    // The type of primitive
    osg::PrimitiveSet::Mode mode;
    switch (primsize) {
    case 1: mode=osg::PrimitiveSet::POINTS;break;
    case 2: mode=osg::PrimitiveSet::LINES;break;
    case 3: mode=osg::PrimitiveSet::TRIANGLES;break;
    case 4: mode=osg::PrimitiveSet::QUADS;break;
    default: mode=osg::PrimitiveSet::QUADS;
    }
    // Now will generate the indices and the primitive
    if (numvertices < 65536) {
	GLushort *indices=new GLushort[count*primsize];
        unsigned int count2=0;
	for (i=0;i<polys.size();i++) {
	    VertexIndexList vindex=*polys[i];
	    if (vindex.size() == primsize) {
		for (j=0;j<vindex.size();j++) {
                    indices[count2*primsize+j]=vindex[j];
		}
		count2++;
	    }
	}
	p=new osg::DrawElementsUShort(mode,count*primsize,indices);
	delete indices;
    } else {
	GLuint *indices=new GLuint[count*primsize];
        unsigned int count2=0;
	for (i=0;i<polys.size();i++) {
	    VertexIndexList vindex=*polys[i];
	    if (vindex.size() == primsize) {
		for (j=0;j<vindex.size();j++) {
                    indices[count2*primsize+j]=vindex[j];
		}
		count2++;
	    }
	}
	p=new osg::DrawElementsUInt(mode,count*primsize,indices);
	delete indices;
    }
    return p;
}

void OSGVisitor::makeGeode(osg::Geode *geode, osg::Geometry *geometry, bool twoSided) {
    osg::StateSet *state=new osg::StateSet();
    osg::FrontFace *frontface=new osg::FrontFace();
    frontface->setMode(osg::FrontFace::CLOCKWISE);
    state->setAttributeAndModes(frontface,osg::StateAttribute::ON);
    osg::CullFace *cull=new osg::CullFace();
    cull->setMode(osg::CullFace::BACK);
    if (!twoSided) {
	state->setAttributeAndModes(cull,osg::StateAttribute::ON);
    } else {
	//osg::notify(osg::INFO) <<  "Deactivating culling for this object" << std::endl;
	state->setAttributeAndModes(cull,osg::StateAttribute::OFF);
	osg::BlendFunc    *transp=new osg::BlendFunc();
	transp->setFunction(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	state->setAttribute(transp);
	state->setMode(GL_BLEND,osg::StateAttribute::ON);
	state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    }
    geode->setStateSet(state);
    /** Applying the material */
    if (material_active!=0) {
    osg::Material *material = ObjectCache::getMaterial(material_active);
	state->setAttributeAndModes(material,osg::StateAttribute::ON);
    }
    /** Applying the texture */
    if (texture_active!=0) {
        AtrString *filename=(AtrString*)texture_active->getAttribute("filename");
	if (filename) {
	    osg::Texture2D *texture=ObjectCache::getTextura(filename->getValue());
	    state->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
	}
    }

    /* If needed, apply per-vertex colors */
    if (material_active) {
	AtrVec3List *diffuse=dynamic_cast<AtrVec3List*>(material_active->getAttribute("diffuseColor"));
	if (diffuse) { // Has per-vertex colors
	    osg::notify(osg::INFO) << "Per vertex colors" << std::endl;
	    VertexList *colors=diffuse->getList();
	    osg::Vec3Array *colors_osg=new osg::Vec3Array();
	    for (unsigned i=0;i<colors->size();i++) {
		colors_osg->push_back((*colors)[i]);
	    }
	    osg::notify(osg::INFO) << colors->size() << " colors" << std::endl;
	    geometry->setColorArray(colors_osg);
	    geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	}
    }
}

void OSGVisitor::applyIndexedFaceSet(IndexedFaceSet *ifs) {
    unsigned i;
    if (coord3_active == 0) {
	std::cerr << "ERROR: IndexedFaceSet without previous Coordinate3!" << std::endl;
        throw -1;
    }
    osg::Geode *geode=new osg::Geode();
    osg::Geometry *geometry=new osg::Geometry();
    makeGeode(geode,geometry,ifs->getTwoSided());
    /* Converting list of vertices to the OSG way (mostly the same) */
    VertexList vertices=coord3_active->getVertices();
    osg::Vec3Array *vertices_osg=new osg::Vec3Array();
    for (i=0;i<vertices.size();i++) {
        vertices_osg->push_back(vertices[i]);
    }
    geometry->setVertexArray(vertices_osg);
    total_vert+=vertices.size();

    /* Converting list of polys */
    PolygonList polys=ifs->getPolygons();
    for (i=1;i<=4;i++) {
	osg::PrimitiveSet *p=generatePrimitive(polys,i,vertices.size());
	if (p!=0) {
            geometry->addPrimitiveSet(p);
	}
    }
    TextureCoordList tcoord;
    if (tcoord_active) tcoord=tcoord_active->getTextureCoords();
    PolygonList textureIndices = ifs->getTextureIndices();
    if (tcoord_active) {
	if (ifs->hasTextureIndices()) {
	    std::cerr << "texture indices are not supported!" << std::endl;
	} else {
	    osg::Vec2Array *texCoords=new osg::Vec2Array();
	    for (i=0;i<vertices.size();i++) {
		texCoords->push_back(osg::Vec2(tcoord[i].first,tcoord[i].second));
	    }
            geometry->setTexCoordArray(0,texCoords);
	}
    }

    osgUtil::SmoothingVisitor v;
    v.smooth(*geometry);

    // As SmoothingVisitor doesn't take the front face into account:
    osg::Vec3Array *norm=geometry->getNormalArray();
    for (i=0;i<norm->size();i++) {
        (*norm)[i] = - (*norm)[i];
    }

    geode->addDrawable(geometry);
    parent->addChild(geode);
}

void OSGVisitor::applyIndexedTriStripSet(IndexedTriStripSet *its) {
    unsigned i,j;
    if (coord3_active == 0) {
	std::cerr << "ERROR: IndexedFaceSet without previous Coordinate3!" << std::endl;
        throw -1;
    }
    osg::Geode *geode=new osg::Geode();
    osg::Geometry *geometry=new osg::Geometry();
    makeGeode(geode,geometry,its->getTwoSided());
    /* Converting list of vertices to the OSG way (mostly the same) */
    VertexList vertices=coord3_active->getVertices();
    osg::Vec3Array *vertices_osg=new osg::Vec3Array();
    for (i=0;i<vertices.size();i++) {
        vertices_osg->push_back(vertices[i]);
    }
    geometry->setVertexArray(vertices_osg);
    total_vert+=vertices.size();

    /* Converting list of polys */
    PolygonList polys=its->getPolygons();
    for (i=0;i<polys.size();i++) {
	VertexIndexList vindex=*polys[i];
	unsigned short *indices=new unsigned short[vindex.size()];
	for (j=0;j<vindex.size();j++) {
            indices[j]=vindex[j];
	}
        geometry->addPrimitiveSet(new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_STRIP,vindex.size(),indices));
    }


    TextureCoordList tcoord;
    if (tcoord_active) tcoord=tcoord_active->getTextureCoords();
    PolygonList textureIndices = its->getTextureIndices();
    if (tcoord_active) {
	if (its->hasTextureIndices()) {
	    std::cerr << "texture indices are not supported!" << std::endl;
	} else {
	    osg::Vec2Array *texCoords=new osg::Vec2Array();
	    for (i=0;i<vertices.size();i++) {
		texCoords->push_back(osg::Vec2(tcoord[i].first,tcoord[i].second));
	    }
            geometry->setTexCoordArray(0,texCoords);
	}
    }

    osgUtil::SmoothingVisitor v;
    v.smooth(*geometry);

    // As SmoothingVisitor doesn't take the front face into account:
    osg::Vec3Array *norm=geometry->getNormalArray();
    for (i=0;i<norm->size();i++) {
        (*norm)[i] = - (*norm)[i];
    }

    geode->addDrawable(geometry);
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

