#include <osgDB/Output>
#include <osgDB/Registry>
#include <osgDB/FileNameUtils>

#include <osg/Notify>

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

Output::Output(const Output&) : ofstream() {}

Output& Output::operator = (const Output&) { return *this; }

Output::~Output()
{
}


void Output::init()
{
    _indent = 0;
    _indentStep = 2;
    _numIndicesPerLine = 10;
    _pathNameHint = AS_IS;
}

void Output::open(const char *name)
{
    init();
    ofstream::open(name);
    _filename = name;
}

void Output::open(const char *name,int mode)
{
    init();
    ofstream::open(name,mode);
    _filename = name;
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
    sprintf(str,"%s_%i",obj->className(),_objectToUniqueIDMap.size());
    uniqueID = str;
    return true;
}


bool Output::registerUniqueIDForObject(const osg::Object* obj,std::string& uniqueID)
{
    _objectToUniqueIDMap[obj] = uniqueID;
    return true;
}

const std::string Output::getFileNameForOutput(const std::string& filename) const
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
