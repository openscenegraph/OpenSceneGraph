#include <stdio.h>

#include <osg/StateSet>
#include <osg/State>
#include <osg/Notify>

#include <osg/AlphaFunc>
#include <osg/Material>
#include <osg/CullFace>
#include <osg/FrontFace>
#include <osg/PolygonMode>
#include <osg/Transparency>

using namespace osg;

StateSet::StateSet()
{
    _renderingHint = DEFAULT_BIN;
    
    setRendingBinToInherit();
}


StateSet::~StateSet()
{
    // note, all attached state attributes will be automatically
    // unreferenced by ref_ptr<> and therefore there is no need to
    // delete the memory manually.
}


void StateSet::setGlobalDefaults()
{    
    _renderingHint = DEFAULT_BIN;

    setRendingBinToInherit();

    setMode(GL_LIGHTING,StateAttribute::OFF);
    setMode(GL_FOG,StateAttribute::OFF);
    setMode(GL_POINT_SMOOTH,StateAttribute::OFF);
    
    setMode(GL_TEXTURE_2D,StateAttribute::OFF);
    
    setMode(GL_TEXTURE_GEN_S,StateAttribute::OFF);
    setMode(GL_TEXTURE_GEN_T,StateAttribute::OFF);
    setMode(GL_TEXTURE_GEN_R,StateAttribute::OFF);
    setMode(GL_TEXTURE_GEN_Q,StateAttribute::OFF);

    setAttributeAndModes(new AlphaFunc,StateAttribute::OFF);
    setAttributeAndModes(new CullFace,StateAttribute::ON);
    setAttributeAndModes(new FrontFace,StateAttribute::ON);

    Material *material       = new Material;
    material->setColorMode(Material::AMBIENT_AND_DIFFUSE);
    setAttributeAndModes(material,StateAttribute::ON);
    
    setAttributeAndModes(new PolygonMode,StateAttribute::OFF);
    setAttributeAndModes(new Transparency,StateAttribute::OFF);
}


void StateSet::setAllToInherit()
{
    _renderingHint = DEFAULT_BIN;

    setRendingBinToInherit();

    _modeList.clear();
    _attributeList.clear();
}

void StateSet::setMode(const StateAttribute::GLMode mode, const StateAttribute::GLModeValue value)
{
    if ((value&StateAttribute::INHERIT)) setModeToInherit(mode);
    else _modeList[mode] = value;
}

void StateSet::setModeToInherit(const StateAttribute::GLMode mode)
{
    ModeList::iterator itr = _modeList.find(mode);
    if (itr!=_modeList.end())
    {
        _modeList.erase(itr);
    }
}

const StateAttribute::GLModeValue StateSet::getMode(const StateAttribute::GLMode mode) const
{
    ModeList::const_iterator itr = _modeList.find(mode);
    if (itr!=_modeList.end())
    {
        return itr->second;
    }
    else
        return StateAttribute::INHERIT;
}

void StateSet::setAttribute(StateAttribute *attribute, const StateAttribute::OverrideValue value)
{
    if (attribute)
    {
        if ((value&StateAttribute::INHERIT)) setAttributeToInherit(attribute->getType());
        else _attributeList[attribute->getType()] = RefAttributePair(attribute,value&StateAttribute::OVERRIDE);
    }
}

void StateSet::setAttributeAndModes(StateAttribute *attribute, const StateAttribute::GLModeValue value)
{
    if (attribute)
    {
        _attributeList[attribute->getType()] = RefAttributePair(attribute,value&StateAttribute::OVERRIDE);
        attribute->setStateSetModes(*this,value);
    }
}

void StateSet::setAttributeToInherit(const StateAttribute::Type type)
{
    AttributeList::iterator itr = _attributeList.find(type);
    if (itr!=_attributeList.end())
    {
        itr->second.first->setStateSetModes(*this,StateAttribute::INHERIT);
        _attributeList.erase(itr);
    }
}

const StateAttribute* StateSet::getAttribute(const StateAttribute::Type type) const
{
    AttributeList::const_iterator itr = _attributeList.find(type);
    if (itr!=_attributeList.end())
    {
        return itr->second.first.get();
    }
    else
        return NULL;
}

const StateSet::RefAttributePair* StateSet::getAttributePair(const StateAttribute::Type type) const
{
    AttributeList::const_iterator itr = _attributeList.find(type);
    if (itr!=_attributeList.end())
    {
        return &(itr->second);
    }
    else
        return NULL;
}

void StateSet::compile(State& state) const
{
    for(AttributeList::const_iterator itr = _attributeList.begin();
        itr!=_attributeList.end();
        ++itr)
    {
        itr->second.first->compile(state);
    }
}

void StateSet::setRenderingHint(const int hint)
{
    _renderingHint = hint;
    // temporary hack to get new render bins working.
    if (_renderingHint==TRANSPARENT_BIN)
    {
        _binMode = USE_RENDERBIN_DETAILS;
        _binNum = 1;
        _binName = "DepthSortedBin";
//        _binName = "RenderBin";
    }
}

void StateSet::setRenderBinDetails(const int binNum,const std::string& binName,const RenderBinMode mode)
{
    _binMode = mode;
    _binNum = binNum;
    _binName = binName;
}

void StateSet::setRendingBinToInherit()
{
    _binMode = INHERIT_RENDERBIN_DETAILS;
    _binNum = 0;
    _binName = "";
}
