// simple animation demo written by Graeme Harkness.
// note from Robert to Robert.  The animiation techinque
// present here using glut timer callbacks is one
// approach, other approaches will soon be supported
// within the osg itself via an app cullback which
// can be attached to nodes themselves. This later
// method will be the prefered approach (have a look
// at osgreflect's app visitor for a clue.)


#include <osg/Geode>
#include <osg/GeoSet>
#include <osg/Material>
#include <osg/Vec3>
#include <osg/Transform>

#include <osgUtil/TrackballManipulator>

#include <osgGLUT/Viewer>

#include <GL/glut.h>

#include <math.h>

// ----------------------------------------------------------------------
// Global variables - this is basically the stuff which will be animated
// ----------------------------------------------------------------------
osg::Transform* myTransform;
osg::GeoSet* cube;

int mytime=0;                    // in milliseconds
unsigned int dt=50;              // in milliseconds
double omega=0.002;              // in inverse milliseconds

// ----------------------------------------------------------------
// This is the callback function registered with GLUT to be called
// at a future time in the GLUT loop
// ----------------------------------------------------------------
void timedCB( int delta_t )
{

    static float lastdx = 0;
    static float lastdy = 0;
    static float lastdz = 0;

    mytime+=dt;

    // ---------------------------------------------------------
    // Update the Transform so that the cube will appear to oscillate
    // ---------------------------------------------------------
    double dx = 0.5 * cos( (double) omega * (double) mytime );
    double dy = 0.5 * sin( (double) omega * (double) mytime );
    double dz = 0.0;
    myTransform->preTranslate( -lastdx, -lastdy, -lastdz );
    myTransform->preTranslate( (float) dx, (float) dy, dz );
    lastdx=dx; lastdy=dy; lastdz=dz;

    // Graeme, commeted out this call as the cube itself remains static.
    // cube->dirtyDisplayList();

    // -------------------------------------------
    // If required, reschedule the timed callback
    // -------------------------------------------
    if ( delta_t != 0 )
    {
        glutTimerFunc( (unsigned int) delta_t, timedCB, delta_t );
    }
}


osg::Geode* createCube()
{
    osg::Geode* geode = new osg::Geode();

    // -------------------------------------------
    // Set up a new GeoSet which will be our cube
    // -------------------------------------------
    cube = new osg::GeoSet();

    // set up the primitives
    cube->setPrimType( osg::GeoSet::POLYGON );
    cube->setNumPrims( 6 );      // the six square faces

    // set up the primitive indices
    int* cubeLengthList = new int[6];
    cubeLengthList[0] = 4; // each side of the cube has 4 vertices
    cubeLengthList[1] = 4;
    cubeLengthList[2] = 4;
    cubeLengthList[3] = 4;
    cubeLengthList[4] = 4;
    cubeLengthList[5] = 4;

    cube->setPrimLengths( cubeLengthList );


    // set up the coordinates.
    osg::Vec3 *cubeCoords = new osg::Vec3[24];
    cubeCoords[0].set( -1.0000f,   1.0000f,  -1.000f );
    cubeCoords[1].set( 1.0000f,   1.0000f,  -1.0000f );
    cubeCoords[2].set( 1.0000f,  -1.0000f,  -1.0000f );
    cubeCoords[3].set( -1.0000f,  -1.0000f,  -1.000 );

    cubeCoords[4].set( 1.0000f,   1.0000f,  -1.0000f );
    cubeCoords[5].set( 1.0000f,   1.0000f,   1.0000f );
    cubeCoords[6].set( 1.0000f,  -1.0000f,   1.0000f );
    cubeCoords[7].set( 1.0000f,  -1.0000f,  -1.0000f );

    cubeCoords[8].set( 1.0000f,   1.0000f,   1.0000f );
    cubeCoords[9].set( -1.0000f,   1.0000f,   1.000f );
    cubeCoords[10].set( -1.0000f,  -1.0000f,   1.000f );
    cubeCoords[11].set( 1.0000f,  -1.0000f,   1.0000f );

    cubeCoords[12].set( -1.0000f,   1.0000f,   1.000 );
    cubeCoords[13].set( -1.0000f,   1.0000f,  -1.000 );
    cubeCoords[14].set( -1.0000f,  -1.0000f,  -1.000 );
    cubeCoords[15].set( -1.0000f,  -1.0000f,   1.000 );

    cubeCoords[16].set( -1.0000f,   1.0000f,   1.000 );
    cubeCoords[17].set( 1.0000f,   1.0000f,   1.0000f );
    cubeCoords[18].set( 1.0000f,   1.0000f,  -1.0000f );
    cubeCoords[19].set( -1.0000f,   1.0000f,  -1.000f );

    cubeCoords[20].set( -1.0000f,  -1.0000f,   1.000f );
    cubeCoords[21].set( -1.0000f,  -1.0000f,  -1.000f );
    cubeCoords[22].set( 1.0000f,  -1.0000f,  -1.0000f );
    cubeCoords[23].set( 1.0000f,  -1.0000f,   1.0000f );

    cube->setCoords( cubeCoords );
    
    
    // set up the normals.
    osg::Vec3 *cubeNormals = new osg::Vec3[6];
    cubeNormals[0].set(0.0f,0.0f,-1.0f);
    cubeNormals[1].set(1.0f,0.0f,0.0f);
    cubeNormals[2].set(0.0f,0.0f,1.0f);
    cubeNormals[3].set(-1.0f,0.0f,0.0f);
    cubeNormals[4].set(0.0f,1.0f,0.0f);
    cubeNormals[5].set(0.0f,-1.0f,0.0f);
    cube->setNormals( cubeNormals );
    cube->setNormalBinding( osg::GeoSet::BIND_PERPRIM );

    // ---------------------------------------
    // Set up a StateSet to make the cube red
    // ---------------------------------------
    osg::StateSet* cubeState = new osg::StateSet();
    osg::Material* redMaterial = new osg::Material();
    osg::Vec4 red( 1.0f, 0.0f, 0.0f, 1.0f );
    redMaterial->setEmission( osg::Material::FRONT_AND_BACK, red );
    redMaterial->setAmbient( osg::Material::FRONT_AND_BACK, red );
    redMaterial->setDiffuse( osg::Material::FRONT_AND_BACK, red );
    redMaterial->setSpecular( osg::Material::FRONT_AND_BACK, red );
    cubeState->setAttribute( redMaterial );

    cube->setStateSet( cubeState );

    geode->addDrawable( cube );
    
    return geode;
}

int main( int argc, char **argv )
{

    glutInit( &argc, argv );


    myTransform = new osg::Transform();
    myTransform->addChild( createCube() );

    // ---------------------------------------------------------------------
    // Register a timer callback with GLUT, in the first instance as a test
    // This will call the function "timedCB(value)" after dt ms
    // I have decided to use value as the time for the next scheduling
    // If the last parameter=0 then don't reschedule the timer.
    // ---------------------------------------------------------------------
    glutTimerFunc( dt, timedCB, dt );

    osgGLUT::Viewer viewer;
    viewer.addViewport( myTransform );

    // register trackball, flight and drive.
    viewer.registerCameraManipulator(new osgUtil::TrackballManipulator);

    viewer.open();

    viewer.run();

    return 0;
}
