#include "osg/Object"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Object_readLocalData(Object& obj, Input& fr);
bool Object_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
// note, Object doesn't currently require any read and write.
RegisterDotOsgWrapperProxy g_ObjectProxy
(
    /*new osg::Object*/NULL,
    "Object",
    "Object",
    &Object_readLocalData,
    &Object_writeLocalData
);

bool Object_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    if (fr[0].matchWord("DataVariance"))
    {
        if (fr[1].matchWord("DYNAMIC"))
        {
            obj.setDataVariance(osg::Object::DYNAMIC);
            fr +=2 ;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("STATIC"))
        {
            obj.setDataVariance(osg::Object::STATIC);
            fr +=2 ;
            iteratorAdvanced = true;
        }
    }    

    return iteratorAdvanced;
}


bool Object_writeLocalData(const Object& obj, Output& fw)
{
    switch(obj.getDataVariance())
    {
        case(osg::Object::STATIC): fw.indent() << "DataVariance STATIC" << std::endl;break;
        default:                   fw.indent() << "DataVariance DYNAMIC" << std::endl;break;
    }

    return true;
}
