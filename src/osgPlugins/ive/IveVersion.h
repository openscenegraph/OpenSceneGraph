
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
#define VERSION_0009 9
#define VERSION_0010 10
#define VERSION_0011 11
#define VERSION_0012 12
#define VERSION_0013 13
#define VERSION_0014 14
#define VERSION_0015 15
#define VERSION_0016 16
#define VERSION_0017 17
#define VERSION_0018 18
#define VERSION_0019 19
#define VERSION_0020 20
#define VERSION_0021 21
#define VERSION_0022 22
#define VERSION_0023 23
#define VERSION_0024 24
#define VERSION_0025 25
#define VERSION_0026 26
#define VERSION_0027 27
#define VERSION_0028 28
#define VERSION_0029 29
#define VERSION_0030 30
#define VERSION_0031 31
#define VERSION_0032 32
#define VERSION_0033 33
#define VERSION_0034 34
#define VERSION_0035 35
#define VERSION_0036 36
#define VERSION_0037 37
#define VERSION_0038 38
#define VERSION_0039 39
#define VERSION_0040 40
#define VERSION_0041 41
#define VERSION_0042 42
#define VERSION_0043 43
#define VERSION_0044 44
#define VERSION_0045 45

#define VERSION VERSION_0045

/* The BYTE_SEX tag is used to check the endian
   of the IVE file being read in.  The IVE format
   is always written in the native endian of the
   machine to provide optimum reading of the file.
   BYTE_SEX is stored in the first 4 bytes of the
   file */
#define ENDIAN_TYPE 0x01020304
#define OPPOSITE_ENDIAN_TYPE 0x04030201


#endif
