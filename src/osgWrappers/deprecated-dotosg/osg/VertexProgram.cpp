#include "osg/VertexProgram"
#include <osg/io_utils>

#include <iostream>
#include <sstream>
#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"
#include "osgDB/fstream"

#include "Matrix.h"

using namespace osg;
using namespace osgDB;
using namespace std;

// forward declare functions to use later.
bool VertexProgram_readLocalData(Object& obj, Input& fr);
bool VertexProgram_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(VertexProgram)
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
        vertexProgram.setProgramLocalParameter(index, vec);
    }

    if (fr[0].matchWord("Matrix"))
    {
        int index;
        fr[1].getInt(index);
        fr += 2;
        osg::Matrix matrix;
        if (readMatrix(matrix,fr))
        {
            vertexProgram.setMatrix(index, matrix);
        }
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("code {")) {
        std::string code;
        fr += 2;
        iteratorAdvanced = true;
        int entry = fr[0].getNoNestedBrackets();
        while (!fr.eof() && fr[0].getNoNestedBrackets() >= entry)
        {
            if (fr[0].getStr())
            {
                code.append(std::string(fr[0].getStr()));
                code += '\n';
            }
            ++fr;
        }
        vertexProgram.setVertexProgram(code);
    }

    if( fr.matchSequence("file %s"))
    {
        std::string filename = fr[1].getStr();

        fr+=2;
        iteratorAdvanced = true;

        osgDB::ifstream vfstream( filename.c_str() );

        if( vfstream )
        {
            ostringstream vstream;
            char ch;

            /* xxx better way to transfer a ifstream to a string?? */
            while( vfstream.get(ch)) vstream.put(ch);

            vertexProgram.setVertexProgram( vstream.str() );
        }
    }

    return iteratorAdvanced;
}


bool VertexProgram_writeLocalData(const Object& obj,Output& fw)
{
    const VertexProgram& vertexProgram = static_cast<const VertexProgram&>(obj);

    const VertexProgram::LocalParamList& lpl = vertexProgram.getLocalParameters();
    VertexProgram::LocalParamList::const_iterator i;
    for(i=lpl.begin(); i!=lpl.end(); i++)
    {
        fw.indent() << "ProgramLocalParameter " << (*i).first << " " << (*i).second << std::endl;
    }

    const VertexProgram::MatrixList& mpl = vertexProgram.getMatrices();
    VertexProgram::MatrixList::const_iterator mi;
    for(mi=mpl.begin(); mi!=mpl.end(); mi++)
    {
        fw.indent() << "Matrix " << (*mi).first << " ";
        writeMatrix((*mi).second,fw);
    }


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
