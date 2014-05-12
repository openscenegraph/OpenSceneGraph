#include <osgUI/Label>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>


REGISTER_OBJECT_WRAPPER( Label,
                         new osgUI::Label,
                         osgUI::Label,
                         "osg::Object osg::Node osg::Group osgUI::Widget osgUI::Label" )
{
    ADD_STRING_SERIALIZER( Text, std::string());

}
