#include <osg/FragmentProgram>
#include <osg/io_utils>

#include <iostream>
#include <sstream>
#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

#include "Matrix.h"

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

    if (fr[0].matchWord("ProgramLocalParameter"))
    {
        int index;
        Vec4 vec;
        fr[1].getInt(index);
        fr[2].getFloat(vec[0]);
        fr[3].getFloat(vec[1]);
        fr[4].getFloat(vec[2]);
        fr[5].getFloat(vec[3]);
        fr += 6;
        iteratorAdvanced = true;
        fragmentProgram.setProgramLocalParameter(index, vec);
    }

    if (fr[0].matchWord("Matrix"))
    {
        int index;
        fr[1].getInt(index);
        fr += 2;
        osg::Matrix matrix;
        if (readMatrix(matrix,fr))
        {
            fragmentProgram.setMatrix(index, matrix);
        }
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("code {"))
    {
        std::string code;
        fr += 2;
        iteratorAdvanced = true;
        int entry = fr[0].getNoNestedBrackets();
        while (!fr.eof() && fr[0].getNoNestedBrackets() >= entry) {
            if (fr[0].getStr()) {
            code.append(std::string(fr[0].getStr()));
            code += '\n';
            }
            ++fr;
        }
        fragmentProgram.setFragmentProgram(code);
    }

    if( fr.matchSequence("file %s"))
    {
        std::string filename = fr[1].getStr();
        fr += 2;
        iteratorAdvanced = true;

        ifstream vfstream( filename.c_str() );

        if( vfstream ) {
            ostringstream vstream;
            char ch;

            /* xxx better way to transfer a ifstream to a string?? */
            while( vfstream.get(ch)) vstream.put(ch);

            fragmentProgram.setFragmentProgram( vstream.str() );
        }
    }
    
    return iteratorAdvanced;
}


bool FragmentProgram_writeLocalData(const Object& obj,Output& fw)
{
    const FragmentProgram& fragmentProgram = static_cast<const FragmentProgram&>(obj);

    const FragmentProgram::LocalParamList& lpl = fragmentProgram.getLocalParameters();
    FragmentProgram::LocalParamList::const_iterator i;
    for(i=lpl.begin(); i!=lpl.end(); i++)
    {
        fw.indent() << "ProgramLocalParameter " << (*i).first << " " << (*i).second << std::endl;
    }

    const FragmentProgram::MatrixList& mpl = fragmentProgram.getMatrices();
    FragmentProgram::MatrixList::const_iterator mi;
    for(mi=mpl.begin(); mi!=mpl.end(); mi++)
    {
        fw.indent() << "Matrix " << (*mi).first << " ";
        writeMatrix((*mi).second,fw);
    }

    std::vector<std::string> lines;
    std::istringstream iss(fragmentProgram.getFragmentProgram());
    std::string line;
    while (std::getline(iss, line))
    {
        lines.push_back(line);
    }

    fw.indent() << "code {\n";
    fw.moveIn();

    std::vector<std::string>::const_iterator j;
    for (j=lines.begin(); j!=lines.end(); ++j)
    {
        fw.indent() << "\"" << *j << "\"\n";
    }

    fw.moveOut();
    fw.indent() << "}\n";

    return true;
}
