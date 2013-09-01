#include <osgPresentation/Movie>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgPresentation_Movie,
                         new osgPresentation::Movie,
                         osgPresentation::Movie,
                         "osg::Object osg::Node osg::Group osg::Transform osg::MatrixTransform osgPresentation::Group osgPresentation::Element osgPresentation::Movie" )
{
}
