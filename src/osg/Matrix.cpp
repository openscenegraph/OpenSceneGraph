#include <math.h>
#include "osg/Matrix"
#include "osg/Input"
#include "osg/Output"
#include "osg/Notify"

#define square(x)   ((x)*(x))
#define DEG2RAD(x)  ((x)*M_PI/180.0)

using namespace osg;

typedef struct quaternion_
{
    double x ;
    double y ;
    double z ;
    double w ;
} quaternion ;

/* C = a(row).b(row) */

#define matrix_inner_product( a, b, row, col, C ) \
    { \
        (C)[row][col] = (a)[row][0] * (b)[0][col] + \
            (a)[row][1] * (b)[1][col] + \
            (a)[row][2] * (b)[2][col] + \
            (a)[row][3] * (b)[3][col]; \
    }

/* C = a.b */

#define matrix_mult( a, b, C ) \
    { \
        matrix_inner_product( a, b, 0, 0, C ); \
        matrix_inner_product( a, b, 0, 1, C ); \
        matrix_inner_product( a, b, 0, 2, C ); \
        matrix_inner_product( a, b, 0, 3, C ); \
        matrix_inner_product( a, b, 1, 0, C ); \
        matrix_inner_product( a, b, 1, 1, C ); \
        matrix_inner_product( a, b, 1, 2, C ); \
        matrix_inner_product( a, b, 1, 3, C ); \
        matrix_inner_product( a, b, 2, 0, C ); \
        matrix_inner_product( a, b, 2, 1, C ); \
        matrix_inner_product( a, b, 2, 2, C ); \
        matrix_inner_product( a, b, 2, 3, C ); \
        matrix_inner_product( a, b, 3, 0, C ); \
        matrix_inner_product( a, b, 3, 1, C ); \
        matrix_inner_product( a, b, 3, 2, C ); \
        matrix_inner_product( a, b, 3, 3, C ); \
    }

static void quaternion_matrix( quaternion *q, double mat[4][4] )
{
/* copied from Shoemake/ACM SIGGRAPH 89 */
    double xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz ;

    xs = q->x + q->x;
    ys = q->y + q->y;
    zs = q->z + q->z;

    wx = q->w * xs ; wy = q->w * ys ; wz = q->w * zs ;
    xx = q->x * xs ; xy = q->x * ys ; xz = q->x * zs ;
    yy = q->y * ys ; yz = q->y * zs ; zz = q->z * zs ;

    mat[0][0] = 1.0 - ( yy + zz ) ;
    mat[0][1] = xy - wz ;
    mat[0][2] = xz + wy ;
    mat[1][0] = xy + wz ;
    mat[1][1] = 1.0 - ( xx + zz ) ;
    mat[1][2] = yz - wx ;
    mat[2][0] = xz - wy ;
    mat[2][1] = yz + wx ;
    mat[2][2] = 1.0 - ( xx + yy ) ;

    mat[0][3] = 0.0;
    mat[1][3] = 0.0;
    mat[2][3] = 0.0;

    mat[3][0] = 0.0;
    mat[3][1] = 0.0;
    mat[3][2] = 0.0;
    mat[3][3] = 1.0;
}


Matrix::Matrix()
{
    makeIdent();
}


Matrix::Matrix(const Matrix& matrix) : Object()
{
    memcpy(_mat,matrix._mat,sizeof(_mat));
}


Matrix& Matrix::operator = (const Matrix& matrix)
{
    if (&matrix==this) return *this;
    memcpy(_mat,matrix._mat,sizeof(_mat));
    return *this;
}


Matrix::Matrix(
float a00, float a01, float a02, float a03,
float a10, float a11, float a12, float a13,
float a20, float a21, float a22, float a23,
float a30, float a31, float a32, float a33)
{
    _mat[0][0] = a00;
    _mat[0][1] = a01;
    _mat[0][2] = a02;
    _mat[0][3] = a03;

    _mat[1][0] = a10;
    _mat[1][1] = a11;
    _mat[1][2] = a12;
    _mat[1][3] = a13;

    _mat[2][0] = a20;
    _mat[2][1] = a21;
    _mat[2][2] = a22;
    _mat[2][3] = a23;

    _mat[3][0] = a30;
    _mat[3][1] = a31;
    _mat[3][2] = a32;
    _mat[3][3] = a33;
}


Matrix::~Matrix()
{
}


Matrix* Matrix::instance()
{
    static ref_ptr<Matrix> s_matrix(new Matrix());
    return s_matrix.get();
}


bool Matrix::readLocalData(Input& fr)
{
    bool iteratorAdvanced = false;
    bool matched = true;
    for(int k=0;k<16 && matched;++k)
    {
        matched = fr[k].isFloat();
    }
    if (matched)
    {
        int k=0;
        for(int i=0;i<4;++i)
        {
            for(int j=0;j<4;++j)
            {
                fr[k].getFloat(_mat[i][j]);
                k++;
            }
        }
        fr += 16;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool Matrix::writeLocalData(Output& fw)
{
    fw.indent() << _mat[0][0] << " " << _mat[0][1] << " " << _mat[0][2] << " " << _mat[0][3] << endl;
    fw.indent() << _mat[1][0] << " " << _mat[1][1] << " " << _mat[1][2] << " " << _mat[1][3] << endl;
    fw.indent() << _mat[2][0] << " " << _mat[2][1] << " " << _mat[2][2] << " " << _mat[2][3] << endl;
    fw.indent() << _mat[3][0] << " " << _mat[3][1] << " " << _mat[3][2] << " " << _mat[3][3] << endl;
    return true;
}


void Matrix::makeIdent()
{
    _mat[0][0] = 1.0f;
    _mat[0][1] = 0.0f;
    _mat[0][2] = 0.0f;
    _mat[0][3] = 0.0f;

    _mat[1][0] = 0.0f;
    _mat[1][1] = 1.0f;
    _mat[1][2] = 0.0f;
    _mat[1][3] = 0.0f;

    _mat[2][0] = 0.0f;
    _mat[2][1] = 0.0f;
    _mat[2][2] = 1.0f;
    _mat[2][3] = 0.0f;

    _mat[3][0] = 0.0f;
    _mat[3][1] = 0.0f;
    _mat[3][2] = 0.0f;
    _mat[3][3] = 1.0f;
}

void Matrix::copy(const Matrix& matrix)
{
    memcpy(_mat,matrix._mat,sizeof(_mat));
}

void Matrix::makeScale(float sx, float sy, float sz)
{
    makeIdent();
    _mat[0][0] = sx;
    _mat[1][1] = sy;
    _mat[2][2] = sz;
}


void Matrix::preScale( float sx, float sy, float sz, const Matrix& m )
{
    Matrix transMat;
    transMat.makeScale(sx, sy, sz);
    mult(transMat,m);
}

void Matrix::postScale( const Matrix& m, float sx, float sy, float sz )
{
    Matrix transMat;
    transMat.makeScale(sx, sy, sz);
    mult(m,transMat);
}

void Matrix::preScale( float sx, float sy, float sz )
{
    Matrix transMat;
    transMat.makeScale(sx, sy, sz);
    preMult(transMat);
}

void Matrix::postScale( float sx, float sy, float sz )
{
    Matrix transMat;
    transMat.makeScale(sx, sy, sz);
    postMult(transMat);
}




void Matrix::makeTrans( float tx, float ty, float tz )
{
    makeIdent();
    _mat[3][0] = tx;
    _mat[3][1] = ty;
    _mat[3][2] = tz;
}

void Matrix::preTrans( float tx, float ty, float tz, const Matrix& m )
{
    Matrix transMat;
    transMat.makeTrans(tx, ty, tz);
    mult(transMat,m);
}

void Matrix::postTrans( const Matrix& m, float tx, float ty, float tz )
{
    Matrix transMat;
    transMat.makeTrans(tx, ty, tz);
    mult(m,transMat);
}

void Matrix::preTrans( float tx, float ty, float tz )
{
    _mat[3][0] = (tx * _mat[0][0]) + (ty * _mat[1][0]) + (tz * _mat[2][0]) + _mat[3][0];
    _mat[3][1] = (tx * _mat[0][1]) + (ty * _mat[1][1]) + (tz * _mat[2][1]) + _mat[3][1];
    _mat[3][2] = (tx * _mat[0][2]) + (ty * _mat[1][2]) + (tz * _mat[2][2]) + _mat[3][2];
    _mat[3][3] = (tx * _mat[0][3]) + (ty * _mat[1][3]) + (tz * _mat[2][3]) + _mat[3][3];
}

void Matrix::postTrans( float tx, float ty, float tz )
{
    Matrix transMat;
    transMat.makeTrans(tx, ty, tz);
    postMult(transMat);
}

void Matrix::makeRot( float deg, float x, float y, float z )
{
    double __mat[4][4];
    quaternion q;
    float d = sqrtf( square(x) + square(y) + square(z) );

    if( d == 0 )
        return;

    float sin_HalfAngle = sinf( DEG2RAD(deg/2) );
    float cos_HalfAngle = cosf( DEG2RAD(deg/2) );

    q.x = sin_HalfAngle * (x/d);
    q.y = sin_HalfAngle * (y/d);
    q.z = sin_HalfAngle * (z/d);
    q.w = cos_HalfAngle;

    quaternion_matrix( &q, __mat );

    for(int i=0;i<4;++i)
    {
        for(int j=0;j<4;++j)
        {
            _mat[i][j]=__mat[i][j];
        }
    }
}

void Matrix::preRot( float deg, float x, float y, float z, const Matrix& m  )
{
    Matrix rotMat;
    rotMat.makeRot( deg, x, y, z );
    mult(rotMat,m);
}

void Matrix::postRot( const Matrix& m, float deg, float x, float y, float z )
{
    Matrix rotMat;
    rotMat.makeRot( deg, x, y, z );
    mult(m,rotMat);
}

void Matrix::preRot( float deg, float x, float y, float z )
{
    quaternion q;
    double __mat[4][4];
    float res_mat[4][4];

    float d = sqrtf( square(x) + square(y) + square(z) );

    if( d == 0 )
        return;

    float sin_HalfAngle = sinf( DEG2RAD(deg/2) );
    float cos_HalfAngle = cosf( DEG2RAD(deg/2) );

    q.x = sin_HalfAngle * (x/d);
    q.y = sin_HalfAngle * (y/d);
    q.z = sin_HalfAngle * (z/d);
    q.w = cos_HalfAngle;

    quaternion_matrix( &q, __mat );
    matrix_mult( __mat, _mat, res_mat );
    memcpy( _mat, res_mat, sizeof( _mat ) );
}

void Matrix::postRot( float deg, float x, float y, float z )
{
    quaternion q;
    double __mat[4][4];
    float res_mat[4][4];

    float d = sqrtf( square(x) + square(y) + square(z) );

    if( d == 0 )
        return;

    float sin_HalfAngle = sinf( DEG2RAD(deg/2) );
    float cos_HalfAngle = cosf( DEG2RAD(deg/2) );

    q.x = sin_HalfAngle * (x/d);
    q.y = sin_HalfAngle * (y/d);
    q.z = sin_HalfAngle * (z/d);
    q.w = cos_HalfAngle;

    quaternion_matrix( &q, __mat );
    matrix_mult( _mat, __mat , res_mat );
    memcpy( _mat, res_mat, sizeof( _mat ) );
}

void Matrix::setTrans( float tx, float ty, float tz )
{
    _mat[3][0] = tx;
    _mat[3][1] = ty;	
    _mat[3][2] = tz;
}
void Matrix::setTrans( const Vec3& v )
{
    _mat[3][0] = v[0];
    _mat[3][1] = v[1];
    _mat[3][2] = v[2];
}

void Matrix::preMult(const Matrix& m)
{
    Matrix tm;
    matrix_mult( m._mat, _mat, tm._mat );
    *this = tm;
}

void Matrix::postMult(const Matrix& m)
{
    Matrix tm;
    matrix_mult( _mat, m._mat, tm._mat );
    *this = tm;
}

void Matrix::mult(const Matrix& lhs,const Matrix& rhs)
{
    matrix_mult( lhs._mat, rhs._mat, _mat );
}

Matrix Matrix::operator * (const Matrix& m) const
{
    Matrix nm;
    matrix_mult( _mat,m._mat, nm._mat );
    return nm;
}


bool Matrix::invert(const Matrix& _m)
{
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

/* Set working matrix */
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
            notify(WARN) << "*** pivot = %f in mat_inv. ***\n";
//exit( 0);
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

/* Assign invers to a matrix */
    for ( i = 0; i < n; i++ )
        for ( j = 0; j < n; j++ )
            row[ i] = ( c[ j] == i ) ? r[j] : row[ i];

    for ( i = 0; i < n; i++ )
        for ( j = 0; j < n; j++ )
            b[ i * n +  j] = m[ row[ i]][j + n];

    return true;                                  // It worked
}
