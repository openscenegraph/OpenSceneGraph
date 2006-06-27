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

#ifndef trpage_swap_h_
#define trpage_swap_h_

/* trpage_swap.h
	Byte swapping utility functions.
  */

#include <trpage_sys.h>

#include <trpage_io.h>

// Byte swap and return a short
// {group:Byte Ordering Utilities}
short trpg_byteswap_short( short number );
// Byte swap and return an integer
// {group:Byte Ordering Utilities}
TX_CPPDECL int	trpg_byteswap_int( int number );
// Byte swap and return a long
// {group:Byte Ordering Utilities}
long trpg_byteswap_long( long number );
// Byte swap and return a 64 bit long
// {group:Byte Ordering Utilities}
trpgllong trpg_byteswap_llong ( trpgllong number );
// Byte swap a float value into 4 characters.  We do it this way to avoid floating point exceptions.
// {group:Byte Ordering Utilities}
void trpg_byteswap_float_to_4bytes( float number, char result[4] );
// Byte swap a double value into 8 characters.  We do it this way to avoid floating point exceptions.
// {group:Byte Ordering Utilities}
void trpg_byteswap_double_to_8bytes( double number, char result[8] );
// Swap 4 bytes into a float and return it
// {group:Byte Ordering Utilities}
float trpg_byteswap_4bytes_to_float( const char result[4] );
// Swap 8 bytes into a double and return it
// {group:Byte Ordering Utilities}
double trpg_byteswap_8bytes_to_double( const char result[8] );
// Determine the current CPU's byte ordering
// {group:Byte Ordering Utilities}
TX_CPPDECL trpgEndian trpg_cpu_byte_order(void);

// Swap two chars
// {group:Byte Ordering Utilities}
void trpg_swap_two ( const char *in, char *out );
// Swap 4 chars
// {group:Byte Ordering Utilities}
void trpg_swap_four ( const char *in, char *out );
// Swap 8 chars
// {group:Byte Ordering Utilities}
void trpg_swap_eight ( const char *in, char *out );
// Swap sixteen chars
// {group:Byte Ordering Utilities}
void trpg_swap_sixteen ( const char *in, char *out );

#endif
