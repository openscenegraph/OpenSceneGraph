
#include <osg/Notify>
#include <osg/Object>
#include <osg/Image>
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>

#include <osgDB/Registry>
#include <osgDB/WriteFile>

using namespace osg;
using namespace osgDB;

bool osgDB::writeObjectFile(const Object& object,const std::string& filename)
{
    ReaderWriter::WriteResult wr = Registry::instance()->writeObject(object,filename);
    if (wr.error()) notify(WARN) << wr.message() << std::endl;
    return wr.success();
}


bool osgDB::writeImageFile(const Image& image,const std::string& filename)
{
    ReaderWriter::WriteResult wr = Registry::instance()->writeImage(image,filename);
    if (wr.error()) notify(WARN) << wr.message() << std::endl;
    return wr.success();
}


bool osgDB::writeNodeFile(const Node& node,const std::string& filename)
{
    ReaderWriter::WriteResult wr = Registry::instance()->writeNode(node,filename);
    if (wr.error()) notify(WARN) << wr.message() << std::endl;
    return wr.success();
}
