#include <osgSim/Sector>
#include <osg/io_utils>

#include <iostream>
#include <string>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

bool AzimSector_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool AzimSector_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy AzimSector_Proxy
(
    new osgSim::AzimSector,
    "AzimSector",
    "Object AzimSector",
    &AzimSector_readLocalData,
    &AzimSector_writeLocalData,
    osgDB::DotOsgWrapper::READ_AND_WRITE
);

bool AzimSector_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    bool iteratorAdvanced = false;
    osgSim::AzimSector &sector = static_cast<osgSim::AzimSector &>(obj);

    if (fr.matchSequence("azimuthRange %f %f %f"))
    {
        float minAzimuth;
        float maxAzimuth;
        float fadeRange;
        fr[1].getFloat(minAzimuth);
        fr[2].getFloat(maxAzimuth);
        fr[3].getFloat(fadeRange);
        fr += 4;
        sector.setAzimuthRange(minAzimuth, maxAzimuth, fadeRange);
        iteratorAdvanced = true;
    }
    return iteratorAdvanced;
}

bool AzimSector_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    float minAzimuth, maxAzimuth, fadeAngle;

    const osgSim::AzimSector &sector = static_cast<const osgSim::AzimSector &>(obj);
    sector.getAzimuthRange(minAzimuth, maxAzimuth, fadeAngle);
    fw.indent()<<"azimuthRange "<<minAzimuth<< " "<<maxAzimuth<< " "<<fadeAngle<<std::endl;
    return true;
}

/******************************************************************/

bool ElevationSector_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool ElevationSector_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy ElevationSector_Proxy
(
    new osgSim::ElevationSector,
    "ElevationSector",
    "Object ElevationSector",
    &ElevationSector_readLocalData,
    &ElevationSector_writeLocalData,
    osgDB::DotOsgWrapper::READ_AND_WRITE
);

bool ElevationSector_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    bool iteratorAdvanced = false;
    osgSim::ElevationSector &sector = static_cast<osgSim::ElevationSector &>(obj);

    if (fr.matchSequence("elevationRange %f %f %f"))
    {
        float minElevation;
        float maxElevation;
        float fadeAngle;
        
        fr[1].getFloat(minElevation);
        fr[2].getFloat(maxElevation);
        fr[3].getFloat(fadeAngle);
        fr += 4;
        sector.setElevationRange(minElevation, maxElevation, fadeAngle);
        iteratorAdvanced = true;
    }
    return iteratorAdvanced;
}

bool ElevationSector_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{

    const osgSim::ElevationSector &sector = static_cast<const osgSim::ElevationSector &>(obj);

    float minElevation = sector.getMinElevation();
    float maxElevation = sector.getMaxElevation();
    float fadeAngle = sector.getFadeAngle();
    fw.indent()<<"elevationRange "<<minElevation<< " "<<maxElevation<< " "<<fadeAngle<<std::endl;

    return true;
}

/******************************************************************/

bool AzimElevationSector_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool AzimElevationSector_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy AzimElevationSector_Proxy
(
    new osgSim::AzimElevationSector,
    "AzimElevationSector",
    "Object AzimElevationSector",
    &AzimElevationSector_readLocalData,
    &AzimElevationSector_writeLocalData,
    osgDB::DotOsgWrapper::READ_AND_WRITE
);

bool AzimElevationSector_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    bool iteratorAdvanced = false;
    osgSim::AzimElevationSector &sector = static_cast<osgSim::AzimElevationSector &>(obj);

    if (fr.matchSequence("azimuthRange %f %f %f"))
    {
        float minAzimuth;
        float maxAzimuth;
        float fadeAngle;
        fr[1].getFloat(minAzimuth);
        fr[2].getFloat(maxAzimuth);
        fr[3].getFloat(fadeAngle);
        fr += 4;
        sector.setAzimuthRange(minAzimuth, maxAzimuth, fadeAngle);
        iteratorAdvanced = true;
    }
    if (fr.matchSequence("elevationRange %f %f %f"))
    {
        float minElevation;
        float maxElevation;
        float fadeAngle;
        
        fr[1].getFloat(minElevation);
        fr[2].getFloat(maxElevation);
        fr[3].getFloat(fadeAngle);
        fr += 4;
        sector.setElevationRange(minElevation, maxElevation, fadeAngle);
        iteratorAdvanced = true;
    }
    return iteratorAdvanced;
}

bool AzimElevationSector_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{

    const osgSim::AzimElevationSector &sector = static_cast<const osgSim::AzimElevationSector &>(obj);

    float minElevation = sector.getMinElevation();
    float maxElevation = sector.getMaxElevation();
    float fadeAngle = sector.getFadeAngle();
    fw.indent()<<"elevationRange "<<minElevation<< " "<<maxElevation<< " "<<fadeAngle<<std::endl;

    float minAzimuth, maxAzimuth;
    sector.getAzimuthRange(minAzimuth, maxAzimuth, fadeAngle);
    fw.indent()<<"azimuthRange "<<minAzimuth<< " "<<maxAzimuth<< " "<<fadeAngle<<std::endl;
    return true;
}

/******************************************************************/

bool ConeSector_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool ConeSector_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy ConeSector_Proxy
(
    new osgSim::ConeSector,
    "ConeSector",
    "Object ConeSector",
    &ConeSector_readLocalData,
    &ConeSector_writeLocalData,
    osgDB::DotOsgWrapper::READ_AND_WRITE
);

bool ConeSector_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    bool iteratorAdvanced = false;
    osgSim::ConeSector &sector = static_cast<osgSim::ConeSector &>(obj);

    if (fr.matchSequence("axis %f %f %f"))
    {
        float x, y, z;

        fr[1].getFloat(x);
        fr[2].getFloat(y);
        fr[3].getFloat(z);
        fr += 4;
        sector.setAxis(osg::Vec3(x, y, z));
        iteratorAdvanced = true;
    }
    if (fr.matchSequence("angle %f %f"))
    {
        float angle;
        float fadeangle;
        fr[1].getFloat(angle);
        fr[2].getFloat(fadeangle);
        fr += 3;
        sector.setAngle(angle, fadeangle);
        iteratorAdvanced = true;
    }
    return iteratorAdvanced;
}

bool ConeSector_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgSim::ConeSector &sector = static_cast<const osgSim::ConeSector &>(obj);

    const osg::Vec3& axis = sector.getAxis();
    fw.indent()<<"axis "<<axis<<std::endl;

    float angle = sector.getAngle();
    float fadeangle = sector.getFadeAngle();
    fw.indent()<<"angle "<<angle<<" "<<fadeangle<<std::endl;
    return true;
}

/******************************************************************/

bool DirectionalSector_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool DirectionalSector_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy DirectionalSector_Proxy
(
    new osgSim::DirectionalSector,
    "DirectionalSector",
    "Object DirectionalSector",
    &DirectionalSector_readLocalData,
    &DirectionalSector_writeLocalData,
    osgDB::DotOsgWrapper::READ_AND_WRITE
);

bool DirectionalSector_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    bool iteratorAdvanced = false;
    osgSim::DirectionalSector &sector = static_cast<osgSim::DirectionalSector &>(obj);

    if (fr.matchSequence("direction %f %f %f"))
    {
        float x, y, z;

        fr[1].getFloat(x);
        fr[2].getFloat(y);
        fr[3].getFloat(z);
        fr += 4;
        sector.setDirection(osg::Vec3(x, y, z));
        iteratorAdvanced = true;
    }
    if (fr.matchSequence("angles %f %f %f %f"))
    {
        float horizangle;
        float vertangle;
        float rollangle;
        float fadeangle;
        fr[1].getFloat(horizangle);
        fr[2].getFloat(vertangle);
        fr[3].getFloat(rollangle);
        fr[4].getFloat(fadeangle);
        fr += 5;
        sector.setHorizLobeAngle(horizangle);
        sector.setVertLobeAngle(vertangle);
        sector.setLobeRollAngle(rollangle);
        sector.setFadeAngle(fadeangle);
        iteratorAdvanced = true;
    }
    return iteratorAdvanced;
}

bool DirectionalSector_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgSim::DirectionalSector &sector = static_cast<const osgSim::DirectionalSector &>(obj);

    const osg::Vec3& axis = sector.getDirection();
    fw.indent()<<"direction "<<axis<<std::endl;

    float horizangle = sector.getHorizLobeAngle();
    float vertangle = sector.getVertLobeAngle();
    float rollangle = sector.getLobeRollAngle();
    float fadeangle = sector.getFadeAngle();
    fw.indent()<<"angles "<<horizangle<<" "<<vertangle<<" "<<rollangle<<" "<<fadeangle<<std::endl;
    return true;
}

