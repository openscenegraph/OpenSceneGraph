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

/* trpage_basic.cpp
	Methods for checkable base class.
	*/

#include <stdlib.h>
#include <stdio.h>

#include "trpage_io.h"

/* Checkable
	This is just a class that checks validity.
	Starts out invalid.
	*/

trpgCheckable::trpgCheckable()
{
	valid = false;
}
trpgCheckable::~trpgCheckable()
{
	valid = false;
}
bool trpgCheckable::isValid() const
{
	return valid;
}

