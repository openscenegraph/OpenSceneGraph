#include <osgPresentation/Element>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgPresentation_Element,
                         new osgPresentation::Element,
                         osgPresentation::Element,
                         "osg::Object osg::Node osg::Group osg::Transform osg::MatrixTransform osgPresentation::Group osgPresentation::Element" )
{
}
