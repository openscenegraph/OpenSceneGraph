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

/* trpage_basic.cpp
	Methods for checkable base class.
	*/

#include <stdlib.h>
#include <stdio.h>

#include <trpage_io.h>

/* Checkable
	This is just a class that checks validity.
	Starts out invalid.
	*/

trpgCheckable::trpgCheckable()
{
	valid = false;
	handle = -1;
	writeHandle = false;
}
trpgCheckable::~trpgCheckable()
{
	valid = false;
}
bool trpgCheckable::isValid() const
{
	return valid;
}

