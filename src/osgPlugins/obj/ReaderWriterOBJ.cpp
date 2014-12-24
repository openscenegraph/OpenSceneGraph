// -*-c++-*-

/*
 * Wavefront OBJ loader for Open Scene Graph
 *
 * Copyright (C) 2001,2007 Ulrich Hertlein <u.hertlein@sandbox.de>
 *
 * Modified by Robert Osfield to support per Drawable coord, normal and
 * texture coord arrays, bug fixes, and support for texture mapping.
 *
 * Writing support added 2007 by Stephan Huber, http://digitalmind.de,
 * some ideas taken from the dae-plugin
 *
 * The Open Scene Graph (OSG) is a cross platform C++/OpenGL library for
 * real-time rendering of large 3D photo-realistic models.
 * The OSG homepage is http://www.openscenegraph.org/
 */

#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <stdlib.h>
#include <string>

#include <osg/Notify>
#include <osg/Node>
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/Vec3f>

#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/TexGen>
#include <osg/TexMat>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <osgUtil/TriStripVisitor>
#include <osgUtil/SmoothingVisitor>
#include <osgUtil/Tessellator>

#include "obj.h"
#include "OBJWriterNodeVisitor.h"

#include <map>
#include <set>

class ReaderWriterOBJ : public osgDB::ReaderWriter
{
public:
    ReaderWriterOBJ()
    {
        supportsExtension("obj","Alias Wavefront OBJ format");
        supportsOption("noRotation","Do not do the default rotate about X axis");
        supportsOption("noTesselateLargePolygons","Do not do the default tesselation of large polygons");
        supportsOption("noTriStripPolygons","Do not do the default tri stripping of polygons");
        supportsOption("generateFacetNormals","generate facet normals for verticies without normals");
        supportsOption("noReverseFaces","avoid to reverse faces when normals and triangles orientation are reversed");

        supportsOption("DIFFUSE=<unit>", "Set texture unit for diffuse texture");
        supportsOption("AMBIENT=<unit>", "Set texture unit for ambient texture");
        supportsOption("SPECULAR=<unit>", "Set texture unit for specular texture");
        supportsOption("SPECULAR_EXPONENT=<unit>", "Set texture unit for specular exponent texture");
        supportsOption("OPACITY=<unit>", "Set texture unit for opacity/dissolve texture");
        supportsOption("BUMP=<unit>", "Set texture unit for bumpmap texture");
        supportsOption("DISPLACEMENT=<unit>", "Set texture unit for displacement texture");
        supportsOption("REFLECTION=<unit>", "Set texture unit for reflection texture");

    }

    virtual const char* className() const { return "Wavefront OBJ Reader"; }

    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const;

    virtual ReadResult readNode(std::istream& fin, const Options* options) const;

    virtual WriteResult writeObject(const osg::Object& obj,const std::string& fileName,const Options* options=NULL) const
    {
        const osg::Node* node = dynamic_cast<const osg::Node*>(&obj);
        if (node)
            return writeNode(*node, fileName, options);
        else
            return WriteResult(WriteResult::FILE_NOT_HANDLED);
    }

    virtual WriteResult writeNode(const osg::Node& node,const std::string& fileName,const Options* options =NULL) const
    {
        if (!acceptsExtension(osgDB::getFileExtension(fileName)))
            return WriteResult(WriteResult::FILE_NOT_HANDLED);

        osgDB::ofstream f(fileName.c_str());
        std::string materialFile = osgDB::getNameLessExtension(fileName) + ".mtl";
        OBJWriterNodeVisitor nv(f, osgDB::getSimpleFileName(materialFile));

        // we must cast away constness
        (const_cast<osg::Node*>(&node))->accept(nv);

        osgDB::ofstream mf(materialFile.c_str());
        nv.writeMaterials(mf);

        return WriteResult(WriteResult::FILE_SAVED);
    }


    virtual WriteResult writeObject(const osg::Object& obj,std::ostream& fout,const Options* options=NULL) const
    {
        const osg::Node* node = dynamic_cast<const osg::Node*>(&obj);
        if (node)
            return writeNode(*node, fout, options);
        else
            return WriteResult(WriteResult::FILE_NOT_HANDLED);
    }

    virtual WriteResult writeNode(const osg::Node& node,std::ostream& fout,const Options* =NULL) const
    {
        // writing to a stream does not support materials

        OBJWriterNodeVisitor nv(fout);

        // we must cast away constness
        (const_cast<osg::Node*>(&node))->accept(nv);

        return WriteResult(WriteResult::FILE_SAVED);
    }



protected:

     struct ObjOptionsStruct {
        bool rotate;
        bool noTesselateLargePolygons;
        bool noTriStripPolygons;
        bool generateFacetNormals;
        bool fixBlackMaterials;
        bool noReverseFaces;
        // This is the order in which the materials will be assigned to texture maps, unless
        // otherwise overriden
        typedef std::vector< std::pair<int,obj::Material::Map::TextureMapType> > TextureAllocationMap;
        TextureAllocationMap textureUnitAllocation;
    };

    typedef std::map< std::string, osg::ref_ptr<osg::StateSet> > MaterialToStateSetMap;

    void buildMaterialToStateSetMap(obj::Model& model, MaterialToStateSetMap& materialToSetSetMapObj, ObjOptionsStruct& localOptions, const Options* options) const;

    osg::Geometry* convertElementListToGeometry(obj::Model& model, obj::Model::ElementList& elementList, ObjOptionsStruct& localOptions) const;

    osg::Node* convertModelToSceneGraph(obj::Model& model, ObjOptionsStruct& localOptions, const Options* options) const;

    inline osg::Vec3 transformVertex(const osg::Vec3& vec, const bool rotate) const ;
    inline osg::Vec3 transformNormal(const osg::Vec3& vec, const bool rotate) const ;

    ObjOptionsStruct parseOptions(const Options* options) const;


};

inline osg::Vec3 ReaderWriterOBJ::transformVertex(const osg::Vec3& vec, const bool rotate) const
{
    if(rotate==true)
    {
        return osg::Vec3(vec.x(),-vec.z(),vec.y());
    }
    else
    {
        return vec;
    }
}

inline osg::Vec3 ReaderWriterOBJ::transformNormal(const osg::Vec3& vec, const bool rotate) const
{
    if(rotate==true)
    {
        return osg::Vec3(vec.x(),-vec.z(),vec.y());
    }
    else
    {
        return vec;
    }
}


// register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN(obj, ReaderWriterOBJ)

static void load_material_texture(    obj::Model &model,
                                    obj::Material::Map &map,
                                    osg::StateSet *stateset,
                                    const unsigned int texture_unit,
                                    const osgDB::Options* options)
{
    std::string filename = map.name;
    if (!filename.empty())
    {
        osg::ref_ptr< osg::Image > image;
        if ( !model.getDatabasePath().empty() )
        {
            // first try with database path of parent.
            image = osgDB::readRefImageFile(model.getDatabasePath()+'/'+filename, options);
        }

        if ( !image.valid() )
        {
            // if not already set then try the filename as is.
            image = osgDB::readRefImageFile(filename, options);
        }

        if ( image.valid() )
        {
            osg::Texture2D* texture = new osg::Texture2D( image.get() );
            osg::Texture::WrapMode textureWrapMode;
            if(map.clamp == true)
            {
                textureWrapMode = osg::Texture::CLAMP_TO_BORDER;
                texture->setBorderColor(osg::Vec4(0.0,0.0,0.0,0.0));    // transparent
                //stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
                //stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
            }
            else
            {
                textureWrapMode = osg::Texture::REPEAT;
            }

            texture->setWrap(osg::Texture2D::WRAP_R, textureWrapMode);
            texture->setWrap(osg::Texture2D::WRAP_S, textureWrapMode);
            texture->setWrap(osg::Texture2D::WRAP_T, textureWrapMode);
            stateset->setTextureAttributeAndModes( texture_unit, texture,osg::StateAttribute::ON );

            if ( map.type == obj::Material::Map::REFLECTION )
            {
                osg::TexGen* texgen = new osg::TexGen;
                texgen->setMode(osg::TexGen::SPHERE_MAP);
                stateset->setTextureAttributeAndModes( texture_unit,texgen,osg::StateAttribute::ON );
            }

            if  ( image->isImageTranslucent())
            {
                OSG_INFO<<"Found transparent image"<<std::endl;
                stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
                stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
            }
        }
    }

    if (map.uScale != 1.0f || map.vScale != 1.0f ||
            map.uOffset != 0.0f || map.vOffset != 0.0f)
    {
        osg::Matrix mat;
        if (map.uScale != 1.0f || map.vScale != 1.0f)
        {
            OSG_DEBUG << "Obj TexMat scale=" << map.uScale << "," << map.vScale << std::endl;
            mat *= osg::Matrix::scale(map.uScale, map.vScale, 1.0);
        }
        if (map.uOffset != 0.0f || map.vOffset != 0.0f)
        {
            OSG_DEBUG << "Obj TexMat offset=" << map.uOffset << "," << map.uOffset << std::endl;
            mat *= osg::Matrix::translate(map.uOffset, map.vOffset, 0.0);
        }

        osg::TexMat* texmat = new osg::TexMat;
        texmat->setMatrix(mat);
        stateset->setTextureAttributeAndModes( texture_unit,texmat,osg::StateAttribute::ON );
    }
}


void ReaderWriterOBJ::buildMaterialToStateSetMap(obj::Model& model, MaterialToStateSetMap& materialToStateSetMap, ObjOptionsStruct& localOptions, const Options* options) const
{
    if (localOptions.fixBlackMaterials)
    {
        // hack to fix Maya exported models that contian all black materials.
        int numBlack = 0;
        int numNotBlack = 0;
        obj::Model::MaterialMap::iterator itr;
        for(itr = model.materialMap.begin();
            itr != model.materialMap.end();
            ++itr)
        {
            obj::Material& material = itr->second;
            if (material.ambient==osg::Vec4(0.0f,0.0f,0.0f,1.0f) &&
                material.diffuse==osg::Vec4(0.0f,0.0f,0.0f,1.0f))
            {
                ++numBlack;
            }
            else
            {
                ++numNotBlack;
            }
        }

        if (numNotBlack==0 && numBlack!=0)
        {
            for(itr = model.materialMap.begin();
                itr != model.materialMap.end();
                ++itr)
            {
                obj::Material& material = itr->second;
                if (material.ambient==osg::Vec4(0.0f,0.0f,0.0f,1.0f) &&
                    material.diffuse==osg::Vec4(0.0f,0.0f,0.0f,1.0f))
                {
                    material.ambient.set(0.3f,0.3f,0.3f,1.0f);
                    material.diffuse.set(1.0f,1.0f,1.0f,1.0f);
                }
            }
        }
    }

    for(obj::Model::MaterialMap::iterator itr = model.materialMap.begin();
        itr != model.materialMap.end();
        ++itr)
    {
        obj::Material& material = itr->second;

        osg::ref_ptr< osg::StateSet > stateset = new osg::StateSet;

        bool isTransparent = false;

        // handle material colors
        // http://java3d.j3d.org/utilities/loaders/obj/sun.html
        if (material.illum != 0)
        {
            osg::Material* osg_material = new osg::Material;
            stateset->setAttribute(osg_material);
            osg_material->setName(material.name);
            osg_material->setAmbient(osg::Material::FRONT_AND_BACK,material.ambient);
            osg_material->setDiffuse(osg::Material::FRONT_AND_BACK,material.diffuse);
            osg_material->setEmission(osg::Material::FRONT_AND_BACK,material.emissive);

            if (material.illum == 2) {
                osg_material->setSpecular(osg::Material::FRONT_AND_BACK,material.specular);
            } else {
                osg_material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
            }
            osg_material->setShininess(osg::Material::FRONT_AND_BACK,(material.Ns/1000.0f)*128.0f ); // note OBJ shiniess is 0..1000.

            if (material.ambient[3]!=1.0 ||
                material.diffuse[3]!=1.0 ||
                material.specular[3]!=1.0||
                material.emissive[3]!=1.0)
            {
                OSG_INFO<<"Found transparent material"<<std::endl;
                isTransparent = true;
            }
        }

        // If the user has explicitly set the required texture type to unit map via the options
        // string, then we load ONLY those textures that are in the map.
        if(localOptions.textureUnitAllocation.size()>0)
        {
            for(unsigned int i=0;i<localOptions.textureUnitAllocation.size();i++)
            {
                // firstly, get the option set pair
                int unit = localOptions.textureUnitAllocation[i].first;
                obj::Material::Map::TextureMapType type = localOptions.textureUnitAllocation[i].second;
                // secondly, see if this texture type (e.g. DIFFUSE) is one of those in the material
                int index = -1;
                for(unsigned int j=0;j<material.maps.size();j++)
                {
                    if(material.maps[j].type == type)
                    {
                        index = (int) j;
                        break;
                    }
                }
                if(index>=0) load_material_texture( model, material.maps[index], stateset.get(), unit, options );
            }
        }
        // If the user has set no options, then we load them up in the order contained in the enum. This
        // latter method is an attempt not to break user's existing code
        else
        {
            int unit = 0;
            for(int i=0;i<(int) obj::Material::Map::UNKNOWN;i++) // for each type
            {
                obj::Material::Map::TextureMapType type = (obj::Material::Map::TextureMapType) i;
                // see if this texture type (e.g. DIFFUSE) is one of those in the material
                int index = -1;
                for(unsigned int j=0;j<material.maps.size();j++)
                {
                    if(material.maps[j].type == type)
                    {
                        index = (int) j;
                        break;
                    }
                }
                if(index>=0)
                {
                    load_material_texture( model, material.maps[index], stateset.get(), unit, options );
                    unit++;
                }
            }
        }

        if (isTransparent)
        {
            stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
            stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        }

        materialToStateSetMap[material.name] = stateset.get();
    }
}

osg::Geometry* ReaderWriterOBJ::convertElementListToGeometry(obj::Model& model, obj::Model::ElementList& elementList, ObjOptionsStruct& localOptions) const
{

    unsigned int numVertexIndices = 0;
    unsigned int numNormalIndices = 0;
    unsigned int numTexCoordIndices = 0;

    unsigned int numPointElements = 0;
    unsigned int numPolylineElements = 0;
    unsigned int numPolygonElements = 0;

    obj::Model::ElementList::iterator itr;

    if (localOptions.generateFacetNormals == true) {
        for(itr=elementList.begin();
                itr!=elementList.end();
                    ++itr)
        {
            obj::Element& element = *(*itr);
            if (element.dataType==obj::Element::POINTS || element.dataType==obj::Element::POLYLINE)
                continue;

            if (element.normalIndices.size() == 0) {
                // fill in the normals
                int a = element.vertexIndices[0];
                int b = element.vertexIndices[1];
                int c = element.vertexIndices[2];

                osg::Vec3f ab(model.vertices[b]);
                osg::Vec3f ac(model.vertices[c]);

                ab -= model.vertices[a];
                ac -= model.vertices[a];

                osg::Vec3f Norm( ab ^ ac );
                Norm.normalize();
                int normal_idx = model.normals.size();
                model.normals.push_back(Norm);

                for (unsigned i=0 ; i < element.vertexIndices.size() ; i++)
                    element.normalIndices.push_back(normal_idx);
            }
        }
    }



    for(itr=elementList.begin();
        itr!=elementList.end();
        ++itr)
    {
        obj::Element& element = *(*itr);

        numVertexIndices += element.vertexIndices.size();
        numNormalIndices += element.normalIndices.size();
        numTexCoordIndices += element.texCoordIndices.size();

        numPointElements += (element.dataType==obj::Element::POINTS) ? 1 : 0;
        numPolylineElements += (element.dataType==obj::Element::POLYLINE) ? 1 : 0;
        numPolygonElements += (element.dataType==obj::Element::POLYGON) ? 1 : 0;

    }

    if (numVertexIndices==0) return 0;

    if (numNormalIndices!=0 && numNormalIndices!=numVertexIndices)
    {
        OSG_NOTICE<<"Incorrect number of normals, ignore them"<<std::endl;
        numNormalIndices = 0;
    }

    if (numTexCoordIndices!=0 && numTexCoordIndices!=numVertexIndices)
    {
        OSG_NOTICE<<"Incorrect number of normals, ignore them"<<std::endl;
        numTexCoordIndices = 0;
    }

    osg::Vec3Array* vertices = numVertexIndices ? new osg::Vec3Array : 0;
    osg::Vec3Array* normals = numNormalIndices ? new osg::Vec3Array : 0;
    osg::Vec2Array* texcoords = numTexCoordIndices ? new osg::Vec2Array : 0;
    osg::Vec4Array* colors = (!model.colors.empty()) ? new osg::Vec4Array : 0;

    if (vertices) vertices->reserve(numVertexIndices);
    if (normals) normals->reserve(numNormalIndices);
    if (texcoords) texcoords->reserve(numTexCoordIndices);
    if (colors) colors->reserve(numVertexIndices);


    osg::Geometry* geometry = new osg::Geometry;
    if (vertices) geometry->setVertexArray(vertices);
    if (normals)
    {
        geometry->setNormalArray(normals, osg::Array::BIND_PER_VERTEX);
    }

    if (texcoords)
    {
        geometry->setTexCoordArray(0,texcoords);
    }

    if (colors)
    {
        geometry->setColorArray(colors);
        geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    }

    if (numPointElements>0)
    {
        unsigned int startPos = vertices->size();
        unsigned int numPoints = 0;
        for(itr=elementList.begin();
            itr!=elementList.end();
            ++itr)
        {
            obj::Element& element = *(*itr);
            if (element.dataType==obj::Element::POINTS)
            {
                for(obj::Element::IndexList::iterator index_itr = element.vertexIndices.begin();
                    index_itr != element.vertexIndices.end();
                    ++index_itr)
                {
                    // if use color extension ( not standard but used by meshlab)
                    if (colors)
                    {
                        colors->push_back(model.colors[*index_itr]);
                    }
                    vertices->push_back(transformVertex(model.vertices[*index_itr],localOptions.rotate));
                    ++numPoints;
                }
                if (numNormalIndices)
                {
                    for(obj::Element::IndexList::iterator index_itr = element.normalIndices.begin();
                        index_itr != element.normalIndices.end();
                        ++index_itr)
                    {
                        normals->push_back(transformNormal(model.normals[*index_itr],localOptions.rotate));
                    }
                }
                if (numTexCoordIndices)
                {
                    for(obj::Element::IndexList::iterator index_itr = element.texCoordIndices.begin();
                        index_itr != element.texCoordIndices.end();
                        ++index_itr)
                    {
                        texcoords->push_back(model.texcoords[*index_itr]);
                    }
                }
            }
        }

        osg::DrawArrays* drawArrays = new osg::DrawArrays(GL_POINTS,startPos,numPoints);
        geometry->addPrimitiveSet(drawArrays);
    }

    if (numPolylineElements>0)
    {
        unsigned int startPos = vertices->size();
        osg::DrawArrayLengths* drawArrayLengths = new osg::DrawArrayLengths(GL_LINES,startPos);

        for(itr=elementList.begin();
            itr!=elementList.end();
            ++itr)
        {
            obj::Element& element = *(*itr);
            if (element.dataType==obj::Element::POLYLINE)
            {
                drawArrayLengths->push_back(element.vertexIndices.size());

                for(obj::Element::IndexList::iterator index_itr = element.vertexIndices.begin();
                    index_itr != element.vertexIndices.end();
                    ++index_itr)
                {
                    // if use color extension ( not standard but used by meshlab)
                    if (colors)
                    {
                        colors->push_back(model.colors[*index_itr]);
                    }

                    vertices->push_back(transformVertex(model.vertices[*index_itr],localOptions.rotate));
                }
                if (numNormalIndices)
                {
                    for(obj::Element::IndexList::iterator index_itr = element.normalIndices.begin();
                        index_itr != element.normalIndices.end();
                        ++index_itr)
                    {
                        normals->push_back(transformNormal(model.normals[*index_itr],localOptions.rotate));
                    }
                }
                if (numTexCoordIndices)
                {
                    for(obj::Element::IndexList::iterator index_itr = element.texCoordIndices.begin();
                        index_itr != element.texCoordIndices.end();
                        ++index_itr)
                    {
                        texcoords->push_back(model.texcoords[*index_itr]);
                    }
                }
            }
        }

        geometry->addPrimitiveSet(drawArrayLengths);

    }

    // #define USE_DRAWARRAYLENGTHS
    bool hasReversedFaces = false ;
    if (numPolygonElements>0)
    {
        unsigned int startPos = vertices->size();

        #ifdef USE_DRAWARRAYLENGTHS
            osg::DrawArrayLengths* drawArrayLengths = new osg::DrawArrayLengths(GL_POLYGON,startPos);
            geometry->addPrimitiveSet(drawArrayLengths);
        #endif

        for(itr=elementList.begin();
            itr!=elementList.end();
            ++itr)
        {
            obj::Element& element = *(*itr);
            if (element.dataType==obj::Element::POLYGON)
            {






                #ifdef USE_DRAWARRAYLENGTHS
                    drawArrayLengths->push_back(element.vertexIndices.size());
                #else
                    if (element.vertexIndices.size()>4)
                    {
                        osg::DrawArrays* drawArrays = new osg::DrawArrays(GL_POLYGON,startPos,element.vertexIndices.size());
                        startPos += element.vertexIndices.size();
                        geometry->addPrimitiveSet(drawArrays);
                    }
                    else
                    {
                        osg::DrawArrays* drawArrays = new osg::DrawArrays(GL_TRIANGLE_FAN,startPos,element.vertexIndices.size());
                        startPos += element.vertexIndices.size();
                        geometry->addPrimitiveSet(drawArrays);
                    }
                #endif


                if (model.needReverse(element) && !localOptions.noReverseFaces)
                {
                    hasReversedFaces = true;
                    // need to reverse so add to OSG arrays in same order as in OBJ, as OSG assume anticlockwise ordering.
                    for(obj::Element::IndexList::reverse_iterator index_itr = element.vertexIndices.rbegin();
                        index_itr != element.vertexIndices.rend();
                        ++index_itr)
                    {
                        // if use color extension ( not standard but used by meshlab)
                        if (colors)
                        {
                            colors->push_back(model.colors[*index_itr]);
                        }

                        vertices->push_back(transformVertex(model.vertices[*index_itr],localOptions.rotate));
                    }
                    if (numNormalIndices)
                    {
                        for(obj::Element::IndexList::reverse_iterator index_itr = element.normalIndices.rbegin();
                            index_itr != element.normalIndices.rend();
                            ++index_itr)
                        {
                            normals->push_back(transformNormal(model.normals[*index_itr],localOptions.rotate));
                        }
                    }


                    if (numTexCoordIndices)
                    {
                        for(obj::Element::IndexList::reverse_iterator index_itr = element.texCoordIndices.rbegin();
                            index_itr != element.texCoordIndices.rend();
                            ++index_itr)
                        {
                            texcoords->push_back(model.texcoords[*index_itr]);
                        }
                    }
                }
                else
                {
                    // no need to reverse so add to OSG arrays in same order as in OBJ.
                    for(obj::Element::IndexList::iterator index_itr = element.vertexIndices.begin();
                        index_itr != element.vertexIndices.end();
                        ++index_itr)
                    {
                        // if use color extension ( not standard but used by meshlab)
                        if (colors)
                        {
                            colors->push_back(model.colors[*index_itr]);
                        }

                        vertices->push_back(transformVertex(model.vertices[*index_itr],localOptions.rotate));
                    }
                    if (numNormalIndices)
                    {
                        for(obj::Element::IndexList::iterator index_itr = element.normalIndices.begin();
                            index_itr != element.normalIndices.end();
                            ++index_itr)
                        {
                            normals->push_back(transformNormal(model.normals[*index_itr],localOptions.rotate));
                        }
                    }
                    if (numTexCoordIndices)
                    {
                        for(obj::Element::IndexList::iterator index_itr = element.texCoordIndices.begin();
                            index_itr != element.texCoordIndices.end();
                            ++index_itr)
                        {
                            texcoords->push_back(model.texcoords[*index_itr]);
                        }
                    }
                }
            }
        }
    }

    if(hasReversedFaces)
    {
        OSG_WARN << "Warning: [ReaderWriterOBJ::convertElementListToGeometry] Some faces from geometry '" << geometry->getName() << "' were reversed by the plugin" << std::endl;
    }

    return geometry;
}

osg::Node* ReaderWriterOBJ::convertModelToSceneGraph(obj::Model& model, ObjOptionsStruct& localOptions, const Options* options) const
{

    if (model.elementStateMap.empty()) return 0;

    osg::Group* group = new osg::Group;

    // set up the materials
    MaterialToStateSetMap materialToStateSetMap;
    buildMaterialToStateSetMap(model, materialToStateSetMap, localOptions, options);

    // go through the groups of related elements and build geometry from them.
    for(obj::Model::ElementStateMap::iterator itr=model.elementStateMap.begin();
        itr!=model.elementStateMap.end();
        ++itr)
    {

        const obj::ElementState& es = itr->first;
        obj::Model::ElementList& el = itr->second;

        osg::Geometry* geometry = convertElementListToGeometry(model,el,localOptions);

        if (geometry)
        {
            MaterialToStateSetMap::const_iterator it = materialToStateSetMap.find(es.materialName);
            if (it == materialToStateSetMap.end())
            {
                OSG_WARN << "Obj unable to find material '" << es.materialName << "'" << std::endl;
            }

            osg::StateSet* stateset = materialToStateSetMap[es.materialName].get();
            geometry->setStateSet(stateset);

            // tesseleate any large polygons
            if (!localOptions.noTesselateLargePolygons)
            {
                osgUtil::Tessellator tessellator;
                tessellator.retessellatePolygons(*geometry);
            }

            // tri strip polygons to improve graphics peformance
            if (!localOptions.noTriStripPolygons)
            {
                osgUtil::TriStripVisitor tsv;
                tsv.stripify(*geometry);
            }

            // if no normals present add them.
            if (localOptions.generateFacetNormals==false && (!geometry->getNormalArray() || geometry->getNormalArray()->getNumElements()==0))
            {
                osgUtil::SmoothingVisitor sv;
                sv.smooth(*geometry);
            }


            osg::Geode* geode = new osg::Geode;
            geode->addDrawable(geometry);

            if (es.objectName.empty())
            {
                geode->setName(es.groupName);
            }
            else if (es.groupName.empty())
            {
                geode->setName(es.objectName);
            }
            else
            {
                geode->setName(es.groupName + std::string(":") + es.objectName);
            }

            group->addChild(geode);

        }
    }

    return group;
}

ReaderWriterOBJ::ObjOptionsStruct ReaderWriterOBJ::parseOptions(const osgDB::ReaderWriter::Options* options) const
{
    ObjOptionsStruct localOptions;
    localOptions.rotate = true;
    localOptions.noTesselateLargePolygons = false;
    localOptions.noTriStripPolygons = false;
    localOptions.generateFacetNormals = false;
    localOptions.fixBlackMaterials = true;
    localOptions.noReverseFaces = false;

    if (options!=NULL)
    {
        std::istringstream iss(options->getOptionString());
        std::string opt;
        while (iss >> opt)
        {
            // split opt into pre= and post=
            std::string pre_equals;
            std::string post_equals;

            size_t found = opt.find("=");
            if(found!=std::string::npos)
            {
                pre_equals = opt.substr(0,found);
                post_equals = opt.substr(found+1);
            }
            else
            {
                pre_equals = opt;
            }

            if (pre_equals == "noRotation")
            {
                localOptions.rotate = false;
            }
            else if (pre_equals == "noTesselateLargePolygons")
            {
                localOptions.noTesselateLargePolygons = true;
            }
            else if (pre_equals == "noTriStripPolygons")
            {
                localOptions.noTriStripPolygons = true;
            }
            else if (pre_equals == "generateFacetNormals")
            {
                localOptions.generateFacetNormals = true;
            }
            else if (pre_equals == "noReverseFaces")
            {
                localOptions.noReverseFaces = true;
            }
            else if (post_equals.length()>0)
            {
                obj::Material::Map::TextureMapType type = obj::Material::Map::UNKNOWN;
                // Now we check to see if we have anything forcing a texture allocation
                if        (pre_equals == "DIFFUSE")            type = obj::Material::Map::DIFFUSE;
                else if (pre_equals == "AMBIENT")            type = obj::Material::Map::AMBIENT;
                else if (pre_equals == "SPECULAR")            type = obj::Material::Map::SPECULAR;
                else if (pre_equals == "SPECULAR_EXPONENT") type = obj::Material::Map::SPECULAR_EXPONENT;
                else if (pre_equals == "OPACITY")            type = obj::Material::Map::OPACITY;
                else if (pre_equals == "BUMP")                type = obj::Material::Map::BUMP;
                else if (pre_equals == "DISPLACEMENT")        type = obj::Material::Map::DISPLACEMENT;
                else if (pre_equals == "REFLECTION")        type = obj::Material::Map::REFLECTION;

                if (type!=obj::Material::Map::UNKNOWN)
                {
                    int unit = atoi(post_equals.c_str());    // (probably should use istringstream rather than atoi)
                    localOptions.textureUnitAllocation.push_back(std::make_pair(unit,(obj::Material::Map::TextureMapType) type));
                    OSG_NOTICE<<"Obj Found map in options, ["<<pre_equals<<"]="<<unit<<std::endl;
                }
            }
        }
    }
    return localOptions;
}


// read file and convert to OSG.
osgDB::ReaderWriter::ReadResult ReaderWriterOBJ::readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
{
    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    std::string fileName = osgDB::findDataFile( file, options );
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;


    osgDB::ifstream fin(fileName.c_str());
    if (fin)
    {

        // code for setting up the database path so that internally referenced file are searched for on relative paths.
        osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
        local_opt->getDatabasePathList().push_front(osgDB::getFilePath(fileName));

        obj::Model model;
        model.setDatabasePath(osgDB::getFilePath(fileName.c_str()));
        model.readOBJ(fin, local_opt.get());

        ObjOptionsStruct localOptions = parseOptions(options);

        osg::Node* node = convertModelToSceneGraph(model, localOptions, local_opt.get());
        return node;
    }

    return ReadResult::FILE_NOT_HANDLED;
}

osgDB::ReaderWriter::ReadResult ReaderWriterOBJ::readNode(std::istream& fin, const Options* options) const
{
    if (fin)
    {
        fin.imbue(std::locale::classic());

        obj::Model model;
        model.readOBJ(fin, options);

        ObjOptionsStruct localOptions = parseOptions(options);

        osg::Node* node = convertModelToSceneGraph(model, localOptions, options);
        return node;
    }

    return ReadResult::FILE_NOT_HANDLED;
}
