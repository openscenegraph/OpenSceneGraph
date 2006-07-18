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
//#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <osg/Timer>
#include <osg/Notify>

using namespace osg;

// follows are the constructors of the Timer class, once version
// for each OS combination.  The order is WIN32, FreeBSD, Linux, IRIX,
// and the rest of the world.
//
// all the rest of the timer methods are implemented within the header.


const Timer* Timer::instance()
{
    static Timer s_timer;
    return &s_timer;
}

#ifdef WIN32

    #include <sys/types.h>
    #include <fcntl.h>
    #include <windows.h>
    #include <winbase.h>
    Timer::Timer()
    {
        LARGE_INTEGER frequency;
        if(QueryPerformanceFrequency(&frequency))
        {
            _secsPerTick = 1.0/(double)frequency.QuadPart;
        }
        else
        {
            _secsPerTick = 1.0;
            notify(NOTICE)<<"Error: Timer::Timer() unable to use QueryPerformanceFrequency, "<<std::endl;
            notify(NOTICE)<<"timing code will be wrong, Windows error code: "<<GetLastError()<<std::endl;
        }
    }

    Timer_t Timer::tick() const
    {
        LARGE_INTEGER qpc;
        if (QueryPerformanceCounter(&qpc))
        {
            return qpc.QuadPart;
        }
        else
        {
            notify(NOTICE)<<"Error: Timer::Timer() unable to use QueryPerformanceCounter, "<<std::endl;
            notify(NOTICE)<<"timing code will be wrong, Windows error code: "<<GetLastError()<<std::endl;
            return 0;
        }
    }

#else

    #include <sys/time.h>

    Timer::Timer( void )
    {
        _secsPerTick = (1.0 / (double) 1000000);
    }

    Timer_t Timer::tick() const
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return ((osg::Timer_t)tv.tv_sec)*1000000+(osg::Timer_t)tv.tv_usec;
    }

#endif
