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

#include <fstream>

#include "obj.h"

#include <osg/Notify>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

using namespace obj;

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
                // osg::notify(osg::NOTICE)<<"We have dos line ending"<<std::endl;
                if (skipNewline)
                {
                    skipNewline = false; 
                    continue;
                }
                else break;
            }
            // we have Mac line ending
            // osg::notify(osg::NOTICE)<<"We have mac line ending"<<std::endl;
            if (skipNewline)
            {
                skipNewline = false; 
                continue;
            }
            else break;
        }
        else if (c=='\n')
        {
            // we have unix line ending.
            // osg::notify(osg::NOTICE)<<"We have unix line ending"<<std::endl;
            if (skipNewline)
            {
                skipNewline = false; 
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


bool Model::readMTL(std::istream& fin)
{
    osg::notify(osg::INFO)<<"Reading MTL file"<<std::endl;

    const int LINE_SIZE = 4096;
    char line[LINE_SIZE];
    float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;

    Material* material = 0;// &(materialMap[""]);

    while (fin)
    {
        readline(fin,line,LINE_SIZE);
        if (line[0]=='#' || line[0]=='$')
        {
            // comment line
            // osg::notify(osg::NOTICE) <<"Comment: "<<line<<std::endl;
        }
        else if (strlen(line)>0)
        {
            if (strncmp(line,"newmtl ",7)==0)
            {
                std::string materialName(line+7);
                material = & materialMap[materialName];
                material->name = materialName;
            }
            else if (material)
            {
                if (strncmp(line,"Ka ",3)==0)
                {
                    unsigned int fieldsRead = sscanf(line+3,"%f %f %f %f", &r, &g, &b, &a);

                    if (fieldsRead==1)      material->ambient.set(r,0.0f,0.0f,1.0f);
                    else if (fieldsRead==2) material->ambient.set(r,g,0.0f,1.0f);
                    else if (fieldsRead==3) material->ambient.set(r,g,b,1.0f);
                    else if (fieldsRead==4) material->ambient.set(r,g,b,a);
                }
                else if (strncmp(line,"Kd ",3)==0)
                {
                    unsigned int fieldsRead = sscanf(line+3,"%f %f %f %f", &r, &g, &b, &a);

                    if (fieldsRead==1)      material->diffuse.set(r,0.0f,0.0f,1.0f);
                    else if (fieldsRead==2) material->diffuse.set(r,g,0.0f,1.0f);
                    else if (fieldsRead==3) material->diffuse.set(r,g,b,1.0f);
                    else if (fieldsRead==4) material->diffuse.set(r,g,b,a);
                }
                else if (strncmp(line,"Ks ",3)==0)
                {
                    unsigned int fieldsRead = sscanf(line+3,"%f %f %f %f", &r, &g, &b, &a);

                    if (fieldsRead==1)      material->specular.set(r,0.0f,0.0f,1.0f);
                    else if (fieldsRead==2) material->specular.set(r,g,0.0f,1.0f);
                    else if (fieldsRead==3) material->specular.set(r,g,b,1.0f);
                    else if (fieldsRead==4) material->specular.set(r,g,b,a);
                }
                else if (strncmp(line,"Ke ",3)==0)
                {
                    unsigned int fieldsRead = sscanf(line+3,"%f %f %f %f", &r, &g, &b, &a);

                    if (fieldsRead==1)      material->emissive.set(r,0.0f,0.0f,1.0f);
                    else if (fieldsRead==2) material->emissive.set(r,g,0.0f,1.0f);
                    else if (fieldsRead==3) material->emissive.set(r,g,b,1.0f);
                    else if (fieldsRead==4) material->emissive.set(r,g,b,a);
                }
                else if (strncmp(line,"Tf ",3)==0)
                {
                    unsigned int fieldsRead = sscanf(line+3,"%f %f %f %f", &r, &g, &b, &a);

                    if (fieldsRead==1)      material->Tf.set(r,0.0f,0.0f,1.0f);
                    else if (fieldsRead==2) material->Tf.set(r,g,0.0f,1.0f);
                    else if (fieldsRead==3) material->Tf.set(r,g,b,1.0f);
                    else if (fieldsRead==4) material->Tf.set(r,g,b,a);
                }
                else if (strncmp(line,"sharpness ",10)==0)
                {
                    float sharpness = 0.0f;
                    unsigned int fieldsRead = sscanf(line+10,"%f", &sharpness);

                    if (fieldsRead==1) material->sharpness = sharpness;
                }
                else if (strncmp(line,"illum ",6)==0)
                {
                    int illum = 0;
                    unsigned int fieldsRead = sscanf(line+6,"%d", &illum);

                    if (fieldsRead==1) material->illum = illum;
                }
                else if (strncmp(line,"Ns ",3)==0)
                {
                    int Ns = 0;
                    unsigned int fieldsRead = sscanf(line+3,"%d", &Ns);

                    if (fieldsRead==1) material->Ns = Ns;
                }
                else if (strncmp(line,"Ni ",3)==0)
                {
                    int Ni = 0;
                    unsigned int fieldsRead = sscanf(line+3,"%d", &Ni);

                    if (fieldsRead==1) material->Ni = Ni;
                }
                else if (strncmp(line,"illum ",6)==0)
                {
                    int illum = 0;
                    unsigned int fieldsRead = sscanf(line+6,"%d", &illum);

                    if (fieldsRead==1) material->illum = illum;
                }
                else if (strncmp(line,"Tr ",3)==0)
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
                else if (strncmp(line,"d ",2)==0)
                {
                    float alpha=1.0f;
                    unsigned int fieldsRead = sscanf(line+2,"%f", &alpha);

                    if (fieldsRead==1)
                    {
                        material->ambient[3] = alpha;
                        material->diffuse[3] = alpha;
                        material->specular[3] = alpha;
                        material->emissive[3] = alpha;
                    }
                }
                else if (strncmp(line,"map_Ka ",7)==0)
                {
                    std::string filename(line+7);
                    material->map_Ka = filename;
                }
                else if (strncmp(line,"map_Kd ",7)==0)
                {
                    std::string filename(line+7);
                    material->map_Kd = filename;
                }
                else if (strncmp(line,"map_Ks ",7)==0)
                {
                    std::string filename(line+7);
                    material->map_Ks = filename;
                }
                else if (strcmp(line,"refl")==0 || strncmp(line,"refl ",5)==0)
                {
                    material->textureReflection = true;
                }
                else
                {
                    osg::notify(osg::NOTICE) <<"*** line not handled *** :"<<line<<std::endl;
                }
            }
            else
            {
                osg::notify(osg::NOTICE) <<"*** line not handled *** :"<<line<<std::endl;
            }
        
        }

    }

    return true;
}

bool Model::readOBJ(std::istream& fin, const osgDB::ReaderWriter::Options* options)
{
    osg::notify(osg::INFO)<<"Reading OBJ file"<<std::endl;

    const int LINE_SIZE = 4096;
    char line[LINE_SIZE];
    float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

    while (fin)
    {
        readline(fin,line,LINE_SIZE);
        if (line[0]=='#' || line[0]=='$')
        {
            // comment line
            // osg::notify(osg::NOTICE) <<"Comment: "<<line<<std::endl;
        }
        else if (strlen(line)>0)
        {
            if (strncmp(line,"v ",2)==0)
            {
                unsigned int fieldsRead = sscanf(line+2,"%f %f %f %f", &x, &y, &z, &w);

                if (fieldsRead==1) vertices.push_back(osg::Vec3(x,0.0f,0.0f));
                else if (fieldsRead==2) vertices.push_back(osg::Vec3(x,y,0.0f));
                else if (fieldsRead==3) vertices.push_back(osg::Vec3(x,y,z));
                else if (fieldsRead>=4) vertices.push_back(osg::Vec3(x/w,y/w,z/w));
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

                // osg::notify(osg::NOTICE)<<"face"<<ptr<<std::endl;

                int vi=0, ti=0, ni=0;
                while(*ptr!=0)
                {
                    // skip white space
                    while(*ptr==' ') ++ptr;
                    
                    if (sscanf(ptr, "%d/%d/%d", &vi, &ti, &ni) == 3)
                    {
                        // osg::notify(osg::NOTICE)<<"   vi="<<vi<<"/ti="<<ti<<"/ni="<<ni<<std::endl;
                        element->vertexIndices.push_back(remapVertexIndex(vi));
                        element->normalIndices.push_back(remapNormalIndex(ni));
                        element->texCoordIndices.push_back(remapTexCoordIndex(ti));
                    }
                    else if (sscanf(ptr, "%d//%d", &vi, &ni) == 2)
                    {
                        // osg::notify(osg::NOTICE)<<"   vi="<<vi<<"//ni="<<ni<<std::endl;
                        element->vertexIndices.push_back(remapVertexIndex(vi));
                        element->normalIndices.push_back(remapNormalIndex(ni));
                    }
                    else if (sscanf(ptr, "%d/%d", &vi, &ti) == 2)
                    {
                        // osg::notify(osg::NOTICE)<<"   vi="<<vi<<"/ti="<<ti<<std::endl;
                        element->vertexIndices.push_back(remapVertexIndex(vi));
                        element->texCoordIndices.push_back(remapTexCoordIndex(ti));
                    }
                    else if (sscanf(ptr, "%d", &vi) == 1)
                    {
                        // osg::notify(osg::NOTICE)<<"   vi="<<vi<<std::endl;
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
                std::string materialName(line+7);
                if (currentElementState.materialName != materialName)
                {
                    currentElementState.materialName = materialName;
                    currentElementList = 0; // reset the element list to force a recompute of which ElementList to use
                }
            }
            else if (strncmp(line,"mtllib ",7)==0)
            {
            
                std::string fileName = osgDB::findDataFile( line+7, options );
                if (!fileName.empty())
                {
                    std::ifstream mfin(fileName.c_str());
                    if (mfin)
                    {
                        readMTL(mfin);
                    }
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
                osg::notify(osg::NOTICE) <<"*** line not handled *** :"<<line<<std::endl;
            }
        
        }

    }
#if 0
    osg::notify(osg::NOTICE) <<"vertices :"<<vertices.size()<<std::endl;
    osg::notify(osg::NOTICE) <<"normals :"<<normals.size()<<std::endl;
    osg::notify(osg::NOTICE) <<"texcoords :"<<texcoords.size()<<std::endl;
    osg::notify(osg::NOTICE) <<"materials :"<<materialMap.size()<<std::endl;
    osg::notify(osg::NOTICE) <<"elementStates :"<<elementStateMap.size()<<std::endl;
    
    unsigned int pos=0;
    for(ElementStateMap::iterator itr=elementStateMap.begin();
        itr!=elementStateMap.end();
        ++itr,++pos)
    {
        const ElementState& es = itr->first;
        ElementList& el = itr->second;
        osg::notify(osg::NOTICE)<<"ElementState "<<pos<<std::endl;
        osg::notify(osg::NOTICE)<<"    es.objectName="<<es.objectName<<std::endl;
        osg::notify(osg::NOTICE)<<"    es.groupName="<<es.groupName<<std::endl;
        osg::notify(osg::NOTICE)<<"    es.materialName="<<es.materialName<<std::endl;
        osg::notify(osg::NOTICE)<<"    es.smoothGroup="<<es.smoothingGroup<<std::endl;
        osg::notify(osg::NOTICE)<<"    ElementList ="<<el.size()<<std::endl;
        
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
