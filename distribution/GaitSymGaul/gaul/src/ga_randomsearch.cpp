/**********************************************************************
  ga_randomsearch.c
 **********************************************************************

  ga_randomsearch - Random search algorithm for comparison and search.
  Copyright ©2002-2004, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:     Randomsearch algorithm for comparison and local search.

  		I strongly recommend that you only use this function
		for benchmarking purposes!

 **********************************************************************/

#include "gaul/ga_randomsearch.h"

/**********************************************************************
  ga_random_search()
  synopsis:	Performs a random search procedure by repeattedly
  		calling the seed and evaluation functions.
		The passed entity will have its data overwritten.  The
		remainder of the population will be let untouched.
		Note that it is safe to pass a NULL initial structure,
		in which case a random starting structure wil be
		generated, however the final solution will not be
		available to the caller in any obvious way.
  parameters:
  return:
  last updated:	06 Nov 2002
 **********************************************************************/

int ga_random_search(	population		*pop,
			entity			*best,
			const int		max_iterations )
  {
  int		iteration=0;		/* Current iteration number. */
  entity	*putative;		/* Current solution. */
  entity	*tmp;			/* Used to swap entities. */

/* Checks. */
  if (!pop) die("NULL pointer to population structure passed.");
  if (pop->size < 1) die("Population is empty.");
  if (!pop->evaluate) die("Population's evaluation callback is undefined.");
  if (!pop->seed) die("Population's seed callback is undefined.");

/* Prepare working entity. */
  putative = ga_get_free_entity(pop);

/* Do we need to generate a random starting solution? */
  if (!best)
    {
    plog(LOG_VERBOSE, "Will perform random search with random starting solution.");

    best = ga_get_free_entity(pop);
    ga_entity_seed(pop, best);
    }
  else
    {   
    plog(LOG_VERBOSE, "Will perform random search with specified starting solution.");
    }

/*
 * Ensure that initial solution is scored.
 */
  if (best->fitness==GA_MIN_FITNESS) pop->evaluate(pop, best);

  plog( LOG_VERBOSE,
        "Prior to the first iteration, the current solution has fitness score of %f",
        best->fitness );

/*
 * Do all the iterations:
 *
 * Stop when (a) max_iterations reached, or
 *           (b) "pop->iteration_hook" returns FALSE.
 */
  while ( (pop->iteration_hook?pop->iteration_hook(iteration, best):TRUE) &&
           iteration<max_iterations )
    {
    iteration++;

/*
 * Generate and score a new solution.
 */
    ga_entity_blank(pop, putative);
    pop->seed(pop, putative);
    pop->evaluate(pop, putative);

/*
 * Decide whether this new solution should be selected or discarded based
 * on the relative fitnesses.
 */
  if ( putative->fitness > best->fitness )
    {
    tmp = best;
    best = putative;
    putative = tmp;
    }

/*
 * Use the iteration callback.
 */
    plog( LOG_VERBOSE,
          "After iteration %d, the current solution has fitness score of %f",
          iteration,
          best->fitness );

    }	/* Iteration loop. */

/*
 * Cleanup.
 */
  ga_entity_dereference(pop, putative);

  return iteration;
  }


