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
#include <osg/GL>
#include <osg/PolygonStipple>
#include <osg/Notify>

#include <algorithm>

using namespace osg;

static GLubyte defaultPolygonStippleMask[] =
{
    0x44, 0x44, 0x44, 0x44, 0x99, 0x99, 0x99, 0x99,
    0x44, 0x44, 0x44, 0x44, 0x99, 0x99, 0x99, 0x99,
    0x44, 0x44, 0x44, 0x44, 0x99, 0x99, 0x99, 0x99,
    0x44, 0x44, 0x44, 0x44, 0x99, 0x99, 0x99, 0x99,
    0x44, 0x44, 0x44, 0x44, 0x99, 0x99, 0x99, 0x99,
    0x44, 0x44, 0x44, 0x44, 0x99, 0x99, 0x99, 0x99,
    0x44, 0x44, 0x44, 0x44, 0x99, 0x99, 0x99, 0x99,
    0x44, 0x44, 0x44, 0x44, 0x99, 0x99, 0x99, 0x99,
    0x44, 0x44, 0x44, 0x44, 0x99, 0x99, 0x99, 0x99,
    0x44, 0x44, 0x44, 0x44, 0x99, 0x99, 0x99, 0x99,
    0x44, 0x44, 0x44, 0x44, 0x99, 0x99, 0x99, 0x99,
    0x44, 0x44, 0x44, 0x44, 0x99, 0x99, 0x99, 0x99,
    0x44, 0x44, 0x44, 0x44, 0x99, 0x99, 0x99, 0x99,
    0x44, 0x44, 0x44, 0x44, 0x99, 0x99, 0x99, 0x99,
    0x44, 0x44, 0x44, 0x44, 0x99, 0x99, 0x99, 0x99,
    0x44, 0x44, 0x44, 0x44, 0x99, 0x99, 0x99, 0x99
};

PolygonStipple::PolygonStipple()
{
    setMask(defaultPolygonStippleMask);
}

PolygonStipple::PolygonStipple(const PolygonStipple& ps,const CopyOp& copyop):
    StateAttribute(ps,copyop)
{
    setMask(ps.getMask());
}

PolygonStipple::~PolygonStipple()
{
}

int PolygonStipple::compare(const StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Paramter macro's below.
    COMPARE_StateAttribute_Types(PolygonStipple,sa)

    // compare each parameter in turn against the rhs.
    for(unsigned int i=0;i<128;++i)
    {
        if (_mask[i]<rhs._mask[i]) return -1;
        else if (_mask[i]>rhs._mask[i]) return 1;
    }

    return 0; // passed all the above comparison macro's, must be equal.
}
    
void PolygonStipple::setMask(const GLubyte* givenMask)
{
    std::copy(givenMask,givenMask+128,_mask);
}

void PolygonStipple::apply(State&) const
{
    glPolygonStipple(_mask);
}

