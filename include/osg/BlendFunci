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

#ifndef OSG_BLENDFUNCI
#define OSG_BLENDFUNCI 1

#include <osg/BlendFunc>

namespace osg {

/** Encapsulates glBlendFunci function : the index version of glBlendEquation for multiple render target.
*/
class OSG_EXPORT BlendFunci : public BlendFunc
{
    public :

        BlendFunci();

        BlendFunci(unsigned int buf, GLenum source, GLenum destination):
            BlendFunc(source, destination),
            _index(buf) {}

        BlendFunci(unsigned int buf, GLenum source, GLenum destination, GLenum source_alpha, GLenum destination_alpha):
            BlendFunc(source, destination, source_alpha, destination_alpha),
            _index(buf) {}

        /** Copy constructor using CopyOp to manage deep vs shallow copy. */
        BlendFunci(const BlendFunci& cm,const CopyOp& copyop=CopyOp::SHALLOW_COPY):
            BlendFunc(cm,copyop),
            _index(cm._index) {}

        META_StateAttribute(osg, BlendFunci, BLENDFUNC);

        /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
        virtual int compare(const StateAttribute& sa) const
        {
            // Check for equal types, then create the rhs variable
            // used by the COMPARE_StateAttribute_Parameter macros below.
            COMPARE_StateAttribute_Types(BlendFunci,sa)

            COMPARE_StateAttribute_Parameter(_index);

            return BlendFunc::compare(sa);
        }

        /** Return the buffer index as the member identifier.*/
        virtual unsigned int getMember() const { return _index; }

        /** Set the renderbuffer index of the BlendFunci. */
        void setIndex(unsigned int buf);

        /** Get the renderbuffer index of the BlendFunci. */
        unsigned int getIndex() const { return _index; }

        virtual void apply(State& state) const;

    protected:

        virtual ~BlendFunci();

        unsigned int    _index;

};

}

#endif
