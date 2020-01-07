/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include <osgSim/Sector>
#include <osg/Vec2>

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

void AzimRange::getAzimuthRange(float& minAzimuth, float& maxAzimuth, float& fadeAngle) const
{
    float centerAzim = atan2(_sinAzim, _cosAzim);
    float angle = acos(_cosAngle);
    minAzimuth = centerAzim - angle;
    maxAzimuth = centerAzim + angle;
    if (_cosFadeAngle == -1.0f) {
        fadeAngle = 2.0f * osg::PI;
    } else {
        fadeAngle = acos(_cosFadeAngle) - angle;
    }
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

float ElevationRange::getFadeAngle() const
{
    float fadeAngle = 0.0;

    // Take the appropriate (unclipped) elevation angle to calculate the fade angle
    if (_cosMinFadeElevation != -1.0f) {
        float minFadeAngle = acos(_cosMinFadeElevation);
        float minElevation = osg::PI_2 - acos(_cosMinElevation);
        fadeAngle = minFadeAngle + minElevation - osg::PI_2;

    } else if (_cosMaxFadeElevation != 1.0f) {
        float maxFadeAngle = acos(_cosMaxFadeElevation);
        float maxElevation = osg::PI_2 - acos(_cosMaxElevation);
        fadeAngle = osg::PI_2 - maxFadeAngle - maxElevation;
    }

    return fadeAngle;
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

//
// DirectionalSector
//
DirectionalSector::DirectionalSector(const osg::Vec3& direction, float horizLobeAngle, float vertLobeAngle, float lobeRollAngle, float fadeAngle):
            Sector()
{
    _direction = direction;
    _cosHorizAngle = cos(horizLobeAngle*0.5);
    _cosVertAngle = cos(vertLobeAngle*0.5);
    _rollAngle = lobeRollAngle;

    setFadeAngle(fadeAngle);

    computeMatrix();
}

void DirectionalSector::computeMatrix()
{
  double heading = atan2(_direction[0], _direction[1]);
  double pitch   = atan2(_direction[2], sqrt(_direction[0]*_direction[0] + _direction[1]*_direction[1]));
  double roll    = _rollAngle;

  _local_to_LP.setRotate(osg::Quat(heading,osg::Vec3d(0.0, 0.0, -1.0)));
  _local_to_LP.preMultRotate(osg::Quat(pitch, osg::Vec3d(1.0, 0.0, 0.0)));
  _local_to_LP.preMultRotate(osg::Quat(roll, osg::Vec3d(0.0, 1.0, 0.0)));
}

void DirectionalSector::setDirection(const osg::Vec3& direction)
{
   _direction = direction ;
   computeMatrix() ;
}

const osg::Vec3& DirectionalSector::getDirection() const
{
    return _direction;
}

void DirectionalSector::setHorizLobeAngle(float angle)
{
    _cosHorizAngle = cos(angle*0.5);
}

float DirectionalSector::getHorizLobeAngle() const
{
    return acos(_cosHorizAngle)*2.0;
}

void DirectionalSector::setVertLobeAngle(float angle)
{
    _cosVertAngle = cos(angle*0.5);
}

float DirectionalSector::getVertLobeAngle() const
{
    return acos(_cosVertAngle)*2.0;
}

void DirectionalSector::setLobeRollAngle(float angle)
{
    _rollAngle = angle ;
    computeMatrix() ;
}

float DirectionalSector::getLobeRollAngle() const
{
    return _rollAngle ;
}

void DirectionalSector::setFadeAngle(float angle)
{
    float ang = acos(_cosHorizAngle)+angle ;
    if ( ang > osg::PI ) _cosHorizFadeAngle = -1.0 ;
    else _cosHorizFadeAngle = cos(ang);

    ang = acos(_cosVertAngle)+angle ;
    if ( ang > osg::PI ) _cosVertFadeAngle = -1.0 ;
    else _cosVertFadeAngle = cos(ang);
}

float DirectionalSector::getFadeAngle() const
{
    return acos(_cosHorizFadeAngle)-acos(_cosHorizAngle);
}

float DirectionalSector::operator() (const osg::Vec3& eyeLocal) const
{
   float elev_intensity, azim_intensity ;

   // Transform eyeLocal into the LightPoint frame
   osg::Vec3 EPlp = _local_to_LP * eyeLocal ;

   /*fprintf(stderr, "    eyeLocal = %f, %f, %f\n", eyeLocal[0], eyeLocal[1], eyeLocal[2]) ;
   fprintf(stderr, "    EPlp     = %f, %f, %f\n", EPlp[0], EPlp[1], EPlp[2]) ;*/

   // Elevation check
     // Project EPlp into LP YZ plane and dot with LPy
   osg::Vec2 EPyz(EPlp[1], EPlp[2]) ;
   EPyz.normalize() ;
   /*fprintf(stderr, "    EPyz.normalize() = %f, %f\n", EPyz[0], EPyz[1]) ;
   fprintf(stderr, "        _cosVertFadeAngle = %f\n", _cosVertFadeAngle) ;
   fprintf(stderr, "        _cosVertAngle     = %f\n", _cosVertAngle) ;*/
      // cosElev = EPyz* LPy = EPyz[0]
   if ( EPyz[0] < _cosVertFadeAngle ) {
      // Completely outside elevation range
      //fprintf(stderr, "   >> outside el range\n") ;
      return(0.0f) ;
   }
   if ( EPyz[0] < _cosVertAngle ) {
      // In the fade range
      //fprintf(stderr, "   >> inside el fade range\n") ;
      elev_intensity = (EPyz[0]-_cosVertFadeAngle)/(_cosVertAngle-_cosVertFadeAngle) ;
   } else {
      // Fully in elevation range
      elev_intensity = 1.0 ;
      //fprintf(stderr, "   >> fully inside el range\n") ;
   }
   // Elevation check passed

   // Azimuth check
     // Project EPlp into LP XY plane and dot with LPy
   osg::Vec2 EPxy(EPlp[0], EPlp[1]) ;
   EPxy.normalize() ;
   /*fprintf(stderr, "    EPxy.normalize() = %f, %f\n", EPxy[0], EPxy[1]) ;
   fprintf(stderr, "        _cosHorizFadeAngle = %f\n", _cosHorizFadeAngle) ;
   fprintf(stderr, "        _cosHorizAngle     = %f\n", _cosHorizAngle) ;*/
      // cosAzim = EPxy * LPy = EPxy[1]
      // if cosElev < 0.0, then need to negate EP for azimuth check
   if ( EPyz[0] < 0.0 ) EPxy.set(-EPxy[0], -EPxy[1]) ;
   if ( EPxy[1] < _cosHorizFadeAngle ) {
      // Completely outside azimuth range
      //fprintf(stderr, "   >> outside az range\n") ;
      return(0.0f) ;
   }
   if ( EPxy[1] < _cosHorizAngle ) {
      // In fade range
      //fprintf(stderr, "   >> inside az fade range\n") ;
      azim_intensity = (EPxy[1]-_cosHorizFadeAngle)/(_cosHorizAngle-_cosHorizFadeAngle) ;
   } else {
      // Fully in azimuth range
      //fprintf(stderr, "   >> fully inside az range\n") ;
      azim_intensity = 1.0 ;
   }
   // Azimuth check passed

   // We're good! Return full intensity
   //fprintf(stderr, "   %%%% Returning intensity = %f\n", elev_intensity * azim_intensity) ;
   return elev_intensity * azim_intensity ;
}
