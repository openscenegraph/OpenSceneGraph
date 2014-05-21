#include <osgUI/PushButton>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>


REGISTER_OBJECT_WRAPPER( PushButton,
                         new osgUI::PushButton,
                         osgUI::PushButton,
                         "osg::Object osg::Node osg::Group osgUI::Widget osgUI::PushButton" )
{
    ADD_STRING_SERIALIZER( Text, std::string());
    ADD_METHOD( pressed );
    ADD_METHOD( pressedImplementation );
    ADD_METHOD( released );
    ADD_METHOD( releasedImplementation );
}
