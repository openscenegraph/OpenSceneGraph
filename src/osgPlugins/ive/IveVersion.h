
#ifndef IVE_VERSION
#define IVE_VERSION 1


/* The VERSION tag should be updated any time the
   IVE format changes in order to support backward
   compatibility (if implemented).  VERSION is 
   stored in the 2nd 4 bytes of the file */

#define VERSION_0001 1
#define VERSION_0002 2
#define VERSION_0003 3
#define VERSION_0004 4
#define VERSION_0005 5
#define VERSION_0006 6
#define VERSION_0007 7

#define VERSION_0008 8


#define VERSION      VERSION_0008


/* The BYTE_SEX tag is used to check the endian
   of the IVE file being read in.  The IVE format
   is always written in the native endian of the
   machine to provide optimum reading of the file.
   BYTE_SEX is stored in the first 4 bytes of the 
   file */
#define ENDIAN_TYPE 0x01020304
#define OPPOSITE_ENDIAN_TYPE 0x04030201


#endif
