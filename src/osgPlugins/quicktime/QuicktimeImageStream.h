// -*-c++-*-

/*
 * Copyright (C) 2004 Stephan Huber http://digitalmind.de
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

#ifndef _QUICKTIMEIMAGESTREAM_H_
#define _QUICKTIMEIMAGESTREAM_H_

#include <osg/ImageStream>

#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>

#define NUM_CMD_INDEX 20

namespace osg {

    class MovieData;

    /**
     * Quicktime Image Stream class.
     */
    class QuicktimeImageStream : public osg::ImageStream, public OpenThreads::Thread
    {
    public:
        QuicktimeImageStream(std::string fileName = "");

        virtual Object* clone() const { return new QuicktimeImageStream; }
        virtual bool isSameKindAs(const Object* obj) const {
            return dynamic_cast<const QuicktimeImageStream*>(obj) != NULL;
        }
        virtual const char* className() const { return "QuicktimeImageStream"; }

        /// Start or continue stream.
        virtual void play()
        {
            if (!isRunning()) start();
            
            setCmd(THREAD_START);
        }

        /// Pause stream at current position.
        virtual void pause() { setCmd(THREAD_STOP); }

        /// Rewind stream to beginning.
        virtual void rewind() { setCmd(THREAD_REWIND); }

        virtual void quit(bool wiatForThreadToExit);
	
        /// Get total length in seconds.
        inline float getLength() const { return _len; }
        
        void load(std::string fileName);

        virtual void run();

    protected:
        virtual ~QuicktimeImageStream();

    private:
        float _lastUpdate;
        float _len;
        
        MovieData* _data;

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


    };

} // namespace

#endif
