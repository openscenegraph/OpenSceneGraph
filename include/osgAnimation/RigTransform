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

#ifndef OSGANIMATION_RIGTRANSFORM
#define OSGANIMATION_RIGTRANSFORM 1

#include <osg/Object>

namespace osgAnimation
{

    class RigGeometry;

    class RigTransform : public osg::Object
    {
    public:
        RigTransform() {}
        RigTransform(const RigTransform& org, const osg::CopyOp& copyop):
            osg::Object(org, copyop) {}

        META_Object(osgAnimation,RigTransform)

        virtual void operator()(RigGeometry&) {}

        /// to call manually when a skeleton is reacheable from the rig
        /// in order to prepare technic data before rendering
        virtual bool prepareData(RigGeometry&) { return true; }

    protected:
        virtual ~RigTransform() {}

    };
    class MorphGeometry;

    class MorphTransform : public osg::Object
    {
    public:
        MorphTransform() {}
        MorphTransform(const MorphTransform& org, const osg::CopyOp& copyop):
            osg::Object(org, copyop) {}

        META_Object(osgAnimation,MorphTransform)

        virtual void operator()(MorphGeometry&) {}

    protected:
        virtual ~MorphTransform() {}

    };

}

#endif
