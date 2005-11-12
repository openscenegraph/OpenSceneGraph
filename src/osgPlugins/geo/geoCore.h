/*===========================================================================*\

  NAME:			geoCore.h

  DESCRIPTION:	High level DLL interface macros for the database library

  AUTHOR:		Andy Bushnell

  ---------------------------------------------------------------------------

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


#ifndef _GEO_CORE_H_
#define _GEO_CORE_H_


// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the GEO_DB_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// GEO_DB_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

#ifdef WIN32
	#ifdef GEO_DB_EXPORTS
		#define GEO_DB_API __declspec( dllexport )
	#else
		#define GEO_DB_API __declspec( dllimport )
	#endif
#else	
	#define GEO_DB_API
#endif


#include "geoTypes.h"
#include "geoVersion.h"

#endif //_GEO_CORE_H_

