#include <math.h>

#include <osg/GL>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/StateSet>
#include <osg/Matrix>

#include <osgDB/ReadFile>

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

using namespace osg;

extern void getDatabaseCenterRadius( float dbcenter[3], float *dbradius );

static float radius = 2.0;
static float dbcenter[3], dbradius;

static void conv( const Vec3& a, const Matrix& mat, Vec3& b )
{
    int i;
    Vec3 t;

    for( i = 0; i < 3; i++ )
    {
        t[i] =  (a[0] * mat(0,i)) +
            (a[1] * mat(1,i)) +
            (a[2] * mat(2,i)) +
            mat(3,i);
    }
    b[0] = t[0];
    b[1] = t[1];
    b[2] = t[2];
}


Node *makeTank( void )
{

    Geode *geode = new Geode;

    getDatabaseCenterRadius( dbcenter, &dbradius );

    Matrix mat(
        0.05, 0, 0, 0,
        0, 0.05, 0, 0,
        0, 0, 0.05, 0,
        1.5999 - 0.3,
        3.1474,
        dbcenter[2] + 0.6542 - 0.09,
        1
        );

    // 42 required for sodes, 22 for the top.
    Vec3Array& vc = *(new Vec3Array(42+22));
    Vec2Array& tc = *(new Vec2Array(42+22));

    Geometry *gset = new Geometry;
    gset->setVertexArray( &vc );
    gset->setTexCoordArray( 0, &tc );

    // create the sides of the tank.
    unsigned int i, c = 0;
    for( i = 0; i <= 360; i += 18 )
    {
        float x, y, z;
        float s, t;
        float theta = osg::DegreesToRadians((float)i);

        s = (float)i/90.0;
        t = 1.0;

        x = radius * cosf( theta );
        y = radius * sinf( theta );
        z = 1.0;

        vc[c][0] = x;
        vc[c][1] = y;
        vc[c][2] = z;

        tc[c][0] = s;
        tc[c][1] = t;

        c++;

        t = 0.0;
        z = 0.0;

        vc[c][0] = x;
        vc[c][1] = y;
        vc[c][2] = z;

        tc[c][0] = s;
        tc[c][1] = t;
        c++;
    }

    gset->addPrimitiveSet( new DrawArrays(PrimitiveSet::TRIANGLE_STRIP,0,c) );

    // create the top of the tank.

    int prev_c = c;

    vc[c][0] = 0.0f;
    vc[c][1] = 0.0f;
    vc[c][2] = 1.0f;

    tc[c][0] = 0.0f;
    tc[c][1] = 0.0f;
    c++;

    for( i = 0; i <= 360; i += 18 )
    {
        float x, y, z;
        float s, t;
        float theta = osg::DegreesToRadians((float)i);

        //    s = (float)i/360.0;
        //   t = 1.0;
        s = cosf( theta );
        t = sinf( theta );

        x = radius * cosf( theta );
        y = radius * sinf( theta );
        z = 1.0;

        vc[c][0] = x;
        vc[c][1] = y;
        vc[c][2] = z;

        tc[c][0] = s;
        tc[c][1] = t;

        c++;
    }

    for( i = 0; i < c; i++ )
        conv( vc[i], mat, vc[i] );
     
     gset->addPrimitiveSet(new DrawArrays(PrimitiveSet::TRIANGLE_FAN,prev_c,c-prev_c));




    Texture2D *tex = new Texture2D;

    tex->setWrap( Texture2D::WRAP_S, Texture2D::REPEAT );
    tex->setWrap( Texture2D::WRAP_T, Texture2D::REPEAT );
    tex->setImage(osgDB::readImageFile("Images/tank.rgb"));

    StateSet *dstate = new StateSet;
    dstate->setTextureAttributeAndModes(0, tex, StateAttribute::ON );
    dstate->setTextureAttribute(0, new TexEnv );

    gset->setStateSet( dstate );
    geode->addDrawable( gset );

    return geode;
}
