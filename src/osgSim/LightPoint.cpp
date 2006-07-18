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

#include <osgSim/LightPoint>

using namespace osgSim;

LightPoint::LightPoint():
    _on(true),
    _position(0.0f,0.0f,0.0f),
    _color(1.0f,1.0f,1.0f,1.0f),
    _intensity(1.0f),
    _radius(1.0f),
    _sector(0),
    _blinkSequence(0),
    _blendingMode(BLENDED)
{
}

LightPoint::LightPoint(const osg::Vec3& position,const osg::Vec4& color):
    _on(true),
    _position(position),
    _color(color),
    _intensity(1.0f),
    _radius(1.0f),
    _sector(0),
    _blinkSequence(0),
    _blendingMode(BLENDED)
{
}    

LightPoint::LightPoint(bool                 on,
                       const osg::Vec3&     position,
                       const osg::Vec4&     color,
                       float                intensity,
                       float                radius,
                       Sector*              sector,
                       BlinkSequence*       blinkSequence,
                       BlendingMode         blendingMode):
    _on(on),
    _position(position),
    _color(color),
    _intensity(intensity),
    _radius(radius),
    _sector(sector),
    _blinkSequence(blinkSequence),
    _blendingMode(blendingMode)
{
}

LightPoint::LightPoint(const LightPoint& lp):
    _on(lp._on),
    _position(lp._position),
    _color(lp._color),
    _intensity(lp._intensity),
    _radius(lp._radius),
    _sector(lp._sector),
    _blinkSequence(lp._blinkSequence),
    _blendingMode(lp._blendingMode)
{
}

LightPoint& LightPoint::operator = (const LightPoint& lp)
{
    _on = lp._on;
    _position = lp._position;
    _color = lp._color;
    _intensity = lp._intensity;
    _radius = lp._radius;
    _sector = lp._sector;
    _blinkSequence = lp._blinkSequence;
    _blendingMode = lp._blendingMode;
    
    return *this;
}
