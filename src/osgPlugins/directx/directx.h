// -*-c++-*-

/*
 * $Id$
 *
 * Loader for DirectX .x files.
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

#ifndef _DIRECTX_H_
#define _DIRECTX_H_

#include <string>
#include <vector>
#include <fstream>

#include <osg/Math>

namespace DX {

    /*
     * DirectX templates
     * http://astronomy.swin.edu.au/~pbourke/geomformats/directx/
     */

    // Vector
    typedef struct {
        float x,y,z;

        inline void normalize() {
            float lenRecip = 1.0f / sqrtf(x * x + y * y + z * z);
            x *= lenRecip;
            y *= lenRecip;
            z *= lenRecip;
        }
    } Vector;

    // Coords2d
    typedef struct {
        float u,v;
    } Coords2d;

    // ColorRGBA
    typedef struct {
        float red,green,blue,alpha;
    } ColorRGBA;

    // ColorRGB
    typedef struct {
        float red,green,blue;
    } ColorRGB;

    // IndexedColor
    typedef struct {
        unsigned int index;
        ColorRGBA indexColor;
    } IndexedColor;

    // TextureFilename
    typedef std::string TextureFilename;

    // Material (potentially with multiple textures)
    typedef struct {
        ColorRGBA faceColor;
        float power;
        ColorRGB specularColor;
        ColorRGB emissiveColor;
        std::vector<TextureFilename> texture;
    } Material;

    // MeshFace
    typedef std::vector<unsigned int> MeshFace;

    // MeshTextureCoords
    typedef std::vector<Coords2d> MeshTextureCoords;

    // MeshNormals
    typedef struct {
        std::vector<Vector> normals;
        std::vector<MeshFace> faceNormals;
    } MeshNormals;

    // MeshVertexColors.
    typedef std::vector<IndexedColor> MeshVertexColors;

    // MeshMaterialList
    typedef struct {
        std::vector<unsigned int> faceIndices;
        std::vector<Material> material;
    } MeshMaterialList;

    // Mesh
    typedef struct {
        std::vector<Vector> vertices;
        std::vector<MeshFace> faces;
    } Mesh;

    /**
     * DirectX object.
     */
    class Object {
    public:
        /// Constructor.
        Object();

        /// Destructor.
        virtual ~Object() {
            clear();
        }

        /**
         * Load model from file
         * Discards old data.
         * @param filename Filename.
         * @return false if the model could not be loaded, else true.
         */
        bool load(const char* filename);

        /**
         * Generate per-vertex normals for the entire model.
         * Discards any previously loaded or generated normals.
         * @param creaseAngle TODO: The angle above which two adjacent faces are no
         * longer considered to belong to a common surface.
         * @return false if an error occurred, else true.
         */
        bool generateNormals(float creaseAngle = 80.0f);

        /// Get MeshTextureCoords.
        inline const MeshTextureCoords* getMeshTextureCoords() const {
            return _textureCoords;
        }

        /// Get MeshMaterialList.
        inline const MeshMaterialList* getMeshMaterialList() const {
            return _materialList;
        }

        /// Get MeshNormals.
        inline const MeshNormals* getMeshNormals() const {
            return _normals;
        }

        /// Get Mesh.
        inline const Mesh* getMesh() const {
            return _mesh;
        }

    private:
        /// Texture coordinates (per-vertex).
        MeshTextureCoords* _textureCoords;

        /// Material list (per-face).
        MeshMaterialList* _materialList;

        /// Normals (per-face).
        MeshNormals* _normals;

        /// Mesh.
        Mesh* _mesh;

        /// Clear object.
        void clear();

        /// Read 'TextureFilename'.
        void readTexFilename(std::ifstream& fin, TextureFilename& texture);

        /// Parse 'Material'.
        void parseMaterial(std::ifstream& fin, Material& material);
        
        /// Read 'Coords2d'.
        void readCoords2d(std::ifstream& fin, std::vector<Coords2d>& v, unsigned int count);

        /// Read 'MeshTextureCoords'.
        void readMeshTexCoords(std::ifstream& fin);

        /// Read index list.
        void readIndexList(std::ifstream& fin, std::vector<unsigned int>& v, unsigned int count);

        /// Parse 'MeshMaterialList'.
        void parseMeshMaterialList(std::ifstream& fin);

        /// Read 'Vector'.
        void readVector(std::ifstream& fin, std::vector<Vector>& v, unsigned int count);

        /// Read 'MeshFace'.
        void readMeshFace(std::ifstream& fin, std::vector<MeshFace>& v, unsigned int count);

        /// Parse 'MeshNormals'.
        void parseMeshNormals(std::ifstream& fin);

        /// Parse 'Mesh'.
        void parseMesh(std::ifstream& fin);

        /// Parse section until '}'; recurse as needed.
        void parseSection(std::ifstream& fin);
    };
} // namespace

#endif
