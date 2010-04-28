#include <osgManipulator/TranslateAxisDragger>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgManipulator_TranslateAxisDragger,
                         new osgManipulator::TranslateAxisDragger,
                         osgManipulator::TranslateAxisDragger,
                         "osg::Object osg::Node osg::Transform osg::MatrixTransform osgManipulator::Dragger "
                         "osgManipulator::TranslateAxisDragger" )  // No need to contain CompositeDragger here
{
}
