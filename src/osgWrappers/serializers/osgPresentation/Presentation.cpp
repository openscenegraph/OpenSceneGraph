#include <osgPresentation/Presentation>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgPresentation_Presentation,
                         new osgPresentation::Presentation,
                         osgPresentation::Presentation,
                         "osg::Object osg::Node osg::Group osg::Transform osg::MatrixTransform osgPresentation::Group osgPresentation::Presentation" )
{
}
