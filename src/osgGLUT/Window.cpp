#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <stdio.h>
#include <string>

#include <osg/Notify>

#include <osgGLUT/glut>
#include <osgGLUT/Window>

#include <osg/Timer>

using namespace osgGLUT;

GLUTWindow* GLUTWindow::s_theGLUTWindow = 0;

GLUTWindow::GLUTWindow()
{
    s_theGLUTWindow = this;

    _fullscreen = false; 
    _is_open   = 0;
    _saved_wx = _wx = _saved_wy = _wy = 0;
    _saved_ww = _ww = 800,
    _saved_wh = _wh = 600;

    _title = "OSG Window";
    
    _displayMode = GLUT_DOUBLE| GLUT_RGB | GLUT_DEPTH;

    _mx = _ww/2,
    _my = _wh/2;
    _mbutton = 0;    
}


GLUTWindow::~GLUTWindow()
{
}


/**
  * Configure and open the GLUT window for this GLUTWindow
  * 
  */
bool GLUTWindow::open()
{
    if ( _is_open ) {
        osg::notify(osg::NOTICE)<<"osgGLUT::GLUTWindow::open() called with window already open."<< std::endl;
        return false;
    }

    //glutInit( &argc, argv );    // I moved this into main to avoid passing
    // argc and argv to the GLUTWindow
    glutInitWindowPosition( _wx, _wy );
    glutInitWindowSize( _ww, _wh );

    glutInitDisplayMode( _displayMode);
    
    glutCreateWindow( _title.c_str() );

    glutReshapeFunc(    reshapeCB );
    glutVisibilityFunc( visibilityCB );
    glutDisplayFunc(    displayCB );
    glutKeyboardFunc(   keyboardCB );

    glutMouseFunc( mouseCB );
    glutMotionFunc( mouseMotionCB );
    glutPassiveMotionFunc( mousePassiveMotionCB );

    _is_open = 1;
    return true;
}

void GLUTWindow::displayCB()
{
    s_theGLUTWindow->display();
}


//void GLUTWindow::reshapeCB(GLint w, GLint h)
void GLUTWindow::reshapeCB(int w, int h)
{
    s_theGLUTWindow->reshape(w, h);
}


void GLUTWindow::visibilityCB( int state )
{
    s_theGLUTWindow->visibility(state);
}


void GLUTWindow::mouseCB(int button, int state, int x, int y)
{
    s_theGLUTWindow->mouse(button, state, x, y);
}


void GLUTWindow::mouseMotionCB(int x, int y)
{
    s_theGLUTWindow->mouseMotion(x,y);
}


void GLUTWindow::mousePassiveMotionCB(int x, int y)
{
    s_theGLUTWindow->mousePassiveMotion(x,y);
}


void GLUTWindow::keyboardCB(unsigned char key, int x, int y)
{
    s_theGLUTWindow->keyboard(key,x,y);
}


void GLUTWindow::display()
{
}


void GLUTWindow::reshape(GLint w, GLint h)
{
    _ww = w;
    _wh = h;
}


void GLUTWindow::visibility(int state)
{
    if (state == GLUT_VISIBLE)
        glutIdleFunc( displayCB );
    else
        glutIdleFunc(0L);
}


void GLUTWindow::mouseMotion(int x, int y)
{
}


void GLUTWindow::mousePassiveMotion(int x, int y)
{
}


void GLUTWindow::mouse(int button, int state, int x, int y)
{
}


void GLUTWindow::keyboard(unsigned char key, int x, int y)
{
    switch( key )
    {
        case 'f' :
            _fullscreen = !_fullscreen;
            if (_fullscreen)
            {
                _saved_ww = _ww;
                _saved_wh = _wh;
                glutFullScreen();
            } else
            {
                //glutPositionWindow(wx,wy);
                glutReshapeWindow(_saved_ww,_saved_wh);
            }
            break;
    }
}

bool GLUTWindow::run()
{
    if (!_is_open) {
        osg::notify(osg::NOTICE)<<"osgGLUT::GLUTWindow::run() called without window open.  Opening window."<< std::endl;
        if ( !open() )
            return false;
    }

    glutMainLoop();

    return true;
}
