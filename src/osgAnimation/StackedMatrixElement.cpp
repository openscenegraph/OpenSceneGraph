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

#include <osgAnimation/StackedMatrixElement>

using namespace osgAnimation;

StackedMatrixElement::StackedMatrixElement() {}
StackedMatrixElement::StackedMatrixElement(const std::string& name, const osg::Matrix& matrix) : StackedTransformElement(), _matrix(matrix) { setName(name); }
StackedMatrixElement::StackedMatrixElement(const osg::Matrix& matrix) : _matrix(matrix) { setName("matrix"); }

StackedMatrixElement::StackedMatrixElement(const StackedMatrixElement& rhs, const osg::CopyOp& c) : StackedTransformElement(rhs, c), _matrix(rhs._matrix)
{
    if (rhs._target.valid())
        _target = new MatrixTarget(*rhs._target);
}

Target* StackedMatrixElement::getOrCreateTarget()
{
    if (!_target.valid())
        _target = new MatrixTarget(_matrix);
    return _target.get(); 
}

void StackedMatrixElement::update()
{
    if (_target.valid())
        _matrix = _target->getValue();
}
