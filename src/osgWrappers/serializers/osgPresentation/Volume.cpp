#include <osgPresentation/Volume>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgPresentation_Volume,
                         new osgPresentation::Volume,
                         osgPresentation::Volume,
                         "osg::Object osg::Node osg::Group osg::Transform osg::MatrixTransform osgPresentation::Group osgPresentation::Element osgPresentation::Volume" )
{
}
