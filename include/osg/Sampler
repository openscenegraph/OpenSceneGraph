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
#ifndef OSG_SAMPLER_H
#define OSG_SAMPLER_H 1

#include <osg/Texture>

namespace osg{
/**  OpenGL Sampler
 *   OpenGL 3.3 required
 *   https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_sampler_objects.txt
 *   State Attribute controllig sampling instead of Texture
 *   Sampler is prioritary over Texture sample parameter (don't play with both)
*/


class OSG_EXPORT Sampler : public osg::StateAttribute
{
    public:
        Sampler();

        /** Copy constructor using CopyOp to manage deep vs shallow copy. */
        Sampler(const Sampler& text,const CopyOp& copyop=CopyOp::SHALLOW_COPY);

        META_StateAttribute(osg,Sampler,SAMPLER)

        virtual bool isTextureAttribute() const { return true; }

        /** Sets the texture wrap mode. */
        void setWrap(Texture::WrapParameter which, Texture::WrapMode wrap);

        /** Gets the texture wrap mode. */
        Texture::WrapMode getWrap(Texture::WrapParameter which) const;

        /** Sets the texture filter mode. */
        void setFilter(Texture::FilterParameter which, Texture::FilterMode filter);

        /** Gets the texture filter mode. */
        Texture::FilterMode getFilter(Texture::FilterParameter which) const;

        /** Sets shadow texture comparison function. */
        void setShadowCompareFunc(Texture::ShadowCompareFunc func);
        Texture::ShadowCompareFunc getShadowCompareFunc() const { return _shadow_compare_func; }

        /** Sets shadow texture mode after comparison. */
        void setShadowTextureMode(Texture::ShadowTextureMode mode);
        Texture::ShadowTextureMode getShadowTextureMode() const { return _shadow_texture_mode; }

        /** Sets the border color. Only used when wrap mode is CLAMP_TO_BORDER.
         * The border color will be casted to the appropriate type to match the
         * internal pixel format of the texture. */
        void setBorderColor(const Vec4d& color);

        /** Gets the border color. */
        const Vec4d& getBorderColor() const { return _borderColor; }

        /** Sets the maximum anisotropy value, default value is 1.0 for no
          * anisotropic filtering. If hardware does not support anisotropic
          * filtering, use normal filtering (equivalent to a max anisotropy
          * value of 1.0. Valid range is 1.0f upwards.  The maximum value
          * depends on the graphics system. */
        void setMaxAnisotropy(float anis);

        /** Gets the maximum anisotropy value. */
        inline float getMaxAnisotropy() const { return _maxAnisotropy; }

        void setMinLOD(float anis);

        /** Gets the maximum anisotropy value. */
        inline float getMinLOD() const { return _minlod; }

        void setMaxLOD(float anis);

        /** Gets the maximum anisotropy value. */
        inline float getMaxLOD() const { return _maxlod; }

        void setLODBias(float anis);

        /** Gets the maximum anisotropy value. */
        inline float getLODBias() const { return _lodbias; }

        /** helper method to generate Sampler from Texture's sampling parameters (except shadow_texture_mode left to NONE) */
        static void generateSamplerObjects(StateSet&);

        virtual void apply(State& state) const;

        virtual void compileGLObjects(State&) const;

        /** release state's SamplerObject **/
        virtual void releaseGLObjects(State* state=0) const;

        virtual int compare(const StateAttribute& sa) const;

    protected:
        Texture::WrapMode _wrap_s;
        Texture::WrapMode _wrap_t;
        Texture::WrapMode _wrap_r;
        Texture::ShadowCompareFunc _shadow_compare_func;
        Texture::ShadowTextureMode _shadow_texture_mode;
        Vec4d _borderColor;

        Texture::FilterMode _min_filter;
        Texture::FilterMode _mag_filter;
        float _maxAnisotropy, _minlod, _maxlod, _lodbias;

        mutable buffered_value<GLuint> _PCsampler;
        mutable buffered_value<uint8_t> _PCdirtyflags;
};
}
#endif
