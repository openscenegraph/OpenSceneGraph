#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "vector.h"

#ifdef WIN32
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#define SET_VEC( v, a, b, c )    v->x = (a); v->y = (b); v->z = (c) 

Vector NewVector( void )
{
    Vector temp;

    temp = (Vector)calloc( 1, sizeof( struct _vector ) );

    SET_VEC( temp, 0., 0., 0. );

    return temp;
}

void DeleteVector( Vector v )
{
    SET_VEC( v, 0., 0., 0. );
    free( v ); 
}

void VectorSet( Vector v, float a, float b, float c )
{
    SET_VEC( v, a, b, c );
}

void VectorGet( Vector v, float *a, float *b, float *c )
{
    *a = v->x;
    *b = v->y;
    *c = v->z;
}

float VectorDotProduct( Vector p, Vector q )
{
    return (p->x * p->x) + (p->y * p->y) + (p->z * p->z);
}


void VectorNormalize( Vector v )
{
    float f;

    f = sqrt( (v->x*v->x)+(v->y*v->y)+(v->z*v->z));

    if( f == 0.0 )
        v->x = v->y = v->z = 0.0;
    else
    {
        v->x /= f;
        v->y /= f;
        v->z /= f;
    }
}

void VectorCrossProduct(Vector v1, Vector v2, Vector prod)
{
    struct _vector p;  /* in case prod == v1 or v2 */

    p.x = v1->y*v2->z - v2->y*v1->z;
    p.y = v1->z*v2->x - v2->z*v1->x;
    p.z = v1->x*v2->y - v2->x*v1->y;
    prod->x = p.x; prod->y = p.y; prod->z = p.z;
}

void VectorAdd( Vector v1, Vector v2, Vector sum )
{
    sum->x = v1->x + v2->x;
    sum->y = v1->y + v2->y;
    sum->z = v1->z + v2->z;
}

void VectorDiff( Vector v1, Vector v2, Vector diff )
{
    diff->x = v1->x - v2->x;
    diff->y = v1->y - v2->y;
    diff->z = v1->z - v2->z;
}

float VectorAngle( Vector v1, Vector v2 )
{
    float d;
    float f1, f2;

    d = ((v1->x*v2->x) + (v1->y*v2->y) + (v1->z*v2->z));

    f1 = sqrt( (v1->x*v1->x) + (v1->y *v1->y) + (v1->z*v1->z) );
    f2 = sqrt( (v2->x*v2->x) + (v2->y *v2->y) + (v2->z*v2->z) );

    if( f1*f2 == 0.0 ) return 0.0;

    return acos( d/(f1*f2) );
}

char * VectorToA( Vector v, char *ptr, int size )
{
    char buff[512];

    sprintf( buff, "%12.4f %12.4f %12.4f", v->x, v->y, v->z );

    strncpy( ptr, buff, size );

    return ptr;
}

float VectorMagnitude( Vector v )
{
    return sqrt( (v->x*v->x) + (v->y *v->y) + (v->z*v->z) );
}

void VectorMagnify( Vector v, float magnitude, Vector res )
{
    float x;
    float d;

    x = magnitude;
    d = sqrt( (v->x*v->x) + (v->y*v->y) + (v->z*v->z) );

    if( d == 0.0 )
    {
        res->x = res->y = res->z = 0.0;
    }
    else
    {
        res->x = x * v->x/d;
        res->y = x * v->y/d;
        res->z = x * v->z/d;
    }
}
