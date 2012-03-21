/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include <osg/Quat>
#include <osg/Notify>
#include <osg/Math>
#include <osg/Timer>

#include <osg/GL>

#include <limits>
#include <stdlib.h>
#include <float.h>

using namespace osg;

#define SET_ROW(row, v1, v2, v3, v4 )    \
    _mat[(row)][0] = (v1); \
    _mat[(row)][1] = (v2); \
    _mat[(row)][2] = (v3); \
    _mat[(row)][3] = (v4);

#define INNER_PRODUCT(a,b,r,c) \
     ((a)._mat[r][0] * (b)._mat[0][c]) \
    +((a)._mat[r][1] * (b)._mat[1][c]) \
    +((a)._mat[r][2] * (b)._mat[2][c]) \
    +((a)._mat[r][3] * (b)._mat[3][c])


Matrix_implementation::Matrix_implementation( value_type a00, value_type a01, value_type a02, value_type a03,
                  value_type a10, value_type a11, value_type a12, value_type a13,
                  value_type a20, value_type a21, value_type a22, value_type a23,
                  value_type a30, value_type a31, value_type a32, value_type a33)
{
    SET_ROW(0, a00, a01, a02, a03 )
    SET_ROW(1, a10, a11, a12, a13 )
    SET_ROW(2, a20, a21, a22, a23 )
    SET_ROW(3, a30, a31, a32, a33 )
}

void Matrix_implementation::set( value_type a00, value_type a01, value_type a02, value_type a03,
                   value_type a10, value_type a11, value_type a12, value_type a13,
                   value_type a20, value_type a21, value_type a22, value_type a23,
                   value_type a30, value_type a31, value_type a32, value_type a33)
{
    SET_ROW(0, a00, a01, a02, a03 )
    SET_ROW(1, a10, a11, a12, a13 )
    SET_ROW(2, a20, a21, a22, a23 )
    SET_ROW(3, a30, a31, a32, a33 )
}

#define QX  q._v[0]
#define QY  q._v[1]
#define QZ  q._v[2]
#define QW  q._v[3]

void Matrix_implementation::setRotate(const Quat& q)
{
    double length2 = q.length2();
    if (fabs(length2) <= std::numeric_limits<double>::min())
    {
        _mat[0][0] = 0.0; _mat[1][0] = 0.0; _mat[2][0] = 0.0;
        _mat[0][1] = 0.0; _mat[1][1] = 0.0; _mat[2][1] = 0.0;
        _mat[0][2] = 0.0; _mat[1][2] = 0.0; _mat[2][2] = 0.0;
    }
    else
    {
        double rlength2;
        // normalize quat if required.
        // We can avoid the expensive sqrt in this case since all 'coefficients' below are products of two q components.
        // That is a square of a square root, so it is possible to avoid that
        if (length2 != 1.0)
        {
            rlength2 = 2.0/length2;
        }
        else
        {
            rlength2 = 2.0;
        }

        // Source: Gamasutra, Rotating Objects Using Quaternions
        //
        //http://www.gamasutra.com/features/19980703/quaternions_01.htm

        double wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

        // calculate coefficients
        x2 = rlength2*QX;
        y2 = rlength2*QY;
        z2 = rlength2*QZ;

        xx = QX * x2;
        xy = QX * y2;
        xz = QX * z2;

        yy = QY * y2;
        yz = QY * z2;
        zz = QZ * z2;

        wx = QW * x2;
        wy = QW * y2;
        wz = QW * z2;

        // Note.  Gamasutra gets the matrix assignments inverted, resulting
        // in left-handed rotations, which is contrary to OpenGL and OSG's
        // methodology.  The matrix assignment has been altered in the next
        // few lines of code to do the right thing.
        // Don Burns - Oct 13, 2001
        _mat[0][0] = 1.0 - (yy + zz);
        _mat[1][0] = xy - wz;
        _mat[2][0] = xz + wy;


        _mat[0][1] = xy + wz;
        _mat[1][1] = 1.0 - (xx + zz);
        _mat[2][1] = yz - wx;

        _mat[0][2] = xz - wy;
        _mat[1][2] = yz + wx;
        _mat[2][2] = 1.0 - (xx + yy);
    }

#if 0
    _mat[0][3] = 0.0;
    _mat[1][3] = 0.0;
    _mat[2][3] = 0.0;

    _mat[3][0] = 0.0;
    _mat[3][1] = 0.0;
    _mat[3][2] = 0.0;
    _mat[3][3] = 1.0;
#endif
}

#define COMPILE_getRotate_David_Spillings_Mk1
//#define COMPILE_getRotate_David_Spillings_Mk2
//#define COMPILE_getRotate_Original

#ifdef COMPILE_getRotate_David_Spillings_Mk1
// David Spillings implementation Mk 1
Quat Matrix_implementation::getRotate() const
{
    Quat q;

    value_type s;
    value_type tq[4];
    int    i, j;

    // Use tq to store the largest trace
    tq[0] = 1 + _mat[0][0]+_mat[1][1]+_mat[2][2];
    tq[1] = 1 + _mat[0][0]-_mat[1][1]-_mat[2][2];
    tq[2] = 1 - _mat[0][0]+_mat[1][1]-_mat[2][2];
    tq[3] = 1 - _mat[0][0]-_mat[1][1]+_mat[2][2];

    // Find the maximum (could also use stacked if's later)
    j = 0;
    for(i=1;i<4;i++) j = (tq[i]>tq[j])? i : j;

    // check the diagonal
    if (j==0)
    {
        /* perform instant calculation */
        QW = tq[0];
        QX = _mat[1][2]-_mat[2][1];
        QY = _mat[2][0]-_mat[0][2];
        QZ = _mat[0][1]-_mat[1][0];
    }
    else if (j==1)
    {
        QW = _mat[1][2]-_mat[2][1];
        QX = tq[1];
        QY = _mat[0][1]+_mat[1][0];
        QZ = _mat[2][0]+_mat[0][2];
    }
    else if (j==2)
    {
        QW = _mat[2][0]-_mat[0][2];
        QX = _mat[0][1]+_mat[1][0];
        QY = tq[2];
        QZ = _mat[1][2]+_mat[2][1];
    }
    else /* if (j==3) */
    {
        QW = _mat[0][1]-_mat[1][0];
        QX = _mat[2][0]+_mat[0][2];
        QY = _mat[1][2]+_mat[2][1];
        QZ = tq[3];
    }

    s = sqrt(0.25/tq[j]);
    QW *= s;
    QX *= s;
    QY *= s;
    QZ *= s;

    return q;

}
#endif


#ifdef COMPILE_getRotate_David_Spillings_Mk2
// David Spillings implementation Mk 2
Quat Matrix_implementation::getRotate() const
{
    Quat q;

    // From http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm
    QW = 0.5 * sqrt( osg::maximum( 0.0, 1.0 + _mat[0][0] + _mat[1][1] + _mat[2][2] ) );
    QX = 0.5 * sqrt( osg::maximum( 0.0, 1.0 + _mat[0][0] - _mat[1][1] - _mat[2][2] ) );
    QY = 0.5 * sqrt( osg::maximum( 0.0, 1.0 - _mat[0][0] + _mat[1][1] - _mat[2][2] ) );
    QZ = 0.5 * sqrt( osg::maximum( 0.0, 1.0 - _mat[0][0] - _mat[1][1] + _mat[2][2] ) );

#if 0
    // Robert Osfield, June 7th 2007, arggg this new implementation produces many many errors, so have to revert to sign(..) orignal below.
    QX = QX * osg::signOrZero(  _mat[1][2] - _mat[2][1]) ;
    QY = QY * osg::signOrZero(  _mat[2][0] - _mat[0][2]) ;
    QZ = QZ * osg::signOrZero(  _mat[0][1] - _mat[1][0]) ;
#else
    QX = QX * osg::sign(  _mat[1][2] - _mat[2][1]) ;
    QY = QY * osg::sign(  _mat[2][0] - _mat[0][2]) ;
    QZ = QZ * osg::sign(  _mat[0][1] - _mat[1][0]) ;
#endif

    return q;
}
#endif

#ifdef COMPILE_getRotate_Original
// Original implementation
Quat Matrix_implementation::getRotate() const
{
    Quat q;

    // Source: Gamasutra, Rotating Objects Using Quaternions
    //
    //http://www.gamasutra.com/features/programming/19980703/quaternions_01.htm

    value_type  tr, s;
    value_type tq[4];
    int    i, j, k;

    int nxt[3] = {1, 2, 0};

    tr = _mat[0][0] + _mat[1][1] + _mat[2][2]+1.0;

    // check the diagonal
    if (tr > 0.0)
    {
        s = (value_type)sqrt (tr);
        QW = s / 2.0;
        s = 0.5 / s;
        QX = (_mat[1][2] - _mat[2][1]) * s;
        QY = (_mat[2][0] - _mat[0][2]) * s;
        QZ = (_mat[0][1] - _mat[1][0]) * s;
    }
    else
    {
        // diagonal is negative
        i = 0;
        if (_mat[1][1] > _mat[0][0])
            i = 1;
        if (_mat[2][2] > _mat[i][i])
            i = 2;
        j = nxt[i];
        k = nxt[j];

        s = (value_type)sqrt ((_mat[i][i] - (_mat[j][j] + _mat[k][k])) + 1.0);

        tq[i] = s * 0.5;

        if (s != 0.0)
            s = 0.5 / s;

        tq[3] = (_mat[j][k] - _mat[k][j]) * s;
        tq[j] = (_mat[i][j] + _mat[j][i]) * s;
        tq[k] = (_mat[i][k] + _mat[k][i]) * s;

        QX = tq[0];
        QY = tq[1];
        QZ = tq[2];
        QW = tq[3];
    }

    return q;
}
#endif

int Matrix_implementation::compare(const Matrix_implementation& m) const
{
    const Matrix_implementation::value_type* lhs = reinterpret_cast<const Matrix_implementation::value_type*>(_mat);
    const Matrix_implementation::value_type* end_lhs = lhs+16;
    const Matrix_implementation::value_type* rhs = reinterpret_cast<const Matrix_implementation::value_type*>(m._mat);
    for(;lhs!=end_lhs;++lhs,++rhs)
    {
        if (*lhs < *rhs) return -1;
        if (*rhs < *lhs) return 1;
    }
    return 0;
}

void Matrix_implementation::setTrans( value_type tx, value_type ty, value_type tz )
{
    _mat[3][0] = tx;
    _mat[3][1] = ty;
    _mat[3][2] = tz;
}


void Matrix_implementation::setTrans( const Vec3f& v )
{
    _mat[3][0] = v[0];
    _mat[3][1] = v[1];
    _mat[3][2] = v[2];
}
void Matrix_implementation::setTrans( const Vec3d& v )
{
    _mat[3][0] = v[0];
    _mat[3][1] = v[1];
    _mat[3][2] = v[2];
}

void Matrix_implementation::makeIdentity()
{
    SET_ROW(0,    1, 0, 0, 0 )
    SET_ROW(1,    0, 1, 0, 0 )
    SET_ROW(2,    0, 0, 1, 0 )
    SET_ROW(3,    0, 0, 0, 1 )
}

void Matrix_implementation::makeScale( const Vec3f& v )
{
    makeScale(v[0], v[1], v[2] );
}

void Matrix_implementation::makeScale( const Vec3d& v )
{
    makeScale(v[0], v[1], v[2] );
}

void Matrix_implementation::makeScale( value_type x, value_type y, value_type z )
{
    SET_ROW(0,    x, 0, 0, 0 )
    SET_ROW(1,    0, y, 0, 0 )
    SET_ROW(2,    0, 0, z, 0 )
    SET_ROW(3,    0, 0, 0, 1 )
}

void Matrix_implementation::makeTranslate( const Vec3f& v )
{
    makeTranslate( v[0], v[1], v[2] );
}

void Matrix_implementation::makeTranslate( const Vec3d& v )
{
    makeTranslate( v[0], v[1], v[2] );
}

void Matrix_implementation::makeTranslate( value_type x, value_type y, value_type z )
{
    SET_ROW(0,    1, 0, 0, 0 )
    SET_ROW(1,    0, 1, 0, 0 )
    SET_ROW(2,    0, 0, 1, 0 )
    SET_ROW(3,    x, y, z, 1 )
}

void Matrix_implementation::makeRotate( const Vec3f& from, const Vec3f& to )
{
    makeIdentity();

    Quat quat;
    quat.makeRotate(from,to);
    setRotate(quat);
}
void Matrix_implementation::makeRotate( const Vec3d& from, const Vec3d& to )
{
    makeIdentity();

    Quat quat;
    quat.makeRotate(from,to);
    setRotate(quat);
}

void Matrix_implementation::makeRotate( value_type angle, const Vec3f& axis )
{
    makeIdentity();

    Quat quat;
    quat.makeRotate( angle, axis);
    setRotate(quat);
}
void Matrix_implementation::makeRotate( value_type angle, const Vec3d& axis )
{
    makeIdentity();

    Quat quat;
    quat.makeRotate( angle, axis);
    setRotate(quat);
}

void Matrix_implementation::makeRotate( value_type angle, value_type x, value_type y, value_type z )
{
    makeIdentity();

    Quat quat;
    quat.makeRotate( angle, x, y, z);
    setRotate(quat);
}

void Matrix_implementation::makeRotate( const Quat& quat )
{
    makeIdentity();

    setRotate(quat);
}

void Matrix_implementation::makeRotate( value_type angle1, const Vec3f& axis1,
                         value_type angle2, const Vec3f& axis2,
                         value_type angle3, const Vec3f& axis3)
{
    makeIdentity();

    Quat quat;
    quat.makeRotate(angle1, axis1,
                    angle2, axis2,
                    angle3, axis3);
    setRotate(quat);
}

void Matrix_implementation::makeRotate( value_type angle1, const Vec3d& axis1,
                         value_type angle2, const Vec3d& axis2,
                         value_type angle3, const Vec3d& axis3)
{
    makeIdentity();

    Quat quat;
    quat.makeRotate(angle1, axis1,
                    angle2, axis2,
                    angle3, axis3);
    setRotate(quat);
}

void Matrix_implementation::mult( const Matrix_implementation& lhs, const Matrix_implementation& rhs )
{
    if (&lhs==this)
    {
        postMult(rhs);
        return;
    }
    if (&rhs==this)
    {
        preMult(lhs);
        return;
    }

// PRECONDITION: We assume neither &lhs nor &rhs == this
// if it did, use preMult or postMult instead
    _mat[0][0] = INNER_PRODUCT(lhs, rhs, 0, 0);
    _mat[0][1] = INNER_PRODUCT(lhs, rhs, 0, 1);
    _mat[0][2] = INNER_PRODUCT(lhs, rhs, 0, 2);
    _mat[0][3] = INNER_PRODUCT(lhs, rhs, 0, 3);
    _mat[1][0] = INNER_PRODUCT(lhs, rhs, 1, 0);
    _mat[1][1] = INNER_PRODUCT(lhs, rhs, 1, 1);
    _mat[1][2] = INNER_PRODUCT(lhs, rhs, 1, 2);
    _mat[1][3] = INNER_PRODUCT(lhs, rhs, 1, 3);
    _mat[2][0] = INNER_PRODUCT(lhs, rhs, 2, 0);
    _mat[2][1] = INNER_PRODUCT(lhs, rhs, 2, 1);
    _mat[2][2] = INNER_PRODUCT(lhs, rhs, 2, 2);
    _mat[2][3] = INNER_PRODUCT(lhs, rhs, 2, 3);
    _mat[3][0] = INNER_PRODUCT(lhs, rhs, 3, 0);
    _mat[3][1] = INNER_PRODUCT(lhs, rhs, 3, 1);
    _mat[3][2] = INNER_PRODUCT(lhs, rhs, 3, 2);
    _mat[3][3] = INNER_PRODUCT(lhs, rhs, 3, 3);
}

void Matrix_implementation::preMult( const Matrix_implementation& other )
{
    // brute force method requiring a copy
    //Matrix_implementation tmp(other* *this);
    // *this = tmp;

    // more efficient method just use a value_type[4] for temporary storage.
    value_type t[4];
    for(int col=0; col<4; ++col) {
        t[0] = INNER_PRODUCT( other, *this, 0, col );
        t[1] = INNER_PRODUCT( other, *this, 1, col );
        t[2] = INNER_PRODUCT( other, *this, 2, col );
        t[3] = INNER_PRODUCT( other, *this, 3, col );
        _mat[0][col] = t[0];
        _mat[1][col] = t[1];
        _mat[2][col] = t[2];
        _mat[3][col] = t[3];
    }

}

void Matrix_implementation::postMult( const Matrix_implementation& other )
{
    // brute force method requiring a copy
    //Matrix_implementation tmp(*this * other);
    // *this = tmp;

    // more efficient method just use a value_type[4] for temporary storage.
    value_type t[4];
    for(int row=0; row<4; ++row)
    {
        t[0] = INNER_PRODUCT( *this, other, row, 0 );
        t[1] = INNER_PRODUCT( *this, other, row, 1 );
        t[2] = INNER_PRODUCT( *this, other, row, 2 );
        t[3] = INNER_PRODUCT( *this, other, row, 3 );
        SET_ROW(row, t[0], t[1], t[2], t[3] )
    }
}

#undef INNER_PRODUCT

// orthoNormalize the 3x3 rotation matrix
void Matrix_implementation::orthoNormalize(const Matrix_implementation& rhs)
{
    value_type x_colMag = (rhs._mat[0][0] * rhs._mat[0][0]) + (rhs._mat[1][0] * rhs._mat[1][0]) + (rhs._mat[2][0] * rhs._mat[2][0]);
    value_type y_colMag = (rhs._mat[0][1] * rhs._mat[0][1]) + (rhs._mat[1][1] * rhs._mat[1][1]) + (rhs._mat[2][1] * rhs._mat[2][1]);
    value_type z_colMag = (rhs._mat[0][2] * rhs._mat[0][2]) + (rhs._mat[1][2] * rhs._mat[1][2]) + (rhs._mat[2][2] * rhs._mat[2][2]);

    if(!equivalent((double)x_colMag, 1.0) && !equivalent((double)x_colMag, 0.0))
    {
      x_colMag = sqrt(x_colMag);
      _mat[0][0] = rhs._mat[0][0] / x_colMag;
      _mat[1][0] = rhs._mat[1][0] / x_colMag;
      _mat[2][0] = rhs._mat[2][0] / x_colMag;
    }
    else
    {
      _mat[0][0] = rhs._mat[0][0];
      _mat[1][0] = rhs._mat[1][0];
      _mat[2][0] = rhs._mat[2][0];
    }

    if(!equivalent((double)y_colMag, 1.0) && !equivalent((double)y_colMag, 0.0))
    {
      y_colMag = sqrt(y_colMag);
      _mat[0][1] = rhs._mat[0][1] / y_colMag;
      _mat[1][1] = rhs._mat[1][1] / y_colMag;
      _mat[2][1] = rhs._mat[2][1] / y_colMag;
    }
    else
    {
      _mat[0][1] = rhs._mat[0][1];
      _mat[1][1] = rhs._mat[1][1];
      _mat[2][1] = rhs._mat[2][1];
    }

    if(!equivalent((double)z_colMag, 1.0) && !equivalent((double)z_colMag, 0.0))
    {
      z_colMag = sqrt(z_colMag);
      _mat[0][2] = rhs._mat[0][2] / z_colMag;
      _mat[1][2] = rhs._mat[1][2] / z_colMag;
      _mat[2][2] = rhs._mat[2][2] / z_colMag;
    }
    else
    {
      _mat[0][2] = rhs._mat[0][2];
      _mat[1][2] = rhs._mat[1][2];
      _mat[2][2] = rhs._mat[2][2];
    }

    _mat[3][0] = rhs._mat[3][0];
    _mat[3][1] = rhs._mat[3][1];
    _mat[3][2] = rhs._mat[3][2];

    _mat[0][3] = rhs._mat[0][3];
    _mat[1][3] = rhs._mat[1][3];
    _mat[2][3] = rhs._mat[2][3];
    _mat[3][3] = rhs._mat[3][3];

}

/******************************************
  Matrix inversion technique:
Given a matrix mat, we want to invert it.
mat = [ r00 r01 r02 a
        r10 r11 r12 b
        r20 r21 r22 c
        tx  ty  tz  d ]
We note that this matrix can be split into three matrices.
mat = rot * trans * corr, where rot is rotation part, trans is translation part, and corr is the correction due to perspective (if any).
rot = [ r00 r01 r02 0
        r10 r11 r12 0
        r20 r21 r22 0
        0   0   0   1 ]
trans = [ 1  0  0  0
          0  1  0  0
          0  0  1  0
          tx ty tz 1 ]
corr = [ 1 0 0 px
         0 1 0 py
         0 0 1 pz
         0 0 0 s ]
where the elements of corr are obtained from linear combinations of the elements of rot, trans, and mat.
So the inverse is mat' = (trans * corr)' * rot', where rot' must be computed the traditional way, which is easy since it is only a 3x3 matrix.
This problem is simplified if [px py pz s] = [0 0 0 1], which will happen if mat was composed only of rotations, scales, and translations (which is common).  In this case, we can ignore corr entirely which saves on a lot of computations.
******************************************/

bool Matrix_implementation::invert_4x3( const Matrix_implementation& mat )
{
    if (&mat==this)
    {
       Matrix_implementation tm(mat);
       return invert_4x3(tm);
    }

    register value_type r00, r01, r02,
                        r10, r11, r12,
                        r20, r21, r22;
      // Copy rotation components directly into registers for speed
    r00 = mat._mat[0][0]; r01 = mat._mat[0][1]; r02 = mat._mat[0][2];
    r10 = mat._mat[1][0]; r11 = mat._mat[1][1]; r12 = mat._mat[1][2];
    r20 = mat._mat[2][0]; r21 = mat._mat[2][1]; r22 = mat._mat[2][2];

        // Partially compute inverse of rot
    _mat[0][0] = r11*r22 - r12*r21;
    _mat[0][1] = r02*r21 - r01*r22;
    _mat[0][2] = r01*r12 - r02*r11;

      // Compute determinant of rot from 3 elements just computed
    register value_type one_over_det = 1.0/(r00*_mat[0][0] + r10*_mat[0][1] + r20*_mat[0][2]);
    r00 *= one_over_det; r10 *= one_over_det; r20 *= one_over_det;  // Saves on later computations

      // Finish computing inverse of rot
    _mat[0][0] *= one_over_det;
    _mat[0][1] *= one_over_det;
    _mat[0][2] *= one_over_det;
    _mat[0][3] = 0.0;
    _mat[1][0] = r12*r20 - r10*r22; // Have already been divided by det
    _mat[1][1] = r00*r22 - r02*r20; // same
    _mat[1][2] = r02*r10 - r00*r12; // same
    _mat[1][3] = 0.0;
    _mat[2][0] = r10*r21 - r11*r20; // Have already been divided by det
    _mat[2][1] = r01*r20 - r00*r21; // same
    _mat[2][2] = r00*r11 - r01*r10; // same
    _mat[2][3] = 0.0;
    _mat[3][3] = 1.0;

// We no longer need the rxx or det variables anymore, so we can reuse them for whatever we want.  But we will still rename them for the sake of clarity.

#define d r22
    d  = mat._mat[3][3];

    if( osg::square(d-1.0) > 1.0e-6 )  // Involves perspective, so we must
    {                       // compute the full inverse

        Matrix_implementation TPinv;
        _mat[3][0] = _mat[3][1] = _mat[3][2] = 0.0;

#define px r00
#define py r01
#define pz r02
#define one_over_s  one_over_det
#define a  r10
#define b  r11
#define c  r12

        a  = mat._mat[0][3]; b  = mat._mat[1][3]; c  = mat._mat[2][3];
        px = _mat[0][0]*a + _mat[0][1]*b + _mat[0][2]*c;
        py = _mat[1][0]*a + _mat[1][1]*b + _mat[1][2]*c;
        pz = _mat[2][0]*a + _mat[2][1]*b + _mat[2][2]*c;

#undef a
#undef b
#undef c
#define tx r10
#define ty r11
#define tz r12

        tx = mat._mat[3][0]; ty = mat._mat[3][1]; tz = mat._mat[3][2];
        one_over_s  = 1.0/(d - (tx*px + ty*py + tz*pz));

        tx *= one_over_s; ty *= one_over_s; tz *= one_over_s;  // Reduces number of calculations later on

        // Compute inverse of trans*corr
        TPinv._mat[0][0] = tx*px + 1.0;
        TPinv._mat[0][1] = ty*px;
        TPinv._mat[0][2] = tz*px;
        TPinv._mat[0][3] = -px * one_over_s;
        TPinv._mat[1][0] = tx*py;
        TPinv._mat[1][1] = ty*py + 1.0;
        TPinv._mat[1][2] = tz*py;
        TPinv._mat[1][3] = -py * one_over_s;
        TPinv._mat[2][0] = tx*pz;
        TPinv._mat[2][1] = ty*pz;
        TPinv._mat[2][2] = tz*pz + 1.0;
        TPinv._mat[2][3] = -pz * one_over_s;
        TPinv._mat[3][0] = -tx;
        TPinv._mat[3][1] = -ty;
        TPinv._mat[3][2] = -tz;
        TPinv._mat[3][3] = one_over_s;

        preMult(TPinv); // Finish computing full inverse of mat

#undef px
#undef py
#undef pz
#undef one_over_s
#undef d
    }
    else // Rightmost column is [0; 0; 0; 1] so it can be ignored
    {
        tx = mat._mat[3][0]; ty = mat._mat[3][1]; tz = mat._mat[3][2];

        // Compute translation components of mat'
        _mat[3][0] = -(tx*_mat[0][0] + ty*_mat[1][0] + tz*_mat[2][0]);
        _mat[3][1] = -(tx*_mat[0][1] + ty*_mat[1][1] + tz*_mat[2][1]);
        _mat[3][2] = -(tx*_mat[0][2] + ty*_mat[1][2] + tz*_mat[2][2]);

#undef tx
#undef ty
#undef tz
    }

    return true;
}


template <class T>
inline T SGL_ABS(T a)
{
   return (a >= 0 ? a : -a);
}

#ifndef SGL_SWAP
#define SGL_SWAP(a,b,temp) ((temp)=(a),(a)=(b),(b)=(temp))
#endif

bool Matrix_implementation::invert_4x4( const Matrix_implementation& mat )
{
    if (&mat==this) {
       Matrix_implementation tm(mat);
       return invert_4x4(tm);
    }

    unsigned int indxc[4], indxr[4], ipiv[4];
    unsigned int i,j,k,l,ll;
    unsigned int icol = 0;
    unsigned int irow = 0;
    double temp, pivinv, dum, big;

    // copy in place this may be unnecessary
    *this = mat;

    for (j=0; j<4; j++) ipiv[j]=0;

    for(i=0;i<4;i++)
    {
       big=0.0;
       for (j=0; j<4; j++)
          if (ipiv[j] != 1)
             for (k=0; k<4; k++)
             {
                if (ipiv[k] == 0)
                {
                   if (SGL_ABS(operator()(j,k)) >= big)
                   {
                      big = SGL_ABS(operator()(j,k));
                      irow=j;
                      icol=k;
                   }
                }
                else if (ipiv[k] > 1)
                   return false;
             }
       ++(ipiv[icol]);
       if (irow != icol)
          for (l=0; l<4; l++) SGL_SWAP(operator()(irow,l),
                                       operator()(icol,l),
                                       temp);

       indxr[i]=irow;
       indxc[i]=icol;
       if (operator()(icol,icol) == 0)
          return false;

       pivinv = 1.0/operator()(icol,icol);
       operator()(icol,icol) = 1;
       for (l=0; l<4; l++) operator()(icol,l) *= pivinv;
       for (ll=0; ll<4; ll++)
          if (ll != icol)
          {
             dum=operator()(ll,icol);
             operator()(ll,icol) = 0;
             for (l=0; l<4; l++) operator()(ll,l) -= operator()(icol,l)*dum;
          }
    }
    for (int lx=4; lx>0; --lx)
    {
       if (indxr[lx-1] != indxc[lx-1])
          for (k=0; k<4; k++) SGL_SWAP(operator()(k,indxr[lx-1]),
                                       operator()(k,indxc[lx-1]),temp);
    }

    return true;
}

void Matrix_implementation::makeOrtho(double left, double right,
                       double bottom, double top,
                       double zNear, double zFar)
{
    // note transpose of Matrix_implementation wr.t OpenGL documentation, since the OSG use post multiplication rather than pre.
    double tx = -(right+left)/(right-left);
    double ty = -(top+bottom)/(top-bottom);
    double tz = -(zFar+zNear)/(zFar-zNear);
    SET_ROW(0, 2.0/(right-left),               0.0,               0.0, 0.0 )
    SET_ROW(1,              0.0,  2.0/(top-bottom),               0.0, 0.0 )
    SET_ROW(2,              0.0,               0.0,  -2.0/(zFar-zNear), 0.0 )
    SET_ROW(3,               tx,                ty,                 tz, 1.0 )
}

bool Matrix_implementation::getOrtho(Matrix_implementation::value_type& left, Matrix_implementation::value_type& right,
                                     Matrix_implementation::value_type& bottom, Matrix_implementation::value_type& top,
                                     Matrix_implementation::value_type& zNear, Matrix_implementation::value_type& zFar) const
{
    if (_mat[0][3]!=0.0 || _mat[1][3]!=0.0 || _mat[2][3]!=0.0 || _mat[3][3]!=1.0) return false;

    zNear = (_mat[3][2]+1.0) / _mat[2][2];
    zFar = (_mat[3][2]-1.0) / _mat[2][2];

    left = -(1.0+_mat[3][0]) / _mat[0][0];
    right = (1.0-_mat[3][0]) / _mat[0][0];

    bottom = -(1.0+_mat[3][1]) / _mat[1][1];
    top = (1.0-_mat[3][1]) / _mat[1][1];

    return true;
}

bool Matrix_implementation::getOrtho(Matrix_implementation::other_value_type& left, Matrix_implementation::other_value_type& right,
                                     Matrix_implementation::other_value_type& bottom, Matrix_implementation::other_value_type& top,
                                     Matrix_implementation::other_value_type& zNear, Matrix_implementation::other_value_type& zFar) const
{
    Matrix_implementation::value_type temp_left, temp_right, temp_bottom, temp_top, temp_zNear, temp_zFar;
    if (getOrtho(temp_left, temp_right, temp_bottom, temp_top, temp_zNear, temp_zFar))
    {
        left = temp_left;
        right = temp_right;
        bottom = temp_bottom;
        top = temp_top;
        zNear = temp_zNear;
        zFar = temp_zFar;
        return true;
    }
    else
    {
        return false;
    }
}

void Matrix_implementation::makeFrustum(double left, double right,
                         double bottom, double top,
                         double zNear, double zFar)
{
    // note transpose of Matrix_implementation wr.t OpenGL documentation, since the OSG use post multiplication rather than pre.
    double A = (right+left)/(right-left);
    double B = (top+bottom)/(top-bottom);
    double C = (fabs(zFar)>DBL_MAX) ? -1. : -(zFar+zNear)/(zFar-zNear);
    double D = (fabs(zFar)>DBL_MAX) ? -2.*zNear : -2.0*zFar*zNear/(zFar-zNear);
    SET_ROW(0, 2.0*zNear/(right-left),                    0.0, 0.0,  0.0 )
    SET_ROW(1,                    0.0, 2.0*zNear/(top-bottom), 0.0,  0.0 )
    SET_ROW(2,                      A,                      B,   C, -1.0 )
    SET_ROW(3,                    0.0,                    0.0,   D,  0.0 )
}

bool Matrix_implementation::getFrustum(Matrix_implementation::value_type& left, Matrix_implementation::value_type& right,
                                       Matrix_implementation::value_type& bottom, Matrix_implementation::value_type& top,
                                       Matrix_implementation::value_type& zNear, Matrix_implementation::value_type& zFar) const
{
    if (_mat[0][3]!=0.0 || _mat[1][3]!=0.0 || _mat[2][3]!=-1.0 || _mat[3][3]!=0.0)
        return false;

    // note: near and far must be used inside this method instead of zNear and zFar
    // because zNear and zFar are references and they may point to the same variable.
    Matrix_implementation::value_type temp_near = _mat[3][2] / (_mat[2][2]-1.0);
    Matrix_implementation::value_type temp_far = _mat[3][2] / (1.0+_mat[2][2]);

    left = temp_near * (_mat[2][0]-1.0) / _mat[0][0];
    right = temp_near * (1.0+_mat[2][0]) / _mat[0][0];

    top = temp_near * (1.0+_mat[2][1]) / _mat[1][1];
    bottom = temp_near * (_mat[2][1]-1.0) / _mat[1][1];

    zNear = temp_near;
    zFar = temp_far;

    return true;
}

bool Matrix_implementation::getFrustum(Matrix_implementation::other_value_type& left, Matrix_implementation::other_value_type& right,
                                       Matrix_implementation::other_value_type& bottom, Matrix_implementation::other_value_type& top,
                                       Matrix_implementation::other_value_type& zNear, Matrix_implementation::other_value_type& zFar) const
{
    Matrix_implementation::value_type temp_left, temp_right, temp_bottom, temp_top, temp_zNear, temp_zFar;
    if (getFrustum(temp_left, temp_right, temp_bottom, temp_top, temp_zNear, temp_zFar))
    {
        left = temp_left;
        right = temp_right;
        bottom = temp_bottom;
        top = temp_top;
        zNear = temp_zNear;
        zFar = temp_zFar;
        return true;
    }
    else
    {
        return false;
    }
}

void Matrix_implementation::makePerspective(double fovy,double aspectRatio,
                                            double zNear, double zFar)
{
    // calculate the appropriate left, right etc.
    double tan_fovy = tan(DegreesToRadians(fovy*0.5));
    double right  =  tan_fovy * aspectRatio * zNear;
    double left   = -right;
    double top    =  tan_fovy * zNear;
    double bottom =  -top;
    makeFrustum(left,right,bottom,top,zNear,zFar);
}

bool Matrix_implementation::getPerspective(Matrix_implementation::value_type& fovy, Matrix_implementation::value_type& aspectRatio,
                                           Matrix_implementation::value_type& zNear, Matrix_implementation::value_type& zFar) const
{
    Matrix_implementation::value_type right  =  0.0;
    Matrix_implementation::value_type left   =  0.0;
    Matrix_implementation::value_type top    =  0.0;
    Matrix_implementation::value_type bottom =  0.0;

    // note: near and far must be used inside this method instead of zNear and zFar
    // because zNear and zFar are references and they may point to the same variable.
    Matrix_implementation::value_type temp_near   =  0.0;
    Matrix_implementation::value_type temp_far    =  0.0;

    // get frustum and compute results
    bool r = getFrustum(left,right,bottom,top,temp_near,temp_far);
    if (r)
    {
        fovy = RadiansToDegrees(atan(top/temp_near)-atan(bottom/temp_near));
        aspectRatio = (right-left)/(top-bottom);
    }
    zNear = temp_near;
    zFar = temp_far;
    return r;
}

bool Matrix_implementation::getPerspective(Matrix_implementation::other_value_type& fovy, Matrix_implementation::other_value_type& aspectRatio,
                                           Matrix_implementation::other_value_type& zNear, Matrix_implementation::other_value_type& zFar) const
{
    Matrix_implementation::value_type temp_fovy, temp_aspectRatio, temp_zNear, temp_zFar;
    if (getPerspective(temp_fovy, temp_aspectRatio, temp_zNear, temp_zFar))
    {
        fovy = temp_fovy;
        aspectRatio = temp_aspectRatio;
        zNear = temp_zNear;
        zFar = temp_zFar;
        return true;
    }
    else
    {
        return false;
    }
}

void Matrix_implementation::makeLookAt(const Vec3d& eye,const Vec3d& center,const Vec3d& up)
{
    Vec3d f(center-eye);
    f.normalize();
    Vec3d s(f^up);
    s.normalize();
    Vec3d u(s^f);
    u.normalize();

    set(
        s[0],     u[0],     -f[0],     0.0,
        s[1],     u[1],     -f[1],     0.0,
        s[2],     u[2],     -f[2],     0.0,
        0.0,     0.0,     0.0,      1.0);

    preMultTranslate(-eye);
}


void Matrix_implementation::getLookAt(Vec3f& eye,Vec3f& center,Vec3f& up,value_type lookDistance) const
{
    Matrix_implementation inv;
    inv.invert(*this);

    // note: e and c variables must be used inside this method instead of eye and center
    // because eye and center are references and they may point to the same variable.
    Vec3f e = osg::Vec3f(0.0,0.0,0.0)*inv;
    up = transform3x3(*this,osg::Vec3f(0.0,1.0,0.0));
    Vec3f c = transform3x3(*this,osg::Vec3f(0.0,0.0,-1));
    c.normalize();
    c = e + c*lookDistance;

    // assign the results
    eye = e;
    center = c;
}

void Matrix_implementation::getLookAt(Vec3d& eye,Vec3d& center,Vec3d& up,value_type lookDistance) const
{
    Matrix_implementation inv;
    inv.invert(*this);

    // note: e and c variables must be used inside this method instead of eye and center
    // because eye and center are references and they may point to the same variable.
    Vec3d e = osg::Vec3d(0.0,0.0,0.0)*inv;
    up = transform3x3(*this,osg::Vec3d(0.0,1.0,0.0));
    Vec3d c = transform3x3(*this,osg::Vec3d(0.0,0.0,-1));
    c.normalize();
    c = e + c*lookDistance;

    // assign the results
    eye = e;
    center = c;
}

#undef SET_ROW
