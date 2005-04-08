#ifndef __QTTEXTURE_H__
#define __QTTEXTURE_H__

#ifdef __cplusplus
extern "C" {
#endif

unsigned char* LoadBufferFromDarwinPath ( const char *fname, long *origWidth, 
			long *origHeight, long *origDepth,
			long *buffWidth, long *buffHeight, long *buffDepth);
								
char* QTfailureMessage(void);
FSSpec *darwinPathToFSSpec (char *fname );

#ifdef __cplusplus
}
#endif

#endif
