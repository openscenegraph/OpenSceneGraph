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

EventAdapter::KeySymbolMap EventAdapter::s_keySymbolMap;
bool EventAdapter::s_keySymbolMapInitialized = false;

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

bool EventAdapter::initKeySymbolMap()
{
#ifdef WIN32


    #if 0

    /*
        // not mapped yet as I can't see an
        // obvious mapping to X11/osgGA::GUIEventAdapter::KeySymbol.
        s_keySymbolMap[VK_CAPITAL] = ;
        s_keySymbolMap[VK_CONVERT] = ;
        s_keySymbolMap[VK_NONCONVERT] = ;
        s_keySymbolMap[VK_ACCEPT] = ;
        s_keySymbolMap[VK_SNAPSHOT] = ;

        s_keySymbolMap[VK_LWIN] = ;
        s_keySymbolMap[VK_RWIN] = ;
        s_keySymbolMap[VK_APPS] = ;

        s_keySymbolMap[VK_ATTN] = ;
        s_keySymbolMap[VK_CRSEL] = ;
        s_keySymbolMap[VK_EXSEL] = ;
        s_keySymbolMap[VK_EREOF] = ;
        s_keySymbolMap[VK_PLAY] = ;
        s_keySymbolMap[VK_ZOOM] = ;
        s_keySymbolMap[VK_NONAME] = ;
        s_keySymbolMap[VK_PA1] = ;
    */

        // mapped to osgGA::GUIEventAdapter::KeySymbol
        s_keySymbolMap[VK_CANCEL] = KEY_Cancel;

        s_keySymbolMap[VK_BACK] = KEY_BackSpace;
        s_keySymbolMap[VK_TAB] = KEY_Tab;

        s_keySymbolMap[VK_CLEAR] = KEY_Clear;
        s_keySymbolMap[VK_RETURN] = KEY_Return;

        s_keySymbolMap[VK_SHIFT] = KEY_Shift_Lock;
        s_keySymbolMap[VK_CONTROL] = KEY_Control_L;
        s_keySymbolMap[VK_MENU] = KEY_Menu;
        s_keySymbolMap[VK_PAUSE ] = KEY_Pause;

        s_keySymbolMap[VK_ESCAPE] = KEY_Escape;

        s_keySymbolMap[VK_MODECHANGE] = KEY_Mode_switch;

        s_keySymbolMap[VK_SPACE] = KEY_Space;
        s_keySymbolMap[VK_PRIOR] = KEY_Prior;
        s_keySymbolMap[VK_NEXT] = KEY_Next;
        s_keySymbolMap[VK_END] = KEY_End;
        s_keySymbolMap[VK_HOME] = KEY_Home;
        s_keySymbolMap[VK_LEFT] = KEY_Left;
        s_keySymbolMap[VK_UP] = KEY_Up;
        s_keySymbolMap[VK_RIGHT] = KEY_Right;
        s_keySymbolMap[VK_DOWN] = KEY_Down;
        s_keySymbolMap[VK_SELECT] = KEY_Select;
        s_keySymbolMap[VK_PRINT] = KEY_Print;
        s_keySymbolMap[VK_EXECUTE] = KEY_Execute;
        s_keySymbolMap[VK_INSERT] = KEY_Insert;
        s_keySymbolMap[VK_DELETE] = KEY_Delete;
        s_keySymbolMap[VK_HELP] = KEY_Help;

        s_keySymbolMap[VK_NUMPAD0] = KEY_KP_0;
        s_keySymbolMap[VK_NUMPAD1] = KEY_KP_1;
        s_keySymbolMap[VK_NUMPAD2] = KEY_KP_2;
        s_keySymbolMap[VK_NUMPAD3] = KEY_KP_3;
        s_keySymbolMap[VK_NUMPAD4] = KEY_KP_4;
        s_keySymbolMap[VK_NUMPAD5] = KEY_KP_5;
        s_keySymbolMap[VK_NUMPAD6] = KEY_KP_6;
        s_keySymbolMap[VK_NUMPAD7] = KEY_KP_7;
        s_keySymbolMap[VK_NUMPAD8] = KEY_KP_8;
        s_keySymbolMap[VK_NUMPAD9] = KEY_KP_9;
        s_keySymbolMap[VK_MULTIPLY] = KEY_KP_Multiply;
        s_keySymbolMap[VK_ADD] = KEY_KP_Add;
        s_keySymbolMap[VK_SEPARATOR] = KEY_KP_Separator;
        s_keySymbolMap[VK_SUBTRACT] = KEY_KP_Subtract;
        s_keySymbolMap[VK_DECIMAL] = KEY_KP_Decimal;
        s_keySymbolMap[VK_DIVIDE] = KEY_KP_Divide;
        s_keySymbolMap[VK_F1] = KEY_F1;
        s_keySymbolMap[VK_F2] = KEY_F2;
        s_keySymbolMap[VK_F3] = KEY_F3;
        s_keySymbolMap[VK_F4] = KEY_F4;
        s_keySymbolMap[VK_F5] = KEY_F5;
        s_keySymbolMap[VK_F6] = KEY_F6;
        s_keySymbolMap[VK_F7] = KEY_F7;
        s_keySymbolMap[VK_F8] = KEY_F8;
        s_keySymbolMap[VK_F9] = KEY_F9;
        s_keySymbolMap[VK_F10] = KEY_F10;
        s_keySymbolMap[VK_F11] = KEY_F11;
        s_keySymbolMap[VK_F12] = KEY_F12;
        s_keySymbolMap[VK_F13] = KEY_F13;
        s_keySymbolMap[VK_F14] = KEY_F14;
        s_keySymbolMap[VK_F15] = KEY_F15;
        s_keySymbolMap[VK_F16] = KEY_F16;
        s_keySymbolMap[VK_F17] = KEY_F17;
        s_keySymbolMap[VK_F18] = KEY_F18;
        s_keySymbolMap[VK_F19] = KEY_F19;
        s_keySymbolMap[VK_F20] = KEY_F20;
        s_keySymbolMap[VK_F21] = KEY_F21;
        s_keySymbolMap[VK_F22] = KEY_F22;
        s_keySymbolMap[VK_F23] = KEY_F23;
        s_keySymbolMap[VK_F24] = KEY_F24;

        s_keySymbolMap[VK_NUMLOCK] = KEY_Num_Lock;
        s_keySymbolMap[VK_SCROLL] = KEY_Scroll_Lock;

        s_keySymbolMap[VK_LSHIFT] = KEY_Shift_L;
        s_keySymbolMap[VK_RSHIFT] = KEY_Shift_R;
        s_keySymbolMap[VK_LCONTROL] = KEY_Control_L;
        s_keySymbolMap[VK_RCONTROL] = KEY_Control_R;
        s_keySymbolMap[VK_LMENU] = KEY_Menu;
        s_keySymbolMap[VK_RMENU] = KEY_Menu;
        s_keySymbolMap[VK_OEM_CLEAR] = KEY_Clear;
    #endif    
    
    
#else

    // no mapping required for non windows (i.e. X11 based) 
    // since the osgGA::GUIEventAdapter::KeySybol values are
    // take from X11/keysymdef.h

#endif

    return true;
}

int EventAdapter::adaptKeySymbol(Producer::KeySymbol key)
{
    if (!s_keySymbolMapInitialized) s_keySymbolMapInitialized = initKeySymbolMap();

    KeySymbolMap::iterator itr = s_keySymbolMap.find(key);
    if (itr!=s_keySymbolMap.end()) return itr->second;
    else return key;
}

