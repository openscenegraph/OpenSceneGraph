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

#ifndef _DX_TYPES_H_
#define _DX_TYPES_H_

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
        // dgm - materials can have names for later reference
        std::string name;
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

    /// Tokenize a string.
    extern void tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters = " \t\r\n;,");

    /// Parse 'Material'.
    extern void parseMaterial(std::istream& fin, Material& material);

    /// Read 'TextureFilename'.
    extern void readTexFilename(std::istream& fin, TextureFilename& texture);

    /// Read 'Coords2d'.
    extern void readCoords2d(std::istream& fin, std::vector<Coords2d>& v, unsigned int count);

    // Read 'Vector'
    extern void readVector(std::istream& fin, std::vector<Vector>& v, unsigned int count);

    /// Read index list.
    extern void readIndexList(std::istream& fin, std::vector<unsigned int>& v, unsigned int count);

    /// Read 'MeshFace'.
    extern void readMeshFace(std::istream& fin, std::vector<MeshFace>& v, unsigned int count);

} // namespace

#endif
