#include "osg/LightSource"

using namespace osg;

LightSource::LightSource()
{
    _bsphere_computed = false;
}


LightSource::~LightSource()
{
    // ref_ptr<> automactially decrements the reference count of attached lights.
}

const bool LightSource::computeBound() const
{
    // note, don't do anything right now as the light itself is not
    // visualised, just having an effect on the lighting of geodes.
    _bsphere.init();

    _bsphere_computed = true;

    return true;
}
