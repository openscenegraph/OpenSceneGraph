#include <osg/Object>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/Input>

using namespace osgDB;

Input::Input()
{
}


Input::~Input()
{
}


osg::Object* Input::getObjectForUniqueID(const std::string& uniqueID)
{
    UniqueIDToObjectMapping::iterator fitr = _uniqueIDToObjectMap.find(uniqueID);
    if (fitr != _uniqueIDToObjectMap.end()) return (*fitr).second;
    else return NULL;
}


void Input::regisiterUniqueIDForObject(const std::string& uniqueID,osg::Object* obj)
{
    _uniqueIDToObjectMap[uniqueID] = obj;
}


osg::Object* Input::readObjectOfType(const osg::Object& compObj)
{
    return Registry::instance()->readObjectOfType(compObj,*this);
}

osg::Object* Input::readObject()
{
    return Registry::instance()->readObject(*this);
}


osg::Image*  Input::readImage()
{
    return Registry::instance()->readImage(*this);
}

osg::Drawable* Input::readDrawable()
{
    return Registry::instance()->readDrawable(*this);
}

osg::StateAttribute* Input::readStateAttribute()
{
    return Registry::instance()->readStateAttribute(*this);
}

osg::Node* Input::readNode()
{
    return Registry::instance()->readNode(*this);
}

osg::Object* Input::readObject(const std::string& fileName)
{
    return readObjectFile(fileName);
}

osg::Image*  Input::readImage(const std::string& fileName)
{
    return readImageFile(fileName);
}

osg::Node* Input::readNode(const std::string& fileName)
{
    return readNodeFile(fileName);
}
