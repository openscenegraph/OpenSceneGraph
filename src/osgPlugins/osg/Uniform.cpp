#include "osg/Uniform"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;
using namespace std;

// forward declare functions to use later.
bool Uniform_readLocalData(Object& obj, Input& fr);
bool Uniform_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_UniformProxy
(
    new osg::Uniform,
    "Uniform",
    "Object Uniform",
    &Uniform_readLocalData,
    &Uniform_writeLocalData
);


bool Uniform_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Uniform& uniform = static_cast<Uniform&>(obj);

    if (fr.matchSequence("type %w"))
    {
	uniform.setType( Uniform::getTypeId(fr[1].getStr()) );
	fr+=2;
	iteratorAdvanced = true;
    }

    if (fr.matchSequence("name %s"))
    {
        uniform.setName(fr[1].getStr());
        fr+=2;
        iteratorAdvanced = true;
    }

    // TODO read uniform value based on type

    return iteratorAdvanced;
}


bool Uniform_writeLocalData(const Object& obj,Output& fw)
{
    const Uniform& uniform = static_cast<const Uniform&>(obj);

    fw.indent() << "type " << Uniform::getTypename( uniform.getType() ) << std::endl;

    fw.indent() << "name "<< fw.wrapString(uniform.getName()) << std::endl;

    // TODO write uniform value based on type

    return true;
}
