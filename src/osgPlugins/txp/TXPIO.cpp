#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/WriteFile>
#include <osg/ref_ptr>
#include <iostream>

#include "TXPNode.h"

using namespace osg;

bool TXPNode_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool TXPNode_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy TXPNode_Proxy
(
    new txp::TXPNode,
    "TXPNode",
    "Object Node TXPNode",
    TXPNode_readLocalData,
    TXPNode_writeLocalData
);

bool TXPNode_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    txp::TXPNode &txpNode = static_cast<txp::TXPNode &>(obj);
    bool itrAdvanced = false;

    if (fr.matchSequence("databaseOptions %s"))
    {
        txpNode.setOptions(fr[1].getStr());
        fr += 2;
        itrAdvanced = true;
    }

    if (fr.matchSequence("databaseName %s"))
    {
        txpNode.setArchiveName(fr[1].getStr());
        txpNode.loadArchive();
        
        fr += 2;
        itrAdvanced = true;
    }


    return itrAdvanced;
}

class Dump2Osg : public osg::NodeVisitor
{
public:
	Dump2Osg( osgDB::Output &fw ) : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ), _fw( fw )
	{}

    virtual void apply(osg::Node& node)
    {
        _fw.writeObject(node);
        NodeVisitor::apply(node);
    }
    osgDB::Output &_fw;
};


bool TXPNode_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const txp::TXPNode &txpNode = static_cast<const txp::TXPNode&>(obj);

	if ( !txpNode.getOptions().empty() )
		fw.indent() << "databaseOptions \"" << txpNode.getOptions() << "\"" << std::endl;
	if ( !txpNode.getArchiveName().empty() )
		fw.indent() << "databaseName \"" << txpNode.getArchiveName() << "\"" << std::endl;

    osg::Group* grp = const_cast<osg::Group*>(txpNode.asGroup());

    Dump2Osg wrt(fw);
    grp->accept(wrt);
    
    return true;
}


