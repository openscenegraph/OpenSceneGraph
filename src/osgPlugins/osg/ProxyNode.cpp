#include "osg/ProxyNode"
#include "osg/Notify"
#include <osg/io_utils>

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

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

    if (fr.getOptions() && !fr.getOptions()->getDatabasePathList().empty())
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
                if (fr[0].getStr()) proxyNode.setFileName(i,fr[0].getStr());
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

    unsigned int num_children;
    if (fr[0].matchWord("num_children") &&
        fr[1].getUInt(num_children))
    {
        // could allocate space for children here...
        fr+=2;
        iteratorAdvanced = true;
    }

    unsigned int i;
    for(i=0; i<num_children; i++)
    {
        osgDB::FilePathList& fpl = ((osgDB::ReaderWriter::Options*)fr.getOptions())->getDatabasePathList();
        fpl.push_front( fpl.empty() ? osgDB::getFilePath(proxyNode.getFileName(i)) : fpl.front()+'/'+ osgDB::getFilePath(proxyNode.getFileName(i)));
        Node* node = NULL;
        if((node=fr.readNode())!=NULL)
        {
            proxyNode.addChild(node);
            iteratorAdvanced = true;
        }
        fpl.pop_front();
    }

    for(i=0; i<proxyNode.getNumFileNames(); i++)
    {
        if(i>=proxyNode.getNumChildren() && !proxyNode.getFileName(i).empty())
        {
            osgDB::FilePathList& fpl = ((osgDB::ReaderWriter::Options*)fr.getOptions())->getDatabasePathList();
            fpl.push_front( fpl.empty() ? osgDB::getFilePath(proxyNode.getFileName(i)) : fpl.front()+'/'+ osgDB::getFilePath(proxyNode.getFileName(i)));
            osg::Node *node = osgDB::readNodeFile(proxyNode.getFileName(i), fr.getOptions());
            fpl.pop_front();
            if(node)
            {
                proxyNode.insertChild(i, node);
            }
        }
    }

    return iteratorAdvanced;
}


bool ProxyNode_writeLocalData(const Object& obj, Output& fw)
{
    bool includeExternalReferences = false;
    bool useOriginalExternalReferences = true;
    bool writeExternalReferenceFiles = false;

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
            if(useOriginalExternalReferences)
            {
                fw.indent() << proxyNode.getFileName(i) << std::endl;
            }
            else
            {
                std::string path = osgDB::getFilePath(fw.getFileName());
                std::string new_filename = osgDB::getStrippedName(proxyNode.getFileName(i)) +".osg";
                std::string osgname = path.empty() ? new_filename :  (path +"/"+ new_filename) ;
                fw.indent() << osgname << std::endl;
            }
        }
    }
    fw.moveOut();
    fw.indent() << "}"<< std::endl;


    if(includeExternalReferences) //out->getIncludeExternalReferences()) // inlined mode
    {
        fw.indent() << "num_children " << proxyNode.getNumChildren() << std::endl;
        for(unsigned int i=0; i<proxyNode.getNumChildren(); i++)
        {
            fw.writeObject(*proxyNode.getChild(i));
        }
    }
    else //----------------------------------------- no inlined mode
    {
        fw.indent() << "num_children " << numChildrenToWriteOut << std::endl;
        for(unsigned int i=0; i<proxyNode.getNumChildren(); ++i)
        {
            if (proxyNode.getFileName(i).empty())
            {
                fw.writeObject(*proxyNode.getChild(i));
            }
            else if(writeExternalReferenceFiles) //out->getWriteExternalReferenceFiles())
            {
                if(useOriginalExternalReferences) //out->getUseOriginalExternalReferences())
                {
                    osgDB::writeNodeFile(*proxyNode.getChild(i), proxyNode.getFileName(i));
                }
                else
                {
                    std::string path = osgDB::getFilePath(fw.getFileName());
                    std::string new_filename = osgDB::getStrippedName(proxyNode.getFileName(i)) +".osg";
                    std::string osgname = path.empty() ? new_filename :  (path +"/"+ new_filename) ;
                    osgDB::writeNodeFile(*proxyNode.getChild(i), osgname);
                }
            }
        }
    }

    return true;
}
