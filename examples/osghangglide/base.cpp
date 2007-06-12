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

#include <math.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/Depth>
#include <osg/StateSet>

#include <osgDB/ReadFile>

using namespace osg;

Node *makeBase( void )
{
    int i, c;
    float theta;
    float ir = 20.0f;

    Vec3Array *coords = new Vec3Array(19);
    Vec2Array *tcoords = new Vec2Array(19);
    Vec4Array *colors = new Vec4Array(1);

    (*colors)[0].set(1.0f,1.0f,1.0f,1.0f);

    c = 0;
    (*coords)[c].set(0.0f,0.0f,0.0f);
    (*tcoords)[c].set(0.0f,0.0f);
    
    for( i = 0; i <= 18; i++ )
    {
        theta = osg::DegreesToRadians((float)i * 20.0);

        (*coords)[c].set(ir * cosf( theta ), ir * sinf( theta ), 0.0f);
        (*tcoords)[c].set((*coords)[c][0]/36.0f,(*coords)[c][1]/36.0f);

        c++;
    }

    Geometry *geom = new Geometry;

    geom->setVertexArray( coords );

    geom->setTexCoordArray( 0, tcoords );

    geom->setColorArray( colors );
    geom->setColorBinding( Geometry::BIND_OVERALL );

    geom->addPrimitiveSet( new DrawArrays(PrimitiveSet::TRIANGLE_FAN,0,19) );

    Texture2D *tex = new Texture2D;

    tex->setImage(osgDB::readImageFile("Images/water.rgb"));
    tex->setWrap( Texture2D::WRAP_S, Texture2D::REPEAT );
    tex->setWrap( Texture2D::WRAP_T, Texture2D::REPEAT );

    StateSet *dstate = new StateSet;
    dstate->setMode( GL_LIGHTING, StateAttribute::OFF );
    dstate->setTextureAttributeAndModes(0, tex, StateAttribute::ON );

    dstate->setTextureAttribute(0, new TexEnv );

    // clear the depth to the far plane.
    osg::Depth* depth = new osg::Depth;
    depth->setFunction(osg::Depth::ALWAYS);
    depth->setRange(1.0,1.0);   
    dstate->setAttributeAndModes(depth,StateAttribute::ON );

    dstate->setRenderBinDetails(-1,"RenderBin");


    geom->setStateSet( dstate );

    Geode *geode = new Geode;
    geode->addDrawable( geom );

    return geode;
}
