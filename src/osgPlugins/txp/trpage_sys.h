/* ************************
   Copyright Terrain Experts Inc.
   Terrain Experts Inc (TERREX) reserves all rights to this source code
   unless otherwise specified in writing by the President of TERREX.
   This copyright may be updated in the future, in which case that version
   supercedes this one.
   -------------------
   Terrex Experts Inc.
   4400 East Broadway #314
   Tucson, AZ  85711
   info@terrex.com
   Tel: (520) 323-7990
   ************************
   */

/* trpage_sys.h
    System specific declarations.
    */

#ifndef trpage_sys_h_
#define trpage_sys_h_

#ifndef PATHSEPERATOR
#ifdef macintosh
#define PATHSEPERATOR ":"
#else
#define PATHSEPERATOR "/"
#endif
#endif

#if defined(_WIN32)
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
typedef __int64 int64;
#endif

#else   // Unix

#include <stdio.h>

// Delete a file
#define TRPGDELETEFILE(file) remove((file))

#ifndef int64
typedef long long int64;
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

#if defined(_WIN32)
#include <vector>
#include <map>
#else
#include <vector.h>
#include <map.h>
#endif

#if defined(_WIN32)     // PJM
// Had to take this out because of an iostream conflict
// Now referencing everything by std::
// using namespace std;
#endif

// We use long longs for addresses within a paging file
typedef int64 trpgllong;

// These are used to export classes from a DLL
// Definitely Windows specific
#include "trpage_ident.h"
#include "trdll.h"

#endif
