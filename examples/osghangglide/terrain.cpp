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

// #include <math.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/StateSet>

#include <osgDB/ReadFile>

#include "terrain_coords.h"
#include "terrain_texcoords.h"

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
    int  i, j;
    float dbcenter[3];
    float dbradius;

    getDatabaseCenterRadius( dbcenter, &dbradius );

    m = (sizeof( vertex ) /(sizeof( float[3])))/39;
    n = 39;

    Vec3Array& v    = *(new Vec3Array(m*n));
    Vec2Array& t    = *(new Vec2Array(m*n));
    Vec4Array& col  = *(new Vec4Array(1));

    col[0][0] = col[0][1] = col[0][2] = col[0][3] = 1.0f;

    for( i = 0; i < m * n; i++ )
    {
        v[i][0] = vertex[i][0] - dbcenter[0];
        v[i][1] = vertex[i][1] - dbcenter[1];
        v[i][2] = vertex[i][2];

        t[i][0] = texcoord[i][0] + 0.025;
        t[i][1] = texcoord[i][1];
    }

    Geometry *geom = new Geometry;

    geom->setVertexArray( &v );
    geom->setTexCoordArray( 0, &t );

    geom->setColorArray( &col, Array::BIND_OVERALL );

    for( i = 0; i < m-2; i++ )
    {
        DrawElementsUShort* elements = new DrawElementsUShort(PrimitiveSet::TRIANGLE_STRIP);
        elements->reserve(39*2);
        for( j = 0; j < n; j++ )
        {
            elements->push_back((i+0)*n+j);
            elements->push_back((i+1)*n+j);
        }
        geom->addPrimitiveSet(elements);
    }


    Texture2D *tex = new Texture2D;

    tex->setImage(osgDB::readImageFile("Images/lz.rgb"));

    StateSet *dstate = new StateSet;
    dstate->setMode( GL_LIGHTING, StateAttribute::OFF );
    dstate->setTextureAttributeAndModes(0, tex, StateAttribute::ON );
    dstate->setTextureAttribute(0, new TexEnv );

    geom->setStateSet( dstate );

    Geode *geode = new Geode;
    geode->addDrawable( geom );

    return geode;
}
