/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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

#include <osg/GL>

#include <stdlib.h>

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

#define QX  q._fv[0]
#define QY  q._fv[1]
#define QZ  q._fv[2]
#define QW  q._fv[3]

void Matrix_implementation::set(const Quat& q)
{
    // Source: Gamasutra, Rotating Objects Using Quaternions
    //
    //http://www.gamasutra.com/features/programming/19980703/quaternions_01.htm

    double wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

    // calculate coefficients
    x2 = QX + QX;
    y2 = QY + QY;
    z2 = QZ + QZ;

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
    _mat[0][0] = 1.0f - (yy + zz);
    _mat[1][0] = xy - wz;
    _mat[2][0] = xz + wy;
    _mat[3][0] = 0.0f;

    _mat[0][1] = xy + wz;
    _mat[1][1] = 1.0f - (xx + zz);
    _mat[2][1] = yz - wx;
    _mat[3][1] = 0.0f;

    _mat[0][2] = xz - wy;
    _mat[1][2] = yz + wx;
    _mat[2][2] = 1.0f - (xx + yy);
    _mat[3][2] = 0.0f;

    _mat[0][3] = 0;
    _mat[1][3] = 0;
    _mat[2][3] = 0;
    _mat[3][3] = 1;
}

void Matrix_implementation::get( Quat& q ) const
{
    // Source: Gamasutra, Rotating Objects Using Quaternions
    //
    //http://www.gamasutra.com/features/programming/19980703/quaternions_01.htm

    value_type  tr, s;
    value_type tq[4];
    int    i, j, k;

    int nxt[3] = {1, 2, 0};

    tr = _mat[0][0] + _mat[1][1] + _mat[2][2];

    // check the diagonal
    if (tr > 0.0)
    {
        s = (value_type)sqrt (tr + 1.0);
        QW = s / 2.0f;
        s = 0.5f / s;
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

        tq[i] = s * 0.5f;

        if (s != 0.0f)
            s = 0.5f / s;

        tq[3] = (_mat[j][k] - _mat[k][j]) * s;
        tq[j] = (_mat[i][j] + _mat[j][i]) * s;
        tq[k] = (_mat[i][k] + _mat[k][i]) * s;

        QX = tq[0];
        QY = tq[1];
        QZ = tq[2];
        QW = tq[3];
    }
}


void Matrix_implementation::setTrans( value_type tx, value_type ty, value_type tz )
{
    _mat[3][0] = tx;
    _mat[3][1] = ty;
    _mat[3][2] = tz;
}


void Matrix_implementation::setTrans( const Vec3& v )
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

void Matrix_implementation::makeScale( const Vec3& v )
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

void Matrix_implementation::makeTranslate( const Vec3& v )
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

void Matrix_implementation::makeRotate( const Vec3& from, const Vec3& to )
{
    Quat quat;
    quat.makeRotate(from,to);
    set(quat);
}

void Matrix_implementation::makeRotate( float angle, const Vec3& axis )
{
    Quat quat;
    quat.makeRotate( angle, axis);
    set(quat);
}

void Matrix_implementation::makeRotate( float angle, float x, float y, float z ) 
{
    Quat quat;
    quat.makeRotate( angle, x, y, z);
    set(quat);
}

void Matrix_implementation::makeRotate( const Quat& quat )
{
    set(quat);
}

void Matrix_implementation::makeRotate( float angle1, const Vec3& axis1, 
                         float angle2, const Vec3& axis2,
                         float angle3, const Vec3& axis3)
{
    Quat quat;
    quat.makeRotate(angle1, axis1, 
                    angle2, axis2,
                    angle3, axis3);
    set(quat);
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

    // more efficient method just use a float[4] for temporary storage.
    float t[4];
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

    // more efficient method just use a float[4] for temporary storage.
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


template <class T>
inline T SGL_ABS(T a)
{
   return (a >= 0 ? a : -a);
}

#ifndef SGL_SWAP
#define SGL_SWAP(a,b,temp) ((temp)=(a),(a)=(b),(b)=(temp))
#endif

bool Matrix_implementation::invert( const Matrix_implementation& mat )
{
    if (&mat==this) {
       Matrix_implementation tm(mat);
       return invert(tm);
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
       big=(float)0.0;
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
    SET_ROW(0, 2.0f/(right-left),              0.0f,               0.0f, 0.0f )
    SET_ROW(1,              0.0f, 2.0f/(top-bottom),               0.0f, 0.0f )
    SET_ROW(2,              0.0f,              0.0f, -2.0f/(zFar-zNear), 0.0f )
    SET_ROW(3,                tx,                ty,                 tz, 1.0f )
}

void Matrix_implementation::getOrtho(double& left, double& right,
                      double& bottom, double& top,
                      double& zNear, double& zFar)
{
    zNear = (_mat[3][2]+1.0f) / _mat[2][2];
    zFar = (_mat[3][2]-1.0f) / _mat[2][2];
    
    left = -(1.0f+_mat[3][0]) / _mat[0][0];
    right = (1.0f-_mat[3][0]) / _mat[0][0];

    bottom = -(1.0f+_mat[3][1]) / _mat[1][1];
    top = (1.0f-_mat[3][1]) / _mat[1][1];
}            


void Matrix_implementation::makeFrustum(double left, double right,
                         double bottom, double top,
                         double zNear, double zFar)
{
    // note transpose of Matrix_implementation wr.t OpenGL documentation, since the OSG use post multiplication rather than pre.
    double A = (right+left)/(right-left);
    double B = (top+bottom)/(top-bottom);
    double C = -(zFar+zNear)/(zFar-zNear);
    double D = -2.0*zFar*zNear/(zFar-zNear);
    SET_ROW(0, 2.0f*zNear/(right-left),                    0.0f, 0.0f,  0.0f )
    SET_ROW(1,                    0.0f, 2.0f*zNear/(top-bottom), 0.0f,  0.0f )
    SET_ROW(2,                       A,                       B,    C, -1.0f )
    SET_ROW(3,                    0.0f,                   0.0f,     D,  0.0f )
}

void Matrix_implementation::getFrustum(double& left, double& right,
                        double& bottom, double& top,
                        double& zNear, double& zFar)
{
    zNear = _mat[3][2] / (_mat[2][2]-1.0f);
    zFar = _mat[3][2] / (1.0f+_mat[2][2]);
    
    left = zNear * (_mat[2][0]-1.0f) / _mat[0][0];
    right = zNear * (1.0f+_mat[2][0]) / _mat[0][0];

    top = zNear * (1.0f+_mat[2][1]) / _mat[1][1];
    bottom = zNear * (_mat[2][1]-1.0f) / _mat[1][1];
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


void Matrix_implementation::makeLookAt(const Vec3& eye,const Vec3& center,const Vec3& up)
{
    Vec3 f(center-eye);
    f.normalize();
    Vec3 s(f^up);
    s.normalize();
    Vec3 u(s^f);
    u.normalize();

    set(
        s[0],     u[0],     -f[0],     0.0f,
        s[1],     u[1],     -f[1],     0.0f,
        s[2],     u[2],     -f[2],     0.0f,
        0.0f,     0.0f,     0.0f,      1.0f);

    preMult(Matrix_implementation::translate(-eye));
}

void Matrix_implementation::getLookAt(Vec3& eye,Vec3& center,Vec3& up,float lookDistance)
{
    Matrix_implementation inv;
    inv.invert(*this);
    eye = osg::Vec3(0.0f,0.0f,0.0f)*inv;
    up = transform3x3(*this,osg::Vec3(0.0f,1.0f,0.0f));
    center = transform3x3(*this,osg::Vec3(0.0f,0.0f,-1));
    center.normalize();
    center = eye + center*lookDistance;
}

#undef SET_ROW
