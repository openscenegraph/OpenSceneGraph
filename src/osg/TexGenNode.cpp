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
#include <osg/TexGenNode>

using namespace osg;

TexGenNode::TexGenNode():
    _referenceFrame(RELATIVE_RF)
{
    // switch off culling of tex gen nodes by default.
    setCullingActive(false);
    
    _textureUnit = 0;
    _value = StateAttribute::ON;
    setStateSet(new StateSet);
    _texgen = new TexGen;
}

TexGenNode::TexGenNode(const TexGenNode& cn, const CopyOp& copyop):
    Group(cn,copyop),
    _textureUnit(cn._textureUnit),
    _value(cn._value),
    _texgen(static_cast<TexGen*>(copyop(cn._texgen.get()))),
    _referenceFrame(cn._referenceFrame)
{
    setStateSet(new StateSet);
}

TexGenNode::TexGenNode(TexGen *texgen):
    _referenceFrame(RELATIVE_RF)
{
    // switch off culling of tex gen nodes by default.
    setCullingActive(false);

    _textureUnit = 0;
    _value = StateAttribute::ON;
    setStateSet(new StateSet);
    _texgen = texgen;
}

TexGenNode::~TexGenNode()
{
}

void TexGenNode::setReferenceFrame(ReferenceFrame rf)
{
    _referenceFrame = rf;
}

void TexGenNode::setTexGen(TexGen* texgen)
{
    _texgen = texgen;
}


void TexGenNode::setThreadSafeRefUnref(bool threadSafe)
{
    Group::setThreadSafeRefUnref(threadSafe);

    if (_texgen.valid()) _texgen->setThreadSafeRefUnref(threadSafe);
}
