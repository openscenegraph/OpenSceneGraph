#include <osg/GLExtensions>
#include <osg/State>
#include <osg/Notify>
#include <osg/GLU>

#include <algorithm>

using namespace osg;

State::State()
{
    _contextID = 0;
    _identity = new osg::Matrix(); // default Matrix constructs to identity.
    _projection = _identity;
    _modelView = _identity;

    _currentActiveTextureUnit=0;
    _currentClientActiveTextureUnit=0;
}

State::~State()
{
}

void State::reset()
{
/*
    for(ModeMap::iterator mitr=_modeMap.begin();
        mitr!=_modeMap.end();
        ++mitr)
    {
        ModeStack& ms = mitr->second;
        ms.valueVec.clear();
        ms.last_applied_value = !ms.global_default_value;
        ms.changed = true;
    }        
*/

    _modeMap.clear();
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
    
    // we can do a straight clear, we arn't intrested in GL_DEPTH_TEST defaults in texture modes.
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

    _drawStateStack.clear();
    
    _modelView = _identity;
    _projection = _identity;
}

void State::pushStateSet(const StateSet* dstate)
{
    _drawStateStack.push_back(dstate);
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

    }
}

void State::popStateSet()
{
    if (_drawStateStack.empty()) return;
    
    const StateSet* dstate = _drawStateStack.back().get();

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
    }
    
    // remove the top draw state from the stack.
    _drawStateStack.pop_back();
}

void State::captureCurrentState(StateSet& stateset) const
{
    // empty the stateset first.
    stateset.setAllToInherit();
    
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

// visual studio 6.0 doesn't appear to define std::max?!? So do our own here.. 
template<class T>
T mymax(const T& a,const T& b)
{
	return (((a) > (b)) ? (a) : (b));
}

void State::apply(const StateSet* dstate)
{
    // equivilant to:
    //pushStateSet(dstate);
    //apply();
    //popStateSet();

    if (dstate)
    {

        applyModeList(_modeMap,dstate->getModeList());
        applyAttributeList(_attributeMap,dstate->getAttributeList());

        const StateSet::TextureModeList& ds_textureModeList = dstate->getTextureModeList();
        const StateSet::TextureAttributeList& ds_textureAttributeList = dstate->getTextureAttributeList();

        unsigned int unit;
        unsigned int unitMax = mymax(static_cast<unsigned int>(ds_textureModeList.size()),static_cast<unsigned int>(ds_textureAttributeList.size()));
        unitMax = mymax(static_cast<unsigned int>(unitMax),static_cast<unsigned int>(_textureModeMapList.size()));
        unitMax = mymax(static_cast<unsigned int>(unitMax),static_cast<unsigned int>(_textureAttributeMapList.size()));
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
    
    }
    else
    {
        // no incomming stateset, so simply apply state.
        apply();
    }

}

void State::apply()
{

    // go through all active OpenGL modes, enabling/disable where
    // appropriate.
    applyModeMap(_modeMap);

    // go through all active StateAttribute's, applying where appropriate.
    applyAttributeMap(_attributeMap);
       
    unsigned int unit;
    unsigned int unitMax = mymax(_textureModeMapList.size(),_textureAttributeMapList.size());
    for(unit=0;unit<unitMax;++unit)
    {
        if (setActiveTextureUnit(unit))
        {
            if (unit<_textureModeMapList.size()) applyModeMap(_textureModeMapList[unit]);
            if (unit<_textureAttributeMapList.size()) applyAttributeMap(_textureAttributeMapList[unit]);
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

void State::haveAppliedAttribute(StateAttribute::Type type)
{
    haveAppliedAttribute(_attributeMap,type);
}

bool State::getLastAppliedMode(StateAttribute::GLMode mode) const
{
    return getLastAppliedMode(_modeMap,mode);
}

const StateAttribute* State::getLastAppliedAttribute(StateAttribute::Type type) const
{
    return getLastAppliedAttribute(_attributeMap,type);
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

void State::haveAppliedTextureAttribute(unsigned int unit,StateAttribute::Type type)
{
    haveAppliedAttribute(getOrCreateTextureAttributeMap(unit),type);
}

bool State::getLastAppliedTextureMode(unsigned int unit,StateAttribute::GLMode mode) const
{
    if (unit>=_textureModeMapList.size()) return false;
    return getLastAppliedMode(_textureModeMapList[unit],mode);
}

const StateAttribute* State::getLastAppliedTextureAttribute(unsigned int unit,StateAttribute::Type type) const
{
    if (unit>=_textureAttributeMapList.size()) return false;
    return getLastAppliedAttribute(_textureAttributeMapList[unit],type);
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
        AttributeStack& as = attributeMap[attribute->getType()];

        as.last_applied_attribute = attribute;

        // will need to update this attribute on next apply so set it to changed.
        as.changed = true;
    }
}

void State::haveAppliedAttribute(AttributeMap& attributeMap,StateAttribute::Type type)
{
    
    AttributeMap::iterator itr = attributeMap.find(type);
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

const StateAttribute* State::getLastAppliedAttribute(const AttributeMap& attributeMap,StateAttribute::Type type) const
{
    AttributeMap::const_iterator itr = attributeMap.find(type);
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

Polytope State::getViewFrustum() const
{
    Polytope cv;
    cv.setToUnitFrustum();
    cv.transformProvidingInverse((*_modelView)*(*_projection));
    return cv;
}

typedef void (APIENTRY * ActiveTextureProc) (GLenum texture);

bool State::setClientActiveTextureUnit( unsigned int unit )
{
    if (unit!=_currentClientActiveTextureUnit)
    {
        static ActiveTextureProc s_glClientActiveTexture =  
                (ActiveTextureProc) osg::getGLExtensionFuncPtr("glClientActiveTexture","glClientActiveTextureARB");

        if (s_glClientActiveTexture)
        {
            s_glClientActiveTexture(GL_TEXTURE0+unit);
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
        static ActiveTextureProc s_glActiveTexture =  
               (ActiveTextureProc) osg::getGLExtensionFuncPtr("glActiveTexture","glActiveTextureARB");

        if (s_glActiveTexture)
        {
            s_glActiveTexture(GL_TEXTURE0+unit);
            _currentActiveTextureUnit = unit;
        }
        else
        {
            return unit==0;
        }
    }
    return true;
}

typedef void (APIENTRY * FogCoordPointerProc) (GLenum type, GLsizei stride, const GLvoid *pointer);
void State::setFogCoordPointer(GLenum type, GLsizei stride, const GLvoid *ptr)
{
    static FogCoordPointerProc s_glFogCoordPointer =
            (FogCoordPointerProc) osg::getGLExtensionFuncPtr("glFogCoordPointer","glFogCoordPointerEXT");

    if (s_glFogCoordPointer)
    {

        if (!_fogArray._enabled)
        {
            _fogArray._enabled = true;
            glEnableClientState(GL_FOG_COORDINATE_ARRAY);
        }
        if (_fogArray._pointer!=ptr)
        {
            _fogArray._pointer=ptr;
            s_glFogCoordPointer( type, stride, ptr );
        }
    }
    
}

typedef void (APIENTRY * SecondaryColorPointerProc) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void State::setSecondaryColorPointer( GLint size, GLenum type,
                                      GLsizei stride, const GLvoid *ptr )
{
    static SecondaryColorPointerProc s_glSecondaryColorPointer =
            (SecondaryColorPointerProc) osg::getGLExtensionFuncPtr("glSecondaryColorPointer","glSecondaryColorPointerEXT");

    if (s_glSecondaryColorPointer)
    {
        if (!_secondaryColorArray._enabled)
        {
            _secondaryColorArray._enabled = true;
            glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
        }
        if (_secondaryColorArray._pointer!=ptr)
        {
            _secondaryColorArray._pointer=ptr;
            s_glSecondaryColorPointer( size, type, stride, ptr );
        }
    }
}
