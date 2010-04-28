#include <osgManipulator/ScaleAxisDragger>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgManipulator_ScaleAxisDragger,
                         new osgManipulator::ScaleAxisDragger,
                         osgManipulator::ScaleAxisDragger,
                         "osg::Object osg::Node osg::Transform osg::MatrixTransform osgManipulator::Dragger "
                         "osgManipulator::ScaleAxisDragger" )  // No need to contain CompositeDragger here
{
}
