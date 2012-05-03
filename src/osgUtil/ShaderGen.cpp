/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2009 Robert Osfield
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

/**
 * \brief    Shader generator framework.
 * \author   Maciej Krol
 */

#include <osgUtil/ShaderGen>
#include <osg/Geode>
#include <osg/Geometry> // for ShaderGenVisitor::update
#include <osg/Fog>
#include <sstream>

using namespace osgUtil;

namespace osgUtil
{

/// State extended by mode/attribute accessors
class StateEx : public osg::State
{
public:
    StateEx() : State() {}

    osg::StateAttribute::GLModeValue getMode(osg::StateAttribute::GLMode mode,
        osg::StateAttribute::GLModeValue def = osg::StateAttribute::INHERIT) const
    {
        return getMode(_modeMap, mode, def);
    }

    osg::StateAttribute *getAttribute(osg::StateAttribute::Type type, unsigned int member = 0) const
    {
        return getAttribute(_attributeMap, type, member);
    }

    osg::StateAttribute::GLModeValue getTextureMode(unsigned int unit,
        osg::StateAttribute::GLMode mode,
        osg::StateAttribute::GLModeValue def = osg::StateAttribute::INHERIT) const
    {
        return unit < _textureModeMapList.size() ? getMode(_textureModeMapList[unit], mode, def) : def;
    }

    osg::StateAttribute *getTextureAttribute(unsigned int unit, osg::StateAttribute::Type type) const
    {
        return unit < _textureAttributeMapList.size() ? getAttribute(_textureAttributeMapList[unit], type, 0) : 0;
    }

    osg::Uniform *getUniform(const std::string& name) const
    {
        UniformMap::const_iterator it = _uniformMap.find(name);
        return it != _uniformMap.end() ?
            const_cast<osg::Uniform *>(it->second.uniformVec.back().first) : 0;
    }

protected:

    osg::StateAttribute::GLModeValue getMode(const ModeMap &modeMap,
        osg::StateAttribute::GLMode mode,
        osg::StateAttribute::GLModeValue def = osg::StateAttribute::INHERIT) const
    {
        ModeMap::const_iterator it = modeMap.find(mode);
        return (it != modeMap.end() && it->second.valueVec.size()) ? it->second.valueVec.back() : def;
    }

    osg::StateAttribute *getAttribute(const AttributeMap &attributeMap,
        osg::StateAttribute::Type type, unsigned int member = 0) const
    {
        AttributeMap::const_iterator it = attributeMap.find(std::make_pair(type, member));
        return (it != attributeMap.end() && it->second.attributeVec.size()) ?
            const_cast<osg::StateAttribute*>(it->second.attributeVec.back().first) : 0;
    }
};

}

void ShaderGenCache::setStateSet(int stateMask, osg::StateSet *stateSet)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _stateSetMap[stateMask] = stateSet;
}

osg::StateSet *ShaderGenCache::getStateSet(int stateMask) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    StateSetMap::const_iterator it = _stateSetMap.find(stateMask);
    return (it != _stateSetMap.end()) ? it->second.get() : 0;
}

osg::StateSet *ShaderGenCache::getOrCreateStateSet(int stateMask)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    StateSetMap::iterator it = _stateSetMap.find(stateMask);
    if (it == _stateSetMap.end())
    {
        osg::StateSet *stateSet = createStateSet(stateMask);
        _stateSetMap.insert(it, StateSetMap::value_type(stateMask, stateSet));
        return stateSet;
    }
    return it->second.get();
}

osg::StateSet *ShaderGenCache::createStateSet(int stateMask) const
{
    osg::StateSet *stateSet = new osg::StateSet;
    osg::Program *program = new osg::Program;
    stateSet->setAttribute(program);

    std::ostringstream vert;
    std::ostringstream frag;

    // write varyings
    if ((stateMask & LIGHTING) && !(stateMask & NORMAL_MAP))
    {
        vert << "varying vec3 normalDir;\n";
    }

    if (stateMask & (LIGHTING | NORMAL_MAP))
    {
        vert << "varying vec3 lightDir;\n";
    }

    if (stateMask & (LIGHTING | NORMAL_MAP | FOG))
    {
        vert << "varying vec3 viewDir;\n";
    }

    // copy varying to fragment shader
    frag << vert.str();

    // write uniforms and attributes
    int unit = 0;
    if (stateMask & DIFFUSE_MAP)
    {
        osg::Uniform *diffuseMap = new osg::Uniform("diffuseMap", unit++);
        stateSet->addUniform(diffuseMap);
        frag << "uniform sampler2D diffuseMap;\n";
    }

    if (stateMask & NORMAL_MAP)
    {
        osg::Uniform *normalMap = new osg::Uniform("normalMap", unit++);
        stateSet->addUniform(normalMap);
        frag << "uniform sampler2D normalMap;\n";
        program->addBindAttribLocation("tangent", 6);
        vert << "attribute vec3 tangent;\n";
    }

    vert << "\n"\
        "void main()\n"\
        "{\n"\
        "  gl_Position = ftransform();\n";

    if (stateMask & (DIFFUSE_MAP | NORMAL_MAP))
    {
        vert << "  gl_TexCoord[0] = gl_MultiTexCoord0;\n";
    }

    if (stateMask & NORMAL_MAP)
    {
        vert <<
            "  vec3 n = gl_NormalMatrix * gl_Normal;\n"\
            "  vec3 t = gl_NormalMatrix * tangent;\n"\
            "  vec3 b = cross(n, t);\n"\
            "  vec3 dir = -vec3(gl_ModelViewMatrix * gl_Vertex);\n"\
            "  viewDir.x = dot(dir, t);\n"\
            "  viewDir.y = dot(dir, b);\n"\
            "  viewDir.z = dot(dir, n);\n"\
            "  vec4 lpos = gl_LightSource[0].position;\n"\
            "  if (lpos.w == 0.0)\n"\
            "    dir = lpos.xyz;\n"\
            "  else\n"\
            "    dir += lpos.xyz;\n"\
            "  lightDir.x = dot(dir, t);\n"\
            "  lightDir.y = dot(dir, b);\n"\
            "  lightDir.z = dot(dir, n);\n";
    }
    else if (stateMask & LIGHTING)
    {
        vert <<
            "  normalDir = gl_NormalMatrix * gl_Normal;\n"\
            "  vec3 dir = -vec3(gl_ModelViewMatrix * gl_Vertex);\n"\
            "  viewDir = dir;\n"\
            "  vec4 lpos = gl_LightSource[0].position;\n"\
            "  if (lpos.w == 0.0)\n"\
            "    lightDir = lpos.xyz;\n"\
            "  else\n"\
            "    lightDir = lpos.xyz + dir;\n";
    }
    else if (stateMask & FOG)
    {
        vert <<
            "  viewDir = -vec3(gl_ModelViewMatrix * gl_Vertex);\n"\
            "  gl_FrontColor = gl_Color;\n";
    }
    else
    {
        vert << "  gl_FrontColor = gl_Color;\n";
    }

    vert << "}\n";

    frag << "\n"\
        "void main()\n"\
        "{\n";

    if (stateMask & DIFFUSE_MAP)
    {
        frag << "  vec4 base = texture2D(diffuseMap, gl_TexCoord[0].st);\n";
    }
    else
    {
        frag << "  vec4 base = vec4(1.0);\n";
    }

    if (stateMask & NORMAL_MAP)
    {
        frag << "  vec3 normalDir = texture2D(normalMap, gl_TexCoord[0].st).xyz*2.0-1.0;\n";
    }

    if (stateMask & (LIGHTING | NORMAL_MAP))
    {
        frag <<
            "  vec3 nd = normalize(normalDir);\n"\
            "  vec3 ld = normalize(lightDir);\n"\
            "  vec3 vd = normalize(viewDir);\n"\
            "  vec4 color = gl_FrontLightModelProduct.sceneColor;\n"\
            "  color += gl_FrontLightProduct[0].ambient;\n"\
            "  float diff = max(dot(ld, nd), 0.0);\n"\
            "  color += gl_FrontLightProduct[0].diffuse * diff;\n"\
            "  color *= base;\n"\
            "  if (diff > 0.0)\n"\
            "  {\n"\
            "    vec3 halfDir = normalize(ld+vd);\n"\
            "    color.rgb += base.a * gl_FrontLightProduct[0].specular.rgb * \n"\
            "      pow(max(dot(halfDir, nd), 0.0), gl_FrontMaterial.shininess);\n"\
            "  }\n";
    }
    else
    {
        frag << "  vec4 color = base;\n";
    }

    if (!(stateMask & LIGHTING))
    {
        frag << "  color *= gl_Color;\n";
    }

    if (stateMask & FOG)
    {
        frag <<
            "  float d2 = dot(viewDir, viewDir);//gl_FragCoord.z/gl_FragCoord.w;\n"\
            "  float f = exp2(-1.442695*gl_Fog.density*gl_Fog.density*d2);\n"\
            "  color.rgb = mix(gl_Fog.color.rgb, color.rgb, clamp(f, 0.0, 1.0));\n";
    }

    frag << "  gl_FragColor = color;\n";
    frag << "}\n";

    std::string vertstr = vert.str();
    std::string fragstr = frag.str();

    OSG_DEBUG << "ShaderGenCache Vertex shader:\n" << vertstr << std::endl;
    OSG_DEBUG << "ShaderGenCache Fragment shader:\n" << fragstr << std::endl;

    program->addShader(new osg::Shader(osg::Shader::VERTEX, vertstr));
    program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragstr));

    return stateSet;
}

ShaderGenVisitor::ShaderGenVisitor() :
    NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
    _stateCache(new ShaderGenCache),
    _state(new StateEx)
{
}

ShaderGenVisitor::ShaderGenVisitor(ShaderGenCache *stateCache) :
    NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
    _stateCache(stateCache),
    _state(new StateEx)
{
}

void ShaderGenVisitor::setRootStateSet(osg::StateSet *stateSet)
{
    if (_rootStateSet.valid())
        _state->removeStateSet(0);
    _rootStateSet = stateSet;
    if (_rootStateSet.valid())
        _state->pushStateSet(_rootStateSet.get());
}

void ShaderGenVisitor::reset()
{
    _state->popAllStateSets();
    if (_rootStateSet.valid())
        _state->pushStateSet(_rootStateSet.get());
}

void ShaderGenVisitor::apply(osg::Node &node)
{
    osg::StateSet *stateSet = node.getStateSet();

    if (stateSet)
        _state->pushStateSet(stateSet);

    traverse(node);

    if (stateSet)
        _state->popStateSet();
}

void ShaderGenVisitor::apply(osg::Geode &geode)
{
    osg::StateSet *stateSet = geode.getStateSet();
    if (stateSet)
        _state->pushStateSet(stateSet);

    for (unsigned int i=0; i<geode.getNumDrawables(); ++i)
    {
        osg::Drawable *drawable = geode.getDrawable(i);
        osg::StateSet *ss = drawable->getStateSet();
        if (ss)
            _state->pushStateSet(ss);

        update(drawable);

        if (ss)
            _state->popStateSet();
    }

    if (stateSet)
        _state->popStateSet();
}

void ShaderGenVisitor::update(osg::Drawable *drawable)
{
    // update only geometry due to compatibility issues with user defined drawables
    osg::Geometry *geometry = drawable->asGeometry();
#if 0
    if (!geometry)
        return;
#endif

    StateEx *state = static_cast<StateEx *>(_state.get());
    // skip nodes without state sets
    if (state->getStateSetStackSize() == (_rootStateSet.valid() ? 1u : 0u))
        return;

    // skip state sets with already attached programs
    if (state->getAttribute(osg::StateAttribute::PROGRAM))
        return;

    int stateMask = 0;
    //if (state->getMode(GL_BLEND) & osg::StateAttribute::ON)
    //    stateMask |= ShaderGen::BLEND;
    if (state->getMode(GL_LIGHTING) & osg::StateAttribute::ON)
        stateMask |= ShaderGenCache::LIGHTING;
    if (state->getMode(GL_FOG) & osg::StateAttribute::ON)
        stateMask |= ShaderGenCache::FOG;
    if (state->getTextureAttribute(0, osg::StateAttribute::TEXTURE))
        stateMask |= ShaderGenCache::DIFFUSE_MAP;

    if (state->getTextureAttribute(1, osg::StateAttribute::TEXTURE) && geometry!=0 &&
        geometry->getVertexAttribArray(6)) //tangent
        stateMask |= ShaderGenCache::NORMAL_MAP;

    // Get program and uniforms for accumulated state.
    osg::StateSet *progss = _stateCache->getOrCreateStateSet(stateMask);
    // Set program and uniforms to the last state set.
    osg::StateSet *ss = const_cast<osg::StateSet *>(state->getStateSetStack().back());
    ss->setAttribute(progss->getAttribute(osg::StateAttribute::PROGRAM));
    ss->setUniformList(progss->getUniformList());

    // remove any modes that won't be appropriate when using shaders
    if ((stateMask&ShaderGenCache::LIGHTING)!=0)
    {
        ss->removeMode(GL_LIGHTING);
        ss->removeMode(GL_LIGHT0);
    }
    if ((stateMask&ShaderGenCache::FOG)!=0)
    {
        ss->removeMode(GL_FOG);
    }
    if ((stateMask&ShaderGenCache::DIFFUSE_MAP)!=0) ss->removeTextureMode(0, GL_TEXTURE_2D);
    if ((stateMask&ShaderGenCache::NORMAL_MAP)!=0) ss->removeTextureMode(1, GL_TEXTURE_2D);
}
