#include "osg/Object"
#include "osg/Notify"

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
REGISTER_DOTOSGWRAPPER(Object)
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
        else if (fr[1].matchWord("UNSPECIFIED"))
        {
            obj.setDataVariance(osg::Object::UNSPECIFIED);
            fr +=2 ;
            iteratorAdvanced = true;
        }
    }

    if (fr.matchSequence("name %s"))
    {
        obj.setName(fr[1].getStr());
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("UserData {"))
    {
        osg::notify(osg::DEBUG_INFO) << "Matched UserData {"<< std::endl;
        int entry = fr[0].getNoNestedBrackets();
        fr += 2;

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            Object* object = fr.readObject();
            if (object) obj.setUserData(object);
            osg::notify(osg::DEBUG_INFO) << "read "<<object<< std::endl;
            ++fr;
        }
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool Object_writeLocalData(const Object& obj, Output& fw)
{
    switch(obj.getDataVariance())
    {
        case(osg::Object::STATIC):      fw.indent() << "DataVariance STATIC" << std::endl;break;
        case(osg::Object::DYNAMIC):     fw.indent() << "DataVariance DYNAMIC" << std::endl;break;
        case(osg::Object::UNSPECIFIED): break; // fw.indent() << "DataVariance UNSPECIFIED" << std::endl;break;
    }

    if (!obj.getName().empty()) fw.indent() << "name "<<fw.wrapString(obj.getName())<< std::endl;

    if (obj.getUserData())
    {
        const Object* object = dynamic_cast<const Object*>(obj.getUserData());
        if (object)
        {
            fw.indent() << "UserData {"<< std::endl;
            fw.moveIn();
            fw.writeObject(*object);
            fw.moveOut();
            fw.indent() << "}"<< std::endl;
        }
    }

    return true;
}
