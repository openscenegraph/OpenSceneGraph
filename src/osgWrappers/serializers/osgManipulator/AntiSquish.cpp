#include <osgManipulator/AntiSquish>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgManipulator_AntiSquish,
                         new osgManipulator::AntiSquish,
                         osgManipulator::AntiSquish,
                         "osg::Object osg::Node osg::Group osg::Transform osg::MatrixTransform osgManipulator::AntiSquish" )
{
    ADD_VEC3D_SERIALIZER( Pivot, osg::Vec3d() );  // _pivot
    ADD_VEC3D_SERIALIZER( Position, osg::Vec3d() );  // _position
}
