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

#ifndef OSG_VIEWPORTINDEXED
#define OSG_VIEWPORTINDEXED 1

#include <osg/Viewport>

namespace osg {

/** Encapsulates glViewportIndexed function : the index version of glViewport for multiple render target.
*/
class OSG_EXPORT ViewportIndexed : public Viewport
{
    public :

        ViewportIndexed();

        ViewportIndexed(unsigned int index, value_type x,value_type y,value_type width,value_type height):
            Viewport(x,y,width,height),
            _index(index) {}

        /** Copy constructor using CopyOp to manage deep vs shallow copy. */
        ViewportIndexed(const ViewportIndexed& cm,const CopyOp& copyop=CopyOp::SHALLOW_COPY):
            Viewport(cm,copyop),
            _index(cm._index) {}

        META_StateAttribute(osg, ViewportIndexed, VIEWPORTINDEXED);

        /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
        virtual int compare(const StateAttribute& sa) const
        {
            // Check for equal types, then create the rhs variable
            // used by the COMPARE_StateAttribute_Parameter macros below.
            COMPARE_StateAttribute_Types(ViewportIndexed,sa)

            COMPARE_StateAttribute_Parameter(_index);

            return Viewport::compare(sa);
        }

        /** Return the buffer index as the member identifier.*/
        virtual unsigned int getMember() const { return _index; }

        /** Set the index of the ViewportIndexed. */
        void setIndex(unsigned int index);

        /** Get the index of the ViewportIndexed. */
        unsigned int getIndex() const { return _index; }

        virtual void apply(State& state) const;

    protected:

        virtual ~ViewportIndexed();

        unsigned int    _index;

};

}

#endif
