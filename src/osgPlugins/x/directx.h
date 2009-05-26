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

#include <osg/Export>
#include <osg/Math>

#include "types.h"
#include "mesh.h"

namespace DX {

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
         * Load model from stream.
         * Discards old data.
         * @param filename Filename.
         * @return false if the model could not be loaded, else true.
         */
        bool load(std::istream& fin);

        /**
         * Generate per-vertex normals for the entire model.
         * Discards any previously loaded or generated normals.
         * @param creaseAngle TODO: The angle above which two adjacent faces are no
         * longer considered to belong to a common surface.
         * @return false if an error occurred, else true.
         */
        bool generateNormals(float creaseAngle = 80.0f);

        /// Get number of meshes.
        inline unsigned int getNumMeshes() const {
            return _meshes.size();
        }

        /// Get Mesh.
        inline Mesh* getMesh(unsigned int i) {
            return _meshes[i];
        }
        inline const Mesh* getMesh(unsigned int i) const {
            return _meshes[i];
        }

        /// Find global material.
        Material * findMaterial(const std::string & name);

        /// Parse section until '}'; recurse as needed.
        void parseSection(std::istream& fin);

    private:
        // dgm - keep list of materials global to the file
        std::vector<Material> _globalMaterials;

        /// Meshes.
        std::vector<Mesh*> _meshes;

        /// Clear object.
        void clear();
    };
} // namespace

#endif
