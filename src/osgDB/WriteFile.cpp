
#include "osg/Notify"
#include "osg/Object"
#include "osg/Image"
#include "osg/Node"
#include "osg/Group"
#include "osg/Geode"

#include "osgDB/Registry"
#include "osgDB/WriteFile"

using namespace osg;
using namespace osgDB;

bool osgDB::writeObjectFile(const Object& object,const std::string& filename)
{
    return Registry::instance()->writeObject(object,filename);
}


bool osgDB::writeImageFile(const Image& image,const std::string& filename)
{
    return Registry::instance()->writeImage(image,filename);
}


bool osgDB::writeNodeFile(const Node& node,const std::string& filename)
{
    return Registry::instance()->writeNode(node,filename);
}
