#include <osgManipulator/TabBoxDragger>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgManipulator_TabBoxDragger,
                         new osgManipulator::TabBoxDragger,
                         osgManipulator::TabBoxDragger,
                         "osg::Object osg::Node osg::Transform osg::MatrixTransform osgManipulator::Dragger "
                         "osgManipulator::TabBoxDragger" )  // No need to contain CompositeDragger here
{
}
