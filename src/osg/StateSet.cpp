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
#include <osg/Depth>

using namespace osg;

StateSet::StateSet()
{
    setDataVariance(osg::StateAttribute::STATIC);

    _renderingHint = DEFAULT_BIN;
    
    setRendingBinToInherit();
}

StateSet::StateSet(const StateSet& rhs,const CopyOp& copyop):Object(rhs,copyop)
{
    _modeList = rhs._modeList;

    for(AttributeList::const_iterator itr=rhs._attributeList.begin();
        itr!=rhs._attributeList.end();
        ++itr)
    {
        StateAttribute::Type type = itr->first;
        const RefAttributePair& rap = itr->second;
        StateAttribute* attr = copyop(rap.first.get());
        if (attr) _attributeList[type]=RefAttributePair(attr,rap.second);
    }
    
    // copy texture related modes.
    _textureModeList = rhs._textureModeList;
    
    // set up the size of the texture attribute list.
    _textureAttributeList.resize(rhs._textureAttributeList.size());
    
    // copy the contents across.
    for(unsigned int i=0;i<rhs._textureAttributeList.size();++i)
    {
        
        AttributeList& lhs_attributeList = _textureAttributeList[i];
        const AttributeList& rhs_attributeList = rhs._textureAttributeList[i];
        for(AttributeList::const_iterator itr=rhs_attributeList.begin();
            itr!=rhs_attributeList.end();
            ++itr)
        {
            StateAttribute::Type type = itr->first;
            const RefAttributePair& rap = itr->second;
            StateAttribute* attr = copyop(rap.first.get());
            if (attr) lhs_attributeList[type]=RefAttributePair(attr,rap.second);
        }
    }
    
    
    _renderingHint = rhs._renderingHint;

    _binMode = rhs._binMode;
    _binNum = rhs._binNum;
    _binName = rhs._binName;
}

StateSet::~StateSet()
{
    // note, all attached state attributes will be automatically
    // unreferenced by ref_ptr<> and therefore there is no need to
    // delete the memory manually.
}

int StateSet::compare(const StateSet& rhs,bool compareAttributeContents) const
{

    if (_textureAttributeList.size()<rhs._textureAttributeList.size()) return -1;
    if (_textureAttributeList.size()>rhs._textureAttributeList.size()) return 1;
    
    for(unsigned int ai=0;ai<_textureAttributeList.size();++ai)
    {
        const AttributeList& rhs_attributeList = _textureAttributeList[ai];
        const AttributeList& lhs_attributeList = rhs._textureAttributeList[ai];
        if (compareAttributeContents)
        {
            // now check to see how the attributes compare.
            AttributeList::const_iterator lhs_attr_itr = lhs_attributeList.begin();
            AttributeList::const_iterator rhs_attr_itr = rhs_attributeList.begin();
            while (lhs_attr_itr!=lhs_attributeList.end() && rhs_attr_itr!=rhs_attributeList.end())
            {
                if      (lhs_attr_itr->first<rhs_attr_itr->first) return -1;
                else if (rhs_attr_itr->first<lhs_attr_itr->first) return 1;
                if      (*(lhs_attr_itr->second.first)<*(rhs_attr_itr->second.first)) return -1;
                else if (*(rhs_attr_itr->second.first)<*(lhs_attr_itr->second.first)) return 1;
                if      (lhs_attr_itr->second.second<rhs_attr_itr->second.second) return -1;
                else if (rhs_attr_itr->second.second<lhs_attr_itr->second.second) return 1;
                ++lhs_attr_itr;
                ++rhs_attr_itr;
            }
            if (lhs_attr_itr==lhs_attributeList.end())
            {
                if (rhs_attr_itr!=rhs_attributeList.end()) return -1;
            }
            else if (rhs_attr_itr == rhs_attributeList.end()) return 1;
        }
        else // just compare pointers.
        {
            // now check to see how the attributes compare.
            AttributeList::const_iterator lhs_attr_itr = lhs_attributeList.begin();
            AttributeList::const_iterator rhs_attr_itr = rhs_attributeList.begin();
            while (lhs_attr_itr!=lhs_attributeList.end() && rhs_attr_itr!=rhs_attributeList.end())
            {
                if      (lhs_attr_itr->first<rhs_attr_itr->first) return -1;
                else if (rhs_attr_itr->first<lhs_attr_itr->first) return 1;
                if      (lhs_attr_itr->second.first<rhs_attr_itr->second.first) return -1;
                else if (rhs_attr_itr->second.first<lhs_attr_itr->second.first) return 1;
                if      (lhs_attr_itr->second.second<rhs_attr_itr->second.second) return -1;
                else if (rhs_attr_itr->second.second<lhs_attr_itr->second.second) return 1;
                ++lhs_attr_itr;
                ++rhs_attr_itr;
            }
            if (lhs_attr_itr==lhs_attributeList.end())
            {
                if (rhs_attr_itr!=rhs_attributeList.end()) return -1;
            }
            else if (rhs_attr_itr == rhs_attributeList.end()) return 1;
        }
    }

    
    // now check the rest of the non texture attributes
    if (compareAttributeContents)
    {
        // now check to see how the attributes compare.
        AttributeList::const_iterator lhs_attr_itr = _attributeList.begin();
        AttributeList::const_iterator rhs_attr_itr = rhs._attributeList.begin();
        while (lhs_attr_itr!=_attributeList.end() && rhs_attr_itr!=rhs._attributeList.end())
        {
            if      (lhs_attr_itr->first<rhs_attr_itr->first) return -1;
            else if (rhs_attr_itr->first<lhs_attr_itr->first) return 1;
            if      (*(lhs_attr_itr->second.first)<*(rhs_attr_itr->second.first)) return -1;
            else if (*(rhs_attr_itr->second.first)<*(lhs_attr_itr->second.first)) return 1;
            if      (lhs_attr_itr->second.second<rhs_attr_itr->second.second) return -1;
            else if (rhs_attr_itr->second.second<lhs_attr_itr->second.second) return 1;
            ++lhs_attr_itr;
            ++rhs_attr_itr;
        }
        if (lhs_attr_itr==_attributeList.end())
        {
            if (rhs_attr_itr!=rhs._attributeList.end()) return -1;
        }
        else if (rhs_attr_itr == rhs._attributeList.end()) return 1;
    }
    else // just compare pointers.
    {
        // now check to see how the attributes compare.
        AttributeList::const_iterator lhs_attr_itr = _attributeList.begin();
        AttributeList::const_iterator rhs_attr_itr = rhs._attributeList.begin();
        while (lhs_attr_itr!=_attributeList.end() && rhs_attr_itr!=rhs._attributeList.end())
        {
            if      (lhs_attr_itr->first<rhs_attr_itr->first) return -1;
            else if (rhs_attr_itr->first<lhs_attr_itr->first) return 1;
            if      (lhs_attr_itr->second.first<rhs_attr_itr->second.first) return -1;
            else if (rhs_attr_itr->second.first<lhs_attr_itr->second.first) return 1;
            if      (lhs_attr_itr->second.second<rhs_attr_itr->second.second) return -1;
            else if (rhs_attr_itr->second.second<lhs_attr_itr->second.second) return 1;
            ++lhs_attr_itr;
            ++rhs_attr_itr;
        }
        if (lhs_attr_itr==_attributeList.end())
        {
            if (rhs_attr_itr!=rhs._attributeList.end()) return -1;
        }
        else if (rhs_attr_itr == rhs._attributeList.end()) return 1;
    }
    
    // we've got here so attributes must be equal...    


    if (_textureModeList.size()<rhs._textureModeList.size()) return -1;
    if (_textureModeList.size()>rhs._textureModeList.size()) return 1;

    // check to see how the modes compare.
    // first check the rest of the texture modes
    for(unsigned int ti=0;ti<_textureModeList.size();++ti)
    {
        const ModeList& lhs_modeList = _textureModeList[ti];
        const ModeList& rhs_modeList = rhs._textureModeList[ti];

        ModeList::const_iterator lhs_mode_itr = lhs_modeList.begin();
        ModeList::const_iterator rhs_mode_itr = rhs_modeList.begin();
        while (lhs_mode_itr!=lhs_modeList.end() && rhs_mode_itr!=rhs_modeList.end())
        {
            if      (lhs_mode_itr->first<rhs_mode_itr->first) return -1;
            else if (rhs_mode_itr->first<lhs_mode_itr->first) return 1;
            if      (lhs_mode_itr->second<rhs_mode_itr->second) return -1;
            else if (rhs_mode_itr->second<lhs_mode_itr->second) return 1;
            ++lhs_mode_itr;
            ++rhs_mode_itr;
        }
        if (lhs_mode_itr==lhs_modeList.end())
        {
            if (rhs_mode_itr!=rhs_modeList.end()) return -1;
        }
        else if (rhs_mode_itr == rhs_modeList.end()) return 1;
    }

    // check non texture modes.
    ModeList::const_iterator lhs_mode_itr = _modeList.begin();
    ModeList::const_iterator rhs_mode_itr = rhs._modeList.begin();
    while (lhs_mode_itr!=_modeList.end() && rhs_mode_itr!=rhs._modeList.end())
    {
        if      (lhs_mode_itr->first<rhs_mode_itr->first) return -1;
        else if (rhs_mode_itr->first<lhs_mode_itr->first) return 1;
        if      (lhs_mode_itr->second<rhs_mode_itr->second) return -1;
        else if (rhs_mode_itr->second<lhs_mode_itr->second) return 1;
        ++lhs_mode_itr;
        ++rhs_mode_itr;
    }
    if (lhs_mode_itr==_modeList.end())
    {
        if (rhs_mode_itr!=rhs._modeList.end()) return -1;
    }
    else if (rhs_mode_itr == rhs._modeList.end()) return 1;

    return 0;
}

void StateSet::setGlobalDefaults()
{    
    _renderingHint = DEFAULT_BIN;

    setRendingBinToInherit();


    setMode(GL_DEPTH_TEST,StateAttribute::ON);
    setAttributeAndModes(osgNew AlphaFunc,StateAttribute::OFF);
    setAttributeAndModes(osgNew Transparency,StateAttribute::OFF);

    Material *material       = osgNew Material;
    material->setColorMode(Material::AMBIENT_AND_DIFFUSE);
    setAttributeAndModes(material,StateAttribute::ON);
/*
    setMode(GL_LIGHTING,StateAttribute::OFF);
    setMode(GL_FOG,StateAttribute::OFF);
    setMode(GL_POINT_SMOOTH,StateAttribute::OFF);
    
    setMode(GL_TEXTURE_2D,StateAttribute::OFF);
    
    setMode(GL_TEXTURE_GEN_S,StateAttribute::OFF);
    setMode(GL_TEXTURE_GEN_T,StateAttribute::OFF);
    setMode(GL_TEXTURE_GEN_R,StateAttribute::OFF);
    setMode(GL_TEXTURE_GEN_Q,StateAttribute::OFF);

    setAttributeAndModes(osgNew AlphaFunc,StateAttribute::OFF);
    setAttributeAndModes(osgNew CullFace,StateAttribute::ON);
    setAttributeAndModes(osgNew FrontFace,StateAttribute::ON);

    Material *material       = osgNew Material;
    material->setColorMode(Material::AMBIENT_AND_DIFFUSE);
    setAttributeAndModes(material,StateAttribute::ON);
    
    setAttributeAndModes(osgNew PolygonMode,StateAttribute::OFF);
    setAttributeAndModes(osgNew Transparency,StateAttribute::OFF);
    setAttributeAndModes(osgNew Depth,StateAttribute::ON);
*/
}


void StateSet::setAllToInherit()
{
    _renderingHint = DEFAULT_BIN;

    setRendingBinToInherit();

    _modeList.clear();
    _attributeList.clear();
    
    _textureModeList.clear();
    _textureAttributeList.clear();
    
}

void StateSet::merge(const StateSet& rhs)
{
    // merge the modes of rhs into this, 
    // this overrides rhs if OVERRIDE defined in this.
    for(ModeList::const_iterator rhs_mitr = rhs._modeList.begin();
        rhs_mitr != rhs._modeList.end();
        ++rhs_mitr)
    {
        ModeList::iterator lhs_mitr = _modeList.find(rhs_mitr->first);
        if (lhs_mitr!=_modeList.end())
        {
            if (!(lhs_mitr->second & StateAttribute::OVERRIDE)) 
            {
                // override isn't on in rhs, so overrite it with incomming
                // value.
                lhs_mitr->second = rhs_mitr->second;
            }
        }
        else
        {
            // entry doesn't exist so insert it.
            _modeList.insert(*rhs_mitr);
        }
    }

    // merge the attributes of rhs into this, 
    // this overrides rhs if OVERRIDE defined in this.
    for(AttributeList::const_iterator rhs_aitr = rhs._attributeList.begin();
        rhs_aitr != rhs._attributeList.end();
        ++rhs_aitr)
    {
        AttributeList::iterator lhs_aitr = _attributeList.find(rhs_aitr->first);
        if (lhs_aitr!=_attributeList.end())
        {
            if (!(lhs_aitr->second.second & StateAttribute::OVERRIDE)) 
            {
                // override isn't on in rhs, so overrite it with incomming
                // value.
                lhs_aitr->second = rhs_aitr->second;
            }
        }
        else
        {
            // entry doesn't exist so insert it.
            _attributeList.insert(*rhs_aitr);
        }
    }


    if (_textureModeList.size()<rhs._textureModeList.size()) _textureModeList.resize(rhs._textureModeList.size());
    for(unsigned int mi=0;mi<rhs._textureModeList.size();++mi)
    {
        ModeList& lhs_modeList = _textureModeList[mi];
        const ModeList& rhs_modeList = rhs._textureModeList[mi];
        // merge the modes of rhs into this, 
        // this overrides rhs if OVERRIDE defined in this.
        for(ModeList::const_iterator rhs_mitr = rhs_modeList.begin();
            rhs_mitr != rhs_modeList.end();
            ++rhs_mitr)
        {
            ModeList::iterator lhs_mitr = lhs_modeList.find(rhs_mitr->first);
            if (lhs_mitr!=lhs_modeList.end())
            {
                if (!(lhs_mitr->second & StateAttribute::OVERRIDE)) 
                {
                    // override isn't on in rhs, so overrite it with incomming
                    // value.
                    lhs_mitr->second = rhs_mitr->second;
                }
            }
            else
            {
                // entry doesn't exist so insert it.
                lhs_modeList.insert(*rhs_mitr);
            }
        }
    }
    
    if (_textureAttributeList.size()<rhs._textureAttributeList.size()) _textureAttributeList.resize(rhs._textureAttributeList.size());
    for(unsigned int ai=0;ai<rhs._textureAttributeList.size();++ai)
    {
        AttributeList& lhs_attributeList = _textureAttributeList[ai];
        const AttributeList& rhs_attributeList = rhs._textureAttributeList[ai];
        
        // merge the attributes of rhs into this, 
        // this overrides rhs if OVERRIDE defined in this.
        for(AttributeList::const_iterator rhs_aitr = rhs_attributeList.begin();
            rhs_aitr != rhs_attributeList.end();
            ++rhs_aitr)
        {
            AttributeList::iterator lhs_aitr = lhs_attributeList.find(rhs_aitr->first);
            if (lhs_aitr!=lhs_attributeList.end())
            {
                if (!(lhs_aitr->second.second & StateAttribute::OVERRIDE)) 
                {
                    // override isn't on in rhs, so overrite it with incomming
                    // value.
                    lhs_aitr->second = rhs_aitr->second;
                }
            }
            else
            {
                // entry doesn't exist so insert it.
                lhs_attributeList.insert(*rhs_aitr);
            }
        }
    }

    // need to merge rendering hints
    // but will need to think how best to do this first
    // RO, Nov. 2001.

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

StateAttribute* StateSet::getAttribute(const StateAttribute::Type type)
{
    AttributeList::iterator itr = _attributeList.find(type);
    if (itr!=_attributeList.end())
    {
        return itr->second.first.get();
    }
    else
        return NULL;
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



void StateSet::setTextureMode(unsigned int unit,const StateAttribute::GLMode mode, const StateAttribute::GLModeValue value)
{
    ModeList& modeList = getOrCreateTextureModeList(unit);
    if ((value&StateAttribute::INHERIT)) setTextureModeToInherit(unit,mode);
    else modeList[mode] = value;
}

void StateSet::setTextureModeToInherit(unsigned int unit,const StateAttribute::GLMode mode)
{
    if (unit>=_textureModeList.size()) return;
    ModeList& modeList = _textureModeList[unit];
    ModeList::iterator itr = modeList.find(mode);
    if (itr!=modeList.end())
    {
        modeList.erase(itr);
    }
}


const StateAttribute::GLModeValue StateSet::getTextureMode(unsigned int unit,const StateAttribute::GLMode mode) const
{
    if (unit>=_textureModeList.size()) return StateAttribute::INHERIT;
    
    const ModeList& modeList = _textureModeList[unit];
    ModeList::const_iterator itr = modeList.find(mode);
    if (itr!=modeList.end())
    {
        return itr->second;
    }
    else
        return StateAttribute::INHERIT;
}

void StateSet::setTextureAttribute(unsigned int unit,StateAttribute *attribute, const StateAttribute::OverrideValue value=StateAttribute::OFF)
{
    if (attribute)
    {
        AttributeList& attributeList = getOrCreateTextureAttributeList(unit);
        if ((value&StateAttribute::INHERIT)) setAttributeToInherit(attribute->getType());
        else attributeList[attribute->getType()] = RefAttributePair(attribute,value&StateAttribute::OVERRIDE);
    }
}


void StateSet::setTextureAttributeAndModes(unsigned int unit,StateAttribute *attribute, const StateAttribute::GLModeValue value=StateAttribute::ON)
{
    if (attribute)
    {
        AttributeList& attributeList = getOrCreateTextureAttributeList(unit);
        attributeList[attribute->getType()] = RefAttributePair(attribute,value&StateAttribute::OVERRIDE);
        attribute->setStateSetModes(*this,value);
    }
}


void StateSet::setTextureAttributeToInherit(unsigned int unit,const StateAttribute::Type type)
{
    if (unit>=_textureAttributeList.size()) return;
    AttributeList& attributeList = _textureAttributeList[unit];
    AttributeList::iterator itr = attributeList.find(type);
    if (itr!=attributeList.end())
    {
        itr->second.first->setStateSetModes(*this,StateAttribute::INHERIT);
        attributeList.erase(itr);
    }
}


StateAttribute* StateSet::getTextureAttribute(unsigned int unit,const StateAttribute::Type type)
{
    if (unit>=_textureAttributeList.size()) return 0;
    AttributeList& attributeList = _textureAttributeList[unit];
    AttributeList::iterator itr = attributeList.find(type);
    if (itr!=attributeList.end())
    {
        return itr->second.first.get();
    }
    else
        return NULL;
}


const StateAttribute* StateSet::getTextureAttribute(unsigned int unit,const StateAttribute::Type type) const
{
    if (unit>=_textureAttributeList.size()) return 0;
    const AttributeList& attributeList = _textureAttributeList[unit];
    AttributeList::const_iterator itr = attributeList.find(type);
    if (itr!=attributeList.end())
    {
        return itr->second.first.get();
    }
    else
        return NULL;
}


const StateSet::RefAttributePair* StateSet::getTextureAttributePair(unsigned int unit,const StateAttribute::Type type) const
{
    if (unit>=_textureAttributeList.size()) return 0;
    const AttributeList& attributeList = _textureAttributeList[unit];
    AttributeList::const_iterator itr = attributeList.find(type);
    if (itr!=attributeList.end())
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

    for(TextureAttributeList::const_iterator taitr=_textureAttributeList.begin();
        taitr!=_textureAttributeList.end();
        ++taitr)
    {
        for(AttributeList::const_iterator itr = taitr->begin();
            itr!=taitr->end();
            ++itr)
        {
            itr->second.first->compile(state);
        }
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






