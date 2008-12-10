#include "osg/Geode"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Geode_readLocalData(Object& obj, Input& fr);
bool Geode_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Geode)
(
    new osg::Geode,
    "Geode",
    "Object Node Geode",
    &Geode_readLocalData,
    &Geode_writeLocalData
);

bool Geode_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Geode& geode = static_cast<Geode&>(obj);

    int num_drawables;
    if ((fr[0].matchWord("num_drawables") || fr[0].matchWord("num_geosets")) &&
        fr[1].getInt(num_drawables))
    {
        // could allocate space for children here...
        fr+=2;
        iteratorAdvanced = true;
    }
    
    Drawable* drawable = NULL;
    while((drawable=fr.readDrawable())!=NULL)
    {
        geode.addDrawable(drawable);
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool Geode_writeLocalData(const osg::Object& obj, Output& fw)
{
    const Geode& geode = static_cast<const Geode&>(obj);

    fw.indent() << "num_drawables " << geode.getNumDrawables() << std::endl;
    
    for(unsigned int i=0;i<geode.getNumDrawables();++i)
    {
        fw.writeObject(*geode.getDrawable(i));
    }

    return true;
}
