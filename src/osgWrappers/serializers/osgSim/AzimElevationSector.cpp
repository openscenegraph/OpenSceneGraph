#include <osgSim/Sector>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkAzimRange( const osgSim::AzimElevationSector& sector )
{ return true; }

static bool readAzimRange( osgDB::InputStream& is, osgSim::AzimElevationSector& sector )
{
    float minAzimuth, maxAzimuth, fadeAngle;
    is >> minAzimuth >> maxAzimuth >> fadeAngle;
    sector.setAzimuthRange( minAzimuth, maxAzimuth, fadeAngle );
    return true;
}

static bool writeAzimRange( osgDB::OutputStream& os, const osgSim::AzimElevationSector& sector )
{
    float minAzimuth, maxAzimuth, fadeAngle;
    sector.getAzimuthRange( minAzimuth, maxAzimuth, fadeAngle );
    os << minAzimuth << maxAzimuth << fadeAngle << std::endl;
    return true;
}

static bool checkElevationRange( const osgSim::AzimElevationSector& sector )
{ return true; }

static bool readElevationRange( osgDB::InputStream& is, osgSim::AzimElevationSector& sector )
{
    float minElevation, maxElevation, fadeAngle;
    is >> minElevation >> maxElevation >> fadeAngle;
    sector.setElevationRange( minElevation, maxElevation, fadeAngle );
    return true;
}

static bool writeElevationRange( osgDB::OutputStream& os, const osgSim::AzimElevationSector& sector )
{
    os << sector.getMinElevation() << sector.getMaxElevation() << sector.getFadeAngle() << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgSim_AzimElevationSector,
                         new osgSim::AzimElevationSector,
                         osgSim::AzimElevationSector,
                         "osg::Object osgSim::Sector osgSim::AzimElevationSector" )
{
    ADD_USER_SERIALIZER( AzimRange );  // AzimRange
    ADD_USER_SERIALIZER( ElevationRange );  // ElevationRange
}
