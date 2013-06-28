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

#include <osgAnimation/StackedTranslateElement>

using namespace osgAnimation;


StackedTranslateElement::StackedTranslateElement(const std::string& name, const osg::Vec3& translate) : _translate(translate) { setName(name); }
StackedTranslateElement::StackedTranslateElement(const osg::Vec3& translate) : _translate(translate) { setName("translate"); }


StackedTranslateElement::StackedTranslateElement() {}
StackedTranslateElement::StackedTranslateElement(const StackedTranslateElement& rhs, const osg::CopyOp&) : StackedTransformElement(rhs), _translate(rhs._translate)
{
    if (rhs._target.valid())
        _target = new Vec3Target(*rhs._target);
}

void StackedTranslateElement::applyToMatrix(osg::Matrix& matrix) const {matrix.preMultTranslate(_translate);}
osg::Matrix StackedTranslateElement::getAsMatrix() const { return osg::Matrix::translate(_translate); }
bool StackedTranslateElement::isIdentity() const { return (_translate[0] == 0 && _translate[1] == 0 && _translate[2] == 0); }
const osg::Vec3& StackedTranslateElement::getTranslate() const {    return _translate; }
void StackedTranslateElement::setTranslate(const osg::Vec3& value) { _translate = value; }

Target* StackedTranslateElement::getOrCreateTarget()
{
    if (!_target.valid())
        _target = new Vec3Target(_translate);
    return _target.get();
}
Target* StackedTranslateElement::getTarget() {return _target.get();}
const Target* StackedTranslateElement::getTarget() const {return _target.get();}

void StackedTranslateElement::update(float /*t*/)
{
    if (_target.valid())
        _translate = _target->getValue();
}
