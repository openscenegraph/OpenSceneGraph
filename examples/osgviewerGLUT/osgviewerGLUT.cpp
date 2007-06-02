// C++ source file - (C) 2003 Robert Osfield, released under the OSGPL.
// (C) 2005 Mike Weiblen http://mew.cx/ released under the OSGPL.
// Simple example using GLUT to create an OpenGL window and OSG for rendering.
// Derived from osgGLUTsimple.cpp and osgkeyboardmouse.cpp

#include <iostream>
#ifdef WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include <osgViewer/Viewer>
#include <osgViewer/StatsHandler>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>

osg::ref_ptr<osgViewer::Viewer> viewer;
osg::ref_ptr<osgViewer::GraphicsWindow> window;

void display(void)
{
    // update and render the scene graph
    viewer->frame();

    // Swap Buffers
    glutSwapBuffers();
    glutPostRedisplay();
}

void reshape( int w, int h )
{
    // update the window dimensions, in case the window has been resized.
    window->resized(window->getTraits()->x, window->getTraits()->y, w, h);
    window->getEventQueue()->windowResize(window->getTraits()->x, window->getTraits()->y, w, h );
}

void mousebutton( int button, int state, int x, int y )
{
    if (state==0) window->getEventQueue()->mouseButtonPress( x, y, button+1 );
    else window->getEventQueue()->mouseButtonRelease( x, y, button+1 );
}

void mousemove( int x, int y )
{
    window->getEventQueue()->mouseMotion( x, y );
}

void keyboard( unsigned char key, int /*x*/, int /*y*/ )
{
    switch( key )
    {
        case 27:
            glutDestroyWindow(glutGetWindow());
            break;
        default:
            window->getEventQueue()->keyPress( (osgGA::GUIEventAdapter::KeySymbol) key );
            window->getEventQueue()->keyRelease( (osgGA::GUIEventAdapter::KeySymbol) key );
            break;
    }
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
    glutMouseFunc( mousebutton );
    glutMotionFunc( mousemove );
    glutKeyboardFunc( keyboard );

    window = new osgViewer::GraphicsWindowEmbedded(100,100,800,600);

    // create the view of the scene.
    viewer = new osgViewer::Viewer;
    viewer->getCamera()->setGraphicsContext(window.get());
    viewer->getCamera()->setViewport(new osg::Viewport(0,0,800,600));
    viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);

    viewer->setSceneData(loadedModel.get());
    viewer->setCameraManipulator(new osgGA::TrackballManipulator);
    viewer->addEventHandler(new osgViewer::StatsHandler);
    viewer->realize();

    glutMainLoop();
    
    return 0;
}

/*EOF*/
