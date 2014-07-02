/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2009 Robert Osfield
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

#include <osgVolume/VolumeSettings>

using namespace osgVolume;

VolumeSettings::VolumeSettings():
    _technique(MultiPass),
    _shadingModel(Standard),
    _sampleRatio(1.0f),
    _sampleRatioWhenMoving(1.0f),
    _cutoff(0.0f),
    _transparency(1.0f)
{
}

VolumeSettings::VolumeSettings(const VolumeSettings& vs,const osg::CopyOp& copyop):
    osg::Object(vs, copyop),
    _technique(vs._technique),
    _shadingModel(vs._shadingModel),
    _sampleRatio(vs._sampleRatio),
    _sampleRatioWhenMoving(vs._sampleRatioWhenMoving),
    _cutoff(vs._cutoff),
    _transparency(vs._transparency)
{
}
