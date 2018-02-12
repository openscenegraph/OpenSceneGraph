/* OpenSceneGraph example, osgdirectinput.
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

#include <osgGA/EventQueue>
#include <iostream>
#include "DirectInputRegistry"

typedef std::pair<int, int> KeyValue;
typedef std::map<int, KeyValue> KeyMap;
KeyMap g_keyMap;

void buildKeyMap()
{
    // TODO: finish the key map as you wish
    g_keyMap[DIK_ESCAPE] = KeyValue(osgGA::GUIEventAdapter::KEY_Escape, 0);
    g_keyMap[DIK_1] = KeyValue('1', 0);
    g_keyMap[DIK_2] = KeyValue('2', 0);
    g_keyMap[DIK_3] = KeyValue('3', 0);
    g_keyMap[DIK_4] = KeyValue('4', 0);
    g_keyMap[DIK_5] = KeyValue('5', 0);
    g_keyMap[DIK_6] = KeyValue('6', 0);
    g_keyMap[DIK_7] = KeyValue('7', 0);
    g_keyMap[DIK_8] = KeyValue('8', 0);
    g_keyMap[DIK_9] = KeyValue('9', 0);
    g_keyMap[DIK_0] = KeyValue('0', 0);
    g_keyMap[DIK_MINUS] = KeyValue('-', 0);
    g_keyMap[DIK_EQUALS] = KeyValue('=', 0);
    g_keyMap[DIK_BACK] = KeyValue(osgGA::GUIEventAdapter::KEY_BackSpace, 0);
    g_keyMap[DIK_TAB] = KeyValue(osgGA::GUIEventAdapter::KEY_Tab, 0);
    g_keyMap[DIK_SPACE] = KeyValue(osgGA::GUIEventAdapter::KEY_Space, 0);
}

bool DirectInputRegistry::initKeyboard( HWND handle )
{
    if ( !_inputDevice ) return false;
    
    HRESULT hr = _inputDevice->CreateDevice( GUID_SysKeyboard, &_keyboard, NULL );
    if ( FAILED(hr) || _keyboard==NULL )
    {
        osg::notify(osg::WARN) << "Unable to create keyboard." << std::endl;
        return false;
    }
    buildKeyMap();
    return initImplementation( handle, _keyboard, &c_dfDIKeyboard );
}

bool DirectInputRegistry::initMouse( HWND handle )
{
    if ( !_inputDevice ) return false;
    
    HRESULT hr = _inputDevice->CreateDevice( GUID_SysMouse, &_mouse, NULL );
    if ( FAILED(hr) || _mouse==NULL )
    {
        osg::notify(osg::WARN) << "Unable to create mouse." << std::endl;
        return false;
    }
    return initImplementation( handle, _mouse, &c_dfDIMouse2 );
}

bool DirectInputRegistry::initJoystick( HWND handle )
{
    if ( !_inputDevice ) return false;
    
    HRESULT hr = _inputDevice->EnumDevices( DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback,
                                            NULL, DIEDFL_ATTACHEDONLY );
    if ( FAILED(hr) || _joystick==NULL )
    {
        osg::notify(osg::WARN) << "Unable to enumerate joysticks." << std::endl;
        return false;
    }
    return initImplementation( handle, _joystick, &c_dfDIJoystick2 );
}

void DirectInputRegistry::updateState( osgGA::EventQueue* eventQueue )
{
    HRESULT hr;
    if ( !_supportDirectInput || !eventQueue ) return;
    
    if ( _keyboard )
    {
        pollDevice( _keyboard );
        
        char buffer[256] = {0};
        hr = _keyboard->GetDeviceState( sizeof(buffer), &buffer );
        if ( SUCCEEDED(hr) )
        {
            for ( KeyMap::iterator itr=g_keyMap.begin(); itr!=g_keyMap.end(); ++itr )
            {
                KeyValue& key = itr->second;
                char value = buffer[itr->first];
                if ( key.second==value ) continue;
                
                key.second = value;
                if ( value&0x80 )
                    eventQueue->keyPress( key.first );
                else
                    eventQueue->keyRelease( key.first );
            }
        }
    }
    
    if ( _mouse )
    {
        pollDevice( _mouse );
        
        DIMOUSESTATE2 mouseState;
        hr = _mouse->GetDeviceState( sizeof(DIMOUSESTATE2), &mouseState );
        
        // TODO: add mouse handlers
    }
    
    if ( _joystick )
    {
        pollDevice( _joystick );
        
        osg::ref_ptr<JoystickEvent> event = new JoystickEvent;
        hr = _joystick->GetDeviceState( sizeof(DIJOYSTATE2), &(event->_js) );
        if ( SUCCEEDED(hr) ) eventQueue->userEvent( event.get() );
    }
}

DirectInputRegistry::DirectInputRegistry()
:   _keyboard(0), _mouse(0), _joystick(0),
    _supportDirectInput(true)
{
    HRESULT hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION,
                                     IID_IDirectInput8, (VOID**)&_inputDevice, NULL );
    if ( FAILED(hr) )
    {
        osg::notify(osg::WARN) << "Unable to create DirectInput object." << std::endl;
        _supportDirectInput = false;
    }
}

DirectInputRegistry::~DirectInputRegistry()
{
    releaseDevice( _keyboard );
    releaseDevice( _mouse );
    releaseDevice( _joystick );
    if ( _inputDevice ) _inputDevice->Release();
}

bool DirectInputRegistry::initImplementation( HWND handle, LPDIRECTINPUTDEVICE8 device, LPCDIDATAFORMAT format )
{
    _supportDirectInput = true;
    HRESULT hr = device->SetDataFormat( format );
    if ( FAILED(hr) )
    {
        osg::notify(osg::WARN) << "Unable to set device data format." << std::endl;
        _supportDirectInput = false;
    }
    
    hr = device->SetCooperativeLevel( handle, DISCL_EXCLUSIVE|DISCL_FOREGROUND );
    if ( FAILED(hr) )
    {
        osg::notify(osg::WARN) << "Unable to attach device to window." << std::endl;
        _supportDirectInput = false;
    }
    
    device->Acquire();
    return _supportDirectInput;
}

void DirectInputRegistry::pollDevice( LPDIRECTINPUTDEVICE8 device )
{
    HRESULT hr = device->Poll();
    if ( FAILED(hr) )
    {
        device->Acquire();
        if ( hr==DIERR_INPUTLOST )
            osg::notify(osg::WARN) << "Device lost." << std::endl;
    }
}

void DirectInputRegistry::releaseDevice( LPDIRECTINPUTDEVICE8 device )
{
    if ( device )
    {
        device->Unacquire();
        device->Release();
    }
}

BOOL CALLBACK DirectInputRegistry::EnumJoysticksCallback( const DIDEVICEINSTANCE* didInstance, VOID* )
{
    HRESULT hr;
    LPDIRECTINPUT8 device = DirectInputRegistry::instance()->getDevice();
    if ( device )
    {
        hr = device->CreateDevice( didInstance->guidInstance,
                                   &(DirectInputRegistry::instance()->getJoyStick()), NULL );
        if ( SUCCEEDED(hr) ) return DIENUM_STOP;
    }
    return DIENUM_CONTINUE;
}
