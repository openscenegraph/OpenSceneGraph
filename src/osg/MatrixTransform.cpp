#include <osg/MatrixTransform>

using namespace osg;

MatrixTransform::MatrixTransform()
{
    _matrix = osgNew Matrix;
    _inverse = osgNew Matrix;
    _inverseDirty = false;
}

MatrixTransform::MatrixTransform(const MatrixTransform& transform,const CopyOp& copyop):
    Transform(transform,copyop),
    _matrix(osgNew Matrix(*transform._matrix)),
    _inverse(osgNew Matrix(*transform._inverse)),
    _inverseDirty(transform._inverseDirty)
{    
}

MatrixTransform::MatrixTransform(const Matrix& mat )
{
    _referenceFrame = RELATIVE_TO_PARENTS;

    _matrix = osgNew Matrix(mat);
    _inverse = osgNew Matrix();
    _inverseDirty = false;
}


MatrixTransform::~MatrixTransform()
{
}
