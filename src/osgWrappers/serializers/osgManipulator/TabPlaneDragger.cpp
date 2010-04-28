#include <osgManipulator/TabPlaneDragger>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgManipulator_TabPlaneDragger,
                         new osgManipulator::TabPlaneDragger,
                         osgManipulator::TabPlaneDragger,
                         "osg::Object osg::Node osg::Transform osg::MatrixTransform osgManipulator::Dragger "
                         "osgManipulator::TabPlaneDragger" )  // No need to contain CompositeDragger here
{
}
