#include <osgSim/Sector>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkAngle( const osgSim::ConeSector& sector )
{ return true; }

static bool readAngle( osgDB::InputStream& is, osgSim::ConeSector& sector )
{
    float angle, fadeAngle;
    is >> angle >> fadeAngle;
    sector.setAngle( angle, fadeAngle );
    return true;
}

static bool writeAngle( osgDB::OutputStream& os, const osgSim::ConeSector& sector )
{
    os << sector.getAngle() << sector.getFadeAngle() << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgSim_ConeSector,
                         new osgSim::ConeSector,
                         osgSim::ConeSector,
                         "osg::Object osgSim::Sector osgSim::ConeSector" )
{
    ADD_VEC3_SERIALIZER( Axis, osg::Vec3() );  // _axis
    ADD_USER_SERIALIZER( Angle );  // _cosAngle
}
