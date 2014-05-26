#include <osgUI/Dialog>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>


REGISTER_OBJECT_WRAPPER( Dialog,
                         new osgUI::Dialog,
                         osgUI::Dialog,
                         "osg::Object osg::Node osg::Group osgUI::Widget osgUI::Dialog" )
{
    ADD_STRING_SERIALIZER( Title, std::string());
}
