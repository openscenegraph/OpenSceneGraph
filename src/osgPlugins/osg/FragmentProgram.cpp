#include "osg/FragmentProgram"

#include <iostream>
#include <sstream>
#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;
using namespace std;

// forward declare functions to use later.
bool FragmentProgram_readLocalData(Object& obj, Input& fr);
bool FragmentProgram_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_FragmentProgramProxy
(
    new osg::FragmentProgram,
    "FragmentProgram",
    "Object StateAttribute FragmentProgram",
    &FragmentProgram_readLocalData,
    &FragmentProgram_writeLocalData
);


bool FragmentProgram_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    FragmentProgram& fragmentProgram = static_cast<FragmentProgram&>(obj);

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
        fragmentProgram.setFragmentProgram(code);
    }
    return iteratorAdvanced;
}


bool FragmentProgram_writeLocalData(const Object& obj,Output& fw)
{
    const FragmentProgram& fragmentProgram = static_cast<const FragmentProgram&>(obj);

    std::vector<std::string> lines;
    std::istringstream iss(fragmentProgram.getFragmentProgram());
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
