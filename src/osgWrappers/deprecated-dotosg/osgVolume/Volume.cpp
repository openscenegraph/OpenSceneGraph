#include <osgVolume/Volume>

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

bool Volume_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool Volume_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(Volume_Proxy)
(
    new osgVolume::Volume,
    "Volume",
    "Object Node Volume Group",
    Volume_readLocalData,
    Volume_writeLocalData
);

bool Volume_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osgVolume::Volume& volume = static_cast<osgVolume::Volume&>(obj);

    bool itrAdvanced = false;

    osg::ref_ptr<osg::Object> readObject = fr.readObjectOfType(osgDB::type_wrapper<osgVolume::VolumeTechnique>());
    if (readObject.valid())
    {
        volume.setVolumeTechniquePrototype(dynamic_cast<osgVolume::VolumeTechnique*>(readObject.get()));
        itrAdvanced = true;
    }

    return itrAdvanced;
}

bool Volume_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgVolume::Volume& volume = static_cast<const osgVolume::Volume&>(obj);

    osg::notify(osg::NOTICE)<<"Volume write"<<std::endl;

    if (volume.getVolumeTechniquePrototype())
    {
        fw.writeObject(*volume.getVolumeTechniquePrototype());
    }

    return true;
}
