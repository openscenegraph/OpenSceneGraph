#include "TerrapageNode.h"

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osg/ref_ptr>

#include <iostream>

bool TerrapageNode_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool TerrapageNode_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy TerrapageNode_Proxy
(
    new txp::TerrapageNode,
    "TerrapageNode",
    "Object Node TerrapageNode",
    TerrapageNode_readLocalData,
    TerrapageNode_writeLocalData
);

bool TerrapageNode_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    txp::TerrapageNode &pager = static_cast<txp::TerrapageNode &>(obj);
    bool itrAdvanced = false;

    if (fr.matchSequence("databaseOptions %s"))
    {
        pager.setDatabaseOptions(fr[1].getStr());
        fr += 2;
        itrAdvanced = true;
    }

    if (fr.matchSequence("databaseName %s"))
    {
        pager.setDatabaseName(fr[1].getStr());
        pager.loadDatabase();
        
        fr += 2;
        itrAdvanced = true;
    }


    return itrAdvanced;
}

bool TerrapageNode_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const txp::TerrapageNode &pager = static_cast<const txp::TerrapageNode &>(obj);

    if (!pager.getDatabaseOptions().empty()) fw.indent() << "databaseOptions \"" << pager.getDatabaseOptions() << "\""<<std::endl;
    if (!pager.getDatabaseName().empty()) fw.indent() << "databaseName \"" << pager.getDatabaseName() << "\"" << std::endl;

    return true;
}


