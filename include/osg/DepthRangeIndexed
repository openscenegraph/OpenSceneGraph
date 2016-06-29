/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2016 Robert Osfield
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

#ifndef OSG_DepthRangeIndexed
#define OSG_DepthRangeIndexed 1

#include <osg/Depth>

namespace osg {

/** Encapsulates glDepthRangeIndexed function : the index version of glDepth
*/
class OSG_EXPORT DepthRangeIndexed : public osg::StateAttribute
{
    public :

        DepthRangeIndexed();

        DepthRangeIndexed(unsigned int index, double zNear=0.0, double zFar=1.0):
            _index(index),
            _zNear(zNear),
            _zFar(zFar) {}

        /** Copy constructor using CopyOp to manage deep vs shallow copy. */
        DepthRangeIndexed(const DepthRangeIndexed& dp,const CopyOp& copyop=CopyOp::SHALLOW_COPY):
            StateAttribute(dp,copyop),
            _index(dp._index),
            _zNear(dp._zNear),
            _zFar(dp._zFar) {}

        META_StateAttribute(osg, DepthRangeIndexed, DEPTHRANGEINDEXED);

        /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
        virtual int compare(const StateAttribute& sa) const
        {
            // Check for equal types, then create the rhs variable
            // used by the COMPARE_StateAttribute_Parameter macros below.
            COMPARE_StateAttribute_Types(DepthRangeIndexed,sa)

            COMPARE_StateAttribute_Parameter(_index);
            COMPARE_StateAttribute_Parameter(_zNear)
            COMPARE_StateAttribute_Parameter(_zFar)

            return 0;
        }

        /** Return the buffer index as the member identifier.*/
        virtual unsigned int getMember() const { return _index; }

        /** Set the index of the DepthRangeIndexed. */
        void setIndex(unsigned int index);

        /** Get the index of the DepthRangeIndexed. */
        unsigned int getIndex() const { return _index; }

        inline void setZNear(double zNear) { _zNear=zNear; }
        inline double getZNear() const { return _zNear; }

        inline void setZFar(double zFar) { _zFar=zFar; }
        inline double getZFar() const { return _zFar; }

        virtual void apply(State& state) const;

    protected:

        virtual ~DepthRangeIndexed();

        unsigned int    _index;
        double          _zNear;
        double          _zFar;

};

}

#endif
