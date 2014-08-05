#include <osgUI/LineEdit>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>


REGISTER_OBJECT_WRAPPER( LineEdit,
                         new osgUI::LineEdit,
                         osgUI::LineEdit,
                         "osg::Object osg::Node osg::Group osgUI::Widget osgUI::LineEdit" )
{
    ADD_OBJECT_SERIALIZER( Validator, osgUI::Validator, NULL);
    ADD_STRING_SERIALIZER( Text, std::string());
}
