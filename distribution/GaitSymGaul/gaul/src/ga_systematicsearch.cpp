/**********************************************************************
  ga_systematicsearch.c
 **********************************************************************

  ga_systematicsearch - Systematic algorithm for comparison and search.
  Copyright ©2002-2005, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:     A systematic search algorithm for comparison and local
                search.

 **********************************************************************/

#include "gaul/ga_systematicsearch.h"

/**********************************************************************
  ga_population_set_search_parameters()
  synopsis:     Sets the systematic search parameters for a population.
  parameters:
  return:
  last updated: 08 Nov 2002
 **********************************************************************/

void ga_population_set_search_parameters( population              *pop,
                                        GAscan_chromosome	scan_chromosome)
  {

  if ( !pop ) die("Null pointer to population structure passed.");
  if ( !scan_chromosome ) die("Null pointer to GAscan_chromosome callback passed.");

  if (pop->search_params == NULL)
    pop->search_params = (ga_search_t *)s_malloc(sizeof(ga_search_t));

  pop->search_params->scan_chromosome = scan_chromosome;
  pop->search_params->chromosome_state = 0;
  pop->search_params->allele_state = 0;

  return;
  }


/**********************************************************************
  ga_search()
  synopsis:	Performs a systematic search procedure.
                The passed entity will have its data overwritten.  The
                remainder of the population will be let untouched.
                Note that it is safe to pass a NULL initial structure,
                however the final solution will not be
                available to the caller in any obvious way.
  parameters:
  return:
  last updated:	18 Feb 2005
 **********************************************************************/

int ga_search(	population		*pop,
                entity			*best)
  {
  int		iteration=0;		/* Current iteration number. */
  entity	*putative;		/* Current solution. */
  entity	*tmp;			/* Used to swap entities. */
  int		enumeration=0;		/* Enumeration index. */
  boolean	finished=FALSE;		/* Whether search is complete. */

/* Checks. */
  if (!pop) die("NULL pointer to population structure passed.");
  if (!pop->evaluate) die("Population's evaluation callback is undefined.");
  if (!pop->search_params) die("ga_population_set_search_params(), or similar, must be used prior to ga_search().");
  if (!pop->search_params->scan_chromosome) die("Population's chromosome scan callback is undefined.");

/* Prepare working entity. */
  putative = ga_get_free_entity(pop);

  plog(LOG_VERBOSE, "Will perform systematic search.");

/* Do we need to allocate starting solution? */
  if (!best)
    {
    best = ga_get_free_entity(pop);
    ga_entity_seed(pop, best);
    }

/*
 * Ensure that initial solution is scored.
 */
  if (best->fitness==GA_MIN_FITNESS) pop->evaluate(pop, best);

/*
 * Prepare internal data for the enumeration algorithm.
 */
  pop->search_params->chromosome_state = 0;
  pop->search_params->allele_state = 0;

/*
 * Do all the iterations:
 *
 * Stop when (a) All enumerations performed or
 *           (b) "pop->iteration_hook" returns FALSE.
 */
  while ( (pop->iteration_hook?pop->iteration_hook(iteration, best):TRUE) &&
           finished == FALSE )
    {
    iteration++;

/*
 * Generate and score a new solution.
 */
    ga_entity_blank(pop, putative);
    finished = pop->search_params->scan_chromosome(pop, putative, enumeration);
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


