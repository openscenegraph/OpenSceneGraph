#include <osgProducer/EventAdapter>

using namespace osgProducer;

// default to no mouse buttons being pressed.
unsigned int EventAdapter::_s_accumulatedButtonMask = 0;

int EventAdapter::_s_button = 0;
int EventAdapter::_s_modKeyMask = 0;
float EventAdapter::_s_Xmin = 0;
float EventAdapter::_s_Xmax = 1280;
float EventAdapter::_s_Ymin = 0;
float EventAdapter::_s_Ymax = 1024;
float EventAdapter::_s_mx = 0;
float EventAdapter::_s_my = 0;

static float s_xOffset=1.0f;
static float s_xScale=0.5f;
static float s_yOffset=1.0f;
static float s_yScale=0.5f;


EventAdapter::EventAdapter()
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


void EventAdapter::copyStaticVariables()
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


void EventAdapter::setWindowSize(float Xmin, float Ymin, float Xmax, float Ymax)
{
    _s_Xmin = Xmin;
    _s_Xmax = Xmax;
    _s_Ymin = Ymin;
    _s_Ymax = Ymax;
}


void EventAdapter::setButtonMask(unsigned int buttonMask)
{
    _s_accumulatedButtonMask = buttonMask;
}


void EventAdapter::adaptResize(double time, float Xmin, float Ymin, float Xmax, float Ymax)
{
    setWindowSize(Xmin,Ymin,Xmax,Ymax);
    _eventType = RESIZE;
    _time = time;
    copyStaticVariables();
}

void EventAdapter::adaptButtonPress(double time,float x, float y, unsigned int button)
{
    _time = time;

    _eventType = PUSH;
    _button = button-1;

    switch(_button)
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

    _s_mx = (float)((x+s_xOffset)*s_xScale*(float)(_s_Xmax-_s_Xmin))+_s_Xmin;
    _s_my = (float)((y+s_yOffset)*s_yScale*(float)(_s_Ymin-_s_Ymax))+_s_Ymax;

    copyStaticVariables();
}

void EventAdapter::adaptButtonRelease(double time,float x, float y, unsigned int button)
{
    _time = time;

    _eventType = RELEASE;
    _button = button-1;

    switch(_button)
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

    _s_mx = (float)((x+s_xOffset)*s_xScale*(float)(_s_Xmax-_s_Xmin))+_s_Xmin;
    _s_my = (float)((y+s_yOffset)*s_yScale*(float)(_s_Ymin-_s_Ymax))+_s_Ymax;

    copyStaticVariables();
}

/** method for adapting mouse motion events whilst mouse buttons are pressed.*/
void EventAdapter::adaptMouseMotion(double time, float x, float y)
{
    
    _eventType = (_s_accumulatedButtonMask) ?
                 DRAG :
                 MOVE;

    _time = time;
    _s_mx = (float)((x+s_xOffset)*s_xScale*(float)(_s_Xmax-_s_Xmin))+_s_Xmin;
    _s_my = (float)((y+s_yOffset)*s_yScale*(float)(_s_Ymin-_s_Ymax))+_s_Ymax;
    copyStaticVariables();

}


/** method for adapting keyboard events.*/
void EventAdapter::adaptKeyPress( double time, Producer::KeySymbol key)
{
    _eventType = KEYDOWN;
    _time = time;
    _key = key;
    
    switch(key)
    {
        case(KEY_Shift_L):      _s_modKeyMask = MODKEY_LEFT_SHIFT | _s_modKeyMask; break;
        case(KEY_Shift_R):      _s_modKeyMask = MODKEY_RIGHT_SHIFT | _s_modKeyMask; break;
        case(KEY_Control_L):    _s_modKeyMask = MODKEY_LEFT_CTRL | _s_modKeyMask; break;
        case(KEY_Control_R):    _s_modKeyMask = MODKEY_RIGHT_CTRL | _s_modKeyMask; break;
        case(KEY_Meta_L):       _s_modKeyMask = MODKEY_LEFT_META | _s_modKeyMask; break;
        case(KEY_Meta_R):       _s_modKeyMask = MODKEY_RIGHT_META | _s_modKeyMask; break;
        case(KEY_Alt_L):        _s_modKeyMask = MODKEY_LEFT_ALT | _s_modKeyMask; break;
        case(KEY_Alt_R):        _s_modKeyMask = MODKEY_LEFT_ALT | _s_modKeyMask; break;

        case(KEY_Caps_Lock):
        {
            if ((_s_modKeyMask & MODKEY_CAPS_LOCK)!=0) 
                _s_modKeyMask = ~MODKEY_CAPS_LOCK & _s_modKeyMask;
            else 
                _s_modKeyMask = MODKEY_CAPS_LOCK | _s_modKeyMask; 
            break;
        }
        case(KEY_Num_Lock):
        {
            if ((_s_modKeyMask & MODKEY_NUM_LOCK)!=0)
                 _s_modKeyMask = ~MODKEY_NUM_LOCK & _s_modKeyMask;
            else
                 _s_modKeyMask = MODKEY_NUM_LOCK | _s_modKeyMask;
            break;
        }
    }        

    copyStaticVariables();
}

void EventAdapter::adaptKeyRelease( double time, Producer::KeySymbol key)
{
    // we won't handle this correctly right now.. GUIEventAdapter isn't up to it
    _eventType = KEYUP;
    _time = time;
    _key = key;

    switch(key)
    {
        case(KEY_Shift_L):      _s_modKeyMask = ~MODKEY_LEFT_SHIFT & _s_modKeyMask; break;
        case(KEY_Shift_R):      _s_modKeyMask = ~MODKEY_RIGHT_SHIFT & _s_modKeyMask; break;
        case(KEY_Control_L):    _s_modKeyMask = ~MODKEY_LEFT_CTRL & _s_modKeyMask; break;
        case(KEY_Control_R):    _s_modKeyMask = ~MODKEY_RIGHT_CTRL & _s_modKeyMask; break;
        case(KEY_Meta_L):       _s_modKeyMask = ~MODKEY_LEFT_META & _s_modKeyMask; break;
        case(KEY_Meta_R):       _s_modKeyMask = ~MODKEY_RIGHT_META & _s_modKeyMask; break;
        case(KEY_Alt_L):        _s_modKeyMask = ~MODKEY_LEFT_ALT & _s_modKeyMask; break;
        case(KEY_Alt_R):        _s_modKeyMask = ~MODKEY_LEFT_ALT & _s_modKeyMask; break;
    }        

    copyStaticVariables();
}



/** method for adapting frame events, i.e. iddle/display callback.*/
void EventAdapter::adaptFrame(double time)
{
    _eventType = FRAME;
    _time = time;

    copyStaticVariables();
}
