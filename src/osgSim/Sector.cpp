//C++ header - Open Scene Graph Simulation - Copyright (C) 1998-2002 Robert Osfield
// Distributed under the terms of the GNU General Public License (GPL)
// as published by the Free Software Foundation.
//
// All software using osgSim must be GPL'd or excempted via the 
// purchase of the Open Scene Graph Professional License (OSGPL)
// for further information contact robert@openscenegraph.com.

#include <osgSim/Sector>

using namespace osgSim;


//
// Elevation Range 
//

void AzimRange::setAzimuthRange(float minAzimuth,float maxAzimuth,float fadeAngle)
{
    // clamp the azimuth range.
    const float twoPI = 2.0f*(float)osg::PI;
    while(minAzimuth>maxAzimuth) minAzimuth -= twoPI;

    // compute the centerline
    float centerAzim = (minAzimuth+maxAzimuth)*0.5f;
    _cosAzim = cos(centerAzim);
    _sinAzim = sin(centerAzim);

    // compute the half angle range of the sector.
    float angle = (maxAzimuth-minAzimuth)*0.5f;
    _cosAngle = cos(angle);

    // clamp the fade angle to valid values.
    fadeAngle = osg::clampAbove(fadeAngle,0.0f);
    if (angle+fadeAngle>osg::PI) _cosFadeAngle = -1.0f;
    else _cosFadeAngle = cos(angle+fadeAngle);

}


//
// Elevation Range 
//
void ElevationRange::setElevationRange(float minElevation,float maxElevation,float fadeAngle)
{
    if (minElevation>maxElevation)
    {
        // need to swap angle pair.
        float tmp = minElevation;
        minElevation = maxElevation;
        maxElevation = tmp;
    }

    minElevation = osg::clampTo(minElevation,(float)-osg::PI_2,(float)osg::PI_2);
    maxElevation = osg::clampTo(maxElevation,(float)-osg::PI_2,(float)osg::PI_2);
    fadeAngle = osg::clampTo(fadeAngle,0.0f,(float)osg::PI_2);

    _cosMinElevation = cos(osg::PI_2-minElevation);
    _cosMaxElevation = cos(osg::PI_2-maxElevation);

    float minFadeAngle = osg::PI_2-minElevation+fadeAngle;
    if (minFadeAngle>=osg::PI) _cosMinFadeElevation = -1.0f;
    else _cosMinFadeElevation = cos(minFadeAngle);

    float maxFadeAngle = osg::PI_2-maxElevation-fadeAngle;
    if (maxFadeAngle<=0.0f) _cosMaxFadeElevation = 1.0f;
    else _cosMaxFadeElevation = cos(maxFadeAngle);

}

float ElevationRange::getMinElevation() const
{
    return osg::PI_2-acos(_cosMinElevation);
}

float ElevationRange::getMaxElevation() const
{
    return osg::PI_2-acos(_cosMaxElevation);
}

//
// ElevationSector
//
AzimSector::AzimSector(float minAzimuth,float maxAzimuth,float fadeAngle):
    Sector(),
    AzimRange()
{
    setAzimuthRange(minAzimuth,maxAzimuth,fadeAngle);
}

float AzimSector::operator() (const osg::Vec3& eyeLocal) const
{
    return azimSector(eyeLocal);
}

//
// ElevationSector
//
ElevationSector::ElevationSector(float minElevation,float maxElevation,float fadeAngle):
    Sector(),
    ElevationRange()
{
    setElevationRange(minElevation,maxElevation,fadeAngle);
}

float ElevationSector::operator() (const osg::Vec3& eyeLocal) const
{
    return elevationSector(eyeLocal);
}

//
// AzimElevationSector
//
AzimElevationSector::AzimElevationSector(float minAzimuth,float maxAzimuth,float minElevation,float maxElevation,float fadeAngle):
    Sector(),
    AzimRange(),
    ElevationRange()
{
    setAzimuthRange(minAzimuth,maxAzimuth,fadeAngle);
    setElevationRange(minElevation,maxElevation,fadeAngle);
}


float AzimElevationSector::operator() (const osg::Vec3& eyeLocal) const
{
    float azimIntensity = azimSector(eyeLocal);
    if (azimIntensity==0.0) return 0.0; // out of sector.
    float elevIntensity = elevationSector(eyeLocal);
    if (elevIntensity==0.0) return 0.0; // out of sector.
    if (azimIntensity<=elevIntensity) return azimIntensity;
    return elevIntensity;
}

//
// ConeSector
//
ConeSector::ConeSector(const osg::Vec3& axis,float angle,float fadeangle):
            Sector()
{
    setAxis(axis);
    setAngle(angle,fadeangle);
}

void ConeSector::setAxis(const osg::Vec3& axis)
{
    _axis = axis;
    _axis.normalize();
}

const osg::Vec3& ConeSector::getAxis() const
{
    return _axis;
}

void ConeSector::setAngle(float angle,float fadeangle)
{
    _cosAngle = cos(angle);
    _cosAngleFade = cos(angle+fadeangle);
}

float ConeSector::getAngle() const
{
    return acos(_cosAngle);
}

float ConeSector::getFadeAngle() const
{
    return acos(_cosAngleFade)-acos(_cosAngle);
}

float ConeSector::operator() (const osg::Vec3& eyeLocal) const
{
    float dotproduct = eyeLocal*_axis;
    float length = eyeLocal.length();
    if (dotproduct>_cosAngle*length) return 1.0f; // fully in sector
    if (dotproduct<_cosAngleFade*length) return 0.0f; // out of sector
    return (dotproduct-_cosAngleFade*length)/((_cosAngle-_cosAngleFade)*length);
}
