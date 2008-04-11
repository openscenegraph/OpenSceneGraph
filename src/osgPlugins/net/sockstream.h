// sockstream.h -*- C++ -*- socket library
// Copyright (C) 2002 Herbert Straub
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
// 
// Copyright (C) 1992-1996 Gnanasekaran Swaminathan <gs4t@virginia.edu>
//
// Permission is granted to use at your own risk and distribute this software
// in source and  binary forms provided  the above copyright notice and  this
// paragraph are  preserved on all copies.  This software is provided "as is"
// with no express or implied warranty.
//
// Version: 12Jan97 1.11
//
// Version: 1.2 2002-07-25 Herbert Straub 
//     Improved Error Handling - extending the sockerr class by cOperation
// 2003-03-06 Herbert Straub
//     adding sockbuf::getname und setname (sockname)
//     sockbuf methods throw method name + sockname

#ifndef _SOCKSTREAM_H
#define    _SOCKSTREAM_H

#include <iostream> // must be ANSI compatible
//#include <cstddef>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <string>
//#include <cstdio>
#if defined(__CYGWIN__) || !defined(WIN32)
#  include <sys/types.h>
#  include <sys/uio.h>
#  include <sys/socket.h>
#  define SOCKET int
#  define SOCKET_ERROR -1
#else
#  include <windows.h>
#  include <wininet.h>
//#  include <errno.h>
#ifdef _MSC_VER 
#  pragma comment(lib, "Wininet")
#endif
#endif


using namespace std;

#if defined(__linux__) || defined(__CYGWIN__)
#  define MSG_MAXIOVLEN     16
#endif // __linux__

// socket exception classes
class sockerr 
{
    int  err;
    string text;
    public:
        sockerr (int e, const char *operation = NULL): err (e) 
        {
            if (operation != NULL) 
            {
                text = operation;
            }
        }
        sockerr (int e, const char *operation, const char *specification) : err (e) 
        {
            if (operation != NULL)
                text = operation;
            if (specification != NULL) 
            {
                text += "(";
                text += specification;
                text += ")";
            }
        }
        sockerr (int e, const string &operation): err (e) 
        {
            text = operation;
        }
        sockerr (const sockerr &O) 
        {
            err = O.err;
            text = O.text;
        }

        const char* what () const { return "sockerr"; }
        const char* operation () const { return text.c_str(); }

//      int errno () const { return err; }
        int serrno () const { return err; } // LN
        const char* errstr () const;
        bool error (int eno) const { return eno == err; }

        bool io () const; // non-blocking and interrupt io recoverable error.
        bool arg () const; // incorrect argument supplied. recoverable error.
        bool op () const; // operational error. recovery difficult.

        bool conn () const;   // connection error
        bool addr () const;   // address error
        bool benign () const; // recoverable read/write error like EINTR etc.
};

class sockoob 
{
    public:
        const char* what () const { return "sockoob"; }
};  

// socket address classes
struct sockaddr;

class sockAddr 
{
    public:
        virtual ~sockAddr() {}
        virtual operator void* () const =0;
        operator sockaddr* () const { return addr (); }
        virtual int size() const =0;
        virtual int family() const =0;
        virtual sockaddr* addr      () const =0;
};

struct msghdr;

// socket buffer class
class sockbuf: public streambuf 
{
    public:
        enum type {
            sock_stream            = SOCK_STREAM,
            sock_dgram            = SOCK_DGRAM,
            sock_raw            = SOCK_RAW,
            sock_rdm            = SOCK_RDM,
            sock_seqpacket      = SOCK_SEQPACKET
        };
        enum option {
            so_debug            = SO_DEBUG,
            so_reuseaddr    = SO_REUSEADDR,
            so_keepalive    = SO_KEEPALIVE,
            so_dontroute    = SO_DONTROUTE,
            so_broadcast    = SO_BROADCAST,
            so_linger            = SO_LINGER,
            so_oobinline    = SO_OOBINLINE,
            so_sndbuf        = SO_SNDBUF,
            so_rcvbuf        = SO_RCVBUF,
            so_error        = SO_ERROR,
            so_type        = SO_TYPE
        };    
        enum level {
            sol_socket          = SOL_SOCKET
        };
        enum msgflag {
            msg_oob        = MSG_OOB,
            msg_peek            = MSG_PEEK,
            msg_dontroute    = MSG_DONTROUTE

#if !(defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__APPLE__))
            ,msg_maxiovlen    = MSG_MAXIOVLEN
#endif
        };
        enum shuthow {
            shut_read,
            shut_write,
            shut_readwrite
        };
        enum { somaxconn    = SOMAXCONN };
        struct socklinger {
            int    l_onoff;    // option on/off
            int    l_linger;    // linger time

            socklinger (int a, int b): l_onoff (a), l_linger (b) {}
        };

        typedef char          char_type;
        typedef streampos     pos_type;
        typedef streamoff     off_type;
        typedef int           int_type;
        typedef int           seekdir;
        //  const int_type eof = EOF;
        enum { eof = EOF }; // LN

        struct sockdesc {
            int sock;
            sockdesc (int d): sock (d) {}
        };

    protected:
        struct sockcnt {
            SOCKET    sock;
            int            cnt;
            int            stmo; // -1==block, 0==poll, >0 == waiting time in secs
            int            rtmo; // -1==block, 0==poll, >0 == waiting time in secs
            bool        oob;    // check for out-of-band byte while reading
            void*        gend; // end of input buffer
            void*        pend; // end of output buffer

            sockcnt(SOCKET s): 
                sock(s), cnt(1), stmo (-1), rtmo (-1), oob (false),
                gend (0), pend (0) {}
        };

        sockcnt* rep;  // counts the # refs to sock
        string        sockname; // name of sockbuf - Herbert Straub

#if 0
        virtual sockbuf*      setbuf (char_type* s, int_type* n);
        virtual pos_type      seekoff (off_type off,
                                       seekdir way,
                                       ios::openmode which = ios::in|ios::out);
        virtual pos_type      seekpos (pos_type sp,
                                       ios::openmode which = ios::in|ios::out);
#endif

        virtual int           sync ();
  
        virtual int           showmanyc () const;
        virtual streamsize    xsgetn (char_type* s, streamsize n);
        virtual int_type      underflow ();
        virtual int_type      uflow ();

        virtual int_type      pbackfail (int_type c = eof);

        virtual streamsize    xsputn (const char_type* s, streamsize n);
        virtual int_type      overflow (int_type c = eof);

    public:
        sockbuf (const sockdesc& sd);
        sockbuf (int domain, type, int proto);
        sockbuf (const sockbuf&);
//      sockbuf&        operator = (const sockbuf&);
        virtual ~sockbuf ();

        int sd () const { return rep->sock; }
        int pubsync () { return sync (); }
        virtual bool is_open () const;
    
        virtual void bind    (sockAddr&);
        virtual void connect    (sockAddr&);
    
        void listen    (int num=somaxconn);
        virtual sockdesc accept();
        virtual sockdesc accept(sockAddr& sa);
    
        int read(void* buf, int len);
        int recv    (void* buf, int len, int msgf=0);
        int recvfrom(sockAddr& sa, void* buf, int len, int msgf=0);

#if    !defined(__linux__) && !defined(WIN32)
        int recvmsg(msghdr* msg, int msgf=0);
        int sendmsg(msghdr* msg, int msgf=0);
#endif
    
        int write(const void* buf, int len);
        int send(const void* buf, int len, int msgf=0);
        int sendto    (sockAddr& sa, const void* buf, int len, int msgf=0);
    
        int sendtimeout (int wp=-1);
        int recvtimeout (int wp=-1);
        int is_readready (int wp_sec, int wp_usec=0) const;
        int is_writeready (int wp_sec, int wp_usec=0) const;
        int is_exceptionpending (int wp_sec, int wp_usec=0) const;
    
        void shutdown (shuthow sh);
    
        int getopt(int op, void* buf, int len,
                   int level=sol_socket) const;
        void setopt(int op, void* buf, int len,
                    int level=sol_socket) const;
    
        type gettype () const;
        int  clearerror () const;
        bool debug      () const;
        bool debug      (bool set) const;
        bool reuseaddr () const;
        bool reuseaddr (bool set) const;
        bool keepalive () const;
        bool keepalive (bool set) const;
        bool dontroute () const;
        bool dontroute (bool set) const;
        bool broadcast () const;
        bool broadcast (bool set) const;
        bool oobinline () const;
        bool oobinline (bool set) const;
        bool oob       () const { return rep->oob; }
        bool oob       (bool b);
        int  sendbufsz () const;
        int  sendbufsz (int sz)   const;
        int  recvbufsz () const;
        int  recvbufsz (int sz)   const;
        socklinger linger() const;
        socklinger linger(socklinger opt) const;
        socklinger linger(int onoff, int tm) const
        { return linger (socklinger (onoff, tm)); }

        bool atmark() const;  
        long nread() const;
        long howmanyc() const;
        void nbio(bool set=true) const;
        inline void setname(const char *name);
        inline void setname(const string &name);
        inline const string& getname();

#if defined(__CYGWIN__) || !defined(WIN32)
        void async(bool set=true) const;
#endif
#if !defined(WIN32)
        int  pgrp() const;
        int  pgrp(int new_pgrp) const;
        void closeonexec(bool set=true) const;
#endif
};

class isockstream: public istream 
{
    protected:
        //isockstream (): istream(rdbuf()), ios (0) {}

    public:
        isockstream(sockbuf* sb): ios (sb) , istream(sb) {}
        virtual ~isockstream () {}
                
        sockbuf* rdbuf () { return (sockbuf*)ios::rdbuf(); }
        sockbuf* operator -> () { return rdbuf(); }
};

class osockstream: public ostream 
{
    protected:
        //osockstream (): ostream(static_cast<>rdbuf()), ios (0) {}
    public:
        osockstream(sockbuf* sb): ios (sb) , ostream(sb) {}
        virtual ~osockstream () {}
        sockbuf* rdbuf () { return (sockbuf*)ios::rdbuf(); }
        sockbuf* operator -> () { return rdbuf(); }
};

class iosockstream: public iostream 
{
    protected:
        iosockstream ();
    public:
        iosockstream(sockbuf* sb): ios(sb), iostream(sb) {}
        virtual ~iosockstream () {}

        sockbuf* rdbuf () { return (sockbuf*)ios::rdbuf(); }
        sockbuf* operator -> () { return rdbuf(); }
};

// manipulators
extern osockstream& crlf (osockstream&);
extern osockstream& lfcr (osockstream&);

// inline

void sockbuf::setname (const char *name)
{
    sockname = name;
}

void sockbuf::setname (const string &name)
{
    sockname = name;
}

const string& sockbuf::getname ()
{
    return sockname;
}

#endif    // _SOCKSTREAM_H
