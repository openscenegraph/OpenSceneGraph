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

Window* Window::s_theWindow = 0;

Window::Window()
{
    s_theWindow = this;

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


Window::~Window()
{
}


/**
  * Configure and open the GLUT window for this Window
  * 
  */
bool Window::open()
{
    if ( _is_open ) {
        osg::notify(osg::NOTICE)<<"osgGLUT::Window::open() called with window already open."<< std::endl;
        return false;
    }

    //glutInit( &argc, argv );    // I moved this into main to avoid passing
    // argc and argv to the Window
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

void Window::displayCB()
{
    s_theWindow->display();
}


//void Window::reshapeCB(GLint w, GLint h)
void Window::reshapeCB(int w, int h)
{
    s_theWindow->reshape(w, h);
}


void Window::visibilityCB( int state )
{
    s_theWindow->visibility(state);
}


void Window::mouseCB(int button, int state, int x, int y)
{
    s_theWindow->mouse(button, state, x, y);
}


void Window::mouseMotionCB(int x, int y)
{
    s_theWindow->mouseMotion(x,y);
}


void Window::mousePassiveMotionCB(int x, int y)
{
    s_theWindow->mousePassiveMotion(x,y);
}


void Window::keyboardCB(unsigned char key, int x, int y)
{
    s_theWindow->keyboard(key,x,y);
}


void Window::display()
{
}


void Window::reshape(GLint w, GLint h)
{
    _ww = w;
    _wh = h;
}


void Window::visibility(int state)
{
    if (state == GLUT_VISIBLE)
        glutIdleFunc( displayCB );
    else
        glutIdleFunc(0L);
}


void Window::mouseMotion(int , int )
{
}


void Window::mousePassiveMotion(int , int )
{
}


void Window::mouse(int , int , int , int )
{
}


void Window::keyboard(unsigned char key, int , int )
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

bool Window::run()
{
    if (!_is_open) {
        osg::notify(osg::NOTICE)<<"osgGLUT::Window::run() called without window open.  Opening window."<< std::endl;
        if ( !open() )
            return false;
    }

    glutMainLoop();

    return true;
}
