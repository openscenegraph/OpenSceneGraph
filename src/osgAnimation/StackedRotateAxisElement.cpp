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

#include <osgAnimation/StackedRotateAxisElement>

using namespace osgAnimation;

StackedRotateAxisElement::StackedRotateAxisElement(const std::string& name, const osg::Vec3& axis, double angle) : StackedTransformElement(), _axis(axis), _angle(angle) { setName(name); }
StackedRotateAxisElement::StackedRotateAxisElement(const osg::Vec3& axis, double angle) : _axis(axis), _angle(angle) { setName("rotateaxis"); }
StackedRotateAxisElement::StackedRotateAxisElement() : _axis(osg::Vec3(1,0,0)), _angle(0) {}
StackedRotateAxisElement::StackedRotateAxisElement(const StackedRotateAxisElement& rhs, const osg::CopyOp&) : StackedTransformElement(rhs), _axis(rhs._axis), _angle(rhs._angle)
{
    if (rhs._target.valid())
        _target = new FloatTarget(*rhs._target);
}


osg::Matrix StackedRotateAxisElement::getAsMatrix() const { return osg::Matrix::rotate(osg::Quat(_angle, _axis)); }
void StackedRotateAxisElement::update(float /*t*/)
{
    if (_target.valid())
        _angle = _target->getValue();
}

const osg::Vec3& StackedRotateAxisElement::getAxis() const { return _axis; }
double StackedRotateAxisElement::getAngle() const { return _angle; }
void StackedRotateAxisElement::setAxis(const osg::Vec3& axis)
{
    _axis = axis;
}

void StackedRotateAxisElement::setAngle(double angle)
{
    _angle = angle;
}

Target* StackedRotateAxisElement::getOrCreateTarget()
{
    if (!_target.valid())
        _target = new FloatTarget(_angle);
    return _target.get();
}

void StackedRotateAxisElement::applyToMatrix(osg::Matrix& matrix) const {    matrix.preMultRotate(osg::Quat(_angle, _axis)); }
