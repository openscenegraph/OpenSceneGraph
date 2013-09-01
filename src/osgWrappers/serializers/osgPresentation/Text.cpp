#include <osgPresentation/Text>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgPresentation_Text,
                         new osgPresentation::Text,
                         osgPresentation::Text,
                         "osg::Object osg::Node osg::Group osg::Transform osg::MatrixTransform osgPresentation::Group osgPresentation::Element osgPresentation::Text" )
{
}
