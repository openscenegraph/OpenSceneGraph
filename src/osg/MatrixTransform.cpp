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
#include <osg/MatrixTransform>

using namespace osg;

MatrixTransform::MatrixTransform():
    _inverseDirty(false)
{
}

MatrixTransform::MatrixTransform(const MatrixTransform& transform,const CopyOp& copyop):
    Transform(transform,copyop),
    _matrix(transform._matrix),
    _inverse(transform._inverse),
    _inverseDirty(transform._inverseDirty)
{
}

MatrixTransform::MatrixTransform(const Matrix& mat )
{
    _referenceFrame = RELATIVE_RF;

    _matrix = mat;
    _inverseDirty = true;
}


MatrixTransform::~MatrixTransform()
{
}

bool MatrixTransform::computeLocalToWorldMatrix(Matrix& matrix,NodeVisitor*) const
{
    if (_referenceFrame==RELATIVE_RF)
    {
        matrix.preMult(_matrix);
    }
    else // absolute
    {
        matrix = _matrix;
    }
    return true;
}

bool MatrixTransform::computeWorldToLocalMatrix(Matrix& matrix,NodeVisitor*) const
{
    const Matrix& inverse = getInverseMatrix();

    if (_referenceFrame==RELATIVE_RF)
    {
        matrix.postMult(inverse);
    }
    else // absolute
    {
        matrix = inverse;
    }
    return true;
}
