#include <osg/LightSource>

using namespace osg;

LightSource::LightSource():
    _value(StateAttribute::ON)
{
    // switch off culling of light source nodes by default.
    setCullingActive(false);
    _stateset = new StateSet;
    _light = new Light;
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
void LightSource::setStateSetModes(StateSet& stateset,StateAttribute::GLModeValue value) const
{
    if (_light.valid())
    {
        stateset.setAssociatedModes(_light.get(),value);
    }
}

void LightSource::setLocalStateSetModes(StateAttribute::GLModeValue value)
{
    if (!_stateset) _stateset = new StateSet;
    _stateset->setAllToInherit();
    setStateSetModes(*_stateset,value);
}

bool LightSource::computeBound() const
{
    Group::computeBound();
    
    if (_light.valid())
    {
        const Light* light = dynamic_cast<const Light*>(_light.get());
        if (light)
        {
            const Vec4& pos = light->getPosition();
            if (pos[3]!=0.0f)
            {
                float div = 1.0f/pos[3];
                _bsphere.expandBy(Vec3(pos[0]*div,pos[1]*div,pos[2]*div));
            }
        }
    }

    _bsphere_computed = true;

    return true;
}
