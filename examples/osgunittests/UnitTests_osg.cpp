#include "UnitTestFramework.h"

#include <osg/Matrixd>
#include <osg/Vec3>
#include <sstream>

namespace osg
{

///////////////////////////////////////////////////////////////////////////////
// 
//  Matrixd Tests
//

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

///////////////////////////////////////////////////////////////////////////////
// 
//  Vec3 Tests
//
class Vec3TestFixture
{
public:

    Vec3TestFixture();

    void testAddition(const osgUtx::TestContext& ctx);
    void testSubtraction(const osgUtx::TestContext& ctx);
    void testScalarMultiplication(const osgUtx::TestContext& ctx);
    void testDotProduct(const osgUtx::TestContext& ctx);

private:

    // Some convenience variables for use in the tests
    Vec3 v1_, v2_, v3_;

};

Vec3TestFixture::Vec3TestFixture():
    v1_(1.0f, 1.0f, 1.0f),
    v2_(2.0f, 2.0f, 2.0f),
    v3_(3.0f, 3.0f, 3.0f)
{
}

void Vec3TestFixture::testAddition(const osgUtx::TestContext&)
{
    OSGUTX_TEST_F( v1_ + v2_ == v3_ )
}

void Vec3TestFixture::testSubtraction(const osgUtx::TestContext&)
{
    OSGUTX_TEST_F( v3_ - v1_ == v2_ )
}

void Vec3TestFixture::testScalarMultiplication(const osgUtx::TestContext&)
{
    OSGUTX_TEST_F( v1_ * 3 == v3_ )
}

void Vec3TestFixture::testDotProduct(const osgUtx::TestContext&)
{
    
}

OSGUTX_BEGIN_TESTSUITE(Vec3)
    OSGUTX_ADD_TESTCASE(Vec3TestFixture, testAddition)
    OSGUTX_ADD_TESTCASE(Vec3TestFixture, testSubtraction)
    OSGUTX_ADD_TESTCASE(Vec3TestFixture, testScalarMultiplication)
    OSGUTX_ADD_TESTCASE(Vec3TestFixture, testDotProduct)
OSGUTX_END_TESTSUITE

OSGUTX_AUTOREGISTER_TESTSUITE_AT(Vec3, root.osg)

}
