#include "osg/DCS"
#include "osg/Registry"
#include "osg/Input"
#include "osg/Output"

using namespace osg;

RegisterObjectProxy<DCS> g_DCSProxy;

DCS::DCS()
{
    _mat = new Matrix(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0  );

    _mat->ref();
}


DCS::DCS(const Matrix& mat )
{
    _mat = new Matrix(mat);
    _mat->ref();
}


DCS::~DCS()
{
    _mat->unref();
}

void DCS::setMatrix(const Matrix& mat )
{
    *_mat = mat;
    dirtyBound();
}

void DCS::preTranslate( float tx, float ty, float tz )
{
    _mat->preTrans( tx, ty, tz );
    dirtyBound();
}

void DCS::preRotate( float deg, float x, float y, float z )
{
    _mat->preRot( deg, x, y, z );
    dirtyBound();
}

bool DCS::readLocalData(Input& fr)
{
    bool iteratorAdvanced = false;

    if (Matrix* tmpMatrix = static_cast<Matrix*>(Matrix::instance()->readClone(fr)))
    {

        if (_mat) _mat->unref();
        _mat = tmpMatrix;
        _mat->ref();

        iteratorAdvanced = true;
    }
    
    if (Group::readLocalData(fr)) iteratorAdvanced = true;

    return iteratorAdvanced;
}


bool DCS::writeLocalData(Output& fw)
{
    if (_mat) _mat->write(fw);

    
    return Group::writeLocalData(fw);
}

bool DCS::computeBound( void )
{
    if (!Group::computeBound()) return false;

    Vec3 xdash = _bsphere._center;
    xdash.x() += _bsphere._radius;
    xdash = xdash*(*_mat);

    Vec3 ydash = _bsphere._center;
    ydash.y() += _bsphere._radius;
    ydash = ydash*(*_mat);

    Vec3 zdash = _bsphere._center;
    zdash.y() += _bsphere._radius;
    zdash = zdash*(*_mat);
    
    _bsphere._center = _bsphere._center*(*_mat);
    
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
