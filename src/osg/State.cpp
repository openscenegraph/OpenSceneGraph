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
#include <osg/Notify>
#include <osg/GLU>
#include <osg/GLExtensions>
#include <osg/ApplicationUsage>

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
    _identity = new osg::RefMatrix(); // default RefMatrix constructs to identity.
    _initialViewMatrix = _identity;
    _projection = _identity;
    _modelView = _identity;

    _abortRenderingPtr = false;    

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
    _glEnableVertexAttribArray = 0;
    _glDisableVertexAttribArray = 0;
    _glDrawArraysInstanced = 0;
    _glDrawElementsInstanced = 0;

    _dynamicObjectCount  = 0;

    _glMaxTextureCoords = 1;
    _glMaxTextureUnits = 1;
}

State::~State()
{
    for(AppliedProgramObjectSet::iterator itr = _appliedProgramObjectSet.begin();
        itr != _appliedProgramObjectSet.end();
        ++itr)
    {
        (*itr)->removeObserver(this);
    }
}

void State::objectDeleted(void* object)
{
    const Program::PerContextProgram* ppcp = reinterpret_cast<const Program::PerContextProgram*>(object);
    AppliedProgramObjectSet::iterator itr = _appliedProgramObjectSet.find(ppcp);
    if (itr != _appliedProgramObjectSet.end()) 
    {
        // osg::notify(osg::NOTICE)<<"Removing _appliedProgramObjectSet entry "<<ppcp<<std::endl;
        _appliedProgramObjectSet.erase(itr);
    }
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
    
    _lastAppliedProgramObject = 0;

    for(AppliedProgramObjectSet::iterator apitr=_appliedProgramObjectSet.begin();
        apitr!=_appliedProgramObjectSet.end();
        ++apitr)
    {
        (*apitr)->resetAppliedUniforms();
        (*apitr)->removeObserver(this);
    }
    
    _appliedProgramObjectSet.clear();
    
    
    // what about uniforms??? need to clear them too...
    // go through all active Unfirom's, setting to change to force update,
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
    }

    // osg::notify(osg::NOTICE)<<"State::pushStateSet()"<<_stateStateStack.size()<<std::endl;
}

void State::popAllStateSets()
{
    // osg::notify(osg::NOTICE)<<"State::popAllStateSets()"<<_stateStateStack.size()<<std::endl;

    while (!_stateStateStack.empty()) popStateSet();
    
    applyProjectionMatrix(0);
    applyModelViewMatrix(0);
    
    _lastAppliedProgramObject = 0;
}

void State::popStateSet()
{
    // osg::notify(osg::NOTICE)<<"State::popStateSet()"<<_stateStateStack.size()<<std::endl;

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
        osg::notify(osg::NOTICE)<<"Warning: State::removeStateSet("<<pos<<") out of range"<<std::endl;
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

// revert to using maximum for consistency, maximum should be defined by STLport on VS.
// // visual studio 6.0 doesn't appear to define maximum?!? So do our own here.. 
// template<class T>
// T mymax(const T& a,const T& b)
// {
//     return (((a) > (b)) ? (a) : (b));
// }

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

        applyModeList(_modeMap,dstate->getModeList());
        applyAttributeList(_attributeMap,dstate->getAttributeList());

        const StateSet::TextureModeList& ds_textureModeList = dstate->getTextureModeList();
        const StateSet::TextureAttributeList& ds_textureAttributeList = dstate->getTextureAttributeList();

        unsigned int unit;
        unsigned int unitMax = maximum(static_cast<unsigned int>(ds_textureModeList.size()),static_cast<unsigned int>(ds_textureAttributeList.size()));
        unitMax = maximum(static_cast<unsigned int>(unitMax),static_cast<unsigned int>(_textureModeMapList.size()));
        unitMax = maximum(static_cast<unsigned int>(unitMax),static_cast<unsigned int>(_textureAttributeMapList.size()));
        for(unit=0;unit<unitMax;++unit)
        {
            if (setActiveTextureUnit(unit))
            {
                if (unit<ds_textureModeList.size()) applyModeList(getOrCreateTextureModeMap(unit),ds_textureModeList[unit]);
                else if (unit<_textureModeMapList.size()) applyModeMap(_textureModeMapList[unit]);
                
                if (unit<ds_textureAttributeList.size()) applyAttributeList(getOrCreateTextureAttributeMap(unit),ds_textureAttributeList[unit]);
                else if (unit<_textureAttributeMapList.size()) applyAttributeMap(_textureAttributeMapList[unit]);
            }
        }
        
#if 1        
        applyUniformList(_uniformMap,dstate->getUniformList());
#else                
        if (_lastAppliedProgramObject)
        {
            for(StateSetStack::iterator sitr=_stateStateStack.begin();
                sitr!=_stateStateStack.end();
                ++sitr)
            {
                const StateSet* stateset = *sitr;
                const StateSet::UniformList& uniformList = stateset->getUniformList();
                for(StateSet::UniformList::const_iterator itr=uniformList.begin();
                    itr!=uniformList.end();
                    ++itr)
                {
                    _lastAppliedProgramObject->apply(*(itr->second.first));
                }
            }

            const StateSet::UniformList& uniformList = dstate->getUniformList();
            for(StateSet::UniformList::const_iterator itr=uniformList.begin();
                itr!=uniformList.end();
                ++itr)
            {
                _lastAppliedProgramObject->apply(*(itr->second.first));
            }
        }
#endif

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

    // go through all active OpenGL modes, enabling/disable where
    // appropriate.
    applyModeMap(_modeMap);

    // go through all active StateAttribute's, applying where appropriate.
    applyAttributeMap(_attributeMap);
       
    unsigned int unit;
    unsigned int unitMax = maximum(_textureModeMapList.size(),_textureAttributeMapList.size());
    for(unit=0;unit<unitMax;++unit)
    {
        if (setActiveTextureUnit(unit))
        {
            if (unit<_textureModeMapList.size()) applyModeMap(_textureModeMapList[unit]);
            if (unit<_textureAttributeMapList.size()) applyAttributeMap(_textureAttributeMapList[unit]);
        }
    }

#if 1        
    applyUniformMap(_uniformMap);
#else        
    if (_lastAppliedProgramObject && !_stateStateStack.empty())
    {
        for(StateSetStack::iterator sitr=_stateStateStack.begin();
            sitr!=_stateStateStack.end();
            ++sitr)
        {
            const StateSet* stateset = *sitr;
            const StateSet::UniformList& uniformList = stateset->getUniformList();
            for(StateSet::UniformList::const_iterator itr=uniformList.begin();
                itr!=uniformList.end();
                ++itr)
            {
                _lastAppliedProgramObject->apply(*(itr->second.first));
            }
        }
    }
#endif


    if (_checkGLErrors==ONCE_PER_ATTRIBUTE) checkGLErrors("end of State::apply()");
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
    if (unit>=_textureAttributeMapList.size()) return false;
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



void State::disableAllVertexArrays()
{
    disableVertexPointer();
    disableTexCoordPointersAboveAndIncluding(0);
    disableVertexAttribPointersAboveAndIncluding(0);
    disableColorPointer();
    disableFogCoordPointer();
    disableIndexPointer();
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
    dirtyIndexPointer();
    dirtyNormalPointer();
    dirtySecondaryColorPointer();
}

void State::setInterleavedArrays( GLenum format, GLsizei stride, const GLvoid* pointer)
{
    disableAllVertexArrays();

    glInterleavedArrays( format, stride, pointer);

    // the crude way, assume that all arrays have been effected so dirty them and
    // disable them...
    dirtyAllVertexArrays();
}

void State::initializeExtensionProcs()
{
    if (_extensionProcsInitialized) return;

    setGLExtensionFuncPtr(_glClientActiveTexture,"glClientActiveTexture","glClientActiveTextureARB");
    setGLExtensionFuncPtr(_glActiveTexture, "glActiveTexture","glActiveTextureARB");
    setGLExtensionFuncPtr(_glFogCoordPointer, "glFogCoordPointer","glFogCoordPointerEXT");
    setGLExtensionFuncPtr(_glSecondaryColorPointer, "glSecondaryColorPointer","glSecondaryColorPointerEXT");
    setGLExtensionFuncPtr(_glVertexAttribPointer, "glVertexAttribPointer","glVertexAttribPointerARB");
    setGLExtensionFuncPtr(_glEnableVertexAttribArray, "glEnableVertexAttribArray","glEnableVertexAttribArrayARB");
    setGLExtensionFuncPtr(_glDisableVertexAttribArray, "glDisableVertexAttribArray","glDisableVertexAttribArrayARB");
    setGLExtensionFuncPtr(_glBindBuffer, "glBindBuffer","glBindBufferARB");

    setGLExtensionFuncPtr(_glDrawArraysInstanced, "glDrawArraysInstanced","glDrawArraysInstancedARB","glDrawArraysInstancedEXT");
    setGLExtensionFuncPtr(_glDrawElementsInstanced, "glDrawElementsInstanced","glDrawElementsInstancedARB","glDrawElementsInstancedEXT");

    if ( osg::getGLVersionNumber() >= 2.0 || osg::isGLExtensionSupported(_contextID,"GL_ARB_vertex_shader") )
    {
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,&_glMaxTextureUnits);
        glGetIntegerv(GL_MAX_TEXTURE_COORDS,&_glMaxTextureCoords);
    }
    else if ( osg::getGLVersionNumber() >= 1.3 ||
                                 osg::isGLExtensionSupported(_contextID,"GL_ARB_multitexture") ||
                                 osg::isGLExtensionSupported(_contextID,"GL_EXT_multitexture") )
    {
        GLint maxTextureUnits;
        glGetIntegerv(GL_MAX_TEXTURE_UNITS,&maxTextureUnits);
        _glMaxTextureUnits = maxTextureUnits;
        _glMaxTextureCoords = maxTextureUnits;
    }
    else
    {
        _glMaxTextureUnits = 1;
        _glMaxTextureCoords = 1;
    }

    _extensionProcsInitialized = true;
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


/** set the current texture unit, return true if selected, false if selection failed such as when multitexturing is not supported.
  * note, only updates values that change.*/
bool State::setActiveTextureUnit( unsigned int unit )
{
    if (unit!=_currentActiveTextureUnit)
    {
        if (_glActiveTexture && unit < (unsigned int)(maximum(_glMaxTextureCoords,_glMaxTextureUnits)) )
        {
            _glActiveTexture(GL_TEXTURE0+unit);
            _currentActiveTextureUnit = unit;
        }
        else
        {
            return unit==0;
        }
    }
    return true;
}

void State::setFogCoordPointer(GLenum type, GLsizei stride, const GLvoid *ptr)
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
        _fogArray._dirty = false;
    }
    
}

void State::setSecondaryColorPointer( GLint size, GLenum type,
                                      GLsizei stride, const GLvoid *ptr )
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
        _secondaryColorArray._dirty = false;
    }
}

/** wrapper around glEnableVertexAttribArrayARB(index);glVertexAttribPointerARB(..);
* note, only updates values that change.*/
void State::setVertexAttribPointer( unsigned int index,
                                      GLint size, GLenum type, GLboolean normalized, 
                                    GLsizei stride, const GLvoid *ptr )
{
    if (_glVertexAttribPointer)
    {
        if ( index >= _vertexAttribArrayList.size()) _vertexAttribArrayList.resize(index+1);
        EnabledArrayPair& eap = _vertexAttribArrayList[index];

        if (!eap._enabled || eap._dirty)
        {
            eap._enabled = true;
            _glEnableVertexAttribArray( index );
        }
        //if (eap._pointer != ptr || eap._normalized!=normalized || eap._dirty)
        {
            _glVertexAttribPointer( index, size, type, normalized, stride, ptr );
            eap._pointer = ptr;
            eap._normalized = normalized;
        }
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
                _glDisableVertexAttribArray( index );
            }
            ++index;
        }
    }
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
    _isVertexBufferObjectSupported = osg::isGLExtensionSupported(_contextID,"GL_ARB_vertex_buffer_object");
    return _isVertexBufferObjectSupported;
}

bool State::checkGLErrors(const char* str) const
{
    GLenum errorNo = glGetError();
    if (errorNo!=GL_NO_ERROR)
    {
        const char* error = (char*)gluErrorString(errorNo);
        if (error) osg::notify(WARN)<<"Warning: detected OpenGL error '" << error<<"'";
        else       osg::notify(WARN)<<"Warning: detected OpenGL error number 0x" << std::hex << errorNo << std::dec;

        if (str) osg::notify(WARN)<<" at "<<str<< std::endl;
        else     osg::notify(WARN)<<" in osg::State."<< std::endl;

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
        if (error) osg::notify(WARN)<<"Warning: detected OpenGL error '"<< error <<"' after applying GLMode 0x"<<hex<<mode<<dec<< std::endl;
        else       osg::notify(WARN)<<"Warning: detected OpenGL error number 0x"<< std::hex << errorNo <<" after applying GLMode 0x"<<hex<<mode<<dec<< std::endl;

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
        if (error) osg::notify(WARN)<<"Warning: detected OpenGL error '"<< error <<"' after applying attribute "<<attribute->className()<<" "<<attribute<< std::endl;
        else       osg::notify(WARN)<<"Warning: detected OpenGL error number 0x"<< std::hex << errorNo <<" after applying attribute "<<attribute->className()<<" "<<attribute<< std::dec << std::endl;

        return true;
    }
    return false;
}


