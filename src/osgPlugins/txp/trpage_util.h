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

#ifndef trpage_util_h_
#define trpage_util_h_
#include <stdlib.h>
#include <trpage_read.h>
#include <trpage_write.h>
#include <trpage_scene.h>

TX_EXDECL class TX_CLDECL trpgUtil {
public:
    enum {DoReport = 1<<0,DoCopy = 1<<1, DoTileOpt = 1<<2};
    int merge(trpgr_Archive &inArch1,trpgr_Archive &inArch2,trpgwArchive &outArch, int flags = 0);
};
#endif
