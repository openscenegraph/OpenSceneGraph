#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <net/if.h>
#include <netdb.h>
#include <string.h>

#if defined(__linux)
#  include <linux/sockios.h>
#elif defined(__FreeBSD__)
#  include <sys/sockio.h>
#elif defined(__sgi)
#  include <net/soioctl.h>
#elif defined(__CYGWIN__)
// nothing needed
#else
#  error Teach me how to build on this system
#endif

#include "broadcaster.h"

#define _VERBOSE 1

Broadcaster::Broadcaster( void )
{
    _port = 0;
    _initialized = false;
    _buffer = 0L;
    _address = 0;
}

Broadcaster::~Broadcaster( void )
{
    close( _so );
}

bool Broadcaster::init( void )
{
    if( _port == 0 )
    {
	fprintf( stderr, "Broadcaster::init() - port not defined\n" );
	return false;
    }

    if( (_so = socket( AF_INET, SOCK_DGRAM, 0 )) < 0 )
    {
        perror( "socket" );
	return false;
    }
    int on = 1;
    setsockopt( _so, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    saddr.sin_family = AF_INET;
    saddr.sin_port   = htons( _port );
    if( _address == 0 )
    {
    struct ifreq ifr;
    setsockopt( _so, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
#ifdef __linux
    strcpy( ifr.ifr_name, "eth0" );
#else
    strcpy( ifr.ifr_name, "ef0" );
#endif
    if( (ioctl( _so, SIOCGIFBRDADDR, &ifr)) < 0 )
    {
        perror( "Broadcaster::init() Cannot get Broadcast Address" );
	return false;
    }
    saddr.sin_addr.s_addr = (
		((sockaddr_in *)&ifr.ifr_broadaddr)->sin_addr.s_addr);
    }
    else
    {
	saddr.sin_addr.s_addr = _address;
    }
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
    _initialized || init();

    if( _buffer == 0L )
    {
	fprintf( stderr, "Broadcaster::sync() - No buffer\n" );
	return;
    }

    unsigned int size = sizeof( struct sockaddr_in );
    sendto( _so, (const void *)_buffer, _buffer_size,
			    0, (struct sockaddr *)&saddr, size );

}

