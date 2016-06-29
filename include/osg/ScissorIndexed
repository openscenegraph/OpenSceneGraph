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

#ifndef OSG_ScissorIndexed
#define OSG_ScissorIndexed 1

#include <osg/Depth>

namespace osg {

/** Encapsulates glScissorIndexed function : the index version of glDepth
*/
class OSG_EXPORT ScissorIndexed : public osg::StateAttribute
{
    public :

        ScissorIndexed();

        ScissorIndexed(unsigned int index, float x, float y, float width, float height):
            _index(index),
            _x(x),
            _y(y),
            _width(width),
            _height(height) {}

        /** Copy constructor using CopyOp to manage deep vs shallow copy. */
        ScissorIndexed(const ScissorIndexed& dp,const CopyOp& copyop=CopyOp::SHALLOW_COPY):
            StateAttribute(dp,copyop),
            _index(dp._index),
            _x(dp._x),
            _y(dp._y),
            _width(dp._width),
            _height(dp._height) {}

        META_StateAttribute(osg, ScissorIndexed, SCISSORINDEXED);

        /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
        virtual int compare(const StateAttribute& sa) const
        {
            // Check for equal types, then create the rhs variable
            // used by the COMPARE_StateAttribute_Parameter macros below.
            COMPARE_StateAttribute_Types(ScissorIndexed,sa)

            COMPARE_StateAttribute_Parameter(_index);
            COMPARE_StateAttribute_Parameter(_x)
            COMPARE_StateAttribute_Parameter(_y)
            COMPARE_StateAttribute_Parameter(_width)
            COMPARE_StateAttribute_Parameter(_height)

            return 0;
        }

        /** Return the buffer index as the member identifier.*/
        virtual unsigned int getMember() const { return _index; }

        /** Set the index of the ScissorIndexed. */
        void setIndex(unsigned int index);

        /** Get the index of the ScissorIndexed. */
        unsigned int getIndex() const { return _index; }

        inline void setX(float x) { _x=x; }
        inline float getX() const { return _x; }

        inline void setY(float y) { _y=y; }
        inline float getY() const { return _y; }

        inline void setWidth(float w) { _width=w; }
        inline float getWidth() const { return _width; }

        inline void setHeight(float height) { _height=height; }
        inline float getHeight() const { return _height; }

        virtual void apply(State& state) const;

    protected:

        virtual ~ScissorIndexed();

        unsigned int    _index;
        float           _x;
        float           _y;
        float           _width;
        float           _height;

};

}

#endif
