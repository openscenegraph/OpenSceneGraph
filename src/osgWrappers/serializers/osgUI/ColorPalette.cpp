#include <osgUI/ColorPalette>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>


REGISTER_OBJECT_WRAPPER( ColorPalette,
                         new osgUI::ColorPalette,
                         osgUI::ColorPalette,
                         "osg::Object osgUI::ColorPalette" )
{
    ADD_VECTOR_SERIALIZER( Colors, osgUI::ColorPalette::Colors, osgDB::BaseSerializer::RW_VEC4F, 4 );
    ADD_VECTOR_SERIALIZER( Names, osgUI::ColorPalette::Names, osgDB::BaseSerializer::RW_STRING, 1 );
}
