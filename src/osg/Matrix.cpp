#include <osg/Matrix>
#include <osg/Quat>
#include <osg/Notify>
#include <osg/Types>
#include <osg/Math>

#include <stdlib.h>

using namespace osg;


#define DEG2RAD(x)    ((x)*M_PI/180.0)
#define RAD2DEG(x)    ((x)*180.0/M_PI)


// temporary #define's for warning that deprecated methods are being
// used which should be replaced by the new variants.
#define WARN_DEPRECATED
//#define ABORT_DEPRECATED

#ifdef WARN_DEPRECATED
    #ifdef ABORT_DEPRECATED

        #define DEPRECATED(message) \
                notify(NOTICE) << message<<endl; \
                abort();
    #else
        #define DEPRECATED(message) \
                notify(NOTICE) << message<<endl;
    #endif
#else
    #define DEPRECATED(message)
#endif        


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


Matrix::Matrix() : Object(), fully_realized(false) {}

Matrix::Matrix( const Matrix& other ) : Object()
{
    set( (const float *) other._mat );
}

Matrix::Matrix( const float * def )
{    
    set( def ); 
}

Matrix::Matrix( float a00, float a01, float a02, float a03,
                float a10, float a11, float a12, float a13,
                float a20, float a21, float a22, float a23,
                float a30, float a31, float a32, float a33)
{
    SET_ROW(0, a00, a01, a02, a03 )
    SET_ROW(1, a10, a11, a12, a13 )
    SET_ROW(2, a20, a21, a22, a23 )
    SET_ROW(3, a30, a31, a32, a33 )

    fully_realized = true;
}


Matrix& Matrix::operator = (const Matrix& other ) {
    if( &other == this ) return *this;
    set((const float*)other._mat);
    return *this;
}

void Matrix::set( float const * const def ) {
    memcpy( _mat, def, sizeof(_mat) );
    fully_realized = true;
}


void Matrix::set( float a00, float a01, float a02, float a03,
                  float a10, float a11, float a12, float a13,
                  float a20, float a21, float a22, float a23,
                  float a30, float a31, float a32, float a33)
{
    SET_ROW(0, a00, a01, a02, a03 )
    SET_ROW(1, a10, a11, a12, a13 )
    SET_ROW(2, a20, a21, a22, a23 )
    SET_ROW(3, a30, a31, a32, a33 )

    fully_realized = true;
}

void Matrix::setTrans( float tx, float ty, float tz )
{
#ifdef WARN_DEPRECATED
    notify(NOTICE) << "Matrix::setTrans is deprecated."<<endl;
#endif
    ensureRealized();

    _mat[3][0] = tx;
    _mat[3][1] = ty;
    _mat[3][2] = tz;
}


void Matrix::setTrans( const Vec3& v )
{
#ifdef WARN_DEPRECATED
    notify(NOTICE) << "Matrix::setTrans is deprecated."<<endl;
#endif
    _mat[3][0] = v[0];
    _mat[3][1] = v[1];
    _mat[3][2] = v[2];
}

void Matrix::makeIdent()
{
    SET_ROW(0,    1, 0, 0, 0 )
    SET_ROW(1,    0, 1, 0, 0 )
    SET_ROW(2,    0, 0, 1, 0 )
    SET_ROW(3,    0, 0, 0, 1 )
    
    fully_realized = true;
}

void Matrix::makeScale( const Vec3& v )
{
    makeScale(v[0], v[1], v[2] );
}

void Matrix::makeScale( float x, float y, float z )
{
    SET_ROW(0,    x, 0, 0, 0 )
    SET_ROW(1,    0, y, 0, 0 )
    SET_ROW(2,    0, 0, z, 0 )
    SET_ROW(3,    0, 0, 0, 1 )

    fully_realized = true;
}

void Matrix::makeTrans( const Vec3& v )
{
    makeTrans( v[0], v[1], v[2] );
}

void Matrix::makeTrans( float x, float y, float z )
{
    SET_ROW(0,    1, 0, 0, 0 )
    SET_ROW(1,    0, 1, 0, 0 )
    SET_ROW(2,    0, 0, 1, 0 )
    SET_ROW(3,    x, y, z, 1 )

    fully_realized = true;
}

void Matrix::makeRot( const Vec3& from, const Vec3& to )
{
    double d = from * to; // dot product == cos( angle between from & to )
    if( d < 0.9999 ) {
        double angle = acos(d);
        Vec3 axis = to ^ from; //we know ((to) x (from)) is perpendicular to both
        makeRot( inRadians(angle) , axis );
    }        
    else 
        makeIdent();
}

void Matrix::makeRot( float angle, const Vec3& axis )
{
    makeRot( angle, axis.x(), axis.y(), axis.z() );
}

void Matrix::makeRot( float angle, float x, float y, float z ) {
    float d = sqrt( x*x + y*y + z*z );
    if( d == 0 )
        return;

#ifdef USE_DEGREES_INTERNALLY
    angle = DEG2RAD(angle);
#endif

#if 0
    float sin_half = sin( angle/2 );
    float cos_half = cos( angle/2 );

    Quat q( sin_half * (x/d),
            sin_half * (y/d),
            sin_half * (z/d),
            cos_half );//NOTE: original used a private quaternion made of doubles
#endif
    Quat q;
    q.makeRot( angle, x, y, z);
    makeRot( q );        // but Quat stores the values in a Vec4 made of floats.  
}

void Matrix::makeRot( const Quat& q ) {
    // taken from Shoemake/ACM SIGGRAPH 89
    Vec4 v = q.asVec4();

    double xs = 2 * v.x(); //assume q is already normalized? assert?
    double ys = 2 * v.y(); // if not, xs = 2 * v.x() / d, ys = 2 * v.y() / d
    double zs = 2 * v.z(); // and zs = 2 * v.z() /d where d = v.length2()

    double xx = xs * v.x();
    double xy = ys * v.x();
    double xz = zs * v.x();
    double yy = ys * v.y();
    double yz = zs * v.y();
    double zz = zs * v.z();
    double wx = xs * v.w();
    double wy = ys * v.w();
    double wz = zs * v.w();

/*
 * This is inverted - Don Burns
    SET_ROW(0,    1.0-(yy+zz),    xy - wz,    xz + wy,    0.0 )
    SET_ROW(1,    xy + wz,        1.0-(xx+zz),yz - wx,    0.0 )
    SET_ROW(2,    xz - wy,        yz + wx,    1.0-(xx+yy),0.0 )
    SET_ROW(3,    0.0,            0.0,        0.0,        1.0 )
 */

    SET_ROW(0,    1.0-(yy+zz),    xy + wz,    xz - wy,    0.0 )
    SET_ROW(1,    xy - wz,        1.0-(xx+zz),yz + wx,    0.0 )
    SET_ROW(2,    xz + wy,        yz - wx,    1.0-(xx+yy),0.0 )
    SET_ROW(3,    0.0,            0.0,        0.0,        1.0 )

    fully_realized = true;
}

void Matrix::makeRot( float yaw, float pitch, float roll)
{
#ifdef USE_DEGREES_INTERNALLY
    yaw = DEG2RAD(yaw);
    pitch = DEG2RAD(pitch);
    roll = DEG2RAD(roll);
#endif

    // lifted straight from SOLID library v1.01 Quaternion.h
    // available from http://www.win.tue.nl/~gino/solid/
    // and also distributed under the LGPL
    float cosYaw = cos(yaw / 2);
    float sinYaw = sin(yaw / 2);
    float cosPitch = cos(pitch / 2);
    float sinPitch = sin(pitch / 2);
    float cosRoll = cos(roll / 2);
    float sinRoll = sin(roll / 2);
    Quat q(sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw,
            cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw,
            cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw,
            cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw);
    makeRot( q );
}

void Matrix::mult( const Matrix& lhs, const Matrix& rhs )
{
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
    fully_realized = true;
}

void Matrix::preMult( const Matrix& other )
{
    if( !fully_realized ) {
        //act as if this were an identity Matrix
        set((const float*)other._mat);
        return;
    }

    // brute force method requiring a copy
    //Matrix tmp(other* *this);
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

void Matrix::postMult( const Matrix& other )
{
    if( !fully_realized ) {
        //act as if this were an identity Matrix
        set((const float*)other._mat);
        return;
    }
    // brute force method requiring a copy
    //Matrix tmp(*this * other);
    // *this = tmp;

    // more efficient method just use a float[4] for temporary storage.
    float t[4];
    for(int row=0; row<4; ++row)
    {
        t[0] = INNER_PRODUCT( *this, other, row, 0 );
        t[1] = INNER_PRODUCT( *this, other, row, 1 );
        t[2] = INNER_PRODUCT( *this, other, row, 2 );
        t[3] = INNER_PRODUCT( *this, other, row, 3 );
        SET_ROW(row, t[0], t[1], t[2], t[3] )
    }
}

#undef SET_ROW
#undef INNER_PRODUCT

bool Matrix::invert( const Matrix& _m )
{

    if (&_m==this) 
    {
        Matrix tm(_m);
        return invert(tm);
    }
    /*if ( _m._mat[0][3] == 0.0 
        && _m._mat[1][3] == 0.0 
        && _m._mat[2][3] == 0.0 
        && _m._mat[3][3] == 1.0 )
    {
        return invertAffine( _m );
    }*/

    // code lifted from VR Juggler.
    // not cleanly added, but seems to work.  RO.
    const float*  a = reinterpret_cast<const float*>(_m._mat);
    float*  b = reinterpret_cast<float*>(_mat);

    int     n = 4;
    int     i, j, k;
    int     r[ 4], c[ 4], row[ 4], col[ 4];
    float  m[ 4][ 4*2], pivot, max_m, tmp_m, fac;

    /* Initialization */
    for ( i = 0; i < n; i ++ )
    {
        r[ i] = c[ i] = 0;
        row[ i] = col[ i] = 0;
    }

    /* Set working Matrix */
    for ( i = 0; i < n; i++ )
    {
        for ( j = 0; j < n; j++ )
        {
            m[ i][ j] = a[ i * n + j];
            m[ i][ j + n] = ( i == j ) ? 1.0 : 0.0 ;
        }
    }

    /* Begin of loop */
    for ( k = 0; k < n; k++ )
    {
        /* Choosing the pivot */
        for ( i = 0, max_m = 0; i < n; i++ )
        {
            if ( row[ i]  ) continue;
            for ( j = 0; j < n; j++ )
            {
                if ( col[ j] ) continue;
                tmp_m = fabs( m[ i][j]);
                if ( tmp_m > max_m)
                {
                    max_m = tmp_m;
                    r[ k] = i;
                    c[ k] = j;
                }
            }
        }
        row[ r[k] ] = col[ c[k] ] = 1;
        pivot = m[ r[ k] ][ c[ k] ];

        if ( fabs( pivot) <= 1e-20)
        {
            notify(WARN) << "*** pivot = %f in mat_inv. ***"<<endl;
            //abort( 0);
            return false;
        }

        /* Normalization */
        for ( j = 0; j < 2*n; j++ )
        {
            if ( j == c[ k] )
                m[ r[ k]][ j] = 1.0;
            else
                m[ r[ k]][ j] /=pivot;
        }

        /* Reduction */
        for ( i = 0; i < n; i++ )
        {
            if ( i == r[ k] )
                continue;

            for ( j=0, fac = m[ i][ c[k]];j < 2*n; j++ )
            {
                if ( j == c[ k] )
                    m[ i][ j] =0.0;
                else
                    m[ i][ j] -=fac * m[ r[k]][ j];
            }
        }
    }

    /* Assign invers to a Matrix */
    for ( i = 0; i < n; i++ )
        for ( j = 0; j < n; j++ )
            row[ i] = ( c[ j] == i ) ? r[j] : row[ i];

    for ( i = 0; i < n; i++ )
        for ( j = 0; j < n; j++ )
            b[ i * n +  j] = m[ row[ i]][j + n];

    fully_realized = true;

    return true;                 // It worked
}

const double PRECISION_LIMIT = 1.0e-15;

bool Matrix::invertAffine( const Matrix& _m )
{
    // adapted from Graphics Gems II.
    // 
    // This method treats the Matrix as a block Matrix and calculates
    // the inverse of one subMatrix, improving performance over something
    // that inverts any non-singular Matrix:
    //                           -1
    //    -1    [ A  0 ] -1    [ A    0 ]
    //   M   =  [      ]    =  [   -1   ]
    //          [ C  1 ]       [-CA   1 ]
    //
    // returns true if _m is nonsingular, and (*this) contains its inverse
    // otherwise returns false. (*this unchanged)

    // assert( this->isAffine())?
    double det_1, pos, neg, temp;

    pos = neg = 0.0;

#define ACCUMULATE \
    { \
        if(temp >= 0.0) pos += temp; \
        else neg += temp; \
    }

    temp = _m._mat[0][0] * _m._mat[1][1] * _m._mat[2][2]; ACCUMULATE;
    temp = _m._mat[0][1] * _m._mat[1][2] * _m._mat[2][0]; ACCUMULATE;
    temp = _m._mat[0][2] * _m._mat[1][0] * _m._mat[2][1]; ACCUMULATE;

    temp = - _m._mat[0][2] * _m._mat[1][1] * _m._mat[2][0]; ACCUMULATE;
    temp = - _m._mat[0][1] * _m._mat[1][0] * _m._mat[2][2]; ACCUMULATE;
    temp = - _m._mat[0][0] * _m._mat[1][2] * _m._mat[2][1]; ACCUMULATE;

    det_1 = pos + neg;

    if( (det_1 == 0.0) || (fabs(det_1/(pos-neg)) < PRECISION_LIMIT )) {
        // _m has no inverse
        notify(WARN) << "Matrix::invert(): Matrix has no inverse." << endl;
        return false;
    }

    // inverse is adj(A)/det(A)
    det_1 = 1.0 / det_1;

    _mat[0][0] = (_m._mat[1][1] * _m._mat[2][2] - _m._mat[1][2] * _m._mat[2][1]) * det_1;
    _mat[1][0] = (_m._mat[1][0] * _m._mat[2][2] - _m._mat[1][2] * _m._mat[2][0]) * det_1;
    _mat[2][0] = (_m._mat[1][0] * _m._mat[2][1] - _m._mat[1][1] * _m._mat[2][0]) * det_1;
    _mat[0][1] = (_m._mat[0][1] * _m._mat[2][2] - _m._mat[0][2] * _m._mat[2][1]) * det_1;
    _mat[1][1] = (_m._mat[0][0] * _m._mat[2][2] - _m._mat[0][2] * _m._mat[2][0]) * det_1;
    _mat[2][1] = (_m._mat[0][0] * _m._mat[2][1] - _m._mat[0][1] * _m._mat[2][0]) * det_1;
    _mat[0][2] = (_m._mat[0][1] * _m._mat[1][2] - _m._mat[0][2] * _m._mat[1][1]) * det_1;
    _mat[1][2] = (_m._mat[0][0] * _m._mat[1][2] - _m._mat[0][2] * _m._mat[1][0]) * det_1;
    _mat[2][2] = (_m._mat[0][0] * _m._mat[1][1] - _m._mat[0][1] * _m._mat[1][0]) * det_1;

    // calculate -C * inv(A)
    _mat[3][0] = -(_m._mat[3][0] * _mat[0][0] + _m._mat[3][1] * _mat[1][0] + _m._mat[3][2] * _mat[2][0] );
    _mat[3][1] = -(_m._mat[3][0] * _mat[0][1] + _m._mat[3][1] * _mat[1][1] + _m._mat[3][2] * _mat[2][1] );
    _mat[3][2] = -(_m._mat[3][0] * _mat[0][2] + _m._mat[3][1] * _mat[1][2] + _m._mat[3][2] * _mat[2][2] );

    _mat[0][3] = 0.0;
    _mat[1][3] = 0.0;
    _mat[2][3] = 0.0;
    _mat[3][3] = 1.0;

    fully_realized = true;
    return true;
}

#ifdef USE_DEPRECATED_MATRIX_METHODS

//Deprecated methods 
void Matrix::copy( const Matrix& other)
{
    DEPRECATED("Matrix::copy is deprecated.  Use = instead.")

    (*this) = other;
}
void Matrix::preScale( float sx, float sy, float sz, const Matrix& m )
{
    DEPRECATED("Matrix::preScale is deprecated. Use result = (Matrix::scale * m) instead.")

    (*this) = ( scale(sx,sy,sz) * m );
}
void Matrix::postScale( const Matrix& m, float sx, float sy, float sz )
{
    DEPRECATED("Matrix::postScale is deprecated.  Use result = (m * Matrix::scale()) instead.")

    (*this) = ( m * scale(sx,sy,sz) );
}
void Matrix::preScale( float sx, float sy, float sz )
{
    DEPRECATED("Matrix::preScale is deprecated. Use M.preMult( Matrix::scale ) instead.")

    preMult( scale(sx,sy,sz) );
}
void Matrix::postScale( float sx, float sy, float sz )
{
    DEPRECATED("Matrix::postScale is deprecated.  Use M.postMult( Matrix::scale ) instead.")

    postMult( scale(sx,sy,sz) );
}
void Matrix::preTrans( float tx, float ty, float tz, const Matrix& m )
{
    DEPRECATED("Matrix::preTrans is deprecated.  Use result = Matrix::trans * m instead.")

    (*this) = trans(tx,ty,tz) * m;
}
void Matrix::postTrans( const Matrix& m, float tx, float ty, float tz )
{
    DEPRECATED("Matrix::postTrans is deprecated.  Use result = m * Matrix::trans instead.")

    (*this) = m * trans(tx,ty,tz);
}

void Matrix::preTrans( float tx, float ty, float tz )
{
    DEPRECATED("Matrix::preTrans is deprecated.  Use result = Matrix::trans * m instead.")

    preMult( trans(tx,ty,tz) );
}
void Matrix::postTrans( float sx, float sy, float sz )
{
    DEPRECATED("Matrix::postTrans is deprecated.  Use result = m * Matrix::trans instead.")

    postMult( trans(sx,sy,sz) );
}
void Matrix::preRot( float deg, float x, float y, float z, const Matrix& m  )
{
    DEPRECATED("Matrix::preRot is deprecated.  Use result = Matrix::rot * m instead.")

    (*this) = rotate(deg,x,y,z) * m;
}
void Matrix::postRot( const Matrix& m, float deg, float x, float y, float z )
{
    DEPRECATED("Matrix::postRot is deprecated.  Use result = m * Matrix::rotate instead.")

    (*this) = m * rotate(deg,x,y,z);
}

void Matrix::preRot( float deg, float x, float y, float z )
{
    DEPRECATED("Matrix::preRot is deprecated.  Use m.preMult( Matrix::rotate ) instead.")

    preMult( rotate(deg,x,y,z) );
}
void Matrix::postRot( float deg, float x, float y, float z )
{
    DEPRECATED("Matrix::postRot is deprecated.  Use m.postMult( Matrix::rotate ) instead.")

    postMult( rotate(deg,x,y,z) );
}
#endif
