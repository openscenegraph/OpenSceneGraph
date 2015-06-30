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

#ifndef OSG_CLIPCONTROL
#define OSG_CLIPCONTROL 1

#include <osg/StateAttribute>

#ifndef GL_VERSION_4_5
    #define GL_NEGATIVE_ONE_TO_ONE            0x935E
    #define GL_ZERO_TO_ONE                    0x935F
#endif

namespace osg {

/** Encapsulate OpenGL glClipControl functions.
*/
class OSG_EXPORT ClipControl : public StateAttribute
{
    public :

        enum Origin
        {
            LOWER_LEFT = GL_LOWER_LEFT,
            UPPER_LEFT = GL_UPPER_LEFT
        };

        enum DepthMode
        {
            NEGATIVE_ONE_TO_ONE = GL_NEGATIVE_ONE_TO_ONE,
            ZERO_TO_ONE         = GL_ZERO_TO_ONE
        };


        ClipControl(Origin origin=LOWER_LEFT, DepthMode depthMode=NEGATIVE_ONE_TO_ONE);

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        ClipControl(const ClipControl& clipControl,const CopyOp& copyop=CopyOp::SHALLOW_COPY);


        META_StateAttribute(osg, ClipControl, CLIPCONTROL);

        /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs.*/
        virtual int compare(const StateAttribute& sa) const
        {
            // check the types are equal and then create the rhs variable
            // used by the COMPARE_StateAttribute_Parameter macros below.
            COMPARE_StateAttribute_Types(ClipControl,sa)

            // compare each parameter in turn against the rhs.
            COMPARE_StateAttribute_Parameter(_origin)
            COMPARE_StateAttribute_Parameter(_depthMode)

            return 0; // passed all the above comparison macros, must be equal.
        }

        void setOrigin(Origin origin) { _origin = origin; }
        Origin getOrigin() const { return _origin; }

        void setDepthMode(DepthMode depthMode) { _depthMode = depthMode; }
        DepthMode getDepthMode() const { return _depthMode; }

        virtual void apply(State& state) const;

    protected:

        virtual ~ClipControl();

        Origin              _origin;
        DepthMode           _depthMode;
};

}

#endif
