#include <osg/Projection>

using namespace osg;

Projection::Projection()
{
}

Projection::Projection(const Projection& projection,const CopyOp& copyop):
    Group(projection,copyop),
    _matrix(projection._matrix)
{    
}

Projection::Projection(const Matrix& mat )
{
    _matrix = mat;
}


Projection::~Projection()
{
}
