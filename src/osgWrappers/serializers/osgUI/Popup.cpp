#include <osgUI/Popup>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>


REGISTER_OBJECT_WRAPPER( Popup,
                         new osgUI::Popup,
                         osgUI::Popup,
                         "osg::Object osg::Node osg::Group osgUI::Widget osgUI::Popup" )
{
}
