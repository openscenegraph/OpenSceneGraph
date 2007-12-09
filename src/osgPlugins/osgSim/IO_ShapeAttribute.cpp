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

osgDB::RegisterDotOsgWrapperProxy ShapeAttributeList_Proxy
(
    new ShapeAttributeList,
    "ShapeAttributeList",
    "Object ShapeAttributeList",
    &ShapeAttributeList_readLocalData,
    &ShapeAttributeList_writeLocalData,
    osgDB::DotOsgWrapper::READ_AND_WRITE
);



osgSim::ShapeAttribute::Type ShapeAttributeList_typeStringToEnum(std::string type)
{
    if (type == "STRING") return osgSim::ShapeAttribute::STRING;
    else if (type == "DOUBLE") return osgSim::ShapeAttribute::DOUBLE;
    else if (type == "INTEGER") return osgSim::ShapeAttribute::INTEGER;
    else return osgSim::ShapeAttribute::UNKNOW;
}
void ShapeAttributeList_typeEnumToString(osgSim::ShapeAttribute::Type type, std::string & typeStr)
{
    switch (type)
    {
    case osgSim::ShapeAttribute::STRING:  typeStr = "STRING"; break;
    case osgSim::ShapeAttribute::DOUBLE:  typeStr = "DOUBLE"; break;
    case osgSim::ShapeAttribute::INTEGER: typeStr = "INTEGER"; break;
    default: typeStr = "UNKNOW"; break;
    }
}


bool ShapeAttributeList_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    bool iteratorAdvanced = false;
    ShapeAttributeList &sal = static_cast<ShapeAttributeList &>(obj);

    
    int size = 0;
    
    if (fr[0].matchWord("size") && fr[1].getInt(size))
    {
        sal.reserve(size);
        fr += 2;
        iteratorAdvanced = true;
    }
    
    if (size)
    {
        std::string name;
        osgSim::ShapeAttribute::Type type;
        for (int i = 0; i < size; ++i)
        {   
            if (fr.matchSequence("name %s"))
            {
               name = fr[1].getStr();    
               fr += 2;
               iteratorAdvanced = true;
            }
            if (fr[0].matchWord("type"))
            {
               type = ShapeAttributeList_typeStringToEnum(fr[1].getStr());
               fr += 2;
               iteratorAdvanced = true;
            
                switch (type)
                {
                    case osgSim::ShapeAttribute::STRING:
                    {
                        if (fr.matchSequence("value %s"))
                        {
                           std::string value = fr[1].getStr();
                           fr += 2;
                           iteratorAdvanced = true;
                           sal.push_back(osgSim::ShapeAttribute((const char *) name.c_str(), (char*) value.c_str()));
                        }
                        break;
                    }
                    case osgSim::ShapeAttribute::DOUBLE:
                    {
                        double value;
                        if (fr[0].matchWord("value") && fr[1].getFloat(value))
                        {
                           fr += 2;
                           iteratorAdvanced = true;
                           sal.push_back(osgSim::ShapeAttribute((const char *) name.c_str(), value));
                        }
                        break;
                    }
                    case osgSim::ShapeAttribute::INTEGER:
                    {
                        int value;
                        if (fr[0].matchWord("value") && fr[1].getInt(value))
                        {
                           fr += 2;
                           iteratorAdvanced = true;
                           sal.push_back(osgSim::ShapeAttribute((const char *) name.c_str(), value));
                        }
                        break;
                    }
                    case osgSim::ShapeAttribute::UNKNOW:
                    default:
                    {
                        sal.push_back(osgSim::ShapeAttribute((const char *) name.c_str()));
                        break;
                    }
                }
            }
        }
    }

    return iteratorAdvanced;
}

    
bool ShapeAttributeList_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const ShapeAttributeList &sal = static_cast<const ShapeAttributeList &>(obj);

    unsigned int size = sal.size();
    fw.indent()<<"size "<< size << std::endl;

    if (size)
    {
        std::string type;
        ShapeAttributeList::const_iterator it, end = sal.end();
        for (it = sal.begin(); it != end; ++it)
        {
            fw.indent()<<"name \""<< it->getName() << "\"" << std::endl;
            ShapeAttributeList_typeEnumToString(it->getType(), type);
            fw.indent()<<"type "<< type << std::endl;
            switch (it->getType())
            {
            case osgSim::ShapeAttribute::STRING:
            {
                fw.indent()<<"value \""<< it->getString() << "\"" << std::endl;
                break;
            }
            case osgSim::ShapeAttribute::INTEGER:
            {
                fw.indent()<<"value "<< it->getInt() << std::endl;
                break;
            }
            case osgSim::ShapeAttribute::DOUBLE:
            {
                fw.indent()<<"value "<< it->getDouble() << std::endl;
                break;
            }
            case osgSim::ShapeAttribute::UNKNOW:
            default: break;
            }
        }
    }
    return true;
}
