/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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

#include <osg/CullSettings>

using namespace osg;

void CullSettings::setDefaults()
{
    _cullingMode = DEFAULT_CULLING;
    _LODScale = 1.0f;
    _smallFeatureCullingPixelSize = 2.0f;

    _computeNearFar = COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES;
    _nearFarRatio = 0.0005f;
    _impostorActive = true;
    _depthSortImpostorSprites = false;
    _impostorPixelErrorThreshold = 4.0f;
    _numFramesToKeepImpostorSprites = 10;
    _cullMask = 0xffffffff;
    _cullMaskLeft = 0xffffffff;
    _cullMaskRight = 0xffffffff;

    // override during testing
    _computeNearFar = COMPUTE_NEAR_FAR_USING_PRIMITIVES;
    _nearFarRatio = 0.0005f;
}

void CullSettings::setCullSettings(const CullSettings& settings)
{
    _computeNearFar = settings._computeNearFar;
    _nearFarRatio = settings._nearFarRatio;
    _impostorActive = settings._impostorActive;
    _depthSortImpostorSprites = settings._depthSortImpostorSprites;
    _impostorPixelErrorThreshold = settings._impostorPixelErrorThreshold;
    _numFramesToKeepImpostorSprites = settings._numFramesToKeepImpostorSprites;
    _cullMask = settings._cullMask;
    _cullMaskLeft = settings._cullMaskLeft;
    _cullMaskRight = settings._cullMaskRight;
    _cullingMode = settings._cullingMode;
    _LODScale = settings._LODScale;
    _smallFeatureCullingPixelSize = settings._smallFeatureCullingPixelSize;
}
