/**********************************************************************
  ga_bitstring.h
 **********************************************************************

  ga_bitstring - GAUL's low-level bitstring routines.
  Copyright ©2001-2003, Stewart Adcock <stewart@linux-domain.com>
  All rights reserved.

  The latest version of this program should be available at:
  http://gaul.sourceforge.net/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.  Alternatively, if your project
  is incompatible with the GPL, I will probably agree to requests
  for permission to use the terms of any other license.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY WHATSOEVER.

  A full copy of the GNU General Public License should be in the file
  "COPYING" provided with this distribution; if not, see:
  http://www.gnu.org/

 **********************************************************************

  Synopsis:     Low-level bitstring handling functions.

 **********************************************************************/

#ifndef GA_BITSTRING_H_INCLUDED
#define GA_BITSTRING_H_INCLUDED

/*
 * Includes.
 */
#include "gaul.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

/*
 * Prototypes.
 */
FUNCPROTO byte *ga_bit_new( int length );
FUNCPROTO void ga_bit_free( byte *bstr );
FUNCPROTO void ga_bit_set( byte *bstr, int n );
FUNCPROTO void ga_bit_clear( byte *bstr, int n );
FUNCPROTO void ga_bit_invert( byte *bstr, int n );
FUNCPROTO boolean ga_bit_get( byte *bstr, int n );
FUNCPROTO void ga_bit_randomize( byte *bstr, int n );
FUNCPROTO void ga_bit_copy( byte *dest, byte *src, int ndest, int nsrc, int length );
FUNCPROTO size_t ga_bit_sizeof( int length );
FUNCPROTO byte *ga_bit_clone( byte *dest, byte *src, int length );

/* Integer conversion. */
FUNCPROTO unsigned int ga_bit_decode_binary_uint( byte *bstr, int n, int length );
FUNCPROTO void ga_bit_encode_binary_uint( byte *bstr, int n, int length, unsigned int value );
FUNCPROTO int ga_bit_decode_binary_int( byte *bstr, int n, int length );
FUNCPROTO void ga_bit_encode_binary_int( byte *bstr, int n, int length, int value );
FUNCPROTO int ga_bit_decode_gray_int( byte *bstr, int n, int length );
FUNCPROTO unsigned int ga_bit_decode_gray_uint( byte *bstr, int n, int length );
FUNCPROTO void ga_bit_encode_gray_uint( byte *bstr, int n, int length, unsigned int value );

/* Real conversion. */
FUNCPROTO double ga_bit_decode_binary_real( byte *bstr, int n, int mantissa, int exponent );
FUNCPROTO void ga_bit_encode_binary_real( byte *bstr, int n, int mantissa, int exponent, double value );
FUNCPROTO double ga_bit_decode_gray_real( byte *bstr, int n, int mantissa, int exponent );
FUNCPROTO void ga_bit_encode_grayy_real( byte *bstr, int n, int mantissa, int exponent, double value );

/* Test. */
FUNCPROTO boolean ga_bit_test( void );

#endif	/* GA_BITSTRING_H_INCLUDED */

