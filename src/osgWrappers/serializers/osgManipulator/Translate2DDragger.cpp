#include <osgManipulator/Translate2DDragger>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgManipulator_Translate2DDragger,
                         new osgManipulator::Translate2DDragger,
                         osgManipulator::Translate2DDragger,
                         "osg::Object osg::Node osg::Transform osg::MatrixTransform osgManipulator::Dragger "
                         "osgManipulator::Translate2DDragger" )
{
    ADD_VEC4_SERIALIZER( Color, osg::Vec4() );  // _color
    ADD_VEC4_SERIALIZER( PickColor, osg::Vec4() );  // _pickColor
}
