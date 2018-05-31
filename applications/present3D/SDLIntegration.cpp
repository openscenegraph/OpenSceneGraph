/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include <osgViewer/Viewer>

#include "SDLIntegration.h"

#include <SDL.h>
#include <SDL_events.h>
#include <SDL_joystick.h>

#include <iostream>

SDLIntegration::SDLIntegration()
{
    _verbose = false;

    // init SDL
    if ( SDL_Init(SDL_INIT_JOYSTICK) < 0 )
    {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);

    int numJoysticks =  SDL_NumJoysticks();

    if (_verbose)
    {
        std::cout<<"number of joysticks "<<numJoysticks<<std::endl;
        for(int i=0; i<numJoysticks; ++i)
        {
            std::cout<<"Joystick name '"<<SDL_JoystickName(i)<<"'"<<std::endl;
        }
    }

    _joystick = numJoysticks>0 ? SDL_JoystickOpen(0) : 0;

    _numAxes = _joystick ? SDL_JoystickNumAxes(_joystick) : 0;
    _numBalls = _joystick ? SDL_JoystickNumBalls(_joystick) : 0;
    _numHats = _joystick ? SDL_JoystickNumHats(_joystick) : 0;
    _numButtons = _joystick ? SDL_JoystickNumButtons(_joystick) : 0;

    if (_verbose)
    {
        std::cout<<"numAxes = "<<_numAxes<<std::endl;
        std::cout<<"numBalls = "<<_numBalls<<std::endl;
        std::cout<<"numHats = "<<_numHats<<std::endl;
        std::cout<<"numButtons = "<<_numButtons<<std::endl;
    }

    addMouseButtonMapping(4, 1); // left
    addMouseButtonMapping(5, 3); // right
    addMouseButtonMapping(6, 2); // middle

    addKeyMapping(10, ' '); // R2

    addKeyMapping(0, '1'); // 1
    addKeyMapping(1, '2'); // 2
    addKeyMapping(2, '3'); // 3
    addKeyMapping(4, '4'); // 4

    addKeyMapping(7, ' '); // home

    addKeyMapping(8, osgGA::GUIEventAdapter::KEY_Page_Up); // Start
    addKeyMapping(9, osgGA::GUIEventAdapter::KEY_Page_Down); // Start
    addKeyMapping(10, osgGA::GUIEventAdapter::KEY_Home); // Start


    capture(_axisValues, _buttonValues);
}

SDLIntegration::~SDLIntegration()
{
}

void SDLIntegration::capture(ValueList& axisValues, ValueList& buttonValues) const
{
    if (_joystick)
    {
        SDL_JoystickUpdate();

        axisValues.resize(_numAxes);
        for(int ai=0; ai<_numAxes; ++ai)
        {
            axisValues[ai] = SDL_JoystickGetAxis(_joystick, ai);
        }

        buttonValues.resize(_numButtons);
        for(int bi=0; bi<_numButtons; ++bi)
        {
            buttonValues[bi] = SDL_JoystickGetButton(_joystick, bi);
        }
    }
}

void SDLIntegration::update(osgViewer::Viewer& viewer)
{
    if (_joystick)
    {

        ValueList newAxisValues;
        ValueList newButtonValues;

        capture(newAxisValues, newButtonValues);

        unsigned int mouseXaxis = 0;
        unsigned int mouseYaxis = 1;

        float prev_mx = (float)_axisValues[mouseXaxis]/32767.0f;
        float prev_my = -(float)_axisValues[mouseYaxis]/32767.0f;

        float mx = (float)newAxisValues[mouseXaxis]/32767.0f;
        float my = -(float)newAxisValues[mouseYaxis]/32767.0f;


        osgGA::EventQueue* eq = viewer.getEventQueue();
        double time = eq ? eq->getTime() : 0.0;
        osgGA::GUIEventAdapter* previous_event = eq->getCurrentEventState();
        float projected_mx = previous_event->getXmin() + (mx+1.0)*0.5*(previous_event->getXmax()-previous_event->getXmin());
        float projected_my = previous_event->getYmin() + (my+1.0)*0.5*(previous_event->getYmax()-previous_event->getYmin());

        if (mx!=prev_mx || my!=prev_my)
        {
            eq->mouseMotion(projected_mx, projected_my, time);
        }


        if (_verbose)
        {
            for(int ai=0; ai<_numAxes; ++ai)
            {
                if (newAxisValues[ai]!=_axisValues[ai])
                {
                    std::cout<<"axis "<<ai<<" moved to "<<newAxisValues[ai]<<std::endl;
                }
            }
        }

        for(int bi=0; bi<_numButtons; ++bi)
        {
            if (newButtonValues[bi]!=_buttonValues[bi])
            {
                if (_verbose)
                {
                    std::cout<<"button "<<bi<<" changed to "<<newButtonValues[bi]<<std::endl;
                }

                int key =  getKeyMapping(bi);
                int mouseButton =  getMouseButtonMapping(bi);

                if (mouseButton>0)
                {
                    if (newButtonValues[bi]==0) eq->mouseButtonRelease(projected_mx,projected_my,mouseButton,time);
                    else  eq->mouseButtonPress(projected_mx,projected_my,mouseButton,time);
                }
                else if (key>0)
                {

                    if (newButtonValues[bi]==0) eq->keyRelease(key,time);
                    else eq->keyPress(key,time);
                }
            }
        }

        _axisValues.swap(newAxisValues);
        _buttonValues.swap(newButtonValues);

    }

}

