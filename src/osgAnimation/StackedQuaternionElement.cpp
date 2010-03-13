/*  -*-c++-*- 
 *  Copyright (C) 2009 Cedric Pinson <cedric.pinson@plopbyte.net>
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

#include <osgAnimation/StackedQuaternionElement>

using namespace osgAnimation;

StackedQuaternionElement::StackedQuaternionElement(const std::string& name, const osg::Quat& quaternion) : _quaternion(quaternion) { setName(name); }

StackedQuaternionElement::StackedQuaternionElement(const StackedQuaternionElement& rhs, const osg::CopyOp&) : StackedTransformElement(rhs), _quaternion(rhs._quaternion)
{
    if (rhs._target.valid())
        _target = new QuatTarget(*rhs._target);
}


StackedQuaternionElement::StackedQuaternionElement(const osg::Quat& quat) : _quaternion(quat) { setName("quaternion"); }

StackedQuaternionElement::StackedQuaternionElement()
{
}
const osg::Quat& StackedQuaternionElement::getQuaternion() const { return _quaternion; }
void StackedQuaternionElement::setQuaternion(const osg::Quat& q) { _quaternion = q; }

void StackedQuaternionElement::applyToMatrix(osg::Matrix& matrix) const {matrix.preMultRotate(_quaternion);}
osg::Matrix StackedQuaternionElement::getAsMatrix() const { return osg::Matrix(_quaternion); }
bool StackedQuaternionElement::isIdentity() const { return (_quaternion[0] == 0 && _quaternion[1] == 0 && _quaternion[2] == 0 && _quaternion[3] == 1.0); }

void StackedQuaternionElement::update() 
{ 
    if (_target.valid())
        _quaternion = _target->getValue();
}

Target* StackedQuaternionElement::getOrCreateTarget()
{    
    if (!_target.valid())
        _target = new QuatTarget(_quaternion);
    return _target.get();
}
Target* StackedQuaternionElement::getTarget() {return _target.get();}
const Target* StackedQuaternionElement::getTarget() const {return _target.get();}
