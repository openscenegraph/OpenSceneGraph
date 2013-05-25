/* -*-c++-*- Present3D - Copyright (C) 1999-2006 Robert Osfield 
 *
 * This software is open source and may be redistributed and/or modified under  
 * the terms of the GNU General Public License (GPL) version 2.0.
 * The full license is in LICENSE.txt file included with this distribution,.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * include LICENSE.txt for more details.
*/

#ifndef SDLINTEGRATION
#define SDLINTEGRATION

#include <osgGA/Device>

#include <SDL.h>
#include <SDL_events.h>
#include <SDL_joystick.h>

#include <vector>
#include <map>

class JoystickDevice : public osgGA::Device
{
    public:

        JoystickDevice();

        typedef std::vector<int> ValueList;
        typedef std::map<int, int> ButtonMap;

        virtual bool checkEvents();
        
        void addMouseButtonMapping(int joystickButton, int mouseButton)
        {
            _mouseButtonMap[joystickButton] = mouseButton;
        }
        
        int getMouseButtonMapping(int joystickButton)
        {
            ButtonMap::const_iterator itr = _mouseButtonMap.find(joystickButton);
            if (itr != _mouseButtonMap.end()) return itr->second;
            else return -1;
        }

        void addKeyMapping(int joystickButton, int key)
        {
            _keyMap[joystickButton] = key;
        }
        
        int getKeyMapping(int joystickButton)
        {
            ButtonMap::const_iterator itr = _keyMap.find(joystickButton);
            if (itr != _keyMap.end()) return itr->second;
            else return -1;
        }
        
    protected:
    
        virtual ~JoystickDevice();

        void capture(ValueList& axisValues, ValueList& buttonValues) const;

        SDL_Joystick*   _joystick;
        int             _numAxes;
        int             _numBalls;
        int             _numHats;
        int             _numButtons;
        bool            _verbose;
        
        ValueList       _axisValues;
        ValueList       _buttonValues;
        ButtonMap       _mouseButtonMap;
        ButtonMap       _keyMap;
};

#endif
