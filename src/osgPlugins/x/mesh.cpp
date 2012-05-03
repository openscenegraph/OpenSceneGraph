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

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
#pragma warning (disable : 4786)
#endif

#include "mesh.h"
#include "directx.h"

#include <iostream>
#include <string.h>
#include <stdlib.h>

#include <osg/Notify>

using namespace DX;
using namespace std;


/**********************************************************************
 *
 * DirectX mesh.
 *
 **********************************************************************/

Mesh::Mesh(Object * obj)
{
    _obj = obj;
    _normals = 0;
    _textureCoords = 0;
    _materialList = 0;
}

void Mesh::clear()
{
    if (_normals) {
        delete _normals;
        _normals = 0;
    }

    if (_textureCoords) {
        delete _textureCoords;
        _textureCoords = 0;
    }

    if (_materialList) {
        delete _materialList;
        _materialList = 0;
    }
}

bool Mesh::generateNormals(float /*creaseAngle*/)
{
    //cerr << "*** generateNormals\n";

    // Forget old normals
    if (_normals) {
        delete _normals;
        _normals = 0;
    }

    /*
     * Calculate per-face normals from face vertices.
     */
    vector<Vector> faceNormals;
    faceNormals.resize(_faces.size());

    unsigned int fi;
    for (fi = 0; fi < _faces.size(); fi++) {

        vector<Vector> poly;
        unsigned int n = _faces[fi].size();

        if (n < 3)
            continue;
        for (unsigned int i = 0; i < n; i++) {
            unsigned int idx = _faces[fi][i];
            poly.push_back(_vertices[idx]);
        }

        // Edge vectors
        Vector e0;
        e0.x = poly[1].x - poly[0].x;
        e0.y = poly[1].y - poly[0].y;
        e0.z = poly[1].z - poly[0].z;

        Vector e1;
        e1.x = poly[2].x - poly[0].x;
        e1.y = poly[2].y - poly[0].y;
        e1.z = poly[2].z - poly[0].z;

        // Cross-product of e0,e1
        Vector normal;
        normal.x = e0.y * e1.z - e0.z * e1.y;
        normal.y = e0.z * e1.x - e0.x * e1.z;
        normal.z = e0.x * e1.y - e0.y * e1.x;
        normal.normalize();

        // Add to per-face normals
        faceNormals[fi] = normal;
    }

    /*
     * Calculate per-vertex normals as average of all per-face normals that
     * share this vertex. The index of the vertex normal is identical to the
     * vertex index for now. This means each vertex only has a single normal...
     */
    _normals = new MeshNormals;
    _normals->normals.resize(_vertices.size());

    for (unsigned int vi = 0; vi < _vertices.size(); vi++) {

        Vector normal = { 0.0f, 0.0f, 0.0f };
        unsigned int polyCount = 0;

        // Collect normals of polygons that share this vertex
        for (unsigned int fi = 0; fi < _faces.size(); fi++)
            for (unsigned int i = 0; i < _faces[fi].size(); i++) {
                unsigned int idx = _faces[fi][i];
                if (idx == vi) {
                    normal.x += faceNormals[fi].x;
                    normal.y += faceNormals[fi].y;
                    normal.z += faceNormals[fi].z;
                    polyCount++;
                }
            }

        //OSG_INFO << "vertex " << vi << " used by " << polyCount << " faces\n";
        if (polyCount > 1) {
            float polyCountRecip = 1.0f / (float) polyCount;
            normal.x *= polyCountRecip;
            normal.y *= polyCountRecip;
            normal.z *= polyCountRecip;
            normal.normalize();
        }

        // Add vertex normal
        _normals->normals[vi] = normal;
    }

    // Copy face mesh to normals mesh
    _normals->faceNormals.resize(_faces.size());
    for (fi = 0; fi < _faces.size(); fi++)
        _normals->faceNormals[fi] = _faces[fi];

    return true;
}

// Parse 'Mesh'
void Mesh::parseMesh(std::istream& fin)
{
    char buf[256];
    vector<string> token;

    unsigned int nVertices = 0, nFaces = 0;

    //cerr << "*** Mesh\n";
    while (fin.getline(buf, sizeof(buf))) {

        // Tokenize
        token.clear();
        tokenize(buf, token);
        if (token.size() == 0)
            continue;

        //cerr << "*** Mesh token=" << token[0] << endl;
        if (strrchr(buf, '}') != 0) {
            break;
        }
        else if (strrchr(buf, '{') != 0) {
            if (token[0] == "MeshMaterialList")
                parseMeshMaterialList(fin);
            else if (token[0] == "MeshNormals")
                parseMeshNormals(fin);
            else if (token[0] == "MeshTextureCoords")
                readMeshTexCoords(fin);
            else {
                //cerr << "!!! Mesh: Begin section " << token[0] << endl;
                _obj->parseSection(fin);
            }
        }
        else if (nVertices == 0) {
            // Vertices
            nVertices = atoi(token[0].c_str());
            readVector(fin, _vertices, nVertices);

            if (nVertices != _vertices.size())
            {
                OSG_WARN << "DirectX loader: Error reading vertices; " << _vertices.size() << " instead of " << nVertices << endl;
            }
        }
        else if (nFaces == 0) {
            // Faces
            nFaces = atoi(token[0].c_str());
            readMeshFace(fin, _faces, nFaces);

            if (nFaces != _faces.size())
            {
                OSG_WARN << "DirectX loader: Error reading mesh; " << _faces.size() << " instead of " << nFaces << endl;
            }
        }
        else
            OSG_INFO << "!!! " << buf << endl;
    }
}

// Parse 'MeshMaterialList'
void Mesh::parseMeshMaterialList(std::istream& fin)
{
    char buf[256];
    vector<string> token;

    unsigned int nMaterials = 0, nFaceIndices = 0;

    //cerr << "*** MeshMaterialList\n";
    while (fin.getline(buf, sizeof(buf))) {

        // Tokenize
        token.clear();
        tokenize(buf, token);
        if (token.size() == 0)
            continue;

        // check for "{ <material name> }" for a
        // material which was declared globally
        string materialName = token[0];
        // could be given as "{ someName }" which more than 1 tokens
        if (materialName == "{" && token.size()>1)
        {
            materialName = token[1];
        }
        // or could be given as "{someName}" which would be in token[0]
        else if (materialName.size() > 2 && materialName[0] == '{' && materialName[materialName.size()-1] == '}')
        {
            // remove curly brackets
            materialName = materialName.substr(1, materialName.size()-2);
        }
        Material * material = _obj->findMaterial(materialName);

        if (material)
        {
            _materialList->material.push_back(*material);
            continue;
        }

        if (strrchr(buf, '}') != 0)
            break;
        else if (strrchr(buf, '{') != 0) {
            if (token[0] == "Material") {
                Material mm;
                parseMaterial(fin, mm);
                _materialList->material.push_back(mm);
                //cerr << "num mat=" << _materialList->material.size() << endl;
            }
            else {
                //cerr << "!!! MeshMaterialList: Begin section " << token[0] << endl;
                _obj->parseSection(fin);
            }
        }
        else if (nMaterials == 0) {
            // Create MeshMaterialList
            if (!_materialList)
                _materialList = new MeshMaterialList;

            // Materials
            nMaterials = atoi(token[0].c_str());
            //cerr << "expecting num Materials=" << nMaterials << endl;
        }
        else if (nFaceIndices == 0) {
            // Face indices
            nFaceIndices = atoi(token[0].c_str());
            readIndexList(fin, _materialList->faceIndices, nFaceIndices);

            if (nFaceIndices != _materialList->faceIndices.size())
            {
                OSG_WARN << "DirectX loader: Error reading face indices; " << nFaceIndices << " instead of " << _materialList->faceIndices.size() << endl;
            }
        }
    }

    if (nMaterials != _materialList->material.size())
    {
        OSG_WARN << "DirectX loader: Error reading material list; " << nMaterials << " instead of " << _materialList->material.size() << endl;
    }
}

// Parse 'MeshNormals'
void Mesh::parseMeshNormals(std::istream& fin)
{
    char buf[256];
    vector<string> token;

    unsigned int nNormals = 0, nFaceNormals = 0;

    //cerr << "*** MeshNormals\n";
    while (fin.getline(buf, sizeof(buf))) {

        // Tokenize
        token.clear();
        tokenize(buf, token);
        if (token.size() == 0)
            continue;

        if (strrchr(buf, '}') != 0)
            break;
        else if (nNormals == 0) {
            // Create MeshNormals
            if (!_normals)
                _normals = new MeshNormals;

            // Normals
            nNormals = atoi(token[0].c_str());
            readVector(fin, _normals->normals, nNormals);

            if (nNormals != _normals->normals.size())
            {
                OSG_WARN << "DirectX loader: Error reading normals; " << nNormals << " instead of " << _normals->normals.size() << endl;
            }

#define NORMALIZE_NORMALS
#ifdef NORMALIZE_NORMALS
            for (unsigned int i = 0; i < _normals->normals.size(); i++)
                _normals->normals[i].normalize();
#endif
        }
        else if (nFaceNormals == 0) {
            // Face normals
            nFaceNormals = atoi(token[0].c_str());
            readMeshFace(fin, _normals->faceNormals, nFaceNormals);

            if (nFaceNormals != _normals->faceNormals.size())
            {
                OSG_WARN << "DirectX loader: Error reading face normals; " << nFaceNormals << " instead of " << _normals->faceNormals.size() << endl;
            }
        }
    }
}

// Read 'MeshTextureCoords'
void Mesh::readMeshTexCoords(std::istream& fin)
{
    char buf[256];
    vector<string> token;

    unsigned int nTextureCoords = 0;

    //cerr << "*** MeshTextureCoords\n";
    while (fin.getline(buf, sizeof(buf))) {

        // Tokenize
        token.clear();
        tokenize(buf, token);
        if (token.size() == 0)
            continue;

        if (strrchr(buf, '}') != 0)
            break;

        // Create MeshTextureCoords
        if (!_textureCoords)
            _textureCoords = new MeshTextureCoords;

        // Texture coords
        nTextureCoords = atoi(token[0].c_str());
        readCoords2d(fin, *_textureCoords, nTextureCoords);

        if (nTextureCoords != _textureCoords->size())
        {
            OSG_INFO << "DirectX loader: Error reading texcoords; " << _textureCoords->size() << " instead of " << nTextureCoords << endl;
            delete _textureCoords;
            _textureCoords = 0;
        }
    }
}
