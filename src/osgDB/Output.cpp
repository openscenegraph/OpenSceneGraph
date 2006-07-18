/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
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
#include <osgDB/Output>
#include <osgDB/Registry>
#include <osgDB/FileNameUtils>

#include <osg/Notify>

#include <sstream>
#include <stdio.h>

using namespace std;
using namespace osgDB;

Output::Output()
{
    init();
}

Output::Output(const char* name) : ofstream(name)
{
    init();
    _filename = name;
}

Output::~Output()
{
}


void Output::init()
{
    _indent = 0;
    _indentStep = 2;
    _numIndicesPerLine = 10;
    _pathNameHint = AS_IS;
    _outputTextureFiles = false;
    _textureFileNameNumber = 0;
}

void Output::open(const char *name)
{
    init();
    ofstream::open(name);
    _filename = name;
}

// Comment out to avoid compile errors under new compilers, the int mode
// is now a replaced by a class to wrap the mode.  
// This method is not used right now to hopefully nobody will miss it... 
// Jan 2002.
// void Output::open(const char *name,int mode)
// {
//     init();
//     ofstream::open(name,mode);
//     _filename = name;
// }

Output& Output::indent()
{
    for(int i=0;i<_indent;++i) *this<<' ';
    return *this;
}



void Output::moveIn()
{
    _indent += _indentStep;
}


void Output::moveOut()
{
    _indent -= _indentStep;
    if (_indent<0) _indent=0;
}

std::string Output::wrapString(const std::string& str)
{
    std::string newstring;
    newstring += '"';
    for(unsigned int i=0;i<str.size();++i)
    {
        if (str[i]=='"')
        {
            newstring += '\\';
            newstring += '"';
        }
        else newstring += (str[i]);
    }
    newstring += '"';
    return newstring;
}

bool Output::writeObject(const osg::Object& obj)
{
    return Registry::instance()->writeObject(obj,*this);
}

bool Output::getUniqueIDForObject(const osg::Object* obj,std::string& uniqueID)
{
    UniqueIDToLabelMapping::iterator fitr = _objectToUniqueIDMap.find(obj);
    if (fitr != _objectToUniqueIDMap.end())
    {
        uniqueID = (*fitr).second;
        return true;
    }
    else return false;
}


bool Output::createUniqueIDForObject(const osg::Object* obj,std::string& uniqueID)
{
    char str[256];
    sprintf(str,"%s_%i",obj->className(),(unsigned int)_objectToUniqueIDMap.size());
    uniqueID = str;
    return true;
}


bool Output::registerUniqueIDForObject(const osg::Object* obj,std::string& uniqueID)
{
    _objectToUniqueIDMap[obj] = uniqueID;
    return true;
}

std::string Output::getFileNameForOutput(const std::string& filename) const
{
    switch(_pathNameHint)
    {
    case(FULL_PATH):
        {
            // need to think about how best to implement this first...
            osg::notify(osg::WARN)<<"Warning: Output::getFileNameForOutput() does not support FULL_PATH yet."<< std::endl;        
            return filename;
        }
    case(RELATIVE_PATH):
        {
            // need to think about how best to implement this as well...
            osg::notify(osg::WARN)<<"Warning: Output::getFileNameForOutput() does not support RELATIVE_PATH yet."<< std::endl;        
            return filename;
        }
    case(FILENAME_ONLY):
        // this one is straight forward.
        return getSimpleFileName(filename);
    case(AS_IS):
    default:
        // and this one is even more trivial.
        return filename;
    }
}

std::string Output::getTextureFileNameForOutput()
{
    std::string fileName = osgDB::getNameLessExtension(_filename);
    if (_textureFileNameNumber>0)
    {
        std::ostringstream o;
        o << '_' << _textureFileNameNumber;
        fileName += o.str();
    }
    
    fileName += ".dds";
    ++_textureFileNameNumber;
    
    return fileName;
}

