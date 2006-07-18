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

#include <osg/Matrixd>
#include <osg/Matrixf>

#ifdef OSG_COMPILE_UNIT_TESTS


#include <osg/UnitTestFramework>

#include <sstream>
#endif

// specialise Matrix_implementaiton to be Matrixd
#define  Matrix_implementation Matrixd

osg::Matrixd::Matrixd( const osg::Matrixf& mat )
{
    set(mat.ptr());
}

osg::Matrixd& osg::Matrixd::operator = (const osg::Matrixf& rhs)
{
    set(rhs.ptr());
    return *this;
}

void osg::Matrixd::set(const osg::Matrixf& rhs)
{
    set(rhs.ptr());
}

// now compile up Matrix via Matrix_implementation
#include "Matrix_implementation.cpp"

//#if 1
#ifdef OSG_COMPILE_UNIT_TESTS

#include <osg/Vec3>
#include <osg/UnitTestFramework>

#include <sstream>

namespace osg
{

class MatrixdTestFixture
{
public:

    MatrixdTestFixture();

    void testMatrixToQuat(const osgUtx::TestContext& ctx);

private:

    // Some convenience variables for use in the tests
    Matrixd m1_, m2_;
    double l1_, l2_;

};

MatrixdTestFixture::MatrixdTestFixture():
    m1_(0.3583681546368404100000000000000, -0.933580347769909500000000000000, 3.006977197034146200000000000000e-011, 000000000000000, 
    -0.933580347769909500000000000000, -0.3583681546368404100000000000000, -1.275368738108216700000000000000e-010, 000000000000000, 
    1.298419676971558500000000000000e-010, 1.763260594230249800000000000000e-011, -0.9999999999999997800000000000000, 000000000000000, 
    -4.134153519264493800000000000000e-005, 8.473552245044272300000000000000e-008, 0.9999996934706840700000000000000, 100000000000000.0 ),

    m2_(      0.3583681546368407400000000000000, -0.9335803477699099500000000000000, 3.007582030796253300000000000000e-011, 000000000000000, 
    -0.9335803477699099500000000000000, -0.3583681546368407400000000000000, -1.265511449721884600000000000000e-010, 000000000000000, 
    1.289238781567697100000000000000e-010, 1.727370550828948600000000000000e-011, -1.000000000000000400000000000000, 000000000000000, 
    -4.134153473360120600000000000000e-005, 8.473570190103158800000000000000e-008, 0.999999693471385400000000000000, 100000000000000.0),
    l1_(1),
    l2_(1)
{
}

void MatrixdTestFixture::testMatrixToQuat(const osgUtx::TestContext&)
{
    Quat q1,q2;
    q1.set(m1_);
    q2.set(m2_);
    OSGUTX_TEST_F( q1.length() == l1_ )
    OSGUTX_TEST_F( q2.length() == l2_ )
}


OSGUTX_BEGIN_TESTSUITE(Matrixd)
    OSGUTX_ADD_TESTCASE(MatrixdTestFixture, testMatrixToQuat)
OSGUTX_END_TESTSUITE

OSGUTX_AUTOREGISTER_TESTSUITE_AT(Matrixd, root.osg)

}

#endif
