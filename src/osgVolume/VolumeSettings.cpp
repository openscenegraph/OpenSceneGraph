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
    _sampleRatioProperty(new SampleRatioProperty(1.0f)),
    _sampleRatioWhenMovingProperty(new SampleRatioWhenMovingProperty(1.0f)),
    _cutoffProperty(new AlphaFuncProperty(0.0f)),
    _transparencyProperty(new TransparencyProperty(1.0f))
{
}

VolumeSettings::VolumeSettings(const VolumeSettings& vs,const osg::CopyOp& copyop):
    Property(vs, copyop),
    _filename(vs._filename),
    _technique(vs._technique),
    _shadingModel(vs._shadingModel),
    _sampleRatioProperty(osg::clone(vs._sampleRatioProperty.get(), copyop)),
    _sampleRatioWhenMovingProperty(osg::clone(vs._sampleRatioWhenMovingProperty.get(), copyop)),
    _cutoffProperty(osg::clone(vs._cutoffProperty.get(), copyop)),
    _transparencyProperty(osg::clone(vs._transparencyProperty.get(), copyop))
{
}

void VolumeSettings::accept(PropertyVisitor& pv)
{
    pv.apply(*this);
}

void VolumeSettings::traverse(PropertyVisitor& pv)
{
    _sampleRatioProperty->accept(pv);
    _sampleRatioWhenMovingProperty->accept(pv);
    _cutoffProperty->accept(pv);
    _transparencyProperty->accept(pv);
}
