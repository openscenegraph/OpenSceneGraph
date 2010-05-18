#include <osgSim/Sector>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgSim_DirectionalSector,
                         new osgSim::DirectionalSector,
                         osgSim::DirectionalSector,
                         "osg::Object osgSim::Sector osgSim::DirectionalSector" )
{
    ADD_VEC3_SERIALIZER( Direction, osg::Vec3() );  // _direction
    ADD_FLOAT_SERIALIZER( LobeRollAngle, 0.0f );  // _rollAngle
    ADD_FLOAT_SERIALIZER( HorizLobeAngle, -1.0f );  // _cosHorizAngle
    ADD_FLOAT_SERIALIZER( VertLobeAngle, -1.0f );  // _cosVertAngle
    ADD_FLOAT_SERIALIZER( FadeAngle, -1.0f );  // FadeAngle
}
