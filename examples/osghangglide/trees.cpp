/* OpenSceneGraph example, osghangglide.
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

#include <stdlib.h>

#include <osg/Billboard>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/BlendFunc>
#include <osg/AlphaFunc>

#include <osgDB/ReadFile>

#include "hat.h"

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

using namespace osg;

#define sqr(x)  ((x)*(x))

extern void getDatabaseCenterRadius( float dbcenter[3], float *dbradius );

static float dbcenter[3], dbradius;

static struct _tree
{
    int n;
    float x, y, z;
    float w, h;
}


trees[] =
{
    {    0,  -0.4769,  -0.8972,  -0.4011,   0.2000,   0.1200 },
    {    1,  -0.2543,  -0.9117,  -0.3873,   0.2000,   0.1200 },
    {    2,  -0.0424,  -0.8538,  -0.3728,   0.2000,   0.1200 },
    {    3,   0.1590,  -0.8827,  -0.3594,   0.2000,   0.1200 },
    {    4,  -0.4981,  -1.0853,  -0.4016,   0.3500,   0.1200 },
    {    5,  -0.5405,  -1.2590,  -0.4050,   0.2000,   0.1200 },
    {    6,  -0.5723,  -1.5339,  -0.4152,   0.2000,   0.1200 },
    {    7,  -0.6252,  -1.8667,  -0.4280,   0.2000,   0.1200 },
    {    8,  -0.5617,  -2.1851,  -0.4309,   0.2000,   0.1200 },
    {    9,  -0.5087,  -2.4166,  -0.4215,   0.2000,   0.1200 },
    {   10,  -0.4345,  -2.3443,  -0.4214,   0.2000,   0.1200 },
    {   11,  -3.0308,  -1.5484,  -0.4876,   0.2000,   0.1200 },
    {   12,  -3.0202,  -1.6497,  -0.4963,   0.2000,   0.1200 },
    {   13,  -2.9355,  -1.8378,  -0.4969,   0.2000,   0.1200 },
    {   14,  -0.6040,  -2.0259,  -0.4300,   0.2000,   0.1200 },
    {   15,  -0.5442,  -1.3442,  -0.4080,   0.1000,   0.1200 },
    {   16,  -0.5639,  -1.6885,  -0.4201,   0.1000,   0.1200 },
    {   17,   0.9246,   3.4835,   0.5898,   0.2500,   0.1000 },
    {   18,   0.0787,   3.8687,   0.3329,   0.2500,   0.1200 },
    {   19,   0.2885,   3.7130,   0.4047,   0.2500,   0.1200 },
    {   20,   0.2033,   3.6228,   0.3704,   0.2500,   0.1200 },
    {   21,  -0.2098,   3.9015,   0.2327,   0.2500,   0.1200 },
    {   22,  -0.3738,   3.7376,   0.1722,   0.2500,   0.1200 },
    {   23,  -0.2557,   3.6064,   0.1989,   0.2500,   0.1200 },
    {   24,   0.0590,   3.7294,   0.3210,   0.2500,   0.1200 },
    {   25,  -0.4721,   3.8851,   0.1525,   0.2500,   0.1200 },
    {   26,   0.9639,   3.2048,   0.5868,   0.1200,   0.0800 },
    {   27,   0.7082,  -1.0409,  -0.3221,   0.1000,   0.1000 },
    {   28,  -0.2426,  -2.3442,  -0.4150,   0.1000,   0.1380 },
    {   29,  -0.1770,  -2.4179,  -0.4095,   0.1000,   0.1580 },
    {   30,  -0.0852,  -2.5327,  -0.4056,   0.1000,   0.1130 },
    {   31,  -0.0131,  -2.6065,  -0.4031,   0.1000,   0.1150 },
    {   32,   0.0787,  -2.6638,  -0.4012,   0.1000,   0.1510 },
    {   33,   0.1049,  -2.7622,  -0.3964,   0.1000,   0.1270 },
    {   34,   0.1770,  -2.8687,  -0.3953,   0.1000,   0.1100 },
    {   35,   0.3213,  -2.9507,  -0.3974,   0.1000,   0.1190 },
    {   36,   0.4065,  -3.0163,  -0.4014,   0.1000,   0.1120 },
    {   37,   0.3738,  -3.1802,  -0.4025,   0.1000,   0.1860 },
    {   38,   0.5508,  -3.2048,  -0.3966,   0.1000,   0.1490 },
    {   39,   0.5836,  -3.3031,  -0.3900,   0.1000,   0.1670 },
    {   40,  -0.3082,  -2.7212,  -0.3933,   0.1000,   0.1840 },
    {   41,  -0.1967,  -2.6474,  -0.4017,   0.1000,   0.1600 },
    {   42,  -0.1180,  -2.7458,  -0.3980,   0.1000,   0.1250 },
    {   43,  -0.3344,  -2.8359,  -0.3964,   0.1000,   0.1430 },
    {   44,  -0.2492,  -2.8933,  -0.3838,   0.1000,   0.1890 },
    {   45,  -0.1246,  -3.0491,  -0.3768,   0.1000,   0.1830 },
    {   46,   0.0000,  -3.0818,  -0.3696,   0.1000,   0.1370 },
    {   47,  -0.2295,  -3.0409,  -0.3706,   0.1000,   0.1660 },
    {   48,  -1.3245,   2.6638,   0.0733,   0.0500,   0.0500 },
    {   49,   2.2425,  -1.5491,  -0.2821,   0.2300,   0.1200 },
    {   50,   0.2164,  -2.1311,  -0.4000,   0.1000,   0.0690 },
    {   51,   0.2885,  -2.2130,  -0.4000,   0.1000,   0.0790 },
    {   52,   0.3606,  -2.2786,  -0.4000,   0.1000,   0.0565 },
    {   53,   0.4328,  -2.3442,  -0.4000,   0.1000,   0.0575 },
    {   54,   0.5246,  -2.4343,  -0.4086,   0.1000,   0.0755 },
    {   55,   0.6360,  -2.5245,  -0.4079,   0.1000,   0.0635 },
    {   56,   0.7541,  -2.4261,  -0.4007,   0.1000,   0.0550 },
    {   57,   0.7934,  -2.2786,  -0.3944,   0.1000,   0.0595 },
    {   58,   1.0295,  -2.2868,  -0.3837,   0.1000,   0.0560 },
    {   59,   0.8459,  -2.6474,  -0.4051,   0.1000,   0.0930 },
    {   60,   1.0426,  -2.6884,  -0.4001,   0.1000,   0.0745 },
    {   61,   1.1475,  -2.7458,  -0.3883,   0.1000,   0.0835 },
    {   62,  -0.1967,  -1.4180,  -0.3988,   0.1000,   0.0920 },
    {   63,  -0.0131,  -1.2704,  -0.3856,   0.1000,   0.0690 },
    {   64,   0.2098,  -1.2049,  -0.3664,   0.1000,   0.0790 },
    {   65,   0.3410,  -1.3196,  -0.3652,   0.1000,   0.0565 },
    {   66,   0.5705,  -1.2704,  -0.3467,   0.1000,   0.0575 },
    {   67,   0.6360,  -1.4344,  -0.3532,   0.1000,   0.0755 },
    {   68,   0.9246,  -1.4180,  -0.3329,   0.1000,   0.0635 },
    {   69,   1.0623,  -1.3360,  -0.3183,   0.1000,   0.0550 },
    {   70,   1.2393,  -1.3934,  -0.3103,   0.1000,   0.0595 },
    {   71,   1.3639,  -1.4753,  -0.3079,   0.1000,   0.0560 },
    {   72,   1.4819,  -1.5983,  -0.3210,   0.1000,   0.0930 },
    {   73,   1.7835,  -1.5819,  -0.3065,   0.1000,   0.0745 },
    {   74,   1.9343,  -2.1065,  -0.3307,   0.1000,   0.0835 },
    {   75,   2.1245,  -2.3196,  -0.3314,   0.1000,   0.0920 },
    {   76,   2.2556,  -2.3032,  -0.3230,   0.1000,   0.0800 },
    {   77,   2.4196,  -2.3688,  -0.3165,   0.1000,   0.0625 },
    {   78,   1.7835,  -2.5327,  -0.3543,   0.1000,   0.0715 },
    {   79,   1.7180,  -2.8933,  -0.3742,   0.1000,   0.0945 },
    {   80,   1.9343,  -3.0409,  -0.3727,   0.1000,   0.0915 },
    {   81,   2.4524,  -3.4671,  -0.3900,   0.1000,   0.0685 },
    {   82,   2.4786,  -2.8851,  -0.3538,   0.1000,   0.0830 },
    {   83,   2.3343,  -2.6228,  -0.3420,   0.1000,   0.0830 },
    {   84,   2.8130,  -2.0737,  -0.2706,   0.1000,   0.0890 },
    {   85,   2.6360,  -1.8278,  -0.2661,   0.1000,   0.0975 },
    {   86,   2.3958,  -1.7130,  -0.2774,   0.2000,   0.1555 },
    {   87,   2.2688,  -1.2868,  -0.2646,   0.1000,   0.0835 },
    {   88,   2.4196,  -1.1147,  -0.2486,   0.1000,   0.0770 },
    {   89,   2.7802,  -2.3933,  -0.3017,   0.1000,   0.0655 },
    {   90,   3.0163,  -2.4179,  -0.2905,   0.1000,   0.0725 },
    {   91,   2.9310,  -2.2540,  -0.2798,   0.1000,   0.0910 },
    {   92,   2.6622,  -2.0983,  -0.2823,   0.1000,   0.0680 },
    {   93,   2.3147,  -1.9753,  -0.2973,   0.1000,   0.0620 },
    {   94,   2.1573,  -1.8770,  -0.3013,   0.1000,   0.0525 },
    {   95,   2.0196,  -1.7868,  -0.3044,   0.1000,   0.0970 },
    {   96,   2.7802,  -3.3031,  -0.3900,   0.1000,   0.0510 },
    {   97,   2.8589,  -3.1720,  -0.3900,   0.1000,   0.0755 },
    {   98,   3.0163,  -2.8114,  -0.3383,   0.1000,   0.0835 },
    {   99,   3.5081,  -2.4179,  -0.2558,   0.1000,   0.0770 },
    {  100,   3.5277,  -2.3196,  -0.2366,   0.1000,   0.0765 },
    {  101,   3.6654,  -2.5819,  -0.2566,   0.1000,   0.0805 },
    {  102,   3.7179,  -2.7622,  -0.2706,   0.1000,   0.0980 },
    {  103,   3.7769,  -2.4671,  -0.2339,   0.1000,   0.0640 },
    {  104,   3.3441,  -2.4671,  -0.2693,   0.1000,   0.0940 },
    { -1, 0, 0, 0, 0, 0 },
};

static Geometry *makeTree( _tree *tree, StateSet *dstate )
{
    float vv[][3] =
    {
        { -tree->w/2.0f, 0.0f, 0.0f },
        {  tree->w/2.0f, 0.0f, 0.0f },
        {  tree->w/2.0f, 0.0f, 2.0f * tree->h },
        { -tree->w/2.0f, 0.0f, 2.0f * tree->h },
    };

    Vec3Array& v = *(new Vec3Array(4));
    Vec2Array& t = *(new Vec2Array(4));
    Vec4Array& l = *(new Vec4Array(1));

    int   i;

    l[0][0] = l[0][1] = l[0][2] = l[0][3] = 1;

    for( i = 0; i < 4; i++ )
    {
        v[i][0] = vv[i][0];
        v[i][1] = vv[i][1];
        v[i][2] = vv[i][2];
    }

    t[0][0] = 0.0; t[0][1] = 0.0;
    t[1][0] = 1.0; t[1][1] = 0.0;
    t[2][0] = 1.0; t[2][1] = 1.0;
    t[3][0] = 0.0; t[3][1] = 1.0;

    Geometry *geom = new Geometry;

    geom->setVertexArray( &v );

    geom->setTexCoordArray( 0, &t );

    geom->setColorArray( &l, Array::BIND_OVERALL );

    geom->addPrimitiveSet( new DrawArrays(PrimitiveSet::QUADS,0,4) );

    geom->setStateSet( dstate );

    return geom;
}


static float ttx, tty;

static int ct( const void *a, const void *b )
{
    _tree *ta = (_tree *)a;
    _tree *tb = (_tree *)b;

    float da = sqrtf( sqr(ta->x - ttx) + sqr(ta->y - tty) );
    float db = sqrtf( sqr(tb->x - ttx) + sqr(tb->y - tty) );

    if( da < db )
        return -1;
    else
        return 1;
}


Node *makeTrees( void )
{
    Group *group = new Group;
    int i;

    getDatabaseCenterRadius( dbcenter, &dbradius );
    struct _tree  *t;

    Texture2D *tex = new Texture2D;
    tex->setImage(osgDB::readImageFile("Images/tree0.rgba"));

    StateSet *dstate = new StateSet;

    dstate->setTextureAttributeAndModes(0, tex, StateAttribute::ON );
    dstate->setTextureAttribute(0, new TexEnv );

    dstate->setAttributeAndModes( new BlendFunc, StateAttribute::ON );

    AlphaFunc* alphaFunc = new AlphaFunc;
    alphaFunc->setFunction(AlphaFunc::GEQUAL,0.05f);
    dstate->setAttributeAndModes( alphaFunc, StateAttribute::ON );

    dstate->setMode( GL_LIGHTING, StateAttribute::OFF );

    dstate->setRenderingHint( StateSet::TRANSPARENT_BIN );

    int tt[] = { 15, 30, 45, 58, 72, 75, 93, 96, 105, -1 };
    int *ttp = tt;

    i = 0;
    while( i < 105 )
    {
        ttx = trees[i].x;
        tty = trees[i].y;
        qsort( &trees[i], 105 - i, sizeof( _tree ), ct );

        i += *ttp;
        ttp++;
    }

    t = trees;
    i = 0;
    ttp = tt;
    while( *ttp != -1 )
    {
        Billboard *bb = new Billboard;
        //int starti = i;

        for( ; i < (*ttp); i++ )
        {
            t->x -= 0.3f;
            float  h = Hat(t->x, t->y, t->z );
            Vec3 pos( t->x, t->y, t->z-h );
            Geometry *geom = makeTree( t, dstate );
            bb->addDrawable( geom, pos );
            t++;
        }
        group->addChild( bb );
        ttp++;
    }

    return group;
}
