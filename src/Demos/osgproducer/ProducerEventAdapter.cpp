#include "ProducerEventAdapter.h"

// default to no mouse buttons being pressed.
unsigned int ProducerEventAdapter::_s_accumulatedButtonMask = 0;

int ProducerEventAdapter::_s_button = 0;
int ProducerEventAdapter::_s_modKeyMask = 0;
int ProducerEventAdapter::_s_Xmin = 0;
int ProducerEventAdapter::_s_Xmax = 1280;
int ProducerEventAdapter::_s_Ymin = 0;
int ProducerEventAdapter::_s_Ymax = 1024;
int ProducerEventAdapter::_s_mx = 0;
int ProducerEventAdapter::_s_my = 0;

ProducerEventAdapter::ProducerEventAdapter()
{
    _eventType = NONE;           // adaptor does not encapsulate any events.
    _key = -1;                   // set to 'invalid' key value.
    _button = -1;                // set to 'invalid' button value.
    _mx = -1;                    // set to 'invalid' position value.
    _my = -1;                    // set to 'invalid' position value.
    _buttonMask = 0;             // default to no mouse buttons being pressed.
    _modKeyMask = 0;             // default to no mouse buttons being pressed.
    _time = 0.0f;                // default to no time has been set.

    copyStaticVariables();

}


void ProducerEventAdapter::copyStaticVariables()
{
    _buttonMask = _s_accumulatedButtonMask;
    _modKeyMask = _s_modKeyMask;
    _button = _s_button;
    _Xmin = _s_Xmin;
    _Xmax = _s_Xmax;
    _Ymin = _s_Ymin;
    _Ymax = _s_Ymax;
    _mx   = _s_mx;
    _my   = _s_my;
}


void ProducerEventAdapter::setWindowSize(int Xmin, int Ymin, int Xmax, int Ymax)
{
    _s_Xmin = Xmin;
    _s_Xmax = Xmax;
    _s_Ymin = Ymin;
    _s_Ymax = Ymax;
}


void ProducerEventAdapter::setButtonMask(unsigned int buttonMask)
{
    _s_accumulatedButtonMask = buttonMask;
}


void ProducerEventAdapter::adaptResize(double time, int Xmin, int Ymin, int Xmax, int Ymax)
{
    setWindowSize(Xmin,Ymin,Xmax,Ymax);
    _eventType = RESIZE;
    _time = time;
    copyStaticVariables();
}

void ProducerEventAdapter::adaptButtonPress(double time,float x, float y, unsigned int button)
{
    _time = time;

    _eventType = PUSH;
    _button = button;

    switch(button)
    {
        case(0): 
	    _s_accumulatedButtonMask = 
		_s_accumulatedButtonMask | LEFT_MOUSE_BUTTON; 
	    _s_button = LEFT_MOUSE_BUTTON;
	    break;
        case(1): 
	    _s_accumulatedButtonMask = 
		_s_accumulatedButtonMask | MIDDLE_MOUSE_BUTTON; 
	    _s_button = MIDDLE_MOUSE_BUTTON;
	    break;
        case(2): 
	    _s_accumulatedButtonMask = 
		_s_accumulatedButtonMask | RIGHT_MOUSE_BUTTON; 
	    _s_button = RIGHT_MOUSE_BUTTON;
	    break;
    }

    _s_mx = (int)(x*(float)(_s_Xmax-_s_Xmin))+_s_Xmin;
    _s_my = (int)(y*(float)(_s_Ymax-_s_Ymin))+_s_Ymin;

    copyStaticVariables();
}

void ProducerEventAdapter::adaptButtonRelease(double time,float x, float y, unsigned int button)
{
    _time = time;

    _eventType = RELEASE;
    _button = button;

    switch(button)
    {
        case(0): 
	    _s_accumulatedButtonMask = 
		_s_accumulatedButtonMask & ~LEFT_MOUSE_BUTTON;
	    _s_button = LEFT_MOUSE_BUTTON;
	    break;
        case(1): 
	    _s_accumulatedButtonMask = 
		_s_accumulatedButtonMask & ~MIDDLE_MOUSE_BUTTON; 
	    _s_button = MIDDLE_MOUSE_BUTTON;
	    break;
        case(2): 
	    _s_accumulatedButtonMask = 
		_s_accumulatedButtonMask & ~RIGHT_MOUSE_BUTTON; 
	    _s_button = RIGHT_MOUSE_BUTTON;
	    break;
    }

    _s_mx = (int)(x*(float)(_s_Xmax-_s_Xmin))+_s_Xmin;
    _s_my = (int)(y*(float)(_s_Ymax-_s_Ymin))+_s_Ymin;

    copyStaticVariables();
}

/** method for adapting mouse motion events whilst mouse buttons are pressed.*/
void ProducerEventAdapter::adaptMouseMotion(double time, float x, float y)
{
    _eventType = DRAG;
    _time = time;
    _s_mx = (int)(x*(float)(_s_Xmax-_s_Xmin))+_s_Xmin;
    _s_my = (int)(y*(float)(_s_Ymax-_s_Ymin))+_s_Ymin;
    copyStaticVariables();
}


/** method for adapting keyboard events.*/
void ProducerEventAdapter::adaptKeyPress( double time, Producer::KeySymbol key)
{
    _eventType = KEYDOWN;
    _time = time;
    _key = key;

    copyStaticVariables();
}

void ProducerEventAdapter::adaptKeyRelease( double time, Producer::KeySymbol key)
{
    // we won't handle this correctly right now.. GUIEventAdapter isn't up to it
    _eventType = KEYUP;
    _time = time;
    _key = key;

    copyStaticVariables();
}



/** method for adapting frame events, i.e. iddle/display callback.*/
void ProducerEventAdapter::adaptFrame(double time)
{
    _eventType = FRAME;
    _time = time;

    copyStaticVariables();
}
