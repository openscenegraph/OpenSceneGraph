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
#include <stdio.h>
#include <osg/Quat>
#include <osg/Vec4>
#include <osg/Vec3>

#include <math.h>

/// Good introductions to Quaternions at:
/// http://www.gamasutra.com/features/programming/19980703/quaternions_01.htm
/// http://mathworld.wolfram.com/Quaternion.html

using namespace osg;


/// Set the elements of the Quat to represent a rotation of angle
/// (radians) around the axis (x,y,z)
void Quat::makeRotate( float angle,
float x,
float y,
float z    )
{
    float inversenorm  = 1.0/sqrt( x*x + y*y + z*z );
    float coshalfangle = cos( 0.5*angle );
    float sinhalfangle = sin( 0.5*angle );

    _fv[0] = x * sinhalfangle * inversenorm;
    _fv[1] = y * sinhalfangle * inversenorm;
    _fv[2] = z * sinhalfangle * inversenorm;
    _fv[3] = coshalfangle;
}


void Quat::makeRotate( float angle, const Vec3& vec )
{
    makeRotate( angle, vec[0], vec[1], vec[2] );
}


void Quat::makeRotate ( float angle1, const Vec3& axis1, 
                        float angle2, const Vec3& axis2,
                        float angle3, const Vec3& axis3)
{
    Quat q1; q1.makeRotate(angle1,axis1);
    Quat q2; q2.makeRotate(angle2,axis2);
    Quat q3; q3.makeRotate(angle3,axis3);

    *this = q1*q2*q3;
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


void Quat::getRotate( float& angle, Vec3& vec ) const
{
    getRotate(angle,vec[0],vec[1],vec[2]);
}


// Get the angle of rotation and axis of this Quat object.
// Won't give very meaningful results if the Quat is not associated
// with a rotation!
void Quat::getRotate( float& angle, float& x, float& y, float& z ) const
{
    float sinhalfangle = sqrt( _fv[0]*_fv[0] + _fv[1]*_fv[1] + _fv[2]*_fv[2] );

    angle = 2 * atan2( sinhalfangle, _fv[3] );
    if(sinhalfangle)
    {
        x = _fv[0] / sinhalfangle;
        y = _fv[1] / sinhalfangle;
        z = _fv[2] / sinhalfangle;
    }
    else
    {
        x = 0.0f;
        y = 0.0f;
        z = 1.0f;
    }

}


/// Spherical Linear Interpolation
/// As t goes from 0 to 1, the Quat object goes from "from" to "to"
/// Reference: Shoemake at SIGGRAPH 89
/// See also
/// http://www.gamasutra.com/features/programming/19980703/quaternions_01.htm
void Quat::slerp( float t, const Quat& from, const Quat& to )
{
    const double epsilon = 0.00001;
    double omega, cosomega, sinomega, scale_from, scale_to ;
    
    osg::Quat quatTo(to);
    // this is a dot product
    
    cosomega = from.asVec4() * to.asVec4();
    
    if ( cosomega <0.0 )
    { 
        cosomega = -cosomega; 
        quatTo.set(-to._fv);
    }

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
    _fv = (from._fv*scale_from) + (quatTo._fv*scale_to);
    
    // so that we get a Vec4
}


#define QX  _fv[0]
#define QY  _fv[1]
#define QZ  _fv[2]
#define QW  _fv[3]

void Quat::set( const Matrix& m )
{
    // Source: 
    //
    // http://mccammon.ucsd.edu/~adcock/matrixfaq.html#Q55
    
    float x_scale = sqrtf(osg::square(m(0,0))+osg::square(m(1,0))+osg::square(m(2,0)));
    
    if (osg::absolute(x_scale-1.0f)>1e-5)
    {
        osg::Matrix new_m(m*osg::Matrix::scale(1.0f/x_scale,1.0f/x_scale,1.0f/x_scale));
        _set(new_m);
    }
    else
    {
        _set(m);
    }
}
    
void Quat::_set(const Matrix& m )
{    
    //std::cout<<"Matrix scaled "<<m<<std::endl;

    double S;
    double tr = m(0,0) + m(1,1) + m(2,2) + 1.0;

    //cout << "tr="<<tr<<endl;

    // check the diagonal
    if (tr > 2e-5/*0.00000001*/)
    {
        //cout << "path one"<<endl;
    
        S = 0.5/sqrt (tr);
        QW = 0.25 / S;
        QX = (m(1,2) - m(2,1)) * S;
        QY = (m(2,0) - m(0,2)) * S;
        QZ = (m(0,1) - m(1,0)) * S;
    }
    else
    {
        //cout << "path two"<<endl;

        if ( m(0,0) > m(1,1) && m(0,0) > m(2,2) )  {       // Column 0: 
            S  = sqrt( 1.0 + m(0,0) - m(1,1) - m(2,2) ) * 2.0;
            QX = 0.25 * S;
            QY = (m(1,0) + m(0,1) ) / S;
            QZ = (m(0,2) + m(2,0) ) / S;
            QW = (m(2,1) - m(1,2) ) / S;

        } else if ( m(1,1) > m(2,2) ) {                    // Column 1: 
            S  = sqrt( 1.0 + m(1,1) - m(0,0) - m(2,2) ) * 2.0;
            QX = (m(1,0) + m(0,1) ) / S;
            QY = 0.25 * S;
            QZ = (m(2,1) + m(1,2) ) / S;
            QW = (m(0,2) - m(2,0) ) / S;

        } else {                                            // Column 2:
            S  = sqrt( 1.0 + m(2,2) - m(0,0) - m(1,1) ) * 2.0;
            QX = (m(0,2) + m(2,0) ) / S;
            QY = (m(2,1) + m(1,2) ) / S;
            QZ = 0.25f * S;
            QW = (m(1,0) - m(0,1) ) / S;
        }
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

#ifdef OSG_USE_UNIT_TESTS
void test_Quat_Eueler(float heading,float pitch,float roll)
{
    osg::Quat q;
    q.makeRotate(heading,pitch,roll);
    
    osg::Matrix q_m;
    q.get(q_m);
    
    osg::Vec3 xAxis(1,0,0);
    osg::Vec3 yAxis(0,1,0);
    osg::Vec3 zAxis(0,0,1);
    
    cout << "heading = "<<heading<<"  pitch = "<<pitch<<"  roll = "<<roll<<endl;

    cout <<"q_m = "<<q_m;
    cout <<"xAxis*q_m = "<<xAxis*q_m << endl;
    cout <<"yAxis*q_m = "<<yAxis*q_m << endl;
    cout <<"zAxis*q_m = "<<zAxis*q_m << endl;
    
    osg::Matrix r_m = osg::Matrix::rotate(roll,0.0,1.0,0.0)*
                      osg::Matrix::rotate(pitch,1.0,0.0,0.0)*
                      osg::Matrix::rotate(-heading,0.0,0.0,1.0);
                      
    cout << "r_m = "<<r_m;
    cout <<"xAxis*r_m = "<<xAxis*r_m << endl;
    cout <<"yAxis*r_m = "<<yAxis*r_m << endl;
    cout <<"zAxis*r_m = "<<zAxis*r_m << endl;
    
    cout << endl<<"*****************************************" << endl<< endl;
    
}

void test_Quat()
{

    test_Quat_Eueler(osg::DegreesToRadians(20),0,0);
    test_Quat_Eueler(0,osg::DegreesToRadians(20),0);
    test_Quat_Eueler(0,0,osg::DegreesToRadians(20));
    test_Quat_Eueler(osg::DegreesToRadians(20),osg::DegreesToRadians(20),osg::DegreesToRadians(20));
    return 0;
}
#endif
