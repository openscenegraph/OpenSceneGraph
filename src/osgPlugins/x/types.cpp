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

#include "types.h"

#include <iostream>
#include <string.h>
#include <stdlib.h>

using namespace std;


namespace DX {

    // Tokenize a string
    void tokenize(const string & str, vector<string> & tokens, const string & delimiters)
    {
        string::size_type lastPos = str.find_first_not_of(delimiters, 0);
        string::size_type pos     = str.find_first_of(delimiters, lastPos);

        while (string::npos != pos || string::npos != lastPos) {
            tokens.push_back(str.substr(lastPos, pos - lastPos));
            lastPos = str.find_first_not_of(delimiters, pos);
            pos = str.find_first_of(delimiters, lastPos);
        }
    }

    // Read 'TextureFilename'
    void readTexFilename(istream & fin, TextureFilename & texture)
    {
        char buf[256];
        vector<string> token;

        //cerr << "*** TexFilename\n";
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
            //cerr << "* tex='" << texture << "'" << endl;
        }
    }

    // Read 'Coords2d'
    void readCoords2d(istream & fin, vector<Coords2d> & v, unsigned int count)
    {
        char buf[256];
        vector<string> token;

        unsigned int i = 0;

        //cerr << "*** Coords2d\n";
        while (i < count && fin.getline(buf, sizeof(buf))) {

            // Tokenize
            token.clear();
            tokenize(buf, token);
            if (token.size() == 0)
                continue;

            Coords2d c;
            c.u = osg::asciiToFloat(token[0].c_str());
            c.v = osg::asciiToFloat(token[1].c_str());
            v.push_back(c);
            i++;
        }
    }

    // Read 'Vector'
    void readVector(istream & fin, vector<Vector> & v, unsigned int count)
    {
        char buf[256];
        vector<string> token;

        unsigned int i = 0;

        //cerr << "*** Vector\n";
        while (i < count && fin.getline(buf, sizeof(buf))) {

            // Tokenize
            token.clear();
            tokenize(buf, token);
            if (token.size() == 0)
                continue;

            Vector vec;
            vec.x = osg::asciiToFloat(token[0].c_str());
            vec.y = osg::asciiToFloat(token[1].c_str());
            vec.z = osg::asciiToFloat(token[2].c_str());
            v.push_back(vec);
            i++;
        }
    }

    // Parse 'Material'
    void parseMaterial(istream & fin, Material & material)
    {
        char buf[256];
        vector<string> token;

        unsigned int i = 0;

        //cerr << "*** Material\n";
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
            } else {
                switch (i) {
                    case 0: {
                                // ColorRGBA
                                material.faceColor.red = osg::asciiToFloat(token[0].c_str());
                                material.faceColor.green = osg::asciiToFloat(token[1].c_str());
                                material.faceColor.blue = osg::asciiToFloat(token[2].c_str());
                                material.faceColor.alpha = osg::asciiToFloat(token[3].c_str());
                                i++;
                            } break;
                    case 1: {
                                // Power
                                material.power = osg::asciiToFloat(token[0].c_str());
                                i++;
                            } break;
                    case 2: {
                                // ColorRGB
                                material.specularColor.red = osg::asciiToFloat(token[0].c_str());
                                material.specularColor.green = osg::asciiToFloat(token[1].c_str());
                                material.specularColor.blue = osg::asciiToFloat(token[2].c_str());
                                i++;
                            } break;
                    case 3: {
                                // ColorRGB
                                material.emissiveColor.red = osg::asciiToFloat(token[0].c_str());
                                material.emissiveColor.green = osg::asciiToFloat(token[1].c_str());
                                material.emissiveColor.blue = osg::asciiToFloat(token[2].c_str());
                                i++;
                            } break;
                }
            }
        }
    }

    // Read index list
    void readIndexList(istream & fin, vector<unsigned int> & v, unsigned int count)
    {
        char buf[256];
        vector<string> token;

        unsigned int i = 0;

        //cerr << "*** IndexList\n";
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

    // Read 'MeshFace'
    void readMeshFace(istream & fin, vector<MeshFace> & v, unsigned int count)
    {
        char buf[256];
        vector<string> token;

        unsigned int i = 0;

        //cerr << "*** MeshFace\n";
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

}
