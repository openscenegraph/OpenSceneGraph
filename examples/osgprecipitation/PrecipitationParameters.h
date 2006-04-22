/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield 
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

#ifndef OSGPARTICLE_PRECIPITATIONPARAMETERS
#define OSGPARTICLE_PRECIPITATIONPARAMETERS

#include <osgParticle/Export>

namespace osgParticle
{

    struct PrecipitationParameters : public osg::Referenced
    {
        PrecipitationParameters():
            particleVelocity(0.0,0.0,-5.0),
            particleSize(0.02),
            particleColour(0.6, 0.6, 0.6, 1.0),
            particleDensity(8.0),
            cellSizeX(10.0f),
            cellSizeY(10.0f),
            cellSizeZ(10.0f),
            nearTransition(25.0),
            farTransition(100.0),
            fogExponent(1.0),
            fogDensity(0.001),
            fogEnd(1000.0),
            fogColour(0.5, 0.5, 0.5, 1.0),
            clearColour(0.5, 0.5, 0.5, 1.0),
            useFarLineSegments(false)
        {
            rain(0.5);
        }

        void rain (float intensity)
        {
            particleVelocity = osg::Vec3(0.0,0.0,-2.0) + osg::Vec3(0.0,0.0,-10.0)*intensity;
            particleSize = 0.01 + 0.02*intensity;
            particleColour = osg::Vec4(0.6, 0.6, 0.6, 1.0) -  osg::Vec4(0.1, 0.1, 0.1, 1.0)* intensity;
            particleDensity = intensity * 8.5f;
            cellSizeX = 5.0f / (0.25f+intensity);
            cellSizeY = 5.0f / (0.25f+intensity);
            cellSizeZ = 5.0f;
            farTransition = 100.0f - 60.0f*sqrtf(intensity);
            fogExponent = 1.0f;
            fogDensity = 0.005f*intensity;
            fogEnd = 250/(0.01 + intensity);
            fogColour.set(0.5, 0.5, 0.5, 1.0);
            clearColour.set(0.5, 0.5, 0.5, 1.0);
            useFarLineSegments = false;
        }

        void snow(float intensity)
        {
            particleVelocity = osg::Vec3(0.0,0.0,-0.75) + osg::Vec3(0.0,0.0,-0.25)*intensity;
            particleSize = 0.02 + 0.03*intensity;
            particleColour = osg::Vec4(0.85f, 0.85f, 0.85f, 1.0f) -  osg::Vec4(0.1f, 0.1f, 0.1f, 1.0f)* intensity;
            particleDensity = intensity * 8.2f;
            cellSizeX = 5.0f / (0.25f+intensity);
            cellSizeY = 5.0f / (0.25f+intensity);
            cellSizeZ = 5.0f;
            farTransition = 100.0f - 60.0f*sqrtf(intensity);
            fogExponent = 1.0f;
            fogDensity = 0.02f*intensity;
            fogEnd = 150.0f/(0.01f + intensity);
            fogColour.set(0.6, 0.6, 0.6, 1.0);
            clearColour.set(0.6, 0.6, 0.6, 1.0);
            useFarLineSegments = false;
        }

        osg::BoundingBox    boundingBox;
        osg::Vec3           particleVelocity;
        float               particleSize;
        osg::Vec4           particleColour;
        float               particleDensity;
        float               cellSizeX;
        float               cellSizeY;
        float               cellSizeZ;
        float               nearTransition;
        float               farTransition;
        float               fogExponent;
        float               fogDensity;
        float               fogEnd;
        osg::Vec4           fogColour;
        osg::Vec4           clearColour;
        bool                useFarLineSegments;
    };

}

#endif
