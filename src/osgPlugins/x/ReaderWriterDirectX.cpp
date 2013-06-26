// -*-c++-*-

/*
 * $Id$
 *
 * DirectX file converter for OpenSceneGraph.
 * Copyright (c)2002 Ulrich Hertlein <u.hertlein@sandbox.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#include "directx.h"

#include <osg/TexEnv>
#include <osg/CullFace>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/Image>
#include <osg/Texture2D>

#include <osg/Notify>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <map>
#include <iostream>


/**
 * OpenSceneGraph plugin wrapper/converter.
 */
class ReaderWriterDirectX : public osgDB::ReaderWriter
{
public:
    ReaderWriterDirectX()
    {
        supportsExtension("x","DirectX scene format");
        supportsOption("flipTexture", "flip texture upside-down");
        // made hand switching an option - .x models from XSI's export are right-handed already
        supportsOption("rightHanded", "prevents reader from switching handedness for right handed files");
        supportsOption("leftHanded", "reader switches handedness for left handed files");
    }

    virtual const char* className() const {
        return "DirectX Reader";
    }

    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const;
    virtual ReadResult readNode(std::istream& fin, const osgDB::ReaderWriter::Options* options) const;

private:
    osg::Group * convertFromDX(DX::Object & obj, bool switchToLeftHanded, bool flipTexture, float creaseAngle,
            const osgDB::ReaderWriter::Options * options) const;

    osg::Geode * convertFromDX(DX::Mesh & mesh, bool switchToLeftHanded, bool flipTexture, float creaseAngle,
            const osgDB::ReaderWriter::Options * options) const;
};

// Register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN(x, ReaderWriterDirectX)


// Read node
osgDB::ReaderWriter::ReadResult ReaderWriterDirectX::readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
{
    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    std::string fileName = osgDB::findDataFile( file, options );
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

    OSG_INFO << "ReaderWriterDirectX::readNode(" << fileName << ")\n";

    osgDB::ifstream fin(fileName.c_str());
    if (fin.bad()) {
        OSG_WARN << "ReaderWriterDirectX failed to read '" << fileName.c_str() << "'\n";
        return ReadResult::ERROR_IN_READING_FILE;
    }

    // code for setting up the database path so that internally referenced file are searched for on relative paths.
    osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
    local_opt->setDatabasePath(osgDB::getFilePath(fileName));

    return readNode(fin, local_opt.get());
}

osgDB::ReaderWriter::ReadResult ReaderWriterDirectX::readNode(std::istream& fin, const osgDB::ReaderWriter::Options* options) const
{
    DX::Object obj;
    if (obj.load(fin) == false) {
        OSG_WARN << "ReaderWriterDirectX failed to read stream" << std::endl;
        return ReadResult::ERROR_IN_READING_FILE;
    }

    // Options?
    bool flipTexture = true;
    bool switchToLeftHanded = true; // when true: swap y and z for incoming files
    float creaseAngle = 80.0f;
    if (options) {
        const std::string option = options->getOptionString();
        if (option.find("rightHanded") != std::string::npos) {
            switchToLeftHanded = false;
        }
        if (option.find("leftHanded") != std::string::npos) {
            switchToLeftHanded = true;
        }
        if (option.find("flipTexture") != std::string::npos) {
            flipTexture = false;
        }
        if (option.find("creaseAngle") != std::string::npos) {
            // TODO
        }
    }

    // Convert to osg::Group
    osg::Group* group = convertFromDX(obj, switchToLeftHanded, flipTexture, creaseAngle, options);
    if (!group) {
        OSG_WARN << "ReaderWriterDirectX failed to convert\n";
        return ReadResult::ERROR_IN_READING_FILE;
    }

    return group;
}


// Convert DirectX object
osg::Group * ReaderWriterDirectX::convertFromDX(DX::Object & obj, bool switchToLeftHanded,
                                                bool flipTexture, float creaseAngle,
                                                const osgDB::ReaderWriter::Options * options) const
{
    osg::ref_ptr<osg::Group> group = new osg::Group;

    for (unsigned int i = 0; i < obj.getNumMeshes(); ++i) {
        //std::cerr << "converting mesh " << i << std::endl;
        DX::Mesh & mesh = *obj.getMesh(i);
        osg::Geode * geode = convertFromDX(mesh, switchToLeftHanded, flipTexture, creaseAngle, options);
        if (!geode) {
            return 0;
        }
        group->addChild(geode);
    }

    return group.release();
}

// Convert DirectX mesh to osg::Geode
osg::Geode* ReaderWriterDirectX::convertFromDX(DX::Mesh & mesh, bool switchToLeftHanded,
                                               bool flipTexture, float creaseAngle,
                                               const osgDB::ReaderWriter::Options * options) const
{
    const DX::MeshMaterialList* meshMaterial = mesh.getMeshMaterialList();
    if (!meshMaterial)
        return NULL;

    const DX::MeshNormals* meshNormals = mesh.getMeshNormals();
    if (!meshNormals) {
        mesh.generateNormals(creaseAngle);
        meshNormals = mesh.getMeshNormals();
    }
    //std::cerr << "normals=" << meshNormals << std::endl;
    if (!meshNormals)
        return NULL;

    const DX::MeshTextureCoords* meshTexCoords = mesh.getMeshTextureCoords();
    //std::cerr << "texcoord=" << meshTexCoords << std::endl;

    /*
     * - MeshMaterialList contains a list of Material and a per-face
     *   information with Material is to be applied to which face.
     * - Mesh contains a list of Vertices and a per-face information
     *   which vertices (three or four) belong to this face.
     * - MeshNormals contains a list of Normals and a per-face information
     *   which normal is used by which vertex.
     * - MeshTextureCoords contains a list of per-vertex texture coordinates.
     *
     * - Uses left-hand CS with Y-up, Z-into
     *   obj_x -> osg_x
     *   obj_y -> osg_z
     *   obj_z -> osg_y
     *
     * - aa: Changed always change left to right hand to an option that allows
     *   us to read right-handed models as-is.  Our modeler is using XSI, which
     *   exports to right-handed system.
     *
     * - Polys are CW oriented
     */
    std::vector<osg::Geometry*> geomList;

    // Texture-for-Image map
    std::map<std::string, osg::Texture2D*> texForImage;

    unsigned int i;
    for (i = 0; i < meshMaterial->material.size(); i++) {

        //std::cerr << "material " << i << std::endl;

        const DX::Material& mtl = meshMaterial->material[i];
        osg::StateSet* state = new osg::StateSet;

        // Material
        osg::Material* material = new osg::Material;
        state->setAttributeAndModes(material);

        float alpha = mtl.faceColor.alpha;
        osg::Vec4 ambient(mtl.faceColor.red,
                          mtl.faceColor.green,
                          mtl.faceColor.blue,
                          alpha);
        material->setAmbient(osg::Material::FRONT, ambient);
        material->setDiffuse(osg::Material::FRONT, ambient);

        material->setShininess(osg::Material::FRONT, mtl.power);

        osg::Vec4 specular(mtl.specularColor.red,
                           mtl.specularColor.green,
                           mtl.specularColor.blue, alpha);
        material->setSpecular(osg::Material::FRONT, specular);

        osg::Vec4 emissive(mtl.emissiveColor.red,
                           mtl.emissiveColor.green,
                           mtl.emissiveColor.blue, alpha);
        material->setEmission(osg::Material::FRONT, emissive);

        // Transparency? Set render hint & blending
        if (alpha < 1.0f) {
            state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
            state->setMode(GL_BLEND, osg::StateAttribute::ON);
        }
        else
            state->setMode(GL_BLEND, osg::StateAttribute::OFF);

        unsigned int textureCount = mtl.texture.size();
        for (unsigned int j = 0; j < textureCount; j++) {

            //std::cerr << "texture " << j << std::endl;

            // Share image/texture pairs
            osg::Texture2D* texture = texForImage[mtl.texture[j]];
            if (!texture) {
                osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile(mtl.texture[j],options);
                if (!image)
                    continue;

                // Texture
                texture = new osg::Texture2D;
                texForImage[mtl.texture[j]] = texture;

                texture->setImage(image.get());
                texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
                texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
            }
            state->setTextureAttributeAndModes(j, texture);
        }

        // Geometry
        osg::Geometry* geom = new osg::Geometry;
        geomList.push_back(geom);

        geom->setStateSet(state);

        // Arrays to hold vertices, normals, and texcoords.
        geom->setVertexArray(new osg::Vec3Array);
        geom->setNormalArray(new osg::Vec3Array, osg::Array::BIND_PER_VERTEX);
        if (textureCount) {
            // All texture units share the same array
            osg::Vec2Array* texCoords = new osg::Vec2Array;
            for (unsigned int j = 0; j < textureCount; j++)
                geom->setTexCoordArray(j, texCoords);
        }

        geom->addPrimitiveSet(new osg::DrawArrayLengths(osg::PrimitiveSet::POLYGON));
    }

    const std::vector<DX::MeshFace> & faces = mesh.getFaces();
    if (faces.size() != meshMaterial->faceIndices.size())
    {
        OSG_FATAL<<"Error: internal error in DirectX .x loader,"<<std::endl;
        OSG_FATAL<<"       mesh->faces.size() == meshMaterial->faceIndices.size()"<<std::endl;
        return NULL;
    }

    // Add faces to Geometry
    for (i = 0; i < meshMaterial->faceIndices.size(); i++) {

        // Geometry for Material
        unsigned int mi = meshMaterial->faceIndices[i];
        osg::Geometry* geom = geomList[mi];

        // #pts of this face
        unsigned int np = faces[i].size();
        ((osg::DrawArrayLengths*) geom->getPrimitiveSet(0))->push_back(np);

        if (np != meshNormals->faceNormals[i].size())
        {
            OSG_WARN<<"DirectX loader: Error, error in normal list."<<std::endl;
        }

        osg::Vec3Array* vertexArray = (osg::Vec3Array*) geom->getVertexArray();
        osg::Vec3Array* normalArray = (osg::Vec3Array*) geom->getNormalArray();
        osg::Vec2Array* texCoordArray=NULL; // only make them if the original has them
        if(meshTexCoords) texCoordArray = (osg::Vec2Array*) geom->getTexCoordArray(0);

        // Add vertices, normals, texcoords
        for (unsigned int j = 0; j < np; j++) {

            // Convert CW to CCW order
            unsigned int jj = (j > 0 ? np - j : j);
            if(!switchToLeftHanded) jj=j;

            // Vertices
            unsigned int vi = faces[i][jj];
            if (vertexArray) {
                const DX::Vector & v = mesh.getVertices()[vi];
                if(switchToLeftHanded)// Transform Xleft/Yup/Zinto to Xleft/Yinto/Zup
                    vertexArray->push_back(osg::Vec3(v.x,v.z,v.y));
                else
                    vertexArray->push_back(osg::Vec3(v.x,v.y,v.z));
            }

            // Normals
            unsigned int ni = meshNormals->faceNormals[i][jj];
            if (normalArray) {
                const DX::Vector& n = meshNormals->normals[ni];
                if(switchToLeftHanded)// Transform Xleft/Yup/Zinto to Xleft/Yinto/Zup
                    normalArray->push_back(osg::Vec3(n.x,n.z,n.y));
                else
                    normalArray->push_back(osg::Vec3(n.x,n.y,n.z));
            }

            // TexCoords
            if (texCoordArray) {
                const DX::Coords2d& tc = (*meshTexCoords)[vi];
                osg::Vec2 uv;
                if (flipTexture){
                    if(switchToLeftHanded)
                        uv.set(tc.u, 1.0f - tc.v); // Image is upside down
                    else
                        uv.set(1.0f - tc.u, 1.0f - tc.v); // Image is 180 degrees
                }
                else
                    uv.set(tc.u, tc.v);
                texCoordArray->push_back(uv);
            }
        }
    }

    // Add non-empty nodes to Geode
    osg::Geode* geode = new osg::Geode;
    for (i = 0; i < geomList.size(); i++) {
        osg::Geometry* geom = geomList[i];
        if (((osg::Vec3Array*) geom->getVertexArray())->size())
            geode->addDrawable(geom);
    }

    // Back-face culling
    osg::StateSet* state = new osg::StateSet;
    geode->setStateSet(state);

    osg::CullFace* cullFace = new osg::CullFace;
    cullFace->setMode(osg::CullFace::BACK);
    state->setAttributeAndModes(cullFace);

    return geode;
}
