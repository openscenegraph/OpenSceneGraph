/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 * Copyright (C) 2010-10-21 VIRES Simulationstechnologie GmbH
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

/*
 * mod:        Holger Helmich 2010-10-21
 */

#include <osg/ShaderComposer>
#include <osg/Notify>

using namespace osg;

ShaderComposer::ShaderComposer()
{
    OSG_INFO<<"ShaderComposer::ShaderComposer() "<<this<<std::endl;
}

ShaderComposer::ShaderComposer(const ShaderComposer& sa, const CopyOp& copyop):
    Object(sa, copyop)
{
    OSG_INFO<<"ShaderComposer::ShaderComposer(const ShaderComposer&, const CopyOp& copyop) "<<this<<std::endl;
}

ShaderComposer::~ShaderComposer()
{
    OSG_INFO<<"ShaderComposer::~ShaderComposer() "<<this<<std::endl;
}

void ShaderComposer::releaseGLObjects(osg::State* state) const
{
    for(ProgramMap::const_iterator itr = _programMap.begin();
        itr != _programMap.end();
        ++itr)
    {
        itr->second->releaseGLObjects(state);
    }

    for(ShaderMainMap::const_iterator itr = _shaderMainMap.begin();
        itr != _shaderMainMap.end();
        ++itr)
    {
        itr->second->releaseGLObjects(state);
    }
}

osg::Program* ShaderComposer::getOrCreateProgram(const ShaderComponents& shaderComponents)
{
    ProgramMap::iterator pmitr = _programMap.find(shaderComponents);
    if (pmitr != _programMap.end())
    {
        // OSG_NOTICE<<"ShaderComposer::getOrCreateProgram(..) using cached Program"<<std::endl;
        return pmitr->second.get();
    }

    // strip out vertex shaders
    Shaders vertexShaders;
    Shaders tessControlShaders;
    Shaders tessEvaluationShaders;
    Shaders geometryShaders;
    Shaders fragmentShaders;
    Shaders computeShaders;

    OSG_NOTICE<<"ShaderComposer::getOrCreateProgram(shaderComponents.size()=="<<shaderComponents.size()<<std::endl;

    for(ShaderComponents::const_iterator itr = shaderComponents.begin();
        itr != shaderComponents.end();
        ++itr)
    {
        const ShaderComponent* sc = *itr;

        for(unsigned int i=0; i<sc->getNumShaders(); ++i)
        {
            const Shader* shader = sc->getShader(i);
            switch(shader->getType())
            {
                case(Shader::VERTEX):
                    vertexShaders.push_back(shader);
                    break;
                case(Shader::TESSCONTROL):
                    tessControlShaders.push_back(shader);
                    break;
                case(Shader::TESSEVALUATION):
                    tessEvaluationShaders.push_back(shader);
                    break;
                case(Shader::GEOMETRY):
                    geometryShaders.push_back(shader);
                    break;
                case(Shader::FRAGMENT):
                    fragmentShaders.push_back(shader);
                    break;
                case(Shader::COMPUTE):
                    computeShaders.push_back(shader);
                    break;
                case(Shader::UNDEFINED):
                    OSG_WARN<<"Warning: ShaderCompose::getOrCreateProgam(ShaderComponts) encounterd invalid Shader::Type."<<std::endl;
                    break;
            }
        }
    }

    osg::ref_ptr<osg::Program> program = new osg::Program;

    if (!vertexShaders.empty())
    {
        addShaderToProgram(program.get(), vertexShaders);
    }

     if (!tessControlShaders.empty())
    {
        addShaderToProgram(program.get(), tessControlShaders);
    }

    if (!geometryShaders.empty())
    {
        addShaderToProgram(program.get(), geometryShaders);
    }

     if (!tessEvaluationShaders.empty())
    {
        addShaderToProgram(program.get(), tessEvaluationShaders);
    }

    if (!fragmentShaders.empty())
    {
        addShaderToProgram(program.get(), fragmentShaders);
    }

    if (!computeShaders.empty())
    {
        addShaderToProgram(program.get(), computeShaders);
    }

    // assign newly created program to map.
    _programMap[shaderComponents] = program;

    OSG_NOTICE<<"ShaderComposer::getOrCreateProgram(..) created new Program"<<std::endl;

    return program.get();
}

void ShaderComposer::addShaderToProgram(Program* program, const Shaders& shaders)
{
    ShaderMainMap::iterator smitr = _shaderMainMap.find(shaders);
    if (smitr == _shaderMainMap.end())
    {
        // no vertex shader in map yet, need to compose a new main shader
        osg::Shader* mainShader = composeMain(shaders);
        _shaderMainMap[shaders] = mainShader;
        program->addShader(mainShader);
    }
    else
    {
        program->addShader(smitr->second.get());
    }

    for(Shaders::const_iterator itr = shaders.begin();
        itr != shaders.end();
        ++itr)
    {
        Shader* shader = const_cast<Shader*>(*itr);
        if (!(shader->getShaderSource().empty()) || shader->getShaderBinary())
        {
            program->addShader(shader);
        }
    }
}

osg::Shader* ShaderComposer::composeMain(const Shaders& shaders)
{
    OSG_NOTICE<<"ShaderComposer::composeMain(Shaders) shaders.size()=="<<shaders.size()<<std::endl;


    // collect the shader type and the code injection from each of the contributing shaders
    Shader::Type type = Shader::UNDEFINED;
    Shader::CodeInjectionMap codeInjectionMap;
    for(Shaders::const_iterator itr = shaders.begin();
        itr != shaders.end();
        ++itr)
    {
        const Shader* shader = *itr;
        if (type == Shader::UNDEFINED)
        {
            type = shader->getType();
        }
        else if (type != shader->getType())
        {
            OSG_NOTICE<<"Warning:ShaderComposer::composeMain() mixing different types of Shaders prohibited."<<std::endl;
            continue;
        }

        const Shader::CodeInjectionMap& cim = shader->getCodeInjectionMap();
        for(Shader::CodeInjectionMap::const_iterator citr = cim.begin();
            citr != cim.end();
            ++citr)
        {
            codeInjectionMap.insert(*citr);
        }
    }

    // collect together the different parts of the main shader
    std::string before_main;
    std::string in_main;
    std::string after_main;

    for(Shader::CodeInjectionMap::iterator citr = codeInjectionMap.begin();
        citr != codeInjectionMap.end();
        ++citr)
    {
        float position = citr->first;
        if (position<0.0) before_main += citr->second;
        else if (position<=1.0) in_main += citr->second;
        else after_main += citr->second;
    }

    // assembly the final main shader source
    std::string full_source;
    full_source += before_main;
    full_source += std::string("void main(void)\n");
    full_source += std::string("{\n");
    full_source += in_main;
    full_source += std::string("}\n");
    full_source += after_main;


    ref_ptr<Shader> mainShader = new Shader(type, full_source);

    OSG_NOTICE<<"type =="<<type<<std::endl;
    OSG_NOTICE<<"full_source == "<<std::endl<<full_source<<std::endl;

    OSG_NOTICE<<"end of ShaderComposer::composeMain(Shaders)"<<std::endl<<std::endl;

    _shaderMainMap[shaders] = mainShader;

    return mainShader.get();
}
