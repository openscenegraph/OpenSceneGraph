#include <osg/LightSource>

using namespace osg;

LightSource::LightSource()
{
    // switch off culling of light source nodes by default.
    setCullingActive(false);
    _dstate = osgNew StateSet;
    _value = StateAttribute::ON;
    _light = osgNew Light;
}


LightSource::~LightSource()
{
    // ref_ptr<> automactially decrements the reference count of attached lights.
}


void LightSource::setLight(StateAttribute* light)
{
    _light = light;
    setLocalStateSetModes(_value);
}

// Set the GLModes on StateSet associated with the ClipPlanes.
void LightSource::setStateSetModes(StateSet& stateset,const StateAttribute::GLModeValue value) const
{
    if (_light.valid())
    {
        _light->setStateSetModes(stateset,value);
    }
}

void LightSource::setLocalStateSetModes(const StateAttribute::GLModeValue value)
{
    if (!_dstate) _dstate = osgNew StateSet;
    _dstate->setAllToInherit();
    setStateSetModes(*_dstate,value);
}

const bool LightSource::computeBound() const
{
    // note, don't do anything right now as the light itself is not
    // visualised, just having an effect on the lighting of geodes.
    _bsphere.init();

    _bsphere_computed = true;

    return true;
}
