//C++ header - Open Scene Graph Simulation - Copyright (C) 1998-2002 Robert Osfield
// Distributed under the terms of the GNU General Public License (GPL)
// as published by the Free Software Foundation.
//
// All software using osgSim must be GPL'd or excempted via the 
// purchase of the Open Scene Graph Professional License (OSGPL)
// for further information contact robert@openscenegraph.com.

#include <osgSim/LightPoint>

using namespace osgSim;

LightPoint::LightPoint():
    _on(true),
    _position(0.0f,0.0f,0.0f),
    _color(1.0f,1.0f,1.0f,1.0f),
    _intensity(1.0f),
    _radius(1.0f),
    _maxPixelSize(30),
    _sector(0),
    _blinkSequence(0)
{
}

LightPoint::LightPoint(const LightPoint& lp):
    _on(lp._on),
    _position(lp._position),
    _color(lp._color),
    _intensity(lp._intensity),
    _radius(lp._radius),
    _maxPixelSize(lp._maxPixelSize),
    _sector(lp._sector),
    _blinkSequence(lp._blinkSequence)
{
}
