#include <math.h>

#include <osg/Geode>
#include <osg/GeoSet>
#include <osg/Texture>
#include <osg/TexEnv>
#include <osg/Depth>
#include <osg/StateSet>

#include <osgDB/ReadFile>

using namespace osg;

Node *makeBase( void )
{
    int i, c;
    float theta;
    float ir, ori;
    ir = 0.0;
    ori = 20.0;

    Vec3 *coords = new Vec3[38];
    Vec2 *tcoords = new Vec2[38];
    Vec4 *colors = new Vec4[1];
    int *lengths = new int[1];

    colors[0][0] = colors[0][1] = colors[0][2] = colors[0][3] = 1;

    c = 0;
    for( i = 0; i <= 18; i++ )
    {
        theta = osg::DegreesToRadians((float)i * 20.0);

        coords[c][0] = ir * cosf( theta );
        coords[c][1] = ir * sinf( theta );
        coords[c][2] = 0.0;
        tcoords[c][0] = coords[c][0]/36.;
        tcoords[c][1] = coords[c][1]/36.;

        c++;

        coords[c][0] = ori * cosf( theta );
        coords[c][1] = ori * sinf( theta );
        coords[c][2] = 0.0f;

        tcoords[c][0] = coords[c][0]/36.;
        tcoords[c][1] = coords[c][1]/36.;

        c++;
    }
    *lengths = 38;

    GeoSet *gset = new GeoSet;

    gset->setCoords( coords );

    gset->setTextureCoords( tcoords );
    gset->setTextureBinding( GeoSet::BIND_PERVERTEX );

    gset->setColors( colors );
    gset->setColorBinding( GeoSet::BIND_OVERALL );

    gset->setPrimType( GeoSet::TRIANGLE_STRIP );
    gset->setNumPrims( 1 );
    gset->setPrimLengths( lengths );

    Texture *tex = new Texture;

    tex->setImage(osgDB::readImageFile("Images/water.rgb"));
    tex->setWrap( Texture::WRAP_S, Texture::REPEAT );
    tex->setWrap( Texture::WRAP_T, Texture::REPEAT );

    StateSet *dstate = new StateSet;
    dstate->setMode( GL_LIGHTING, StateAttribute::OFF );
    dstate->setAttributeAndModes( tex, StateAttribute::ON );

    dstate->setAttribute( new TexEnv );
    /*
        pfFog *fog = new pfFog;

        fog->setFogType( PFFOG_PIX_EXP2 );
        fog->setColor(   0.1, 0.2, 0.2 );
        fog->setRange(   16.0, 22.0 );

        gstate->setMode( PFSTATE_ENFOG, PF_ON );
        gstate->setAttr( PFSTATE_FOG, fog );
    */

    // clear the depth to the far plane.
    osg::Depth* depth = new osg::Depth;
    depth->setFunction(osg::Depth::ALWAYS);
    depth->setRange(1.0,1.0);   
    dstate->setAttributeAndModes(depth,StateAttribute::ON );

    dstate->setRenderBinDetails(-1,"RenderBin");


    gset->setStateSet( dstate );

    Geode *geode = new Geode;
    geode->addDrawable( gset );

    return geode;
}
