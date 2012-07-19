#include <osgVolume/Property>

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

bool CompositeProperty_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool CompositeProperty_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(CompositeProperty_Proxy)
(
    new osgVolume::CompositeProperty,
    "CompositeProperty",
    "Object CompositeProperty",
    CompositeProperty_readLocalData,
    CompositeProperty_writeLocalData
);


bool CompositeProperty_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osgVolume::CompositeProperty& cp = static_cast<osgVolume::CompositeProperty&>(obj);

    bool itrAdvanced = false;

    osg::ref_ptr<osg::Object> readObject;
    do
    {
        readObject = fr.readObjectOfType(osgDB::type_wrapper<osgVolume::Property>());
        if (readObject.valid()) itrAdvanced = true;

        osgVolume::Property* property = dynamic_cast<osgVolume::Property*>(readObject.get());
        if (property) cp.addProperty(property);

    } while (readObject.valid());

    return itrAdvanced;
}

bool CompositeProperty_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgVolume::CompositeProperty& cp = static_cast<const osgVolume::CompositeProperty&>(obj);


    for(unsigned int i=0; i<cp.getNumProperties(); ++i)
    {
        fw.writeObject(*cp.getProperty(i));
    }

    return true;
}
