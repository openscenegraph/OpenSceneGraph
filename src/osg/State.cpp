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
#include <osg/State>
#include <osg/Texture>
#include <osg/Notify>
#include <osg/GLU>
#include <osg/GLExtensions>
#include <osg/Drawable>
#include <osg/ApplicationUsage>

#include <sstream>
#include <algorithm>

#ifndef GL_MAX_TEXTURE_COORDS
#define GL_MAX_TEXTURE_COORDS 0x8871
#endif

#ifndef GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#endif

#ifndef GL_MAX_TEXTURE_UNITS
#define GL_MAX_TEXTURE_UNITS 0x84E2
#endif

using namespace std;
using namespace osg;

static ApplicationUsageProxy State_e0(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_GL_ERROR_CHECKING <type>","ONCE_PER_ATTRIBUTE | ON | on enables fine grained checking,  ONCE_PER_FRAME enables coarse grained checking");

State::State():
    Referenced(true)
{
    _graphicsContext = 0;
    _contextID = 0;

    _shaderCompositionEnabled = false;
    _shaderCompositionDirty = true;
    _shaderComposer = new ShaderComposer;
    _currentShaderCompositionProgram = 0L;

    _identity = new osg::RefMatrix(); // default RefMatrix constructs to identity.
    _initialViewMatrix = _identity;
    _projection = _identity;
    _modelView = _identity;
    _modelViewCache = new osg::RefMatrix;

    #if !defined(OSG_GL_FIXED_FUNCTION_AVAILABLE)
        _useModelViewAndProjectionUniforms = true;
        _useVertexAttributeAliasing = true;
    #else
        _useModelViewAndProjectionUniforms = false;
        _useVertexAttributeAliasing = false;
    #endif

    _modelViewMatrixUniform = new Uniform(Uniform::FLOAT_MAT4,"osg_ModelViewMatrix");
    _projectionMatrixUniform = new Uniform(Uniform::FLOAT_MAT4,"osg_ProjectionMatrix");
    _modelViewProjectionMatrixUniform = new Uniform(Uniform::FLOAT_MAT4,"osg_ModelViewProjectionMatrix");
    _normalMatrixUniform = new Uniform(Uniform::FLOAT_MAT3,"osg_NormalMatrix");

    resetVertexAttributeAlias();

    _abortRenderingPtr = NULL;

    _checkGLErrors = ONCE_PER_FRAME;

    const char* str = getenv("OSG_GL_ERROR_CHECKING");
    if (str && (strcmp(str,"ONCE_PER_ATTRIBUTE")==0 || strcmp(str,"ON")==0 || strcmp(str,"on")==0))
    {
        _checkGLErrors = ONCE_PER_ATTRIBUTE;
    }

    _currentActiveTextureUnit=0;
    _currentClientActiveTextureUnit=0;

    _currentVBO = 0;
    _currentEBO = 0;
    _currentPBO = 0;

    _isSecondaryColorSupportResolved = false;
    _isSecondaryColorSupported = false;

    _isFogCoordSupportResolved = false;
    _isFogCoordSupported = false;

    _isVertexBufferObjectSupportResolved = false;
    _isVertexBufferObjectSupported = false;

    _lastAppliedProgramObject = 0;

    _extensionProcsInitialized = false;
    _glClientActiveTexture = 0;
    _glActiveTexture = 0;
    _glFogCoordPointer = 0;
    _glSecondaryColorPointer = 0;
    _glVertexAttribPointer = 0;
    _glVertexAttribIPointer = 0;
    _glVertexAttribLPointer = 0;
    _glEnableVertexAttribArray = 0;
    _glDisableVertexAttribArray = 0;
    _glDrawArraysInstanced = 0;
    _glDrawElementsInstanced = 0;

    _dynamicObjectCount  = 0;

    _glMaxTextureCoords = 1;
    _glMaxTextureUnits = 1;

    _maxTexturePoolSize = 0;
    _maxBufferObjectPoolSize = 0;

    _glBeginEndAdapter.setState(this);
    _arrayDispatchers.setState(this);

    _graphicsCostEstimator = new GraphicsCostEstimator;

    _startTick = 0;
    _gpuTick = 0;
    _gpuTimestamp = 0;
    _timestampBits = 0;
}

State::~State()
{
    // delete the GLExtensions object associated with this osg::State.
    if (_glExtensions)
    {
        GLExtensions::Set(_contextID, 0);
        _glExtensions = 0;
    }

    //_texCoordArrayList.clear();

    //_vertexAttribArrayList.clear();
}

void State::releaseGLObjects()
{
    // release any GL objects held by the shader composer
    _shaderComposer->releaseGLObjects(this);

    // release any StateSet's on the stack
    for(StateSetStack::iterator itr = _stateStateStack.begin();
        itr != _stateStateStack.end();
        ++itr)
    {
        (*itr)->releaseGLObjects(this);
    }

    _modeMap.clear();
    _textureModeMapList.clear();

    // release any cached attributes
    for(AttributeMap::iterator aitr = _attributeMap.begin();
        aitr != _attributeMap.end();
        ++aitr)
    {
        AttributeStack& as = aitr->second;
        if (as.global_default_attribute.valid())
        {
            as.global_default_attribute->releaseGLObjects(this);
        }
    }
    _attributeMap.clear();

    // release any cached texture attributes
    for(TextureAttributeMapList::iterator itr = _textureAttributeMapList.begin();
        itr != _textureAttributeMapList.end();
        ++itr)
    {
        AttributeMap& attributeMap = *itr;
        for(AttributeMap::iterator aitr = attributeMap.begin();
            aitr != attributeMap.end();
            ++aitr)
        {
            AttributeStack& as = aitr->second;
            if (as.global_default_attribute.valid())
            {
                as.global_default_attribute->releaseGLObjects(this);
            }
        }
    }

    _textureAttributeMapList.clear();
}

void State::reset()
{
#if 1
    for(ModeMap::iterator mitr=_modeMap.begin();
        mitr!=_modeMap.end();
        ++mitr)
    {
        ModeStack& ms = mitr->second;
        ms.valueVec.clear();
        ms.last_applied_value = !ms.global_default_value;
        ms.changed = true;
    }
#else
    _modeMap.clear();
#endif

    _modeMap[GL_DEPTH_TEST].global_default_value = true;
    _modeMap[GL_DEPTH_TEST].changed = true;

    // go through all active StateAttribute's, setting to change to force update,
    // the idea is to leave only the global defaults left.
    for(AttributeMap::iterator aitr=_attributeMap.begin();
        aitr!=_attributeMap.end();
        ++aitr)
    {
        AttributeStack& as = aitr->second;
        as.attributeVec.clear();
        as.last_applied_attribute = NULL;
        as.last_applied_shadercomponent = NULL;
        as.changed = true;
    }

    // we can do a straight clear, we arn't interested in GL_DEPTH_TEST defaults in texture modes.
    for(TextureModeMapList::iterator tmmItr=_textureModeMapList.begin();
        tmmItr!=_textureModeMapList.end();
        ++tmmItr)
    {
        tmmItr->clear();
    }

    // empty all the texture attributes as per normal attributes, leaving only the global defaults left.
    for(TextureAttributeMapList::iterator tamItr=_textureAttributeMapList.begin();
        tamItr!=_textureAttributeMapList.end();
        ++tamItr)
    {
        AttributeMap& attributeMap = *tamItr;
        // go through all active StateAttribute's, setting to change to force update.
        for(AttributeMap::iterator aitr=attributeMap.begin();
            aitr!=attributeMap.end();
            ++aitr)
        {
            AttributeStack& as = aitr->second;
            as.attributeVec.clear();
            as.last_applied_attribute = NULL;
            as.last_applied_shadercomponent = NULL;
            as.changed = true;
        }
    }

    _stateStateStack.clear();

    _modelView = _identity;
    _projection = _identity;

    dirtyAllVertexArrays();

#if 0
    // reset active texture unit values and call OpenGL
    // note, this OpenGL op precludes the use of State::reset() without a
    // valid graphics context, therefore the new implementation below
    // is preferred.
    setActiveTextureUnit(0);
#else
    // reset active texture unit values without calling OpenGL
    _currentActiveTextureUnit = 0;
    _currentClientActiveTextureUnit = 0;
#endif

    _shaderCompositionDirty = true;
    _currentShaderCompositionUniformList.clear();

    _lastAppliedProgramObject = 0;

    // what about uniforms??? need to clear them too...
    // go through all active Uniform's, setting to change to force update,
    // the idea is to leave only the global defaults left.
    for(UniformMap::iterator uitr=_uniformMap.begin();
        uitr!=_uniformMap.end();
        ++uitr)
    {
        UniformStack& us = uitr->second;
        us.uniformVec.clear();
    }

}

void State::setInitialViewMatrix(const osg::RefMatrix* matrix)
{
    if (matrix) _initialViewMatrix = matrix;
    else _initialViewMatrix = _identity;

    _initialInverseViewMatrix.invert(*_initialViewMatrix);
}

void State::setMaxTexturePoolSize(unsigned int size)
{
    _maxTexturePoolSize = size;
    osg::Texture::getTextureObjectManager(getContextID())->setMaxTexturePoolSize(size);
    OSG_INFO<<"osg::State::_maxTexturePoolSize="<<_maxTexturePoolSize<<std::endl;
}

void State::setMaxBufferObjectPoolSize(unsigned int size)
{
    _maxBufferObjectPoolSize = size;
    osg::GLBufferObjectManager::getGLBufferObjectManager(getContextID())->setMaxGLBufferObjectPoolSize(_maxBufferObjectPoolSize);
    OSG_INFO<<"osg::State::_maxBufferObjectPoolSize="<<_maxBufferObjectPoolSize<<std::endl;
}

void State::pushStateSet(const StateSet* dstate)
{

    _stateStateStack.push_back(dstate);
    if (dstate)
    {

        pushModeList(_modeMap,dstate->getModeList());

        // iterator through texture modes.
        unsigned int unit;
        const StateSet::TextureModeList& ds_textureModeList = dstate->getTextureModeList();
        for(unit=0;unit<ds_textureModeList.size();++unit)
        {
            pushModeList(getOrCreateTextureModeMap(unit),ds_textureModeList[unit]);
        }

        pushAttributeList(_attributeMap,dstate->getAttributeList());

        // iterator through texture attributes.
        const StateSet::TextureAttributeList& ds_textureAttributeList = dstate->getTextureAttributeList();
        for(unit=0;unit<ds_textureAttributeList.size();++unit)
        {
            pushAttributeList(getOrCreateTextureAttributeMap(unit),ds_textureAttributeList[unit]);
        }

        pushUniformList(_uniformMap,dstate->getUniformList());

        pushDefineList(_defineMap,dstate->getDefineList());
    }

    // OSG_NOTICE<<"State::pushStateSet()"<<_stateStateStack.size()<<std::endl;
}

void State::popAllStateSets()
{
    // OSG_NOTICE<<"State::popAllStateSets()"<<_stateStateStack.size()<<std::endl;

    while (!_stateStateStack.empty()) popStateSet();

    applyProjectionMatrix(0);
    applyModelViewMatrix(0);

    _lastAppliedProgramObject = 0;
}

void State::popStateSet()
{
    // OSG_NOTICE<<"State::popStateSet()"<<_stateStateStack.size()<<std::endl;

    if (_stateStateStack.empty()) return;


    const StateSet* dstate = _stateStateStack.back();

    if (dstate)
    {

        popModeList(_modeMap,dstate->getModeList());

        // iterator through texture modes.
        unsigned int unit;
        const StateSet::TextureModeList& ds_textureModeList = dstate->getTextureModeList();
        for(unit=0;unit<ds_textureModeList.size();++unit)
        {
            popModeList(getOrCreateTextureModeMap(unit),ds_textureModeList[unit]);
        }

        popAttributeList(_attributeMap,dstate->getAttributeList());

        // iterator through texture attributes.
        const StateSet::TextureAttributeList& ds_textureAttributeList = dstate->getTextureAttributeList();
        for(unit=0;unit<ds_textureAttributeList.size();++unit)
        {
            popAttributeList(getOrCreateTextureAttributeMap(unit),ds_textureAttributeList[unit]);
        }

        popUniformList(_uniformMap,dstate->getUniformList());

        popDefineList(_defineMap,dstate->getDefineList());

    }

    // remove the top draw state from the stack.
    _stateStateStack.pop_back();
}

void State::insertStateSet(unsigned int pos,const StateSet* dstate)
{
    StateSetStack tempStack;

    // first pop the StateSet above the position we need to insert at
    while (_stateStateStack.size()>pos)
    {
        tempStack.push_back(_stateStateStack.back());
        popStateSet();
    }

    // push our new stateset
    pushStateSet(dstate);

    // push back the original ones
    for(StateSetStack::reverse_iterator itr = tempStack.rbegin();
        itr != tempStack.rend();
        ++itr)
    {
        pushStateSet(*itr);
    }

}

void State::removeStateSet(unsigned int pos)
{
    if (pos >= _stateStateStack.size())
    {
        OSG_NOTICE<<"Warning: State::removeStateSet("<<pos<<") out of range"<<std::endl;
        return;
    }

    // record the StateSet above the one we intend to remove
    StateSetStack tempStack;
    while (_stateStateStack.size()-1>pos)
    {
        tempStack.push_back(_stateStateStack.back());
        popStateSet();
    }

    // remove the intended StateSet as well
    popStateSet();

    // push back the original ones that were above the remove StateSet
    for(StateSetStack::reverse_iterator itr = tempStack.rbegin();
        itr != tempStack.rend();
        ++itr)
    {
        pushStateSet(*itr);
    }
}

void State::captureCurrentState(StateSet& stateset) const
{
    // empty the stateset first.
    stateset.clear();

    for(ModeMap::const_iterator mitr=_modeMap.begin();
        mitr!=_modeMap.end();
        ++mitr)
    {
        // note GLMode = mitr->first
        const ModeStack& ms = mitr->second;
        if (!ms.valueVec.empty())
        {
            stateset.setMode(mitr->first,ms.valueVec.back());
        }
    }

    for(AttributeMap::const_iterator aitr=_attributeMap.begin();
        aitr!=_attributeMap.end();
        ++aitr)
    {
        const AttributeStack& as = aitr->second;
        if (!as.attributeVec.empty())
        {
            stateset.setAttribute(const_cast<StateAttribute*>(as.attributeVec.back().first));
        }
    }

}

void State::apply(const StateSet* dstate)
{
    if (_checkGLErrors==ONCE_PER_ATTRIBUTE) checkGLErrors("start of State::apply(StateSet*)");

    // equivalent to:
    //pushStateSet(dstate);
    //apply();
    //popStateSet();
    //return;

    if (dstate)
    {
        // push the stateset on the stack so it can be querried from within StateAttribute
        _stateStateStack.push_back(dstate);

        _currentShaderCompositionUniformList.clear();

        // apply all texture state and modes
        const StateSet::TextureModeList& ds_textureModeList = dstate->getTextureModeList();
        const StateSet::TextureAttributeList& ds_textureAttributeList = dstate->getTextureAttributeList();

        unsigned int unit;
        unsigned int unitMax = maximum(static_cast<unsigned int>(ds_textureModeList.size()),static_cast<unsigned int>(ds_textureAttributeList.size()));
        unitMax = maximum(static_cast<unsigned int>(unitMax),static_cast<unsigned int>(_textureModeMapList.size()));
        unitMax = maximum(static_cast<unsigned int>(unitMax),static_cast<unsigned int>(_textureAttributeMapList.size()));
        for(unit=0;unit<unitMax;++unit)
        {
            if (unit<ds_textureModeList.size()) applyModeListOnTexUnit(unit,getOrCreateTextureModeMap(unit),ds_textureModeList[unit]);
            else if (unit<_textureModeMapList.size()) applyModeMapOnTexUnit(unit,_textureModeMapList[unit]);

            if (unit<ds_textureAttributeList.size()) applyAttributeListOnTexUnit(unit,getOrCreateTextureAttributeMap(unit),ds_textureAttributeList[unit]);
            else if (unit<_textureAttributeMapList.size()) applyAttributeMapOnTexUnit(unit,_textureAttributeMapList[unit]);
        }

        const Program::PerContextProgram* previousLastAppliedProgramObject = _lastAppliedProgramObject;

        applyModeList(_modeMap,dstate->getModeList());
#if 1
        pushDefineList(_defineMap, dstate->getDefineList());
#else
        applyDefineList(_defineMap, dstate->getDefineList());
#endif

        applyAttributeList(_attributeMap,dstate->getAttributeList());

        if ((_lastAppliedProgramObject!=0) && (previousLastAppliedProgramObject==_lastAppliedProgramObject) && _defineMap.changed)
        {
            // OSG_NOTICE<<"State::apply(StateSet*) Program already applied ("<<(previousLastAppliedProgramObject==_lastAppliedProgramObject)<<") and _defineMap.changed= "<<_defineMap.changed<<std::endl;
            _lastAppliedProgramObject->getProgram()->apply(*this);
        }

        if (_shaderCompositionEnabled)
        {
            if (previousLastAppliedProgramObject == _lastAppliedProgramObject || _lastAppliedProgramObject==0)
            {
                // No program has been applied by the StateSet stack so assume shader composition is required
                applyShaderComposition();
            }
        }

        if (dstate->getUniformList().empty())
        {
            if (_currentShaderCompositionUniformList.empty()) applyUniformMap(_uniformMap);
            else applyUniformList(_uniformMap, _currentShaderCompositionUniformList);
        }
        else
        {
            if (_currentShaderCompositionUniformList.empty()) applyUniformList(_uniformMap, dstate->getUniformList());
            else
            {
                // need top merge uniforms lists, but cheat for now by just applying both.
                _currentShaderCompositionUniformList.insert(dstate->getUniformList().begin(), dstate->getUniformList().end());
                applyUniformList(_uniformMap, _currentShaderCompositionUniformList);
            }
        }

#if 1
        popDefineList(_defineMap, dstate->getDefineList());
#endif

        // pop the stateset from the stack
        _stateStateStack.pop_back();
    }
    else
    {
        // no incoming stateset, so simply apply state.
        apply();
    }

    if (_checkGLErrors==ONCE_PER_ATTRIBUTE) checkGLErrors("end of State::apply(StateSet*)");
}

void State::apply()
{
    if (_checkGLErrors==ONCE_PER_ATTRIBUTE) checkGLErrors("start of State::apply()");

    _currentShaderCompositionUniformList.clear();

    // apply all texture state and modes
    unsigned int unit;
    unsigned int unitMax = maximum(_textureModeMapList.size(),_textureAttributeMapList.size());
    for(unit=0;unit<unitMax;++unit)
    {
        if (unit<_textureModeMapList.size()) applyModeMapOnTexUnit(unit,_textureModeMapList[unit]);
        if (unit<_textureAttributeMapList.size()) applyAttributeMapOnTexUnit(unit,_textureAttributeMapList[unit]);
    }

    // go through all active OpenGL modes, enabling/disable where
    // appropriate.
    applyModeMap(_modeMap);

    const Program::PerContextProgram* previousLastAppliedProgramObject = _lastAppliedProgramObject;

    // go through all active StateAttribute's, applying where appropriate.
    applyAttributeMap(_attributeMap);


    if ((_lastAppliedProgramObject!=0) && (previousLastAppliedProgramObject==_lastAppliedProgramObject) && _defineMap.changed)
    {
        //OSG_NOTICE<<"State::apply() Program already applied ("<<(previousLastAppliedProgramObject==_lastAppliedProgramObject)<<") and _defineMap.changed= "<<_defineMap.changed<<std::endl;
        if (_lastAppliedProgramObject) _lastAppliedProgramObject->getProgram()->apply(*this);
    }


    if (_shaderCompositionEnabled)
    {
        applyShaderComposition();
    }

    if (_currentShaderCompositionUniformList.empty()) applyUniformMap(_uniformMap);
    else applyUniformList(_uniformMap, _currentShaderCompositionUniformList);

    if (_checkGLErrors==ONCE_PER_ATTRIBUTE) checkGLErrors("end of State::apply()");
}

void State::applyShaderComposition()
{
    if (_shaderCompositionEnabled)
    {
        if (_shaderCompositionDirty)
        {
            // if (isNotifyEnabled(osg::INFO)) print(notify(osg::INFO));

            // build lits of current ShaderComponents
            ShaderComponents shaderComponents;

            // OSG_NOTICE<<"State::applyShaderComposition() : _attributeMap.size()=="<<_attributeMap.size()<<std::endl;

            for(AttributeMap::iterator itr = _attributeMap.begin();
                itr != _attributeMap.end();
                ++itr)
            {
                // OSG_NOTICE<<"  itr->first="<<itr->first.first<<", "<<itr->first.second<<std::endl;

                AttributeStack& as = itr->second;
                if (as.last_applied_shadercomponent)
                {
                    shaderComponents.push_back(const_cast<ShaderComponent*>(as.last_applied_shadercomponent));
                }
            }

            _currentShaderCompositionProgram = _shaderComposer->getOrCreateProgram(shaderComponents);
        }

        if (_currentShaderCompositionProgram)
        {
            Program::PerContextProgram* pcp = _currentShaderCompositionProgram->getPCP(*this);
            if (_lastAppliedProgramObject != pcp) applyAttribute(_currentShaderCompositionProgram);
        }
    }
}


void State::haveAppliedMode(StateAttribute::GLMode mode,StateAttribute::GLModeValue value)
{
    haveAppliedMode(_modeMap,mode,value);
}

void State::haveAppliedMode(StateAttribute::GLMode mode)
{
    haveAppliedMode(_modeMap,mode);
}

void State::haveAppliedAttribute(const StateAttribute* attribute)
{
    haveAppliedAttribute(_attributeMap,attribute);
}

void State::haveAppliedAttribute(StateAttribute::Type type, unsigned int member)
{
    haveAppliedAttribute(_attributeMap,type,member);
}

bool State::getLastAppliedMode(StateAttribute::GLMode mode) const
{
    return getLastAppliedMode(_modeMap,mode);
}

const StateAttribute* State::getLastAppliedAttribute(StateAttribute::Type type, unsigned int member) const
{
    return getLastAppliedAttribute(_attributeMap,type,member);
}


void State::haveAppliedTextureMode(unsigned int unit,StateAttribute::GLMode mode,StateAttribute::GLModeValue value)
{
    haveAppliedMode(getOrCreateTextureModeMap(unit),mode,value);
}

void State::haveAppliedTextureMode(unsigned int unit,StateAttribute::GLMode mode)
{
    haveAppliedMode(getOrCreateTextureModeMap(unit),mode);
}

void State::haveAppliedTextureAttribute(unsigned int unit,const StateAttribute* attribute)
{
    haveAppliedAttribute(getOrCreateTextureAttributeMap(unit),attribute);
}

void State::haveAppliedTextureAttribute(unsigned int unit,StateAttribute::Type type, unsigned int member)
{
    haveAppliedAttribute(getOrCreateTextureAttributeMap(unit),type,member);
}

bool State::getLastAppliedTextureMode(unsigned int unit,StateAttribute::GLMode mode) const
{
    if (unit>=_textureModeMapList.size()) return false;
    return getLastAppliedMode(_textureModeMapList[unit],mode);
}

const StateAttribute* State::getLastAppliedTextureAttribute(unsigned int unit,StateAttribute::Type type, unsigned int member) const
{
    if (unit>=_textureAttributeMapList.size()) return NULL;
    return getLastAppliedAttribute(_textureAttributeMapList[unit],type,member);
}


void State::haveAppliedMode(ModeMap& modeMap,StateAttribute::GLMode mode,StateAttribute::GLModeValue value)
{
    ModeStack& ms = modeMap[mode];

    ms.last_applied_value = value & StateAttribute::ON;

    // will need to disable this mode on next apply so set it to changed.
    ms.changed = true;
}

/** mode has been set externally, update state to reflect this setting.*/
void State::haveAppliedMode(ModeMap& modeMap,StateAttribute::GLMode mode)
{
    ModeStack& ms = modeMap[mode];

    // don't know what last applied value is can't apply it.
    // assume that it has changed by toggle the value of last_applied_value.
    ms.last_applied_value = !ms.last_applied_value;

    // will need to disable this mode on next apply so set it to changed.
    ms.changed = true;
}

/** attribute has been applied externally, update state to reflect this setting.*/
void State::haveAppliedAttribute(AttributeMap& attributeMap,const StateAttribute* attribute)
{
    if (attribute)
    {
        AttributeStack& as = attributeMap[attribute->getTypeMemberPair()];

        as.last_applied_attribute = attribute;

        // will need to update this attribute on next apply so set it to changed.
        as.changed = true;
    }
}

void State::haveAppliedAttribute(AttributeMap& attributeMap,StateAttribute::Type type, unsigned int member)
{

    AttributeMap::iterator itr = attributeMap.find(StateAttribute::TypeMemberPair(type,member));
    if (itr!=attributeMap.end())
    {
        AttributeStack& as = itr->second;
        as.last_applied_attribute = 0L;

        // will need to update this attribute on next apply so set it to changed.
        as.changed = true;
    }
}

bool State::getLastAppliedMode(const ModeMap& modeMap,StateAttribute::GLMode mode) const
{
    ModeMap::const_iterator itr = modeMap.find(mode);
    if (itr!=modeMap.end())
    {
        const ModeStack& ms = itr->second;
        return ms.last_applied_value;
    }
    else
    {
        return false;
    }
}

const StateAttribute* State::getLastAppliedAttribute(const AttributeMap& attributeMap,StateAttribute::Type type, unsigned int member) const
{
    AttributeMap::const_iterator itr = attributeMap.find(StateAttribute::TypeMemberPair(type,member));
    if (itr!=attributeMap.end())
    {
        const AttributeStack& as = itr->second;
        return as.last_applied_attribute;
    }
    else
    {
        return NULL;
    }
}

void State::dirtyAllModes()
{
    for(ModeMap::iterator mitr=_modeMap.begin();
        mitr!=_modeMap.end();
        ++mitr)
    {
        ModeStack& ms = mitr->second;
        ms.last_applied_value = !ms.last_applied_value;
        ms.changed = true;

    }

    for(TextureModeMapList::iterator tmmItr=_textureModeMapList.begin();
        tmmItr!=_textureModeMapList.end();
        ++tmmItr)
    {
        for(ModeMap::iterator mitr=tmmItr->begin();
            mitr!=tmmItr->end();
            ++mitr)
        {
            ModeStack& ms = mitr->second;
            ms.last_applied_value = !ms.last_applied_value;
            ms.changed = true;

        }
    }
}

void State::dirtyAllAttributes()
{
    for(AttributeMap::iterator aitr=_attributeMap.begin();
        aitr!=_attributeMap.end();
        ++aitr)
    {
        AttributeStack& as = aitr->second;
        as.last_applied_attribute = 0;
        as.changed = true;
    }


    for(TextureAttributeMapList::iterator tamItr=_textureAttributeMapList.begin();
        tamItr!=_textureAttributeMapList.end();
        ++tamItr)
    {
        AttributeMap& attributeMap = *tamItr;
        for(AttributeMap::iterator aitr=attributeMap.begin();
            aitr!=attributeMap.end();
            ++aitr)
        {
            AttributeStack& as = aitr->second;
            as.last_applied_attribute = 0;
            as.changed = true;
        }
    }

}


Polytope State::getViewFrustum() const
{
    Polytope cv;
    cv.setToUnitFrustum();
    cv.transformProvidingInverse((*_modelView)*(*_projection));
    return cv;
}


void State::resetVertexAttributeAlias(bool compactAliasing, unsigned int numTextureUnits)
{
    _texCoordAliasList.clear();
    _attributeBindingList.clear();

    if (compactAliasing)
    {
        unsigned int slot = 0;
        setUpVertexAttribAlias(_vertexAlias, slot++, "gl_Vertex","osg_Vertex","attribute vec4 ");
        setUpVertexAttribAlias(_normalAlias, slot++, "gl_Normal","osg_Normal","attribute vec3 ");
        setUpVertexAttribAlias(_colorAlias, slot++, "gl_Color","osg_Color","attribute vec4 ");

        _texCoordAliasList.resize(numTextureUnits);
        for(unsigned int i=0; i<_texCoordAliasList.size(); i++)
        {
            std::stringstream gl_MultiTexCoord;
            std::stringstream osg_MultiTexCoord;
            gl_MultiTexCoord<<"gl_MultiTexCoord"<<i;
            osg_MultiTexCoord<<"osg_MultiTexCoord"<<i;

            setUpVertexAttribAlias(_texCoordAliasList[i], slot++, gl_MultiTexCoord.str(), osg_MultiTexCoord.str(), "attribute vec4 ");
        }

        setUpVertexAttribAlias(_secondaryColorAlias, slot++, "gl_SecondaryColor","osg_SecondaryColor","attribute vec4 ");
        setUpVertexAttribAlias(_fogCoordAlias, slot++, "gl_FogCoord","osg_FogCoord","attribute float ");

    }
    else
    {
        setUpVertexAttribAlias(_vertexAlias,0, "gl_Vertex","osg_Vertex","attribute vec4 ");
        setUpVertexAttribAlias(_normalAlias, 2, "gl_Normal","osg_Normal","attribute vec3 ");
        setUpVertexAttribAlias(_colorAlias, 3, "gl_Color","osg_Color","attribute vec4 ");
        setUpVertexAttribAlias(_secondaryColorAlias, 4, "gl_SecondaryColor","osg_SecondaryColor","attribute vec4 ");
        setUpVertexAttribAlias(_fogCoordAlias, 5, "gl_FogCoord","osg_FogCoord","attribute float ");

        unsigned int base = 8;
        _texCoordAliasList.resize(numTextureUnits);
        for(unsigned int i=0; i<_texCoordAliasList.size(); i++)
        {
            std::stringstream gl_MultiTexCoord;
            std::stringstream osg_MultiTexCoord;
            gl_MultiTexCoord<<"gl_MultiTexCoord"<<i;
            osg_MultiTexCoord<<"osg_MultiTexCoord"<<i;

            setUpVertexAttribAlias(_texCoordAliasList[i], base+i, gl_MultiTexCoord.str(), osg_MultiTexCoord.str(), "attribute vec4 ");
        }
    }
}


void State::disableAllVertexArrays()
{
    disableVertexPointer();
    disableTexCoordPointersAboveAndIncluding(0);
    disableVertexAttribPointersAboveAndIncluding(0);
    disableColorPointer();
    disableFogCoordPointer();
    disableNormalPointer();
    disableSecondaryColorPointer();
}

void State::dirtyAllVertexArrays()
{
    dirtyVertexPointer();
    dirtyTexCoordPointersAboveAndIncluding(0);
    dirtyVertexAttribPointersAboveAndIncluding(0);
    dirtyColorPointer();
    dirtyFogCoordPointer();
    dirtyNormalPointer();
    dirtySecondaryColorPointer();
}

void State::setInterleavedArrays( GLenum format, GLsizei stride, const GLvoid* pointer)
{
    disableAllVertexArrays();

#if defined(OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE) && !defined(OSG_GLES1_AVAILABLE)
    glInterleavedArrays( format, stride, pointer);
#else
    OSG_NOTICE<<"Warning: State::setInterleavedArrays(..) not implemented."<<std::endl;
#endif

    // the crude way, assume that all arrays have been affected so dirty them and
    // disable them...
    dirtyAllVertexArrays();
}

void State::initializeExtensionProcs()
{
    if (_extensionProcsInitialized) return;

    const char* vendor = (const char*) glGetString( GL_VENDOR );
    if (vendor)
    {
        std::string str_vendor(vendor);
        std::replace(str_vendor.begin(), str_vendor.end(), ' ', '_');
        OSG_INFO<<"GL_VENDOR = ["<<str_vendor<<"]"<<std::endl;
        _defineMap.map[str_vendor].defineVec.push_back(osg::StateSet::DefinePair("1",osg::StateAttribute::ON));
        _defineMap.map[str_vendor].changed = true;
        _defineMap.changed = true;
    }

    _glExtensions = new GLExtensions(_contextID);
    GLExtensions::Set(_contextID, _glExtensions.get());

    setGLExtensionFuncPtr(_glClientActiveTexture,"glClientActiveTexture","glClientActiveTextureARB");
    setGLExtensionFuncPtr(_glActiveTexture, "glActiveTexture","glActiveTextureARB");
    setGLExtensionFuncPtr(_glFogCoordPointer, "glFogCoordPointer","glFogCoordPointerEXT");
    setGLExtensionFuncPtr(_glSecondaryColorPointer, "glSecondaryColorPointer","glSecondaryColorPointerEXT");
    setGLExtensionFuncPtr(_glVertexAttribPointer, "glVertexAttribPointer","glVertexAttribPointerARB");
    setGLExtensionFuncPtr(_glVertexAttribIPointer, "glVertexAttribIPointer");
    setGLExtensionFuncPtr(_glVertexAttribLPointer, "glVertexAttribLPointer","glVertexAttribPointerARB");
    setGLExtensionFuncPtr(_glEnableVertexAttribArray, "glEnableVertexAttribArray","glEnableVertexAttribArrayARB");
    setGLExtensionFuncPtr(_glMultiTexCoord4f, "glMultiTexCoord4f","glMultiTexCoord4fARB");
    setGLExtensionFuncPtr(_glVertexAttrib4f, "glVertexAttrib4f");
    setGLExtensionFuncPtr(_glVertexAttrib4fv, "glVertexAttrib4fv");
    setGLExtensionFuncPtr(_glDisableVertexAttribArray, "glDisableVertexAttribArray","glDisableVertexAttribArrayARB");
    setGLExtensionFuncPtr(_glBindBuffer, "glBindBuffer","glBindBufferARB");

    setGLExtensionFuncPtr(_glDrawArraysInstanced, "glDrawArraysInstanced","glDrawArraysInstancedARB","glDrawArraysInstancedEXT");
    setGLExtensionFuncPtr(_glDrawElementsInstanced, "glDrawElementsInstanced","glDrawElementsInstancedARB","glDrawElementsInstancedEXT");

    if (osg::getGLVersionNumber() >= 2.0 || osg::isGLExtensionSupported(_contextID, "GL_ARB_vertex_shader") || OSG_GLES2_FEATURES || OSG_GL3_FEATURES)
    {
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,&_glMaxTextureUnits);
        #ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
            glGetIntegerv(GL_MAX_TEXTURE_COORDS, &_glMaxTextureCoords);
        #else
            _glMaxTextureCoords = _glMaxTextureUnits;
        #endif
    }
    else if ( osg::getGLVersionNumber() >= 1.3 ||
                                 osg::isGLExtensionSupported(_contextID,"GL_ARB_multitexture") ||
                                 osg::isGLExtensionSupported(_contextID,"GL_EXT_multitexture") ||
                                 OSG_GLES1_FEATURES)
    {
        GLint maxTextureUnits = 0;
        glGetIntegerv(GL_MAX_TEXTURE_UNITS,&maxTextureUnits);
        _glMaxTextureUnits = maxTextureUnits;
        _glMaxTextureCoords = maxTextureUnits;
    }
    else
    {
        _glMaxTextureUnits = 1;
        _glMaxTextureCoords = 1;
    }

    if (_glExtensions->isARBTimerQuerySupported)
    {
        const GLubyte* renderer = glGetString(GL_RENDERER);
        std::string rendererString = renderer ? (const char*)renderer : "";
        if (rendererString.find("Radeon")!=std::string::npos || rendererString.find("RADEON")!=std::string::npos || rendererString.find("FirePro")!=std::string::npos)
        {
            // AMD/ATI drivers are producing an invalid enumerate error on the
            // glGetQueryiv(GL_TIMESTAMP, GL_QUERY_COUNTER_BITS_ARB, &bits);
            // call so work around it by assuming 64 bits for counter.
            setTimestampBits(64);
            //setTimestampBits(0);
        }
        else
        {
            GLint bits = 0;
            _glExtensions->glGetQueryiv(GL_TIMESTAMP, GL_QUERY_COUNTER_BITS_ARB, &bits);
            setTimestampBits(bits);
        }
    }


    _extensionProcsInitialized = true;

    if (_graphicsCostEstimator.valid())
    {
        RenderInfo renderInfo(this,0);
        _graphicsCostEstimator->calibrate(renderInfo);
    }
}

bool State::setClientActiveTextureUnit( unsigned int unit )
{
    if (unit!=_currentClientActiveTextureUnit)
    {
        if (_glClientActiveTexture && unit < (unsigned int)_glMaxTextureCoords)
        {
            _glClientActiveTexture(GL_TEXTURE0+unit);
            _currentClientActiveTextureUnit = unit;
        }
        else
        {
            return unit==0;
        }
    }
    return true;
}

void State::setFogCoordPointer(GLenum type, GLsizei stride, const GLvoid *ptr, GLboolean normalized)
{
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
    if (_useVertexAttributeAliasing)
    {
        setVertexAttribPointer(_fogCoordAlias._location, 1, type, normalized, stride, ptr);
    }
    else
    {
        if (_glFogCoordPointer)
        {

            if (!_fogArray._enabled || _fogArray._dirty)
            {
                _fogArray._enabled = true;
                glEnableClientState(GL_FOG_COORDINATE_ARRAY);
            }
            //if (_fogArray._pointer!=ptr || _fogArray._dirty)
            {
                _fogArray._pointer=ptr;
                _glFogCoordPointer( type, stride, ptr );
            }
            _fogArray._lazy_disable = false;
            _fogArray._dirty = false;
        }
    }
#else
        setVertexAttribPointer(_fogCoordAlias._location, 1, type, normalized, stride, ptr);
#endif
}

void State::setSecondaryColorPointer( GLint size, GLenum type,
                                      GLsizei stride, const GLvoid *ptr, GLboolean normalized )
{
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
    if (_useVertexAttributeAliasing)
    {
        setVertexAttribPointer(_secondaryColorAlias._location, size, type, normalized, stride, ptr);
    }
    else
    {
        if (_glSecondaryColorPointer)
        {
            if (!_secondaryColorArray._enabled || _secondaryColorArray._dirty)
            {
                _secondaryColorArray._enabled = true;
                glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
            }
            //if (_secondaryColorArray._pointer!=ptr || _secondaryColorArray._dirty)
            {
                _secondaryColorArray._pointer=ptr;
                _glSecondaryColorPointer( size, type, stride, ptr );
            }
            _secondaryColorArray._lazy_disable = false;
            _secondaryColorArray._dirty = false;
            _secondaryColorArray._normalized = normalized;
        }
    }
#else
        setVertexAttribPointer(_secondaryColorAlias._location, size, type, normalized, stride, ptr);
#endif
}

/** wrapper around glEnableVertexAttribArrayARB(index);glVertexAttribPointerARB(..);
* note, only updates values that change.*/
void State::setVertexAttribPointer( unsigned int index,
                                      GLint size, GLenum type, GLboolean normalized,
                                    GLsizei stride, const GLvoid *ptr )
{
    if (_glVertexAttribPointer)
    {
        // OSG_NOTICE<<"State::setVertexAttribPointer("<<index<<",...)"<<std::endl;

        if ( index >= _vertexAttribArrayList.size()) _vertexAttribArrayList.resize(index+1);
        EnabledArrayPair& eap = _vertexAttribArrayList[index];

        if (!eap._enabled || eap._dirty)
        {
            eap._enabled = true;
            // OSG_NOTICE<<"    _glEnableVertexAttribArray( "<<index<<" )"<<std::endl;
            _glEnableVertexAttribArray( index );
        }
        //if (eap._pointer != ptr || eap._normalized!=normalized || eap._dirty)
        {
            // OSG_NOTICE<<"    _glVertexAttribPointer( "<<index<<" )"<<std::endl;
            _glVertexAttribPointer( index, size, type, normalized, stride, ptr );
            eap._pointer = ptr;
            eap._normalized = normalized;
        }
        eap._lazy_disable = false;
        eap._dirty = false;
    }
}

/** wrapper around glEnableVertexAttribArrayARB(index);glVertexAttribIPointer(..);
* note, only updates values that change.*/
void State::setVertexAttribIPointer( unsigned int index,
                                     GLint size, GLenum type,
                                     GLsizei stride, const GLvoid *ptr )
{
    if (_glVertexAttribIPointer)
    {
        // OSG_NOTICE<<"State::setVertexAttribIPointer("<<index<<",...)"<<std::endl;

        if ( index >= _vertexAttribArrayList.size()) _vertexAttribArrayList.resize(index+1);
        EnabledArrayPair& eap = _vertexAttribArrayList[index];

        if (!eap._enabled || eap._dirty)
        {
            eap._enabled = true;
            // OSG_NOTICE<<"    _glEnableVertexAttribArray( "<<index<<" )"<<std::endl;
            _glEnableVertexAttribArray( index );
        }
        //if (eap._pointer != ptr || eap._dirty)
        {
            // OSG_NOTICE<<"    _glVertexAttribIPointer( "<<index<<" )"<<std::endl;
            _glVertexAttribIPointer( index, size, type, stride, ptr );
            eap._pointer = ptr;
            eap._normalized = false;
        }
        eap._lazy_disable = false;
        eap._dirty = false;
    }
}
/** wrapper around glEnableVertexAttribArrayARB(index);glVertexAttribLPointer(..);
* note, only updates values that change.*/
void State::setVertexAttribLPointer( unsigned int index,
                                     GLint size, GLenum type,
                                     GLsizei stride, const GLvoid *ptr )
{
    if (_glVertexAttribLPointer)
    {
        // OSG_NOTICE<<"State::setVertexAttribLPointer("<<index<<",...)"<<std::endl;

        if ( index >= _vertexAttribArrayList.size()) _vertexAttribArrayList.resize(index+1);
        EnabledArrayPair& eap = _vertexAttribArrayList[index];

        if (!eap._enabled || eap._dirty)
        {
            eap._enabled = true;
            // OSG_NOTICE<<"    _glEnableVertexAttribArray( "<<index<<" )"<<std::endl;
            _glEnableVertexAttribArray( index );
        }
        //if (eap._pointer != ptr || eap._dirty)
        {
            // OSG_NOTICE<<"    _glVertexAttribLPointer( "<<index<<" )"<<std::endl;
            _glVertexAttribLPointer( index, size, type, stride, ptr );
            eap._pointer = ptr;
            eap._normalized = false;
        }
        eap._lazy_disable = false;
        eap._dirty = false;
    }
}
/** wrapper around DisableVertexAttribArrayARB(index);
* note, only updates values that change.*/
void State::disableVertexAttribPointer( unsigned int index )
{
    if (_glDisableVertexAttribArray)
    {
        if ( index >= _vertexAttribArrayList.size()) _vertexAttribArrayList.resize(index+1);
        EnabledArrayPair& eap = _vertexAttribArrayList[index];

        if (eap._enabled || eap._dirty)
        {
            eap._enabled = false;
            eap._dirty = false;
            // OSG_NOTICE<<"    _glDisableVertexAttribArray( "<<index<<" )"<<std::endl;
            _glDisableVertexAttribArray( index );
        }
    }
}

void State::disableVertexAttribPointersAboveAndIncluding( unsigned int index )
{
    if (_glDisableVertexAttribArray)
    {
        while (index<_vertexAttribArrayList.size())
        {
            EnabledArrayPair& eap = _vertexAttribArrayList[index];
            if (eap._enabled || eap._dirty)
            {
                eap._enabled = false;
                eap._dirty = false;
                // OSG_NOTICE<<"    State::disableVertexAttribPointersAboveAndIncluding(): _glDisableVertexAttribArray( "<<index<<" )"<<std::endl;
                _glDisableVertexAttribArray( index );
            }
            ++index;
        }
    }
}

void State::lazyDisablingOfVertexAttributes()
{
    // OSG_NOTICE<<"lazyDisablingOfVertexAttributes()"<<std::endl;
    if (!_useVertexAttributeAliasing)
    {
        _vertexArray._lazy_disable = true;
        _normalArray._lazy_disable = true;
        _colorArray._lazy_disable = true;
        _secondaryColorArray._lazy_disable = true;
        _fogArray._lazy_disable = true;
        for(EnabledTexCoordArrayList::iterator itr = _texCoordArrayList.begin();
            itr != _texCoordArrayList.end();
            ++itr)
        {
            itr->_lazy_disable = true;
        }
    }

    for(EnabledVertexAttribArrayList::iterator itr = _vertexAttribArrayList.begin();
        itr != _vertexAttribArrayList.end();
        ++itr)
    {
        itr->_lazy_disable = true;
    }
}

void State::applyDisablingOfVertexAttributes()
{
    //OSG_NOTICE<<"start of applyDisablingOfVertexAttributes()"<<std::endl;
    if (!_useVertexAttributeAliasing)
    {
        if (_vertexArray._lazy_disable) disableVertexPointer();
        if (_normalArray._lazy_disable) disableNormalPointer();
        if (_colorArray._lazy_disable) disableColorPointer();
        if (_secondaryColorArray._lazy_disable) disableSecondaryColorPointer();
        if (_fogArray._lazy_disable) disableFogCoordPointer();
        for(unsigned int i=0; i<_texCoordArrayList.size(); ++i)
        {
            if (_texCoordArrayList[i]._lazy_disable) disableTexCoordPointer(i);
        }
    }
    for(unsigned int i=0; i<_vertexAttribArrayList.size(); ++i)
    {
        if (_vertexAttribArrayList[i]._lazy_disable) disableVertexAttribPointer(i);
    }
    // OSG_NOTICE<<"end of applyDisablingOfVertexAttributes()"<<std::endl;
}


bool State::computeSecondaryColorSupported() const
{
    _isSecondaryColorSupportResolved = true;
    _isSecondaryColorSupported = osg::isGLExtensionSupported(_contextID,"GL_EXT_secondary_color");
    return _isSecondaryColorSupported;
}

bool State::computeFogCoordSupported() const
{
    _isFogCoordSupportResolved = true;
    _isFogCoordSupported = osg::isGLExtensionSupported(_contextID,"GL_EXT_fog_coord");
    return _isFogCoordSupported;
}

bool State::computeVertexBufferObjectSupported() const
{
    _isVertexBufferObjectSupportResolved = true;
    _isVertexBufferObjectSupported = OSG_GLES2_FEATURES || OSG_GL3_FEATURES || osg::isGLExtensionSupported(_contextID,"GL_ARB_vertex_buffer_object");
    return _isVertexBufferObjectSupported;
}

bool State::checkGLErrors(const char* str) const
{
    GLenum errorNo = glGetError();
    if (errorNo!=GL_NO_ERROR)
    {
        osg::NotifySeverity notifyLevel = NOTICE; // WARN;
        const char* error = (char*)gluErrorString(errorNo);
        if (error)
        {
            OSG_NOTIFY(notifyLevel)<<"Warning: detected OpenGL error '" << error<<"'";
        }
        else
        {
            OSG_NOTIFY(notifyLevel)<<"Warning: detected OpenGL error number 0x" << std::hex << errorNo << std::dec;
        }

        if (str)
        {
            OSG_NOTIFY(notifyLevel)<<" at "<<str<< std::endl;
        }
        else
        {
            OSG_NOTIFY(notifyLevel)<<" in osg::State."<< std::endl;
        }

        return true;
    }
    return false;
}

bool State::checkGLErrors(StateAttribute::GLMode mode) const
{
    GLenum errorNo = glGetError();
    if (errorNo!=GL_NO_ERROR)
    {
        const char* error = (char*)gluErrorString(errorNo);
        if (error)
        {
            OSG_NOTIFY(WARN)<<"Warning: detected OpenGL error '"<< error <<"' after applying GLMode 0x"<<hex<<mode<<dec<< std::endl;
        }
        else
        {
            OSG_NOTIFY(WARN)<<"Warning: detected OpenGL error number 0x"<< std::hex << errorNo <<" after applying GLMode 0x"<<hex<<mode<<dec<< std::endl;
        }
        return true;
    }
    return false;
}

bool State::checkGLErrors(const StateAttribute* attribute) const
{
    GLenum errorNo = glGetError();
    if (errorNo!=GL_NO_ERROR)
    {
        const char* error = (char*)gluErrorString(errorNo);
        if (error)
        {
            OSG_NOTIFY(WARN)<<"Warning: detected OpenGL error '"<< error <<"' after applying attribute "<<attribute->className()<<" "<<attribute<< std::endl;
        }
        else
        {
            OSG_NOTIFY(WARN)<<"Warning: detected OpenGL error number 0x"<< std::hex << errorNo <<" after applying attribute "<<attribute->className()<<" "<<attribute<< std::dec << std::endl;
        }

        return true;
    }
    return false;
}


void State::applyModelViewAndProjectionUniformsIfRequired()
{
    if (!_lastAppliedProgramObject) return;

    if (_modelViewMatrixUniform.valid()) _lastAppliedProgramObject->apply(*_modelViewMatrixUniform);
    if (_projectionMatrixUniform) _lastAppliedProgramObject->apply(*_projectionMatrixUniform);
    if (_modelViewProjectionMatrixUniform) _lastAppliedProgramObject->apply(*_modelViewProjectionMatrixUniform);
    if (_normalMatrixUniform) _lastAppliedProgramObject->apply(*_normalMatrixUniform);
}

namespace State_Utils
{
    bool replace(std::string& str, const std::string& original_phrase, const std::string& new_phrase)
    {
        // Prevent infinite loop : if original_phrase is empty, do nothing and return false
        if (original_phrase.empty()) return false;

        bool replacedStr = false;
        std::string::size_type pos = 0;
        while((pos=str.find(original_phrase, pos))!=std::string::npos)
        {
            std::string::size_type endOfPhrasePos = pos+original_phrase.size();
            if (endOfPhrasePos<str.size())
            {
                char c = str[endOfPhrasePos];
                if ((c>='0' && c<='9') ||
                    (c>='a' && c<='z') ||
                    (c>='A' && c<='Z'))
                {
                    pos = endOfPhrasePos;
                    continue;
                }
            }

            replacedStr = true;
            str.replace(pos, original_phrase.size(), new_phrase);
        }
        return replacedStr;
    }

    void replaceAndInsertDeclaration(std::string& source, std::string::size_type declPos, const std::string& originalStr, const std::string& newStr, const std::string& declarationPrefix)
    {
        if (replace(source, originalStr, newStr))
        {
            source.insert(declPos, declarationPrefix + newStr + std::string(";\n"));
        }
    }
}

bool State::convertVertexShaderSourceToOsgBuiltIns(std::string& source) const
{
    OSG_INFO<<"State::convertShaderSourceToOsgBuiltIns()"<<std::endl;

    OSG_INFO<<"++Before Converted source "<<std::endl<<source<<std::endl<<"++++++++"<<std::endl;

    // find the first legal insertion point for replacement declarations. GLSL requires that nothing
    // precede a "#verson" compiler directive, so we must insert new declarations after it.
    std::string::size_type declPos = source.rfind( "#version " );
    if ( declPos != std::string::npos )
    {
        // found the string, now find the next linefeed and set the insertion point after it.
        declPos = source.find( '\n', declPos );
        declPos = declPos != std::string::npos ? declPos+1 : source.length();
    }
    else
    {
        declPos = 0;
    }

    if (_useModelViewAndProjectionUniforms)
    {
        // replace ftransform as it only works with built-ins
        State_Utils::replace(source, "ftransform()", "gl_ModelViewProjectionMatrix * gl_Vertex");

        // replace built in uniform
        State_Utils::replaceAndInsertDeclaration(source, declPos, "gl_ModelViewMatrix", "osg_ModelViewMatrix", "uniform mat4 ");
        State_Utils::replaceAndInsertDeclaration(source, declPos, "gl_ModelViewProjectionMatrix", "osg_ModelViewProjectionMatrix", "uniform mat4 ");
        State_Utils::replaceAndInsertDeclaration(source, declPos, "gl_ProjectionMatrix", "osg_ProjectionMatrix", "uniform mat4 ");
        State_Utils::replaceAndInsertDeclaration(source, declPos, "gl_NormalMatrix", "osg_NormalMatrix", "uniform mat3 ");
    }

    if (_useVertexAttributeAliasing)
    {
        State_Utils::replaceAndInsertDeclaration(source, declPos, _vertexAlias._glName,         _vertexAlias._osgName,         _vertexAlias._declaration);
        State_Utils::replaceAndInsertDeclaration(source, declPos, _normalAlias._glName,         _normalAlias._osgName,         _normalAlias._declaration);
        State_Utils::replaceAndInsertDeclaration(source, declPos, _colorAlias._glName,          _colorAlias._osgName,          _colorAlias._declaration);
        State_Utils::replaceAndInsertDeclaration(source, declPos, _secondaryColorAlias._glName, _secondaryColorAlias._osgName, _secondaryColorAlias._declaration);
        State_Utils::replaceAndInsertDeclaration(source, declPos, _fogCoordAlias._glName,       _fogCoordAlias._osgName,       _fogCoordAlias._declaration);
        for (size_t i=0; i<_texCoordAliasList.size(); i++)
        {
            const VertexAttribAlias& texCoordAlias = _texCoordAliasList[i];
            State_Utils::replaceAndInsertDeclaration(source, declPos, texCoordAlias._glName, texCoordAlias._osgName, texCoordAlias._declaration);
        }
    }

    OSG_INFO<<"-------- Converted source "<<std::endl<<source<<std::endl<<"----------------"<<std::endl;

    return true;
}

void State::setUpVertexAttribAlias(VertexAttribAlias& alias, GLuint location, const std::string glName, const std::string osgName, const std::string& declaration)
{
    alias = VertexAttribAlias(location, glName, osgName, declaration);
    _attributeBindingList[osgName] = location;
    // OSG_NOTICE<<"State::setUpVertexAttribAlias("<<location<<" "<<glName<<" "<<osgName<<")"<<std::endl;
}

void State::applyProjectionMatrix(const osg::RefMatrix* matrix)
{
    if (_projection!=matrix)
    {
        if (matrix)
        {
            _projection=matrix;
        }
        else
        {
            _projection=_identity;
        }

        if (_useModelViewAndProjectionUniforms)
        {
            if (_projectionMatrixUniform.valid()) _projectionMatrixUniform->set(*_projection);
            updateModelViewAndProjectionMatrixUniforms();
        }
#ifdef OSG_GL_MATRICES_AVAILABLE
        glMatrixMode( GL_PROJECTION );
            glLoadMatrix(_projection->ptr());
        glMatrixMode( GL_MODELVIEW );
#endif
    }
}

void State::loadModelViewMatrix()
{
    if (_useModelViewAndProjectionUniforms)
    {
        if (_modelViewMatrixUniform.valid()) _modelViewMatrixUniform->set(*_modelView);
        updateModelViewAndProjectionMatrixUniforms();
    }

#ifdef OSG_GL_MATRICES_AVAILABLE
    glLoadMatrix(_modelView->ptr());
#endif
}

void State::applyModelViewMatrix(const osg::RefMatrix* matrix)
{
    if (_modelView!=matrix)
    {
        if (matrix)
        {
            _modelView=matrix;
        }
        else
        {
            _modelView=_identity;
        }

        loadModelViewMatrix();
    }
}

void State::applyModelViewMatrix(const osg::Matrix& matrix)
{
    _modelViewCache->set(matrix);
    _modelView = _modelViewCache;

    loadModelViewMatrix();
}

#include <osg/io_utils>

void State::updateModelViewAndProjectionMatrixUniforms()
{
    if (_modelViewProjectionMatrixUniform.valid()) _modelViewProjectionMatrixUniform->set((*_modelView) * (*_projection));
    if (_normalMatrixUniform.valid())
    {
        Matrix mv(*_modelView);
        mv.setTrans(0.0, 0.0, 0.0);

        Matrix matrix;
        matrix.invert(mv);

        Matrix3 normalMatrix(matrix(0,0), matrix(1,0), matrix(2,0),
                             matrix(0,1), matrix(1,1), matrix(2,1),
                             matrix(0,2), matrix(1,2), matrix(2,2));

        _normalMatrixUniform->set(normalMatrix);
    }
}

void State::drawQuads(GLint first, GLsizei count, GLsizei primCount)
{
    // OSG_NOTICE<<"State::drawQuads("<<first<<", "<<count<<")"<<std::endl;

    unsigned int array = first % 4;
    unsigned int offsetFirst = ((first-array) / 4) * 6;
    unsigned int numQuads = (count/4);
    unsigned int numIndices = numQuads * 6;
    unsigned int endOfIndices = offsetFirst+numIndices;

    if (endOfIndices<65536)
    {
        IndicesGLushort& indices = _quadIndicesGLushort[array];

        if (endOfIndices >= indices.size())
        {
            // we need to expand the _indexArray to be big enough to cope with all the quads required.
            unsigned int numExistingQuads = indices.size()/6;
            unsigned int numRequiredQuads = endOfIndices/6;
            indices.reserve(endOfIndices);
            for(unsigned int i=numExistingQuads; i<numRequiredQuads; ++i)
            {
                unsigned int base = i*4 + array;
                indices.push_back(base);
                indices.push_back(base+1);
                indices.push_back(base+3);

                indices.push_back(base+1);
                indices.push_back(base+2);
                indices.push_back(base+3);

                // OSG_NOTICE<<"   adding quad indices ("<<base<<")"<<std::endl;
            }
        }

        // if (array!=0) return;

        // OSG_NOTICE<<"  glDrawElements(GL_TRIANGLES, "<<numIndices<<", GL_UNSIGNED_SHORT, "<<&(indices[base])<<")"<<std::endl;
        glDrawElementsInstanced(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, &(indices[offsetFirst]), primCount);
    }
    else
    {
        IndicesGLuint& indices = _quadIndicesGLuint[array];

        if (endOfIndices >= indices.size())
        {
            // we need to expand the _indexArray to be big enough to cope with all the quads required.
            unsigned int numExistingQuads = indices.size()/6;
            unsigned int numRequiredQuads = endOfIndices/6;
            indices.reserve(endOfIndices);
            for(unsigned int i=numExistingQuads; i<numRequiredQuads; ++i)
            {
                unsigned int base = i*4 + array;
                indices.push_back(base);
                indices.push_back(base+1);
                indices.push_back(base+3);

                indices.push_back(base+1);
                indices.push_back(base+2);
                indices.push_back(base+3);

                // OSG_NOTICE<<"   adding quad indices ("<<base<<")"<<std::endl;
            }
        }

        // if (array!=0) return;

        // OSG_NOTICE<<"  glDrawElements(GL_TRIANGLES, "<<numIndices<<", GL_UNSIGNED_SHORT, "<<&(indices[base])<<")"<<std::endl;
        glDrawElementsInstanced(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, &(indices[offsetFirst]), primCount);
    }
}

void State::ModeStack::print(std::ostream& fout) const
{
    fout<<"    valid = "<<valid<<std::endl;
    fout<<"    changed = "<<changed<<std::endl;
    fout<<"    last_applied_value = "<<last_applied_value<<std::endl;
    fout<<"    global_default_value = "<<global_default_value<<std::endl;
    fout<<"    valueVec { "<<std::endl;
    for(ModeStack::ValueVec::const_iterator itr = valueVec.begin();
        itr != valueVec.end();
        ++itr)
    {
        if (itr!=valueVec.begin()) fout<<", ";
        fout<<*itr;
    }
    fout<<" }"<<std::endl;
}

void State::AttributeStack::print(std::ostream& fout) const
{
    fout<<"    changed = "<<changed<<std::endl;
    fout<<"    last_applied_attribute = "<<last_applied_attribute;
    if (last_applied_attribute) fout<<", "<<last_applied_attribute->className()<<", "<<last_applied_attribute->getName()<<std::endl;
    fout<<"    last_applied_shadercomponent = "<<last_applied_shadercomponent<<std::endl;
    if (last_applied_shadercomponent)  fout<<", "<<last_applied_shadercomponent->className()<<", "<<last_applied_shadercomponent->getName()<<std::endl;
    fout<<"    global_default_attribute = "<<global_default_attribute.get()<<std::endl;
    fout<<"    attributeVec { ";
    for(AttributeVec::const_iterator itr = attributeVec.begin();
        itr != attributeVec.end();
        ++itr)
    {
        if (itr!=attributeVec.begin()) fout<<", ";
        fout<<"("<<itr->first<<", "<<itr->second<<")";
    }
    fout<<" }"<<std::endl;
}


void State::UniformStack::print(std::ostream& fout) const
{
    fout<<"    UniformVec { ";
    for(UniformVec::const_iterator itr = uniformVec.begin();
        itr != uniformVec.end();
        ++itr)
    {
        if (itr!=uniformVec.begin()) fout<<", ";
        fout<<"("<<itr->first<<", "<<itr->second<<")";
    }
    fout<<" }"<<std::endl;
}





void State::print(std::ostream& fout) const
{
#if 0
        GraphicsContext*            _graphicsContext;
        unsigned int                _contextID;

        bool                            _shaderCompositionEnabled;
        bool                            _shaderCompositionDirty;
        osg::ref_ptr<ShaderComposer>    _shaderComposer;
#endif

#if 0
        osg::Program*                   _currentShaderCompositionProgram;
        StateSet::UniformList           _currentShaderCompositionUniformList;
#endif

#if 0
        ref_ptr<FrameStamp>         _frameStamp;

        ref_ptr<const RefMatrix>    _identity;
        ref_ptr<const RefMatrix>    _initialViewMatrix;
        ref_ptr<const RefMatrix>    _projection;
        ref_ptr<const RefMatrix>    _modelView;
        ref_ptr<RefMatrix>          _modelViewCache;

        bool                        _useModelViewAndProjectionUniforms;
        ref_ptr<Uniform>            _modelViewMatrixUniform;
        ref_ptr<Uniform>            _projectionMatrixUniform;
        ref_ptr<Uniform>            _modelViewProjectionMatrixUniform;
        ref_ptr<Uniform>            _normalMatrixUniform;

        Matrix                      _initialInverseViewMatrix;

        ref_ptr<DisplaySettings>    _displaySettings;

        bool*                       _abortRenderingPtr;
        CheckForGLErrors            _checkGLErrors;


        bool                        _useVertexAttributeAliasing;
        VertexAttribAlias           _vertexAlias;
        VertexAttribAlias           _normalAlias;
        VertexAttribAlias           _colorAlias;
        VertexAttribAlias           _secondaryColorAlias;
        VertexAttribAlias           _fogCoordAlias;
        VertexAttribAliasList       _texCoordAliasList;

        Program::AttribBindingList  _attributeBindingList;
#endif
        fout<<"ModeMap _modeMap {"<<std::endl;
        for(ModeMap::const_iterator itr = _modeMap.begin();
            itr != _modeMap.end();
            ++itr)
        {
            fout<<"  GLMode="<<itr->first<<", ModeStack {"<<std::endl;
            itr->second.print(fout);
            fout<<"  }"<<std::endl;
        }
        fout<<"}"<<std::endl;

        fout<<"AttributeMap _attributeMap {"<<std::endl;
        for(AttributeMap::const_iterator itr = _attributeMap.begin();
            itr != _attributeMap.end();
            ++itr)
        {
            fout<<"  TypeMemberPaid=("<<itr->first.first<<", "<<itr->first.second<<") AttributeStack {"<<std::endl;
            itr->second.print(fout);
            fout<<"  }"<<std::endl;
        }
        fout<<"}"<<std::endl;

        fout<<"UniformMap _uniformMap {"<<std::endl;
        for(UniformMap::const_iterator itr = _uniformMap.begin();
            itr != _uniformMap.end();
            ++itr)
        {
            fout<<"  name="<<itr->first<<", UniformStack {"<<std::endl;
            itr->second.print(fout);
            fout<<"  }"<<std::endl;
        }
        fout<<"}"<<std::endl;


        fout<<"StateSetStack _stateSetStack {"<<std::endl;
        for(StateSetStack::const_iterator itr = _stateStateStack.begin();
            itr != _stateStateStack.end();
            ++itr)
        {
            fout<<(*itr)->getName()<<"  "<<*itr<<std::endl;
        }
        fout<<"}"<<std::endl;
}

void State::frameCompleted()
{
    if (getTimestampBits())
    {
        GLint64 timestamp;
        _glExtensions->glGetInteger64v(GL_TIMESTAMP, &timestamp);
        setGpuTimestamp(osg::Timer::instance()->tick(), timestamp);
        //OSG_NOTICE<<"State::frameCompleted() setting time stamp. timestamp="<<timestamp<<std::endl;
    }
}

bool State::DefineMap::updateCurrentDefines()
{
    if (changed)
    {
        currentDefines.clear();
        for(DefineStackMap::const_iterator itr = map.begin();
            itr != map.end();
            ++itr)
        {
            const DefineStack::DefineVec& dv = itr->second.defineVec;
            if (!dv.empty())
            {
                const StateSet::DefinePair& dp = dv.back();
                if (dp.second & osg::StateAttribute::ON)
                {
                    currentDefines[itr->first] = dp;
                }
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

std::string State::getDefineString(const osg::ShaderDefines& shaderDefines)
{
    if (_defineMap.changed) _defineMap.updateCurrentDefines();

    const StateSet::DefineList& currentDefines = _defineMap.currentDefines;

    ShaderDefines::const_iterator sd_itr = shaderDefines.begin();
    StateSet::DefineList::const_iterator cd_itr = currentDefines.begin();

    std::string shaderDefineStr;

    while(sd_itr != shaderDefines.end() && cd_itr != currentDefines.end())
    {
        if ((*sd_itr) < cd_itr->first) ++sd_itr;
        else if (cd_itr->first < (*sd_itr)) ++cd_itr;
        else
        {
            const StateSet::DefinePair& dp = cd_itr->second;
            shaderDefineStr += "#define ";
            shaderDefineStr += cd_itr->first;
            if (dp.first.empty())
            {
                shaderDefineStr += "\n";
            }
            else
            {
                shaderDefineStr += " ";
                shaderDefineStr += dp.first;
                shaderDefineStr += "\n";
            }

            ++sd_itr;
            ++cd_itr;
        }
    }
    return shaderDefineStr;
}

bool State::supportsShaderRequirements(const osg::ShaderDefines& shaderRequirements)
{
    if (shaderRequirements.empty()) return true;

    if (_defineMap.changed) _defineMap.updateCurrentDefines();

    const StateSet::DefineList& currentDefines = _defineMap.currentDefines;
    for(ShaderDefines::const_iterator sr_itr = shaderRequirements.begin();
        sr_itr != shaderRequirements.end();
        ++sr_itr)
    {
        if (currentDefines.find(*sr_itr)==currentDefines.end()) return false;
    }
    return true;
}

bool State::supportsShaderRequirement(const std::string& shaderRequirement)
{
    if (_defineMap.changed) _defineMap.updateCurrentDefines();
    const StateSet::DefineList& currentDefines = _defineMap.currentDefines;
    return (currentDefines.find(shaderRequirement)!=currentDefines.end());
}
