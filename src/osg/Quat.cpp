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
#include <osg/Matrixf>
#include <osg/Matrixd>
#include <osg/Notify>

#include <math.h>

/// Good introductions to Quaternions at:
/// http://www.gamasutra.com/features/programming/19980703/quaternions_01.htm
/// http://mathworld.wolfram.com/Quaternion.html

using namespace osg;


void Quat::set(const Matrixf& matrix)
{
    matrix.get(*this);
}

void Quat::set(const Matrixd& matrix)
{
    matrix.get(*this);
}

void Quat::get(Matrixf& matrix) const
{
    matrix.set(*this);
}

void Quat::get(Matrixd& matrix) const
{
    matrix.set(*this);
}


/// Set the elements of the Quat to represent a rotation of angle
/// (radians) around the axis (x,y,z)
void Quat::makeRotate( value_type angle, value_type x, value_type y, value_type z )
{
    const value_type epsilon = 0.0000001;

    value_type length = sqrt( x*x + y*y + z*z );
    if (length < epsilon)
    {
        // ~zero length axis, so reset rotation to zero.
        *this = Quat();
        return;
    }

    value_type inversenorm  = 1.0/length;
    value_type coshalfangle = cos( 0.5*angle );
    value_type sinhalfangle = sin( 0.5*angle );

    _v[0] = x * sinhalfangle * inversenorm;
    _v[1] = y * sinhalfangle * inversenorm;
    _v[2] = z * sinhalfangle * inversenorm;
    _v[3] = coshalfangle;
}


void Quat::makeRotate( value_type angle, const Vec3f& vec )
{
    makeRotate( angle, vec[0], vec[1], vec[2] );
}
void Quat::makeRotate( value_type angle, const Vec3d& vec )
{
    makeRotate( angle, vec[0], vec[1], vec[2] );
}


void Quat::makeRotate ( value_type angle1, const Vec3f& axis1, 
                        value_type angle2, const Vec3f& axis2,
                        value_type angle3, const Vec3f& axis3)
{
    makeRotate(angle1,Vec3d(axis1),
               angle2,Vec3d(axis2),
               angle3,Vec3d(axis3));
}                        

void Quat::makeRotate ( value_type angle1, const Vec3d& axis1, 
                        value_type angle2, const Vec3d& axis2,
                        value_type angle3, const Vec3d& axis3)
{
    Quat q1; q1.makeRotate(angle1,axis1);
    Quat q2; q2.makeRotate(angle2,axis2);
    Quat q3; q3.makeRotate(angle3,axis3);

    *this = q1*q2*q3;
}                        


void Quat::makeRotate( const Vec3f& from, const Vec3f& to )
{
    makeRotate( Vec3d(from), Vec3d(to) );
}

// Make a rotation Quat which will rotate vec1 to vec2
// Generally take adot product to get the angle between these
// and then use a cross product to get the rotation axis
// Watch out for the two special cases of when the vectors
// are co-incident or opposite in direction.
void Quat::makeRotate( const Vec3d& from, const Vec3d& to )
{
    const value_type epsilon = 0.0000001;

    value_type length1  = from.length();
    value_type length2  = to.length();
    
    // dot product vec1*vec2
    value_type cosangle = from*to/(length1*length2);

    if ( fabs(cosangle - 1) < epsilon )
    {
        osg::notify(osg::INFO)<<"*** Quat::makeRotate(from,to) with near co-linear vectors, epsilon= "<<fabs(cosangle-1)<<std::endl;
    
        // cosangle is close to 1, so the vectors are close to being coincident
        // Need to generate an angle of zero with any vector we like
        // We'll choose (1,0,0)
        makeRotate( 0.0, 0.0, 0.0, 1.0 );
    }
    else
    if ( fabs(cosangle + 1.0) < epsilon )
    {
        // vectors are close to being opposite, so will need to find a
        // vector orthongonal to from to rotate about.
        Vec3d tmp;
        if (fabs(from.x())<fabs(from.y()))
            if (fabs(from.x())<fabs(from.z())) tmp.set(1.0,0.0,0.0); // use x axis.
            else tmp.set(0.0,0.0,1.0);
        else if (fabs(from.y())<fabs(from.z())) tmp.set(0.0,1.0,0.0);
        else tmp.set(0.0,0.0,1.0);
        
        Vec3d fromd(from.x(),from.y(),from.z());
        
        // find orthogonal axis.
        Vec3d axis(fromd^tmp);
        axis.normalize();
        
        _v[0] = axis[0]; // sin of half angle of PI is 1.0.
        _v[1] = axis[1]; // sin of half angle of PI is 1.0.
        _v[2] = axis[2]; // sin of half angle of PI is 1.0.
        _v[3] = 0; // cos of half angle of PI is zero.

    }
    else
    {
        // This is the usual situation - take a cross-product of vec1 and vec2
        // and that is the axis around which to rotate.
        Vec3d axis(from^to);
        value_type angle = acos( cosangle );
        makeRotate( angle, axis );
    }
}


void Quat::getRotate( value_type& angle, Vec3f& vec ) const
{
    value_type x,y,z;
    getRotate(angle,x,y,z);
    vec[0]=x;
    vec[1]=y;
    vec[2]=z;
}
void Quat::getRotate( value_type& angle, Vec3d& vec ) const
{
    value_type x,y,z;
    getRotate(angle,x,y,z);
    vec[0]=x;
    vec[1]=y;
    vec[2]=z;
}


// Get the angle of rotation and axis of this Quat object.
// Won't give very meaningful results if the Quat is not associated
// with a rotation!
void Quat::getRotate( value_type& angle, value_type& x, value_type& y, value_type& z ) const
{
    value_type sinhalfangle = sqrt( _v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2] );

    angle = 2.0 * atan2( sinhalfangle, _v[3] );
    if(sinhalfangle)
    {
        x = _v[0] / sinhalfangle;
        y = _v[1] / sinhalfangle;
        z = _v[2] / sinhalfangle;
    }
    else
    {
        x = 0.0;
        y = 0.0;
        z = 1.0;
    }

}


/// Spherical Linear Interpolation
/// As t goes from 0 to 1, the Quat object goes from "from" to "to"
/// Reference: Shoemake at SIGGRAPH 89
/// See also
/// http://www.gamasutra.com/features/programming/19980703/quaternions_01.htm
void Quat::slerp( value_type t, const Quat& from, const Quat& to )
{
    const double epsilon = 0.00001;
    double omega, cosomega, sinomega, scale_from, scale_to ;
    
    osg::Quat quatTo(to);
    // this is a dot product
    
    cosomega = from.asVec4() * to.asVec4();
    
    if ( cosomega <0.0 )
    { 
        cosomega = -cosomega; 
        quatTo = -to;
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

    *this = (from*scale_from) + (quatTo*scale_to);
    
    // so that we get a Vec4
}


#define QX  _v[0]
#define QY  _v[1]
#define QZ  _v[2]
#define QW  _v[3]


#ifdef OSG_USE_UNIT_TESTS
void test_Quat_Eueler(value_type heading,value_type pitch,value_type roll)
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
