//
// class WXEventAdapter
//

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "osgWX/WXEventAdapter"

using namespace osgWX;

// default to no mouse buttons being pressed.
unsigned int WXEventAdapter::_s_accumulatedButtonMask = 0;

int WXEventAdapter::_s_Xmin = 0;
int WXEventAdapter::_s_Xmax = 1280;
int WXEventAdapter::_s_Ymin = 0;
int WXEventAdapter::_s_Ymax = 1024;
int WXEventAdapter::_s_mx = 0;
int WXEventAdapter::_s_my = 0;

WXEventAdapter::WXEventAdapter()
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


void WXEventAdapter::copyStaticVariables()
{
    _buttonMask = _s_accumulatedButtonMask;
    _Xmin = _s_Xmin;
    _Xmax = _s_Xmax;
    _Ymin = _s_Ymin;
    _Ymax = _s_Ymax;
    _mx   = _s_mx;
    _my   = _s_my;
}


void WXEventAdapter::setWindowSize(int Xmin, int Ymin, int Xmax, int Ymax)
{
    _s_Xmin = Xmin;
    _s_Xmax = Xmax;
    _s_Ymin = Ymin;
    _s_Ymax = Ymax;
}


void WXEventAdapter::setButtonMask(unsigned int buttonMask)
{
    _s_accumulatedButtonMask = buttonMask;
}


void WXEventAdapter::adaptResize(float time, int Xmin, int Ymin, int Xmax, int Ymax)
{
    setWindowSize(Xmin,Ymin,Xmax,Ymax);
    _eventType = RESIZE;
    _time = time;
    copyStaticVariables();
}


/** method for adapting mouse motion events whilst mouse buttons are pressed.*/
void WXEventAdapter::adaptMouseMotion(float time, int x, int y)
{
    _eventType = DRAG;
    _time = time;
    _s_mx = x;
    _s_my = y;
    copyStaticVariables();
}


/** method for adapting mouse motion events whilst no mouse button are pressed.*/
void WXEventAdapter::adaptMousePassiveMotion(float time, int x, int y)
{
    _eventType = MOVE;
    _time = time;
    _s_mx = x;
    _s_my = y;
    copyStaticVariables();
}


/** method for adapting mouse button pressed/released events.*/
void WXEventAdapter::adaptMouse(float time, wxMouseEvent *event)
{
    _time = time;

    wxEventType type = event->GetEventType();

    if ( type == wxEVT_LEFT_DOWN ) {
        _eventType = PUSH;
        _s_accumulatedButtonMask = _s_accumulatedButtonMask | LEFT_BUTTON; 
    }
    else if ( type == wxEVT_LEFT_UP ) {
        _eventType = RELEASE;
        _s_accumulatedButtonMask = _s_accumulatedButtonMask & ~LEFT_BUTTON; 
    }
    else if ( type == wxEVT_MIDDLE_DOWN ) {
        _eventType = PUSH;
        _s_accumulatedButtonMask = _s_accumulatedButtonMask | MIDDLE_BUTTON; 
    }
    else if ( type == wxEVT_MIDDLE_UP ) {
        _eventType = RELEASE;
        _s_accumulatedButtonMask = _s_accumulatedButtonMask & ~MIDDLE_BUTTON; 
    }
    else if ( type == wxEVT_RIGHT_DOWN ) {
        _eventType = PUSH;
        _s_accumulatedButtonMask = _s_accumulatedButtonMask | RIGHT_BUTTON; 
    }
    else if ( type == wxEVT_RIGHT_UP ) {
        _eventType = RELEASE;
        _s_accumulatedButtonMask = _s_accumulatedButtonMask & ~RIGHT_BUTTON; 
    }
    else if ( type == wxEVT_MOTION ) {
        if (event->ButtonIsDown(-1))
            _eventType = DRAG;
        else
            _eventType = MOVE;
    }
    else {
        // ignored mouse events, such as wxEVT_LEAVE_WINDOW
        return;
    }

#if 0
    // not yet handled: modifiers
    if (event.ControlDown()) ...;
    if (event.ShiftDown()) ...;
#endif

    _s_mx = event->GetX();
    _s_my = event->GetY();

    copyStaticVariables();
}


/** method for adapting keyboard events.*/
void WXEventAdapter::adaptKeyboard(float time, unsigned char key, int x, int y )
{
    _eventType = KEYBOARD;
    _time = time;
    _key = key;
    _s_mx = x;
    _s_my = y;

    copyStaticVariables();
}


/** method for adapting frame events, i.e. iddle/display callback.*/
void WXEventAdapter::adaptFrame(float time)
{
    _eventType = FRAME;
    _time = time;

    copyStaticVariables();
}
