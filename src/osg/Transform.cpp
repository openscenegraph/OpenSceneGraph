#include <osg/Transform>

using namespace osg;

Transform::Transform()
{
    _referenceFrame = RELATIVE_TO_PARENTS;

    _matrix = osgNew Matrix;
    _inverse = osgNew Matrix;
    _inverseDirty = false;
}

Transform::Transform(const Transform& transform,const CopyOp& copyop):
    Group(transform,copyop),
    _computeTransformCallback(_computeTransformCallback),
    _referenceFrame(transform._referenceFrame),
    _matrix(osgNew Matrix(*transform._matrix)),
    _inverse(osgNew Matrix(*transform._inverse)),
    _inverseDirty(transform._inverseDirty)
{    
}

Transform::Transform(const Matrix& mat )
{
    _referenceFrame = RELATIVE_TO_PARENTS;

    _matrix = osgNew Matrix(mat);
    _inverse = osgNew Matrix();
    _inverseDirty = false;
}


Transform::~Transform()
{
}

void Transform::setReferenceFrame(ReferenceFrame rf)
{
    if (_referenceFrame == rf) return;
    
    _referenceFrame = rf;
    
    // switch off culling if transform is absolute.
    if (_referenceFrame==RELATIVE_TO_ABSOLUTE) setCullingActive(false);
    else setCullingActive(true);
}

const bool Transform::computeBound() const
{
    if (!Group::computeBound()) return false;
    
    // note, NULL pointer for NodeVisitor, so compute's need
    // to handle this case gracefully, normally this should not be a problem.
    Matrix l2w;

    getLocalToWorldMatrix(l2w,NULL);

    Vec3 xdash = _bsphere._center;
    xdash.x() += _bsphere._radius;
    xdash = xdash*l2w;

    Vec3 ydash = _bsphere._center;
    ydash.y() += _bsphere._radius;
    ydash = ydash*l2w;

    Vec3 zdash = _bsphere._center;
    zdash.y() += _bsphere._radius;
    zdash = zdash*l2w;

    _bsphere._center = _bsphere._center*l2w;

    xdash -= _bsphere._center;
    float len_xdash = xdash.length();

    ydash -= _bsphere._center;
    float len_ydash = ydash.length();

    zdash -= _bsphere._center;
    float len_zdash = zdash.length();

    _bsphere._radius = len_xdash;
    if (_bsphere._radius<len_ydash) _bsphere._radius = len_ydash;
    if (_bsphere._radius<len_zdash) _bsphere._radius = len_zdash;
    
    return true;

}
