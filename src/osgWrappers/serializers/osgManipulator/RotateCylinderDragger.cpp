#include <osgManipulator/RotateCylinderDragger>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgManipulator_RotateCylinderDragger,
                         new osgManipulator::RotateCylinderDragger,
                         osgManipulator::RotateCylinderDragger,
                         "osg::Object osg::Node osg::Transform osg::MatrixTransform osgManipulator::Dragger "
                         "osgManipulator::RotateCylinderDragger" )
{
    ADD_VEC4_SERIALIZER( Color, osg::Vec4() );  // _color
    ADD_VEC4_SERIALIZER( PickColor, osg::Vec4() );  // _pickColor
}
