#include "osg/Object"
#include "osg/Registry"
#include "osg/Input"
#include "osg/Output"

using namespace osg;

Object* Object::readClone(Input& fr)
{
    if (fr[0].matchWord("Use"))
    {
        if (fr[1].isString())
        {
            Object* obj = fr.getObjectForUniqueID(fr[1].getStr());
            if (obj && isSameKindAs(obj))
            {
                fr+=2;
                return obj;
            }
        }
        return NULL;
        
    }

    if (!fr[0].matchWord(className()) ||
        !fr[1].isOpenBracket()) return NULL;

    int entry = fr[0].getNoNestedBrackets();

    Object* obj = clone();

    fr+=2;

    while(!fr.eof() && fr[0].getNoNestedBrackets()>entry)
    {
        bool iteratorAdvanced = false;
        if (fr[0].matchWord("UniqueID") && fr[1].isString()) {
            fr.regisiterUniqueIDForObject(fr[1].getStr(),obj);
            fr += 2;
        }
        if (obj->readLocalData(fr)) iteratorAdvanced = true;
        if (!iteratorAdvanced) ++fr;
    }
    ++fr;                                         // step over trailing '}'

    return obj;
}


bool Object::write(Output& fw)
{
    if (_refCount>1) {
        std::string uniqueID;
        if (fw.getUniqueIDForObject(this,uniqueID)) {
            fw.indent() << "Use " << uniqueID << endl;
            return true;
        }
    }

    fw.indent() << className() << " {"<<endl;

    fw.moveIn();

    if (_refCount>1) {
        std::string uniqueID;
        fw.createUniqueIDForObject(this,uniqueID);
        fw.registerUniqueIDForObject(this,uniqueID);
        fw.indent() << "UniqueID " << uniqueID << endl;
    }

    writeLocalData(fw);

    fw.moveOut();

    fw.indent() << "}"<<endl;

    return true;
}
