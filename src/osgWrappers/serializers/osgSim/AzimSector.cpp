#include <osgSim/Sector>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkAzimRange( const osgSim::AzimSector& sector )
{ return true; }

static bool readAzimRange( osgDB::InputStream& is, osgSim::AzimSector& sector )
{
    float minAzimuth, maxAzimuth, fadeAngle;
    is >> minAzimuth >> maxAzimuth >> fadeAngle;
    sector.setAzimuthRange( minAzimuth, maxAzimuth, fadeAngle );
    return true;
}

static bool writeAzimRange( osgDB::OutputStream& os, const osgSim::AzimSector& sector )
{
    float minAzimuth, maxAzimuth, fadeAngle;
    sector.getAzimuthRange( minAzimuth, maxAzimuth, fadeAngle );
    os << minAzimuth << maxAzimuth << fadeAngle << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgSim_AzimSector,
                         new osgSim::AzimSector,
                         osgSim::AzimSector,
                         "osg::Object osgSim::Sector osgSim::AzimSector" )
{
    ADD_USER_SERIALIZER( AzimRange );  // AzimRange
}
