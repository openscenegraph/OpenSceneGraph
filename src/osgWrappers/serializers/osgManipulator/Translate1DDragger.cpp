#include <osgManipulator/Translate1DDragger>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgManipulator_Translate1DDragger,
                         new osgManipulator::Translate1DDragger,
                         osgManipulator::Translate1DDragger,
                         "osg::Object osg::Node osg::Transform osg::MatrixTransform osgManipulator::Dragger "
                         "osgManipulator::Translate1DDragger" )
{
    ADD_VEC4_SERIALIZER( Color, osg::Vec4() );  // _color
    ADD_VEC4_SERIALIZER( PickColor, osg::Vec4() );  // _pickColor
}
