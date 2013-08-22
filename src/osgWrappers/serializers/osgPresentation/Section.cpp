#include <osgPresentation/Section>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgPresentation_Section,
                         new osgPresentation::Section,
                         osgPresentation::Section,
                         "osg::Object osg::Node osg::Group osg::Transform osg::MatrixTransform osgPresentation::Group osgPresentation::Section" )
{
}
