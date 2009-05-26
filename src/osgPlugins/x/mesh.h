// -*-c++-*-

/*
 * $Id$
 *
 * Loader for DirectX .x files.
 * Copyright (c)2002-2006 Ulrich Hertlein <u.hertlein@sandbox.de>
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

#ifndef _MESH_H_
#define _MESH_H_

#include "types.h"

namespace DX {

    class Object;

    /**
     * DirectX mesh.
     */
    class Mesh {
    public:
        Mesh(Object * obj);
        virtual ~Mesh() {
            clear();
        }

        void clear();

        /**
         * Generate per-vertex normals for the mesh.
         * @return false if an error occurred, else true.
         */
        bool generateNormals(float creaseAngle);

        /// Get Vertices.
        const std::vector<Vector> & getVertices() const {
            return _vertices;
        }

        /// Get MeshFaces.
        const std::vector<MeshFace> & getFaces() const {
            return _faces;
        }

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

        /// Parse 'Mesh'.
        void parseMesh(std::istream& fin);

    private:
        Object * _obj;

        std::vector<Vector> _vertices;
        std::vector<MeshFace> _faces;

        /// Normals (per-face).
        MeshNormals* _normals;

        /// Texture coordinates (per-vertex).
        MeshTextureCoords* _textureCoords;

        /// Materials.
        MeshMaterialList* _materialList;

        /// Parse 'MeshNormals'.
        void parseMeshNormals(std::istream& fin);

        /// Parse 'MeshMaterialList'.
        void parseMeshMaterialList(std::istream& fin);

        /// Read 'MeshTextureCoords'.
        void readMeshTexCoords(std::istream& fin);
    };

} // namespace

#endif
