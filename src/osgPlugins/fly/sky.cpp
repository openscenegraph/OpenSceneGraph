#include <math.h>

#include <osg/OSG>
#include <osg/Geode>
#include <osg/GeoSet>
#include <osg/Texture>
#include <osg/GeoState>
#include <osg/Registry>

#ifdef WIN32
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

using namespace osg;

Node *makeSky( void )
{
    int i, j;
    float lev[] = { -5, -1.0, 1.0, 15.0, 30.0, 60.0, 90.0  };
    float cc[][4] = {
        { 0.0, 0.0, 0.15 },
        { 0.0, 0.0, 0.15 },
        { 0.4, 0.4, 0.7 },
        { 0.2, 0.2, 0.6 },
        { 0.1, 0.1, 0.6 },
        { 0.1, 0.1, 0.6 },
        { 0.1, 0.1, 0.6 },
    };
    float x, y, z;
    float alpha, theta;
    float radius = 20.0f;
    int nlev = sizeof( lev )/sizeof(float);

    Vec3 *coords = new Vec3[19*nlev];
    Vec4 *colors = new Vec4[19*nlev];
    Vec2 *tcoords = new Vec2[19*nlev];
    osg::ushort *idx = new osg::ushort[38* nlev];
    int *lengths = new int[nlev];
    

    int ci, ii;
    ii = ci = 0;

    for( i = 0; i < nlev; i++ )
    {
        for( j = 0; j <= 18; j++ )
        {
            alpha = lev[i] * M_PI/180.0;
            theta = (float)(j*20) * M_PI/180.0;

            x = radius * cosf( alpha ) * cosf( theta );
            y = radius * cosf( alpha ) * -sinf( theta );
            z = radius * sinf( alpha );

	    coords[ci][0] = x;
	    coords[ci][1] = y;
	    coords[ci][2] = z;
	    
	    colors[ci][0] = cc[i][0];
	    colors[ci][1] = cc[i][1];
	    colors[ci][2] = cc[i][2];
	    colors[ci][3] = 1.0;


	    tcoords[ci][0] = (float)j/18.0; 
	    tcoords[ci][0] = (float)i/(float)(nlev-1);

            ci++;

            idx[ii++] = ((i+1)*19+j);
            idx[ii++] = ((i+0)*19+j);
        }
        lengths[i] = 38;
    }

    GeoSet *gset = new GeoSet;

    gset->setCoords( coords, idx );
    gset->setTextureCoords( tcoords, idx );
    gset->setTextureBinding( GeoSet::BIND_PERVERTEX );

    gset->setColors( colors, idx );
    gset->setColorBinding( GeoSet::BIND_PERVERTEX );

    gset->setPrimType( GeoSet::TRIANGLE_STRIP );
    gset->setNumPrims( nlev - 1 );
    gset->setPrimLengths( lengths );

    Texture *tex = new Texture;
    tex->setImage(Registry::instance()->readImage( "white.rgb"));

    GeoState *gstate = new GeoState;
//    gstate->setMode( GeoState::TEXTURE, GeoState::ON );
    gstate->setMode( GeoState::TEXTURE, GeoState::OFF);
    gstate->setAttribute( GeoState::TEXTURE, tex );
    gstate->setAttribute( GeoState::TEXENV, new TexEnv );
    gstate->setMode( GeoState::LIGHTING, GeoState::OFF );
    gstate->setMode( GeoState::FACE_CULL, GeoState::ON );

    gset->setGeoState( gstate );

    Geode *geode = new Geode;
    geode->addGeoSet( gset );

    geode->setName( "Sky" );

    return geode;
}
