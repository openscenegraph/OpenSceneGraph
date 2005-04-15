/* dxfReader for OpenSceneGraph  Copyright (C) 2005 by GraphArchitecture ( grapharchitecture.com )
 * Programmed by Paul de Repentigny <pdr@grapharchitecture.com>
 * 
 * OpenSceneGraph is (C) 2004 Robert Osfield
 * 
 * This library is provided as-is, without support of any kind.
 *
 * Read DXF docs or OSG docs for any related questions.
 * 
 * You may contact the author if you have suggestions/corrections/enhancements.
 */

#ifndef DXF_ACI_COLOR
#define DXF_ACI_COLOR 1
// lookup table for autocad color index
struct aci {
	// some color positions
	enum {
		BLACK,
		RED,
		YELLOW,
		GREEN,
		CYAN,
		BLUE,
		MAGENTA,
		WHITE,
		USER_2,
		USER_3,
		BYLAYER = 256,
		MIN = 1,
		MAX = 255
	};
	static double table[256*3];
};
#endif
