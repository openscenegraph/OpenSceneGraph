#include "osg/VertexProgram"

#include <iostream>
#include <sstream>
#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;
using namespace std;

// forward declare functions to use later.
bool VertexProgram_readLocalData(Object& obj, Input& fr);
bool VertexProgram_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_VertexProgramProxy
(
    new osg::VertexProgram,
    "VertexProgram",
    "Object StateAttribute VertexProgram",
    &VertexProgram_readLocalData,
    &VertexProgram_writeLocalData
);


bool VertexProgram_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    VertexProgram& vertexProgram = static_cast<VertexProgram&>(obj);

    if (fr.matchSequence("code {")) {
	std::string code;
	fr += 2;
	iteratorAdvanced = true;
	int entry = fr[0].getNoNestedBrackets();
	while (!fr.eof() && fr[0].getNoNestedBrackets() >= entry) {
	    if (fr[0].getStr()) {
		code.append(std::string(fr[0].getStr()));
		code.push_back('\n');
	    }
	    ++fr;
	}
        vertexProgram.setVertexProgram(code);
    }
    return iteratorAdvanced;
}


bool VertexProgram_writeLocalData(const Object& obj,Output& fw)
{
    const VertexProgram& vertexProgram = static_cast<const VertexProgram&>(obj);

    std::vector<std::string> lines;
    std::istringstream iss(vertexProgram.getVertexProgram());
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
