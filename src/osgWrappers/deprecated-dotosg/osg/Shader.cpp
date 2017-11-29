#include "osg/Shader"
#include "osg/Notify"

#include <iostream>
#include <sstream>

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"
#include "osgDB/FileUtils"
#include "osgDB/ReadFile"
#include "osgDB/WriteFile"

using namespace osg;
using namespace osgDB;
using namespace std;

// forward declare functions to use later.
bool Shader_readLocalData(Object& obj, Input& fr);
bool Shader_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Shader)
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

    if (fr.matchSequence("type %w"))
    {
        shader.setType( Shader::getTypeId(fr[1].getStr()) );
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("file %w") || fr.matchSequence("file %s") )
    {

        osg::ref_ptr<Shader> s = osgDB::readRefShaderFile(fr[1].getStr(), fr.getOptions());
        if(s.get()) shader.setShaderSource(s->getShaderSource());

        fr += 2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("code {"))
    {
        std::string code;
        fr += 2;
        iteratorAdvanced = true;
        int entry = fr[0].getNoNestedBrackets();
        while (!fr.eof() && fr[0].getNoNestedBrackets() >= entry)
        {
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

    // osg::notify(osg::NOTICE)<<"fw.getOutputShaderFiles()="<<fw.getOutputShaderFiles()<<std::endl;

    // check whenever output to shader files is requested
    if (fw.getOutputShaderFiles())
    {
        std::string fileName = shader.getFileName();

        if (fileName.empty())
        {
            fileName = fw.getShaderFileNameForOutput();
        }

        osgDB::writeShaderFile(shader, fileName);

        if (!fileName.empty())
        {
            fw.indent() << "file "<<fw.wrapString(fw.getFileNameForOutput(fileName))<< std::endl;
        }

    }
    else // no need to write shaders to external files, hence embed it
    {

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
            fw.indent() << fw.wrapString(*j) << "\n";
        }

        fw.moveOut();
        fw.indent() << "}\n";
    }

    return true;
}
