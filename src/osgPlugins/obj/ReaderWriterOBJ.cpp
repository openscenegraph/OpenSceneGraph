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
#include <osg/MatrixTransform>
#include <osg/Geode>

#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/TexGen>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include "glm.h"

#include <map>
#include <set>

template <class T>
struct DerefLess
{
    bool operator ()(T lhs,T rhs) const { return *lhs < *rhs; }
};


class ReaderWriterOBJ : public osgDB::ReaderWriter
{
public:
    ReaderWriterOBJ() { }

    virtual const char* className() { return "Wavefront OBJ Reader"; }
    virtual bool acceptsExtension(const std::string& extension) {
        return osgDB::equalCaseInsensitive(extension,"obj");
    }

    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*);

protected:
    osg::Drawable* makeDrawable(GLMmodel* obj, GLMgroup* grp);
    
    typedef std::map< std::string, osg::ref_ptr<osg::Texture2D> > TextureMap;
    typedef std::set< osg::ref_ptr<osg::Material>, DerefLess< osg::ref_ptr<osg::Material> > > MaterialSet;
    typedef std::set< osg::ref_ptr<osg::StateSet>, DerefLess< osg::ref_ptr<osg::StateSet> > > StateSetSet;
    typedef std::vector< osg::ref_ptr<osg::StateSet> > ObjMatierialOsgStateSetArray;
    
};


// register with Registry to instantiate the above reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterOBJ> g_objReaderWriterProxy;


// read file and convert to OSG.
osgDB::ReaderWriter::ReadResult ReaderWriterOBJ::readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*)
{
    std::string ext = osgDB::getFileExtension(fileName);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;


    GLMmodel* obj = glmReadOBJ((char*) fileName.c_str());
    if (!obj)
        return ReadResult::FILE_NOT_HANDLED;

    std::string directory = osgDB::getFilePath(fileName);


    osg::notify(osg::INFO) << "vertices " << obj->numvertices << std::endl;
    osg::notify(osg::INFO)  << "normals " << obj->numnormals << std::endl;
    osg::notify(osg::INFO)  << "texcoords " << obj->numtexcoords << std::endl;
    osg::notify(osg::INFO)  << "face normals " << obj->numfacetnorms << std::endl;
    osg::notify(osg::INFO)  << "tris " << obj->numtriangles << std::endl;
    osg::notify(osg::INFO)  << "materials " << obj->nummaterials << std::endl;
    osg::notify(osg::INFO)  << "groups " << obj->numgroups << std::endl;

    if (obj->numnormals==0)
    {
        osg::notify(osg::NOTICE)  << "No normals in .obj file, automatically calculating normals..."<< std::endl;
        glmFacetNormals(obj);
        glmVertexNormals(obj,90.0f);
    }


    unsigned int i;


    TextureMap textureMap;
    MaterialSet materialSet;
    StateSetSet statesetSet;
    
    // create a sphere mapped texgen just in case we need it.
    osg::ref_ptr<osg::TexGen> osg_texgen = new osg::TexGen;
    osg_texgen->setMode(osg::TexGen::SPHERE_MAP);

    ObjMatierialOsgStateSetArray osg_mtl(obj->nummaterials);

    for (i = 0; i < obj->nummaterials; i++)
    {
        GLMmaterial* omtl = &(obj->materials[i]);
        osg::notify(osg::DEBUG_INFO) << "mtl: " << omtl->name << std::endl;

        osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;

        osg::ref_ptr<osg::Material> mtl = new osg::Material;
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
        mtl->setShininess(osg::Material::FRONT_AND_BACK, omtl->shininess);
        mtl->setAlpha(osg::Material::FRONT_AND_BACK, omtl->alpha);

        MaterialSet::iterator mitr = materialSet.find(mtl);
        if (mitr==materialSet.end())
        {
            materialSet.insert(mtl);
        }
        else
        {
            mtl = *mitr;
        }

        stateset->setAttribute(mtl.get());

        if (omtl->alpha<1.0f) {
            stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
            stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        }

        if (omtl->textureName)
        {
            TextureMap::iterator titr = textureMap.find(omtl->textureName);
            if (titr==textureMap.end())
            {
        
                std::string fileName = osgDB::findFileInDirectory(omtl->textureName,directory,true);
                if (!fileName.empty())
                {

                    osg::Image* osg_image = osgDB::readImageFile(fileName.c_str());
                    if (osg_image)
                    {
                        osg::Texture2D* osg_texture = new osg::Texture2D;
                        osg_texture->setImage(osg_image);
                        stateset->setTextureAttributeAndModes(0,osg_texture,osg::StateAttribute::ON);
                        
                        textureMap[omtl->textureName] = osg_texture;

                    }
                    else
                    {
                        osg::notify(osg::NOTICE) << "Warning: Cannot create texture "<<omtl->textureName<< std::endl;
                    }
                }
                else
                {
                    osg::notify(osg::WARN) << "texture '"<<omtl->textureName<<"' not found"<< std::endl;
                }
                
            }
            else
            {
                stateset->setTextureAttributeAndModes(0,titr->second.get(),osg::StateAttribute::ON);
            }
        }
        
        if (omtl->textureReflection)
        {
            stateset->setTextureAttributeAndModes(0,osg_texgen.get(),osg::StateAttribute::ON);
        }
        
        StateSetSet::iterator sitr = statesetSet.find(stateset);
        if (sitr==statesetSet.end())
        {
            osg_mtl[i] = stateset;
            statesetSet.insert(stateset);
        }
        else
        {
            osg_mtl[i] = *sitr;
        }        
    }

    // toplevel group or transform
    osg::Group* osg_top = NULL;
    if (obj->position[0] != 0.0f || obj->position[2] != 0.0f || obj->position[2] != 0.0f) {
        osg::MatrixTransform* xform = new osg::MatrixTransform;
        // note obj_x -> osg_x,
        //      obj_y -> osg_z,
        //      obj_z -> osg_y,
        xform->setMatrix(osg::Matrix::translate(obj->position[0], obj->position[2], obj->position[1]));
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
            osg::Drawable* drawable = makeDrawable(obj,ogrp);
            
            // state and material (if any)
            if (!osg_mtl.empty()) {
                drawable->setStateSet(osg_mtl[ogrp->material].get());
            }

            osg_geo->addDrawable(drawable);
            osg_top->addChild(osg_geo);
        }
        ogrp = ogrp->next;
    }

    // free
    glmDelete(obj);

    return osg_top;
}


// make drawable from OBJ group
osg::Drawable* ReaderWriterOBJ::makeDrawable(GLMmodel* obj,
                                             GLMgroup* grp)
{

    GLMtriangle* tris = obj->triangles;

    unsigned int ntris = grp->numtriangles;
    unsigned int i = 0;

    // geometry
    osg::Geometry* geom = new osg::Geometry;
    
    // geom->setUseVertexBufferObjects(true);

    // primitives are only triangles
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,0,ntris*3));

    // the following code for mapping the coords, normals and texcoords
    // is complicated greatly by the need to create separate out the
    // sets of coords etc for each drawable.

    bool needNormals = obj->normals && obj->normals>0;
    bool needTexcoords = obj->texcoords && obj->numtexcoords>0 && grp->hastexcoords;
    
    
    osg::Vec3Array* coordArray = new osg::Vec3Array(3*ntris);
    
    osg::Vec3Array::iterator coords = coordArray->begin();
    geom->setVertexArray(coordArray);
    
    osg::Vec3Array::iterator normals = osg::Vec3Array::iterator();// dummy assignment to get round stupid compiler warnings.
    if (needNormals)
    {
        osg::Vec3Array* normalArray = new osg::Vec3Array(3*ntris);
        geom->setNormalArray(normalArray);
        geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
        normals = normalArray->begin();
    }

    osg::Vec2Array::iterator texcoords = osg::Vec2Array::iterator(); // dummy assignment to get round stupid compiler warnings.
    if (needTexcoords)
    {
        osg::Vec2Array* texCoordArray = new osg::Vec2Array(3*ntris);
        geom->setTexCoordArray(0,texCoordArray);
        
        texcoords = texCoordArray->begin();
    }

    // first count the number of vertices used in this group.
    for (i = 0; i < ntris; i++)
    {
        GLMtriangle* tri = &(tris[grp->triangles[i]]);
        
        for(int corner=0;corner<3;++corner)
        {
            int ci = tri->vindices[corner]*3;

            // note obj_x -> osg_x,
            //      obj_y -> -osg_z,
            //      obj_z -> osg_y,
            coords->set(obj->vertices[ci],-obj->vertices[ci+2],obj->vertices[ci+1]);
            ++coords;

            if (needNormals)
            {
                int ni = tri->nindices[corner]*3;

                // note obj_x -> osg_x,
                //      obj_y -> osg_z,
                //      obj_z -> osg_y,
                // to rotate the about x axis to have model z upwards.
                normals->set(obj->normals[ni],-obj->normals[ni+2],obj->normals[ni+1]);
                ++normals;
            }

            if (needTexcoords)
            {
                int ti = tri->tindices[corner]*2;
                texcoords->set(obj->texcoords[ti+0], obj->texcoords[ti+1]);
                ++texcoords;
            }
        }
    }

    return geom;
}

