#include <osgUI/Validator>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#include <limits.h>

REGISTER_OBJECT_WRAPPER( IntValidator,
                         new osgUI::IntValidator,
                         osgUI::IntValidator,
                         "osg::Object osgUI::Validator osgUI::IntValidator" )
{
    ADD_INT_SERIALIZER(Bottom, -INT_MAX);
    ADD_INT_SERIALIZER(Top, INT_MAX);
}
