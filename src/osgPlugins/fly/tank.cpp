#include <math.h>
#include <osg/GL>
#include <osg/OSG>
#include <osg/Group>
#include <osg/Geode>
#include <osg/GeoSet>
#include <osg/Texture>
#include <osg/GeoState>
#include <osg/DCS>
#include <osg/Registry>

#ifdef WIN32
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
	t[i] = 	(a[0] * mat._mat[0][i]) +
		(a[1] * mat._mat[1][i]) +
		(a[2] * mat._mat[2][i]) +
		mat._mat[3][i];
    }
    b[0] = t[0];
    b[1] = t[1];
    b[2] = t[2];
}

Node *makeTank( void )
{

    Geode *geode1 = new Geode;

    getDatabaseCenterRadius( dbcenter, &dbradius );

    ref_ptr<Matrix> mat = new Matrix(
    	0.05, 0, 0, 0,
	0, 0.05, 0, 0, 
	0, 0, 0.05, 0,
	1.5999 - 0.3,
	3.1474,
	dbcenter[2] + 0.6542 - 0.09,
	1
    );

    Vec3 *vc = new Vec3[42];
    Vec2 *tc = new Vec2[42];
    int *lens = new int[1];
    int i, c;

    c = 0;
    for( i = 0; i <= 360; i += 18 )
    {
        float x, y, z;
        float s, t;
        float theta = (float)i * M_PI/180.0;

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
    *lens = 42;

    for( i = 0; i < c; i++ )
	conv( vc[i], *mat, vc[i] );

    GeoSet *gset = new GeoSet;
    gset->setCoords( vc );

    gset->setTextureCoords( tc );
    gset->setTextureBinding( GeoSet::BIND_PERVERTEX );

    gset->setPrimType( GeoSet::TRIANGLE_STRIP );
    gset->setNumPrims( 1 );
    gset->setPrimLengths( lens );

    Texture *tex = new Texture;

    tex->setWrap( Texture::WRAP_S, Texture::REPEAT );
    tex->setWrap( Texture::WRAP_T, Texture::REPEAT );
    tex->setImage(Registry::instance()->readImage( "tank.rgb"));

    GeoState *gstate = new GeoState;
    gstate->setMode(     GeoState::TEXTURE, GeoState::ON );
    gstate->setAttribute( GeoState::TEXTURE, tex );
    gstate->setAttribute( GeoState::TEXENV, new TexEnv );

    gset->setGeoState( gstate );
    geode1->addGeoSet( gset );

    lens = new int[1];
    *lens = 22;

    vc = new Vec3[22];
    tc = new Vec2[22];

    c = 0;
    vc[c][0] = 0;
    vc[c][1] = 0;
    vc[c][2] = 1;
    c++;

    for( i = 0; i <= 360; i += 18 )
    {
        float x, y, z;
        float s, t;
        float theta = (float)i * M_PI/180.0;

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
	conv( vc[i], *mat, vc[i] );

    gset = new GeoSet;
    gset->setCoords( vc );
    gset->setTextureCoords( tc );
    gset->setPrimType(GeoSet::TRIANGLE_FAN);
    gset->setNumPrims( 1 );
    gset->setPrimLengths( lens );

    gset->setGeoState( gstate );

    Geode *geode2 = new Geode;
    geode2->addGeoSet( gset );



    //DCS *dcs = new DCS(mat);
    //dcs->addChild( geode2 );


    Group *grp = new Group;
    grp->addChild( geode1 );
    grp->addChild( geode2 );

    geode1->setName( "Geode 1" );
    geode2->setName( "Geode 2" );

    return grp;
}
