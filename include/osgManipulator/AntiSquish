/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
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
//osgDragger - Copyright (C) 2007 Fugro-Jason B.V.

#ifndef _OSG_ANTISQUISH_
#define _OSG_ANTISQUISH_ 1

#include <osgManipulator/Export>

#include <osg/Transform>
#include <OpenThreads/Mutex>

namespace osgManipulator {

/**
 * Class that performs the Anti Squish by making the scaling uniform along all axes.
 */
class OSGMANIPULATOR_EXPORT AntiSquish: public osg::Transform
{
    public :
        AntiSquish();
        AntiSquish(const osg::Vec3d& pivot);
        AntiSquish(const osg::Vec3d& pivot, const osg::Vec3d& position);
        AntiSquish(const AntiSquish& pat, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        virtual osg::Object* cloneType() const { return new AntiSquish(); }

        virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new AntiSquish (*this,copyop); }

        virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const AntiSquish *>(obj)!=NULL; }

        virtual const char* libraryName() const { return "osgManipulator"; }

        virtual const char* className() const { return "AntiSquish"; }


        void setPivot(const osg::Vec3d& pvt)
        {
            _pivot = pvt;
            _usePivot = true;
            _cacheDirty = true;
        }

        const osg::Vec3d& getPivot() const { return _pivot; }

        void setPosition(const osg::Vec3d& pos)
        {
            _position = pos;
            _usePosition = true;
            _cacheDirty = true;
        }

        const osg::Vec3d& getPosition() const { return _position; }

        bool computeLocalToWorldMatrix(osg::Matrix& matrix,osg::NodeVisitor*) const;
        bool computeWorldToLocalMatrix(osg::Matrix& matrix,osg::NodeVisitor*) const;

    protected:

        virtual ~AntiSquish();

        bool computeUnSquishedMatrix(osg::Matrix&) const;

        osg::Vec3d _pivot;
        bool _usePivot;

        osg::Vec3d _position;
        bool _usePosition;

        mutable OpenThreads::Mutex _cacheLock;
        mutable bool _cacheDirty;
        mutable osg::Matrix _cacheLocalToWorld;
        mutable osg::Matrix _cache;
};

}
#endif
