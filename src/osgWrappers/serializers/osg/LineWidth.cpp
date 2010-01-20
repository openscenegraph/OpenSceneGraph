#include <osg/LineWidth>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( LineWidth,
                         new osg::LineWidth,
                         osg::LineWidth,
                         "osg::Object osg::StateAttribute osg::LineWidth" )
{
    ADD_FLOAT_SERIALIZER( Width, 1.0f );  // _width
}
