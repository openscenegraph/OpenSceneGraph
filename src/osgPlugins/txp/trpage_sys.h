/* ************************
   Copyright Terrain Experts Inc.
   Terrain Experts Inc (TERREX) reserves all rights to this source code
   unless otherwise specified in writing by the Chief Operating Officer
   of TERREX.
   This copyright may be updated in the future, in which case that version
   supercedes this one.
   -------------------
   Terrex Experts Inc.
   84 West Santa Clara St., Suite 380
   San Jose, CA 95113
   info@terrex.com
   Tel: (408) 293-9977
   ************************
   */

/* trpage_sys.h
    System specific declarations.
    */

#ifndef trpage_sys_h_
#define trpage_sys_h_

#if defined(_WIN32) && !defined(__CYGWIN__)
/*    *********************
    System Specific Section.
    This is currently set up for win32.
    *********************
    */

#include <windows.h>

// Microsoft Developer warnings that annoy me
#pragma warning ( disable : 4251)
#pragma warning ( disable : 4275)
#pragma warning ( disable : 4786)

// Somewhat system independent file deletion macro
#define TRPGDELETEFILE(file) DeleteFile((file))

#ifndef int64
 // 64 bit long value.  Need this for really big files.
// #ifdef __CYGWIN__
// typedef long long int64;
// #else
 typedef __int64 int64;
// #endif
#endif

#ifdef __MINGW32__
#include <stdio.h>
#endif

      
#else   // Unix

#include <stdio.h>

// Delete a file
#define TRPGDELETEFILE(file) remove((file))

//#ifndef int64
//typedef long long int64;
//#endif

#if defined(sgi) && defined(unix)
#  include <sgidefs.h>
typedef __int64_t  int64;

#elif defined(sun) && defined(unix) && (defined(SUN551) || defined(SUN56))
// NOTE: SUN56 and SUN551 is assumed to be defined in our makefiles.
#include <sys/types.h>
typedef longlong_t int64;

#elif defined(sun) && defined(unix)
// This should work on SunOS 5.7 and later.
#include <sys/types.h>
typedef int64_t  int64;

#elif defined(linux)
#include <sys/types.h>
typedef int64_t  int64;
//typedef long long int int64;

#elif defined(__ghs__) && defined(__LL_Is_64)
typedef long long int64;

#elif defined(__CYGWIN__)
typedef long long int64;
#include <windows.h>

#else
typedef int int64;  // DON'T KNOW WHAT TO DO
#endif

#endif

// Basic data types
#ifndef uint8
typedef unsigned char uint8;
#endif
#ifndef int32
typedef int int32;
#endif
#ifndef uint32
typedef unsigned int uint32;
#endif
#ifndef float32
typedef float float32;
#endif
#ifndef float64
typedef double float64;
#endif

// Note: replace this with your own STL implementation
//   You can use the Microsoft provided one by deleting the first #include
#ifdef USEROGUE
#include <txRogueWave.h>
#endif

#if defined(WIN32) || defined(_WIN32) || defined(vxw) || (defined(sgi) && defined(_STANDARD_C_PLUS_PLUS)) || (defined(__GNUC__) && (__GNUC__>=2) && (__GNUC_MINOR__>=91))
#include <vector>
#include <map>
namespace std {}
using namespace std;
#else
#include <vector.h>
#include <map.h>
#endif

//#if defined(_WIN32)
//#include <vector>
//#include <map>
//#else
//#include <vector.h>
//#include <map.h>
//#endif

//#if defined(_WIN32)     // PJM
//using namespace std;
//#endif

// We use long longs for addresses within a paging file
typedef int64 trpgllong;

// These are used to export classes from a DLL
// Definitely Windows specific
#include "trpage_ident.h"
#include "trdll.h"

#endif
