#include <osg/Projection>

using namespace osg;

Projection::Projection()
{
    _matrix = osgNew Matrix;
}

Projection::Projection(const Projection& projection,const CopyOp& copyop):
    Group(projection,copyop),
    _matrix(osgNew Matrix(*projection._matrix))
{    
}

Projection::Projection(const Matrix& mat )
{
    _matrix = osgNew Matrix(mat);
}


Projection::~Projection()
{
}
