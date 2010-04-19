#include <osg/LineStipple>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( LineStipple,
                         new osg::LineStipple,
                         osg::LineStipple,
                         "osg::Object osg::StateAttribute osg::LineStipple" )
{
    ADD_GLINT_SERIALIZER( Factor, 1 );  // _factor
    ADD_HEXSHORT_SERIALIZER( Pattern, 0xffff );  // _pattern
}
