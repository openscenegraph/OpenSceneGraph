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

/* trdll.h
    Windows Only

    This header file defines the declaration macros for DLLs.
  */

// Export/import declaration for classes and functions
// Use EXDECL class CPPDECL ... for a class
// Use CPPDECL foo *bar(bletch) for a stand-alone C++ function
// Use CDECL foo *bar(bletch) for a C function
// __COMP_DLL must be defined explicitly in Build->Settings->Code Generation
// __CURR_DLL must be defined in each header in a DLL
#ifdef TX_CLDECL
#undef TX_CLDECL
#undef TX_EXDECL
#undef TX_CDECL
#undef TX_CPPDECL
#undef TX_DODECL
#endif
// Work through cases
// If we're not compiling a DLL, or compiling the wrong DLL do import
//  declarations (0), otherwise do export declarations (1)
#if !defined (__COMP_DLL)
// {secret}
#define TX_DODECL 0
#else
#if __COMP_DLL == __CURR_DLL
#define TX_DODECL 1
#else
#define TX_DODECL 0
#endif
#endif

// #if !defined (__CURR_DLL) || __COMP_DLL != __CURR_DLL
#if TX_DODECL == 0
// Modified by Paul J. Metzger (pjm@rbd.com) to support static link libraries.
// Here's the original code:
//      #define TX_CLDECL __declspec( dllimport )
//      #define TX_EXDECL
//      #ifdef __cplusplus
//      #define TX_CDECL extern "C" __declspec(dllimport)
//      #else
//      #define TX_CDECL extern __declspec(dllimport)
//      #endif
//      #define TX_CPPDECL extern __declspec(dllimport)

#ifndef TX_CLDECL
// Class declaration.  Goes after "class" to handle DLL export in windows.
#define TX_CLDECL
// Goes before "class" to handle DLL export in windows
#define TX_EXDECL       /* no-op */
// Exports a C++ function properly in a windows DLL
#define TX_CPPDECL      /* no-op */
// Exports a C function properly in a windows DLL
#define TX_CDECL        /* no-op */
// {secret}
#ifndef TXDUMMY_DLL_MAIN
#define TXDUMMY_DLL_MAIN /* no-op */
#endif
#endif

#else
#define TX_CLDECL __declspec( dllexport )
#define TX_EXDECL
#define TX_CPPDECL extern __declspec(dllexport)
#ifdef __cplusplus
#define TX_CDECL extern "C" __declspec(dllexport)
#else
#define TX_CDECL extern __declspec(dllexport)
#endif

// The following is a DLL Main function for DLLs that wouldn't otherwise
// have one.  It's needed to initialize the run time library.
// This should appear once within every DLL
#ifndef TXDUMMY_DLL_MAIN
#define TXDUMMY_DLL_MAIN \
extern "C" { \
BOOL WINAPI _CRT_INIT (HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved); \
BOOL APIENTRY DllMain (HANDLE hDLL, DWORD dwReason, LPVOID lpReserved) \
{ \
  switch (dwReason) \
  { \
    case DLL_PROCESS_ATTACH: \
    { \
      if (!_CRT_INIT (hDLL, dwReason, lpReserved)) \
        return FALSE; \
      break; \
    } \
\
    case DLL_PROCESS_DETACH: \
    { \
      break; \
    } \
  } \
  return TRUE; \
} \
}

#endif
#endif

#ifndef txdll_h_
// {secret}
#define txdll_h_

#endif
