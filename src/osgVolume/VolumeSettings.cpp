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
    _transparencyProperty(new TransparencyProperty(1.0f)),
    _isoSurfaceProperty(new IsoSurfaceProperty(0.0f))
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
    _transparencyProperty(osg::clone(vs._transparencyProperty.get(), copyop)),
    _isoSurfaceProperty(osg::clone(vs._isoSurfaceProperty.get(), copyop))
{
}

void VolumeSettings::accept(PropertyVisitor& pv)
{
    pv.apply(*this);
}

void VolumeSettings::traverse(PropertyVisitor& pv)
{
    if (_sampleRatioProperty.valid())                                   _sampleRatioProperty->accept(pv);
    if (_sampleRatioWhenMovingProperty.valid())                         _sampleRatioWhenMovingProperty->accept(pv);
    if (_cutoffProperty.valid())                                        _cutoffProperty->accept(pv);
    if (_transparencyProperty.valid())                                  _transparencyProperty->accept(pv);
    if (_isoSurfaceProperty.valid() && _shadingModel==Isosurface)       _isoSurfaceProperty->accept(pv);
}

void VolumeSettings::setCutoff(float co)
{
    _cutoffProperty->setValue(co);
    if (_isoSurfaceProperty.valid())
    {
        OSG_NOTICE<<"Setting IsoSurface value to "<<co<<std::endl;
        _isoSurfaceProperty->setValue(co);
    }

    dirty();
}
