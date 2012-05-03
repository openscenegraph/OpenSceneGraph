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
#include <string.h>

#include <osg/GL>
#include <osg/PolygonOffset>
#include <osg/Notify>

using namespace osg;

static float s_FactorMultipler = 1.0f;
static float s_UnitsMultipler = 1.0f;
static bool s_MultiplerSet = false;

void PolygonOffset::setFactorMultiplier(float multiplier)
{
    s_MultiplerSet = true;
    s_FactorMultipler = multiplier;
}

float PolygonOffset::getFactorMultiplier()
{
    return s_FactorMultipler;
}

void PolygonOffset::setUnitsMultiplier(float multiplier)
{
    s_MultiplerSet = true;
    s_UnitsMultipler = multiplier;
}

float PolygonOffset::getUnitsMultiplier()
{
    return s_UnitsMultipler;
}

bool PolygonOffset::areFactorAndUnitsMultipliersSet()
{
    return s_MultiplerSet;
}


void PolygonOffset::setFactorAndUnitsMultipliersUsingBestGuessForDriver()
{
    s_MultiplerSet = true;
    // OSG_NOTICE<<"PolygonOffset::setFactorAndUnitMultipliersUsingBestGuessForDriver()"<<std::endl;

#if 0
    const GLubyte* renderer = glGetString(GL_RENDERER);
    if (renderer)
    {
        if ((strstr((const char*)renderer,"Radeon")!=0) ||
            (strstr((const char*)renderer,"RADEON")!=0) ||
            (strstr((const char*)renderer,"ALL-IN-WONDER")!=0))
        {
            setFactorMultiplier(1.0f);
            setUnitsMultiplier(128.0f);
            OSG_INFO<<"PolygonOffset::setFactorAndUnitsMultipliersUsingBestGuessForDriver() apply ATI workaround."<<std::endl;
        }
    }
#endif
}


PolygonOffset::PolygonOffset():
    _factor(0.0f),
    _units(0.0f)
{
}

PolygonOffset::PolygonOffset(float factor, float units):
    _factor(factor),
    _units(units)
{
}

PolygonOffset::~PolygonOffset()
{
}

void PolygonOffset::apply(State&) const
{
    if (!s_MultiplerSet) setFactorAndUnitsMultipliersUsingBestGuessForDriver();

    glPolygonOffset(_factor * s_FactorMultipler,
                    _units * s_UnitsMultipler);
}
