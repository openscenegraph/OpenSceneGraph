#include <osg/Transform>

using namespace osg;

Transform::Transform()
{
    _type = DYNAMIC;
    _mode = MODEL;
    
    _localToWorld = new Matrix;
    _localToWorld->makeIdentity();
    _localToWorldDirty = false;

    _worldToLocal = new Matrix;
    _worldToLocal->makeIdentity();
    _worldToLocalDirty = false;
}

Transform::Transform(const Transform& transform,const CopyOp& copyop):
    Group(transform,copyop),
    _type(transform._type),
    _mode(transform._mode),
    _localToWorldDirty(transform._localToWorldDirty),
    _localToWorld(transform._localToWorld),
    _worldToLocalDirty(transform._worldToLocalDirty),
    _worldToLocal(transform._localToWorld) 
{    
}

Transform::Transform(const Matrix& mat )
{
    _type = DYNAMIC;
    _mode = MODEL;

    _localToWorld = new Matrix(mat);
    _localToWorldDirty = false;

    _worldToLocal = new Matrix;
    _worldToLocalDirty = true;
}


Transform::~Transform()
{
}

void Transform::setMatrix(const Matrix& mat )
{
    if (_mode==MODEL)
    {
        (*_localToWorld) = mat; 
        _localToWorldDirty = false;
        _worldToLocalDirty = true;
    }
    else
    {
        (*_worldToLocal) = mat; 
        _worldToLocalDirty = false;
        _localToWorldDirty = true;
    }

    dirtyBound();
}

/** preMult transform.*/
void Transform::preMult( const Matrix& mat )
{
    if (_mode==MODEL)
    {
        _localToWorld->preMult(mat); 
        _localToWorldDirty = false;
        _worldToLocalDirty = true;
    }
    else
    {
        _worldToLocal->preMult(mat); 
        _worldToLocalDirty = false;
        _localToWorldDirty = true;
    }

    dirtyBound();
}

/** postMult transform.*/
void Transform::postMult( const Matrix& mat )
{
    if (_mode==MODEL)
    {
        _localToWorld->postMult(mat); 
        _localToWorldDirty = false;
        _worldToLocalDirty = true;
    }
    else
    {
        _worldToLocal->postMult(mat); 
        _worldToLocalDirty = false;
        _localToWorldDirty = true;
    }

    dirtyBound();
}

const bool Transform::computeBound() const
{
    if (!Group::computeBound()) return false;

    if (_localToWorldDirty) computeLocalToWorld();

    Vec3 xdash = _bsphere._center;
    xdash.x() += _bsphere._radius;
    xdash = xdash*(*_localToWorld);

    Vec3 ydash = _bsphere._center;
    ydash.y() += _bsphere._radius;
    ydash = ydash*(*_localToWorld);

    Vec3 zdash = _bsphere._center;
    zdash.y() += _bsphere._radius;
    zdash = zdash*(*_localToWorld);

    _bsphere._center = _bsphere._center*(*_localToWorld);

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

void Transform::computeLocalToWorld() const
{
    if (_localToWorldDirty) 
    {
        if  (_mode==VIEW)
        {
            _localToWorld->invert(*_worldToLocal);
        }
        _localToWorldDirty = false;
    }
}

void Transform::computeWorldToLocal() const
{
    if (_worldToLocalDirty) 
    {
        if  (_mode==MODEL)
        {
            _worldToLocal->invert(*_localToWorld);
        }
        _worldToLocalDirty = false;
    }
}
