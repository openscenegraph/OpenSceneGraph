/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
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

osg::Program* ShaderComposer::getOrCreateProgram(const ShaderComponents& shaderComponents)
{
    ProgramMap::iterator itr = _programMap.find(shaderComponents);
    if (itr != _programMap.end())
    {
        OSG_NOTICE<<"ShaderComposer::getOrCreateProgram(..) using cached Program"<<std::endl;
        return itr->second.get();
    }

    // strip out vertex shaders
    Shaders vertexShaders;
    Shaders geometryShaders;
    Shaders fragmentShaders;

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
                case(Shader::GEOMETRY):
                    geometryShaders.push_back(shader);
                    break;
                case(Shader::FRAGMENT):
                    fragmentShaders.push_back(shader);
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
        ShaderMainMap::iterator itr = _shaderMainMap.find(vertexShaders);
        if (itr == _shaderMainMap.end())
        {
            // no vertex shader in map yet, need to compose a new main shader
            osg::Shader* mainShader = composeMain(vertexShaders);
            _shaderMainMap[vertexShaders] = mainShader;
            program->addShader(mainShader);
        }
        else
        {
            program->addShader(itr->second.get());
        }
    }

    if (!geometryShaders.empty())
    {
        ShaderMainMap::iterator itr = _shaderMainMap.find(geometryShaders);
        if (itr == _shaderMainMap.end())
        {
            // no vertex shader in map yet, need to compose a new main shader
            osg::Shader* mainShader = composeMain(geometryShaders);
            _shaderMainMap[geometryShaders] = mainShader;
            program->addShader(mainShader);
        }
        else
        {
            program->addShader(itr->second.get());
        }
    }


    if (!fragmentShaders.empty())
    {
        ShaderMainMap::iterator itr = _shaderMainMap.find(fragmentShaders);
        if (itr == _shaderMainMap.end())
        {
            // no vertex shader in map yet, need to compose a new main shader
            osg::Shader* mainShader = composeMain(fragmentShaders);
            _shaderMainMap[fragmentShaders] = mainShader;
            program->addShader(mainShader);
        }
        else
        {
            program->addShader(itr->second.get());
        }
    }


    // assign newly created program to map.
    _programMap[shaderComponents] = program;

    OSG_NOTICE<<"ShaderComposer::getOrCreateProgram(..) created new Program"<<std::endl;

    return program.get();
}

osg::Shader* ShaderComposer::composeMain(const Shaders& shaders)
{
    OSG_NOTICE<<"ShaderComposer::composeMain(Shaders) shaders.size()=="<<shaders.size()<<std::endl;
    return 0;
}
