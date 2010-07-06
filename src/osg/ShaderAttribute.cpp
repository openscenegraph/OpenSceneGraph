/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2004 Robert Osfield
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

#include <osg/ShaderAttribute>
#include <osg/Notify>
#include <osg/StateSet>
#include <osg/State>

using namespace osg;


ShaderAttribute::ShaderAttribute()
{
    _shaderComponent = new osg::ShaderComponent;
}

ShaderAttribute::ShaderAttribute(const ShaderAttribute& sa,const CopyOp& copyop):
    StateAttribute(sa,copyop),
    _type(sa._type),
    _uniforms(sa._uniforms)
{
}

ShaderAttribute::~ShaderAttribute()
{
}

int ShaderAttribute::compare(const StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Parameter macros below.
    COMPARE_StateAttribute_Types(ShaderAttribute,sa)

    // check if types are same
    if (_type < rhs._type) return -1;
    if (_type > rhs._type) return 1;

    // all properties are the equal so return 0.
    return 0;
}

void ShaderAttribute::setType(Type type)
{
    _type = type;
}

unsigned int ShaderAttribute::addUniform(Uniform* uniform)
{
    // check to see if uniform already add, if so return the index of it
    for(unsigned int i=0; i<_uniforms.size(); ++i)
    {
        if (_uniforms[i] == uniform) return i;
    }

    // add uniform and return it's position
    _uniforms.push_back(uniform);
    return _uniforms.size()-1;
}

void ShaderAttribute::removeUniform(unsigned int i)
{
    _uniforms.erase(_uniforms.begin()+i);
}

bool ShaderAttribute::getModeUsage(StateAttribute::ModeUsage& usage) const
{
    OSG_NOTICE<<"ShaderAttribute::getModeUsage(..)"<<std::endl;
    return false;
}

void ShaderAttribute::apply(State& state) const
{
    for(Uniforms::const_iterator itr = _uniforms.begin();
        itr != _uniforms.end();
        ++itr)
    {
        state.applyShaderCompositionUniform(itr->get());
    }
}

void ShaderAttribute::compose(ShaderComposer& composer) const
{
    OSG_NOTICE<<"ShaderAttribute::compose(..)"<<std::endl;
}

void ShaderAttribute::compileGLObjects(State&) const
{
    OSG_NOTICE<<"ShaderAttribute::compileGLObjects(..)"<<std::endl;
}

void ShaderAttribute::resizeGLObjectBuffers(unsigned int maxSize)
{
    OSG_NOTICE<<"ShaderAttribute::resizeGLObjectBuffers(..)"<<std::endl;
}

void ShaderAttribute::releaseGLObjects(State* state) const
{
    OSG_NOTICE<<"ShaderAttribute::releaseGLObjects(..)"<<std::endl;
}
