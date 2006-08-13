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
#include <osg/TexMat>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <osgUtil/TriStripVisitor>
#include <osgUtil/SmoothingVisitor>
#include <osgUtil/Tesselator>

#include "obj.h"

#include <map>
#include <set>

class ReaderWriterOBJ : public osgDB::ReaderWriter
{
public:
    ReaderWriterOBJ():_fixBlackMaterials(true) {}

    virtual const char* className() const { return "Wavefront OBJ Reader"; }
    virtual bool acceptsExtension(const std::string& extension) const {
        return osgDB::equalCaseInsensitive(extension,"obj");
    }

    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const;

    virtual ReadResult readNode(std::istream& fin, const Options* options) const;


protected:

    typedef std::map< std::string, osg::ref_ptr<osg::StateSet> > MaterialToStateSetMap;
    
    void buildMaterialToStateSetMap(obj::Model& model, MaterialToStateSetMap& materialToSetSetMap) const;
    
    osg::Geometry* convertElementListToGeometry(obj::Model& model, obj::Model::ElementList& elementList, bool& rotate) const;
    
    osg::Node* convertModelToSceneGraph(obj::Model& model, bool& rotate) const;

    inline osg::Vec3 transformVertex(const osg::Vec3& vec, const bool rotate) const ;
    inline osg::Vec3 transformNormal(const osg::Vec3& vec, const bool rotate) const ;
    
    bool _fixBlackMaterials;

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
osgDB::RegisterReaderWriterProxy<ReaderWriterOBJ> g_objReaderWriterProxy;

void ReaderWriterOBJ::buildMaterialToStateSetMap(obj::Model& model, MaterialToStateSetMap& materialToStateSetMap) const
{
    osg::Texture::WrapMode textureWrapMode = osg::Texture::REPEAT;

    if (_fixBlackMaterials)
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
        
        osg::StateSet* stateset = new osg::StateSet;

        bool isTransparent = false;

        // handle material colors
        {
            osg::Material* osg_material = new osg::Material;
            stateset->setAttribute(osg_material);

            osg_material->setAmbient(osg::Material::FRONT_AND_BACK,material.ambient);
            osg_material->setDiffuse(osg::Material::FRONT_AND_BACK,material.diffuse);
            osg_material->setSpecular(osg::Material::FRONT_AND_BACK,material.specular);
            osg_material->setShininess(osg::Material::FRONT_AND_BACK,(material.shininess/1000.0f)*128.0f ); // note OBJ shiniess is 0..1000.
            
            if (material.ambient[3]!=1.0 ||
                material.diffuse[3]!=1.0 ||
                material.specular[3]!=1.0)
            {
                osg::notify(osg::INFO)<<"Found transparent material"<<std::endl;
                isTransparent = true;
            }
        }
        
        // handle textures
        if (!material.map_Kd.empty())
        {
            std::string filename = material.map_Kd;
            osg::Image* image = 0;
            if (!model.getDatabasePath().empty()) 
            {
                // first try with databasr path of parent. 
                image = osgDB::readImageFile(model.getDatabasePath()+'/'+filename);
            }
            
            if (!image)
            {
                // if not already set then try the filename as is.
                image = osgDB::readImageFile(filename);
            }
            if (image)
            {
                osg::Texture2D* texture = new osg::Texture2D(image);
                texture->setWrap(osg::Texture2D::WRAP_R, textureWrapMode);
                texture->setWrap(osg::Texture2D::WRAP_S, textureWrapMode);
                texture->setWrap(osg::Texture2D::WRAP_T, textureWrapMode);
                stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
                
                if (material.textureReflection)
                {
                    osg::TexGen* texgen = new osg::TexGen;
                    texgen->setMode(osg::TexGen::SPHERE_MAP);
                    stateset->setTextureAttributeAndModes(0,texgen,osg::StateAttribute::ON);
                }
                
                if  (!isTransparent && image->isImageTranslucent())
                {
                    osg::notify(osg::INFO)<<"Found transparent image"<<std::endl;
                    isTransparent = true;
                }
            }
        }

        if (isTransparent)
        {
            stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
            stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        }
        
        materialToStateSetMap[material.name] = stateset;
        
    }
}

osg::Geometry* ReaderWriterOBJ::convertElementListToGeometry(obj::Model& model, obj::Model::ElementList& elementList, bool& rotate) const
{
    
    unsigned int numVertexIndices = 0;
    unsigned int numNormalIndices = 0;
    unsigned int numTexCoordIndices = 0;
    
    unsigned int numPointElements = 0;
    unsigned int numPolylineElements = 0;
    unsigned int numPolygonElements = 0;

    obj::Model::ElementList::iterator itr;
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
        osg::notify(osg::NOTICE)<<"Incorrect number of normals, ignore them"<<std::endl;
        numNormalIndices = 0;
    }
    
    if (numTexCoordIndices!=0 && numTexCoordIndices!=numVertexIndices)
    {
        osg::notify(osg::NOTICE)<<"Incorrect number of normals, ignore them"<<std::endl;
        numTexCoordIndices = 0;
    }
    
    osg::Vec3Array* vertices = numVertexIndices ? new osg::Vec3Array : 0;
    osg::Vec3Array* normals = numNormalIndices ? new osg::Vec3Array : 0;
    osg::Vec2Array* texcoords = numTexCoordIndices ? new osg::Vec2Array : 0;
    
    if (vertices) vertices->reserve(numVertexIndices);
    if (normals) normals->reserve(numNormalIndices);
    if (texcoords) texcoords->reserve(numTexCoordIndices);
    
    osg::Geometry* geometry = new osg::Geometry;
    if (vertices) geometry->setVertexArray(vertices);
    if (normals)
    {
        geometry->setNormalArray(normals);
        geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    }
    if (texcoords)
    {
        geometry->setTexCoordArray(0,texcoords);
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
                    vertices->push_back(transformVertex(model.vertices[*index_itr],rotate));
                    ++numPoints;
                }
                if (numNormalIndices)
                {
                    for(obj::Element::IndexList::iterator index_itr = element.normalIndices.begin();
                        index_itr != element.normalIndices.end();
                        ++index_itr)
                    {
                        normals->push_back(transformNormal(model.normals[*index_itr],rotate));
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
                    vertices->push_back(transformVertex(model.vertices[*index_itr],rotate));
                }
                if (numNormalIndices)
                {
                    for(obj::Element::IndexList::iterator index_itr = element.normalIndices.begin();
                        index_itr != element.normalIndices.end();
                        ++index_itr)
                    {
                        normals->push_back(transformNormal(model.normals[*index_itr],rotate));
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

            
                if (model.needReverse(element))
                {
                    // need to reverse so add to OSG arrays in same order as in OBJ, as OSG assume anticlockwise ordering.
                    for(obj::Element::IndexList::reverse_iterator index_itr = element.vertexIndices.rbegin();
                        index_itr != element.vertexIndices.rend();
                        ++index_itr)
                    {
                        vertices->push_back(transformVertex(model.vertices[*index_itr],rotate));
                    }
                    if (numNormalIndices)
                    {
                        for(obj::Element::IndexList::reverse_iterator index_itr = element.normalIndices.rbegin();
                            index_itr != element.normalIndices.rend();
                            ++index_itr)
                        {
                            normals->push_back(transformNormal(model.normals[*index_itr],rotate));
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
                        vertices->push_back(transformVertex(model.vertices[*index_itr],rotate));
                    }
                    if (numNormalIndices)
                    {
                        for(obj::Element::IndexList::iterator index_itr = element.normalIndices.begin();
                            index_itr != element.normalIndices.end();
                            ++index_itr)
                        {
                            normals->push_back(transformNormal(model.normals[*index_itr],rotate));
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
    
    return geometry;
}

osg::Node* ReaderWriterOBJ::convertModelToSceneGraph(obj::Model& model, bool& rotate) const
{

    if (model.elementStateMap.empty()) return 0;

    osg::Group* group = new osg::Group;

    // set up the materials
    MaterialToStateSetMap materialToSetSetMap;
    buildMaterialToStateSetMap(model, materialToSetSetMap);

    // go through the groups of related elements and build geometry from them.
    for(obj::Model::ElementStateMap::iterator itr=model.elementStateMap.begin();
        itr!=model.elementStateMap.end();
        ++itr)
    {
    
        const obj::ElementState& es = itr->first;
        obj::Model::ElementList& el = itr->second;

        osg::Geometry* geometry = convertElementListToGeometry(model,el,rotate);

        if (geometry)
        {

            osg::StateSet* stateset = materialToSetSetMap[es.materialName].get();
            geometry->setStateSet(stateset);
        
            // tesseleate any large polygons
            osgUtil::Tesselator tesselator;
            tesselator.retesselatePolygons(*geometry);

            // tri strip polygons to improve graphics peformance
            osgUtil::TriStripVisitor tsv;
            tsv.stripify(*geometry);

            // if no normals present add them.
            if (!geometry->getNormalArray() || geometry->getNormalArray()->getNumElements()==0)
            {
                osgUtil::SmoothingVisitor tsv;
                tsv.smooth(*geometry);
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


// read file and convert to OSG.
osgDB::ReaderWriter::ReadResult ReaderWriterOBJ::readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
{
    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    std::string fileName = osgDB::findDataFile( file, options );
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;
    
    
    std::ifstream fin(fileName.c_str());
    if (fin)
    {
    
        // code for setting up the database path so that internally referenced file are searched for on relative paths. 
        osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
        local_opt->setDatabasePath(osgDB::getFilePath(fileName));

        obj::Model model;
        model.setDatabasePath(osgDB::getFilePath(fileName.c_str()));
        model.readOBJ(fin, local_opt.get());
        
        // code for checking the nonRotation
        bool rotate = true;
        if ((options!=NULL) && (options->getOptionString() == "noRotation"))
        {
            rotate = false;
        }

        osg::Node* node = convertModelToSceneGraph(model,rotate);
        return node;
    }
    
    return ReadResult::FILE_NOT_HANDLED;
}

osgDB::ReaderWriter::ReadResult ReaderWriterOBJ::readNode(std::istream& fin, const Options* options) const
{
    if (fin)
    {
        obj::Model model;
        model.readOBJ(fin, options);
        
        // code for checking the nonRotation
        bool rotate = true;
        if ((options!=NULL) && (options->getOptionString() == "noRotation"))
        {
            rotate = false;
        }

        osg::Node* node = convertModelToSceneGraph(model,rotate);
        return node;
    }
    
    return ReadResult::FILE_NOT_HANDLED;
}
