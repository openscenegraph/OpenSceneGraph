#include "osg/Shader"

#include <iostream>
#include <sstream>
#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;
using namespace std;

// forward declare functions to use later.
bool Shader_readLocalData(Object& obj, Input& fr);
bool Shader_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_ShaderProxy
(
    new osg::Shader,
    "Shader",
    "Object Shader",
    &Shader_readLocalData,
    &Shader_writeLocalData
);


bool Shader_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Shader& shader = static_cast<Shader&>(obj);

    if (fr.matchSequence("type %s"))
    {
	shader.setType( Shader::getTypeId(fr[1].getStr()) );
	fr+=2;
	iteratorAdvanced = true;
    }

    if (fr.matchSequence("name %s"))
    {
        shader.setName(fr[1].getStr());
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("code {")) {
	std::string code;
	fr += 2;
	iteratorAdvanced = true;
	int entry = fr[0].getNoNestedBrackets();
	while (!fr.eof() && fr[0].getNoNestedBrackets() >= entry) {
	    if (fr[0].getStr()) {
		code.append(std::string(fr[0].getStr()));
		code += '\n' ;
	    }
	    ++fr;
	}
        shader.setShaderSource(code.c_str());
    }

    return iteratorAdvanced;
}


bool Shader_writeLocalData(const Object& obj,Output& fw)
{
    const Shader& shader = static_cast<const Shader&>(obj);

    fw.indent() << "type " << shader.getTypename() << std::endl;

    if (!shader.getName().empty()) fw.indent() << "name "<<fw.wrapString(shader.getName())<< std::endl;

    // split source text into individual lines
    std::vector<std::string> lines;
    std::istringstream iss(shader.getShaderSource());
    std::string line;
    while (std::getline(iss, line)) {
	lines.push_back(line);
    }

    fw.indent() << "code {\n";
    fw.moveIn();

    std::vector<std::string>::const_iterator j;
    for (j=lines.begin(); j!=lines.end(); ++j) {
	fw.indent() << "\"" << *j << "\"\n";
    }

    fw.moveOut();
    fw.indent() << "}\n";

    return true;
}
