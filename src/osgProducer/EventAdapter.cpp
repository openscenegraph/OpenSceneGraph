#include <osgProducer/EventAdapter>

using namespace osgProducer;

// default to no mouse buttons being pressed.
unsigned int EventAdapter::_s_accumulatedButtonMask = 0;

int EventAdapter::_s_button = 0;
int EventAdapter::_s_modKeyMask = 0;
int EventAdapter::_s_Xmin = 0;
int EventAdapter::_s_Xmax = 1280;
int EventAdapter::_s_Ymin = 0;
int EventAdapter::_s_Ymax = 1024;
int EventAdapter::_s_mx = 0;
int EventAdapter::_s_my = 0;

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


void EventAdapter::setWindowSize(int Xmin, int Ymin, int Xmax, int Ymax)
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


void EventAdapter::adaptResize(double time, int Xmin, int Ymin, int Xmax, int Ymax)
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

    _s_mx = (int)((x+s_xOffset)*s_xScale*(float)(_s_Xmax-_s_Xmin))+_s_Xmin;
    _s_my = (int)((y+s_yOffset)*s_yScale*(float)(_s_Ymin-_s_Ymax))+_s_Ymax;

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

    _s_mx = (int)((x+s_xOffset)*s_xScale*(float)(_s_Xmax-_s_Xmin))+_s_Xmin;
    _s_my = (int)((y+s_yOffset)*s_yScale*(float)(_s_Ymin-_s_Ymax))+_s_Ymax;

    copyStaticVariables();
}

/** method for adapting mouse motion events whilst mouse buttons are pressed.*/
void EventAdapter::adaptMouseMotion(double time, float x, float y)
{
    
    _eventType = (_s_accumulatedButtonMask) ?
                 DRAG :
                 MOVE;

    _time = time;
    _s_mx = (int)((x+s_xOffset)*s_xScale*(float)(_s_Xmax-_s_Xmin))+_s_Xmin;
    _s_my = (int)((y+s_yOffset)*s_yScale*(float)(_s_Ymin-_s_Ymax))+_s_Ymax;
    copyStaticVariables();

}


/** method for adapting keyboard events.*/
void EventAdapter::adaptKeyPress( double time, Producer::KeySymbol key)
{
    _eventType = KEYDOWN;
    _time = time;
    _key = adaptKeySymbol(key);

    copyStaticVariables();
}

void EventAdapter::adaptKeyRelease( double time, Producer::KeySymbol key)
{
    // we won't handle this correctly right now.. GUIEventAdapter isn't up to it
    _eventType = KEYUP;
    _time = time;
    _key = adaptKeySymbol(key);

    copyStaticVariables();
}



/** method for adapting frame events, i.e. iddle/display callback.*/
void EventAdapter::adaptFrame(double time)
{
    _eventType = FRAME;
    _time = time;

    copyStaticVariables();
}

int EventAdapter::adaptKeySymbol(Producer::KeySymbol key)
{
    #ifdef WIN32
        // need to implement some kind of mapping, perhaps using an std::map?
        if (key==VK_ESCAPE) return KEY_Escape;
        return key;
        
    #else
        // assume Producer is working under X11, so we have a 1:1 mapping
        // between X11 key symbols and osgGA::GUIEventAdapter::KeySymbols.
        return key;
    #endif
}

