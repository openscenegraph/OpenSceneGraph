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

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "libmpeg3/libmpeg3.h"

using namespace osg;

#define IDLE_TIMEOUT 150000L


// Constructor: setup and start thread
MpegImageStream::MpegImageStream(const char* fileName) : ImageStream()
{
    _useMMX = true;
    _fps = 0.0f;
    _frames = 0;
    _len = 0;

    for (int i = 0; i < NUM_CMD_INDEX; i++)
        _cmd[i] = THREAD_IDLE;
    _wrIndex = _rdIndex = 0;

    ::pthread_mutex_init(&_mutex, NULL);
    ::pthread_create(&_id, NULL, MpegImageStream::s_decode, this);

    if (fileName)
        setFileName(fileName);
}


// Deconstructor: stop and terminate thread
MpegImageStream::~MpegImageStream()
{
    stop();
    setCmd(THREAD_QUIT);
    ::pthread_join(_id, NULL);
    ::pthread_mutex_destroy(&_mutex);
}


// Set command
void MpegImageStream::setCmd(ThreadCommand cmd)
{
    lock();
    _cmd[_wrIndex] = cmd;
    _wrIndex = (_wrIndex + 1) % NUM_CMD_INDEX;
    unlock();
}


// Get command
MpegImageStream::ThreadCommand MpegImageStream::getCmd()
{
    ThreadCommand cmd = THREAD_IDLE;
    lock();
    if (_rdIndex != _wrIndex) {
        cmd = _cmd[_rdIndex];
        _rdIndex = (_rdIndex + 1) % NUM_CMD_INDEX;
    }
    unlock();
    return cmd;
}


/*
 * Decoder thread
 */
void* MpegImageStream::s_decode(void* vp)
{
    return ((MpegImageStream*) vp)->decode(vp);
}

void* MpegImageStream::decode(void*)
{
    bool playing = false;
    mpeg3_t* mpg = NULL;
    int str = 0;
    float t0 = 0.0f;
    unsigned long delay = 0;
    unsigned char** rows = NULL;

    bool done = false;
    while (!done) {

        // Handle commands
        ThreadCommand cmd = getCmd();
        if (cmd != THREAD_IDLE) {
            switch (cmd) {
            case THREAD_START: // Start or continue stream
                playing = false;
                if (!mpg) {
                    mpg = mpeg3_open((char*) _fileName.c_str());
                    if (!mpg) {
                        osg::notify(WARN) << "Unable to open " << _fileName << std::endl;
                        continue;
                    }

                    if (!mpeg3_has_video(mpg)) {
                        osg::notify(WARN) << "No video streams in" << _fileName << std::endl;
                        continue;
                    }
                    if (mpeg3_has_audio(mpg)) {
                        std::cerr << "Stream has audio" << std::endl;
                    }

                    mpeg3_set_cpus(mpg, 1);
                    mpeg3_set_mmx(mpg, _useMMX);

                    str = 0; //mpeg3_total_vstreams(mpg) - 1;

                    _fps = mpeg3_frame_rate(mpg, str);
                    _frames = mpeg3_video_frames(mpg, str);
                    _len = (float) _frames / _fps;

                    t0 = 0.0f;
                    delay = (unsigned long) ((1.0f / _fps) * 1000L);

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
                    unsigned char* data = (unsigned char*) ::malloc(s * t * 3);
                    
                    
                    setImage(s, t, 0,
                             GL_RGB,
                             GL_RGB, GL_UNSIGNED_BYTE, data,
                             osg::Image::USE_MALLOC_FREE);

                    // Allocate decoder rows
                    // documentation says we need add'l bytes at the end of each
                    // row for MMX but this is more efficient and works so far.
                    rows = (unsigned char**) ::malloc(t * sizeof(unsigned char*));
                    unsigned char* dp = data;
                    for (int i = 0; i < t; i++) {
                        rows[i] = dp;
                        dp += (s * 3);
                    }

 
#if 0
                    // Setup texture matrix
                    Matrix mat;
                    mat.makeScale((float) s / (float) texWidth,
                                  ((float) t / (float) texHeight) * -1.0f, 1.0f);
                    mat = mat * Matrix::translate(0.0f, (float) t / (float) texHeight, 0.0f);
                    _texMat->setMatrix(mat);
#else
                    _texMat->setMatrix(osg::Matrix::scale(s,-t,1.0f)*osg::Matrix::translate(0.0f,t,0.0f));
#endif
                    // XXX
                    std::cerr << _frames << " @ " << _fps << " " << _len << "s" << std::endl;
                    std::cerr << "img " << s << "x" << t << std::endl;
                    std::cerr << "tex " << texWidth << "x" << texHeight << std::endl;
                    std::cerr << "delay " << delay << "ms" << std::endl;
                }
                playing = true;
                break;
            case THREAD_STOP: // XXX
                std::cerr << "stop at " << t0 << std::endl;
                playing = false;
                break;
            case THREAD_REWIND: // XXX
                t0 = 0.0;
                mpeg3_seek_percentage(mpg, 0.0);
                break;
            case THREAD_CLOSE: // Stop and close
                playing = false;
                if (mpg) {
                    mpeg3_close(mpg);
                    mpg = NULL;
                }
                break;
            case THREAD_QUIT: // XXX
                std::cerr << "quit" << std::endl;
                done = true;
                break;
            default:
                osg::notify(osg::WARN) << "Unknown command " << cmd << std::endl;
                break;
            }
        }

        if (playing) {
            // XXX needs more work to be real-time
            mpeg3_read_frame(mpg, rows,
                             0, 0, _s, _t,
                             _s, _t,
                             MPEG3_RGB888, str);
            dirty(); //Image();

            if (mpeg3_get_time(mpg) > _len) {
                rewind(); //stop();
            }
            else
                ::usleep(delay);
        }
        else {
            ::usleep(IDLE_TIMEOUT);
        }
    }

    // Cleanup decoder
    if (mpg) {
        mpeg3_close(mpg);
        mpg = NULL;
    }
    if (rows) {
        ::free(rows);
        rows = NULL;
    }

    return NULL;
}
