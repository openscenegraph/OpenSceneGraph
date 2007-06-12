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

}
