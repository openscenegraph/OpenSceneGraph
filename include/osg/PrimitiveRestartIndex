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

#ifndef OSG_PRIMITIVERESTARTINDEX
#define OSG_PRIMITIVERESTARTINDEX 1

#include <osg/StateAttribute>

namespace osg {

/**
 *  osg::PrimitiveRestartIndex does nothing if OpenGL 3.1 is not available.
*/
class OSG_EXPORT PrimitiveRestartIndex : public StateAttribute
{
    public :

        PrimitiveRestartIndex();
        PrimitiveRestartIndex(unsigned int restartIndex);

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        PrimitiveRestartIndex(const PrimitiveRestartIndex& primitiveRestartIndex,const CopyOp& copyop=CopyOp::SHALLOW_COPY);

        META_StateAttribute(osg, PrimitiveRestartIndex, PRIMITIVERESTARTINDEX)

        /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs.*/
        virtual int compare(const StateAttribute& sa) const;

        inline void setRestartIndex(unsigned int restartIndex ) { _restartIndex = restartIndex; }

        inline unsigned int getRestartIndex() const { return _restartIndex; }

        virtual void apply(State& state) const;

    protected:

        virtual ~PrimitiveRestartIndex();

        unsigned int        _restartIndex;
};

}

#endif
