#include <string.h>
#include <math.h>
#include "matrix.h"
#include <osg/Types>

static float ID[4][4] = {
    { 1.0, 0.0, 0.0, 0.0 },
    { 0.0, 1.0, 0.0, 0.0 },
    { 0.0, 0.0, 1.0, 0.0 },
    { 0.0, 0.0, 0.0, 1.0 },
};


static float M[2][4][4];
static int   mi = 0;
static float R[4][4];
static float T[4][4];
static float S[4][4];

static void preMultiply( float m[4][4] )
{
    int i, j, k;
    int mo;

    mo = 1 - mi;

    for( i = 0; i < 4; i++ )
    {
        for( j = 0; j < 4; j++ )
        {
            M[mo][i][j] = 0.0;
            for( k = 0; k < 4; k++ )
                M[mo][i][j] += m[i][k] * M[mi][k][j];
        }
    }

    mi = mo;
}

void m_LoadID( void )
{
    memcpy( M[mi], ID, sizeof( float ) * 16 );
}


void m_Rotate( float a, char axis )
{
    float theta;

    theta = a * M_PI/180.0;

    memcpy( R, ID, sizeof( float) * 16 );
    switch( axis )
    {
        case 'x' :
            R[1][1] =  cosf( theta );
            R[1][2] = -sinf( theta );
            R[2][1] =  sinf( theta );
            R[2][2] =  cosf( theta );
            break;    

        case 'y' :
            R[0][0] = cosf( theta );
            R[0][2] = -sinf( theta );
            R[2][0] = sinf( theta );
            R[2][2] = cosf( theta );
            break;

        case 'z' :
            R[0][0] = cosf( theta );
            R[0][1] = -sinf( theta );
            R[1][0] =  sinf( theta );
            R[1][1] = cosf( theta );
            break;
    } 

    preMultiply( R );
}

void m_Translate( float x, float y, float z )
{
    memcpy( T, ID, sizeof( float) * 16 );

    T[3][0] = x;
    T[3][1] = y;
    T[3][2] = z;

    preMultiply( T );
}

void m_Scale( float x, float y, float z )
{
    memcpy( S, ID, sizeof( float ) * 16 );

    S[0][0] = x;
    S[1][1] = y;
    S[2][2] = z;

    preMultiply( S );
}


void m_MultV( float v[4], float r[4] )
{
    int i, j;

    for( i = 0; i < 4; i++ )
    {
        r[i] = 0.0;
        for( j = 0; j < 4; j++ )
            r[i] += v[j] * M[mi][j][i];
    }
}
