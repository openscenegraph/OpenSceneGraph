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

#include <stdlib.h>
#include <stdio.h>

/* trpage_swap.h
	Byte swapping utility functions.
	*/

#include <trpage_swap.h>

/*
** func:	swap_two( in, out )
**
** desc:	byte-swaps a two-byte array.
*/
void trpg_swap_two ( const char *in, char *out )
{
  char tmp[2];

  tmp[0] = in[1] ;
  tmp[1] = in[0] ;

  memcpy(out,tmp,2);
}

/*
** func:	swap_four( in, out )
**
** desc:	byte-swaps a four-byte array.
*/
void trpg_swap_four ( const char *in, char *out )
{
  char tmp[4];

  tmp[0] = in[3] ;
  tmp[1] = in[2] ;
  tmp[2] = in[1] ;
  tmp[3] = in[0] ;

  memcpy(out,tmp,4);
}

/*
** func:	swap_eight( in, out )
**
** desc:	byte-swaps an eight-byte array.
*/
void trpg_swap_eight ( const char *in, char *out )
{
  char tmp[8];

  tmp[0] = in[7] ;
  tmp[1] = in[6] ;
  tmp[2] = in[5] ;
  tmp[3] = in[4] ;
  tmp[4] = in[3] ;
  tmp[5] = in[2] ;
  tmp[6] = in[1] ;
  tmp[7] = in[0] ;

  memcpy(out,tmp,8);
}

/*
** func:	swap_sixteen( in, out )
**
** desc:	byte-swaps an sixteen-byte array.
*/
void trpg_swap_sixteen ( const char *in, char *out )
{
  char tmp[16];

  tmp[0] = in[15] ;
  tmp[1] = in[14] ;
  tmp[2] = in[13] ;
  tmp[3] = in[12] ;
  tmp[4] = in[11] ;
  tmp[5] = in[10] ;
  tmp[6] = in[9] ;
  tmp[7] = in[8] ;
  tmp[8] = in[7] ;
  tmp[9] = in[6] ;
  tmp[10] = in[5] ;
  tmp[11] = in[4] ;
  tmp[12] = in[3] ;
  tmp[13] = in[2] ;
  tmp[14] = in[1] ;
  tmp[15] = in[0] ;

  memcpy(out,tmp,16);
}

/*
** func:	tx_byteswap_short( number )
**
** desc:	byte-swaps a short int.
*/
short trpg_byteswap_short( short number )
{
	short result;

	trpg_swap_two( (const char*) &number, (char*) &result );
	return result;
}

/*
** func:	tx_byteswap_int( number )
**
** desc:	byte-swaps an int.
*/
TX_CPPDECL int	trpg_byteswap_int( int number )
{
	int result;

	trpg_swap_four( (const char*) &number, (char*) &result );
	return result;
}

/*
** func:	tx_byteswap_long( number )
**
** desc:	byte-swaps a long int.
*/
long trpg_byteswap_long( long number )
{
	long result;

	trpg_swap_four( (const char*) &number, (char*) &result );
	return result;
}

/*
** func:	tx_byteswap_float( number )
**
** desc:	byte-swaps a float.
*/
void trpg_byteswap_float_to_4bytes( float number, char result[4] )
{
	trpg_swap_four( (const char*) &number, result );
}

/*
** func:	tx_byteswap_double_to_8bytes( number )
**
** desc:	byte-swaps a double.
*/
void trpg_byteswap_double_to_8bytes( double number, char result[8] )
{
	trpg_swap_eight( (const char*) &number, result );
}


/*
** func:	tx_byteswap_float( number )
**
** desc:	byte-swaps a float.
*/
float trpg_byteswap_4bytes_to_float( const char result[4] )
{
	float number;
	trpg_swap_four( result, (char*) &number );
	return number;
}

/*
** func:	tx_byteswap_double_to_8bytes( number )
**
** desc:	byte-swaps a double.
*/
double trpg_byteswap_8bytes_to_double( const char result[8] )
{
	double number;
	trpg_swap_eight( result, (char*) &number );
	return number;
}

trpgllong trpg_byteswap_llong ( trpgllong number )
{
	trpgllong result;

	trpg_swap_sixteen ( (char *) &number, (char *) &result);

	return result;
}

TX_CPPDECL trpgEndian trpg_cpu_byte_order(void)
{
	static char big_endian_100[2] = { 0, 100 };

	if ( (*((short*) big_endian_100)) == 100 )
		return BigEndian;
	else 
		return LittleEndian;
}

