#include "osg/Input"
#include "osg/Registry"
#include "osg/Object"

#ifdef __sgi
using std::string;
#endif

using namespace osg;

// Will extend to handle #DEF and use
// functionality similar to Inventor,
// and add the ability to handle #include
// from within the OSG file format.
Input::Input()
{
}


Input::~Input()
{
}

Object* Input::getObjectForUniqueID(const std::string& uniqueID)
{
    UniqueIDToObjectMapping::iterator fitr = _uniqueIDToObjectMap.find(uniqueID);
    if (fitr != _uniqueIDToObjectMap.end()) return (*fitr).second;
    else return NULL;
}

void Input::regisiterUniqueIDForObject(const std::string& uniqueID,Object* obj)
{
    _uniqueIDToObjectMap[uniqueID] = obj;
}

Object* Input::readObject()
{
    return Registry::instance()->readObject(*this);
}


Object* Input::readObject(const string& fileName)
{
    return Registry::instance()->readObject(fileName);
}


Image*  Input::readImage()
{
    return Registry::instance()->readImage(*this);
}


Image*  Input::readImage(const string& fileName)
{
    return Registry::instance()->readImage(fileName);
}


Node* Input::readNode()
{
    return Registry::instance()->readNode(*this);
}


Node* Input::readNode(const string& fileName)
{
    return Registry::instance()->readNode(fileName);
}
