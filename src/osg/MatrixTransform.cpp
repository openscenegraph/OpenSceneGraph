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
    _referenceFrame = RELATIVE_TO_PARENTS;

    _matrix = mat;
    _inverseDirty = false;
}


MatrixTransform::~MatrixTransform()
{
}

bool MatrixTransform::computeLocalToWorldMatrix(Matrix& matrix,NodeVisitor*) const
{
    if (_referenceFrame==RELATIVE_TO_PARENTS)
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

    if (_referenceFrame==RELATIVE_TO_PARENTS)
    {
        matrix.postMult(inverse);
    }
    else // absolute
    {
        matrix = inverse;
    }
    return true;
}
