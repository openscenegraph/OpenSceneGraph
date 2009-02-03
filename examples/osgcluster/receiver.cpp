/* OpenSceneGraph example, osgcluster.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

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


#include "receiver.h"

#include <iostream>

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
#if defined (WIN32) && !defined(__CYGWIN__)
//    const BOOL on = TRUE;
//    setsockopt( _so, SOL_SOCKET, SO_REUSEADDR, (const char*) &on, sizeof(int));
#else
    int on = 1;
    setsockopt( _so, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
#endif

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

#if defined(__linux) || defined(__FreeBSD__) || defined( __APPLE__ )
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
    recvfrom( _so, (char *)_buffer, _buffer_size, 0, (sockaddr*)&saddr, &size );
//    recvfrom(sock_Receive, szMessage, 256, 0, (sockaddr*)&addr_Cli, &clilen)
    int err = WSAGetLastError ();
    if (err!=0) fprintf( stderr, "Receiver::sync() - error %d\n",err );

    while( select( _so+1, &fdset, 0L, 0L, &tv ) )
    {
        if( FD_ISSET( _so, &fdset ) )
        {
            recvfrom( _so, (char *)_buffer, _buffer_size, 0, (sockaddr*)&saddr, &size );
        }
    }
#else
    recvfrom( _so, (caddr_t)_buffer, _buffer_size, 0, 0, &size );
    while( select( _so+1, &fdset, 0L, 0L, &tv ) )
    {
        if( FD_ISSET( _so, &fdset ) )
        {
            recvfrom( _so, (caddr_t)_buffer, _buffer_size, 0, 0, &size );
        }
    }
#endif
}

