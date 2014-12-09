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

#ifndef OSG_BLENDCOLOR
#define OSG_BLENDCOLOR 1

#include <osg/GL>
#include <osg/StateAttribute>
#include <osg/ref_ptr>
#include <osg/Vec4>


namespace osg {

/** Encapsulates OpenGL blend/transparency state. */
class OSG_EXPORT BlendColor : public StateAttribute
{
    public :

        BlendColor();

        BlendColor(const osg::Vec4& constantColor);

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        BlendColor(const BlendColor& trans,const CopyOp& copyop=CopyOp::SHALLOW_COPY):
            StateAttribute(trans,copyop),
            _constantColor(trans._constantColor) {}

        META_StateAttribute(osg, BlendColor,BLENDCOLOR);

        /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
        virtual int compare(const StateAttribute& sa) const
        {
            // Check for equal types, then create the rhs variable
            // used by the COMPARE_StateAttribute_Parameter macros below.
            COMPARE_StateAttribute_Types(BlendColor,sa)

            // Compare each parameter in turn against the rhs.
            COMPARE_StateAttribute_Parameter(_constantColor)

            return 0; // Passed all the above comparison macros, so must be equal.
        }

        virtual bool getModeUsage(StateAttribute::ModeUsage& usage) const
        {
            usage.usesMode(GL_BLEND);
            return true;
        }

        void setConstantColor(const osg::Vec4& color) { _constantColor = color; }

        inline osg::Vec4& getConstantColor() { return _constantColor; }

        inline const osg::Vec4& getConstantColor() const { return _constantColor; }

        virtual void apply(State& state) const;

    protected :

        virtual ~BlendColor();

        osg::Vec4 _constantColor;
};

}

#endif
