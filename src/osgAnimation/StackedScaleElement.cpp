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

#include <osgAnimation/StackedScaleElement>

using namespace osgAnimation;

StackedScaleElement::StackedScaleElement(const std::string& name, const osg::Vec3& scale) : _scale(scale) { setName(name); }
StackedScaleElement::StackedScaleElement(const osg::Vec3& scale) : _scale(scale) { setName("scale"); }

StackedScaleElement::StackedScaleElement(const StackedScaleElement& rhs, const osg::CopyOp&) : StackedTransformElement(rhs), _scale(rhs._scale)
{
    if (rhs._target.valid())
        _target = new Vec3Target(*rhs._target);
}

const osg::Vec3& StackedScaleElement::getScale() const { return _scale; }
void StackedScaleElement::setScale(const osg::Vec3& scale) { _scale = scale; }

Target* StackedScaleElement::getTarget() {return _target.get();}
const Target* StackedScaleElement::getTarget() const {return _target.get();}

bool StackedScaleElement::isIdentity() const { return (_scale.x() == 1 && _scale.y() == 1 && _scale.z() == 1); }

osg::Matrix StackedScaleElement::getAsMatrix() const { return osg::Matrix::scale(_scale); }

void StackedScaleElement::applyToMatrix(osg::Matrix& matrix) const {    matrix.preMultScale(_scale); }

StackedScaleElement::StackedScaleElement()
{
    _scale = osg::Vec3(1,1,1);
}

void StackedScaleElement::update() 
{ 
    if (_target.valid())
        _scale = _target->getValue();
}

Target* StackedScaleElement::getOrCreateTarget()
{    
    if (!_target.valid())
        _target = new Vec3Target(_scale);
    return _target.get();
}
