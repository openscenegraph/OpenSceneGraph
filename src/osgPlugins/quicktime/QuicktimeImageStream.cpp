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
#include <osg/Math>

#include <OpenThreads/ScopedLock>
#include <OpenThreads/Thread>

#include "QTUtils.h"
#include "MovieData.h"



#define IDLE_TIMEOUT 150000L

int QuicktimeImageStream::_qtInstanceCount = 0;

// Constructor: setup and start thread
QuicktimeImageStream::QuicktimeImageStream(std::string fileName) : ImageStream()
{
    setOrigin(osg::Image::TOP_LEFT);

    _len = 0;
    _movieData = new MovieData();

    for (int i = 0; i < NUM_CMD_INDEX; i++)
      _cmd[i] = THREAD_IDLE;
    _wrIndex = _rdIndex = 0;

    load(fileName);

    if (!fileName.empty())
      setFileName(fileName);

    // ricky
    _status = ImageStream::PAUSED;
}


// Deconstructor: stop and terminate thread
QuicktimeImageStream::~QuicktimeImageStream()
{
   if( isRunning() )
   {
      quit(true);
   }


   // clean up quicktime movies.
   delete _movieData;

}


// Set command
void QuicktimeImageStream::setCmd(ThreadCommand cmd, double rate)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

   _cmd[_wrIndex] = cmd;
   _rates[_wrIndex] = rate;
   _wrIndex = (_wrIndex + 1) % NUM_CMD_INDEX;
}


// Get command
QuicktimeImageStream::ThreadCommand QuicktimeImageStream::getCmd()
{
   ThreadCommand cmd = THREAD_IDLE;

   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

   if (_rdIndex != _wrIndex) {
      cmd = _cmd[_rdIndex];
      _currentRate = _rates[_rdIndex];
      _rdIndex = (_rdIndex + 1) % NUM_CMD_INDEX;
   }

   return cmd;
}


void QuicktimeImageStream::load(std::string fileName)
{
   OSG_DEBUG << "QT-ImageStream: loading quicktime movie from " << fileName << std::endl;

   _movieData->load(this, fileName);

   _len = _movieData->getMovieDuration();
   _current = 0.0;
}

void QuicktimeImageStream::quit(bool wiatForThreadToExit)
{
   OSG_DEBUG<<"Sending quit"<<this<<std::endl;
   setCmd(THREAD_QUIT);

   if (isRunning() && wiatForThreadToExit)
   {
      join();
      OSG_DEBUG<<"QuicktimeImageStream has quit"<<this<<std::endl;
   }
}

void QuicktimeImageStream::setVolume(float volume)
{
    _movieData->setVolume(osg::minimum(osg::maximum(volume,0.0f),1.0f));
}

// Get and Set the playback volume of the stream.
float QuicktimeImageStream::getVolume() const
{
    return _movieData->getVolume();
}

void QuicktimeImageStream::run()
{

   bool playing = false;
   bool done = false;

   while (!done) {


      ThreadCommand cmd = getCmd();
      OSG_DEBUG << "movietime: " << _movieData->getMovieTime() << " rate: " << _movieData->getMovieRate() << " state " << cmd << " playing: " << playing << " done " << done << "  " << _wrIndex << "/" << _rdIndex << std::endl;
      // Handle commands
      {
         if (cmd != THREAD_IDLE) {
            OSG_DEBUG << "new cmd: " << cmd << std::endl;
            switch (cmd) {
                    case THREAD_START: // Start or continue stream
                       applyLoopingMode();
                       _movieData->setMovieRate(1.0);

                       playing = true;
                       break;

                    case THREAD_STOP:
                       _movieData->setMovieRate(0);
                       OSG_INFO << "QT-ImageStream: stop at "<< std::endl;
                       playing = false;
                       break;

                    case THREAD_REWIND:
                       SetMovieRate(_movieData->getMovie(),0.0);
                       GoToBeginningOfMovie(_movieData->getMovie());
                       break;

                    case THREAD_FORWARD:
                       SetMovieRate(_movieData->getMovie(),0.0);
                       GoToEndOfMovie(_movieData->getMovie());
                       break;

                    case THREAD_SEEK:
                       _movieData->setMovieTime(_currentRate);
                       playing = true;
                       break;

                    case THREAD_SETRATE:
                       _movieData->setMovieRate(_currentRate);
                       playing = (_currentRate != 0.0);
                       break;

                    case THREAD_CLOSE:
                       _movieData->setMovieRate(0.0);
                       break;

                    case THREAD_QUIT: // TODO
                       _movieData->setMovieRate(0.0);
                       OSG_INFO << "QT-ImageStream: quit" << std::endl;
                       //playing = false;
                       done = true;
                       break;
                    default:
                       OSG_WARN << "QT-ImageStream: Unknown command " << cmd << std::endl;
                       break;
            }
         }

         MoviesTask(_movieData->getMovie(),0);
         _current = _movieData->getMovieTime();
      }


      if (_lastUpdate!= _current)
      {
         // force the texture to update the image
         dirty();
         // update internal time and take care of looping
         _lastUpdate = _current;
      }

      if (playing)
      {
         OpenThreads::Thread::microSleep(16000);
      }
      else if (!done)
      {
         OpenThreads::Thread::microSleep(IDLE_TIMEOUT);
      }
   }


}


void QuicktimeImageStream::applyLoopingMode()
{
    OSG_INFO << "applying loop mode " << getLoopingMode() << std::endl;
    _movieData->setLooping(getLoopingMode() == osg::ImageStream::LOOPING);
}
