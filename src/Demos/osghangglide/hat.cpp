#ifdef _MSC_VER
#include <Windows.h>
#pragma warning( disable : 4244 )
#endif

#include <osg/GL>
#include <osg/Math>
#include <stdio.h>

#include "terrain_coords.h"
#include "hat.h"

static int inited = 0;

static float dbcenter[3];
static float dbradius;

static void getDatabaseCenterRadius( float dbcenter[3], float *dbradius )
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


static void init( void )
{
    getDatabaseCenterRadius( dbcenter, &dbradius );
    inited = 1;
}


static void getNormal( float *v1, float *v2, float *v3, float *n )
{
    float V1[4], V2[4];
    float   f;
    int i;

    /* Two vectors v2->v1 and v2->v3 */

    for( i = 0; i < 3; i++ )
    {
        V1[i] = v1[i] - v2[i];
        V2[i] = v3[i] - v2[i];
    }

    /* Cross product between V1 and V2 */

    n[0] = (V1[1] * V2[2]) - (V1[2] * V2[1]);
    n[1] = -((V1[0] * V2[2]) - ( V1[2] * V2[0] ));
    n[2] = (V1[0] * V2[1] ) - (V1[1] * V2[0] );

    /* Normalize */

    f = sqrtf( ( n[0] * n[0] ) + ( n[1] * n[1] ) + ( n[2] * n[2] ) );
    n[0] /= f;
    n[1] /= f;
    n[2] /= f;
}


float Hat( float x, float y, float z )
{
    int m, n;
    int i, j;
    float tri[3][3];
    float norm[3];
    float d, pz;

    if( inited == 0 ) init();

    // m = columns
    // n = rows
    m = (sizeof( vertex ) /(sizeof( float[3])))/39;
    n = 39;

    i = 0;
    while( i < ((m-1)*39) && x > (vertex[i+n][0] - dbcenter[0]) )
        i += n;

    j = 0;

    while( j < n-1 && y > (vertex[i+j+1][1] - dbcenter[1]) )
        j++;

    tri[0][0] = vertex[i+0+j+0][0] - dbcenter[0];
    tri[0][1] = vertex[i+0+j+0][1] - dbcenter[1];
    //tri[0][2] = vertex[i+0+j+0][2] - dbcenter[2];
    tri[0][2] = vertex[i+0+j+0][2];

    tri[1][0] = vertex[i+n+j+0][0] - dbcenter[0];
    tri[1][1] = vertex[i+n+j+0][1] - dbcenter[1];
    //tri[1][2] = vertex[i+n+j+0][2] - dbcenter[2];
    tri[1][2] = vertex[i+n+j+0][2];

    tri[2][0] = vertex[i+0+j+1][0] - dbcenter[0];
    tri[2][1] = vertex[i+0+j+1][1] - dbcenter[1];
    //tri[2][2] = vertex[i+0+j+1][2] - dbcenter[2];
    tri[2][2] = vertex[i+0+j+1][2];

    getNormal( tri[0], tri[1], tri[2], norm );

    d = (tri[0][0] * norm[0]) +
        (tri[0][1] * norm[1]) +
        (tri[0][2] * norm[2]);

    d *= -1;
    pz = (-(norm[0] * x) - (norm[1] * y) - d)/norm[2];

    return z - pz;
}
