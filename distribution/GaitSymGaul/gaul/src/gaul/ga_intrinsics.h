/**********************************************************************
  ga_intrinsics.h
 **********************************************************************

  ga_intrinsics - Genetic algorithm routine intrinsics.
  Copyright ©2002-2003, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:     Wrappers around the routines for handling populations
		and performing GA operations.

 **********************************************************************/

#ifndef GA_INTRINSICS_H_INCLUDED
#define GA_INTRINSICS_H_INCLUDED

/*
 * Includes
 */
#include "gaul.h"

#if HAVE_SLANG == 1
#include <slang.h>
#endif

/*
 * Function prototypes.
 */

/*
boolean	ga_slang_seed(population *pop, entity *adam);
boolean	ga_slang_select_one(population *pop, entity **mother);
boolean	ga_slang_select_two(population *pop, entity **mother, entity **father);
entity	*ga_slang_adapt(population *pop, entity *child);
void	ga_slang_crossover(population *pop, entity *father, entity *mother, entity *daughter, entity *son);
void	ga_slang_mutate(population *pop, entity *father, entity *son);
void	ga_slang_replace(population *pop, entity *child);
*/
boolean	ga_intrinsic_sladd(void);

#endif	/* GA_INTRINSICS_H_INCLUDED */

