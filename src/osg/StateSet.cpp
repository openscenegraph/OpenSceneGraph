/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2008 Robert Osfield
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
#include <osg/Material>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Node>
#include <osg/NodeVisitor>

#include <osg/TexGen>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/TextureCubeMap>
#include <osg/TextureRectangle>
#include <osg/Texture2DArray>

#include <set>
#include <algorithm>

using namespace osg;


#if (!defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GLES3_AVAILABLE))
    #define GLSL_VERSION_STR "330 core"
#else
    #define GLSL_VERSION_STR "300 es"
#endif

static const char* gl3_VertexShader = {
    "#version " GLSL_VERSION_STR "\n"
    "// gl3_VertexShader\n"
    "#ifdef GL_ES\n"
    "    precision highp float;\n"
    "#endif\n"
    "in vec4 osg_Vertex;\n"
    "in vec4 osg_Color;\n"
    "in vec4 osg_MultiTexCoord0;\n"
    "uniform mat4 osg_ModelViewProjectionMatrix;\n"
    "out vec2 texCoord;\n"
    "out vec4 vertexColor;\n"
    "void main(void)\n"
    "{\n"
    "    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
    "    texCoord = osg_MultiTexCoord0.xy;\n"
    "    vertexColor = osg_Color; \n"
    "}\n"
};

static const char* gl3_FragmentShader = {
    "#version " GLSL_VERSION_STR "\n"
    "// gl3_FragmentShader\n"
    "#ifdef GL_ES\n"
    "    precision highp float;\n"
    "#endif\n"
    "uniform sampler2D baseTexture;\n"
    "in vec2 texCoord;\n"
    "in vec4 vertexColor;\n"
    "out vec4 color;\n"
    "void main(void)\n"
    "{\n"
    "    color = vertexColor * texture(baseTexture, texCoord);\n"
    "}\n"
};


static const char* gl2_VertexShader = {
    "// gl2_VertexShader\n"
    "#ifdef GL_ES\n"
    "    precision highp float;\n"
    "#endif\n"
    "varying vec2 texCoord;\n"
    "varying vec4 vertexColor;\n"
    "void main(void)\n"
    "{\n"
    "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
    "    texCoord = gl_MultiTexCoord0.xy;\n"
    "    vertexColor = gl_Color; \n"
    "}\n"
};

static const char* gl2_FragmentShader = {
    "// gl2_FragmentShader\n"
    "#ifdef GL_ES\n"
    "    precision highp float;\n"
    "#endif\n"
    "uniform sampler2D baseTexture;\n"
    "varying vec2 texCoord;\n"
    "varying vec4 vertexColor;\n"
    "void main(void)\n"
    "{\n"
    "    gl_FragColor = vertexColor * texture2D(baseTexture, texCoord);\n"
    "}\n"
};


extern osg::Texture2D* createDefaultTexture()
{
    osg::ref_ptr<osg::Image> image = new osg::Image;
    image->allocateImage(1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE);
    image->setColor(osg::Vec4(1.0,1.0,1.0,1.0), 0, 0, 0);

    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(image.get());
    return texture.release();
}


// local class to help porting from OSG0.8.x to 0.9.x
class TextureGLModeSet
{

    public:

        TextureGLModeSet()
        {

            _textureModeSet.insert(GL_TEXTURE_1D);
            _textureModeSet.insert(GL_TEXTURE_2D);
            _textureModeSet.insert(GL_TEXTURE_3D);
            _textureModeSet.insert(GL_TEXTURE_BUFFER);

            _textureModeSet.insert(GL_TEXTURE_CUBE_MAP);
            _textureModeSet.insert(GL_TEXTURE_RECTANGLE_NV);
            _textureModeSet.insert(GL_TEXTURE_2D_ARRAY);
            _textureModeSet.insert(GL_TEXTURE_2D_MULTISAMPLE);

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

static TextureGLModeSet& getTextureGLModeSet()
{
    static TextureGLModeSet s_textureGLModeSet;
    return s_textureGLModeSet;
}

bool osg::isTextureMode(StateAttribute::GLMode mode)
{
    return getTextureGLModeSet().isTextureMode(mode);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// StateAttributeCallback
//
bool StateSet::Callback::run(osg::Object* object, osg::Object* data)
{
    osg::StateSet* ss = object->asStateSet();
    osg::NodeVisitor* nv = data->asNodeVisitor();
    if (ss && nv)
    {
        operator()(ss, nv);
        return true;
    }
    else
    {
        return traverse(object, data);
    }
}

StateSet::StateSet():
    Object(true),
    _nestRenderBins(true)
{
    _renderingHint = DEFAULT_BIN;

    _numChildrenRequiringUpdateTraversal = 0;
    _numChildrenRequiringEventTraversal = 0;

    setRenderBinToInherit();
}

StateSet::StateSet(const StateSet& rhs,const CopyOp& copyop):Object(rhs,copyop),
    _nestRenderBins(rhs._nestRenderBins)
{
    _modeList = rhs._modeList;

    for(AttributeList::const_iterator itr=rhs._attributeList.begin();
        itr!=rhs._attributeList.end();
        ++itr)
    {
        const StateAttribute::TypeMemberPair& typemember = itr->first;
        const RefAttributePair& rap = itr->second;
        StateAttribute* attr = copyop(rap.first.get());
        if (attr)
        {
            _attributeList[typemember]=RefAttributePair(attr,rap.second);
            attr->addParent(this);
        }
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
            const StateAttribute::TypeMemberPair& typemember = itr->first;
            const RefAttributePair& rap = itr->second;
            StateAttribute* attr = copyop(rap.first.get());
            if (attr)
            {
                lhs_attributeList[typemember]=RefAttributePair(attr,rap.second);
                attr->addParent(this);
            }
        }
    }

    // copy uniform values
    for(UniformList::const_iterator rhs_uitr = rhs._uniformList.begin();
        rhs_uitr != rhs._uniformList.end();
        ++rhs_uitr)
    {
        const std::string& name = rhs_uitr->first;
        const RefUniformPair& rup = rhs_uitr->second;
        Uniform* uni = copyop(rup.first.get());
        if (uni)
        {
            _uniformList[name] = RefUniformPair(uni, rup.second);
            uni->addParent(this);
        }
    }

    _defineList = rhs._defineList;

    _renderingHint = rhs._renderingHint;

    _binMode = rhs._binMode;
    _binNum = rhs._binNum;
    _binName = rhs._binName;

    _updateCallback = rhs._updateCallback;
    _numChildrenRequiringUpdateTraversal = rhs._numChildrenRequiringUpdateTraversal;

    _eventCallback = rhs._eventCallback;
    _numChildrenRequiringEventTraversal = rhs._numChildrenRequiringEventTraversal;

}

StateSet::~StateSet()
{
    clear();
}

void StateSet::computeDataVariance()
{
    bool dynamic = false;

    if (_updateCallback.valid() ||
        _eventCallback.valid())
    {
        dynamic = true;
    }

    // run attribute callbacks

    for(AttributeList::iterator itr=_attributeList.begin();
        itr!=_attributeList.end();
        ++itr)
    {
        if (itr->second.first->getDataVariance()==UNSPECIFIED &&
            (itr->second.first->getUpdateCallback() || itr->second.first->getEventCallback()))
        {
            itr->second.first->setDataVariance(DYNAMIC);
        }

        if (itr->second.first->getDataVariance()==DYNAMIC) dynamic = true;
    }

    // run texture attribute callbacks.
    for(unsigned int i=0;i<_textureAttributeList.size();++i)
    {
        AttributeList& attributeList = _textureAttributeList[i];
        for(AttributeList::iterator itr=attributeList.begin();
            itr!=attributeList.end();
            ++itr)
        {
            if (itr->second.first->getDataVariance()==UNSPECIFIED &&
                (itr->second.first->getUpdateCallback() || itr->second.first->getEventCallback()))
            {
                itr->second.first->setDataVariance(DYNAMIC);
            }

            if (itr->second.first->getDataVariance()==DYNAMIC) dynamic = true;
        }
    }


    // run uniform callbacks.
    for(UniformList::iterator uitr = _uniformList.begin();
        uitr != _uniformList.end();
        ++uitr)
    {
        if (uitr->second.first->getDataVariance()==UNSPECIFIED &&
            (uitr->second.first->getUpdateCallback() || uitr->second.first->getEventCallback()))
        {
            uitr->second.first->setDataVariance(DYNAMIC);
        }

        if (uitr->second.first->getDataVariance()==DYNAMIC) dynamic = true;
    }

#if 0

    if (dynamic)
    {
        OSG_NOTICE<<"StateSet::computeDataVariance setting to DYNAMIC"<<std::endl;
    }
    else
    {
        OSG_NOTICE<<"StateSet::computeDataVariance to STATIC"<<std::endl;
    }
#endif

    if (getDataVariance()==UNSPECIFIED)
    {
        setDataVariance(dynamic ? DYNAMIC : STATIC);
    }
}


void StateSet::addParent(osg::Node* node)
{
    // OSG_DEBUG_FP<<"Adding parent"<<std::endl;
    OpenThreads::ScopedPointerLock<OpenThreads::Mutex> lock(getRefMutex());

    _parents.push_back(node);
}

void StateSet::removeParent(osg::Node* node)
{
    OpenThreads::ScopedPointerLock<OpenThreads::Mutex> lock(getRefMutex());

    ParentList::iterator pitr = std::find(_parents.begin(),_parents.end(),node);
    if (pitr!=_parents.end()) _parents.erase(pitr);
}

int StateSet::compare(const StateSet& rhs,bool compareAttributeContents) const
{

    if (_textureAttributeList.size()<rhs._textureAttributeList.size()) return -1;
    if (_textureAttributeList.size()>rhs._textureAttributeList.size()) return 1;

    if (_textureModeList.size()<rhs._textureModeList.size()) return -1;
    if (_textureModeList.size()>rhs._textureModeList.size()) return 1;

    if (_attributeList.size()<rhs._attributeList.size()) return -1;
    if (_attributeList.size()>rhs._attributeList.size()) return 1;

    if (_modeList.size()<rhs._modeList.size()) return -1;
    if (_modeList.size()>rhs._modeList.size()) return 1;

    if (_uniformList.size()<rhs._uniformList.size()) return -1;
    if (_uniformList.size()>rhs._uniformList.size()) return 1;

    if (_defineList.size()<rhs._defineList.size()) return -1;
    if (_defineList.size()>rhs._defineList.size()) return 1;


    // check render bin details

    if ( _binMode < rhs._binMode ) return -1;
    else if ( _binMode > rhs._binMode ) return 1;

    if ( _binMode != INHERIT_RENDERBIN_DETAILS )
    {
        if ( _binNum < rhs._binNum ) return -1;
        else if ( _binNum > rhs._binNum ) return 1;

        if ( _binName < rhs._binName ) return -1;
        else if ( _binName > rhs._binName ) return 1;
    }

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


    // check uniforms.
    UniformList::const_iterator lhs_uniform_itr = _uniformList.begin();
    UniformList::const_iterator rhs_uniform_itr = rhs._uniformList.begin();
    while (lhs_uniform_itr!=_uniformList.end() && rhs_uniform_itr!=rhs._uniformList.end())
    {
        if      (lhs_uniform_itr->first<rhs_uniform_itr->first) return -1;
        else if (rhs_uniform_itr->first<lhs_uniform_itr->first) return 1;
        if      (*lhs_uniform_itr->second.first<*rhs_uniform_itr->second.first) return -1;
        else if (*rhs_uniform_itr->second.first<*lhs_uniform_itr->second.first) return 1;
        if      (lhs_uniform_itr->second.second<rhs_uniform_itr->second.second) return -1;
        else if (rhs_uniform_itr->second.second<lhs_uniform_itr->second.second) return 1;
        ++lhs_uniform_itr;
        ++rhs_uniform_itr;
    }
    if (lhs_uniform_itr==_uniformList.end())
    {
        if (rhs_uniform_itr!=rhs._uniformList.end()) return -1;
    }
    else if (rhs_uniform_itr == rhs._uniformList.end()) return 1;


    // check defines.
    DefineList::const_iterator lhs_define_itr = _defineList.begin();
    DefineList::const_iterator rhs_define_itr = rhs._defineList.begin();
    while (lhs_define_itr!=_defineList.end() && rhs_define_itr!=rhs._defineList.end())
    {
        if      (lhs_define_itr->first<rhs_define_itr->first) return -1;
        else if (rhs_define_itr->first<lhs_define_itr->first) return 1;
        if      (lhs_define_itr->second.first<rhs_define_itr->second.first) return -1;
        else if (rhs_define_itr->second.first<lhs_define_itr->second.first) return 1;
        if      (lhs_define_itr->second.second<rhs_define_itr->second.second) return -1;
        else if (rhs_define_itr->second.second<lhs_define_itr->second.second) return 1;
        ++lhs_define_itr;
        ++rhs_define_itr;
    }
    if (lhs_define_itr==_defineList.end())
    {
        if (rhs_define_itr!=rhs._defineList.end()) return -1;
    }
    else if (rhs_define_itr == rhs._defineList.end()) return 1;


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
    setAttributeAndModes(new BlendFunc,StateAttribute::OFF);

    #if defined(OSG_GL_FIXED_FUNCTION_AVAILABLE)

        // setAttributeAndModes(new AlphaFunc,StateAttribute::OFF);

        Material *material       = new Material;
        material->setColorMode(Material::AMBIENT_AND_DIFFUSE);
        setAttributeAndModes(material,StateAttribute::ON);

    #endif


    OSG_INFO<<"void StateSet::setGlobalDefaults()"<<std::endl;

    osg::DisplaySettings::ShaderHint shaderHint = osg::DisplaySettings::instance()->getShaderHint();
    if (shaderHint==osg::DisplaySettings::SHADER_GL3 || shaderHint==osg::DisplaySettings::SHADER_GLES3)
    {
        OSG_INFO<<"   StateSet::setGlobalDefaults() Setting up GL3 compatible shaders"<<std::endl;

        osg::ref_ptr<osg::Program> program = new osg::Program;
        program->addShader(new osg::Shader(osg::Shader::VERTEX, gl3_VertexShader));
        program->addShader(new osg::Shader(osg::Shader::FRAGMENT, gl3_FragmentShader));
        setAttributeAndModes(program.get());
        setTextureAttribute(0, createDefaultTexture());
        addUniform(new osg::Uniform("baseTexture", 0));
    }
    else if (shaderHint==osg::DisplaySettings::SHADER_GL2 || shaderHint==osg::DisplaySettings::SHADER_GLES2)
    {

        OSG_INFO<<"   StateSet::setGlobalDefaults() Setting up GL2 compatible shaders"<<std::endl;

        osg::ref_ptr<osg::Program> program = new osg::Program;
        program->addShader(new osg::Shader(osg::Shader::VERTEX, gl2_VertexShader));
        program->addShader(new osg::Shader(osg::Shader::FRAGMENT, gl2_FragmentShader));
        setAttributeAndModes(program.get());
        setTextureAttribute(0, createDefaultTexture());
        addUniform(new osg::Uniform("baseTexture", 0));
    }
}


void StateSet::clear()
{
    _renderingHint = DEFAULT_BIN;

    setRenderBinToInherit();


    // remove self from as attributes parent
    for(AttributeList::iterator itr=_attributeList.begin();
        itr!=_attributeList.end();
        ++itr)
    {
        itr->second.first->removeParent(this);
    }

    _modeList.clear();
    _attributeList.clear();


    // remove self from as texture attributes parent
    for(unsigned int i=0;i<_textureAttributeList.size();++i)
    {
        AttributeList& attributeList = _textureAttributeList[i];
        for(AttributeList::iterator itr=attributeList.begin();
            itr!=attributeList.end();
            ++itr)
        {
            itr->second.first->removeParent(this);
        }
    }

    _textureModeList.clear();
    _textureAttributeList.clear();


    // remove self from uniforms parent
    for(UniformList::iterator uitr = _uniformList.begin();
        uitr != _uniformList.end();
        ++uitr)
    {
        uitr->second.first->removeParent(this);
    }

    _uniformList.clear();
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
            // take the rhs mode unless the lhs is override and the rhs is not protected
            if (!(lhs_mitr->second & StateAttribute::OVERRIDE ) ||
                 (rhs_mitr->second & StateAttribute::PROTECTED))
            {
                // override isn't on in rhs, so override it with incoming
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
            // take the rhs attribute unless the lhs is override and the rhs is not protected
            if (!(lhs_aitr->second.second & StateAttribute::OVERRIDE) ||
                 (rhs_aitr->second.second & StateAttribute::PROTECTED))
            {
                // override isn't on in rhs, so override it with incoming
                // value.
                if (lhs_aitr->second.first!=rhs_aitr->second.first)
                {
                    // new attribute so need to remove self from outgoing attribute
                    lhs_aitr->second.first->removeParent(this);

                    // override isn't on in rhs, so override it with incoming
                    // value.
                    lhs_aitr->second = rhs_aitr->second;
                    lhs_aitr->second.first->addParent(this);

                }
                else
                {
                    // same attribute but with override to set.
                    lhs_aitr->second = rhs_aitr->second;
                }

            }
        }
        else
        {
            // entry doesn't exist so insert it, and then tell it about self by adding self as parent.
            _attributeList.insert(*rhs_aitr).first->second.first->addParent(this);
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
                // take the rhs mode unless the lhs is override and the rhs is not protected
                if (!(lhs_mitr->second & StateAttribute::OVERRIDE) ||
                     (rhs_mitr->second & StateAttribute::PROTECTED))
                {
                    // override isn't on in rhs, so override it with incoming
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
                // take the rhs attribute unless the lhs is override and the rhs is not protected
                if (!(lhs_aitr->second.second & StateAttribute::OVERRIDE) ||
                     (rhs_aitr->second.second & StateAttribute::PROTECTED))
                {
                    // override isn't on in rhs, so override it with incoming
                    // value.

                    if (lhs_aitr->second.first!=rhs_aitr->second.first)
                    {
                        lhs_aitr->second.first->removeParent(this);
                        lhs_aitr->second = rhs_aitr->second;
                        lhs_aitr->second.first->addParent(this);
                    }
                    else
                    {
                        lhs_aitr->second = rhs_aitr->second;
                    }
                }
            }
            else
            {
                // entry doesn't exist so insert it and add self as parent
                lhs_attributeList.insert(*rhs_aitr).first->second.first->addParent(this);
            }
        }
    }

    // merge the uniforms of rhs into this,
    // this overrides rhs if OVERRIDE defined in this.
    for(UniformList::const_iterator rhs_uitr = rhs._uniformList.begin();
        rhs_uitr != rhs._uniformList.end();
        ++rhs_uitr)
    {
        UniformList::iterator lhs_uitr = _uniformList.find(rhs_uitr->first);
        if (lhs_uitr!=_uniformList.end())
        {
            // take the rhs uniform unless the lhs is override and the rhs is not protected
            if (!(lhs_uitr->second.second & StateAttribute::OVERRIDE) ||
                 (rhs_uitr->second.second & StateAttribute::PROTECTED))
            {
                // override isn't on in rhs, so override it with incoming
                // value.

                if (lhs_uitr->second.first!=rhs_uitr->second.first)
                {
                    lhs_uitr->second.first->removeParent(this);
                    lhs_uitr->second = rhs_uitr->second;
                    lhs_uitr->second.first->addParent(this);
                }
                else
                {
                    lhs_uitr->second = rhs_uitr->second;
                }

            }
        }
        else
        {
            // entry doesn't exist so insert it and add self as parent
            _uniformList.insert(*rhs_uitr).first->second.first->addParent(this);
        }
    }

    // merge the defines of rhs into this,
    // this overrides rhs if OVERRIDE defined in this.
    for(DefineList::const_iterator rhs_mitr = rhs._defineList.begin();
        rhs_mitr != rhs._defineList.end();
        ++rhs_mitr)
    {
        DefineList::iterator lhs_mitr = _defineList.find(rhs_mitr->first);
        if (lhs_mitr!=_defineList.end())
        {
            // take the rhs mode unless the lhs is override and the rhs is not protected
            if (!(lhs_mitr->second.second & StateAttribute::OVERRIDE ) ||
                 (rhs_mitr->second.second & StateAttribute::PROTECTED))
            {
                // override isn't on in rhs, so override it with incoming
                // value.
                lhs_mitr->second = rhs_mitr->second;
            }
        }
        else
        {
            // entry doesn't exist so insert it.
            _defineList.insert(*rhs_mitr);
        }
    }


    // Merge RenderBin state from rhs into this.
    // Only do so if this's RenderBinMode is INHERIT.
    if (getRenderBinMode() == INHERIT_RENDERBIN_DETAILS)
    {
        setRenderingHint( rhs.getRenderingHint() );
        setRenderBinMode( rhs.getRenderBinMode() );
        setBinNumber( rhs.getBinNumber() );
        setBinName( rhs.getBinName() );
    }
}


void StateSet::setMode(StateAttribute::GLMode mode, StateAttribute::GLModeValue value)
{
    if (getTextureGLModeSet().isTextureMode(mode))
    {
        OSG_NOTICE<<"Warning: texture mode '"<<mode<<"'passed to setMode(mode,value), "<<std::endl;
        OSG_NOTICE<<"         assuming setTextureMode(unit=0,mode,value) instead."<<std::endl;
        OSG_NOTICE<<"         please change calling code to use appropriate call."<<std::endl;

        setTextureMode(0,mode,value);
    }
    else if (mode == GL_COLOR_MATERIAL)
    {
        OSG_NOTICE<<"Error: Setting mode 'GL_COLOR_MATERIAL' via osg::StateSet::setMode(mode,value) ignored.\n";
        OSG_NOTICE<<"       The mode 'GL_COLOR_MATERIAL' is set by the osg::Material StateAttribute.\n";
        OSG_NOTICE<<"       Setting this mode would confuse osg's State tracking."<<std::endl;
    }
    else
    {
        setMode(_modeList,mode,value);
    }
}

void StateSet::removeMode(StateAttribute::GLMode mode)
{
    if (getTextureGLModeSet().isTextureMode(mode))
    {
        OSG_NOTICE<<"Warning: texture mode '"<<mode<<"'passed to setModeToInherit(mode), "<<std::endl;
        OSG_NOTICE<<"         assuming setTextureModeToInherit(unit=0,mode) instead."<<std::endl;
        OSG_NOTICE<<"         please change calling code to use appropriate call."<<std::endl;

        removeTextureMode(0,mode);
    }
    else if (mode == GL_COLOR_MATERIAL)
    {
        OSG_NOTICE<<"Error: Setting mode 'GL_COLOR_MATERIAL' via osg::StateSet::removeMode(mode) ignored.\n";
        OSG_NOTICE<<"       The mode 'GL_COLOR_MATERIAL' is set by the osg::Material StateAttribute.\n";
        OSG_NOTICE<<"       Setting this mode would confuse osg's State tracking."<<std::endl;
    }
    else
    {
        setModeToInherit(_modeList,mode);
    }

}

StateAttribute::GLModeValue StateSet::getMode(StateAttribute::GLMode mode) const
{
    if (!getTextureGLModeSet().isTextureMode(mode))
    {
        return getMode(_modeList,mode);
    }
    else
    {
        OSG_NOTICE<<"Warning: texture mode '"<<mode<<"'passed to getMode(mode), "<<std::endl;
        OSG_NOTICE<<"         assuming getTextureMode(unit=0,mode) instead."<<std::endl;
        OSG_NOTICE<<"         please change calling code to use appropriate call."<<std::endl;

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
            OSG_NOTICE<<"Warning: texture attribute '"<<attribute->className()<<"'passed to setAttribute(attr,value), "<<std::endl;
            OSG_NOTICE<<"         assuming setTextureAttribute(unit=0,attr,value) instead."<<std::endl;
            OSG_NOTICE<<"         please change calling code to use appropriate call."<<std::endl;

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
                removeAttribute(attribute->getType());
            }
            else
            {
                setAttribute(_attributeList,attribute,value);
                setAssociatedModes(attribute,value);
            }
        }
        else
        {
            OSG_NOTICE<<"Warning: texture attribute '"<<attribute->className()<<"' passed to setAttributeAndModes(attr,value), "<<std::endl;
            OSG_NOTICE<<"         assuming setTextureAttributeAndModes(unit=0,attr,value) instead."<<std::endl;
            OSG_NOTICE<<"         please change calling code to use appropriate call."<<std::endl;

            setTextureAttributeAndModes(0,attribute,value);
        }
    }
}

void StateSet::removeAttribute(StateAttribute::Type type, unsigned int member)
{
    AttributeList::iterator itr = _attributeList.find(StateAttribute::TypeMemberPair(type,member));
    if (itr!=_attributeList.end())
    {
        if (itr->second.first->getUpdateCallback())
        {
            setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()-1);
        }

        if (itr->second.first->getEventCallback())
        {
            setNumChildrenRequiringEventTraversal(getNumChildrenRequiringEventTraversal()-1);
        }

        itr->second.first->removeParent(this);
        setAssociatedModes(itr->second.first.get(),StateAttribute::INHERIT);
        _attributeList.erase(itr);
    }
}

void StateSet::removeAttribute(StateAttribute* attribute)
{
    if (!attribute) return;

    AttributeList::iterator itr = _attributeList.find(attribute->getTypeMemberPair());
    if (itr!=_attributeList.end())
    {
        if (itr->second.first != attribute) return;

        if (itr->second.first->getUpdateCallback())
        {
            setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()-1);
        }

        if (itr->second.first->getEventCallback())
        {
            setNumChildrenRequiringEventTraversal(getNumChildrenRequiringEventTraversal()-1);
        }

        itr->second.first->removeParent(this);

        setAssociatedModes(itr->second.first.get(),StateAttribute::INHERIT);
        _attributeList.erase(itr);
    }
}

StateAttribute* StateSet::getAttribute(StateAttribute::Type type, unsigned int member)
{
    return getAttribute(_attributeList,type,member);
}

const StateAttribute* StateSet::getAttribute(StateAttribute::Type type, unsigned int member) const
{
    return getAttribute(_attributeList,type,member);
}

StateSet::RefAttributePair* StateSet::getAttributePair(StateAttribute::Type type, unsigned int member)
{
    return getAttributePair(_attributeList,type,member);
}

const StateSet::RefAttributePair* StateSet::getAttributePair(StateAttribute::Type type, unsigned int member) const
{
    return getAttributePair(_attributeList,type,member);
}

void StateSet::addUniform(Uniform* uniform, StateAttribute::OverrideValue value)
{
    if (uniform)
    {
        int delta_update = 0;
        int delta_event = 0;

        UniformList::iterator itr=_uniformList.find(uniform->getName());
        if (itr==_uniformList.end())
        {
            // new entry.
            RefUniformPair& up = _uniformList[uniform->getName()];
            up.first = uniform;
            up.second = value&(StateAttribute::OVERRIDE|StateAttribute::PROTECTED);

            uniform->addParent(this);

            if (uniform->getUpdateCallback())
            {
                delta_update = 1;
            }

            if (uniform->getEventCallback())
            {
                delta_event = 1;
            }
        }
        else
        {
            if (itr->second.first==uniform)
            {
                // changing just override
                itr->second.second = value&(StateAttribute::OVERRIDE|StateAttribute::PROTECTED);
            }
            else
            {
                itr->second.first->removeParent(this);
                if (itr->second.first->getUpdateCallback()) --delta_update;
                if (itr->second.first->getEventCallback()) --delta_event;

                uniform->addParent(this);
                itr->second.first = uniform;
                if (itr->second.first->getUpdateCallback()) ++delta_update;
                if (itr->second.first->getEventCallback()) ++delta_event;

                itr->second.second = value&(StateAttribute::OVERRIDE|StateAttribute::PROTECTED);
            }
        }

        if (delta_update!=0)
        {
            setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+delta_update);
        }

        if (delta_event!=0)
        {
            setNumChildrenRequiringEventTraversal(getNumChildrenRequiringEventTraversal()+delta_event);
        }

    }
}

void StateSet::removeUniform(const std::string& name)
{
    UniformList::iterator itr = _uniformList.find(name);
    if (itr!=_uniformList.end())
    {
        if (itr->second.first->getUpdateCallback())
        {
            setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()-1);
        }

        if (itr->second.first->getEventCallback())
        {
            setNumChildrenRequiringEventTraversal(getNumChildrenRequiringEventTraversal()-1);
        }

        itr->second.first->removeParent(this);

        _uniformList.erase(itr);
    }
}

void StateSet::removeUniform(Uniform* uniform)
{
    if (!uniform) return;

    UniformList::iterator itr = _uniformList.find(uniform->getName());
    if (itr!=_uniformList.end())
    {
        if (itr->second.first != uniform) return;

        if (itr->second.first->getUpdateCallback())
        {
            setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()-1);
        }

        if (itr->second.first->getEventCallback())
        {
            setNumChildrenRequiringEventTraversal(getNumChildrenRequiringEventTraversal()-1);
        }

        itr->second.first->removeParent(this);
        _uniformList.erase(itr);
    }
}

Uniform* StateSet::getUniform(const std::string& name)
{
    UniformList::iterator itr = _uniformList.find(name);
    if (itr!=_uniformList.end()) return itr->second.first.get();
    else return 0;
}

Uniform* StateSet::getOrCreateUniform(const std::string& name, Uniform::Type type, unsigned int numElements)
{
    // for look for an appropriate uniform.
    UniformList::iterator itr = _uniformList.find(name);
    if (itr!=_uniformList.end() &&
        itr->second.first->getType()==type)
    {
        return itr->second.first.get();
    }

    // no uniform found matching name so create it..

    Uniform* uniform = new Uniform(type,name,numElements);
    addUniform(uniform);

    return uniform;
}


const Uniform* StateSet::getUniform(const std::string& name) const
{
    UniformList::const_iterator itr = _uniformList.find(name);
    if (itr!=_uniformList.end()) return itr->second.first.get();
    else return 0;
}

const StateSet::RefUniformPair* StateSet::getUniformPair(const std::string& name) const
{
    UniformList::const_iterator itr = _uniformList.find(name);
    if (itr!=_uniformList.end()) return &(itr->second);
    else return 0;
}

void StateSet::setDefine(const std::string& defineName, StateAttribute::OverrideValue value)
{
    DefinePair& dp = _defineList[defineName];
    dp.first = "";
    dp.second = value;
}

void StateSet::setDefine(const std::string& defineName, const std::string& defineValue, StateAttribute::OverrideValue value)
{
    DefinePair& dp = _defineList[defineName];
    dp.first = defineValue;
    dp.second = value;
}

void StateSet::removeDefine(const std::string& defineName)
{
    DefineList::iterator itr = _defineList.find(defineName);
    if (itr != _defineList.end()) _defineList.erase(itr);
}


void StateSet::setTextureMode(unsigned int unit,StateAttribute::GLMode mode, StateAttribute::GLModeValue value)
{
    if (getTextureGLModeSet().isTextureMode(mode))
    {
        setMode(getOrCreateTextureModeList(unit),mode,value);
    }
    else
    {
        OSG_NOTICE<<"Warning: non-texture mode '"<<mode<<"'passed to setTextureMode(unit,mode,value), "<<std::endl;
        OSG_NOTICE<<"         assuming setMode(mode,value) instead."<<std::endl;
        OSG_NOTICE<<"         please change calling code to use appropriate call."<<std::endl;

        setMode(mode,value);
    }
}

void StateSet::removeTextureMode(unsigned int unit,StateAttribute::GLMode mode)
{
    if (getTextureGLModeSet().isTextureMode(mode))
    {
        if (unit>=_textureModeList.size()) return;
        setModeToInherit(_textureModeList[unit],mode);
    }
    else
    {
        OSG_NOTICE<<"Warning: non-texture mode '"<<mode<<"'passed to setTextureModeToInherit(unit,mode), "<<std::endl;
        OSG_NOTICE<<"         assuming setModeToInherit(unit=0,mode) instead."<<std::endl;
        OSG_NOTICE<<"         please change calling code to use appropriate call."<<std::endl;

        removeMode(mode);
    }
}


StateAttribute::GLModeValue StateSet::getTextureMode(unsigned int unit,StateAttribute::GLMode mode) const
{
    if (getTextureGLModeSet().isTextureMode(mode))
    {
        if (unit>=_textureModeList.size()) return StateAttribute::INHERIT;
        return getMode(_textureModeList[unit],mode);
    }
    else
    {
        OSG_NOTICE<<"Warning: non-texture mode '"<<mode<<"'passed to geTexturetMode(unit,mode), "<<std::endl;
        OSG_NOTICE<<"         assuming getMode(mode) instead."<<std::endl;
        OSG_NOTICE<<"         please change calling code to use appropriate call."<<std::endl;

        return getMode(mode);
    }
}

void StateSet::setTextureAttribute(unsigned int unit,StateAttribute *attribute, StateAttribute::OverrideValue value)
{
    if (attribute)
    {
        if (attribute->isTextureAttribute())
        {
            setAttribute(getOrCreateTextureAttributeList(unit),attribute,value);
        }
        else
        {
            OSG_NOTICE<<"Warning: texture attribute '"<<attribute->className()<<"' passed to setTextureAttribute(unit,attr,value), "<<std::endl;
            OSG_NOTICE<<"         assuming setAttribute(attr,value) instead."<<std::endl;
            OSG_NOTICE<<"         please change calling code to use appropriate call."<<std::endl;
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
                removeTextureAttribute(unit,attribute->getType());
            }
            else
            {
                setAttribute(getOrCreateTextureAttributeList(unit),attribute,value);
                setAssociatedTextureModes(unit,attribute,value);
            }
        }
        else
        {
            OSG_NOTICE<<"Warning: non texture attribute '"<<attribute->className()<<"' passed to setTextureAttributeAndModes(unit,attr,value), "<<std::endl;
            OSG_NOTICE<<"         assuming setAttributeAndModes(attr,value) instead."<<std::endl;
            OSG_NOTICE<<"         please change calling code to use appropriate call."<<std::endl;
            setAttributeAndModes(attribute,value);
        }
    }
}


void StateSet::removeTextureAttribute(unsigned int unit, StateAttribute::Type type)
{
    if (unit>=_textureAttributeList.size()) return;
    AttributeList& attributeList = _textureAttributeList[unit];
    AttributeList::iterator itr = attributeList.find(StateAttribute::TypeMemberPair(type,0));
    if (itr!=attributeList.end())
    {
        if (unit<_textureModeList.size())
        {
            setAssociatedTextureModes(unit,itr->second.first.get(),StateAttribute::INHERIT);
        }

        if (itr->second.first->getUpdateCallback())
        {
            setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()-1);
        }

        if (itr->second.first->getEventCallback())
        {
            setNumChildrenRequiringEventTraversal(getNumChildrenRequiringEventTraversal()-1);
        }


        itr->second.first->removeParent(this);
        attributeList.erase(itr);
    }
}

void StateSet::removeTextureAttribute(unsigned int unit, StateAttribute* attribute)
{
    if (!attribute) return;
    if (unit>=_textureAttributeList.size()) return;

    AttributeList& attributeList = _textureAttributeList[unit];
    AttributeList::iterator itr = attributeList.find(attribute->getTypeMemberPair());
    if (itr!=attributeList.end())
    {
        if (itr->second.first != attribute) return;

        setAssociatedTextureModes(unit,itr->second.first.get(),StateAttribute::INHERIT);

        if (itr->second.first->getUpdateCallback())
        {
            setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()-1);
        }

        if (itr->second.first->getEventCallback())
        {
            setNumChildrenRequiringEventTraversal(getNumChildrenRequiringEventTraversal()-1);
        }

        itr->second.first->removeParent(this);
        attributeList.erase(itr);
    }
}

StateAttribute* StateSet::getTextureAttribute(unsigned int unit,StateAttribute::Type type)
{
    if (unit>=_textureAttributeList.size()) return 0;
    return getAttribute(_textureAttributeList[unit],type,0);
}


const StateAttribute* StateSet::getTextureAttribute(unsigned int unit,StateAttribute::Type type) const
{
    if (unit>=_textureAttributeList.size()) return 0;
    return getAttribute(_textureAttributeList[unit],type,0);
}


StateSet::RefAttributePair* StateSet::getTextureAttributePair(unsigned int unit, StateAttribute::Type type)
{
    if (unit>=_textureAttributeList.size()) return 0;
    return getAttributePair(_textureAttributeList[unit],type,0);
}

const StateSet::RefAttributePair* StateSet::getTextureAttributePair(unsigned int unit, StateAttribute::Type type) const
{
    if (unit>=_textureAttributeList.size()) return 0;
    return getAttributePair(_textureAttributeList[unit],type,0);
}

bool StateSet::checkValidityOfAssociatedModes(osg::State& state) const
{


    bool modesValid = true;
    for(AttributeList::const_iterator itr = _attributeList.begin();
        itr!=_attributeList.end();
        ++itr)
    {
        if (!itr->second.first->checkValidityOfAssociatedModes(state)) modesValid = false;
    }

    for(TextureAttributeList::const_iterator taitr=_textureAttributeList.begin();
        taitr!=_textureAttributeList.end();
        ++taitr)
    {
        for(AttributeList::const_iterator itr = taitr->begin();
            itr!=taitr->end();
            ++itr)
        {
            if (!itr->second.first->checkValidityOfAssociatedModes(state)) modesValid = false;
        }
    }

    return modesValid;
}

void StateSet::setThreadSafeRefUnref(bool threadSafe)
{
    Object::setThreadSafeRefUnref(threadSafe);

    for(AttributeList::const_iterator itr = _attributeList.begin();
        itr!=_attributeList.end();
        ++itr)
    {
        itr->second.first->setThreadSafeRefUnref(threadSafe);
    }

    for(TextureAttributeList::const_iterator taitr=_textureAttributeList.begin();
        taitr!=_textureAttributeList.end();
        ++taitr)
    {
        for(AttributeList::const_iterator itr = taitr->begin();
            itr!=taitr->end();
            ++itr)
        {
            itr->second.first->setThreadSafeRefUnref(threadSafe);
        }
    }
}

void StateSet::compileGLObjects(State& state) const
{
    bool checkForGLErrors = state.getCheckForGLErrors()==osg::State::ONCE_PER_ATTRIBUTE;
    if (checkForGLErrors) state.checkGLErrors("before StateSet::compileGLObejcts()");

    for(AttributeList::const_iterator itr = _attributeList.begin();
        itr!=_attributeList.end();
        ++itr)
    {
        itr->second.first->compileGLObjects(state);
        if (checkForGLErrors) state.checkGLErrors("StateSet::compileGLObejcts() compiling ", itr->second.first->className());
    }

    for(TextureAttributeList::const_iterator taitr=_textureAttributeList.begin();
        taitr!=_textureAttributeList.end();
        ++taitr)
    {
        for(AttributeList::const_iterator itr = taitr->begin();
            itr!=taitr->end();
            ++itr)
        {
            itr->second.first->compileGLObjects(state);
            if (checkForGLErrors) state.checkGLErrors("StateSet::compileGLObejcts() compiling texture attribute", itr->second.first->className());
        }
    }
}

void StateSet::resizeGLObjectBuffers(unsigned int maxSize)
{
    for(AttributeList::const_iterator itr = _attributeList.begin();
        itr!=_attributeList.end();
        ++itr)
    {
        itr->second.first->resizeGLObjectBuffers(maxSize);
    }

    for(TextureAttributeList::const_iterator taitr=_textureAttributeList.begin();
        taitr!=_textureAttributeList.end();
        ++taitr)
    {
        for(AttributeList::const_iterator itr = taitr->begin();
            itr!=taitr->end();
            ++itr)
        {
            itr->second.first->resizeGLObjectBuffers(maxSize);
        }
    }
}

void StateSet::releaseGLObjects(State* state) const
{
    for(AttributeList::const_iterator itr = _attributeList.begin();
        itr!=_attributeList.end();
        ++itr)
    {
        itr->second.first->releaseGLObjects(state);
    }

    for(TextureAttributeList::const_iterator taitr=_textureAttributeList.begin();
        taitr!=_textureAttributeList.end();
        ++taitr)
    {
        for(AttributeList::const_iterator itr = taitr->begin();
            itr!=taitr->end();
            ++itr)
        {
            itr->second.first->releaseGLObjects(state);
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

void StateSet::setModeToInherit(ModeList& modeList, StateAttribute::GLMode mode)
{
    ModeList::iterator itr = modeList.find(mode);
    if (itr!=modeList.end())
    {
        modeList.erase(itr);
    }
}

StateAttribute::GLModeValue StateSet::getMode(const ModeList& modeList, StateAttribute::GLMode mode) const
{
    ModeList::const_iterator itr = modeList.find(mode);
    if (itr!=modeList.end())
    {
        return itr->second;
    }
    else
        return StateAttribute::INHERIT;
}

class SetAssociateModesHelper : public StateAttribute::ModeUsage
{
    public:
        SetAssociateModesHelper(StateSet* stateset, StateAttribute::GLModeValue value,unsigned int unit=0):
            _stateset(stateset),
            _value(value),
            _unit(unit) {}

        virtual ~SetAssociateModesHelper() {}

        virtual void usesMode(StateAttribute::GLMode mode)
        {
            _stateset->setMode(mode,_value);
        }

        virtual void usesTextureMode(StateAttribute::GLMode mode)
        {
            _stateset->setTextureMode(_unit,mode,_value);
        }



        StateSet*                   _stateset;
        StateAttribute::GLModeValue _value;
        unsigned int                _unit;
};

class RemoveAssociateModesHelper : public StateAttribute::ModeUsage
{
    public:
        RemoveAssociateModesHelper(StateSet* stateset, unsigned int unit=0):
            _stateset(stateset),
            _unit(unit) {}

        virtual ~RemoveAssociateModesHelper() {}

        virtual void usesMode(StateAttribute::GLMode mode)
        {
            _stateset->removeMode(mode);
        }

        virtual void usesTextureMode(StateAttribute::GLMode mode)
        {
           _stateset->removeTextureMode(_unit, mode);
        }



        StateSet*                   _stateset;
        unsigned int                _unit;
};

void StateSet::setAssociatedModes(const StateAttribute* attribute, StateAttribute::GLModeValue value)
{
    SetAssociateModesHelper helper(this,value);
    attribute->getModeUsage(helper);
}

void StateSet::removeAssociatedModes(const StateAttribute* attribute)
{
    RemoveAssociateModesHelper helper(this);
    attribute->getModeUsage(helper);
}

void StateSet::setAssociatedTextureModes(unsigned int unit, const StateAttribute* attribute, StateAttribute::GLModeValue value)
{
    SetAssociateModesHelper helper(this,value,unit);
    attribute->getModeUsage(helper);
}

void StateSet::removeAssociatedTextureModes(unsigned int unit, const StateAttribute* attribute)
{
    RemoveAssociateModesHelper helper(this,unit);
    attribute->getModeUsage(helper);
}

void StateSet::setAttribute(AttributeList& attributeList,StateAttribute *attribute, StateAttribute::OverrideValue value)
{
    if (attribute)
    {
        int delta_update = 0;
        int delta_event = 0;

        AttributeList::iterator itr=attributeList.find(attribute->getTypeMemberPair());
        if (itr==attributeList.end())
        {
            // new entry.
            attributeList[attribute->getTypeMemberPair()] = RefAttributePair(attribute,value&(StateAttribute::OVERRIDE|StateAttribute::PROTECTED));
            attribute->addParent(this);

            if (attribute->getUpdateCallback())
            {
                delta_update = 1;
            }

            if (attribute->getEventCallback())
            {
                delta_event = 1;
            }
        }
        else
        {
            if (itr->second.first==attribute)
            {
                // changing just override
                itr->second.second = value&(StateAttribute::OVERRIDE|StateAttribute::PROTECTED);
            }
            else
            {
                itr->second.first->removeParent(this);
                if (itr->second.first->getUpdateCallback()) --delta_update;
                if (itr->second.first->getEventCallback()) --delta_event;

                attribute->addParent(this);
                itr->second.first = attribute;
                if (itr->second.first->getUpdateCallback()) ++delta_update;
                if (itr->second.first->getEventCallback()) ++delta_event;

               itr->second.second = value&(StateAttribute::OVERRIDE|StateAttribute::PROTECTED);
            }
        }

        if (delta_update!=0)
        {
            setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+delta_update);
        }

        if (delta_event!=0)
        {
            setNumChildrenRequiringEventTraversal(getNumChildrenRequiringEventTraversal()+delta_event);
        }
    }
}


StateAttribute* StateSet::getAttribute(AttributeList& attributeList, StateAttribute::Type type, unsigned int member)
{
    AttributeList::iterator itr = attributeList.find(StateAttribute::TypeMemberPair(type,member));
    if (itr!=attributeList.end())
    {
        return itr->second.first.get();
    }
    else
        return NULL;
}

const StateAttribute* StateSet::getAttribute(const AttributeList& attributeList, StateAttribute::Type type, unsigned int member) const
{
    AttributeList::const_iterator itr = attributeList.find(StateAttribute::TypeMemberPair(type,member));
    if (itr!=attributeList.end())
    {
        return itr->second.first.get();
    }
    else
        return NULL;
}

StateSet::RefAttributePair* StateSet::getAttributePair(AttributeList& attributeList, StateAttribute::Type type, unsigned int member)
{
    AttributeList::iterator itr = attributeList.find(StateAttribute::TypeMemberPair(type,member));
    if (itr!=attributeList.end())
    {
        return &(itr->second);
    }
    else
        return NULL;
}

const StateSet::RefAttributePair* StateSet::getAttributePair(const AttributeList& attributeList, StateAttribute::Type type, unsigned int member) const
{
    AttributeList::const_iterator itr = attributeList.find(StateAttribute::TypeMemberPair(type,member));
    if (itr!=attributeList.end())
    {
        return &(itr->second);
    }
    else
        return NULL;
}




void StateSet::setUpdateCallback(Callback* ac)
{
    //OSG_INFO<<"Setting StateSet callbacks"<<std::endl;

    if (_updateCallback==ac) return;

    int delta = 0;
    if (_updateCallback.valid()) --delta;
    if (ac) ++delta;

    _updateCallback = ac;

    if (delta!=0 && _numChildrenRequiringUpdateTraversal==0)
    {
        //OSG_INFO<<"Going to set StateSet parents"<<std::endl;

        for(ParentList::iterator itr=_parents.begin();
            itr!=_parents.end();
            ++itr)
        {
            osg::Node* node = *itr;
            node->setNumChildrenRequiringUpdateTraversal(node->getNumChildrenRequiringUpdateTraversal()+delta);
        }
    }
}

void StateSet::runUpdateCallbacks(osg::NodeVisitor* nv)
{
    //OSG_INFO<<"Running StateSet callbacks"<<std::endl;

    if (_updateCallback.valid()) (*_updateCallback)(this,nv);

    if (_numChildrenRequiringUpdateTraversal!=0)
    {
        // run attribute callbacks
        for(AttributeList::iterator itr=_attributeList.begin();
            itr!=_attributeList.end();
            ++itr)
        {
            StateAttributeCallback* callback = itr->second.first->getUpdateCallback();
            if (callback) (*callback)(itr->second.first.get(),nv);
        }


        // run texture attribute callbacks.
        for(unsigned int i=0;i<_textureAttributeList.size();++i)
        {
            AttributeList& attributeList = _textureAttributeList[i];
            for(AttributeList::iterator itr=attributeList.begin();
                itr!=attributeList.end();
                ++itr)
            {
                StateAttributeCallback* callback = itr->second.first->getUpdateCallback();
                if (callback) (*callback)(itr->second.first.get(),nv);
            }
        }


        // run uniform callbacks.
        for(UniformList::iterator uitr = _uniformList.begin();
            uitr != _uniformList.end();
            ++uitr)
        {
            UniformCallback* callback = uitr->second.first->getUpdateCallback();
            if (callback) (*callback)(uitr->second.first.get(),nv);
        }
    }
}

void StateSet::setEventCallback(Callback* ac)
{
    if (_eventCallback==ac) return;

    int delta = 0;
    if (_eventCallback.valid()) --delta;
    if (ac) ++delta;

    _eventCallback = ac;

    if (delta!=0 && _numChildrenRequiringEventTraversal==0)
    {
        for(ParentList::iterator itr=_parents.begin();
            itr!=_parents.end();
            ++itr)
        {
            osg::Node* node = *itr;
            node->setNumChildrenRequiringEventTraversal(node->getNumChildrenRequiringEventTraversal()+delta);
        }
    }
}

void StateSet::runEventCallbacks(osg::NodeVisitor* nv)
{
    if (_eventCallback.valid()) (*_eventCallback)(this,nv);

    if (_numChildrenRequiringEventTraversal!=0)
    {
        // run attribute callbacks
        for(AttributeList::iterator itr=_attributeList.begin();
            itr!=_attributeList.end();
            ++itr)
        {
            StateAttributeCallback* callback = itr->second.first->getEventCallback();
            if (callback) (*callback)(itr->second.first.get(),nv);
        }


        // run texture attribute callbacks.
        for(unsigned int i=0;i<_textureAttributeList.size();++i)
        {
            AttributeList& attributeList = _textureAttributeList[i];
            for(AttributeList::iterator itr=attributeList.begin();
                itr!=attributeList.end();
                ++itr)
            {
                StateAttributeCallback* callback = itr->second.first->getEventCallback();
                if (callback) (*callback)(itr->second.first.get(),nv);
            }
        }


        // run uniform callbacks.
        for(UniformList::iterator uitr = _uniformList.begin();
            uitr != _uniformList.end();
            ++uitr)
        {
            UniformCallback* callback = uitr->second.first->getEventCallback();
            if (callback) (*callback)(uitr->second.first.get(),nv);
        }
    }
}

void StateSet::setNumChildrenRequiringUpdateTraversal(unsigned int num)
{
    // if no changes just return.
    if (_numChildrenRequiringUpdateTraversal==num) return;

    // note, if _updateCallback is set then the
    // parents won't be affected by any changes to
    // _numChildrenRequiringUpdateTraversal so no need to inform them.
    if (!_updateCallback && !_parents.empty())
    {

        // need to pass on changes to parents.
        int delta = 0;
        if (_numChildrenRequiringUpdateTraversal>0) --delta;
        if (num>0) ++delta;
        if (delta!=0)
        {
            // the number of callbacks has changed, need to pass this
            // on to parents so they know whether app traversal is
            // required on this subgraph.
            for(ParentList::iterator itr =_parents.begin();
                itr != _parents.end();
                ++itr)
            {
                osg::Node* node = *itr;
                node->setNumChildrenRequiringUpdateTraversal(node->getNumChildrenRequiringUpdateTraversal()+delta);
            }
        }
    }

    // finally update this objects value.
    _numChildrenRequiringUpdateTraversal=num;
}

void StateSet::setNumChildrenRequiringEventTraversal(unsigned int num)
{
    // if no changes just return.
    if (_numChildrenRequiringEventTraversal==num) return;

    // note, if _EventCallback is set then the
    // parents won't be affected by any changes to
    // _numChildrenRequiringEventTraversal so no need to inform them.
    if (!_eventCallback && !_parents.empty())
    {

        // need to pass on changes to parents.
        int delta = 0;
        if (_numChildrenRequiringEventTraversal>0) --delta;
        if (num>0) ++delta;
        if (delta!=0)
        {
            // the number of callbacks has changed, need to pass this
            // on to parents so they know whether app traversal is
            // required on this subgraph.
            for(ParentList::iterator itr =_parents.begin();
                itr != _parents.end();
                ++itr)
            {
                osg::Node* node = *itr;
                node->setNumChildrenRequiringEventTraversal(node->getNumChildrenRequiringEventTraversal()+delta);
            }
        }
    }

    // finally Event this objects value.
    _numChildrenRequiringEventTraversal=num;
}
