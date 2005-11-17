// sockinet.C  -*- C++ -*- socket library
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
// 2002-07-25 Version 1.2 (C) Herbert Straub
//     Adding improved Error Handling in sockerr class
//     sockinetaddr::setport if the first character of the port parameter is a 
//         digit, then the parameter is interpreted as a number
// 2002-07-28 Version 1.2 (C) Herbert Straub
//  Eliminating sorry_about_global_temp inititialisation. This don't work
//  in combination with NewsCache. My idea is: initializing the classes with (0)
//  and in the second step call ios::init (sockinetbuf *) and iosockstream::init ...
//  The constructors of isockinet, osockinet and iosockinet are changed.

#include "sockinet.h"
#if defined( __APPLE__ ) && ( __GNUC__ < 4 )
typedef int socklen_t;
#endif

#if defined(__CYGWIN__) || !defined(WIN32)
extern "C" {
#    include <netdb.h>
#    include <sys/time.h>
#    include <sys/socket.h>
#    include <stdlib.h>
#    include <unistd.h>
#    include <errno.h>
#    include <netinet/tcp.h>
#   include <netinet/in.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>

}
#else
#    define socklen_t int
# define EADDRNOTAVAIL                WSAEADDRNOTAVAIL
# define EADDRINUSE                        WSAEADDRINUSE
#    define ENOPROTOOPT                    WSAENOPROTOOPT
#endif // !WIN32

#ifndef INADDR_NONE
#define INADDR_NONE             ((in_addr_t) 0xffffffff)
#endif

void    herror(const char*);

sockinetaddr::sockinetaddr () 
{
  sin_family      = sockinetbuf::af_inet;
  sin_addr.s_addr = htonl(INADDR_ANY);
  sin_port      = 0;
}

sockinetaddr::sockinetaddr(unsigned long addr, int port_no)
// addr and port_no are in host byte order
{
  sin_family      = sockinetbuf::af_inet;
  sin_addr.s_addr = htonl(addr);
  sin_port      = htons(port_no);
}

sockinetaddr::sockinetaddr(unsigned long addr, const char* sn, const char* pn)
// addr is in host byte order
{
  sin_family      = sockinetbuf::af_inet;
  sin_addr.s_addr = htonl (addr); // Added by cgay@cs.uoregon.edu May 29, 1993
  setport(sn, pn);
}

sockinetaddr::sockinetaddr (const char* host_name, int port_no)
// port_no is in host byte order
{
  setaddr(host_name);
  sin_port = htons(port_no);
}

sockinetaddr::sockinetaddr(const char* hn, const char* sn, const char* pn)
{
  setaddr(hn);
  setport(sn, pn);
}

sockinetaddr::sockinetaddr (const sockinetaddr& sina): sockAddr()
{
  sin_family      = sockinetbuf::af_inet;
  sin_addr.s_addr = sina.sin_addr.s_addr;
  sin_port          = sina.sin_port;
}   

void sockinetaddr::setport(const char* sn, const char* pn)
{
    if (isdigit (*sn)) {
        sin_port = htons(atoi(sn));
    } else {
      servent* sp = getservbyname(sn, pn);
      if (sp == 0) throw sockerr (EADDRNOTAVAIL, "sockinetaddr::setport");
      sin_port = sp->s_port;
    }
}

int sockinetaddr::getport () const
{
  return ntohs (sin_port);
}

void sockinetaddr::setaddr(const char* host_name)
{
  if ( (sin_addr.s_addr = inet_addr(host_name)) == INADDR_NONE) {
    hostent* hp = gethostbyname(host_name);
    if (hp == 0) throw sockerr (EADDRNOTAVAIL, "sockinetaddr::setaddr");
    memcpy(&sin_addr, hp->h_addr, hp->h_length);
    sin_family = hp->h_addrtype;
  } else
    sin_family = sockinetbuf::af_inet;
}

const char* sockinetaddr::gethostname () const
{
  if (sin_addr.s_addr == htonl(INADDR_ANY)) {
    static char hostname[64];
    if (::gethostname(hostname, 63) == -1) return "";
    return hostname;        
  }
  
  hostent* hp = gethostbyaddr((const char*) &sin_addr,
                  sizeof(sin_addr),
                  family());
  if (hp == 0) return "";
  if (hp->h_name) return hp->h_name;
  return "";
}

sockinetbuf::sockinetbuf (const sockbuf::sockdesc& sd)
  : sockbuf (sd.sock)
{}

sockinetbuf::sockinetbuf(sockbuf::type ty, int proto)
  : sockbuf (af_inet, ty, proto)
{}

sockinetaddr sockinetbuf::localaddr() const
{
  sockinetaddr sin;
  int len = sin.size();
#ifdef __sgi
  if (::getsockname(rep->sock, sin.addr (), (int *) // LN
#else
  if (::getsockname(rep->sock, sin.addr (), (socklen_t*) // LN
#endif
                    &len) == -1)
    throw sockerr (errno, "sockinetbuf::localaddr");
  return sin;
}

int sockinetbuf::localport() const
{
  sockinetaddr sin = localaddr();
  if (sin.family() != af_inet) return -1;
  return sin.getport();
}

const char* sockinetbuf::localhost() const
{
  sockinetaddr sin = localaddr();
  if (sin.family() != af_inet) return "";
  return sin.gethostname();
}

sockinetaddr sockinetbuf::peeraddr() const
{
  sockinetaddr sin;
  int len = sin.size();
#ifdef __sgi
  if (::getpeername(rep->sock, sin.addr (), (int *) // LN
#else
  if (::getpeername(rep->sock, sin.addr (), (socklen_t*) // LN
#endif
                    &len) == -1)
    throw sockerr (errno, "sockinetbuf::peeraddr");
  return sin;
}

int sockinetbuf::peerport() const
{
  sockinetaddr sin = peeraddr();
  if (sin.family() != af_inet) return -1;
  return sin.getport();
}

const char* sockinetbuf::peerhost() const
{
  sockinetaddr sin = peeraddr();
  if (sin.family() != af_inet) return "";
  return sin.gethostname();
}

void sockinetbuf::bind_until_success (int portno)
// a. bind to (INADDR_ANY, portno)
// b. if success return
// c. if failure and errno is EADDRINUSE, portno++ and go to step a.
{
  for (;;) {
    try {
      bind (portno++);
    }
    catch (sockerr e) {
//      if (e.errno () != EADDRINUSE) throw;
      if (e.serrno () != EADDRINUSE) throw; // LN
      continue;
    }
    break;
  }
}

void sockinetbuf::bind (sockAddr& sa)
{
  sockbuf::bind (sa);
}

void sockinetbuf::bind (int port_no)
{
    sockinetaddr sa ((long unsigned int) // LN
                     INADDR_ANY, port_no);
  bind (sa);
}

void sockinetbuf::bind (unsigned long addr, int port_no)
// address and portno are in host byte order
{
  sockinetaddr sa (addr, port_no);
  bind (sa);
}

void sockinetbuf::bind (const char* host_name, int port_no)
{
  sockinetaddr sa (host_name, port_no);
  bind (sa);
}

void sockinetbuf::bind (unsigned long addr,
            const char* service_name,
            const char* protocol_name)
{
  sockinetaddr sa (addr, service_name, protocol_name);
  bind (sa);
}

void sockinetbuf::bind (const char* host_name,
            const char* service_name,
            const char* protocol_name)
{
  sockinetaddr sa (host_name, service_name, protocol_name);
  bind (sa);
}

void sockinetbuf::connect (sockAddr& sa)
{
  sockbuf::connect (sa);
}

void sockinetbuf::connect (unsigned long addr, int port_no)
// address and portno are in host byte order
{
  sockinetaddr sa (addr, port_no);
  connect (sa);
}

void sockinetbuf::connect (const char* host_name, int port_no)
{
  sockinetaddr sa (host_name, port_no);
  connect (sa);
}

void sockinetbuf::connect (unsigned long addr,
               const char* service_name,
               const char* protocol_name)
{
  sockinetaddr sa (addr, service_name, protocol_name);
  connect (sa);
}

void sockinetbuf::connect (const char* host_name,
               const char* service_name,
               const char* protocol_name)
{
  sockinetaddr sa (host_name, service_name, protocol_name);
  connect (sa);
}

sockbuf::sockdesc sockinetbuf::accept ()
{
  return sockbuf::accept ();
}

sockbuf::sockdesc sockinetbuf::accept (sockAddr& sa)
{
  return sockbuf::accept (sa);
}

sockbuf::sockdesc sockinetbuf::accept (unsigned long addr,
                      int port_no)
{
  sockinetaddr sa (addr, port_no);
  return accept (sa);
}

sockbuf::sockdesc sockinetbuf::accept (const char* host_name,
                      int port_no)
{
  sockinetaddr sa (host_name, port_no);
  return accept (sa);
}

bool sockinetbuf::tcpnodelay () const
{
  struct protoent* proto = getprotobyname ("tcp");
  if (proto == 0) throw sockerr (ENOPROTOOPT, "sockinetbuf::tcpnodelay");

  int old = 0;
  getopt (TCP_NODELAY, &old, sizeof (old), proto->p_proto);
  return old!=0;
}

bool sockinetbuf::tcpnodelay (bool set) const
{
  struct protoent* proto = getprotobyname ("tcp");
  if (proto == 0) throw sockerr (ENOPROTOOPT, "sockinetbuf::tcpnodelay");

  int old = 0;
  int opt = set;
  getopt (TCP_NODELAY, &old, sizeof (old), proto->p_proto);
  setopt (TCP_NODELAY, &opt, sizeof (opt), proto->p_proto);
  return old!=0;
}

isockinet::isockinet (const sockbuf::sockdesc& sd)
  : ios(0), isockstream(0)
{
    sockinetbuf *t = new sockinetbuf (sd);

    ios::init (t);
    isockstream::init (t);
}

isockinet::isockinet (sockbuf::type ty, int proto)
  : ios (0), isockstream(0)
{
    sockinetbuf *t = new sockinetbuf (ty, proto);

    ios::init (t);
    isockstream::init (t);
}

isockinet::isockinet (const sockinetbuf& sb)
  : ios (0), isockstream(0)
{
    sockinetbuf *t = new sockinetbuf (sb);

    ios::init (t);
    isockstream::init (t);
}

isockinet::~isockinet ()
{
  delete ios::rdbuf ();
}

osockinet::osockinet (const sockbuf::sockdesc& sd)
  : ios (0), osockstream(0)
{
    sockinetbuf *t = new sockinetbuf (sd);

    ios::init (t);
    osockstream::init (t);
}

osockinet::osockinet (sockbuf::type ty, int proto)
  : ios (0), osockstream(0)
{
    sockinetbuf *t = new sockinetbuf (ty, proto);

    ios::init (t);
    osockstream::init (t);
}

osockinet::osockinet (const sockinetbuf& sb)
  : ios (0), osockstream(0)
{
    sockinetbuf *t = new sockinetbuf (sb);

    ios::init (t);
    osockstream::init (t);
}

osockinet::~osockinet ()
{
  delete ios::rdbuf ();
}

iosockinet::iosockinet (const sockbuf::sockdesc& sd)
  : ios (0), iosockstream(0)
{
    sockinetbuf *t = new sockinetbuf(sd);

    ios::init (t);
    iosockstream::init (t);
}

iosockinet::iosockinet (sockbuf::type ty, int proto)
    : ios (0), iosockstream (0)
{
     sockinetbuf *t = new sockinetbuf (ty, proto);

    ios::init (t);
    iosockstream::init (t);
}

iosockinet::iosockinet (const sockinetbuf& sb)
  : ios (0), iosockstream(0)
{
    sockinetbuf *t = new sockinetbuf (sb);

    ios::init (t);
    iosockstream::init (t);
}

iosockinet::~iosockinet ()
{
  delete ios::rdbuf ();
}
