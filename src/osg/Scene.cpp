#include "osg/Scene"
#include "osg/Registry"
#include "osg/Input"
#include "osg/Output"

using namespace osg;

RegisterObjectProxy<Scene> g_SceneProxy;

Scene::Scene()
{
    _gstate = NULL;
}

Scene::~Scene()
{
    if (_gstate) _gstate->unref();
}

void Scene::setGState(osg::GeoState* gstate)
{
    if (gstate==_gstate) return;

    if (_gstate) _gstate->unref();

    _gstate = gstate;
    if (_gstate) _gstate->ref();
}

bool Scene::readLocalData(Input& fr)
{
    bool iteratorAdvanced = false;

    if (fr[0].matchWord("Use"))
    {
        if (fr[1].isString()) {
            GeoState* geostate = dynamic_cast<GeoState*>(fr.getObjectForUniqueID(fr[1].getStr()));
            if (geostate) {
                fr+=2;
                _gstate = geostate;
                _gstate->ref();
                iteratorAdvanced = true;
            }
        }
    }

    if (GeoState* readState = static_cast<GeoState*>(GeoState::instance()->readClone(fr)))
    {
        _gstate = readState;
        _gstate->ref();
        iteratorAdvanced = true;
    }

    if (Group::readLocalData(fr)) iteratorAdvanced = true;

    return iteratorAdvanced;
}

bool Scene::writeLocalData(Output& fw)
{
    if (_gstate)
    {
        _gstate->write(fw);
    }
    
    Group::writeLocalData(fw);

    return true;
}
