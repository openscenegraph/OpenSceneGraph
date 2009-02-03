
/****************************************************************************

	Basic data types used in LWO2 files

	Copyright (C) 2002-2003 Marco Jez

****************************************************************************/

#ifndef LWO2TYPES_
#define LWO2TYPES_

#include <string>

namespace lwo2 
{

// basic types

struct ID4
{
    ID4()
    {
        id[0] = 0;
        id[1] = 0;
        id[2] = 0;
        id[3] = 0;
    }
    
    char id[4];
};

typedef signed char			I1;
typedef signed short int	I2;
typedef signed int			I4;
typedef unsigned char		U1;
typedef unsigned short int	U2;
typedef unsigned int		U4;
typedef float				F4;
typedef std::string			S0;

// composite types

struct VX {
	U4 index;
};

struct COL12 {
	F4 red;
	F4 green;
	F4 blue;
};

struct VEC12 {
	F4 X;
	F4 Y;
	F4 Z;
};

struct FP4 {
	F4 fraction;
};

struct ANG4 {
	F4 radians;
};

struct FNAM0 {
	S0 name;
};

}

#endif
