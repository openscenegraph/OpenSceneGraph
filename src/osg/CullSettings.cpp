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
#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>

#include <osg/Notify>

using namespace osg;

CullSettings::CullSettings(const CullSettings& cs)
{
    setCullSettings(cs);
}

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
    //_computeNearFar = COMPUTE_NEAR_FAR_USING_PRIMITIVES;
    //_nearFarRatio = 0.00005f;
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

static ApplicationUsageProxy ApplicationUsageProxyCullSettings_e0(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_COMPUTE_NEAR_FAR_MODE <mode>","DO_NOT_COMPUTE_NEAR_FAR | COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES | COMPUTE_NEAR_FAR_USING_PRIMITIVES");
static ApplicationUsageProxy ApplicationUsageProxyCullSettings_e1(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_NEAR_FAR_RATIO <float>","Set the ratio between near and far planes - must greater than 0.0 but less than 1.0.");

void CullSettings::readEnvironmentalVariables()
{
    osg::notify(osg::INFO)<<"CullSettings::readEnvironmentalVariables()"<<std::endl;

    char *ptr;
    
    if ((ptr = getenv("OSG_COMPUTE_NEAR_FAR_MODE")) != 0)
    {
        if (strcmp(ptr,"DO_NOT_COMPUTE_NEAR_FAR")==0) _computeNearFar = DO_NOT_COMPUTE_NEAR_FAR;
        else if (strcmp(ptr,"COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES")==0) _computeNearFar = COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES;
        else if (strcmp(ptr,"COMPUTE_NEAR_FAR_USING_PRIMITIVES")==0) _computeNearFar = COMPUTE_NEAR_FAR_USING_PRIMITIVES;

        osg::notify(osg::INFO)<<"Set compute near far mode to "<<_computeNearFar<<std::endl;
        
    }
    
    if ((ptr = getenv("OSG_NEAR_FAR_RATIO")) != 0)
    {
        _nearFarRatio = atof(ptr);

        osg::notify(osg::INFO)<<"Set near/far ratio to "<<_nearFarRatio<<std::endl;
    }
    
}

void CullSettings::readCommandLine(ArgumentParser& arguments)
{
    // report the usage options.
    if (arguments.getApplicationUsage())
    {
        arguments.getApplicationUsage()->addCommandLineOption("--COMPUTE_NEAR_FAR_MODE <mode>","DO_NOT_COMPUTE_NEAR_FAR | COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES | COMPUTE_NEAR_FAR_USING_PRIMITIVES");
        arguments.getApplicationUsage()->addCommandLineOption("--NEAR_FAR_RATIO <float>","Set the ratio between near and far planes - must greater than 0.0 but less than 1.0.");
    }

    std::string str;
    while(arguments.read("--COMPUTE_NEAR_FAR_MODE",str))
    {
        if (str=="DO_NOT_COMPUTE_NEAR_FAR") _computeNearFar = DO_NOT_COMPUTE_NEAR_FAR;
        else if (str=="COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES") _computeNearFar = COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES;
        else if (str=="COMPUTE_NEAR_FAR_USING_PRIMITIVES") _computeNearFar = COMPUTE_NEAR_FAR_USING_PRIMITIVES;

        osg::notify(osg::INFO)<<"Set compute near far mode to "<<_computeNearFar<<std::endl;
    }

    double value;
    while(arguments.read("--NEAR_FAR_RATIO",value))
    {
        _nearFarRatio = value;

        osg::notify(osg::INFO)<<"Set near/far ratio to "<<_nearFarRatio<<std::endl;
    }

}
