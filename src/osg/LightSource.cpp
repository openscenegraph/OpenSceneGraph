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
#include <osg/LightSource>

using namespace osg;

LightSource::LightSource():
    _value(StateAttribute::ON),
    _referenceFrame(RELATIVE_RF)
{
    // switch off culling of light source nodes by default.
    setCullingActive(false);
    setStateSet(new StateSet);
    _light = new Light;
}


LightSource::~LightSource()
{
    // ref_ptr<> automactially decrements the reference count of attached lights.
}

void LightSource::setReferenceFrame(ReferenceFrame rf)
{
    _referenceFrame = rf;
}

void LightSource::setLight(Light* light)
{
    _light = light;
    setLocalStateSetModes(_value);
}

// Set the GLModes on StateSet associated with the ClipPlanes.
void LightSource::setStateSetModes(StateSet& stateset,StateAttribute::GLModeValue value) const
{
    if (_light.valid())
    {
        stateset.setAssociatedModes(_light.get(),value);
    }
}

void LightSource::setLocalStateSetModes(StateAttribute::GLModeValue value)
{
    if (!_stateset) setStateSet(new StateSet);

    _stateset->clear();
    setStateSetModes(*_stateset,value);
}

BoundingSphere LightSource::computeBound() const
{
    BoundingSphere bsphere(Group::computeBound());

    if (_light.valid() && _referenceFrame==RELATIVE_RF)
    {
        const Vec4& pos = _light->getPosition();
        if (pos[3]!=0.0f)
        {
            float div = 1.0f/pos[3];
            bsphere.expandBy(Vec3(pos[0]*div,pos[1]*div,pos[2]*div));
        }
    }

    return bsphere;
}

void LightSource::setThreadSafeRefUnref(bool threadSafe)
{
    Group::setThreadSafeRefUnref(threadSafe);

    if (_light.valid()) _light->setThreadSafeRefUnref(threadSafe);
}
