#include <osgSim/Sector>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkElevationRange( const osgSim::ElevationSector& sector )
{ return true; }

static bool readElevationRange( osgDB::InputStream& is, osgSim::ElevationSector& sector )
{
    float minElevation, maxElevation, fadeAngle;
    is >> minElevation >> maxElevation >> fadeAngle;
    sector.setElevationRange( minElevation, maxElevation, fadeAngle );
    return true;
}

static bool writeElevationRange( osgDB::OutputStream& os, const osgSim::ElevationSector& sector )
{
    os << sector.getMinElevation() << sector.getMaxElevation() << sector.getFadeAngle() << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgSim_ElevationSector,
                         new osgSim::ElevationSector,
                         osgSim::ElevationSector,
                         "osg::Object osgSim::Sector osgSim::ElevationSector" )
{
    ADD_USER_SERIALIZER( ElevationRange );  // ElevationRange
}
