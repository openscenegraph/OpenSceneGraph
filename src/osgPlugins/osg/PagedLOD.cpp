#include "osg/PagedLOD"
#include "osg/Notify"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool PagedLOD_readLocalData(Object& obj, Input& fr);
bool PagedLOD_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_PagedLODProxy
(
    new osg::PagedLOD,
    "PagedLOD",
    "Object Node LOD PagedLOD",
    &PagedLOD_readLocalData,
    &PagedLOD_writeLocalData
);

bool PagedLOD_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    PagedLOD& lod = static_cast<PagedLOD&>(obj);
    
    if (lod.getDatabasePath().empty() && fr.getOptions() && !fr.getOptions()->getDatabasePathList().empty())
    {
        const std::string& path = fr.getOptions()->getDatabasePathList().front();
        if (!path.empty()) 
        {
            lod.setDatabasePath(path);
        }
    } 

    unsigned int num;
    if (fr[0].matchWord("NumChildrenThatCannotBeExpired") && fr[1].getUInt(num))
    {
        lod.setNumChildrenThatCannotBeExpired(num);
        fr+=2;
        iteratorAdvanced = true;
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
                if (fr[0].getStr()) lod.setFileName(i,fr[0].getStr());
                else lod.setFileName(i,"");
                
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
        lod.addChild(node);
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool PagedLOD_writeLocalData(const Object& obj, Output& fw)
{
    const PagedLOD& lod = static_cast<const PagedLOD&>(obj);

    fw.indent() << "NumChildrenThatCannotBeExpired "<<lod.getNumChildrenThatCannotBeExpired()<<std::endl;

    fw.indent() << "FileNameList "<<lod.getNumFileNames()<<" {"<< std::endl;
    fw.moveIn();
    
    unsigned int numChildrenToWriteOut = 0;
    
    for(unsigned int i=0; i<lod.getNumFileNames();++i)
    {
        if (lod.getFileName(i).empty())
        {
            fw.indent() << "\"\"" << std::endl;
            ++numChildrenToWriteOut;
        }
        else 
        {
            fw.indent() << lod.getFileName(i) << std::endl;
        }
    }
    fw.moveOut();
    fw.indent() << "}"<< std::endl;

    fw.indent() << "num_children " << numChildrenToWriteOut << std::endl;
    for(unsigned int j=0;j<lod.getNumChildren();++j)
    {
        if (lod.getFileName(j).empty())
        {
            fw.writeObject(*lod.getChild(j));
        }
    }


    return true;
}
