#include <stdio.h>

#include "osg/Output"
#include "osg/Object"

using namespace osg;

Output::Output()
{
    _init();
}


Output::~Output()
{
    _free();
}

void Output::_init()
{
    _indent = 0;
    _indentStep = 2;
    _numIndicesPerLine = 10; 
}


void Output::_free()
{
// should I be calling ofstream's free as well???
}


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

bool Output::getUniqueIDForObject(Object* obj,std::string& uniqueID)
{
    UniqueIDToLabelMapping::iterator fitr = _objectToUniqueIDMap.find(obj);
    if (fitr != _objectToUniqueIDMap.end())
    {
         uniqueID = (*fitr).second;
         return true;
    }
    else return false;
}

bool Output::createUniqueIDForObject(Object* obj,std::string& uniqueID)
{
    char str[256];
    sprintf(str,"%s_%i",obj->className(),_objectToUniqueIDMap.size());
    uniqueID = str;
    return true;  
}

bool Output::registerUniqueIDForObject(Object* obj,std::string& uniqueID)
{
    _objectToUniqueIDMap[obj] = uniqueID;
    return true;
}

