#include <osg/MatrixTransform>

using namespace osg;

MatrixTransform::MatrixTransform():
    _inverseDirty(false)
{
    _matrix = new Matrix;
    _inverse = new Matrix;
}

MatrixTransform::MatrixTransform(const MatrixTransform& transform,const CopyOp& copyop):
    Transform(transform,copyop),
    _matrix(new Matrix(*transform._matrix)),
    _inverse(new Matrix(*transform._inverse)),
    _inverseDirty(transform._inverseDirty)
{    
}

MatrixTransform::MatrixTransform(const Matrix& mat )
{
    _referenceFrame = RELATIVE_TO_PARENTS;

    _matrix = new Matrix(mat);
    _inverse = new Matrix();
    _inverseDirty = false;
}


MatrixTransform::~MatrixTransform()
{
}

bool MatrixTransform::computeLocalToWorldMatrix(Matrix& matrix,NodeVisitor*) const
{
    if (_referenceFrame==RELATIVE_TO_PARENTS)
    {
        matrix.preMult(*_matrix);
    }
    else // absolute
    {
        matrix = *_matrix;
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
