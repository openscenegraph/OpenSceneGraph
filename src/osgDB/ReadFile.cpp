#include <osg/Notify>
#include <osg/Object>
#include <osg/Image>
#include <osg/Node>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

using namespace osg;
using namespace osgDB;

Object* osgDB::readObjectFile(const std::string& filename)
{
    return Registry::instance()->readObject(filename);
}


Image*  osgDB::readImageFile(const std::string& filename)
{
    return Registry::instance()->readImage(filename);
}


Node*   osgDB::readNodeFile(const std::string& filename)
{
    return Registry::instance()->readNode(filename);
}
