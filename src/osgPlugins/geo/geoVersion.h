/*===========================================================================*\

	NAME:			geoVersion.h

	DESCRIPTION:	Compile Time Library Version Info

	AUTHOR:			Andy Bushnell

	-------------------------------------------------------------------------


PROPRIETARY RIGHTS NOTICE:      
	
	This software contains proprietary information and trade secrets of Carbon
	Graphics LLC. No part or all of this software may be reproduced in any 
	form, without the written permission of Carbon Graphics LLC. 

	This software file can only be used in conjunction with the Geo SDK & 
	libraries to create Plugin modules for the Geo 3D Modeling & Animation 
	package.

COPYRIGHT NOTICE: 
   
	Copyright © 1998-2001 Carbon Graphics Llc, ALL RIGHTS RESERVED

\*===========================================================================*/



#ifndef __GEO_VERSION_H__
#define __GEO_VERSION_H__


#include "geoCore.h"


///////////////////////////////////////////////////////////////////////////////
// Constants for the GEO_LIB_LEVEL_VERSION 
///////////////////////////////////////////////////////////////////////////////

/** Signifies a pre-alpha version of the software */
const unsigned char		GEO_DEV_RELEASE			= 10;

/** Signifies an alpha version of the software */
const unsigned char		GEO_ALPHA_RELEASE		= 11;

/** Signifies an beta version of the software */
const unsigned char		GEO_BETA_RELEASE		= 12;

/** Signifies a late beta version of the software - potential release candidate, depending on user feedback */
const unsigned char		GEO_RELEASE_CANDIDATE	= 13;

/** Signifies an full version of the software */
const unsigned char		GEO_FULL_RELEASE		= 14;


///////////////////////////////////////////////////////////////////////////////
// Constants to identify the Geo version 
///////////////////////////////////////////////////////////////////////////////

/** this constant specifies the Geo Major release number */
#define					GEO_LIB_MAJOR_VERSION		1

/** this constant specifies the Geo Minor release number */
#define					GEO_LIB_MINOR_VERSION		2

/** This constant defines the level of type of release - ie alpha,beta */
#define					GEO_LIB_LEVEL_VERSION		GEO_FULL_RELEASE

/** This constant defines the number of releases made at a particular level */
#define					GEO_LIB_RELEASE_VERSION		2

#define					GEO_VERSION					((GEO_LIB_MAJOR_VERSION*1000)+(GEO_LIB_MINOR_VERSION*100)+(GEO_LIB_LEVEL_VERSION*10)+(GEO_LIB_RELEASE_VERSION))

// returns the GEO_VERSION value of the running Geo application. Users can use
// this to control code calls in the plugin.
extern GEO_DB_API int	GetGeoLibraryVersion(void);



#endif // __GEO_VERSION_H__
