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
REGISTER_DOTOSGWRAPPER(ProxyNode)
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

    if (fr.matchSequence("ExtRefMode %s") || fr.matchSequence("ExtRefMode %w"))
    {
        if      (fr[1].matchWord("LOAD_IMMEDIATELY"))
            proxyNode.setLoadingExternalReferenceMode(ProxyNode::LOAD_IMMEDIATELY);
        else if (fr[1].matchWord("DEFER_LOADING_TO_DATABASE_PAGER"))
            proxyNode.setLoadingExternalReferenceMode(ProxyNode::DEFER_LOADING_TO_DATABASE_PAGER);
        else if (fr[1].matchWord("NO_AUTOMATIC_LOADING"))
            proxyNode.setLoadingExternalReferenceMode(ProxyNode::NO_AUTOMATIC_LOADING);

        fr+=2;
        iteratorAdvanced = true;
    }

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

    unsigned int num_children = 0;
    if (fr[0].matchWord("num_children") &&
        fr[1].getUInt(num_children))
    {
        // could allocate space for children here...
        fr+=2;
        iteratorAdvanced = true;
    }

    bool make_options = (fr.getOptions() == NULL);
    if (make_options) fr.setOptions(new osgDB::Options()); //need valid options
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

    if(proxyNode.getLoadingExternalReferenceMode() == ProxyNode::LOAD_IMMEDIATELY)
    {
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
    }
    if (make_options) fr.setOptions(NULL);
    return iteratorAdvanced;
}


bool ProxyNode_writeLocalData(const Object& obj, Output& fw)
{
    bool includeExternalReferences = false;
    bool useOriginalExternalReferences = true;
    bool writeExternalReferenceFiles = false;
    if (fw.getOptions())
    {
        std::string optionsString = fw.getOptions()->getOptionString();
        includeExternalReferences = optionsString.find("includeExternalReferences")!=std::string::npos;
        bool newExternals = optionsString.find("writeExternalReferenceFiles")!=std::string::npos;
        if (newExternals)
        {
            useOriginalExternalReferences = false;
            writeExternalReferenceFiles = true;
        }
    }
    const ProxyNode& proxyNode = static_cast<const ProxyNode&>(obj);

    if (proxyNode.getCenterMode()==osg::ProxyNode::USER_DEFINED_CENTER) fw.indent() << "Center "<< proxyNode.getCenter() << std::endl;

    fw.indent() << "ExtRefMode ";

    switch(proxyNode.getLoadingExternalReferenceMode())
    {
    case ProxyNode::LOAD_IMMEDIATELY:
        fw.indent() << "LOAD_IMMEDIATELY" <<std::endl;
        break;
    case ProxyNode::DEFER_LOADING_TO_DATABASE_PAGER:
        fw.indent() << "DEFER_LOADING_TO_DATABASE_PAGER" <<std::endl;
        break;
    case ProxyNode::NO_AUTOMATIC_LOADING:
        fw.indent() << "NO_AUTOMATIC_LOADING" <<std::endl;
        break;
    }

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
                    std::string origname = proxyNode.getFileName(i);
                    if (!fw.getExternalFileWritten(origname))
                    {
                        osgDB::writeNodeFile(*proxyNode.getChild(i), origname);
                        fw.setExternalFileWritten(origname, true);
                    }
                }
                else
                {
                    std::string path = osgDB::getFilePath(fw.getFileName());
                    std::string new_filename = osgDB::getStrippedName(proxyNode.getFileName(i)) +".osg";
                    std::string osgname = path.empty() ? new_filename :  (path +"/"+ new_filename) ;
                    if (!fw.getExternalFileWritten(osgname))
                    {
                        osgDB::writeNodeFile(*proxyNode.getChild(i), osgname);
                        fw.setExternalFileWritten(osgname, true);
                    }
                }
            }
        }
    }

    return true;
}
