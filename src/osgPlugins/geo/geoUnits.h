/*===========================================================================*\

NAME:			geoUnits.h

DESCRIPTION:	Constants to convert coordinate data to/from meters (Geo 
				default)

AUTHOR:			Andy Bushnell

	-------------------------------------------------------------------------

PROPRIETARY RIGHTS NOTICE:   
	
  This software contains proprietary information and trade secrets of Carbon
  Graphics LLC. No part or all of this software may be reproduced in any form, 
  without the written permission of Carbon Graphics LLC. 

  Exception:
  This Software file can be used by third-party software developers (without
  using the Geo SDK libraries) for any purpose OTHER THAN loading Geo format 
  files into an application or executable (such as, though not limited to, 
  geometry Modelers & animation systems) which is primarily intended to allow for
  the CREATION or MODIFICATION of geometric or animation data. 
  
  Specifically,using this software (either all or part thereof) to aid in the 
  creation of a Geo format loader for a run-time system, game engine, toolkit 
  IG (Image Generation) System or any software where the PRIMARY purpose is
  real-time image playback and interactivity and not Model Creation and/or
  modification is permitted.

COPYRIGHT NOTICE: 
   
  Copyright © 1998-2001 Carbon Graphics Llc, ALL RIGHTS RESERVED

\*===========================================================================*/



#ifndef _GEO_UNITS_H_
#define _GEO_UNITS_H_


const float KM_TO_METERS	= 		1000.0f;
const float CM_TO_METERS	= 		0.01f;
const float MM_TO_METERS	= 		0.001f;
const float NM_TO_METERS	= 		1852.0f;		
const float MILES_TO_METERS = 		1609.344f;	
const float YARDS_TO_METERS = 		0.9144f;
const float FEET_TO_METERS	= 		0.3048f;
const float INCHES_TO_METERS= 		0.0254f;

const float METERS_TO_KM	=		0.001f;
const float METERS_TO_CM	=		100.0f;
const float METERS_TO_MM	=		1000.0f;
const float METERS_TO_NM	=		0.0005399568035f;
const float METERS_TO_MILES	=		0.0006213711922f;
const float METERS_TO_YARDS	=		1.093613298f;
const float METERS_TO_FEET	=		3.280839895f;
const float METERS_TO_INCHES=		39.37007874f;

const float CM_TO_FEET		=	    0.03280839895f;
const float CM_TO_INCHES	=		0.3937007874f;
const float FEET_TO_YARDS	=		0.333333333f;
const float FEET_TO_CM		=	    30.48f;
const float FEET_TO_INCHES	=		12.0f;
const float INCHES_TO_FEET	=		0.083333333f;
const float INCHES_TO_CM	=		2.54f; 

const float MPH_TO_FPS		=       1.4667f;
const float MPH_TO_MPS		=       0.447f;



#endif //_GEO_UNITS_H_
