/**********************************************************************
  ga_randomsearch.h
 **********************************************************************

  ga_randomsearch - Random search algorithm for comparison and search.
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

  Synopsis:	Random search algorithm for comparison and search.

 **********************************************************************/

#ifndef GA_RANDOMSEARCH_H_INCLUDED
#define GA_RANDOMSEARCH_H_INCLUDED

/*
 * Includes.
 */
#include "gaul.h"

/*
 * Prototypes.
 */
FUNCPROTO int ga_random_search(population *pop, entity *initial, const int max_iterations);

#endif	/* GA_RANDOMSEARCH_H_INCLUDED */

