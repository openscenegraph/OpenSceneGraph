
#ifndef IVE_VERSION
#define IVE_VERSION 1


/* The VERSION tag should be updated any time the
   IVE format changes in order to support backward
   compatibility (if implemented).  VERSION is 
   stored in the 2nd 4 bytes of the file */

#define VERSION_0002 0x00000002
#define VERSION_0003 0x00000003
#define VERSION_0004 0x00000004
#define VERSION_0005 0x00000005
#define VERSION      VERSION_0005


/* The BYTE_SEX tag is used to check the endian
   of the IVE file being read in.  The IVE format
   is always written in the native endian of the
   machine to provide optimum reading of the file.
   BYTE_SEX is stored in the first 4 bytes of the 
   file */
#define ENDIAN_TYPE 0x01020304
#define OPPOSITE_ENDIAN_TYPE 0x04030201


#endif
