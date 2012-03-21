#include "osg/LightSource"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool LightSource_readLocalData(Object& obj, Input& fr);
bool LightSource_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(LightSource)
(
    new osg::LightSource,
    "LightSource",
    "Object Node LightSource Group",
    &LightSource_readLocalData,
    &LightSource_writeLocalData
);

bool LightSource_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    LightSource& lightsource = static_cast<LightSource&>(obj);

    if (fr[0].matchWord("referenceFrame"))
    {
        bool cullingActiveBefore = lightsource.getCullingActive();

        if (fr[1].matchWord("RELATIVE_TO_ABSOLUTE") || fr[1].matchWord("ABSOLUTE"))
        {
            lightsource.setReferenceFrame(LightSource::ABSOLUTE_RF);
            fr += 2;
            iteratorAdvanced = true;
        }
        if (fr[1].matchWord("RELATIVE_TO_PARENTS") || fr[1].matchWord("RELATIVE"))
        {
            lightsource.setReferenceFrame(LightSource::RELATIVE_RF);
            fr += 2;
            iteratorAdvanced = true;
        }

        // if culling wasn't before reset it to off.
        if (!cullingActiveBefore && lightsource.getCullingActive())
        {
            lightsource.setCullingActive(cullingActiveBefore);
        }
    }

    osg::ref_ptr<StateAttribute> sa=fr.readStateAttribute();
    osg::Light* light = dynamic_cast<Light*>(sa.get());
    if (light)
    {
        lightsource.setLight(light);
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool LightSource_writeLocalData(const Object& obj, Output& fw)
{
    const LightSource& lightsource = static_cast<const LightSource&>(obj);

    fw.indent() << "referenceFrame ";
    switch (lightsource.getReferenceFrame())
    {
        case LightSource::ABSOLUTE_RF:
            fw << "ABSOLUTE\n";
            break;
        case LightSource::RELATIVE_RF:
        default:
            fw << "RELATIVE\n";
    };

    if (lightsource.getLight()) fw.writeObject(*lightsource.getLight());

    return true;
}
