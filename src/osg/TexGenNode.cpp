/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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
#include <osg/TexGenNode>

using namespace osg;

TexGenNode::TexGenNode()
{
    _value = StateAttribute::ON;
    _stateset = new StateSet;
    _texgen = new TexGen;
    setLocalStateSetModes(_value);
}

TexGenNode::TexGenNode(const TexGenNode& cn, const CopyOp& copyop):
    Group(cn,copyop),
    _value(cn._value),
    _texgen(static_cast<TexGen*>(copyop(cn._texgen.get())))
{
}

TexGenNode::~TexGenNode()
{
}

void TexGenNode::setTexGen(TexGen* texgen)
{
    _texgen = texgen;
    setLocalStateSetModes(_value);
}

// Set the GLModes on StateSet associated with the TexGen.
void TexGenNode::setStateSetModes(StateSet& stateset,const StateAttribute::GLModeValue value) const
{
    if (_texgen.valid())
        stateset.setAssociatedModes(_texgen.get(),value);
}

void TexGenNode::setLocalStateSetModes(const StateAttribute::GLModeValue value)
{
    if (!_stateset) _stateset = new StateSet;
    _stateset->setAllToInherit();
    setStateSetModes(*_stateset,value);
}
