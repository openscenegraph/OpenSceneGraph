// -*-c++-*-

/*
 * Wavefront OBJ loader for Open Scene Graph
 *
 * Copyright (C) 2001 Ulrich Hertlein <u.hertlein@web.de>
 *
 * Modified by Robert Osfield to support per Drawable coord, normal and
 * texture coord arrays, bug fixes, and support for texture mapping.
 *
 * The Open Scene Graph (OSG) is a cross platform C++/OpenGL library for 
 * real-time rendering of large 3D photo-realistic models. 
 * The OSG homepage is http://www.openscenegraph.org/
 */

#if defined(_MSC_VER)
	#pragma warning( disable : 4786 )
#endif

#include <string>

#include <osg/Notify>
#include <osg/Node>
#include <osg/Transform>
#include <osg/Geode>

#include <osg/GeoSet>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Texture>
#include <osg/TexGen>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include "glm.h"


class ReaderWriterOBJ : public osgDB::ReaderWriter
{
public:
    ReaderWriterOBJ() { }

    virtual const char* className() { return "Wavefront OBJ Reader"; }
    virtual bool acceptsExtension(const std::string& extension) {
        return (extension == "obj");
    }

    virtual osg::Node* readNode(const std::string& fileName);

protected:
    osg::Drawable* makeDrawable(GLMmodel* obj, GLMgroup* grp, osg::StateSet**);
};


// register with Registry to instantiate the above reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterOBJ> g_objReaderWriterProxy;


// read file and convert to OSG.
osg::Node* ReaderWriterOBJ::readNode(const std::string& fileName)
{
    GLMmodel* obj = glmReadOBJ((char*) fileName.c_str());
    if (!obj)
        return NULL;

    std::string directory = osgDB::getFilePath(fileName);


    osg::notify(osg::INFO) << "vertices " << obj->numvertices << endl;
    osg::notify(osg::INFO)  << "normals " << obj->numnormals << endl;
    osg::notify(osg::INFO)  << "texcoords " << obj->numtexcoords << endl;
    osg::notify(osg::INFO)  << "face normals " << obj->numfacetnorms << endl;
    osg::notify(osg::INFO)  << "tris " << obj->numtriangles << endl;
    osg::notify(osg::INFO)  << "materials " << obj->nummaterials << endl;
    osg::notify(osg::INFO)  << "groups " << obj->numgroups << endl;

    if (obj->numnormals==0)
    {
        osg::notify(osg::NOTICE)  << "No normals in .obj file, automatically calculating normals..."<<endl;
        glmFacetNormals(obj);
        glmVertexNormals(obj,90.0f);
    }


    unsigned int i;


    // materials
    osg::StateSet** osg_mtl = NULL;
    if (obj->nummaterials > 0) {
        osg_mtl = new osg::StateSet*[obj->nummaterials];
        for (i = 0; i < obj->nummaterials; i++) {
            GLMmaterial* omtl = &(obj->materials[i]);
            cerr << "mtl: " << omtl->name << endl;

            osg::StateSet* stateset = new osg::StateSet;
            osg_mtl[i] = stateset;

            osg::Material* mtl = new osg::Material;
            mtl->setAmbient(osg::Material::FRONT_AND_BACK,
                            osg::Vec4(omtl->ambient[0], omtl->ambient[1],
                                      omtl->ambient[2], omtl->ambient[3]));
            mtl->setDiffuse(osg::Material::FRONT_AND_BACK,
                            osg::Vec4(omtl->diffuse[0], omtl->diffuse[1],
                                      omtl->diffuse[2], omtl->diffuse[3]));
            mtl->setSpecular(osg::Material::FRONT_AND_BACK,
                             osg::Vec4(omtl->specular[0], omtl->specular[1],
                                       omtl->specular[2], omtl->specular[3]));
            mtl->setEmission(osg::Material::FRONT_AND_BACK,
                             osg::Vec4(omtl->emmissive[0], omtl->emmissive[1],
                                       omtl->emmissive[2], omtl->emmissive[3]));
            // note, osg shininess scales between 0.0 and 1.0.
            mtl->setShininess(osg::Material::FRONT_AND_BACK, omtl->shininess/128.0f);
            
            stateset->setAttribute(mtl);
            
            if (omtl->textureName)
            {
                std::string fileName = osgDB::findFileInDirectory(omtl->textureName,directory,true);
                if (!fileName.empty())
                {

                    osg::Image* osg_image = osgDB::readImageFile(fileName.c_str());
                    if (osg_image)
                    {
                        osg::Texture* osg_texture = new osg::Texture;
                        osg_texture->setImage(osg_image);
                        stateset->setAttributeAndModes(osg_texture,osg::StateAttribute::ON);
                        
                        if (omtl->textureReflection)
                        {
                            osg::TexGen* osg_texgen = new osg::TexGen;
                            osg_texgen->setMode(osg::TexGen::SPHERE_MAP);
                            stateset->setAttributeAndModes(osg_texgen,osg::StateAttribute::ON);
                        }
                    }
                    else
                    {
                        osg::notify(osg::NOTICE) << "Warning: Cannot create texture "<<omtl->textureName<<endl;
                    }
                }
                else
                {
                    osg::notify(osg::WARN) << "texture '"<<omtl->textureName<<"' not found"<<endl;
                }
            }
        }
    }

    // toplevel group or transform
    osg::Group* osg_top = NULL;
    if (obj->position[0] != 0.0f || obj->position[2] != 0.0f || obj->position[2] != 0.0f) {
        osg::Transform* xform = new osg::Transform;
		// note obj_x -> osg_x,
		//      obj_y -> osg_z,
		//      obj_z -> osg_y,
        xform->preTranslate(obj->position[0], obj->position[2], obj->position[1]);
        osg_top = xform;
    }
    else
        osg_top = new osg::Group;

    osg_top->setName(obj->pathname);

    // subgroups
    // XXX one Geode per group is probably not necessary...
    GLMgroup* ogrp = obj->groups;
    while (ogrp) {
        if (ogrp->numtriangles > 0) {

            osg::Geode* osg_geo = new osg::Geode;
            osg_geo->setName(ogrp->name);
            osg_geo->addDrawable(makeDrawable(obj,ogrp, osg_mtl));
            osg_top->addChild(osg_geo);
        }
        ogrp = ogrp->next;
    }

    // free
    glmDelete(obj);
    if (osg_mtl)
        delete[] osg_mtl;

    return osg_top;
}


// make drawable from OBJ group
osg::Drawable* ReaderWriterOBJ::makeDrawable(GLMmodel* obj,
                                             GLMgroup* grp,
                                             osg::StateSet** mtl)
{

    GLMtriangle* tris = obj->triangles;

    unsigned int ntris = grp->numtriangles;
    unsigned int i = 0;

    // geoset
    osg::GeoSet* gset = new osg::GeoSet;

    // primitives are only triangles
    gset->setPrimType(osg::GeoSet::TRIANGLES);
    gset->setNumPrims(ntris);
    int* primLen = new int[ntris];
    gset->setPrimLengths(primLen);


    // the following code for mapping the coords, normals and texcoords
    // is complicated greatly by the need to create seperate out the
    // sets of coords etc for each drawable.

    typedef std::vector<int> IndexVec;
    
    // fill an vector full of 0's, one for each vertex.
    IndexVec vcount(obj->numvertices+1,0);
    IndexVec ncount(obj->numnormals+1,0);
    IndexVec tcount(obj->numtexcoords+1,0);

    bool needNormals = obj->normals && obj->normals>0;
    bool needTexcoords = obj->texcoords && obj->numtexcoords>0;

    // first count the number of vertices used in this group.
    for (i = 0; i < ntris; i++)
    {
        GLMtriangle* tri = &(tris[grp->triangles[i]]);
        
        // increment the count once for each traingle corner.
        ++vcount[tri->vindices[0]];
        ++vcount[tri->vindices[1]];
        ++vcount[tri->vindices[2]];
        
        if (needNormals)
        {
            ++ncount[tri->nindices[0]];
            ++ncount[tri->nindices[1]];
            ++ncount[tri->nindices[2]];
        }
        
        if (needTexcoords)
        {
            ++tcount[tri->tindices[0]];
            ++tcount[tri->tindices[1]];
            ++tcount[tri->tindices[2]];
        }
    }


    IndexVec::iterator itr;
    int numCoords = 0;
    for(itr=vcount.begin();
        itr!=vcount.end();
        ++itr)
    {
        if ((*itr)>0) ++numCoords;
    }

    if (numCoords==0) return NULL;

    int numNormals = 0;
    for(itr=ncount.begin();
        itr!=ncount.end();
        ++itr)
    {
        if ((*itr)>0) ++numNormals;
    }

    int numTexcoords = 0;
    for(itr=tcount.begin();
        itr!=tcount.end();
        ++itr)
    {
        if ((*itr)>0) ++numTexcoords;
    }


    // allocate drawables vertices.
    osg::Vec3* coords = new osg::Vec3[numCoords];

    // allocate drawables normals.
    osg::Vec3* normals = NULL;
    if (numNormals>0) normals = new osg::Vec3[numNormals];

    // allocate drawables textcoords.
    osg::Vec2* texcoords = NULL;
    if (numTexcoords>0) texcoords = new osg::Vec2[numTexcoords];


    // fill an vector full of 0's, one for each vertex.
    IndexVec vmapping(obj->numvertices+1,-1);
    IndexVec nmapping(obj->numnormals+1,-1);
    IndexVec tmapping(obj->numtexcoords+1,-1);

    int coordCount = 0;
    for (i = 0; i < vcount.size(); i++)
    {

        if (vcount[i]>0)
        {
            
            // note obj_x -> osg_x,
            //      obj_y -> -osg_z,
            //      obj_z -> osg_y,
            coords[coordCount].set(obj->vertices[i*3+0],
                                   -obj->vertices[i*3+2],
                                   obj->vertices[i*3+1]);

            vmapping[i]=coordCount;
            ++coordCount;
        }
    }
    

    int normCount = 0;
    for (i = 0; i < ncount.size(); i++)
    {

        if (ncount[i]>0)
        {
            // note obj_x -> osg_x,
            //      obj_y -> osg_z,
            //      obj_z -> osg_y,
            // to rotate the about x axis to have model z upwards.
            normals[normCount].set(obj->normals[i*3+0],
                                   -obj->normals[i*3+2],
                                   obj->normals[i*3+1]);
            nmapping[i]=normCount;
            ++normCount;
        }
    }
              
    int texCount = 0;
    for (i = 0; i < tcount.size(); i++)
    {
        if (tcount[i]>0)
        {
            texcoords[texCount].set(obj->texcoords[i*2+0], obj->texcoords[i*2+1]);
            tmapping[i]=texCount;
            ++texCount;
        }
    }

    // index arrays
    unsigned int* cindex = NULL;
    cindex = new unsigned int[ntris * 3];

    unsigned int* nindex = NULL;
    if (normals)
        nindex = new unsigned int[ntris * 3];

    unsigned int* tindex = NULL;
    if (texcoords)
        tindex = new unsigned int[ntris * 3];

    // setup arrays
    for (i = 0; i < ntris; i++) {
        primLen[i] = 3;
        GLMtriangle* tri = &(tris[grp->triangles[i]]);

        cindex[i*3+0] = vmapping[tri->vindices[0]];
        cindex[i*3+1] = vmapping[tri->vindices[1]];
        cindex[i*3+2] = vmapping[tri->vindices[2]];

        if (nindex) {
            nindex[i*3+0] = nmapping[tri->nindices[0]];
            nindex[i*3+1] = nmapping[tri->nindices[1]];
            nindex[i*3+2] = nmapping[tri->nindices[2]];
        }

        if (tindex) {
            tindex[i*3+0] = tmapping[tri->tindices[0]];
            tindex[i*3+1] = tmapping[tri->tindices[1]];
            tindex[i*3+2] = tmapping[tri->tindices[2]];
        }
    }

    if (coords && cindex)
        gset->setCoords(coords, cindex);

    if (normals && nindex) {
        gset->setNormals(normals, nindex);
        gset->setNormalBinding(osg::GeoSet::BIND_PERVERTEX);
    }

    if (texcoords && tindex) {
        gset->setTextureCoords(texcoords, tindex);
        gset->setTextureBinding(osg::GeoSet::BIND_PERVERTEX);
    }

    // state and material (if any)
    if (mtl) {
    
        gset->setStateSet(mtl[grp->material]);
    }
    else
        osg::notify(osg::INFO) << "Group " << grp->name << " has no material" << endl;

    return gset;
}

