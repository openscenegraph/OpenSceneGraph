// -*-c++-*-

/*
 * Copyright (C) 2004 Stephan Huber http://digitalmind.de
 *
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

#include "QuicktimeImageStream.h"
#include <osg/Notify>
#include <osg/Timer>

#include "QTUtils.h"
#include "MovieData.h"


using namespace osg;

#define IDLE_TIMEOUT 150000L
#define ERR_MSG(no,msg) osg::notify(osg::WARN) << "QT-ImageStream: " << msg << " failed with error " << no << std::endl;



// Constructor: setup and start thread
QuicktimeImageStream::QuicktimeImageStream(std::string fileName) : ImageStream()
{
    
    osgQuicktime::initQuicktime(); 
        
    _len = 0;
    
    _data = new MovieData();

    for (int i = 0; i < NUM_CMD_INDEX; i++)
        _cmd[i] = THREAD_IDLE;
    _wrIndex = _rdIndex = 0;

    load(fileName);

    if (!fileName.empty())
        setFileName(fileName);

    startThread();
    start();
}


// Deconstructor: stop and terminate thread
QuicktimeImageStream::~QuicktimeImageStream()
{
    setCmd(THREAD_QUIT);

    if( isRunning() )
    {

        // cancel the thread..
        cancel();
        //join();

        // then wait for the the thread to stop running.
        while(isRunning())
        {
            osg::notify(osg::DEBUG_INFO)<<"Waiting for QuicktimeImageStream to cancel"<<std::endl;
            OpenThreads::Thread::YieldCurrentThread();
        }
        
    }
    
    delete _data;

   
}


// Set command
void QuicktimeImageStream::setCmd(ThreadCommand cmd)
{
    lock();
    _cmd[_wrIndex] = cmd;
    _wrIndex = (_wrIndex + 1) % NUM_CMD_INDEX;
    unlock();
}


// Get command
QuicktimeImageStream::ThreadCommand QuicktimeImageStream::getCmd()
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


void QuicktimeImageStream::load(std::string fileName)
{
    osg::notify(osg::DEBUG_INFO) << "QT-ImageStream: loading quicktime movie from " << fileName << std::endl;
    
    _data->load(this, fileName);
    
    _len = _data->getMovieDuration();
   
    
}


void QuicktimeImageStream::run()
{
    static OpenThreads::Mutex s_qtMutex;

    bool playing = false;
    bool done = false;
    OSErr err;
   
    // err = EnterMoviesOnThread(0);
    // ERR_MSG(err,"EnterMoviesOnThread");
            
    while (!done) {
        // osg::notify(osg::ALWAYS) << "movietime: " << _data->getMovieTime() << " state " << getCmd() << std::endl;
        // Handle commands
        ThreadCommand cmd = getCmd();

	s_qtMutex.lock();

        if (cmd != THREAD_IDLE) {
            switch (cmd) {
                case THREAD_START: // Start or continue stream
                    StartMovie(_data->getMovie());
                    playing = true;
                    break;
                    
                case THREAD_STOP:
                    SetMovieRate(_data->getMovie(),0);
                    osg::notify(NOTICE) << "QT-ImageStream: stop at "<< std::endl;
                    playing = false;
                    break;
                    
                case THREAD_REWIND:
                    SetMovieRate(_data->getMovie(),0);
                    GoToBeginningOfMovie(_data->getMovie());
                    break;
                    
                case THREAD_CLOSE:
                    SetMovieRate(_data->getMovie(),0);
                    break;
                    
                case THREAD_QUIT: // TODO
                    SetMovieRate(_data->getMovie(),0);
                    osg::notify(NOTICE) << "QT-ImageStream: quit" << std::endl;
                    done = true;
                    break;
                default:
                    osg::notify(osg::WARN) << "QT-ImageStream: Unknown command " << cmd << std::endl;
                    break;
            }
        }
        
        MoviesTask(_data->getMovie(),0);
        float currentTime = _data->getMovieTime();

	s_qtMutex.unlock();

        if (_lastUpdate!= currentTime) {
            dirty();
            _lastUpdate = currentTime;
        }
        
        if (playing) {
            // TODO
            
            
        }
        else {
            ::usleep(IDLE_TIMEOUT);
        }
    }
    // err = ExitMoviesOnThread();
    //ERR_MSG(err,"ExitMoviesOnThread");
}
