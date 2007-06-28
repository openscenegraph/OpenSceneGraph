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



class MovieData;

/**
* Quicktime Image Stream class. streams a quicktime movie into an image
*/
class QuicktimeImageStream : public osg::ImageStream, public OpenThreads::Thread
{
public:
   /** Constructor
   * @param fileName movie to open */
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

      // ricky
      _status = ImageStream::PLAYING;
   }

   /// sets the movierate of this movie
   void setMovieRate(float rate) {
      if (!isRunning()) start();
      setCmd(THREAD_SETRATE,rate);
   }

   /// Pause stream at current position.
   virtual void pause() 
   {
      setCmd(THREAD_STOP); 
      // ricky
      _status = ImageStream::PAUSED;
   }

   /// Rewind stream to beginning.
   virtual void rewind() { setCmd(THREAD_REWIND); }

   /// forward stream to the end
   virtual void forward() { setCmd(THREAD_FORWARD); }

   /// stop playing 
   virtual void quit(bool wiatForThreadToExit);

   /// Get total length in seconds.
   virtual double getLength() const
   { 
     return double(_len);
   }

   /// jumps to a specific position 
   void jumpTo(float pos) {
      setCmd(THREAD_SEEK, pos);
   }

   /// returns the current playing position
   inline float getCurrentTime() const { return _current; }

   /// @return the current moviedata-object
   MovieData* getMovieData() { return _movieData; }

   /// loads a movie from fileName
   void load(std::string fileName);

   /// starts the thread
   virtual void run();

   /// Go to a specific position in the stream.
   virtual void setReferenceTime(double time)
   {
     jumpTo(float(time));
   }
   /// Return the current position in the stream.
   virtual double getReferenceTime() const
   { 
     return double(getCurrentTime());
   }
           
   // Set the time multiplier if you want to speed up,
   // slow down, or go normal speed.
   virtual void setTimeMultiplier(double multiplier)
   {
     setMovieRate(float(multiplier));
   }
   virtual double getTimeMultiplier()
   { 
     return 0.0;
   }

    // Get and Set the playback volume of the stream.
    virtual void setVolume(float volume);
    virtual float getVolume() const;


protected:
   /// apply the looping mode to quicktime
   virtual void applyLoopingMode();
   /// destructor
   virtual ~QuicktimeImageStream();

private:
   float _lastUpdate;
   float _len;
   float _current;
   float _currentRate;

   MovieData* _movieData;

   enum ThreadCommand {
      THREAD_IDLE = 0,
      THREAD_START,
      THREAD_STOP,
      THREAD_REWIND,
      THREAD_FORWARD,
      THREAD_SEEK,
      THREAD_SETRATE,
      THREAD_CLOSE,
      THREAD_QUIT
   };
   ThreadCommand _cmd[NUM_CMD_INDEX];
   float _rates[NUM_CMD_INDEX];
   int _wrIndex, _rdIndex;

   OpenThreads::Mutex _mutex;

   /// Set command.
   void setCmd(ThreadCommand cmd, float rate = 0.0f);

   /// Get command.
   ThreadCommand getCmd();

   // ricky
   static int _qtInstanceCount;
};


#endif
