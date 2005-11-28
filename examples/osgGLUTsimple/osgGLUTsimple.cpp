// C++ source file - (C) 2003 Robert Osfield, released under the OSGPL.
// (C) 2005 Mike Weiblen http://mew.cx/ released under the OSGPL.
// Simple example using GLUT to create an OpenGL window and OSG for rendering.
// Derived from osgsimple.cpp

#include <iostream>
#ifdef WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif
#include <osg/Timer>
#include <osgUtil/SceneView>
#include <osgDB/ReadFile>

osg::ref_ptr<osgUtil::SceneView> sceneView;
osg::Timer_t start_tick;
unsigned int frameNum;
osg::Matrix viewMatrix;

void display(void)
{
    // set up the frame stamp for current frame to record the current time and frame number so that animtion code can advance correctly
    osg::ref_ptr<osg::FrameStamp> frameStamp = new osg::FrameStamp;
    frameStamp->setReferenceTime(osg::Timer::instance()->delta_s(start_tick,osg::Timer::instance()->tick()));
    frameStamp->setFrameNumber( frameNum++ );

    // pass frame stamp to the SceneView so that the update, cull and draw traversals all use the same FrameStamp
    sceneView->setFrameStamp( frameStamp.get() );

    // set the view
    sceneView->setViewMatrix( viewMatrix );

    // do the update traversal the scene graph - such as updating animations
    sceneView->update();

    // do the cull traversal, collect all objects in the view frustum into a sorted set of rendering bins
    sceneView->cull();

    // draw the rendering bins.
    sceneView->draw();

    // Swap Buffers
    glutSwapBuffers();
    glutPostRedisplay();
}

void reshape( int w, int h )
{
    // update the viewport dimensions, in case the window has been resized.
    sceneView->setViewport( 0, 0, w, h );
}

int main( int argc, char **argv )
{
    glutInit(&argc, argv);

    if (argc<2)
    {
        std::cout << argv[0] <<": requires filename argument." << std::endl;
        return 1;
    }

    // load the scene.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(argv[1]);
    if (!loadedModel)
    {
        std::cout << argv[0] <<": No data loaded." << std::endl;
        return 1;
    }

    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_ALPHA );
    glutInitWindowPosition( 100, 100 );
    glutInitWindowSize( 800, 600 );
    glutCreateWindow( argv[0] );
    glutDisplayFunc( display );
    glutReshapeFunc( reshape );

    // create the view of the scene.
    sceneView = new osgUtil::SceneView;
    sceneView->setDefaults();
    sceneView->setSceneData(loadedModel.get());

    // initialize the view to look at the center of the scene graph
    const osg::BoundingSphere& bs = loadedModel->getBound();
    viewMatrix.makeLookAt(bs.center()-osg::Vec3(0.0,2.0f*bs.radius(),0.0),bs.center(),osg::Vec3(0.0f,0.0f,1.0f));

    // record the timer tick at the start of rendering.
    start_tick = osg::Timer::instance()->tick();
    frameNum = 0;

    glutMainLoop();
    return 0;
}

/*EOF*/
