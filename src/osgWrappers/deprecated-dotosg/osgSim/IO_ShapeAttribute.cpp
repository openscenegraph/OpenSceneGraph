#include <osgSim/ShapeAttribute>

#include <iostream>
#include <string>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

using namespace osgSim;

bool ShapeAttributeList_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool ShapeAttributeList_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(ShapeAttributeList_Proxy)
(
    new ShapeAttributeList,
    "ShapeAttributeList",
    "Object ShapeAttributeList",
    &ShapeAttributeList_readLocalData,
    &ShapeAttributeList_writeLocalData,
    osgDB::DotOsgWrapper::READ_AND_WRITE
);

bool ShapeAttributeList_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    bool iteratorAdvanced = false;
    ShapeAttributeList &sal = static_cast<ShapeAttributeList &>(obj);


    int entry = fr[0].getNoNestedBrackets();

    while (!fr.eof() && fr[0].getNoNestedBrackets()>=entry)
    {
        if (fr.matchSequence("string %s %s"))
        {
            sal.push_back(osgSim::ShapeAttribute(fr[1].getStr(), fr[2].getStr()));
            fr += 3;
            iteratorAdvanced = true;
        }
        else if (fr.matchSequence("double %s %f"))
        {
            double value;
            fr[2].getFloat(value);
            sal.push_back(osgSim::ShapeAttribute(fr[1].getStr(), value));
            fr += 3;
            iteratorAdvanced = true;
        }
        else if (fr.matchSequence("int %s %i"))
        {
            int value;
            fr[2].getInt(value);
            sal.push_back(osgSim::ShapeAttribute(fr[1].getStr(), value));
            fr += 3;
            iteratorAdvanced = true;
        }
        else ++fr;
    }

    return iteratorAdvanced;
}


bool ShapeAttributeList_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const ShapeAttributeList &sal = static_cast<const ShapeAttributeList &>(obj);

    for (ShapeAttributeList::const_iterator it = sal.begin(); it != sal.end(); ++it)
    {
        switch (it->getType())
        {
            case osgSim::ShapeAttribute::STRING:
            {
                fw.indent()<<"string "<< fw.wrapString(it->getName())<<" "<<fw.wrapString(it->getString()) << std::endl;
                break;
            }
            case osgSim::ShapeAttribute::INTEGER:
            {
                fw.indent()<<"int    "<< fw.wrapString(it->getName())<<" "<<it->getInt() << std::endl;
                break;
            }
            case osgSim::ShapeAttribute::DOUBLE:
            {
                fw.indent()<<"double "<< fw.wrapString(it->getName())<<" "<<it->getDouble() << std::endl;
                break;
            }
            case osgSim::ShapeAttribute::UNKNOWN:
                default: break;
        }
    }
    return true;
}
