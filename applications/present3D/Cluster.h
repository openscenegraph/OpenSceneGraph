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

#ifndef CLUSTER_H
#define CLUSTER_H

#include <osg/Matrix>
#include <osg/FrameStamp>

#include <osgViewer/Viewer>

#if !defined(WIN32) || defined(__CYGWIN__)
    #include <netinet/in.h>
#else
    #include "winsock.h"
#endif

////////////////////////////////////////////////////////////
// Receiver.h
//
// Class definition for the recipient of a broadcasted message
//
class Receiver 
{
    public :

	Receiver();
	~Receiver();

   	// setBuffer defines the buffer into which the broadcasted
	// message will be received.
	void setBuffer( void *buffer, const unsigned int size );

 	// Define what port to listen and bind to
	void setPort( const short port );

	// Sync does a blocking wait to recieve next message
	void sync( void );

    private :
	bool init( void );

    private :
#if defined (WIN32) && !defined(__CYGWIN__)
        SOCKET _so;
        SOCKADDR_IN saddr;
#else
        int _so;
        struct sockaddr_in saddr;
#endif
    bool _initialized;
    short _port;
    void *_buffer;
    unsigned int _buffer_size;
};

////////////////////////////////////////////////////////////
// Broadcaster.h
//
// Class definition for broadcasting a buffer to a LAN
//
class Broadcaster  
{
    public :

	Broadcaster( void );
	~Broadcaster( void );

	// Set the broadcast port
	void setPort( const short port );

	// Set the buffer to be broadcast
	void setBuffer( void *buffer, const unsigned int buffer_size );

	// Set a recipient host.  If this is used, the Broadcaster
	// no longer broadcasts, but rather directs UDP packets at
	// host.
	void setHost( const char *hostname ); 

	// Sync broadcasts the buffer
	void sync( void );

    private :
	bool init( void );

    private :
#if defined(WIN32) && !defined(__CYGWIN__)
        SOCKET _so;
#else
        int _so;
#endif
        bool _initialized;
        short _port;
        void *_buffer;
        unsigned int _buffer_size;
#if defined(WIN32) && !defined(__CYGWIN__)
        SOCKADDR_IN saddr;
#else
        struct sockaddr_in saddr;
#endif
        unsigned long _address;
};

class CameraPacket {
    public:
    
        static const unsigned int MAX_NUM_EVENTS;
        static const unsigned int SWAP_BYTES_COMPARE;
    
        CameraPacket():_masterKilled(false) 
        {
            _byte_order = SWAP_BYTES_COMPARE;
        }
        
        void setPacket(const osg::Matrix& matrix,const osg::FrameStamp* frameStamp)
        {
            _matrix = matrix;
            if (frameStamp)
            {
                _frameStamp    = *frameStamp;
            }
        }
        
        void getModelView(osg::Matrix& matrix,float angle_offset=0.0f)
        {
        
            matrix = _matrix * osg::Matrix::rotate(osg::DegreesToRadians(angle_offset),0.0f,1.0f,0.0f);
        }
        
        void readEventQueue(osgViewer::Viewer& viewer);
        
        void writeEventQueue(osgViewer::Viewer& viewer);

        void setMasterKilled(const bool flag) { _masterKilled = flag; }
        const bool getMasterKilled() const { return _masterKilled; }
        
        unsigned int    _byte_order;
        bool            _masterKilled;
        osg::Matrix     _matrix;

        // note don't use a ref_ptr as used elsewhere for FrameStamp
        // since we don't want to copy the pointer - but the memory.
        // FrameStamp doesn't have a private destructor to allow
        // us to do this, even though its a reference counted object.    
        osg::FrameStamp  _frameStamp;
        
        osgGA::EventQueue::Events _events;
        
};

class DataConverter
{
    public:

        DataConverter(unsigned int numBytes):
            _startPtr(0),
            _endPtr(0),
            _swapBytes(false),
            _currentPtr(0)
        {
            _currentPtr = _startPtr = new char[numBytes];
            _endPtr = _startPtr+numBytes;
            _numBytes = numBytes;
        }

        ~DataConverter()
        {
            delete [] _startPtr;
        }

        void reset()
        {
            _currentPtr = _startPtr;
        }

        inline void write1(char* ptr)
        {
            if (_currentPtr+1>=_endPtr) return;

            *(_currentPtr++) = *(ptr); 
        }

        inline void read1(char* ptr)
        {
            if (_currentPtr+1>=_endPtr) return;

            *(ptr) = *(_currentPtr++); 
        }

        inline void write2(char* ptr)
        {
            if (_currentPtr+2>=_endPtr) return;

            *(_currentPtr++) = *(ptr++); 
            *(_currentPtr++) = *(ptr); 
        }

        inline void read2(char* ptr)
        {
            if (_currentPtr+2>=_endPtr) return;

            if (_swapBytes)
            {
                *(ptr+1) = *(_currentPtr++); 
                *(ptr) = *(_currentPtr++); 
            }
            else
            {
                *(ptr++) = *(_currentPtr++); 
                *(ptr) = *(_currentPtr++); 
            }
        }

        inline void write4(char* ptr)
        {
            if (_currentPtr+4>=_endPtr) return;

            *(_currentPtr++) = *(ptr++); 
            *(_currentPtr++) = *(ptr++); 
            *(_currentPtr++) = *(ptr++); 
            *(_currentPtr++) = *(ptr); 
        }

        inline void read4(char* ptr)
        {
            if (_currentPtr+4>=_endPtr) return;

            if (_swapBytes)
            {
                *(ptr+3) = *(_currentPtr++); 
                *(ptr+2) = *(_currentPtr++); 
                *(ptr+1) = *(_currentPtr++); 
                *(ptr) = *(_currentPtr++); 
            }
            else
            {
                *(ptr++) = *(_currentPtr++); 
                *(ptr++) = *(_currentPtr++); 
                *(ptr++) = *(_currentPtr++); 
                *(ptr) = *(_currentPtr++); 
            }
        }

        inline void write8(char* ptr)
        {
            if (_currentPtr+8>=_endPtr) return;

            *(_currentPtr++) = *(ptr++); 
            *(_currentPtr++) = *(ptr++); 
            *(_currentPtr++) = *(ptr++); 
            *(_currentPtr++) = *(ptr++); 

            *(_currentPtr++) = *(ptr++); 
            *(_currentPtr++) = *(ptr++); 
            *(_currentPtr++) = *(ptr++); 
            *(_currentPtr++) = *(ptr); 
        }

        inline void read8(char* ptr)
        {
            char* endPtr = _currentPtr+8;
            if (endPtr>=_endPtr) return;

            if (_swapBytes)
            {
                *(ptr+7) = *(_currentPtr++); 
                *(ptr+6) = *(_currentPtr++); 
                *(ptr+5) = *(_currentPtr++); 
                *(ptr+4) = *(_currentPtr++); 

                *(ptr+3) = *(_currentPtr++); 
                *(ptr+2) = *(_currentPtr++); 
                *(ptr+1) = *(_currentPtr++); 
                *(ptr) = *(_currentPtr++); 
            }
            else
            {
                *(ptr++) = *(_currentPtr++); 
                *(ptr++) = *(_currentPtr++); 
                *(ptr++) = *(_currentPtr++); 
                *(ptr++) = *(_currentPtr++); 

                *(ptr++) = *(_currentPtr++); 
                *(ptr++) = *(_currentPtr++); 
                *(ptr++) = *(_currentPtr++); 
                *(ptr) = *(_currentPtr++); 
            }
        }

        inline void writeChar(char c)               { write1(&c); }
        inline void writeUChar(unsigned char c)     { write1((char*)&c); }
        inline void writeShort(short c)             { write2((char*)&c); }
        inline void writeUShort(unsigned short c)   { write2((char*)&c); }
        inline void writeInt(int c)                 { write4((char*)&c); }
        inline void writeUInt(unsigned int c)       { write4((char*)&c); }
        inline void writeFloat(float c)             { write4((char*)&c); }
        inline void writeDouble(double c)           { write8((char*)&c); }

        inline char readChar() { char c; read1(&c); return c; }
        inline unsigned char readUChar() { unsigned char c; read1((char*)&c); return c; }
        inline short readShort() { short c; read2((char*)&c); return c; }
        inline unsigned short readUShort() { unsigned short c; read2((char*)&c); return c; }
        inline int readInt() { int c; read4((char*)&c); return c; }
        inline unsigned int readUInt() { unsigned int c; read4((char*)&c); return c; }
        inline float readFloat() { float c; read4((char*)&c); return c; }
        inline double readDouble() { double c; read8((char*)&c); return c; }

        void write(const osg::FrameStamp& fs);
        void read(osg::FrameStamp& fs);

        void write(const osg::Matrix& matrix);
        void read(osg::Matrix& matrix);

        void write(const osgGA::GUIEventAdapter& event);
        void read(osgGA::GUIEventAdapter& event);
        
        void write(CameraPacket& cameraPacket);
        void read(CameraPacket& cameraPacket);

        char* startPtr() { return _startPtr; }
        unsigned int numBytes() { return _numBytes; }
        
    protected:

        char* _startPtr;
        char* _endPtr;
        unsigned int _numBytes;
        bool _swapBytes;

        char* _currentPtr;
};



#endif 
