#include "osg/Program"
#include "osg/Shader"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;
using namespace std;

extern bool Geometry_matchPrimitiveModeStr(const char* str, GLenum& mode);
extern const char* Geometry_getPrimitiveModeStr(GLenum mode);

// forward declare functions to use later.
bool Program_readLocalData(Object& obj, Input& fr);
bool Program_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Program)
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
    
    if(fr.matchSequence("GeometryVerticesOut %i")) 
    {
        unsigned int verticesOut;
        fr[1].getUInt(verticesOut);
        program.setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, verticesOut);
        fr += 2;
        iteratorAdvanced = true;
    }
    
    if(fr.matchSequence("GeometryInputType %w")) 
    {
        std::string primitiveMode = fr[1].getStr();
        GLenum mode;
        if(Geometry_matchPrimitiveModeStr(primitiveMode.c_str(), mode))
            program.setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, mode);
        fr += 2;
        iteratorAdvanced = true;
    }
    
    if(fr.matchSequence("GeometryOutputType %w")) 
    {
        std::string primitiveMode = fr[1].getStr();
        GLenum mode;
        if(Geometry_matchPrimitiveModeStr(primitiveMode.c_str(), mode))
            program.setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, mode);
        fr += 2;
        iteratorAdvanced = true;
    }

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
    
    fw.indent() << "GeometryVerticesOut " << program.getParameter(GL_GEOMETRY_VERTICES_OUT_EXT) << std::endl;
    fw.indent() << "GeometryInputType " << Geometry_getPrimitiveModeStr(program.getParameter(GL_GEOMETRY_INPUT_TYPE_EXT)) << std::endl;
    fw.indent() << "GeometryOutputType " << Geometry_getPrimitiveModeStr(program.getParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT)) << std::endl;

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
