#include <stdio.h>
#include <osg/Quat>
#include <osg/Vec4>
#include <osg/Vec3>
#include <osg/Types>

#include <math.h>

/// Good introductions to Quaternions at:
/// http://www.gamasutra.com/features/programming/19980703/quaternions_01.htm
/// http://mathworld.wolfram.com/Quaternion.html

using namespace osg;


/// Set the elements of the Quat to represent a rotation of angle
/// (radians) around the axis (x,y,z)
void Quat::makeRotate( const float angle,
const float x,
const float y,
const float z    )
{
    float inversenorm  = 1.0/sqrt( x*x + y*y + z*z );
    float coshalfangle = cos( 0.5*angle );
    float sinhalfangle = sin( 0.5*angle );

    _fv[0] = x * sinhalfangle * inversenorm;
    _fv[1] = y * sinhalfangle * inversenorm;
    _fv[2] = z * sinhalfangle * inversenorm;
    _fv[3] = coshalfangle;
}


void Quat::makeRotate( const float angle, const Vec3& vec )
{
    makeRotate( angle, vec[0], vec[1], vec[2] );
}


// Make a rotation Quat which will rotate vec1 to vec2
// Generally take adot product to get the angle between these
// and then use a cross product to get the rotation axis
// Watch out for the two special cases of when the vectors
// are co-incident or opposite in direction.
void Quat::makeRotate( const Vec3& from, const Vec3& to )
{
    const float epsilon = 0.00001f;

    float length1  = from.length();
    float length2  = to.length();
    
    // dot product vec1*vec2
    float cosangle = from*to/(length1*length2);

    if ( fabs(cosangle - 1) < epsilon )
    {
        // cosangle is close to 1, so the vectors are close to being coincident
        // Need to generate an angle of zero with any vector we like
        // We'll choose (1,0,0)
        makeRotate( 0.0, 1.0, 0.0, 0.0 );
    }
    else
    if ( fabs(cosangle + 1.0) < epsilon )
    {
        // vectors are close to being opposite, so will need to find a
        // vector orthongonal to from to rotate about.
        osg::Vec3 tmp;
        if (fabs(from.x())<fabs(from.y()))
            if (fabs(from.x())<fabs(from.z())) tmp.set(1.0,0.0,0.0); // use x axis.
            else tmp.set(0.0,0.0,1.0);
        else if (fabs(from.y())<fabs(from.z())) tmp.set(0.0,1.0,0.0);
        else tmp.set(0.0,0.0,1.0);
        
        // find orthogonal axis.
        Vec3 axis(from^tmp);
        axis.normalize();
        
        _fv[0] = axis[0]; // sin of half angle of PI is 1.0.
        _fv[1] = axis[1]; // sin of half angle of PI is 1.0.
        _fv[2] = axis[2]; // sin of half angle of PI is 1.0.
        _fv[3] = 0; // cos of half angle of PI is zero.

    }
    else
    {
        // This is the usual situation - take a cross-product of vec1 and vec2
        // and that is the axis around which to rotate.
        Vec3 axis(from^to);
        float angle = acos( cosangle );
        makeRotate( angle, axis );
    }
}


// Get the angle of rotation and axis of this Quat object.
// Won't give very meaningful results if the Quat is not associated
// with a rotation!
void Quat::getRotate( float& angle, Vec3& vec ) const
{
    float sinhalfangle = sqrt( _fv[0]*_fv[0] + _fv[1]*_fv[1] + _fv[2]*_fv[2] );
    /// float coshalfangle = _fv[3];

    /// These are not checked for performance reasons ? (cop out!)
    /// Point for  discussion - how do one handle errors in the osg?
    /// if ( abs(sinhalfangle) > 1.0 ) { error };
    /// if ( abs(coshalfangle) > 1.0 ) { error };

    // *angle = atan2( sinhalfangle, coshalfangle );	// see man atan2
                                 // -pi < angle < pi
    angle = 2 * atan2( sinhalfangle, _fv[3] );
    vec = Vec3(_fv[0], _fv[1], _fv[2]) / sinhalfangle;
}


void Quat::getRotate( float& angle, float& x, float& y, float& z ) const
{
    float sinhalfangle = sqrt( _fv[0]*_fv[0] + _fv[1]*_fv[1] + _fv[2]*_fv[2] );

    angle = 2 * atan2( sinhalfangle, _fv[3] );
    x = _fv[0] / sinhalfangle;
    y = _fv[1] / sinhalfangle;
    z = _fv[2] / sinhalfangle;
}


/// Spherical Linear Interpolation
/// As t goes from 0 to 1, the Quat object goes from "from" to "to"
/// Reference: Shoemake at SIGGRAPH 89
/// See also
/// http://www.gamasutra.com/features/programming/19980703/quaternions_01.htm
void Quat::slerp( const float t, const Quat& from, const Quat& to )
{
    const double epsilon = 0.00001;
    double omega, cosomega, sinomega, scale_from, scale_to ;

                                 // this is a dot product
    cosomega = from.asVec4() * to.asVec4() ;

    if( (1.0 - cosomega) > epsilon )
    {
        omega= acos(cosomega) ;  // 0 <= omega <= Pi (see man acos)
        sinomega = sin(omega) ;  // this sinomega should always be +ve so
        // could try sinomega=sqrt(1-cosomega*cosomega) to avoid a sin()?
        scale_from = sin((1.0-t)*omega)/sinomega ;
        scale_to = sin(t*omega)/sinomega ;
    }
    else
    {
        /* --------------------------------------------------
           The ends of the vectors are very close
           we can use simple linear interpolation - no need
           to worry about the "spherical" interpolation
           -------------------------------------------------- */
        scale_from = 1.0 - t ;
        scale_to = t ;
    }

                                 // use Vec4 arithmetic
    _fv = (from._fv*scale_from) + (to._fv*scale_to);
    // so that we get a Vec4
}


#define QX  _fv[0]
#define QY  _fv[1]
#define QZ  _fv[2]
#define QW  _fv[3]

void Quat::set( const Matrix& m )
{
    // Source: Gamasutra, Rotating Objects Using Quaternions
    //
    //http://www.gamasutra.com/features/programming/19980703/quaternions_01.htm

    float  tr, s;
    float  tq[4];
    int    i, j, k;

    int nxt[3] = {1, 2, 0};

    tr = m(0,0) + m(1,1) + m(2,2);

    // check the diagonal
    if (tr > 0.0)
    {
        s = (float)sqrt (tr + 1.0);
        QW = s / 2.0f;
        s = 0.5f / s;
        QX = (m(1,2) - m(2,1)) * s;
        QY = (m(2,0) - m(0,2)) * s;
        QZ = (m(0,1) - m(1,0)) * s;
    }
    else
    {
        // diagonal is negative
        i = 0;
        if (m(1,1) > m(0,0))
            i = 1;
        if (m(2,2) > m(i,i))
            i = 2;
        j = nxt[i];
        k = nxt[j];

        s = (float)sqrt ((m(i,i) - (m(j,j) + m(k,k))) + 1.0);

        tq[i] = s * 0.5f;

        if (s != 0.0f)
            s = 0.5f / s;

        tq[3] = (m(j,k) - m(k,j)) * s;
        tq[j] = (m(i,j) + m(j,i)) * s;
        tq[k] = (m(i,k) + m(k,i)) * s;

        QX = tq[0];
        QY = tq[1];
        QZ = tq[2];
        QW = tq[3];
    }
}


void Quat::get( Matrix& m ) const
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
    m(0,0) = 1.0f - (yy + zz);
    m(1,0) = xy - wz;
    m(2,0) = xz + wy;
    m(3,0) = 0.0f;

    m(0,1) = xy + wz;
    m(1,1) = 1.0f - (xx + zz);
    m(2,1) = yz - wx;
    m(3,1) = 0.0f;

    m(0,2) = xz - wy;
    m(1,2) = yz + wx;
    m(2,2) = 1.0f - (xx + yy);
    m(3,2) = 0.0f;

    m(0,3) = 0;
    m(1,3) = 0;
    m(2,3) = 0;
    m(3,3) = 1;
}
