/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2014 Robert Osfield
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

#ifndef OSG_BLENDEQUATIONI
#define OSG_BLENDEQUATIONI 1

#include <osg/BlendEquation>

namespace osg {

/** Encapsulates glBlendEquationi function : the index version of glBlendEquation for multiple render target.
*/
class OSG_EXPORT BlendEquationi : public BlendEquation
{
    public :

        BlendEquationi();

        BlendEquationi(unsigned int buf, Equation equation):
            BlendEquation(equation),
            _index(buf) {}

        BlendEquationi(unsigned int buf, Equation equationRGB, Equation equationAlpha):
            BlendEquation(equationRGB, equationAlpha),
            _index(buf) {}


        /** Copy constructor using CopyOp to manage deep vs shallow copy. */
        BlendEquationi(const BlendEquationi& cm,const CopyOp& copyop=CopyOp::SHALLOW_COPY):
            BlendEquation(cm,copyop),
            _index(cm._index) {}

        META_StateAttribute(osg, BlendEquationi, BLENDEQUATION);

        /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
        virtual int compare(const StateAttribute& sa) const
        {
            // Check for equal types, then create the rhs variable
            // used by the COMPARE_StateAttribute_Parameter macros below.
            COMPARE_StateAttribute_Types(BlendEquationi,sa)

            COMPARE_StateAttribute_Parameter(_index);

            return BlendEquation::compare(sa);
        }

        /** Return the buffer index as the member identifier.*/
        virtual unsigned int getMember() const { return _index; }

        /** Set the renderbuffer index of the BlendEquationi. */
        void setIndex(unsigned int buf);

        /** Get the renderbuffer index of the BlendEquationi. */
        unsigned int getIndex() const { return _index; }

        virtual void apply(State& state) const;

    protected:

        virtual ~BlendEquationi();

        unsigned int    _index;

};

}

#endif
