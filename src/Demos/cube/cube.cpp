#include "osgGLUT/Viewer"

#include "osg/Geode"
#include "osg/GeoSet"
#include "osg/GeoState"
#include "osg/Material"
#include "osg/Vec3"
#include "osg/DCS"

#include <GL/glut.h>
#include <math.h>

// ----------------------------------------------------------------------
// Global variables - this is basically the stuff which will be animated
// ----------------------------------------------------------------------
osg::DCS* myDCS;
osg::GeoSet* cube;

int mytime=0;	// in milliseconds
unsigned int dt=50;		// in milliseconds
double omega=0.002;	// in inverse milliseconds

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
   // Update the DCS so that the cube will appear to oscillate
   // ---------------------------------------------------------
   double dx = 0.5 * cos( (double) omega * (double) mytime );
   double dy = 0.5 * sin( (double) omega * (double) mytime );
   double dz = 0.0;
   myDCS->preTranslate( -lastdx, -lastdy, -lastdz );   
   myDCS->preTranslate( (float) dx, (float) dy, dz );
   lastdx=dx; lastdy=dy; lastdz=dz;

   // Graeme, commeted out this call as the cube itself remains static.
   // cube->dirtyDisplayList();

   // -------------------------------------------
   // If required, reschedule the timed callback
   // -------------------------------------------
   if ( delta_t != 0 ) {
      glutTimerFunc( (unsigned int) delta_t, timedCB, delta_t );
   }
}


int main( int argc, char **argv )
{

    osg::Geode* geode = new osg::Geode();

    // -------------------------------------------
    // Set up a new GeoSet which will be our cube
    // -------------------------------------------
    cube = new osg::GeoSet();

    cube->setPrimType( osg::GeoSet::POLYGON );
    cube->setNumPrims( 6 ); 		// the six square faces
    
    int cubeLengthList[6] = { 4, 4, 4, 4, 4, 4 };	// each has 4 vertices 
    cube->setPrimLengths( cubeLengthList );
    
    osg::Vec3 cubeCoords[24];
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
    
    // ---------------------------------------
    // Set up a GeoState to make the cube red
    // ---------------------------------------
    osg::GeoState* cubeState = new osg::GeoState();
    osg::Material* redMaterial = new osg::Material();
    osg::Vec4 red( 1.0f, 0.0f, 0.0f, 0.0f );
    redMaterial->setEmission( osg::Material::FACE_FRONT_AND_BACK, red );
    redMaterial->setAmbient( osg::Material::FACE_FRONT_AND_BACK, red );
    redMaterial->setDiffuse( osg::Material::FACE_FRONT_AND_BACK, red );
    redMaterial->setSpecular( osg::Material::FACE_FRONT_AND_BACK, red );
    cubeState->setAttribute( osg::GeoState::MATERIAL, redMaterial );
    
    cube->setGeoState( cubeState );
    
    geode->addGeoSet( cube );

    myDCS = new osg::DCS();
    myDCS->addChild( geode );

    glutInit( &argc, argv );
    
    // ---------------------------------------------------------------------
    // Register a timer callback with GLUT, in the first instance as a test
    // This will call the function "timedCB(value)" after dt ms
    // I have decided to use value as the time for the next scheduling
    // If the last parameter=0 then don't reschedule the timer.
    // ---------------------------------------------------------------------
    glutTimerFunc( dt, timedCB, dt );

    osgGLUT::Viewer viewer;
    viewer.init( myDCS );
    viewer.run();

    return 0;
}
