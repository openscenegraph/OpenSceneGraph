/* OpenSceneGraph example, osgviewerGlut.
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

// (C) 2005 Mike Weiblen http://mew.cx/ released under the OSGPL.
// Simple example using GLUT to create an OpenGL window and OSG for rendering.
// Derived from osgGLUTsimple.cpp and osgkeyboardmouse.cpp

#include <osg/Config>

#if defined(_MSC_VER) && defined(OSG_DISABLE_MSVC_WARNINGS)
    // disable warning "glutCreateMenu_ATEXIT_HACK' : unreferenced local function has been removed"
    #pragma warning( disable : 4505 )
#endif

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
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>

osg::ref_ptr<osgViewer::Viewer> viewer;
osg::observer_ptr<osgViewer::GraphicsWindow> window;

void display(void)
{
    // update and render the scene graph
    if (viewer.valid()) viewer->frame();

    // Swap Buffers
    glutSwapBuffers();
    glutPostRedisplay();
}

void reshape( int w, int h )
{
    // update the window dimensions, in case the window has been resized.
    if (window.valid()) 
    {
        window->resized(window->getTraits()->x, window->getTraits()->y, w, h);
        window->getEventQueue()->windowResize(window->getTraits()->x, window->getTraits()->y, w, h );
    }
}

void mousebutton( int button, int state, int x, int y )
{
    if (window.valid())
    {
        if (state==0) window->getEventQueue()->mouseButtonPress( x, y, button+1 );
        else window->getEventQueue()->mouseButtonRelease( x, y, button+1 );
    }
}

void mousemove( int x, int y )
{
    if (window.valid())
    {
        window->getEventQueue()->mouseMotion( x, y );
    }
}

void keyboard( unsigned char key, int /*x*/, int /*y*/ )
{
    switch( key )
    {
        case 27:
            // clean up the viewer 
            if (viewer.valid()) viewer = 0;
            glutDestroyWindow(glutGetWindow());
            break;
        default:
            if (window.valid())
            {
                window->getEventQueue()->keyPress( (osgGA::GUIEventAdapter::KeySymbol) key );
                window->getEventQueue()->keyRelease( (osgGA::GUIEventAdapter::KeySymbol) key );
            }
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

    // create the view of the scene.
    viewer = new osgViewer::Viewer;
    window = viewer->setUpViewerAsEmbeddedInWindow(100,100,800,600);
    viewer->setSceneData(loadedModel.get());
    viewer->setCameraManipulator(new osgGA::TrackballManipulator);
    viewer->addEventHandler(new osgViewer::StatsHandler);
    viewer->realize();

    glutMainLoop();
    
    return 0;
}

/*EOF*/
