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

#include "directx.h"

#include <iostream>
#include <sstream>

#include <math.h>

#include <osg/Notify>

using namespace DX;
using namespace std;


/**********************************************************************
 *
 * DirectX object.
 *
 **********************************************************************/

Object::Object()
{
}

void Object::clear()
{
    for (unsigned int i = 0; i < _meshes.size(); ++i) {
        _meshes[i]->clear();
    }
    _meshes.clear();
}

bool Object::load(const char* filename)
{
    if (!filename)
        return false;

    ifstream fin(filename);
    if (fin.bad()) {
        osg::notify(osg::WARN) << "Object::load: Unable to open: " << filename << endl;
        return false;
    }

    parseSection(fin);
    fin.close();

    return true;
}

bool Object::generateNormals(float creaseAngle)
{
    bool result = true;
    for (unsigned int i = 0; i < _meshes.size(); ++i) {
        result &= _meshes[i]->generateNormals(creaseAngle);
    }

    return result;
}

Material * Object::findMaterial(const std::string & name)
{
    std::vector<Material>::iterator itr;
    for (itr = _globalMaterials.begin(); itr != _globalMaterials.end(); ++itr) {
        //cerr << "search=" << name << " have=" << (*itr).name << endl;
        if ((*itr).name == name) {
            return &*itr;
        }
    }

    return 0;
}


/**********************************************************************
 *
 * Private
 *
 **********************************************************************/

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

            if (token[0] == "Mesh") {
                // create new mesh
                //cerr << "!!! Begin Mesh" << endl;
                Mesh * mesh = new Mesh(this);
                _meshes.push_back(mesh);
                mesh->parseMesh(fin);
            }
            else if (token[0] == "Material") {
                //
                // dgm - In later versions of directx files, materials
                // can be declared at the top of the file (e.g. globally).
                // Keep this list of global materials in "_globalMaterials"
                //
                Material mm;
                if (token.size() > 1 && token[1] != "") {
                    mm.name = token[1];
                }
                parseMaterial(fin, mm);
                _globalMaterials.push_back(mm);
            }
            else if (token[0] == "Frame") {
                parseFrame(fin);
            }
            else {
                //cerr << "!!! Begin section " << token[0] << endl;
                parseSection(fin);
            }
        }
    }
}

// Parse frame
void Object::parseFrame(ifstream& /*fin*/)
{
}
