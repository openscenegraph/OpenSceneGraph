#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>


#include "receiver.h"

Receiver::Receiver( void )
{
    _port = 0;
    _initialized = false;
    _buffer = 0L;
}

Receiver::~Receiver( void )
{
    close( _so );
}

bool Receiver::init( void )
{
    if( _port == 0 )
    {
	fprintf( stderr, "Receiver::init() - port not defined\n" );
	return false;
    }

    if( (_so = socket( AF_INET, SOCK_DGRAM, 0 )) < 0 )
    {
        perror( "socket" );
	return false;
    }
    int on = 1;
    setsockopt( _so, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port   = htons( _port );
    saddr.sin_addr.s_addr =  0;

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
    _initialized || init();

    if( _buffer == 0L )
    {
	fprintf( stderr, "Receiver::sync() - No buffer\n" );
	return;
    }

#ifdef __linux
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

    recvfrom( _so, (caddr_t)_buffer, _buffer_size, 0, 0, &size );
    while( select( _so+1, &fdset, 0L, 0L, &tv ) )
    {
        if( FD_ISSET( _so, &fdset ) )
	{
            recvfrom( _so, (caddr_t)_buffer, _buffer_size, 0, 0, &size );
	}
    }
}

