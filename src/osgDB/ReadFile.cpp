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
    ReaderWriter::ReadResult rr = Registry::instance()->readObject(filename);
    if (rr.validObject()) return rr.takeObject();
    if (rr.error()) notify(WARN) << rr.message() << endl;
    return NULL;
}


Image* osgDB::readImageFile(const std::string& filename)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readImage(filename);
    if (rr.validImage()) return rr.takeImage();
    if (rr.error()) notify(WARN) << rr.message() << endl;
    return NULL;
}


Node* osgDB::readNodeFile(const std::string& filename)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readNode(filename);
    if (rr.validNode()) return rr.takeNode();
    if (rr.error()) notify(WARN) << rr.message() << endl;
    return NULL;
}
