// -*-c++-*-

/*
 * Copyright (C) 2001 Ulrich Hertlein <u.hertlein@web.de>
 *
 * Uses libmpeg3 by Adam Williams
 * See http://www.heroinewarrior.com
 *
 * The Open Scene Graph (OSG) is a cross platform C++/OpenGL library for 
 * real-time rendering of large 3D photo-realistic models. 
 * The OSG homepage is http://www.openscenegraph.org/
 * 
 * This software is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "MpegImageStream.h"
#include <osg/Notify>
#include <osg/Timer>

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "libmpeg3/libmpeg3.h"

using namespace osg;

#define IDLE_TIMEOUT 150000L


// Constructor: setup and start thread
MpegImageStream::MpegImageStream(const char* fileName) : ImageStream()
{
    _useMMX = false;
    _fps = 0.0f;
    _frames = 0;
    _len = 0;
    _mpg = 0;

    for (int i = 0; i < NUM_CMD_INDEX; i++)
        _cmd[i] = THREAD_IDLE;
    _wrIndex = _rdIndex = 0;

    load(fileName);

    if (fileName)
        setFileName(fileName);
}


// Deconstructor: stop and terminate thread
MpegImageStream::~MpegImageStream()
{
    if( isRunning() )
    {
        quit(true);
    }

    mpeg3_t* mpg = (mpeg3_t*)_mpg;
    if (mpg) {
        mpeg3_close(mpg);
        mpg = NULL;
    }
    if (_rows) {
        delete [] _rows;
        _rows = NULL;
    }
    
    if (_videoWriteData)
    {
        delete [] _videoWriteData;
    }

}

// Set command
void MpegImageStream::setCmd(ThreadCommand cmd)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _cmd[_wrIndex] = cmd;
    _wrIndex = (_wrIndex + 1) % NUM_CMD_INDEX;
}


// Get command
MpegImageStream::ThreadCommand MpegImageStream::getCmd()
{
    ThreadCommand cmd = THREAD_IDLE;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    if (_rdIndex != _wrIndex)
    {
        cmd = _cmd[_rdIndex];
        _rdIndex = (_rdIndex + 1) % NUM_CMD_INDEX;
    }

    return cmd;
}


void MpegImageStream::load(const char* fileName)
{
    mpeg3_t* mpg = mpeg3_open((char*) fileName);
    if (!mpg) {
        osg::notify(WARN) << "Unable to open " << fileName << std::endl;
        return;
    }

    if (!mpeg3_has_video(mpg))
    {
        osg::notify(WARN) << "No video streams in" << fileName << std::endl;
        return;
    }
    if (mpeg3_has_audio(mpg))
    {
        osg::notify(NOTICE) << "Stream has audio" << std::endl;
    }

    _mpg = (void*)mpg;

    mpeg3_set_cpus(mpg, 1);
    mpeg3_set_mmx(mpg, _useMMX);

    int str = 0; //mpeg3_total_vstreams(mpg) - 1;

    _fps = mpeg3_frame_rate(mpg, str);
    _frames = mpeg3_video_frames(mpg, str);
    _len = (float) _frames / _fps;


    int s = mpeg3_video_width(mpg, str);
    int t = mpeg3_video_height(mpg, str);

    // Calculate texture size
    // these are also calculated and stored within osg::Texture but
    // too late (on the first apply) to be of any use...
    int texWidth = 1;
    for (; texWidth < s; texWidth <<= 1)
        ;
    int texHeight = 1;
    for (; texHeight < t; texHeight <<= 1)
        ;


    // Allocate image data
    // maybe use BGR888 and save some conversion somewhere?
    unsigned char* data = new unsigned char [s * t * 3];
    _videoWriteData = new unsigned char [s * t * 3];

    setImage(s, t, 0,
             GL_RGB,
             GL_RGB, GL_UNSIGNED_BYTE, data,
             osg::Image::USE_NEW_DELETE);


    // Allocate decoder rows
    // documentation says we need add'l bytes at the end of each
    // row for MMX but this is more efficient and works so far.
    _rows = new unsigned char* [t];
    
    // set up the rows to the current view write data. 
    swapData();


    osg::notify(INFO) << _frames << " @ " << _fps << " " << _len << "s" << std::endl;
    osg::notify(INFO) << "img " << s << "x" << t << std::endl;
    osg::notify(INFO) << "tex " << texWidth << "x" << texHeight << std::endl;
}


void MpegImageStream::swapData()
{
    std::swap(_videoWriteData,_data);
    unsigned char* dp = _videoWriteData;
    for (int i = 0; i < t(); i++)
    {
        _rows[t()-i-1] = dp;
        dp += (s() * 3);
    }
}


void MpegImageStream::quit(bool wiatForThreadToExit)
{
    osg::notify(osg::DEBUG_INFO)<<"Sending quit"<<std::endl;
    setCmd(THREAD_QUIT);

    if (wiatForThreadToExit)
    {
        while(isRunning())
        {
            osg::notify(osg::DEBUG_INFO)<<"Waiting for MpegImageStream to quit"<<std::endl;
            OpenThreads::Thread::YieldCurrentThread();
        }
    }
}


void MpegImageStream::run()
{
    bool playing = false;
    mpeg3_t* mpg = (mpeg3_t*)_mpg;
    int str = 0;
    float t0 = 0.0f;
    unsigned long delay = (unsigned long) ((1.0f / _fps) * 1000.0f);

    bool done = false;
    
    const osg::Timer* timer = osg::Timer::instance();
    osg::Timer_t start_tick = timer->tick();
    osg::Timer_t last_frame_tick = start_tick;
    double timePerFrame = 1.0f/_fps;
    double frameNumber = 0.0;
    
    while (!done)
    {
        // Handle commands
        ThreadCommand cmd = getCmd();
        if (cmd != THREAD_IDLE)
        {
            switch (cmd)
            {
            case THREAD_START: // Start or continue stream
                playing = true;
                break;
            case THREAD_STOP: // XXX
                playing = false;
                break;
            case THREAD_REWIND: // XXX
                t0 = 0.0;
                mpeg3_seek_percentage(mpg, 0.0);
                start_tick = timer->tick();                
                frameNumber = 0;
                break;
            case THREAD_CLOSE: // Stop and close
                playing = false;
                if (mpg)
                {
                    mpeg3_close(mpg);
                    mpg = NULL;
                }
                break;
            case THREAD_QUIT: 
                playing = false;
                done = true;
                break;
            default:
                osg::notify(osg::WARN) << "Unknown command " << cmd << std::endl;
                break;
            }
        }

        if (playing)
        {
            last_frame_tick = timer->tick();
            
            // XXX needs more work to be real-time
            mpeg3_read_frame(mpg, _rows,
                             0, 0, _s, _t,
                             _s, _t,
                             MPEG3_RGB888, str);

            swapData();

            dirty(); //Image();

            ++frameNumber;
            
            if (frameNumber>=_frames)
            {
                rewind(); 
                //stop();
            }
            else
            {
                while (timePerFrame*(frameNumber+1)>timer->delta_s(start_tick,timer->tick()))
                {
                    ::usleep(delay);
                }
            }
        }
        else if (!done) 
        {
            //OpenThreads::Thread::YieldCurrentThread();
            ::usleep(IDLE_TIMEOUT);
        }
    }
}
