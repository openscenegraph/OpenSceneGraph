#include <osgVolume/VolumeTile>

#include <iostream>
#include <string>

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/io_utils>

#include <osgDB/ReadFile>
#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

bool VolumeTile_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool VolumeTile_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(VolumeTile_Proxy)
(
    new osgVolume::VolumeTile,
    "VolumeTile",
    "Object Node VolumeTile Group",
    VolumeTile_readLocalData,
    VolumeTile_writeLocalData
);

bool VolumeTile_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osgVolume::VolumeTile& volumeTile = static_cast<osgVolume::VolumeTile&>(obj);

    bool itrAdvanced = false;

    osg::ref_ptr<osg::Object> readObject = fr.readObjectOfType(osgDB::type_wrapper<osgVolume::Locator>());
    if (readObject.valid()) itrAdvanced = true;

    osgVolume::Locator* locator = dynamic_cast<osgVolume::Locator*>(readObject.get());
    if (locator) volumeTile.setLocator(locator);


    readObject = fr.readObjectOfType(osgDB::type_wrapper<osgVolume::Layer>());
    if (readObject.valid()) itrAdvanced = true;

    osgVolume::Layer* readLayer = dynamic_cast<osgVolume::Layer*>(readObject.get());
    if (readLayer) volumeTile.setLayer(readLayer);


    readObject = fr.readObjectOfType(osgDB::type_wrapper<osgVolume::VolumeTechnique>());
    if (readObject.valid())
    {
        volumeTile.setVolumeTechnique(dynamic_cast<osgVolume::VolumeTechnique*>(readObject.get()));
        itrAdvanced = true;
    }

    return itrAdvanced;
}

bool VolumeTile_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgVolume::VolumeTile& volumeTile = static_cast<const osgVolume::VolumeTile&>(obj);

    int prec = fw.precision();
    fw.precision(15);

    if (volumeTile.getLocator())
    {
        fw.writeObject(*volumeTile.getLocator());
    }

    if (volumeTile.getLayer())
    {
        fw.writeObject(*volumeTile.getLayer());
    }

    if (volumeTile.getVolumeTechnique())
    {
        fw.writeObject(*volumeTile.getVolumeTechnique());
    }

    fw.precision(prec);

    return true;
}
