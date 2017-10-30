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

#ifndef OSGANIMATION_MORPH_TRANSFORM_HARDWARE
#define OSGANIMATION_MORPH_TRANSFORM_HARDWARE 1

#include <osgAnimation/Export>
#include <osgAnimation/RigTransform>
#include <osgAnimation/VertexInfluence>
#include <osgAnimation/Bone>
#include <osg/Matrix>
#include <osg/Array>

///texture unit reserved for morphtarget TBO
#define MORPHTRANSHW_DEFAULTMORPHTEXTUREUNIT 7

namespace osgAnimation
{
    class MorphGeometry;

    /// This class manage format for hardware morphing
    class OSGANIMATION_EXPORT MorphTransformHardware : public MorphTransform
    {
    public:

        MorphTransformHardware();

        MorphTransformHardware(const MorphTransformHardware& rth, const osg::CopyOp& copyop);

        META_Object(osgAnimation,MorphTransformHardware);

        virtual void operator()(MorphGeometry&);

        inline void setShader( osg::Shader*s ) { _shader=s; }
        inline const osg::Shader * getShader() const { return _shader.get(); }
        inline osg::Shader * getShader() { return _shader.get(); }

        ///texture unit reserved for morphtarget TBO default is 7
        void setReservedTextureUnit(unsigned int t) { _reservedTextureUnit=t; }
        unsigned int getReservedTextureUnit() const { return _reservedTextureUnit; }

    protected:

        bool init(MorphGeometry&);

        osg::ref_ptr<osg::Uniform> _uniformTargetsWeight;
        osg::ref_ptr<osg::Shader> _shader;

        bool _needInit;
        unsigned int _reservedTextureUnit;
    };
}

#endif
