#include "osg/LightSource"
#include "osg/Input"
#include "osg/Output"

#include "osg/Registry"

using namespace osg;

RegisterObjectProxy<LightSource> g_LightSourceProxy;

LightSource::LightSource()
{
    _bsphere_computed = false;
}


LightSource::~LightSource()
{
    // ref_ptr<> automactially decrements the reference count of attached lights.
}


bool LightSource::readLocalData(Input& fr)
{

    bool iteratorAdvanced = false;

    if (Node::readLocalData(fr)) iteratorAdvanced = true;

    Light* light = static_cast<Light*>(Light::instance()->readClone(fr));
    if (light)
    {
        _light = light;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool LightSource::writeLocalData(Output& fw)
{
    Node::writeLocalData(fw);

    if (_light.valid()) _light->write(fw);

    return true;
}

bool LightSource::computeBound( void )
{   
    // note, don't do anything right now as the light itself is not
    // visualised, just having an effect on the lighting of geodes.
    _bsphere.init();
    
    _bsphere_computed = true;

    return true;   
}
