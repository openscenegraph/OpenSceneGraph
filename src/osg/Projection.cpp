#include <osg/Projection>

using namespace osg;

Projection::Projection()
{
    _matrix = new Matrix;
}

Projection::Projection(const Projection& projection,const CopyOp& copyop):
    Group(projection,copyop),
    _matrix(new Matrix(*projection._matrix))
{    
}

Projection::Projection(const Matrix& mat )
{
    _matrix = new Matrix(mat);
}


Projection::~Projection()
{
}
