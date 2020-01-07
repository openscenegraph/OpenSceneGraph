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

#include "Cluster.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>

#if !defined (WIN32) || defined(__CYGWIN__)
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <net/if.h>
#include <netdb.h>
#endif

#include <string.h>

#if defined(__linux)
    #include <unistd.h>
    #include <linux/sockios.h>
#elif defined(__FreeBSD__) || defined(__DragonFly__) || defined(__FreeBSD_kernel__)
    #include <unistd.h>
    #include <sys/sockio.h>
#elif defined(__sgi)
    #include <unistd.h>
    #include <net/soioctl.h>
#elif defined(__CYGWIN__)
    #include <unistd.h>
#elif defined (__GNU__)
    #include <unistd.h>
#elif defined(__sun)
    #include <unistd.h>
    #include <sys/sockio.h>
#elif defined (__APPLE__)
    #include <unistd.h>
    #include <sys/sockio.h>
#elif defined (WIN32)
    #include <winsock.h>
    #include <stdio.h>
#elif defined (__hpux)
    #include <unistd.h>
#else
    #error Teach me how to build on this system
#endif

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#if defined (WIN32) && !defined(__CYGWIN__)
#include <winsock.h>
#else
#include <unistd.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#endif
#include <string.h>

#include <osg/io_utils>
#include <iostream>

const unsigned int CameraPacket::MAX_NUM_EVENTS = 10;
const unsigned int CameraPacket::SWAP_BYTES_COMPARE = 0x12345678;

void DataConverter::write(const osg::FrameStamp& fs)
{
    osg::notify(osg::NOTICE)<<"writeFramestamp = "<<fs.getFrameNumber()<<" "<<fs.getReferenceTime()<<std::endl;

    writeUInt(fs.getFrameNumber());
    writeDouble(fs.getReferenceTime());
}

void DataConverter::read(osg::FrameStamp& fs)
{
    fs.setFrameNumber(readUInt());
    fs.setReferenceTime(readDouble());

    osg::notify(osg::NOTICE)<<"readFramestamp = "<<fs.getFrameNumber()<<" "<<fs.getReferenceTime()<<std::endl;
}

void DataConverter::write(const osg::Matrix& matrix)
{
    writeDouble(matrix(0,0));
    writeDouble(matrix(0,1));
    writeDouble(matrix(0,2));
    writeDouble(matrix(0,3));

    writeDouble(matrix(1,0));
    writeDouble(matrix(1,1));
    writeDouble(matrix(1,2));
    writeDouble(matrix(1,3));

    writeDouble(matrix(2,0));
    writeDouble(matrix(2,1));
    writeDouble(matrix(2,2));
    writeDouble(matrix(2,3));

    writeDouble(matrix(3,0));
    writeDouble(matrix(3,1));
    writeDouble(matrix(3,2));
    writeDouble(matrix(3,3));

    osg::notify(osg::NOTICE)<<"writeMatrix = "<<matrix<<std::endl;

}

void DataConverter::read(osg::Matrix& matrix)
{
    matrix(0,0) = readDouble();
    matrix(0,1) = readDouble();
    matrix(0,2) = readDouble();
    matrix(0,3) = readDouble();

    matrix(1,0) = readDouble();
    matrix(1,1) = readDouble();
    matrix(1,2) = readDouble();
    matrix(1,3) = readDouble();

    matrix(2,0) = readDouble();
    matrix(2,1) = readDouble();
    matrix(2,2) = readDouble();
    matrix(2,3) = readDouble();

    matrix(3,0) = readDouble();
    matrix(3,1) = readDouble();
    matrix(3,2) = readDouble();
    matrix(3,3) = readDouble();

    osg::notify(osg::NOTICE)<<"readMatrix = "<<matrix<<std::endl;

}

void DataConverter::write(const osgGA::GUIEventAdapter& event)
{
    writeUInt(event.getEventType());
    writeUInt(event.getKey());
    writeUInt(event.getButton());
    writeInt(event.getWindowX());
    writeInt(event.getWindowY());
    writeUInt(event.getWindowWidth());
    writeUInt(event.getWindowHeight());
    writeFloat(event.getXmin());
    writeFloat(event.getYmin());
    writeFloat(event.getXmax());
    writeFloat(event.getYmax());
    writeFloat(event.getX());
    writeFloat(event.getY());
    writeUInt(event.getButtonMask());
    writeUInt(event.getModKeyMask());
    writeDouble(event.getTime());
}

void DataConverter::read(osgGA::GUIEventAdapter& event)
{
    event.setEventType((osgGA::GUIEventAdapter::EventType)readUInt());
    event.setKey(readUInt());
    event.setButton(readUInt());
    int x = readInt();
    int y = readInt();
    int width = readUInt();
    int height = readUInt();
    event.setWindowRectangle(x,y,width,height);
    float xmin = readFloat();
    float ymin = readFloat();
    float xmax = readFloat();
    float ymax = readFloat();
    event.setInputRange(xmin,ymin,xmax,ymax);
    event.setX(readFloat());
    event.setY(readFloat());
    event.setButtonMask(readUInt());
    event.setModKeyMask(readUInt());
    event.setTime(readDouble());
}

void DataConverter::write(CameraPacket& cameraPacket)
{
    writeUInt(cameraPacket._byte_order);

    writeUInt(cameraPacket._masterKilled);

    write(cameraPacket._matrix);
    write(cameraPacket._frameStamp);

    writeUInt(cameraPacket._events.size());
    for(osgGA::EventQueue::Events::iterator itr = cameraPacket._events.begin();
        itr != cameraPacket._events.end();
        ++itr)
    {
        osgGA::GUIEventAdapter* event = (*itr)->asGUIEventAdapter();
        if (event) write(*(event));
    }
}

void DataConverter::read(CameraPacket& cameraPacket)
{
    cameraPacket._byte_order = readUInt();
    if (cameraPacket._byte_order != CameraPacket::SWAP_BYTES_COMPARE)
    {
        _swapBytes = !_swapBytes;
    }

    cameraPacket._masterKilled = readUInt()!=0;

    read(cameraPacket._matrix);
    read(cameraPacket._frameStamp);

    cameraPacket._events.clear();
    unsigned int numEvents = readUInt();
    for(unsigned int i=0;i<numEvents;++i)
    {
        osgGA::GUIEventAdapter* event = new osgGA::GUIEventAdapter;
        read(*(event));
        cameraPacket._events.push_back(event);
    }
}

void CameraPacket::readEventQueue(osgViewer::Viewer& viewer)
{
    _events.clear();

    viewer.getEventQueue()->copyEvents(_events);

    osg::notify(osg::INFO)<<"written events = "<<_events.size()<<std::endl;
}

void CameraPacket::writeEventQueue(osgViewer::Viewer& viewer)
{
    osg::notify(osg::INFO)<<"received events = "<<_events.size()<<std::endl;

    // copy the events to osgProducer style events.
    viewer.getEventQueue()->appendEvents(_events);
}


//////////////////////////////////////////////////////////////////////////////
//
//  Receiver
//
Receiver::Receiver( void )
{
    _port = 0;
    _initialized = false;
    _buffer = 0L;
}

Receiver::~Receiver( void )
{
#if defined (WIN32) && !defined(__CYGWIN__)
    closesocket( _so);
#else
    close( _so );
#endif
}

bool Receiver::init( void )
{
#if defined(WIN32) && !defined(__CYGWIN__)
    WORD version = MAKEWORD(1,1);
    WSADATA wsaData;
    // First, we start up Winsock
    WSAStartup(version, &wsaData);
#endif

    if( _port == 0 )
    {
    fprintf( stderr, "Receiver::init() - port not defined\n" );
    return false;
    }

    if( (_so = socket( AF_INET, SOCK_DGRAM, 0 )) < 0 )
    {
        perror( "Socket" );
    return false;
    }

    int result = 0;

#if defined (WIN32) && !defined(__CYGWIN__)
//    const BOOL on = TRUE;
//    setsockopt( _so, SOL_SOCKET, SO_REUSEADDR, (const char*) &on, sizeof(int));
#else
    int on = 1;
    result = setsockopt( _so, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
#endif

    if (result)
    {
        OSG_NOTICE<<"Warning: Reciever::init() setsockopt(..) failed, errno="<<errno<<std::endl;
        return false;
    }

//    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port   = htons( _port );
#if defined (WIN32) && !defined(__CYGWIN__)
    saddr.sin_addr.s_addr =  htonl(INADDR_ANY);
#else
    saddr.sin_addr.s_addr =  0;
#endif

    if( bind( _so, (struct sockaddr *)&saddr, sizeof( saddr )) < 0 )
    {
        perror( "bind" );
        return false;
    }

    _initialized = true;
    return _initialized;
}


void Receiver::setPort( const short port )
{
    _port = port;
}

void Receiver::setBuffer( void *buffer, const unsigned int size )
{
    _buffer = buffer;
    _buffer_size = size;
}

void Receiver::sync( void )
{
    if(!_initialized) init();

    if( _buffer == 0L )
    {
        fprintf( stderr, "Receiver::sync() - No buffer\n" );
        return;
    }

#if defined(__linux) || defined(__FreeBSD__) || defined( __APPLE__ ) || \
    defined(__DragonFly__) || defined(__FreeBSD_kernel__) || defined(__GNU__)
    socklen_t
#else
    int
#endif
        size = sizeof( struct sockaddr_in );

    fd_set fdset;
    FD_ZERO( &fdset );
    FD_SET( _so, &fdset );

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

#if defined (WIN32) && !defined(__CYGWIN__)
//    saddr.sin_port   = htons( _port );
    int result = recvfrom( _so, (char *)_buffer, _buffer_size, 0, (sockaddr*)&saddr, &size );
//    recvfrom(sock_Receive, szMessage, 256, 0, (sockaddr*)&addr_Cli, &clilen)
    //int err = WSAGetLastError ();
    //int *dum = (int*) _buffer;

    if (result<0)
    {
        OSG_NOTICE<<"Warning: Receiver::sync() recvfrom(..) failed, errno="<<errno<<std::endl;
        return;
    }

    while( select( _so+1, &fdset, 0L, 0L, &tv ) )
    {
        if( FD_ISSET( _so, &fdset ) )
        {
            recvfrom( _so, (char *)_buffer, _buffer_size, 0, (sockaddr*)&saddr, &size );
        }
    }
#else
    int result = recvfrom( _so, (caddr_t)_buffer, _buffer_size, 0, 0, &size );
    if (result<0)
    {
        OSG_NOTICE<<"Warning: Receiver::sync() recvfrom(..) failed, errno="<<errno<<std::endl;
        return;
    }

    while( select( _so+1, &fdset, 0L, 0L, &tv ) )
    {
        if( FD_ISSET( _so, &fdset ) )
        {
            recvfrom( _so, (caddr_t)_buffer, _buffer_size, 0, 0, &size );
        }
    }
#endif
}


//////////////////////////////////////////////////////////////////////////////
//
//  Broadcaster
//
Broadcaster::Broadcaster( void )
{
    _port = 0;
    _initialized = false;
    _buffer = 0L;
    _address = 0;
}

Broadcaster::~Broadcaster( void )
{
#if defined (WIN32) && !defined(__CYGWIN__)
    closesocket( _so);
#else
    close( _so );
#endif
}

bool Broadcaster::init( void )
{
#if defined (WIN32) && !defined(__CYGWIN__)
    WORD version = MAKEWORD(1,1);
    WSADATA wsaData;
    // First, we start up Winsock
    WSAStartup(version, &wsaData);
#endif

    if( _port == 0 )
    {
        fprintf( stderr, "Broadcaster::init() - port not defined\n" );
        return false;
    }

    if( (_so = socket( AF_INET, SOCK_DGRAM, 0 )) < 0 )
    {
        perror( "Socket" );
        return false;
    }
#if defined (WIN32) && !defined(__CYGWIN__)
    const BOOL on = TRUE;
#else
    int on = 1;
#endif

    int result = 0;

#if defined (WIN32) && !defined(__CYGWIN__)
    result = setsockopt( _so, SOL_SOCKET, SO_REUSEADDR, (const char *) &on, sizeof(int));
#else
    result = setsockopt( _so, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
#endif

    if (result) return false;

    saddr.sin_family = AF_INET;
    saddr.sin_port   = htons( _port );
    if( _address == 0 )
    {
#if defined (WIN32) && !defined(__CYGWIN__)
        result = setsockopt( _so, SOL_SOCKET, SO_BROADCAST, (const char *) &on, sizeof(int));
#else
        result = setsockopt( _so, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
#endif
        if (result) return false;

#if !defined (WIN32) || defined(__CYGWIN__)
        struct ifreq ifr;
#endif
#if defined (__linux) || defined(__CYGWIN__)
        strcpy( ifr.ifr_name, "eth0" );
#elif defined(__sun)
        strcpy( ifr.ifr_name, "hme0" );
#elif !defined (WIN32)
        strcpy( ifr.ifr_name, "ef0" );
#endif
#if defined (WIN32) // get the server address
        saddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    }
#else
        if( (ioctl( _so, SIOCGIFBRDADDR, &ifr)) < 0 )
        {
            perror( "Broadcaster::init() Cannot get Broadcast Address" );
            return false;
        }
            saddr.sin_addr.s_addr = (((sockaddr_in *)&ifr.ifr_broadaddr)->sin_addr.s_addr);
        }
        else
        {
            saddr.sin_addr.s_addr = _address;
        }
#endif
#define _VERBOSE 1
#ifdef _VERBOSE
    unsigned char *ptr = (unsigned char *)&saddr.sin_addr.s_addr;
    printf( "Broadcast address : %u.%u.%u.%u\n", ptr[0], ptr[1], ptr[2], ptr[3] );
#endif

    _initialized = true;
    return _initialized;
}

void Broadcaster::setHost( const char *hostname )
{
    struct hostent *h;
    if( (h = gethostbyname( hostname )) == 0L )
    {
        fprintf( stderr, "Broadcaster::setHost() - Cannot resolv an address for \"%s\".\n", hostname );
        _address = 0;
    }
    else
        _address = *(( unsigned long  *)h->h_addr);
}

void Broadcaster::setPort( const short port )
{
    _port = port;
}

void Broadcaster::setBuffer( void *buffer, const unsigned int size )
{
    _buffer = buffer;
    _buffer_size = size;
}

void Broadcaster::sync( void )
{
    if(!_initialized) init();

    if( _buffer == 0L )
    {
        fprintf( stderr, "Broadcaster::sync() - No buffer\n" );
        return;
    }

    int result = 0;
#if defined (WIN32) && !defined(__CYGWIN__)
    unsigned int size = sizeof( SOCKADDR_IN );
    result = sendto( _so, (const char *)_buffer, _buffer_size, 0, (struct sockaddr *)&saddr, size );
    // int err = WSAGetLastError ();
    // int *dum = (int*) _buffer;
#else
    unsigned int size = sizeof( struct sockaddr_in );
    result = sendto( _so, (const void *)_buffer, _buffer_size, 0, (struct sockaddr *)&saddr, size );
#endif

    if (result)
    {
        OSG_NOTICE<<"Warning: sentTo(...) failed : errno="<<errno<<std::endl;
    }
}
