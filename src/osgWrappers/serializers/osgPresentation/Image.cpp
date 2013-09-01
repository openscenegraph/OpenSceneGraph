#include <osgPresentation/Image>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgPresentation_Image,
                         new osgPresentation::Image,
                         osgPresentation::Image,
                         "osg::Object osg::Node osg::Group osg::Transform osg::MatrixTransform osgPresentation::Group osgPresentation::Element osgPresentation::Image" )
{
}
