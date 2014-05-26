#include <osgUI/ComboBox>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>


REGISTER_OBJECT_WRAPPER( Item,
                         new osgUI::Item,
                         osgUI::Item,
                         "osg::Object osgUI::Item" )
{
    ADD_STRING_SERIALIZER(Text, "");
    ADD_VEC4F_SERIALIZER(Color, osg::Vec4(1.0f,1.0f,1.0f,0.0f));
}
