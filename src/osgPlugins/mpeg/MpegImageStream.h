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

#ifndef _MPEGIMAGESTREAM_H_
#define _MPEGIMAGESTREAM_H_

#include <osg/ImageStream>

#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>

#define NUM_CMD_INDEX 4

namespace osg {

    /**
     * MPEG1/2 Image Stream class.
     */
    class SG_EXPORT MpegImageStream : public osg::ImageStream, public OpenThreads::Thread
    {
    public:
        MpegImageStream(const char* fileName = NULL);

        virtual Object* clone() const { return new MpegImageStream; }
        virtual bool isSameKindAs(const Object* obj) const {
            return dynamic_cast<const MpegImageStream*>(obj) != NULL;
        }
        virtual const char* className() const { return "MpegImageStream"; }

        /// Start or continue stream.
        virtual void play() { setCmd(THREAD_START); }

        /// Pause stream at current position.
        virtual void pause() { setCmd(THREAD_STOP); }

        /// Rewind stream to beginning.
        virtual void rewind() { setCmd(THREAD_REWIND); }

        /// Enable/disable MMX.
        inline void enableMMX(bool b) { _useMMX = b; }

        /**
         * Set frame rate in fps.
         * This is overwritten by the actual frame rate of the stream when
         * it is opened.
         */
        inline void setFrameRate(float fps) { _fps = (fps < 0.0f ? 0.0f : fps); }

        /// Get frame rate in fps.
        inline float getFrameRate() const { return _fps; }

        /// Get number of frames.
        inline long getNumFrames() const { return _frames; }

        /// Get total length in seconds.
        inline float getLength() const { return _len; }
        
        void load(const char* fileName);

        virtual void run();

    protected:
        virtual ~MpegImageStream();

    private:
        bool _useMMX;
        float _fps;
        long _frames;
        float _len;

        enum ThreadCommand {
            THREAD_IDLE = 0,
            THREAD_START,
            THREAD_STOP,
            THREAD_REWIND,
            THREAD_CLOSE,
            THREAD_QUIT
        };
        ThreadCommand _cmd[NUM_CMD_INDEX];
        int _wrIndex, _rdIndex;

        OpenThreads::Mutex _mutex;

        /// Set command.
        void setCmd(ThreadCommand cmd);

        /// Get command.
        ThreadCommand getCmd();

        /// Decoder hook.
        
        void* _mpg;
        unsigned char** _rows;

    };

} // namespace

#endif
