#include <osgPresentation/Model>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgPresentation_Model,
                         new osgPresentation::Model,
                         osgPresentation::Model,
                         "osg::Object osg::Node osg::Group osg::Transform osg::MatrixTransform osgPresentation::Group osgPresentation::Element osgPresentation::Model" )
{
}
