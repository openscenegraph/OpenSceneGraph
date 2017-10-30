/*  -*-c++-*-
 *  Copyright (C) 2009 Cedric Pinson <cedric.pinson@plopbyte.net>
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

#ifndef OSGANIMATION_MORPHTRANSFORM_SOFTWARE
#define OSGANIMATION_MORPHTRANSFORM_SOFTWARE 1

#include <osgAnimation/Export>
#include <osgAnimation/RigTransform>
#include <osgAnimation/Bone>
#include <osg/observer_ptr>

namespace osgAnimation
{

    class MorphGeometry;

    /// This class manage format for software morphing
    class OSGANIMATION_EXPORT MorphTransformSoftware : public MorphTransform
    {
    public:
        MorphTransformSoftware():_needInit(true) {}
        MorphTransformSoftware(const MorphTransformSoftware& rts,const osg::CopyOp& copyop): MorphTransform(rts, copyop), _needInit(true) {}

        META_Object(osgAnimation,MorphTransformSoftware)

        bool init(MorphGeometry&);
        virtual void operator()(MorphGeometry&);

    protected:
        bool _needInit;

    };
}

#endif
