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
    
    _exit = false;

}


Window::~Window()
{
}


void Window::clear()
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
    
    glutCreateWindow( const_cast<char*>(_title.c_str()) );

    glutReshapeFunc(    reshapeCB );
    glutVisibilityFunc( visibilityCB );
    glutDisplayFunc(    displayCB );
    glutKeyboardFunc(   keyboardCB );
    glutKeyboardUpFunc(   keyboardUpCB );

    glutMouseFunc( mouseCB );
    glutMotionFunc( mouseMotionCB );
    glutPassiveMotionFunc( mousePassiveMotionCB );

    glutSpecialFunc( specialCB );
#if (GLUT_API_VERSION >= 4 || GLUT_XLIB_IMPLEMENTATION >= 13)
    glutSpecialUpFunc( specialUpCB );
#endif
    glutSpaceballMotionFunc( spaceballMotionCB );
    glutSpaceballRotateFunc( spaceballRotateCB );
    glutSpaceballButtonFunc( spaceballButtonCB );

    if( _fullscreen )
    {
        _saved_ww = _ww;
        _saved_wh = _wh;
	glutFullScreen();
    }

    _is_open = 1;
    return true;
}

void Window::displayCB()
{
    s_theWindow->display();
    s_theWindow->check_if_exit();
}


//void Window::reshapeCB(GLint w, GLint h)
void Window::reshapeCB(int w, int h)
{
    s_theWindow->reshape(w, h);
    s_theWindow->check_if_exit();
}


void Window::visibilityCB( int state )
{
    s_theWindow->visibility(state);
    s_theWindow->check_if_exit();
}


void Window::mouseCB(int button, int state, int x, int y)
{
    s_theWindow->mouse(button, state, x, y);
    s_theWindow->check_if_exit();
}


void Window::mouseMotionCB(int x, int y)
{
    s_theWindow->mouseMotion(x,y);
    s_theWindow->check_if_exit();
}


void Window::mousePassiveMotionCB(int x, int y)
{
    s_theWindow->mousePassiveMotion(x,y);
    s_theWindow->check_if_exit();
}


void Window::keyboardCB(unsigned char key, int x, int y)
{
    s_theWindow->keyboard((int)key,x,y,true);
    s_theWindow->check_if_exit();
}

void Window::keyboardUpCB(unsigned char key, int x, int y)
{
    s_theWindow->keyboard((int)key,x,y,false);
    s_theWindow->check_if_exit();
}

void Window::specialCB(int key, int x, int y)
{
     s_theWindow->special(key,x,y,true);
     s_theWindow->check_if_exit();
}

void Window::specialUpCB(int key, int x, int y)
{
     s_theWindow->special(key,x,y,false);
     s_theWindow->check_if_exit();
}

void Window::spaceballMotionCB(int x, int y, int z)
{
    s_theWindow->spaceballMotion(x,y,z);
    s_theWindow->check_if_exit();
}

void Window::spaceballRotateCB(int x, int y, int z)
{
    s_theWindow->spaceballRotate(x,y,z);
    s_theWindow->check_if_exit();
}

void Window::spaceballButtonCB(int button, int state)
{
    s_theWindow->spaceballButton(button,state);
    s_theWindow->check_if_exit();
}


void Window::display()
{
    osg::notify(osg::INFO)<<"info : Window::display() unhandled."<<std::endl;
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
    osg::notify(osg::INFO)<<"info : Window::mouseMotion() unhandled."<<std::endl;
}


void Window::mousePassiveMotion(int , int )
{
    osg::notify(osg::INFO)<<"info : Window::mousePassiveMotion() unhandled."<<std::endl;
}


void Window::mouse(int , int , int , int )
{
    osg::notify(osg::INFO)<<"info : mouse::() unhandled."<<std::endl;
}


void Window::keyboard(int key, int , int, bool keydown )
{
    if ( !keydown )
        return;

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

void Window::special(int k, int x, int y, bool keydown)
{
    // will remap to a straight keyboard event...
    keyboard( k * 1000, x, y, keydown);
//    osg::notify(osg::INFO)<<"info : Window::special() unhandled."<<std::endl;
}

void Window::spaceballMotion(int , int , int )
{
    osg::notify(osg::INFO)<<"info : Window::spaceballMotion() unhandled."<<std::endl;
}

void Window::spaceballRotate(int , int , int )
{
    osg::notify(osg::INFO)<<"info : Window::spaceballRotate() unhandled."<<std::endl;
}

void Window::spaceballButton(int , int )
{
    osg::notify(osg::INFO)<<"info : Window::spaceballButton() unhandled."<<std::endl;
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

void Window::check_if_exit()
{
    if (_exit)
    {
        clear();
    
        #ifdef __MWERKS__
        std::exit(0); // avoid collision of std::exit(..) / exit(..) compile errors.
        #else
        exit(0);
        #endif
    }
}

