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


#if defined(_MSC_VER) && (_MSC_VER <= 1200)
#pragma warning (disable : 4786)
#endif

#include "directx.h"

#include <iostream>
#include <sstream>

#include <math.h>
#include <assert.h>


using namespace DX;
using namespace std;


// Tokenize a string
static void tokenize(const string& str, vector<string>& tokens,
                     const string& delimiters = " \t\r\n;,")
{
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos) {
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of(delimiters, pos);
        pos = str.find_first_of(delimiters, lastPos);
    }
}


// Constructor
Object::Object()
{
    _textureCoords = NULL;
    _materialList = NULL;
    _normals = NULL;
    _mesh = NULL;
}


// Clear object
void Object::clear()
{
    if (_textureCoords) {
        delete _textureCoords;
        _textureCoords = NULL;
    }

    if (_materialList) {
        delete _materialList;
        _materialList = NULL;
    }

    if (_normals) {
        delete _normals;
        _normals = NULL;
    }

    if (_mesh) {
        delete _mesh;
        _mesh = NULL;
    }
}


// Load
bool Object::load(const char* filename)
{
    if (!filename)
        return false;

    // Delete previous
    clear();

    // Open
    ifstream fin(filename);
    if (fin.bad()) {
        cerr << "Object::load: Unable to open: " << filename << endl;
        return false;
    }

    // Parse
    parseSection(fin);
    fin.close();

    return true;
}


/*
 * Generate per-face normals
 */
bool Object::generateNormals(float /*creaseAngle*/)
{
    if (!_mesh)
        return false;

    // Forget old normals
    if (_normals) {
        delete _normals;
        _normals = NULL;
    }

    cout << "*** generateNormals\n";

    /*
     * Calculate per-face normals from face vertices.
     */
    vector<Vector> faceNormals;
    faceNormals.resize(_mesh->faces.size());

    unsigned int fi;
    for (fi = 0; fi < _mesh->faces.size(); fi++) {

        vector<Vector> poly;
        unsigned int n = _mesh->faces[fi].size();

        if (n < 3)
            continue;
        for (unsigned int i = 0; i < n; i++) {
            unsigned int idx = _mesh->faces[fi][i];
            poly.push_back(_mesh->vertices[idx]);
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
    _normals->normals.resize(_mesh->vertices.size());

    for (unsigned int vi = 0; vi < _mesh->vertices.size(); vi++) {

        Vector normal = { 0.0f, 0.0f, 0.0f };
        unsigned int polyCount = 0;

        // Collect normals of polygons that share this vertex
        for (unsigned int fi = 0; fi < _mesh->faces.size(); fi++)
            for (unsigned int i = 0; i < _mesh->faces[fi].size(); i++) {
                unsigned int idx = _mesh->faces[fi][i];
                if (idx == vi) {
                    normal.x += faceNormals[fi].x;
                    normal.y += faceNormals[fi].y;
                    normal.z += faceNormals[fi].z;
                    polyCount++;
                }
            }

        //cerr << "vertex " << vi << " used by " << polyCount << " faces\n";
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
    _normals->faceNormals.resize(_mesh->faces.size());
    for (fi = 0; fi < _mesh->faces.size(); fi++)
        _normals->faceNormals[fi] = _mesh->faces[fi];

    return true;
}


/**********************************************************************
 *
 * Private
 *
 **********************************************************************/

// Read 'TextureFilename'
void Object::readTexFilename(ifstream& fin, TextureFilename& texture)
{
    char buf[256];
    vector<string> token;

    //cout << "*** TexFilename\n";
    while (fin.getline(buf, sizeof(buf))) {

        // Tokenize
        token.clear();
        tokenize(buf, token);
        if (token.size() == 0)
            continue;
        if (token[0] == "}")
            break;

        // Strip " if present
        std::string line = buf;
        std::string::size_type pos = line.find('"');
        if (pos != string::npos) {
            std::string::size_type end = line.rfind('"');
            int len = (end != std::string::npos ? (end-pos-1) : (line.size()-pos));
            texture = line.substr(pos+1, len);
        }
        else
            texture = token[0];
        //cerr << "tex=" << texture << endl;
    }
}


// Parse 'Material'
void Object::parseMaterial(ifstream& fin, Material& material)
{
    char buf[256];
    vector<string> token;

    unsigned int i = 0;

    //cout << "*** Material\n";
    while (fin.getline(buf, sizeof(buf))) {

        // Tokenize
        token.clear();
        tokenize(buf, token);
        if (token.size() == 0)
            continue;
        if (token[0] == "}")
            break;

        if (token[0] == "TextureFilename") {
            TextureFilename tf;
            readTexFilename(fin, tf);
            material.texture.push_back(tf);
            //cerr << "* num tex=" << material.texture.size() << endl;
        }
        else
            switch (i) {
            case 0: {
                // ColorRGBA
                material.faceColor.red = (float) atof(token[0].c_str());
                material.faceColor.green = (float) atof(token[1].c_str());
                material.faceColor.blue = (float) atof(token[2].c_str());
                material.faceColor.alpha = (float) atof(token[3].c_str());
                i++;
            } break;
            case 1: {
                // Power
                material.power = (float) atof(token[0].c_str());
                i++;
            } break;
            case 2: {
                // ColorRGB
                material.specularColor.red = (float) atof(token[0].c_str());
                material.specularColor.green = (float) atof(token[1].c_str());
                material.specularColor.blue = (float) atof(token[2].c_str());
                i++;
            } break;
            case 3: {
                // ColorRGB
                material.emissiveColor.red = (float) atof(token[0].c_str());
                material.emissiveColor.green = (float) atof(token[1].c_str());
                material.emissiveColor.blue = (float) atof(token[2].c_str());
                i++;
            } break;
            }
    }
}


// Read 'Coords2d'
void Object::readCoords2d(ifstream& fin, vector<Coords2d>& v, unsigned int count)
{
    char buf[256];
    vector<string> token;

    unsigned int i = 0;

    //cout << "*** Coords2d\n";
    while (i < count && fin.getline(buf, sizeof(buf))) {

        // Tokenize
        token.clear();
        tokenize(buf, token);
        if (token.size() == 0)
            continue;

        Coords2d c;
        c.u = (float) atof(token[0].c_str());
        c.v = (float) atof(token[1].c_str());
        v.push_back(c);
        i++;
    }
}


// Read 'MeshTextureCoords'
void Object::readMeshTexCoords(ifstream& fin)
{
    char buf[256];
    vector<string> token;

    unsigned int nTextureCoords = 0;

    //cout << "*** MeshTextureCoords\n";
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

        cerr << "* nTextureCoords=" << _textureCoords->size() << endl;
        assert(nTextureCoords == _textureCoords->size());
    }
}


// Read index list
void Object::readIndexList(ifstream& fin, vector<unsigned int>& v, unsigned int count)
{
    char buf[256];
    vector<string> token;

    unsigned int i = 0;

    //cout << "*** IndexList\n";
    while (i < count && fin.getline(buf, sizeof(buf))) {

        // Tokenize
        token.clear();
        tokenize(buf, token);
        if (token.size() == 0)
            continue;

        unsigned int idx = atoi(token[0].c_str());
        v.push_back(idx);
        i++;
    }
}


// Parse 'MeshMaterialList'
void Object::parseMeshMaterialList(ifstream& fin)
{
    char buf[256];
    vector<string> token;

    unsigned int nMaterials = 0, nFaceIndices = 0;

    //cout << "*** MeshMaterialList\n";
    while (fin.getline(buf, sizeof(buf))) {

        // Tokenize
        token.clear();
        tokenize(buf, token);
        if (token.size() == 0)
            continue;

        if (strrchr(buf, '}') != 0)
            break;
        else if (strrchr(buf, '{') != 0) {
            if (token[0] == "Material") {
                Material mm;
                parseMaterial(fin, mm);

                _materialList->material.push_back(mm);
                //cerr << "* num mat=" << _materialList->material.size() << endl;
            }
            else {
                cerr << "!!! MeshMaterialList: Section " << token[0] << endl;
                parseSection(fin);
            }
        }
        else if (nMaterials == 0) {
            // Create MeshMaterialList
            if (!_materialList)
                _materialList = new MeshMaterialList;

            // Materials
            nMaterials = atoi(token[0].c_str());
            //cerr << "* nMaterials=" << nMaterials << endl;
        }
        else if (nFaceIndices == 0) {
            // Face indices
            nFaceIndices = atoi(token[0].c_str());
            readIndexList(fin, _materialList->faceIndices, nFaceIndices);

            cerr << "* nFaceIndices=" << _materialList->faceIndices.size() << endl;
            assert(nFaceIndices == _materialList->faceIndices.size());
        }
    }

    assert(nMaterials == _materialList->material.size());
}


// Read 'Vector'
void Object::readVector(ifstream& fin, vector<Vector>& v, unsigned int count)
{
    char buf[256];
    vector<string> token;

    unsigned int i = 0;

    //cout << "*** Vector\n";
    while (i < count && fin.getline(buf, sizeof(buf))) {

        // Tokenize
        token.clear();
        tokenize(buf, token);
        if (token.size() == 0)
            continue;

        Vector vec;
        vec.x = (float) atof(token[0].c_str());
        vec.y = (float) atof(token[1].c_str());
        vec.z = (float) atof(token[2].c_str());
        v.push_back(vec);
        i++;
    }
}


// Read 'MeshFace'
void Object::readMeshFace(ifstream& fin, vector<MeshFace>& v, unsigned int count)
{
    char buf[256];
    vector<string> token;

    unsigned int i = 0;

    //cout << "*** MeshFace\n";
    while (i < count && fin.getline(buf, sizeof(buf))) {

        // Tokenize
        token.clear();
        tokenize(buf, token);
        if (token.size() == 0)
            continue;

        MeshFace mf;
        unsigned int n = atoi(token[0].c_str());

        for (unsigned int j = 0; j < n; j++) {
            unsigned int idx = atoi(token[j+1].c_str());
            mf.push_back(idx);
        }
        v.push_back(mf);
        i++;
    }
}


// Parse 'MeshNormals'
void Object::parseMeshNormals(ifstream& fin)
{
    char buf[256];
    vector<string> token;

    unsigned int nNormals = 0, nFaceNormals = 0;

    //cout << "*** MeshNormals\n";
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

            cerr << "* nNormals=" << _normals->normals.size() << endl;
            assert(nNormals == _normals->normals.size());

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

            cerr << "* nFaceNormals=" << _normals->faceNormals.size() << endl;
            assert(nFaceNormals == _normals->faceNormals.size());
        }
    }
}


// Parse 'Mesh'
void Object::parseMesh(ifstream& fin)
{
    char buf[256];
    vector<string> token;

    unsigned int nVertices = 0, nFaces = 0;

    //cout << "*** Mesh\n";
    while (fin.getline(buf, sizeof(buf))) {

        // Tokenize
        token.clear();
        tokenize(buf, token);
        if (token.size() == 0)
            continue;

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
                cerr << "!!! Mesh: Section " << token[0] << endl;
                parseSection(fin);
            }
        }
        else if (nVertices == 0) {
            // Create mesh
            if (!_mesh)
                _mesh = new Mesh;

            // Vertices
            nVertices = atoi(token[0].c_str());
            readVector(fin, _mesh->vertices, nVertices);

            cerr << "* nVertices=" << _mesh->vertices.size() << endl;
            assert(nVertices == _mesh->vertices.size());
        }
        else if (nFaces == 0) {
            // Faces
            nFaces = atoi(token[0].c_str());
            readMeshFace(fin, _mesh->faces, nFaces);

            cerr << "* nFaces=" << _mesh->faces.size() << endl;
            assert(nFaces == _mesh->faces.size());
        }
        else
            cerr << "!!! " << buf << endl;
    }
}


// Parse section
void Object::parseSection(ifstream& fin)
{
    char buf[256];
    vector<string> token;

    while (fin.getline(buf, sizeof(buf))) {

        if (strrchr(buf, '}') != 0) {
            //cerr << "!!! End section\n";
            break;
        }
        else if (strrchr(buf, '{') != 0) {
            // Tokenize
            token.clear();
            tokenize(buf, token);
            if (token.size() == 0)
                continue;

            //cerr << "!!! Section " << token[0] << endl;
            if (token[0] == "Mesh")
                parseMesh(fin);
            else
                parseSection(fin);
        }
    }
}
