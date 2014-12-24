/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2004 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <functional>

#include "obj.h"

#include <osg/Notify>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <string.h>

using namespace obj;


#ifdef _MSC_VER
#define strncasecmp strnicmp
#endif


static std::string strip( const std::string& ss )
{
    std::string result;
    result.assign( std::find_if( ss.begin(), ss.end(), std::not1( std::ptr_fun< int, int >( isspace ) ) ),
                   std::find_if( ss.rbegin(), ss.rend(), std::not1( std::ptr_fun< int, int >( isspace ) ) ).base() );
    return( result );
}

/*
 * parse a subset of texture options, following
 * http://local.wasp.uwa.edu.au/~pbourke/dataformats/mtl/
 */
static Material::Map parseTextureMap( const std::string& ss, Material::Map::TextureMapType type)
{
    Material::Map map;
    std::string s(ss);
    for (;;)
    {
        if (s[0] != '-')
            break;

        int n;
        if (s[1] == 's' || s[1] == 'o')
        {
            float x, y, z;
            if (sscanf(s.c_str(), "%*s %f %f %f%n", &x, &y, &z, &n) != 3)
            {
                break;
            }

            if (s[1] == 's')
            {
                // texture scale
                map.uScale = x;
                map.vScale = y;
            }
            else if (s[1] == 'o')
            {
                // texture offset
                map.uOffset = x;
                map.vOffset = y;
            }
        }
        else if (s.compare(1,2,"mm")==0)
        {
            // texture color offset and gain
            float base, gain;
            if (sscanf(s.c_str(), "%*s %f %f%n", &base, &gain, &n) != 2)
            {
                break;
            }
            // UNUSED
        }
        else if (s.compare(1,2,"bm")==0)
        {
            // blend multiplier
            float mult;
            if (sscanf(s.c_str(), "%*s %f%n", &mult, &n) != 2)
            {
                break;
            }
            // UNUSED
        }
        else if (s.compare(1,5,"clamp")==0)
        {
            OSG_NOTICE<<"Got Clamp\n";
            char c[4];
            if (sscanf(s.c_str(), "%*s %3s%n", c, &n) != 1)
            {
                break;
            }
            if(strncmp(c,"on",2)==0) map.clamp = true;
            else map.clamp = false;    // default behavioud
        }
        else
            break;

        s = strip(s.substr(n));
    }

    map.name = osgDB::convertFileNameToNativeStyle(s);
    map.type = type;
    return map;
}

bool Model::readline(std::istream& fin, char* line, const int LINE_SIZE)
{
    if (LINE_SIZE<1) return false;

    bool eatWhiteSpaceAtStart = true;
    bool changeTabsToSpaces = true;

    char* ptr = line;
    char* end = line+LINE_SIZE-1;
    bool skipNewline = false;
    while (fin && ptr<end)
    {

        int c=fin.get();
        int p=fin.peek();
        if (c=='\r')
        {
            if (p=='\n')
            {
                // we have a windows line endings.
                fin.get();
                // OSG_NOTICE<<"We have dos line ending"<<std::endl;
                if (skipNewline)
                {
                    skipNewline = false;
                    *ptr++ = ' ';
                    continue;
                }
                else break;
            }
            // we have Mac line ending
            // OSG_NOTICE<<"We have mac line ending"<<std::endl;
            if (skipNewline)
            {
                skipNewline = false;
                *ptr++ = ' ';
                continue;
            }
            else break;
        }
        else if (c=='\n')
        {
            // we have unix line ending.
            // OSG_NOTICE<<"We have unix line ending"<<std::endl;
            if (skipNewline)
            {
                *ptr++ = ' ';
                continue;
            }
            else break;
        }
        else if (c=='\\' && (p=='\r' || p=='\n'))
        {
            // need to keep return;
            skipNewline = true;
        }
        else if (c!=std::ifstream::traits_type::eof()) // don't copy eof.
        {
            skipNewline = false;

            if (!eatWhiteSpaceAtStart || (c!=' ' && c!='\t'))
            {
                eatWhiteSpaceAtStart = false;
                *ptr++ = c;
            }
        }


    }

    // strip trailing spaces
    while (ptr>line && *(ptr-1)==' ')
    {
        --ptr;
    }

    *ptr = 0;

    if (changeTabsToSpaces)
    {

        for(ptr = line; *ptr != 0; ++ptr)
        {
            if (*ptr == '\t') *ptr=' ';
        }
    }

    return true;
}


std::string Model::lastComponent(const char* linep)
{
    std::string line = std::string(linep);
    int space = line.find_last_of(" ");
    if (space >= 0) {
        line = line.substr(space+1);
    }
    return line;
}

bool Model::readMTL(std::istream& fin)
{
    OSG_INFO<<"Reading MTL file"<<std::endl;

    const int LINE_SIZE = 4096;
    char line[LINE_SIZE];
    float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
    bool usingDissolve = false;

    Material* material = 0;// &(materialMap[""]);
    std::string filename;

    while (fin)
    {
        readline(fin,line,LINE_SIZE);
        if (line[0]=='#' || line[0]=='$')
        {
            // comment line
            // OSG_NOTICE <<"Comment: "<<line<<std::endl;
        }
        else if (strlen(line)>0)
        {
            if (strncasecmp(line,"newmtl ",7)==0)
            {
                // get material name and left- and right-trim all the white-space
                std::string materialName(strip(line+7));
                material = & materialMap[materialName];
                material->name = materialName;
                usingDissolve = false;
            }
            else if (material)
            {
                if (strncasecmp(line,"Ka ",3)==0)
                {
                    unsigned int fieldsRead = sscanf(line+3,"%f %f %f %f", &r, &g, &b, &a);

                    if (fieldsRead==1)
                    {
                        material->ambient[ 0 ] = r;
                    }
                    else if (fieldsRead==2)
                    {
                        material->ambient[ 0 ] = r;
                        material->ambient[ 1 ] = g;
                    }
                    else if (fieldsRead==3)
                    {
                        material->ambient[ 0 ] = r;
                        material->ambient[ 1 ] = g;
                        material->ambient[ 2 ] = b;
                    }
                    else if (fieldsRead==4)
                    {
                        material->ambient[ 0 ] = r;
                        material->ambient[ 1 ] = g;
                        material->ambient[ 2 ] = b;
                        material->ambient[ 3 ] = a;
                    }
                }
                else if (strncasecmp(line,"Kd ",3)==0)
                {
                    unsigned int fieldsRead = sscanf(line+3,"%f %f %f %f", &r, &g, &b, &a);

                    if (fieldsRead==1)
                    {
                        material->diffuse[ 0 ] = r;
                    }
                    else if (fieldsRead==2)
                    {
                        material->diffuse[ 0 ] = r;
                        material->diffuse[ 1 ] = g;
                    }
                    else if (fieldsRead==3)
                    {
                        material->diffuse[ 0 ] = r;
                        material->diffuse[ 1 ] = g;
                        material->diffuse[ 2 ] = b;
                    }
                    else if (fieldsRead==4)
                    {
                        material->diffuse[ 0 ] = r;
                        material->diffuse[ 1 ] = g;
                        material->diffuse[ 2 ] = b;
                        material->diffuse[ 3 ] = a;
                    }
                }
                else if (strncasecmp(line,"Ks ",3)==0)
                {
                    unsigned int fieldsRead = sscanf(line+3,"%f %f %f %f", &r, &g, &b, &a);

                    if (fieldsRead==1)
                    {
                        material->specular[ 0 ] = r;
                    }
                    else if (fieldsRead==2)
                    {
                        material->specular[ 0 ] = r;
                        material->specular[ 1 ] = g;
                    }
                    else if (fieldsRead==3)
                    {
                        material->specular[ 0 ] = r;
                        material->specular[ 1 ] = g;
                        material->specular[ 2 ] = b;
                    }
                    else if (fieldsRead==4)
                    {
                        material->specular[ 0 ] = r;
                        material->specular[ 1 ] = g;
                        material->specular[ 2 ] = b;
                        material->specular[ 3 ] = a;
                    }
                }
                else if (strncasecmp(line,"Ke ",3)==0)
                {
                    unsigned int fieldsRead = sscanf(line+3,"%f %f %f %f", &r, &g, &b, &a);

                    if (fieldsRead==1)
                    {
                        material->emissive[ 0 ] = r;
                    }
                    else if (fieldsRead==2)
                    {
                        material->emissive[ 0 ] = r;
                        material->emissive[ 1 ] = g;
                    }
                    else if (fieldsRead==3)
                    {
                        material->emissive[ 0 ] = r;
                        material->emissive[ 1 ] = g;
                        material->emissive[ 2 ] = b;
                    }
                    else if (fieldsRead==4)
                    {
                        material->emissive[ 0 ] = r;
                        material->emissive[ 1 ] = g;
                        material->emissive[ 2 ] = b;
                        material->emissive[ 3 ] = a;
                    }
                }
                else if (strncasecmp(line,"Tf ",3)==0)
                {
                    unsigned int fieldsRead = sscanf(line+3,"%f %f %f %f", &r, &g, &b, &a);

                    if (fieldsRead==1)
                    {
                        material->Tf[ 0 ] = r;
                    }
                    else if (fieldsRead==2)
                    {
                        material->Tf[ 0 ] = r;
                        material->Tf[ 1 ] = g;
                    }
                    else if (fieldsRead==3)
                    {
                        material->Tf[ 0 ] = r;
                        material->Tf[ 1 ] = g;
                        material->Tf[ 2 ] = b;
                    }
                    else if (fieldsRead==4)
                    {
                        material->Tf[ 0 ] = r;
                        material->Tf[ 1 ] = g;
                        material->Tf[ 2 ] = b;
                        material->Tf[ 3 ] = a;
                    }
                }
                else if (strncasecmp(line,"sharpness ",10)==0)
                {
                    float sharpness = 0.0f;
                    unsigned int fieldsRead = sscanf(line+10,"%f", &sharpness);

                    if (fieldsRead==1) material->sharpness = sharpness;
                }
                else if (strncasecmp(line,"illum ",6)==0)
                {
                    int illum = 0;
                    unsigned int fieldsRead = sscanf(line+6,"%d", &illum);

                    if (fieldsRead==1) material->illum = illum;
                }
                else if (strncasecmp(line,"Ns ",3)==0)
                {
                    int Ns = 0;
                    unsigned int fieldsRead = sscanf(line+3,"%d", &Ns);

                    if (fieldsRead==1) material->Ns = Ns;
                }
                else if (strncasecmp(line,"Ni ",3)==0)
                {
                    int Ni = 0;
                    unsigned int fieldsRead = sscanf(line+3,"%d", &Ni);

                    if (fieldsRead==1) material->Ni = Ni;
                }
                //
                // Tr - transparency
                //
                // Seems that the world did not agreed about the specification of the item.
                //
                // Some thinks that value of 1 means opaque material and 0 transparent material,
                // such as http://people.sc.fsu.edu/~jburkardt/data/mtl/mtl.html .
                //
                // However, 3ds Max export uses the opposite: 0 means opaque material and
                // 1 completely transparent material. These 3ds Max exported files
                // carry the following signature as the first line in the file (*.obj, *.mtl):
                // # 3ds Max Wavefront OBJ Exporter v0.97b - (c)2007 guruware
                //
                // Moreover, at least one model uses Tr followed by two numbers.
                // Such model can be downloaded from http://graphics.cs.williams.edu/data/meshes/cube.zip
                // (part of the following collection: http://graphics.cs.williams.edu/data/meshes.xml).
                //
                // Current solution: As we do not know what is the correct interpretation of
                // the value 0 and value 1 for Tr, we will rely on d (dissolve) parameter instead
                // whenever it is present. This seems to fix the problem on large number of models.
                //
                else if (strncasecmp(line,"Tr ",3)==0)
                {
                    if( !usingDissolve )
                    {
                        float alpha=1.0f;
                        unsigned int fieldsRead = sscanf(line+3,"%f", &alpha);

                        if (fieldsRead==1)
                        {
                            material->ambient[3] = alpha;
                            material->diffuse[3] = alpha;
                            material->specular[3] = alpha;
                            material->emissive[3] = alpha;
                        }
                    }
                }
                //
                // d - dissolve (pseudo-transparency)
                //
                // Dissolve of value 1 means completely opaque material
                // and value of 0 results in completely transparent material.
                //
                // To be compatible with 3D Max obj exporter,
                // d takes precedence over Tr (handled through usingDissolve variable).
                //
                else if (strncasecmp(line,"d ",2)==0)
                {
                    float alpha=1.0f;
                    unsigned int fieldsRead = sscanf(line+2,"%f", &alpha);

                    if (fieldsRead==1)
                    {
                        material->ambient[3] = alpha;
                        material->diffuse[3] = alpha;
                        material->specular[3] = alpha;
                        material->emissive[3] = alpha;
                        usingDissolve = true;
                    }
                }
                else if (strncasecmp(line,"map_Ka ",7)==0)
                {
                    material->maps.push_back(parseTextureMap(strip(line+7),Material::Map::AMBIENT));
                }
                // diffuse map
                else if (strncasecmp(line,"map_Kd ",7)==0)
                {
                    material->maps.push_back(parseTextureMap(strip(line+7),Material::Map::DIFFUSE));
                }
                // specular colour/level map
                else if (strncasecmp(line,"map_Ks ",7)==0)
                {
                     material->maps.push_back(parseTextureMap(strip(line+7),Material::Map::SPECULAR));
                }
                // map_opacity doesn't exist in the spec, but was already in the plugin
                // so leave it or plugin will break for some users
                else if (strncasecmp(line,"map_opacity ",12)==0)
                {
                    material->maps.push_back(parseTextureMap(strip(line+12),Material::Map::OPACITY));
                }
                // proper dissolve/opacity map
                else if (strncasecmp(line,"map_d ",6)==0)
                {
                    material->maps.push_back(parseTextureMap(strip(line+6),Material::Map::OPACITY));
                }
                // specular exponent map
                else if (strncasecmp(line,"map_Ns ",7)==0)
                {
                    material->maps.push_back(parseTextureMap(strip(line+7),Material::Map::SPECULAR_EXPONENT));
                }
                // modelling tools and convertors variously produce bump, map_bump, and map_Bump so parse them all
                else if (strncasecmp(line,"bump ",5)==0 || strncasecmp(line,"map_bump ",9)==0)
                {
                    material->maps.push_back(parseTextureMap(strip(line+5),Material::Map::BUMP));
                }
                else if (strncasecmp(line,"map_bump ",9)==0)
                {
                    material->maps.push_back(parseTextureMap(strip(line+9),Material::Map::BUMP));
                }
                else if (strncasecmp(line,"map_Bump ",9)==0)
                {
                    material->maps.push_back(parseTextureMap(strip(line+9),Material::Map::BUMP));
                }
                // displacement map
                else if (strncasecmp(line,"disp ",5)==0)
                {
                    material->maps.push_back(parseTextureMap(strip(line+5),Material::Map::DISPLACEMENT));
                }
                // reflection map (the original code had the possibility of a blank "refl" line
                // which isn't correct according to the spec, so this bit might break for some
                // modelling packages...
                else if (strncasecmp(line,"refl ",5)==0)
                {
                    material->maps.push_back(parseTextureMap(strip(line+5),Material::Map::REFLECTION));
                }
                else
                {
                    OSG_NOTICE <<"*** line not handled *** :"<<line<<std::endl;
                }
            }
            else
            {
                OSG_NOTICE <<"*** line not handled *** :"<<line<<std::endl;
            }

        }

    }

    return true;
}

std::string trim(const std::string& s)
{
  if(s.length() == 0)
    return s;
  int b = s.find_first_not_of(" \t");
  int e = s.find_last_not_of(" \t");
  if(b == -1) // No non-spaces
    return "";
  return std::string(s, b, e - b + 1);
}

inline bool isZBrushColorField(char* line)
{
    return strncmp(line, "#MRGB", 5) == 0;
}

bool Model::readOBJ(std::istream& fin, const osgDB::ReaderWriter::Options* options)
{
    OSG_INFO<<"Reading OBJ file"<<std::endl;

    const int LINE_SIZE = 4096;
    char line[LINE_SIZE];
    float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;
    float r,g,b,a;

    while (fin)
    {
        readline(fin,line,LINE_SIZE);
        if ((line[0]=='#' && !isZBrushColorField(line)) || line[0]=='$')
        {
            // comment line
            // OSG_NOTICE <<"Comment: "<<line<<std::endl;
        }
        else if(isZBrushColorField(line))
        {
            // Get the zBrush vertex colors given in comments under the form :
            // * #MRGB MMRRGGBB MMRRGGBB ... (up to 64 hexadecimal color fields)
            std::string colorFields(line + 6);
            while (colorFields.size() >= 8)
            {
                std::string currentValue;

                // Skipping the MM component
                colorFields = colorFields.substr(2);

                currentValue = colorFields.substr(0,2);
                r = static_cast<float>(strtol(currentValue.c_str(), NULL, 16)) / 255.;
                colorFields = colorFields.substr(2);

                currentValue = colorFields.substr(0,2);
                g = static_cast<float>(strtol(currentValue.c_str(), NULL, 16)) / 255.;
                colorFields = colorFields.substr(2);

                currentValue = colorFields.substr(0,2);
                b = static_cast<float>(strtol(currentValue.c_str(), NULL, 16)) / 255.;
                colorFields = colorFields.substr(2);

                colors.push_back(osg::Vec4(r, g, b, 1.0));
            }
        }
        else if (strlen(line)>0)
        {
            if (strncmp(line,"v ",2)==0)
            {
                unsigned int fieldsRead = sscanf(line+2,"%f %f %f %f %f %f %f", &x, &y, &z, &w, &g, &b, &a);

                if (fieldsRead==1)
                    vertices.push_back(osg::Vec3(x,0.0f,0.0f));
                else if (fieldsRead==2)
                    vertices.push_back(osg::Vec3(x,y,0.0f));
                else if (fieldsRead==3)
                    vertices.push_back(osg::Vec3(x,y,z));
                else if (fieldsRead == 4)
                    vertices.push_back(osg::Vec3(x/w,y/w,z/w));
                else if (fieldsRead == 6)
                {
                    vertices.push_back(osg::Vec3(x,y,z));
                    colors.push_back(osg::Vec4(w, g, b, 1.0));
                }
                else if ( fieldsRead == 7 )
                {
                    vertices.push_back(osg::Vec3(x,y,z));
                    colors.push_back(osg::Vec4(w, g, b, a));
                }
            }
            else if (strncmp(line,"vn ",3)==0)
            {
                unsigned int fieldsRead = sscanf(line+3,"%f %f %f", &x, &y, &z);

                if (fieldsRead==1) normals.push_back(osg::Vec3(x,0.0f,0.0f));
                else if (fieldsRead==2) normals.push_back(osg::Vec3(x,y,0.0f));
                else if (fieldsRead==3) normals.push_back(osg::Vec3(x,y,z));
            }
            else if (strncmp(line,"vt ",3)==0)
            {
                unsigned int fieldsRead = sscanf(line+3,"%f %f %f", &x, &y, &z);

                if (fieldsRead==1) texcoords.push_back(osg::Vec2(x,0.0f));
                else if (fieldsRead==2) texcoords.push_back(osg::Vec2(x,y));
                else if (fieldsRead==3) texcoords.push_back(osg::Vec2(x,y));
            }
            else if (strncmp(line,"l ",2)==0 ||
                     strncmp(line,"p ",2)==0 ||
                     strncmp(line,"f ",2)==0)
            {
                char* ptr = line+2;

                Element* element = new Element( (line[0]=='p') ? Element::POINTS :
                                                (line[0]=='l') ? Element::POLYLINE :
                                                Element::POLYGON );

                // OSG_NOTICE<<"face"<<ptr<<std::endl;

                int vi=0, ti=0, ni=0;
                while(*ptr!=0)
                {
                    // skip white space
                    while(*ptr==' ') ++ptr;

                    if (sscanf(ptr, "%d/%d/%d", &vi, &ti, &ni) == 3)
                    {
                        // OSG_NOTICE<<"   vi="<<vi<<"/ti="<<ti<<"/ni="<<ni<<std::endl;
                        element->vertexIndices.push_back(remapVertexIndex(vi));
                        element->normalIndices.push_back(remapNormalIndex(ni));
                        element->texCoordIndices.push_back(remapTexCoordIndex(ti));
                    }
                    else if (sscanf(ptr, "%d//%d", &vi, &ni) == 2)
                    {
                        // OSG_NOTICE<<"   vi="<<vi<<"//ni="<<ni<<std::endl;
                        element->vertexIndices.push_back(remapVertexIndex(vi));
                        if (remapNormalIndex(ni) < static_cast<int>(normals.size()))
                            element->normalIndices.push_back(remapNormalIndex(ni));
                    }
                    else if (sscanf(ptr, "%d/%d", &vi, &ti) == 2)
                    {
                        // OSG_NOTICE<<"   vi="<<vi<<"/ti="<<ti<<std::endl;
                        element->vertexIndices.push_back(remapVertexIndex(vi));
                        if (remapTexCoordIndex(ti) < static_cast<int>(texcoords.size()))
                            element->texCoordIndices.push_back(remapTexCoordIndex(ti));
                    }
                    else if (sscanf(ptr, "%d", &vi) == 1)
                    {
                        // OSG_NOTICE<<"   vi="<<vi<<std::endl;
                        element->vertexIndices.push_back(remapVertexIndex(vi));
                    }

                    // skip to white space or end of line
                    while(*ptr!=' ' && *ptr!=0) ++ptr;

                }

                if (!element->normalIndices.empty() && element->normalIndices.size() != element->vertexIndices.size())
                {
                    element->normalIndices.clear();
                }

                if (!element->texCoordIndices.empty() && element->texCoordIndices.size() != element->vertexIndices.size())
                {
                    element->texCoordIndices.clear();
                }

                if (!element->vertexIndices.empty())
                {
                    Element::CoordinateCombination coordateCombination = element->getCoordinateCombination();
                    if (coordateCombination!=currentElementState.coordinateCombination)
                    {
                        currentElementState.coordinateCombination = coordateCombination;
                        currentElementList = 0; // reset the element list to force a recompute of which ElementList to use
                    }
                    addElement(element);
                }
                else
                {
                    // empty element, don't both adding, just unref to delete it.
                    element->unref();
                }

            }
            else if (strncmp(line,"usemtl ",7)==0)
            {
                std::string materialName( line+7 );
                if (currentElementState.materialName != materialName)
                {
                    currentElementState.materialName = materialName;
                    currentElementList = 0; // reset the element list to force a recompute of which ElementList to use
                }
            }
            else if (strncmp(line,"mtllib ",7)==0)
            {
                std::string materialFileName = trim( line+7 );
                std::string fullPathFileName = osgDB::findDataFile( materialFileName, options );
                if (!fullPathFileName.empty())
                {
                    osgDB::ifstream mfin( fullPathFileName.c_str() );
                    if (mfin)
                    {
                        OSG_INFO << "Obj reading mtllib '" << fullPathFileName << "'\n";
                        readMTL(mfin);
                    }
                    else
                    {
                        OSG_WARN << "Obj unable to load mtllib '" << fullPathFileName << "'\n";
                    }
                }
                else
                {
                    OSG_WARN << "Obj unable to find mtllib '" << materialFileName << "'\n";
                }
            }
            else if (strncmp(line,"o ",2)==0)
            {
                std::string objectName(line+2);
                if (currentElementState.objectName != objectName)
                {
                    currentElementState.objectName = objectName;
                    currentElementList = 0; // reset the element list to force a recompute of which ElementList to use
                }
            }
            else if (strcmp(line,"o")==0)
            {
                std::string objectName(""); // empty name
                if (currentElementState.objectName != objectName)
                {
                    currentElementState.objectName = objectName;
                    currentElementList = 0; // reset the element list to force a recompute of which ElementList to use
                }
            }
            else if (strncmp(line,"g ",2)==0)
            {
                std::string groupName(line+2);
                if (currentElementState.groupName != groupName)
                {
                    currentElementState.groupName = groupName;
                    currentElementList = 0; // reset the element list to force a recompute of which ElementList to use
                }
            }
            else if (strcmp(line,"g")==0)
            {
                std::string groupName(""); // empty name
                if (currentElementState.groupName != groupName)
                {
                    currentElementState.groupName = groupName;
                    currentElementList = 0; // reset the element list to force a recompute of which ElementList to use
                }
            }
            else if (strncmp(line,"s ",2)==0)
            {
                int smoothingGroup=0;
                if (strncmp(line+2,"off",3)==0) smoothingGroup = 0;
                else sscanf(line+2,"%d",&smoothingGroup);

                if (currentElementState.smoothingGroup != smoothingGroup)
                {
                    currentElementState.smoothingGroup = smoothingGroup;
                    currentElementList = 0; // reset the element list to force a recompute of which ElementList to use
                }
            }
            else
            {
                OSG_NOTICE <<"*** line not handled *** :"<<line<<std::endl;
            }

        }

    }
#if 0
    OSG_NOTICE <<"vertices :"<<vertices.size()<<std::endl;
    OSG_NOTICE <<"normals :"<<normals.size()<<std::endl;
    OSG_NOTICE <<"texcoords :"<<texcoords.size()<<std::endl;
    OSG_NOTICE <<"materials :"<<materialMap.size()<<std::endl;
    OSG_NOTICE <<"elementStates :"<<elementStateMap.size()<<std::endl;

    unsigned int pos=0;
    for(ElementStateMap::iterator itr=elementStateMap.begin();
        itr!=elementStateMap.end();
        ++itr,++pos)
    {
        const ElementState& es = itr->first;
        ElementList& el = itr->second;
        OSG_NOTICE<<"ElementState "<<pos<<std::endl;
        OSG_NOTICE<<"    es.objectName="<<es.objectName<<std::endl;
        OSG_NOTICE<<"    es.groupName="<<es.groupName<<std::endl;
        OSG_NOTICE<<"    es.materialName="<<es.materialName<<std::endl;
        OSG_NOTICE<<"    es.smoothGroup="<<es.smoothingGroup<<std::endl;
        OSG_NOTICE<<"    ElementList ="<<el.size()<<std::endl;

    }
#endif
    return true;
}


void Model::addElement(Element* element)
{
    if (!currentElementList)
    {
        currentElementList = & (elementStateMap[currentElementState]);
    }
    currentElementList->push_back(element);

}

osg::Vec3 Model::averageNormal(const Element& element) const
{
    osg::Vec3 normal;
    for(Element::IndexList::const_iterator itr=element.normalIndices.begin();
        itr!=element.normalIndices.end();
        ++itr)
    {
        normal += normals[*itr];
    }
    normal.normalize();

    return normal;
}

osg::Vec3 Model::computeNormal(const Element& element) const
{
    osg::Vec3 normal;
    for(unsigned int i=0;i<element.vertexIndices.size()-2;++i)
    {
        osg::Vec3 a = vertices[element.vertexIndices[i]];
        osg::Vec3 b = vertices[element.vertexIndices[i+1]];
        osg::Vec3 c = vertices[element.vertexIndices[i+2]];
        osg::Vec3 localNormal = (b-a)   ^(c-b);
        normal += localNormal;
    }
    normal.normalize();

    return normal;
}

bool Model::needReverse(const Element& element) const
{
    if (element.normalIndices.empty()) return false;

    return computeNormal(element)*averageNormal(element) < 0.0f;
}
