// #include <math.h>

#include <osg/Geode>
#include <osg/GeoSet>
#include <osg/Texture>
#include <osg/TexEnv>
#include <osg/StateSet>

#include <osgDB/ReadFile>

#include "terrain_data.h"

using namespace osg;

void getDatabaseCenterRadius( float dbcenter[3], float *dbradius )
{
    int i;
    double n=0.0;
    double center[3] = { 0.0f, 0.0f, 0.0f };
    float  cnt;

    cnt = 39 * 38;
    for( i = 0; i < cnt; i++ )
    {
        center[0] += (double)vertex[i][0];
        center[1] += (double)vertex[i][1];
        center[2] += (double)vertex[i][2];

        n = n + 1.0;
    }

    center[0] /= n;
    center[1] /= n;
    center[2] /= n;

    float r = 0.0;

    //    for( i = 0; i < sizeof( vertex ) / (sizeof( float[3] )); i++ )
    for( i = 0; i < cnt; i++ )
    {
        double d = sqrt(
            (((double)vertex[i][0] - center[0]) * ((double)vertex[i][0] - center[0])) +
            (((double)vertex[i][1] - center[1]) * ((double)vertex[i][1] - center[1])) +
            (((double)vertex[i][2] - center[2]) * ((double)vertex[i][2] - center[2]))  );

        if( d > (double)r ) r = (float)d;

    }

    *dbradius = r;
    dbcenter[0] = (float)center[0];
    dbcenter[1] = (float)center[1];
    dbcenter[2] = (float)center[2];

    int index = 19 * 39 + 19;
    dbcenter[0] = vertex[index][0] - 0.15;
    dbcenter[1] = vertex[index][1];
    dbcenter[2] = vertex[index][2] + 0.35;

}


Node *makeTerrain( void )
{
    int m, n;
    int  i, j, c;
    float dbcenter[3];
    float dbradius;

    getDatabaseCenterRadius( dbcenter, &dbradius );

    m = (sizeof( vertex ) /(sizeof( float[3])))/39;
    n = 39;

    Vec3 *v    = new Vec3[m*n];
    Vec2 *t    = new Vec2[m*n];
    Vec4 *col  = new Vec4[1];
    osg::ushort *cidx = new osg::ushort[1];
    osg::ushort *idx = new osg::ushort[m*n*2];
    int    *lens = new int[m];

    col[0][0] = col[0][1] = col[0][2] = col[0][3] = 1;
    *cidx = 0;

    for( i = 0; i < m * n; i++ )
    {
        v[i][0] = vertex[i][0] - dbcenter[0];
        v[i][1] = vertex[i][1] - dbcenter[1];
        v[i][2] = vertex[i][2];

        t[i][0] = texcoord[i][0] + 0.025;
        t[i][1] = texcoord[i][1];
    }

    c = 0;
    for( i = 0; i < m-2; i++ )
    {
        for( j = 0; j < n; j++ )
        {
            idx[c++] = (i+0)*n+j;
            idx[c++] = (i+1)*n+j;
        }
        lens[i] = 39*2;
    }

    GeoSet *gset = new GeoSet;

    gset->setCoords( v, idx );
    gset->setTextureCoords( t, idx );
    gset->setTextureBinding( GeoSet::BIND_PERVERTEX );

    gset->setColors( col, cidx );
    gset->setColorBinding( GeoSet::BIND_OVERALL );

    gset->setPrimType( GeoSet::TRIANGLE_STRIP );
    gset->setNumPrims( m-2 );
    gset->setPrimLengths( lens );

    Texture *tex = new Texture;

    tex->setImage(osgDB::readImageFile("Images/lz.rgb"));

    StateSet *dstate = new StateSet;
    dstate->setMode( GL_LIGHTING, StateAttribute::OFF );
    dstate->setAttributeAndModes( tex, StateAttribute::ON );
    dstate->setAttribute( new TexEnv );

    gset->setStateSet( dstate );

    Geode *geode = new Geode;
    geode->addDrawable( gset );

    gset->check();

    return geode;
}
