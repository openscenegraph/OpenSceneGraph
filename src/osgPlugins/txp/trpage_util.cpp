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

#include <trpage_util.h>

/* trpage_util.cpp
    This source file implements various utility routines for paging archive
    */

/*  The merge routine used to be in here.
    However, merge isn't actually general enough to be part of the library.
    Only portable code should be in the TerraPage API.
    Instead, there's a Windows specific program that merges TerraPage archives in
    the merge/ directory of this distribution.
 */
