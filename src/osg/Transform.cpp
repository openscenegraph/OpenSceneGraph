#include <osg/Transform>

using namespace osg;

Transform::Transform()
{
    _type = DYNAMIC;
    _matrix = new osg::Matrix();
    _matrix->makeIdent();
}


Transform::Transform(const Matrix& mat )
{
    _type = DYNAMIC;
    (*_matrix) = mat;
}


Transform::~Transform()
{
}

const bool Transform::computeBound() const
{
    if (!Group::computeBound()) return false;

    Vec3 xdash = _bsphere._center;
    xdash.x() += _bsphere._radius;
    xdash = xdash*(*_matrix);

    Vec3 ydash = _bsphere._center;
    ydash.y() += _bsphere._radius;
    ydash = ydash*(*_matrix);

    Vec3 zdash = _bsphere._center;
    zdash.y() += _bsphere._radius;
    zdash = zdash*(*_matrix);

    _bsphere._center = _bsphere._center*(*_matrix);

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
