/* -*-c++-*-
*
*  OpenSceneGraph example, osgcluster.
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

#ifndef __RECEIVER_H
#define __RECEIVER_H


////////////////////////////////////////////////////////////
// Receiver.h
//
// Class definition for the recipient of a broadcasted message
//

#if !defined(WIN32) || defined(__CYGWIN__)
    #include <netinet/in.h>
#endif

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
#endif 
