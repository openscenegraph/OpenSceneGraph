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

#ifndef OSG_POINT
#define OSG_POINT 1

#include <osg/Vec3>
#include <osg/StateAttribute>

#ifndef GL_POINT_SMOOTH
    #define GL_POINT_SMOOTH     0x0B10
#endif

#ifndef GL_POINT_SMOOTH_HINT
    #define GL_POINT_SMOOTH_HINT 0x0C51
#endif

namespace osg {

/** Point - encapsulates the OpenGL point smoothing and size state.*/
class OSG_EXPORT Point : public StateAttribute
{
    public :

        Point();

        Point(float size);

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        Point(const Point& point,const CopyOp& copyop=CopyOp::SHALLOW_COPY):
            StateAttribute(point,copyop),
            _size(point._size),
            _fadeThresholdSize(point._fadeThresholdSize),
            _distanceAttenuation(point._distanceAttenuation),
            _minSize(point._minSize),
            _maxSize(point._maxSize) {}

        META_StateAttribute(osg, Point, POINT);

        /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs.*/
        virtual int compare(const StateAttribute& sa) const
        {
            // check the types are equal and then create the rhs variable
            // used by the COMPARE_StateAttribute_Parameter macros below.
            COMPARE_StateAttribute_Types(Point,sa)

            // compare each parameter in turn against the rhs.
            COMPARE_StateAttribute_Parameter(_size)
            COMPARE_StateAttribute_Parameter(_fadeThresholdSize)
            COMPARE_StateAttribute_Parameter(_distanceAttenuation)
            COMPARE_StateAttribute_Parameter(_minSize)
            COMPARE_StateAttribute_Parameter(_maxSize)

            return 0; // passed all the above comparison macros, must be equal.
        }

        virtual bool getModeUsage(StateAttribute::ModeUsage& usage) const
        {
            usage.usesMode(GL_POINT_SMOOTH);
            return true;
        }

        void setSize(float size);
        inline float getSize() const { return _size; }

        void setFadeThresholdSize(float fadeThresholdSize);
        inline float getFadeThresholdSize() const { return _fadeThresholdSize; }

        void setDistanceAttenuation(const Vec3& distanceAttenuation);
        inline const Vec3& getDistanceAttenuation() const { return _distanceAttenuation; }

        void setMinSize(float minSize);
        inline float getMinSize() const {return _minSize;}

        void setMaxSize(float maxSize);
        inline float getMaxSize() const {return _maxSize;}

        virtual void apply(State& state) const;

    protected :

        virtual ~Point();

        float       _size;
        float       _fadeThresholdSize;
        Vec3        _distanceAttenuation;
        float        _minSize;
        float        _maxSize;

};

}

#endif
