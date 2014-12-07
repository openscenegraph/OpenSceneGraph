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

#ifndef OSG_SAMPLEMASKI
#define OSG_SAMPLEMASKI 1

#include <osg/StateAttribute>

namespace osg {

/**
 *  osg::SampleMaski does nothing if OpenGL 3.2 or ARB_texture_multisample are not available.
*/
class OSG_EXPORT SampleMaski : public StateAttribute
{
    public :

        SampleMaski();

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        SampleMaski(const SampleMaski& sampleMaski,const CopyOp& copyop=CopyOp::SHALLOW_COPY);

        META_StateAttribute(osg, SampleMaski, SAMPLEMASKI)

        /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs.*/
        virtual int compare(const StateAttribute& sa) const;

        inline void setMask(unsigned int mask, unsigned int maskNumber = 0u ) { _sampleMask[maskNumber] = mask; }

        inline unsigned int getMask(unsigned int maskNumber = 0u) const { return _sampleMask[maskNumber]; }

        virtual void apply(State& state) const;

    protected:

        virtual ~SampleMaski();

//For now support only up to 64 bit mask;
        unsigned int        _sampleMask[2];
};

}

#endif
