#ifndef __BROADCASTER_H
#define __BROADCASTER_H

////////////////////////////////////////////////////////////
// Broadcaster.h
//
// Class definition for broadcasting a buffer to a LAN
//

#if !defined(WIN32) || defined(__CYGWIN__)
    #include <netinet/in.h>
#endif

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
#endif
