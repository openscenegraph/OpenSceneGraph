#include <osgPresentation/Group>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgPresentation_Group,
                         new osgPresentation::Group,
                         osgPresentation::Group,
                         "osg::Object osg::Node osg::Group osg::Transform osg::MatrixTransform osgPresentation::Group" )
{
}
