/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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
#include <stdio.h>

#include <osg/StateSet>
#include <osg/State>
#include <osg/Notify>

#include <osg/AlphaFunc>
#include <osg/Material>
#include <osg/CullFace>
#include <osg/FrontFace>
#include <osg/PolygonMode>
#include <osg/BlendFunc>
#include <osg/Depth>

#include <osg/TextureCubeMap>

#include <set>


using namespace osg;

// local class to help porting from OSG0.8.x to 0.9.x 
class TextureGLModeSet
{

    public:

        TextureGLModeSet()
        {

            _textureModeSet.insert(GL_TEXTURE_1D);
            _textureModeSet.insert(GL_TEXTURE_2D);
            _textureModeSet.insert(GL_TEXTURE_3D);

            _textureModeSet.insert(GL_TEXTURE_CUBE_MAP);
	    _textureModeSet.insert(GL_TEXTURE_RECTANGLE_NV);

            _textureModeSet.insert(GL_TEXTURE_GEN_Q);
            _textureModeSet.insert(GL_TEXTURE_GEN_R);
            _textureModeSet.insert(GL_TEXTURE_GEN_S);
            _textureModeSet.insert(GL_TEXTURE_GEN_T);
        }
        
        bool isTextureMode(StateAttribute::GLMode mode) const
        {
            return _textureModeSet.find(mode)!=_textureModeSet.end();
        }

    protected:

        std::set<StateAttribute::GLMode> _textureModeSet;
        
};

TextureGLModeSet s_textureGLModeSet;


StateSet::StateSet()
{
    setDataVariance(osg::StateAttribute::STATIC);

    _renderingHint = DEFAULT_BIN;
    
    setRenderBinToInherit();
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

int StateSet::compareModes(const ModeList& lhs,const ModeList& rhs)
{
    ModeList::const_iterator lhs_mode_itr = lhs.begin();
    ModeList::const_iterator rhs_mode_itr = rhs.begin();
    while (lhs_mode_itr!=lhs.end() && rhs_mode_itr!=rhs.end())
    {
        if      (lhs_mode_itr->first<rhs_mode_itr->first) return -1;
        else if (rhs_mode_itr->first<lhs_mode_itr->first) return 1;
        if      (lhs_mode_itr->second<rhs_mode_itr->second) return -1;
        else if (rhs_mode_itr->second<lhs_mode_itr->second) return 1;
        ++lhs_mode_itr;
        ++rhs_mode_itr;
    }
    if (lhs_mode_itr==lhs.end())
    {
        if (rhs_mode_itr!=rhs.end()) return -1;
    }
    else if (rhs_mode_itr == rhs.end()) return 1;
    return 0;
}

int StateSet::compareAttributePtrs(const AttributeList& lhs,const AttributeList& rhs)
{
    AttributeList::const_iterator lhs_attr_itr = lhs.begin();
    AttributeList::const_iterator rhs_attr_itr = rhs.begin();
    while (lhs_attr_itr!=lhs.end() && rhs_attr_itr!=rhs.end())
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
    if (lhs_attr_itr==lhs.end())
    {
        if (rhs_attr_itr!=rhs.end()) return -1;
    }
    else if (rhs_attr_itr == rhs.end()) return 1;
    return 0;
}

int StateSet::compareAttributeContents(const AttributeList& lhs,const AttributeList& rhs)
{
    AttributeList::const_iterator lhs_attr_itr = lhs.begin();
    AttributeList::const_iterator rhs_attr_itr = rhs.begin();
    while (lhs_attr_itr!=lhs.end() && rhs_attr_itr!=rhs.end())
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
    if (lhs_attr_itr==lhs.end())
    {
        if (rhs_attr_itr!=rhs.end()) return -1;
    }
    else if (rhs_attr_itr == rhs.end()) return 1;
    return 0;
}

void StateSet::setGlobalDefaults()
{    
    _renderingHint = DEFAULT_BIN;

    setRenderBinToInherit();


    setMode(GL_DEPTH_TEST,StateAttribute::ON);
    setAttributeAndModes(new AlphaFunc,StateAttribute::OFF);
    setAttributeAndModes(new BlendFunc,StateAttribute::OFF);

    Material *material       = new Material;
    material->setColorMode(Material::AMBIENT_AND_DIFFUSE);
    setAttributeAndModes(material,StateAttribute::ON);
}


void StateSet::setAllToInherit()
{
    _renderingHint = DEFAULT_BIN;

    setRenderBinToInherit();

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


void StateSet::setMode(StateAttribute::GLMode mode, StateAttribute::GLModeValue value)
{
    if (!s_textureGLModeSet.isTextureMode(mode))
    {
        setMode(_modeList,mode,value);
    }
    else
    {
        notify(NOTICE)<<"Warning: texture mode '"<<mode<<"'passed to setMode(mode,value), "<<std::endl;
        notify(NOTICE)<<"         assuming setTextureMode(unit=0,mode,value) instead."<<std::endl;
        notify(NOTICE)<<"         please change calling code to use appropriate call."<<std::endl;

        setTextureMode(0,mode,value);
    }
}

void StateSet::setModeToInherit(StateAttribute::GLMode mode)
{
    if (!s_textureGLModeSet.isTextureMode(mode))
    {
        setModeToInherit(_modeList,mode);
    }
    else
    {
        notify(NOTICE)<<"Warning: texture mode '"<<mode<<"'passed to setModeToInherit(mode), "<<std::endl;
        notify(NOTICE)<<"         assuming setTextureModeToInherit(unit=0,mode) instead."<<std::endl;
        notify(NOTICE)<<"         please change calling code to use appropriate call."<<std::endl;

        setTextureModeToInherit(0,mode);
    }

}

StateAttribute::GLModeValue StateSet::getMode(StateAttribute::GLMode mode) const
{
    if (!s_textureGLModeSet.isTextureMode(mode))
    {
        return getMode(_modeList,mode);
    }
    else
    {
        notify(NOTICE)<<"Warning: texture mode '"<<mode<<"'passed to getMode(mode), "<<std::endl;
        notify(NOTICE)<<"         assuming getTextureMode(unit=0,mode) instead."<<std::endl;
        notify(NOTICE)<<"         please change calling code to use appropriate call."<<std::endl;

        return getTextureMode(0,mode);
    }
}

void StateSet::setAttribute(StateAttribute *attribute, StateAttribute::OverrideValue value)
{
    if (attribute)
    {
        if (!attribute->isTextureAttribute())
        {
            setAttribute(_attributeList,attribute,value);
        }
        else
        {
            notify(NOTICE)<<"Warning: texture attribute '"<<attribute->className()<<"'passed to setAttribute(attr,value), "<<std::endl;
            notify(NOTICE)<<"         assuming setTextureAttribute(unit=0,attr,value) instead."<<std::endl;
            notify(NOTICE)<<"         please change calling code to use appropriate call."<<std::endl;

            setTextureAttribute(0,attribute,value);
        }
    }        
}

void StateSet::setAttributeAndModes(StateAttribute *attribute, StateAttribute::GLModeValue value)
{
    if (attribute)
    {
        if (!attribute->isTextureAttribute())
        {
            if (value&StateAttribute::INHERIT)
            {
                setAttributeToInherit(attribute->getType());
            }    
            else
            {
                setAttribute(_attributeList,attribute,value);
                setAssociatedModes(_modeList,attribute,value);
            }
        }
        else
        {
            notify(NOTICE)<<"Warning: texture attribute '"<<attribute->className()<<"' passed to setAttributeAndModes(attr,value), "<<std::endl;
            notify(NOTICE)<<"         assuming setTextureAttributeAndModes(unit=0,attr,value) instead."<<std::endl;
            notify(NOTICE)<<"         please change calling code to use appropriate call."<<std::endl;

            setTextureAttributeAndModes(0,attribute,value);
        }
    }
}

void StateSet::setAttributeToInherit(StateAttribute::Type type)
{
    AttributeList::iterator itr = _attributeList.find(type);
    if (itr!=_attributeList.end())
    {
        setAssociatedModes(_modeList,itr->second.first.get(),StateAttribute::INHERIT);
        _attributeList.erase(itr);
    }
}

StateAttribute* StateSet::getAttribute(StateAttribute::Type type)
{
    return getAttribute(_attributeList,type);
}

const StateAttribute* StateSet::getAttribute(StateAttribute::Type type) const
{
    return getAttribute(_attributeList,type);
}

const StateSet::RefAttributePair* StateSet::getAttributePair(StateAttribute::Type type) const
{
    return getAttributePair(_attributeList,type);
}

void StateSet::setAssociatedModes(const StateAttribute* attribute, StateAttribute::GLModeValue value)
{
    setAssociatedModes(_modeList,attribute,value);
}


void StateSet::setTextureMode(unsigned int unit,StateAttribute::GLMode mode, StateAttribute::GLModeValue value)
{
    if (s_textureGLModeSet.isTextureMode(mode))
    {
        setMode(getOrCreateTextureModeList(unit),mode,value);
    }
    else
    {
        notify(NOTICE)<<"Warning: non-texture mode '"<<mode<<"'passed to setTextureMode(unit,mode,value), "<<std::endl;
        notify(NOTICE)<<"         assuming setMode(mode,value) instead."<<std::endl;
        notify(NOTICE)<<"         please change calling code to use appropriate call."<<std::endl;

        setMode(mode,value);
    }
}

void StateSet::setTextureModeToInherit(unsigned int unit,StateAttribute::GLMode mode)
{
    if (s_textureGLModeSet.isTextureMode(mode))
    {
        if (unit>=_textureModeList.size()) return;
        setModeToInherit(_textureModeList[unit],mode);
    }
    else
    {
        notify(NOTICE)<<"Warning: non-texture mode '"<<mode<<"'passed to setTextureModeToInherit(unit,mode), "<<std::endl;
        notify(NOTICE)<<"         assuming setModeToInherit(unit=0,mode) instead."<<std::endl;
        notify(NOTICE)<<"         please change calling code to use appropriate call."<<std::endl;

        setModeToInherit(mode);
    }
}


StateAttribute::GLModeValue StateSet::getTextureMode(unsigned int unit,StateAttribute::GLMode mode) const
{
    if (s_textureGLModeSet.isTextureMode(mode))
    {
        if (unit>=_textureModeList.size()) return StateAttribute::INHERIT;
        return getMode(_textureModeList[unit],mode);
    }
    else
    {
        notify(NOTICE)<<"Warning: non-texture mode '"<<mode<<"'passed to geTexturetMode(unit,mode), "<<std::endl;
        notify(NOTICE)<<"         assuming getMode(mode) instead."<<std::endl;
        notify(NOTICE)<<"         please change calling code to use appropriate call."<<std::endl;

        return getMode(mode);
    }
}

void StateSet::setTextureAttribute(unsigned int unit,StateAttribute *attribute, const StateAttribute::OverrideValue value)
{
    if (attribute)
    {
        if (attribute->isTextureAttribute())
        {
            setAttribute(getOrCreateTextureAttributeList(unit),attribute,value);
        }
        else
        {
            notify(NOTICE)<<"Warning: texture attribute '"<<attribute->className()<<"' passed to setTextureAttribute(unit,attr,value), "<<std::endl;
            notify(NOTICE)<<"         assuming setAttribute(attr,value) instead."<<std::endl;
            notify(NOTICE)<<"         please change calling code to use appropriate call."<<std::endl;
            setAttribute(attribute,value);
        }
    }
}


void StateSet::setTextureAttributeAndModes(unsigned int unit,StateAttribute *attribute, StateAttribute::GLModeValue value)
{
    if (attribute)
    {
    
        if (attribute->isTextureAttribute())
        {
            if (value&StateAttribute::INHERIT)
            {
                setTextureAttributeToInherit(unit,attribute->getType());
            }
            else
            {
                setAttribute(getOrCreateTextureAttributeList(unit),attribute,value);
                setAssociatedModes(getOrCreateTextureModeList(unit),attribute,value);
            }
        }
        else
        {
            notify(NOTICE)<<"Warning: non texture attribute '"<<attribute->className()<<"' passed to setTextureAttributeAndModes(unit,attr,value), "<<std::endl;
            notify(NOTICE)<<"         assuming setAttributeAndModes(attr,value) instead."<<std::endl;
            notify(NOTICE)<<"         please change calling code to use appropriate call."<<std::endl;
            setAttribute(attribute,value);
        }
    }
}


void StateSet::setTextureAttributeToInherit(unsigned int unit,StateAttribute::Type type)
{
    if (unit>=_textureAttributeList.size()) return;
    AttributeList& attributeList = _textureAttributeList[unit];
    AttributeList::iterator itr = attributeList.find(type);
    if (itr!=attributeList.end())
    {
        if (unit<_textureModeList.size())
        {
            setAssociatedModes(_textureModeList[unit],itr->second.first.get(),StateAttribute::INHERIT);
        }
        attributeList.erase(itr);
    }
}


StateAttribute* StateSet::getTextureAttribute(unsigned int unit,StateAttribute::Type type)
{
    if (unit>=_textureAttributeList.size()) return 0;
    return getAttribute(_textureAttributeList[unit],type);
}


const StateAttribute* StateSet::getTextureAttribute(unsigned int unit,StateAttribute::Type type) const
{
    if (unit>=_textureAttributeList.size()) return 0;
    return getAttribute(_textureAttributeList[unit],type);
}


const StateSet::RefAttributePair* StateSet::getTextureAttributePair(unsigned int unit,StateAttribute::Type type) const
{
    if (unit>=_textureAttributeList.size()) return 0;
    return getAttributePair(_textureAttributeList[unit],type);
}

void StateSet::setAssociatedTextureModes(unsigned int unit,const StateAttribute* attribute, StateAttribute::GLModeValue value)
{
    setAssociatedModes(getOrCreateTextureModeList(unit),attribute,value);
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

void StateSet::setRenderingHint(int hint)
{
    _renderingHint = hint;
    // temporary hack to get new render bins working.
    switch(_renderingHint)
    {
        case(TRANSPARENT_BIN):
        {
            _binMode = USE_RENDERBIN_DETAILS;
            _binNum = 10;
            _binName = "DepthSortedBin";
            break;
        }
        case(OPAQUE_BIN):
        {
            _binMode = USE_RENDERBIN_DETAILS;
            _binNum = 0;
            _binName = "RenderBin";
            break;
        }
        default: // DEFAULT_BIN
        {
            setRenderBinToInherit();
            break;
        }
    }
}

void StateSet::setRenderBinDetails(int binNum,const std::string& binName,RenderBinMode mode)
{
    _binMode = mode;
    _binNum = binNum;
    _binName = binName;
}

void StateSet::setRenderBinToInherit()
{
    _binMode = INHERIT_RENDERBIN_DETAILS;
    _binNum = 0;
    _binName = "";
}

void StateSet::setMode(ModeList& modeList,StateAttribute::GLMode mode, StateAttribute::GLModeValue value)
{
    if ((value&StateAttribute::INHERIT)) setModeToInherit(modeList,mode);
    else modeList[mode] = value;
}

void StateSet::setModeToInherit(ModeList& modeList,StateAttribute::GLMode mode)
{
    ModeList::iterator itr = modeList.find(mode);
    if (itr!=modeList.end())
    {
        modeList.erase(itr);
    }
}

StateAttribute::GLModeValue StateSet::getMode(const ModeList& modeList,StateAttribute::GLMode mode) const
{
    ModeList::const_iterator itr = modeList.find(mode);
    if (itr!=modeList.end())
    {
        return itr->second;
    }
    else
        return StateAttribute::INHERIT;
}

void StateSet::setAssociatedModes(ModeList& modeList,const StateAttribute* attribute, StateAttribute::GLModeValue value)
{
    // get the associated modes.
    std::vector<StateAttribute::GLMode> modes;
    attribute->getAssociatedModes(modes);

    // set the modes on the StateSet.
    for(std::vector<StateAttribute::GLMode>::iterator itr=modes.begin();
        itr!=modes.end();
        ++itr)
    {
        setMode(modeList,*itr,value);
    }
}

void StateSet::setAttribute(AttributeList& attributeList,StateAttribute *attribute, const StateAttribute::OverrideValue value)
{
    if (attribute)
    {
        attributeList[attribute->getType()] = RefAttributePair(attribute,value&StateAttribute::OVERRIDE);
    }
}


StateAttribute* StateSet::getAttribute(AttributeList& attributeList,StateAttribute::Type type)
{
    AttributeList::iterator itr = attributeList.find(type);
    if (itr!=attributeList.end())
    {
        return itr->second.first.get();
    }
    else
        return NULL;
}

const StateAttribute* StateSet::getAttribute(const AttributeList& attributeList,StateAttribute::Type type) const
{
    AttributeList::const_iterator itr = attributeList.find(type);
    if (itr!=attributeList.end())
    {
        return itr->second.first.get();
    }
    else
        return NULL;
}

const StateSet::RefAttributePair* StateSet::getAttributePair(const AttributeList& attributeList,StateAttribute::Type type) const
{
    AttributeList::const_iterator itr = attributeList.find(type);
    if (itr!=attributeList.end())
    {
        return &(itr->second);
    }
    else
        return NULL;
}




