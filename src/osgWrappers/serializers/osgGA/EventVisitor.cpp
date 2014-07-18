#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgGA/EventVisitor>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>


REGISTER_OBJECT_WRAPPER( EventVisitor,
                         new osgGA::EventVisitor,
                         osgGA::EventVisitor,
                         "osg::Object osg::NodeVisitor osgGA::EventVisitor" )
{
}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
