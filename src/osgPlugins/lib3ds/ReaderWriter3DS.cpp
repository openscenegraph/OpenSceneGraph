#include <osg/Notify>
#include <osg/Group>
#include <osg/Geode>
#include <osg/GeoSet>
#include <osg/Texture>
#include <osg/Material>
#include <osg/TexEnv>
#include <osg/ref_ptr>

#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>

#include <file.h>
#include <mesh.h>
#include <material.h>
#include <vector.h>

#include <stdlib.h>
#include <string.h>

#include <set>
#include <map>

class ReaderWriter3DS : public osgDB::ReaderWriter
{
    public:

        ReaderWriter3DS();

        virtual const char* className() { return "3DS Auto Studio Reader"; }
        virtual bool acceptsExtension(const std::string& extension) { return extension=="3ds"; }

        virtual osg::Node* readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*);

        typedef std::vector<int> FaceList;
        typedef std::map<std::string,osg::StateSet*> GeoStateMap;

    protected:

        osg::Texture*  createTexture(Lib3dsTextureMap *texture,const char* label,bool& transparancy);
        osg::StateSet* createStateSet(Lib3dsMaterial *materials);
        osg::GeoSet*   createGeoSet(Lib3dsMesh *meshes,FaceList& faceList);

        std::string _directory;
        bool _useSmoothingGroups;
        bool _usePerVertexNormals;
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriter3DS> g_readerWriter_3DS_Proxy;

ReaderWriter3DS::ReaderWriter3DS()
{
    _useSmoothingGroups = true;
    _usePerVertexNormals = true;
}


osg::Node* ReaderWriter3DS::readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*)
{

    Lib3dsFile *f = lib3ds_open(fileName.c_str());
    if (f==NULL) return NULL;

    _directory = osgDB::getFilePath(fileName);

    osg::Group* group = new osg::Group;
    group->setName(fileName);

    typedef std::map<std::string,osg::StateSet*> StateSetMap;
    StateSetMap drawStateMap;

    for (Lib3dsMaterial *mat=f->materials; mat; mat=mat->next)
    {
        drawStateMap[mat->name] = createStateSet(mat);
    }

    for (Lib3dsMesh *mesh=f->meshes; mesh; mesh=mesh->next)
    {

        typedef std::vector<int> FaceList;
        typedef std::map<std::string,FaceList> MaterialFaceMap;
        MaterialFaceMap materialFaceMap;
        for (unsigned int i=0; i<mesh->faces; ++i)
        {
            materialFaceMap[mesh->faceL[i].material].push_back(i);
        }

        if (materialFaceMap.empty())
        {
            osg::notify(osg::NOTICE)<<"Warning : no triangles assigned to mesh '"<<mesh->name<<"'"<<endl;
        }
        else
        {

            osg::Geode* geode = new osg::Geode;
            geode->setName(mesh->name);

            for(MaterialFaceMap::iterator itr=materialFaceMap.begin();
                itr!=materialFaceMap.end();
                ++itr)
            {
                FaceList& faceList = itr->second;
                
                if (_useSmoothingGroups)
                {

                    typedef std::map<int,FaceList> SmoothingFaceMap;
                    SmoothingFaceMap smoothingFaceMap;
                    for (FaceList::iterator flitr=faceList.begin();
                         flitr!=faceList.end();
                         ++flitr)
                    {
                        smoothingFaceMap[mesh->faceL[*flitr].smoothing].push_back(*flitr);
                    }

                    for(SmoothingFaceMap::iterator sitr=smoothingFaceMap.begin();
                        sitr!=smoothingFaceMap.end();
                        ++sitr)
                    {
                        // each smoothing group to have its own geoset 
                        // to ensure the vertices on adjacent groups
                        // don't get shared.
                        FaceList& smoothFaceMap = sitr->second;

                        osg::GeoSet* geoset = createGeoSet(mesh,smoothFaceMap);
                        if (geoset)
                        {
                            geoset->setStateSet(drawStateMap[itr->first]);
                            geode->addDrawable(geoset);
                        }
                    }
                }
                else // ignore smoothing groups.
                {
                    osg::GeoSet* geoset = createGeoSet(mesh,faceList);
                    if (geoset)
                    {
                        geoset->setStateSet(drawStateMap[itr->first]);
                        geode->addDrawable(geoset);
                    }
                }
            }

            group->addChild(geode);

        }

    }

    lib3ds_close(f);

    return group;
}


osg::Texture*  ReaderWriter3DS::createTexture(Lib3dsTextureMap *texture,const char* label,bool& transparancy)
{
    if (texture && *(texture->name))
    {
        std::string fileName = osgDB::findFileInDirectory(texture->name,_directory,true);
        if (fileName.empty())
        {
            osg::notify(osg::WARN) << "texture '"<<texture->name<<"' not found"<<endl;
            return NULL;
        }

        if (label) osg::notify(osg::DEBUG_INFO) << label;
        else osg::notify(osg::DEBUG_INFO) << "texture name";
        osg::notify(osg::DEBUG_INFO) << " '"<<texture->name<<"'"<<endl;
        osg::notify(osg::DEBUG_INFO) << "    texture flag        "<<texture->flags<<endl;
        osg::notify(osg::DEBUG_INFO) << "    LIB3DS_DECALE       "<<((texture->flags)&LIB3DS_DECALE)<<endl;
        osg::notify(osg::DEBUG_INFO) << "    LIB3DS_MIRROR       "<<((texture->flags)&LIB3DS_MIRROR)<<endl;
        osg::notify(osg::DEBUG_INFO) << "    LIB3DS_NEGATE       "<<((texture->flags)&LIB3DS_NEGATE)<<endl;
        osg::notify(osg::DEBUG_INFO) << "    LIB3DS_NO_TILE      "<<((texture->flags)&LIB3DS_NO_TILE)<<endl;
        osg::notify(osg::DEBUG_INFO) << "    LIB3DS_SUMMED_AREA  "<<((texture->flags)&LIB3DS_SUMMED_AREA)<<endl;
        osg::notify(osg::DEBUG_INFO) << "    LIB3DS_ALPHA_SOURCE "<<((texture->flags)&LIB3DS_ALPHA_SOURCE)<<endl;
        osg::notify(osg::DEBUG_INFO) << "    LIB3DS_TINT         "<<((texture->flags)&LIB3DS_TINT)<<endl;
        osg::notify(osg::DEBUG_INFO) << "    LIB3DS_IGNORE_ALPHA "<<((texture->flags)&LIB3DS_IGNORE_ALPHA)<<endl;
        osg::notify(osg::DEBUG_INFO) << "    LIB3DS_RGB_TINT     "<<((texture->flags)&LIB3DS_RGB_TINT)<<endl;

        osg::Image* osg_image = osgDB::readImageFile(fileName.c_str());
        if (osg_image==NULL)
        {
            osg::notify(osg::NOTICE) << "Warning: Cannot create texture "<<texture->name<<endl;
            return NULL;
        }

        osg::Texture* osg_texture = new osg::Texture;
        osg_texture->setImage(osg_image);

        // does the texture support transparancy?
        transparancy = ((texture->flags)&LIB3DS_ALPHA_SOURCE)!=0;

        // what is the wrap mode of the texture.
        osg::Texture::WrapMode wm = ((texture->flags)&LIB3DS_NO_TILE) ?
        osg::Texture::CLAMP : wm=osg::Texture::REPEAT;
        osg_texture->setWrap(osg::Texture::WRAP_S,wm);
        osg_texture->setWrap(osg::Texture::WRAP_T,wm);
        osg_texture->setWrap(osg::Texture::WRAP_R,wm);
                                 // bilinear.
        osg_texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_NEAREST);

        return osg_texture;
    }
    else
        return NULL;
}


osg::StateSet* ReaderWriter3DS::createStateSet(Lib3dsMaterial *mat)
{
    if (mat==NULL) return NULL;

    osg::StateSet* stateset = new osg::StateSet;

    osg::Material* material = new osg::Material;

    float transparency = mat->transparency;
    float alpha = 1.0f-transparency;

    osg::Vec4 ambient(mat->ambient[0],mat->ambient[1],mat->ambient[2],alpha);
    osg::Vec4 diffuse(mat->diffuse[0],mat->diffuse[1],mat->diffuse[2],alpha);
    osg::Vec4 specular(mat->specular[0],mat->specular[1],mat->specular[2],alpha);

    float shininess = mat->shininess;

    material->setAmbient(osg::Material::FRONT_AND_BACK,ambient);
    material->setDiffuse(osg::Material::FRONT_AND_BACK,diffuse);
    material->setSpecular(osg::Material::FRONT_AND_BACK,specular);
    material->setShininess(osg::Material::FRONT_AND_BACK,shininess);

    stateset->setAttribute(material);

    bool decal = false;
    bool textureTransparancy=false;
    osg::Texture* texture1_map = createTexture(&(mat->texture1_map),"texture1_map",textureTransparancy);
    if (texture1_map)
    {
        stateset->setAttributeAndModes(texture1_map,osg::StateAttribute::ON);
        
        // not sure exactly how to interpret what is best for .3ds
        // but the default text env MODULATE doesn't work well, and
        // DECAL seems to work better.
        osg::TexEnv* texenv = new osg::TexEnv;
        if (decal)
        {
            texenv->setMode(osg::TexEnv::DECAL);
        }
        else
        {
            texenv->setMode(osg::TexEnv::MODULATE);
        }

        stateset->setAttribute(texenv);
    }

    if (transparency>0.0f || textureTransparancy)
    {
        stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
        stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    }

/*
    osg::ref_ptr<osg::Texture> texture1_mask = createTexture(&(mat->texture1_mask),"texture1_mask",textureTransparancy);
    osg::ref_ptr<osg::Texture> texture2_map = createTexture(&(mat->texture2_map),"texture2_map",textureTransparancy);
    osg::ref_ptr<osg::Texture> texture2_mask = createTexture(&(mat->texture2_mask),"texture2_mask",textureTransparancy);
    osg::ref_ptr<osg::Texture> opacity_map = createTexture(&(mat->opacity_map),"opacity_map",textureTransparancy);
    osg::ref_ptr<osg::Texture> opacity_mask = createTexture(&(mat->opacity_mask),"opacity_mask",textureTransparancy);
    osg::ref_ptr<osg::Texture> bump_map = createTexture(&(mat->bump_map),"bump_map",textureTransparancy);
    osg::ref_ptr<osg::Texture> bump_mask = createTexture(&(mat->bump_mask),"bump_mask",textureTransparancy);
    osg::ref_ptr<osg::Texture> specular_map = createTexture(&(mat->specular_map),"specular_map",textureTransparancy);
    osg::ref_ptr<osg::Texture> specular_mask = createTexture(&(mat->specular_mask),"specular_mask",textureTransparancy);
    osg::ref_ptr<osg::Texture> shininess_map = createTexture(&(mat->shininess_map),"shininess_map",textureTransparancy);
    osg::ref_ptr<osg::Texture> shininess_mask = createTexture(&(mat->shininess_mask),"shininess_mask",textureTransparancy);
    osg::ref_ptr<osg::Texture> self_illum_map = createTexture(&(mat->self_illum_map),"self_illum_map",textureTransparancy);
    osg::ref_ptr<osg::Texture> self_illum_mask = createTexture(&(mat->self_illum_mask),"self_illum_mask",textureTransparancy);
    osg::ref_ptr<osg::Texture> reflection_map = createTexture(&(mat->reflection_map),"reflection_map",textureTransparancy);
    osg::ref_ptr<osg::Texture> reflection_mask = createTexture(&(mat->reflection_mask),"reflection_mask",textureTransparancy);
*/
    return stateset;
}


osg::GeoSet*   ReaderWriter3DS::createGeoSet(Lib3dsMesh *m,FaceList& faceList)
{

    osg::GeoSet* geoset = new osg::GeoSet;

    unsigned int i;

    std::vector<int> orig2NewMapping;
    for(i=0;i<m->points;++i) orig2NewMapping.push_back(-1);

    unsigned int noVertex=0;
    FaceList::iterator fitr;
    for (fitr=faceList.begin();
        fitr!=faceList.end();
        ++fitr)
    {

        Lib3dsFace& face = m->faceL[*fitr];

        if (orig2NewMapping[face.points[0]]<0)
            orig2NewMapping[face.points[0]] = noVertex++;

        if (orig2NewMapping[face.points[1]]<0)
            orig2NewMapping[face.points[1]] = noVertex++;

        if (orig2NewMapping[face.points[2]]<0)
            orig2NewMapping[face.points[2]] = noVertex++;

    }

    osg::Vec3* osg_coords = new osg::Vec3[noVertex];
    for (i=0; i<m->points; ++i)
    {
        // lib3ds_vector_transform(pos, m->matrix, m->pointL[i].pos);
        if (orig2NewMapping[i]>=0) osg_coords[orig2NewMapping[i]].set(m->pointL[i].pos[0],m->pointL[i].pos[1],m->pointL[i].pos[2]);
    }

    osg::Vec2* osg_tcoords = NULL;
    if (m->texels>0)
    {
        if (m->texels==m->points)
        {
            osg_tcoords = new osg::Vec2[noVertex];
            for (i=0; i<m->texels; ++i)
            {
                if (orig2NewMapping[i]>=0) osg_tcoords[orig2NewMapping[i]].set(m->texelL[i][0],m->texelL[i][1]);
            }
        }
        else
        {
            osg::notify(osg::WARN)<<"Warning: in 3ds loader m->texels ("<<m->texels<<") != m->points ("<<m->points<<")"<<endl;
        }
    }


    // handle normals.
    osg::Vec3* osg_normals;
    if (_usePerVertexNormals)
    {
        osg_normals = new osg::Vec3[noVertex];
        
        // initialize normal list to zero's.
        for (i=0; i<noVertex; ++i)
        {
            osg_normals[i].set(0.0f,0.0f,0.0f);
        }
    }
    else 
    {
        osg_normals = new osg::Vec3[faceList.size()];
    }
    
    osg::Vec3* normal_ptr = osg_normals;

    int numIndices = faceList.size()*3;
    osg::ushort* osg_indices = new osg::ushort[numIndices];
    osg::ushort* index_ptr = osg_indices;

    for (fitr=faceList.begin();
        fitr!=faceList.end();
        ++fitr)
    {
        Lib3dsFace& face = m->faceL[*fitr];

        *(index_ptr++) = orig2NewMapping[face.points[0]];
        *(index_ptr++) = orig2NewMapping[face.points[1]];
        *(index_ptr++) = orig2NewMapping[face.points[2]];

        if (_usePerVertexNormals)
        {
            osg_normals[orig2NewMapping[face.points[0]]] += osg::Vec3(face.normal[0],face.normal[1],face.normal[2]);;
            osg_normals[orig2NewMapping[face.points[1]]] += osg::Vec3(face.normal[0],face.normal[1],face.normal[2]);;
            osg_normals[orig2NewMapping[face.points[2]]] += osg::Vec3(face.normal[0],face.normal[1],face.normal[2]);;
        }
        else
        {
            *(normal_ptr++) =  osg::Vec3(face.normal[0],face.normal[1],face.normal[2]);
        }
        
    }

    if (_usePerVertexNormals)
    {
        // normalize the normal list to unit length normals.
        for (i=0; i<noVertex; ++i)
        {
//            osg_normals[i].normalize();
            float len = osg_normals[i].length();
            if (len) osg_normals[i]/=len;
        }

        geoset->setNormals(osg_normals,osg_indices);
        geoset->setNormalBinding(osg::GeoSet::BIND_PERVERTEX);

    }
    else
    {
        geoset->setNormals(osg_normals);
        geoset->setNormalBinding(osg::GeoSet::BIND_PERPRIM);
    }


    geoset->setCoords(osg_coords,osg_indices);
    if (osg_tcoords)
    {
        geoset->setTextureBinding(osg::GeoSet::BIND_PERVERTEX);
        geoset->setTextureCoords(osg_tcoords,osg_indices);
    }

    geoset->setPrimType(osg::GeoSet::TRIANGLES);
    geoset->setNumPrims(faceList.size());

    return geoset;
}
