#include "osg/ProxyNode"
#include "osg/Notify"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"
#include <osgDB/ReadFile>

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool ProxyNode_readLocalData(Object& obj, Input& fr);
bool ProxyNode_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_ProxyNodeProxy
(
    new osg::ProxyNode,
    "ProxyNode",
    "Object Node ProxyNode",
    &ProxyNode_readLocalData,
    &ProxyNode_writeLocalData
);

bool ProxyNode_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    ProxyNode& proxyNode = static_cast<ProxyNode&>(obj);
    
    if (fr.matchSequence("Center %f %f %f"))
    {
        Vec3 center;
        fr[1].getFloat(center[0]);
        fr[2].getFloat(center[1]);
        fr[3].getFloat(center[2]);
        proxyNode.setCenter(center);

        iteratorAdvanced = true;
        fr+=4;
    }
	else
		proxyNode.setCenterMode(osg::ProxyNode::USE_BOUNDING_SPHERE_CENTER);

    float radius;
    if (fr[0].matchWord("Radius") && fr[1].getFloat(radius))
    {
        proxyNode.setRadius(radius);
        fr+=2;
        iteratorAdvanced = true;
    }

	if (proxyNode.getDatabasePath().empty() && fr.getOptions() && !fr.getOptions()->getDatabasePathList().empty())
    {
        const std::string& path = fr.getOptions()->getDatabasePathList().front();
        if (!path.empty()) 
        {
            proxyNode.setDatabasePath(path);
        }
    } 

    bool matchFirst;
    if ((matchFirst=fr.matchSequence("FileNameList {")) || fr.matchSequence("FileNameList %i {"))
    {

        // set up coordinates.
        int entry = fr[0].getNoNestedBrackets();
        if (matchFirst)
        {
            fr += 2;
        }
        else
        {
            fr += 3;
        }

        unsigned int i=0;
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            if (fr[0].isString() || fr[0].isQuotedString())
            {
				if (fr[0].getStr())
				{
					osg::Node *node = osgDB::readNodeFile(proxyNode.getDatabasePath() + fr[0].getStr()); // If filename is flt, will need getDatabasePath()
					if(node)
					{
						printf("cargando: %s\n", fr[0].getStr());
						proxyNode.addChild(node, fr[0].getStr());
					}
				}
				//if (fr[0].getStr()) proxyNode.setFileName(i,fr[0].getStr());
                else proxyNode.setFileName(i,"");
                
                ++fr;
                ++i;
            }
            else
            {
                ++fr;
            }
        }

        iteratorAdvanced = true;
        ++fr;

    }

    int num_children;
    if (fr[0].matchWord("num_children") &&
        fr[1].getInt(num_children))
    {
        // could allocate space for children here...
        fr+=2;
        iteratorAdvanced = true;
    }

    Node* node = NULL;
    while((node=fr.readNode())!=NULL)
    {
        proxyNode.addChild(node);
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool ProxyNode_writeLocalData(const Object& obj, Output& fw)
{
    const ProxyNode& proxyNode = static_cast<const ProxyNode&>(obj);

    if (proxyNode.getCenterMode()==osg::ProxyNode::USER_DEFINED_CENTER) fw.indent() << "Center "<< proxyNode.getCenter() << std::endl;

    fw.indent() << "Radius "<<proxyNode.getRadius()<<std::endl;

	fw.indent() << "FileNameList "<<proxyNode.getNumFileNames()<<" {"<< std::endl;
    fw.moveIn();
    
    unsigned int numChildrenToWriteOut = 0;
    
    for(unsigned int i=0; i<proxyNode.getNumFileNames();++i)
    {
        if (proxyNode.getFileName(i).empty())
        {
            fw.indent() << "\"\"" << std::endl;
            ++numChildrenToWriteOut;
        }
        else 
        {
            fw.indent() << proxyNode.getFileName(i) << std::endl;
        }
    }
    fw.moveOut();
    fw.indent() << "}"<< std::endl;

    fw.indent() << "num_children " << numChildrenToWriteOut << std::endl;
    for(unsigned int j=0;j<proxyNode.getNumChildren();++j)
    {
        if (proxyNode.getFileName(j).empty())
        {
            fw.writeObject(*proxyNode.getChild(j));
        }
    }


    return true;
}
