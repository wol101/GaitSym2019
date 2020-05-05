/**********************************************************************
  ga_tabu.h
 **********************************************************************

  ga_tabu - A tabu-search algorithm for comparison and local search.
  Copyright ©2002, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:	A tabu-search algorithm for comparison and local search.

 **********************************************************************/

#ifndef GA_TABU_H_INCLUDED
#define GA_TABU_H_INCLUDED

/*
 * Includes.
 */
#include "gaul.h"

/*
 * Prototypes.
 */
FUNCPROTO boolean ga_tabu_check_integer(  population      *pop,
                  entity          *putative,
                  entity          *tabu);
FUNCPROTO boolean ga_tabu_check_boolean(  population      *pop,
                  entity          *putative,
                  entity          *tabu);
FUNCPROTO boolean ga_tabu_check_char(  population      *pop,
                  entity          *putative,
                  entity          *tabu);
FUNCPROTO boolean ga_tabu_check_double(  population      *pop,
                  entity          *putative,
                  entity          *tabu);
FUNCPROTO boolean ga_tabu_check_bitstring(  population      *pop,
                  entity          *putative,
                  entity          *tabu);
FUNCPROTO void ga_population_set_tabu_parameters( population              *pop,
                      GAtabu_accept           tabu_accept,
                      const int               list_length,
                      const int               search_count);
FUNCPROTO int ga_tabu(    population              *pop,
		entity                  *initial,
	        const int               max_iterations );

#endif	/* GA_TABU_H_INCLUDED */

