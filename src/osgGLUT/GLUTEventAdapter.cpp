#include <osgGLUT/GLUTEventAdapter>
#include <osgGLUT/glut>

using namespace osgGLUT;

// default to no mouse buttons being pressed.
unsigned int GLUTEventAdapter::_s_accumulatedButtonMask = 0;

int GLUTEventAdapter::_s_Xmin = 0;
int GLUTEventAdapter::_s_Xmax = 1280;
int GLUTEventAdapter::_s_Ymin = 0;
int GLUTEventAdapter::_s_Ymax = 1024;
int GLUTEventAdapter::_s_mx = 0;
int GLUTEventAdapter::_s_my = 0;

GLUTEventAdapter::GLUTEventAdapter()
{
    _eventType = NONE;           // adaptor does not encapsulate any events.
    _key = -1;                   // set to 'invalid' key value.
    _button = -1;                // set to 'invalid' button value.
    _mx = -1;                    // set to 'invalid' position value.
    _my = -1;                    // set to 'invalid' position value.
    _buttonMask = 0;             // default to no mouse buttons being pressed.
    _time = 0.0f;                // default to no time has been set.

    copyStaticVariables();

}


void GLUTEventAdapter::copyStaticVariables()
{
    _buttonMask = _s_accumulatedButtonMask;
    _Xmin = _s_Xmin;
    _Xmax = _s_Xmax;
    _Ymin = _s_Ymin;
    _Ymax = _s_Ymax;
    _mx   = _s_mx;
    _my   = _s_my;
}


void GLUTEventAdapter::setWindowSize(int Xmin, int Ymin, int Xmax, int Ymax)
{
    _s_Xmin = Xmin;
    _s_Xmax = Xmax;
    _s_Ymin = Ymin;
    _s_Ymax = Ymax;
}


void GLUTEventAdapter::setButtonMask(unsigned int buttonMask)
{
    _s_accumulatedButtonMask = buttonMask;
}


void GLUTEventAdapter::adaptResize(float time, int Xmin, int Ymin, int Xmax, int Ymax)
{
    setWindowSize(Xmin,Ymin,Xmax,Ymax);
    _eventType = RESIZE;
    _time = time;
    copyStaticVariables();
}


/** method for adapting mouse motion events whilst mouse buttons are pressed.*/
void GLUTEventAdapter::adaptMouseMotion(float time, int x, int y)
{
    _eventType = DRAG;
    _time = time;
    _s_mx = x;
    _s_my = y;
    copyStaticVariables();
}


/** method for adapting mouse motion events whilst no mouse button are pressed.*/
void GLUTEventAdapter::adaptMousePassiveMotion(float time, int x, int y)
{
    _eventType = MOVE;
    _time = time;
    _s_mx = x;
    _s_my = y;
    copyStaticVariables();
}


/** method for adapting mouse button pressed/released events.*/
void GLUTEventAdapter::adaptMouse(float time, int button, int state, int x, int y)
{
    _time = time;

    if( state == GLUT_DOWN )
    {

        _eventType = PUSH;
        _button = button;

        switch(button)
        {
            case(GLUT_LEFT_BUTTON): _s_accumulatedButtonMask = _s_accumulatedButtonMask | LEFT_MOUSE_BUTTON; break;
            case(GLUT_MIDDLE_BUTTON): _s_accumulatedButtonMask = _s_accumulatedButtonMask | MIDDLE_MOUSE_BUTTON; break;
            case(GLUT_RIGHT_BUTTON): _s_accumulatedButtonMask = _s_accumulatedButtonMask | RIGHT_MOUSE_BUTTON; break;
        }

    }
    else if( state == GLUT_UP )
    {

        _eventType = RELEASE;
        _button = button;

        switch(button)
        {
            case(GLUT_LEFT_BUTTON): _s_accumulatedButtonMask = _s_accumulatedButtonMask & ~LEFT_MOUSE_BUTTON; break;
            case(GLUT_MIDDLE_BUTTON): _s_accumulatedButtonMask = _s_accumulatedButtonMask & ~MIDDLE_MOUSE_BUTTON; break;
            case(GLUT_RIGHT_BUTTON): _s_accumulatedButtonMask = _s_accumulatedButtonMask & ~RIGHT_MOUSE_BUTTON; break;
        }

    }

    _s_mx = x;
    _s_my = y;

    copyStaticVariables();
}


/** method for adapting keyboard events.*/
void GLUTEventAdapter::adaptKeyboard(float time, unsigned char key, int x, int y )
{
    _eventType = KEYBOARD;
    _time = time;
    _key = key;
    _s_mx = x;
    _s_my = y;

    copyStaticVariables();
}


/** method for adapting frame events, i.e. iddle/display callback.*/
void GLUTEventAdapter::adaptFrame(float time)
{
    _eventType = FRAME;
    _time = time;

    copyStaticVariables();
}
