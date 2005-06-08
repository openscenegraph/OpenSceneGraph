#include "osg/Program"
#include "osg/Shader"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;
using namespace std;

// forward declare functions to use later.
bool Program_readLocalData(Object& obj, Input& fr);
bool Program_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_ProgramProxy
(
    new osg::Program,
    "Program",
    "Object StateAttribute Program",
    &Program_readLocalData,
    &Program_writeLocalData
);


bool Program_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Program& program = static_cast<Program&>(obj);

    while(fr.matchSequence("AttribBindingLocation %i %w"))
    {
        unsigned int index;
        fr[1].getUInt(index);
        program.addBindAttribLocation(fr[2].getStr(), index);
        fr += 3;
        iteratorAdvanced = true;
    }


    while(fr.matchSequence("AttribBindingLocation %w %i"))
    {
        unsigned int index;
        fr[2].getUInt(index);
        program.addBindAttribLocation(fr[1].getStr(), index);
        fr += 3;
        iteratorAdvanced = true;
    }

    int num_shaders;
    if (fr[0].matchWord("num_shaders") &&
        fr[1].getInt(num_shaders))
    {
        // could allocate space for shaders here...
        fr+=2;
        iteratorAdvanced = true;
    }

    Object* object = NULL;
    while((object=fr.readObject())!=NULL)
    {
        program.addShader(dynamic_cast<Shader*>(object));
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool Program_writeLocalData(const Object& obj,Output& fw)
{
    const Program& program = static_cast<const Program&>(obj);

    const Program::AttribBindingList& abl = program.getAttribBindingList();
    Program::AttribBindingList::const_iterator i;
    for(i=abl.begin(); i!=abl.end(); i++)
    {
        fw.indent() << "AttribBindingLocation " << (*i).first << " " << (*i).second << std::endl;
    }

    fw.indent() << "num_shaders " << program.getNumShaders() << std::endl;
    for(unsigned int ic=0;ic<program.getNumShaders();++ic)
    {
        fw.writeObject(*program.getShader(ic));
    }

    return true;
}
