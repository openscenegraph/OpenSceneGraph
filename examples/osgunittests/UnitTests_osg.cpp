/* OpenSceneGraph example, osgunittests.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include "UnitTestFramework.h"

#include <osg/Matrixd>
#include <osg/Matrixf>
#include <osg/Vec3d>
#include <osg/Vec3>
#include <sstream>

namespace osg
{


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

///////////////////////////////////////////////////////////////////////////////
// 
//  Matrix Tests
//
class MatrixTestFixture
{
public:

    MatrixTestFixture();

    void testPreMultTranslate(const osgUtx::TestContext& ctx);
    void testPostMultTranslate(const osgUtx::TestContext& ctx);
    void testPreMultScale(const osgUtx::TestContext& ctx);
    void testPostMultScale(const osgUtx::TestContext& ctx);
    void testPreMultRotate(const osgUtx::TestContext& ctx);
    void testPostMultRotate(const osgUtx::TestContext& ctx);

private:

    // Some convenience variables for use in the tests
    Matrixd _md;
    Matrixf _mf;
    Vec3d _v3d;
    Vec3 _v3;
    Quat _q1;
    Quat _q2;
    Quat _q3;
    Quat _q4;

};

MatrixTestFixture::MatrixTestFixture():
    _md(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16),
    _mf(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16),
    _v3d(1, 2, 3),
    _v3(1, 2, 3),
    _q1(1, 0, 0, 0),
    _q2(0, 1, 0, 0),
    _q3(0, 0, 1, 0),
    _q4(0, 0, 0, 1)
{
}

void MatrixTestFixture::testPreMultTranslate(const osgUtx::TestContext&)
{
    osg::Matrixd tdo;
    osg::Matrixd tdn;
    osg::Matrixf tfo;
    osg::Matrixf tfn;

    tdo = _md;
    tdn = _md;
    tdo.preMult(osg::Matrixd::translate(_v3d));
    tdn.preMultTranslate(_v3d);
    OSGUTX_TEST_F( tdo == tdn )

    tdo = _md;
    tdn = _md;
    tdo.preMult(osg::Matrixd::translate(_v3));
    tdn.preMultTranslate(_v3);
    OSGUTX_TEST_F( tdo == tdn )

    tfo = _mf;
    tfn = _mf;
    tfo.preMult(osg::Matrixf::translate(_v3d));
    tfn.preMultTranslate(_v3d);
    OSGUTX_TEST_F( tfo == tfn )

    tfo = _mf;
    tfn = _mf;
    tfo.preMult(osg::Matrixf::translate(_v3));
    tfn.preMultTranslate(_v3);
    OSGUTX_TEST_F( tfo == tfn )
}

void MatrixTestFixture::testPostMultTranslate(const osgUtx::TestContext&)
{
    osg::Matrixd tdo;
    osg::Matrixd tdn;
    osg::Matrixf tfo;
    osg::Matrixf tfn;

    tdo = _md;
    tdn = _md;
    tdo.postMult(osg::Matrixd::translate(_v3d));
    tdn.postMultTranslate(_v3d);
    OSGUTX_TEST_F( tdo == tdn )

    tdo = _md;
    tdn = _md;
    tdo.postMult(osg::Matrixd::translate(_v3));
    tdn.postMultTranslate(_v3);
    OSGUTX_TEST_F( tdo == tdn )

    tfo = _mf;
    tfn = _mf;
    tfo.postMult(osg::Matrixf::translate(_v3d));
    tfn.postMultTranslate(_v3d);
    OSGUTX_TEST_F( tfo == tfn )

    tfo = _mf;
    tfn = _mf;
    tfo.postMult(osg::Matrixf::translate(_v3));
    tfn.postMultTranslate(_v3);
    OSGUTX_TEST_F( tfo == tfn )
}

void MatrixTestFixture::testPreMultScale(const osgUtx::TestContext&)
{
    osg::Matrixd tdo;
    osg::Matrixd tdn;
    osg::Matrixf tfo;
    osg::Matrixf tfn;

    tdo = _md;
    tdn = _md;
    tdo.preMult(osg::Matrixd::scale(_v3d));
    tdn.preMultScale(_v3d);
    OSGUTX_TEST_F( tdo == tdn )

    tdo = _md;
    tdn = _md;
    tdo.preMult(osg::Matrixd::scale(_v3));
    tdn.preMultScale(_v3);
    OSGUTX_TEST_F( tdo == tdn )

    tfo = _mf;
    tfn = _mf;
    tfo.preMult(osg::Matrixf::scale(_v3d));
    tfn.preMultScale(_v3d);
    OSGUTX_TEST_F( tfo == tfn )

    tfo = _mf;
    tfn = _mf;
    tfo.preMult(osg::Matrixf::scale(_v3));
    tfn.preMultScale(_v3);
    OSGUTX_TEST_F( tfo == tfn )
}

void MatrixTestFixture::testPostMultScale(const osgUtx::TestContext&)
{
    osg::Matrixd tdo;
    osg::Matrixd tdn;
    osg::Matrixf tfo;
    osg::Matrixf tfn;

    tdo = _md;
    tdn = _md;
    tdo.postMult(osg::Matrixd::scale(_v3d));
    tdn.postMultScale(_v3d);
    OSGUTX_TEST_F( tdo == tdn )

    tdo = _md;
    tdn = _md;
    tdo.postMult(osg::Matrixd::scale(_v3));
    tdn.postMultScale(_v3);
    OSGUTX_TEST_F( tdo == tdn )

    tfo = _mf;
    tfn = _mf;
    tfo.postMult(osg::Matrixf::scale(_v3d));
    tfn.postMultScale(_v3d);
    OSGUTX_TEST_F( tfo == tfn )

    tfo = _mf;
    tfn = _mf;
    tfo.postMult(osg::Matrixf::scale(_v3));
    tfn.postMultScale(_v3);
    OSGUTX_TEST_F( tfo == tfn )
}

void MatrixTestFixture::testPreMultRotate(const osgUtx::TestContext&)
{
    osg::Matrixd tdo;
    osg::Matrixd tdn;
    osg::Matrixf tfo;
    osg::Matrixf tfn;

    tdo = _md;
    tdn = _md;
    tdo.preMult(osg::Matrixd::rotate(_q1));
    tdn.preMultRotate(_q1);
    OSGUTX_TEST_F( tdo == tdn )

    tdo = _md;
    tdn = _md;
    tdo.preMult(osg::Matrixd::rotate(_q2));
    tdn.preMultRotate(_q2);
    OSGUTX_TEST_F( tdo == tdn )

    tdo = _md;
    tdn = _md;
    tdo.preMult(osg::Matrixd::rotate(_q3));
    tdn.preMultRotate(_q3);
    OSGUTX_TEST_F( tdo == tdn )

    tdo = _md;
    tdn = _md;
    tdo.preMult(osg::Matrixd::rotate(_q4));
    tdn.preMultRotate(_q4);
    OSGUTX_TEST_F( tdo == tdn )

    tfo = _mf;
    tfn = _mf;
    tfo.preMult(osg::Matrixf::rotate(_q1));
    tfn.preMultRotate(_q1);
    OSGUTX_TEST_F( tfo == tfn )

    tfo = _mf;
    tfn = _mf;
    tfo.preMult(osg::Matrixf::rotate(_q2));
    tfn.preMultRotate(_q2);
    OSGUTX_TEST_F( tfo == tfn )

    tfo = _mf;
    tfn = _mf;
    tfo.preMult(osg::Matrixf::rotate(_q3));
    tfn.preMultRotate(_q3);
    OSGUTX_TEST_F( tfo == tfn )

    tfo = _mf;
    tfn = _mf;
    tfo.preMult(osg::Matrixf::rotate(_q4));
    tfn.preMultRotate(_q4);
    OSGUTX_TEST_F( tfo == tfn )
}

void MatrixTestFixture::testPostMultRotate(const osgUtx::TestContext&)
{
    osg::Matrixd tdo;
    osg::Matrixd tdn;
    osg::Matrixf tfo;
    osg::Matrixf tfn;

    tdo = _md;
    tdn = _md;
    tdo.postMult(osg::Matrixd::rotate(_q1));
    tdn.postMultRotate(_q1);
    OSGUTX_TEST_F( tdo == tdn )

    tdo = _md;
    tdn = _md;
    tdo.postMult(osg::Matrixd::rotate(_q2));
    tdn.postMultRotate(_q2);
    OSGUTX_TEST_F( tdo == tdn )

    tdo = _md;
    tdn = _md;
    tdo.postMult(osg::Matrixd::rotate(_q3));
    tdn.postMultRotate(_q3);
    OSGUTX_TEST_F( tdo == tdn )

    tdo = _md;
    tdn = _md;
    tdo.postMult(osg::Matrixd::rotate(_q4));
    tdn.postMultRotate(_q4);
    OSGUTX_TEST_F( tdo == tdn )

    tfo = _mf;
    tfn = _mf;
    tfo.postMult(osg::Matrixf::rotate(_q1));
    tfn.postMultRotate(_q1);
    OSGUTX_TEST_F( tfo == tfn )

    tfo = _mf;
    tfn = _mf;
    tfo.postMult(osg::Matrixf::rotate(_q2));
    tfn.postMultRotate(_q2);
    OSGUTX_TEST_F( tfo == tfn )

    tfo = _mf;
    tfn = _mf;
    tfo.postMult(osg::Matrixf::rotate(_q3));
    tfn.postMultRotate(_q3);
    OSGUTX_TEST_F( tfo == tfn )

    tfo = _mf;
    tfn = _mf;
    tfo.postMult(osg::Matrixf::rotate(_q4));
    tfn.postMultRotate(_q4);
    OSGUTX_TEST_F( tfo == tfn )
}

OSGUTX_BEGIN_TESTSUITE(Matrix)
    OSGUTX_ADD_TESTCASE(MatrixTestFixture, testPreMultTranslate)
    OSGUTX_ADD_TESTCASE(MatrixTestFixture, testPostMultTranslate)
    OSGUTX_ADD_TESTCASE(MatrixTestFixture, testPreMultScale)
    OSGUTX_ADD_TESTCASE(MatrixTestFixture, testPostMultScale)
    OSGUTX_ADD_TESTCASE(MatrixTestFixture, testPreMultRotate)
    OSGUTX_ADD_TESTCASE(MatrixTestFixture, testPostMultRotate)
OSGUTX_END_TESTSUITE

OSGUTX_AUTOREGISTER_TESTSUITE_AT(Matrix, root.osg)


}
