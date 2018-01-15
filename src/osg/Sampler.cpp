/*  -*-c++-*-
 *  Copyright (C) 2017 Julien Valentin <mp3butcher@hotmail.com>
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

#include <osg/Sampler>
#include <osg/Texture3D>


#ifndef GL_TEXTURE_WRAP_R
#define GL_TEXTURE_WRAP_R 0x2804
#endif

#ifndef GL_TEXTURE_COMPARE_MODE
#define GL_TEXTURE_COMPARE_MODE 0x884C
#endif

#ifndef GL_TEXTURE_COMPARE_FUNC
#define GL_TEXTURE_COMPARE_FUNC 0x884D
#endif


using namespace osg;

Sampler::Sampler(): StateAttribute(),
    _wrap_s(Texture::CLAMP),
    _wrap_t(Texture::CLAMP),
    _wrap_r(Texture::CLAMP),
    _shadow_compare_func(Texture::LEQUAL),
    _shadow_texture_mode(Texture::NONE),
    _min_filter(Texture::LINEAR_MIPMAP_LINEAR), // trilinear
    _mag_filter(Texture::LINEAR),
    _maxAnisotropy(1.0f),
    _minlod(0.0f),
    _maxlod(-1.0f),
    _lodbias(0.0f)
{
    _PCdirtyflags.setAllElementsTo(true);
    _PCsampler.setAllElementsTo(0);
}

Sampler::Sampler(const Sampler& sampler,const CopyOp &copyop ):StateAttribute(sampler,copyop),
    _wrap_s(sampler._wrap_s),
    _wrap_t(sampler._wrap_t),
    _wrap_r(sampler._wrap_r),
    _shadow_compare_func(sampler._shadow_compare_func),
    _shadow_texture_mode(sampler._shadow_texture_mode),
    _min_filter(sampler._min_filter),
    _mag_filter(sampler._mag_filter),
    _maxAnisotropy(sampler._maxAnisotropy),
    _minlod(sampler._minlod),
    _maxlod(sampler._maxlod),
    _lodbias(sampler._lodbias)
{
    _PCdirtyflags.setAllElementsTo(true);
    _PCsampler.setAllElementsTo(0);
}

void Sampler::setWrap(Texture::WrapParameter which, Texture::WrapMode wrap)
{
    switch( which )
    {
        case Texture::WRAP_S : _wrap_s = wrap;  _PCdirtyflags.setAllElementsTo(true); break;
        case Texture::WRAP_T : _wrap_t = wrap;  _PCdirtyflags.setAllElementsTo(true); break;
        case Texture::WRAP_R : _wrap_r = wrap;  _PCdirtyflags.setAllElementsTo(true); break;
        default : OSG_WARN<<"Error: invalid 'which' passed Sampler::setWrap("<<(unsigned int)which<<","<<(unsigned int)wrap<<")"<<std::endl; break;
    }
}


Texture::WrapMode Sampler::getWrap(Texture::WrapParameter which) const
{
    switch( which )
    {
        case Texture::WRAP_S : return _wrap_s;
        case Texture::WRAP_T : return _wrap_t;
        case Texture::WRAP_R : return _wrap_r;
        default : OSG_WARN<<"Error: invalid 'which' passed Sampler::getWrap(which)"<<std::endl; return _wrap_s;
    }
}


void Sampler::setFilter(Texture::FilterParameter which, Texture::FilterMode filter)
{
    switch( which )
    {
        case Texture::MIN_FILTER : _min_filter = filter; _PCdirtyflags.setAllElementsTo(true); break;
        case Texture::MAG_FILTER : _mag_filter = filter; _PCdirtyflags.setAllElementsTo(true); break;
        default : OSG_WARN<<"Error: invalid 'which' passed Sampler::setFilter("<<(unsigned int)which<<","<<(unsigned int)filter<<")"<<std::endl; break;
    }
}


Texture::FilterMode Sampler::getFilter(Texture::FilterParameter which) const
{
    switch( which )
    {
        case Texture::MIN_FILTER : return _min_filter;
        case Texture::MAG_FILTER : return _mag_filter;
        default : OSG_WARN<<"Error: invalid 'which' passed Sampler::getFilter(which)"<< std::endl; return _min_filter;
    }
}

void Sampler::setMaxAnisotropy(float anis)
{
    if (_maxAnisotropy!=anis)
    {
        _maxAnisotropy = anis;
        _PCdirtyflags.setAllElementsTo(true);
    }
}

void Sampler::setMinLOD(float func) { _minlod = func; _PCdirtyflags.setAllElementsTo(true); }
void Sampler::setMaxLOD(float func) { _maxlod = func; _PCdirtyflags.setAllElementsTo(true); }
void Sampler::setLODBias(float func) { _lodbias = func; _PCdirtyflags.setAllElementsTo(true); }

/** getOrCreate Sampler Object and setup embedded Texture Parameters */
void Sampler::setShadowCompareFunc(Texture::ShadowCompareFunc func) { _shadow_compare_func = func; _PCdirtyflags.setAllElementsTo(true);}

/** Sets shadow texture mode after comparison. */
void Sampler::setShadowTextureMode(Texture::ShadowTextureMode mode) { _shadow_texture_mode = mode; _PCdirtyflags.setAllElementsTo(true);}

void Sampler::setBorderColor(const Vec4d& color) { _borderColor = color; _PCdirtyflags.setAllElementsTo(true); }

void Sampler::compileGLObjects(State& state) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if (extensions->glGenSamplers==0) return;

    unsigned int contextID = state.getContextID();
    if(_PCdirtyflags[contextID])
    {
        GLuint samplerobject = _PCsampler[contextID];
        if(samplerobject==0)
        {
            extensions->glGenSamplers(1,&_PCsampler[contextID]);
            samplerobject = _PCsampler[contextID];
        }

        Texture::WrapMode ws = _wrap_s, wt = _wrap_t, wr = _wrap_r;

        // GL_IBM_texture_mirrored_repeat, fall-back REPEAT
        if (!extensions->isTextureMirroredRepeatSupported)
        {
            if (ws ==  Texture::MIRROR)
                ws =  Texture::REPEAT;
            if (wt ==  Texture::MIRROR)
                wt =  Texture::REPEAT;
            if (wr ==  Texture::MIRROR)
                wr =  Texture::REPEAT;
        }

        // GL_EXT_texture_edge_clamp, fall-back CLAMP
        if (!extensions->isTextureEdgeClampSupported)
        {
            if (ws ==  Texture::CLAMP_TO_EDGE)
                ws =  Texture::CLAMP;
            if (wt ==  Texture::CLAMP_TO_EDGE)
                wt =  Texture::CLAMP;
            if (wr ==  Texture::CLAMP_TO_EDGE)
                wr =  Texture::CLAMP;
        }

        if(!extensions->isTextureBorderClampSupported)
        {
            if(ws == Texture::CLAMP_TO_BORDER)
                ws = Texture::CLAMP;
            if(wt == Texture::CLAMP_TO_BORDER)
                wt = Texture::CLAMP;
            if(wr == Texture::CLAMP_TO_BORDER)
                wr = Texture::CLAMP;
        }

        #if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE) || defined(OSG_GL3_AVAILABLE)
            if (ws == Texture::CLAMP) ws = Texture::CLAMP_TO_EDGE;
            if (wt == Texture::CLAMP) wt = Texture::CLAMP_TO_EDGE;
            if (wr == Texture::CLAMP) wr = Texture::CLAMP_TO_EDGE;
        #endif

        extensions->glSamplerParameteri( samplerobject, GL_TEXTURE_WRAP_S, ws );
        extensions->glSamplerParameteri( samplerobject, GL_TEXTURE_WRAP_T, wt );
        extensions->glSamplerParameteri( samplerobject, GL_TEXTURE_WRAP_R, wr );


        extensions->glSamplerParameteri( samplerobject, GL_TEXTURE_MIN_FILTER, _min_filter);
        extensions->glSamplerParameteri( samplerobject, GL_TEXTURE_MAG_FILTER, _mag_filter);

        if (extensions->isTextureBorderClampSupported)
        {
            #ifndef GL_TEXTURE_BORDER_COLOR
                #define GL_TEXTURE_BORDER_COLOR     0x1004
            #endif

            GLfloat color[4] = {(GLfloat)_borderColor.r(), (GLfloat)_borderColor.g(), (GLfloat)_borderColor.b(), (GLfloat)_borderColor.a()};
            extensions->glSamplerParameterfv(samplerobject, GL_TEXTURE_BORDER_COLOR, color);
        }

        extensions->glSamplerParameteri(samplerobject, GL_TEXTURE_COMPARE_MODE, _shadow_texture_mode);
        extensions->glSamplerParameteri(samplerobject, GL_TEXTURE_COMPARE_FUNC, _shadow_compare_func);

        if (extensions->isTextureFilterAnisotropicSupported)
        {
            // note, GL_TEXTURE_MAX_ANISOTROPY_EXT will either be defined
            // by gl.h (or via glext.h) or by include/osg/Texture.
            extensions->glSamplerParameterf(samplerobject, GL_TEXTURE_MAX_ANISOTROPY_EXT, _maxAnisotropy);
        }

        if(_maxlod - _minlod >= 0)
        {   // if range is valid
            extensions->glSamplerParameterf(samplerobject, GL_TEXTURE_MIN_LOD, _minlod);
            extensions->glSamplerParameterf(samplerobject, GL_TEXTURE_MAX_LOD, _maxlod);
        }

        extensions->glSamplerParameterf(samplerobject, GL_TEXTURE_LOD_BIAS, _lodbias);

        _PCdirtyflags[contextID]=false;
    }
}

/** bind SamplerObject **/
void Sampler::apply(State&state) const
{
    if (state.get<GLExtensions>()->glGenSamplers==0) return;

    unsigned int contextID = state.getContextID();
    if(  _PCdirtyflags[contextID] )
        compileGLObjects(state);

    state.get<GLExtensions>()->glBindSampler( state.getActiveTextureUnit(), _PCsampler[contextID] );
}

void Sampler::releaseGLObjects(State* state) const
{
    if(state)
    {
        if (state->get<GLExtensions>()->glDeleteSamplers==0) return;

        unsigned int contextID=state->getContextID();
        state->get<GLExtensions>()->glDeleteSamplers(1,&_PCsampler[contextID]);
    }
}

int Sampler::compare(const StateAttribute& sa) const
{
    COMPARE_StateAttribute_Types(Sampler,sa)
    COMPARE_StateAttribute_Parameter(_wrap_t)
    COMPARE_StateAttribute_Parameter(_wrap_r)
    COMPARE_StateAttribute_Parameter(_min_filter)
    COMPARE_StateAttribute_Parameter(_mag_filter)
    COMPARE_StateAttribute_Parameter(_shadow_compare_func)
    COMPARE_StateAttribute_Parameter(_shadow_texture_mode)
    COMPARE_StateAttribute_Parameter(_maxAnisotropy)
    COMPARE_StateAttribute_Parameter(_minlod)
    COMPARE_StateAttribute_Parameter(_maxlod)
    COMPARE_StateAttribute_Parameter(_lodbias)
    return 0; // passed all the above comparison macros, must be equal.
}

void Sampler::generateSamplerObjects(StateSet& ss)
{
    const osg::StateSet::TextureAttributeList& texAttributes = ss.getTextureAttributeList();
    for(unsigned int unit=0; unit<texAttributes.size(); ++unit)
    {
        StateSet::RefAttributePair attmode;
        Sampler * sampler = 0;
        const StateSet::AttributeList& tex_attributes = texAttributes[unit];
        for(StateSet::AttributeList::const_iterator aitr = tex_attributes.begin();
            aitr!=tex_attributes.end();
            ++aitr)
        {
            if( aitr->second.first.get()->getType() != StateAttribute::TEXTURE )
            {
                if( aitr->second.first.get()->getType() == StateAttribute::SAMPLER)
                    sampler = static_cast< Sampler* > (aitr->second.first.get());
            }
            else attmode= aitr->second;
        }

        if( attmode.first.valid() )
        {
            if( !sampler )
            {
                ///create new Sampler and add it to this
                sampler = new Sampler();
                Texture * tex = attmode.first->asTexture();
                sampler->setFilter( Texture::MIN_FILTER, tex->getFilter(Texture::MIN_FILTER) );
                sampler->setFilter( Texture::MAG_FILTER, tex->getFilter(Texture::MAG_FILTER) );
                sampler->setWrap( Texture::WRAP_S, tex->getWrap(Texture::WRAP_S) );
                sampler->setWrap( Texture::WRAP_T, tex->getWrap(Texture::WRAP_T) );
                sampler->setWrap( Texture::WRAP_R, tex->getWrap(Texture::WRAP_R) );
                sampler->setMaxAnisotropy( tex->getMaxAnisotropy() );
                sampler->setShadowCompareFunc( tex->getShadowCompareFunc() );
                //sampler->setShadowTextureMode( tex->getShadowTextureMode() ); default LUMINANCE tex param incompatible with default NONE Sampler param
                sampler->setBorderColor( tex->getBorderColor() );
                sampler->setLODBias( tex->getLODBias() );
                sampler->setMinLOD( tex->getMinLOD() );
                sampler->setMaxLOD( tex->getMaxLOD() );
                ss.setTextureAttributeAndModes(unit,sampler,attmode.second);
            }
        }
    }
}
