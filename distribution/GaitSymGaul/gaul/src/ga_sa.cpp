/**********************************************************************
  ga_sa.c
 **********************************************************************

  ga_sa - A simulated annealling algorithm for comparison and search.
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

  Synopsis:     A simulated annealling algorithm for comparison and search.

 **********************************************************************/

#include "gaul/ga_sa.h"

/**********************************************************************
  ga_sa_boltzmann_acceptance()
  synopsis:     Simulated annealling acceptance criterion.
  parameters:
  return:
  last updated: 14 Oct 2002
 **********************************************************************/

boolean ga_sa_boltzmann_acceptance( population	*pop,
                                entity		*original,
                                entity		*putative )
  {

  return ( original->fitness < putative->fitness ||
           random_boolean_prob(exp((putative->fitness-original->fitness)
           /(GA_BOLTZMANN_FACTOR*pop->sa_params->temperature))) );
  }


/**********************************************************************
  ga_sa_linear_acceptance()
  synopsis:     Simulated annealling acceptance criterion.
  parameters:
  return:
  last updated: 14 Oct 2002
 **********************************************************************/

boolean ga_sa_linear_acceptance( population	*pop,
                                entity		*original,
                                entity		*putative )
  {

  return ( original->fitness < putative->fitness+pop->sa_params->temperature );
  }


/**********************************************************************
  ga_population_set_sa_temperature()
  synopsis:     Sets the simulated annealling temperature.
                Valid only for use during callbacks from
                ga_simulated_annealling().
  parameters:
  return:
  last updated: 11 Oct 2002
 **********************************************************************/

void ga_population_set_sa_temperature( population              *pop,
                                      const double            temp )
  {

  if ( !pop ) die("Null pointer to population structure passed.");
  if ( !pop->sa_params )
    die("ga_population_set_sa_parameters() must be called prior to ga_population_set_sa_temperature()");

  pop->sa_params->temperature = temp;

  return;
  }


/**********************************************************************
  ga_population_get_sa_temperature()
  synopsis:     Returns the current simulated annealling temperature.
  parameters:
  return:
  last updated: 11 Oct 2002
 **********************************************************************/

double ga_population_get_sa_temperature( population *pop )
  {

  if ( !pop ) die("Null pointer to population structure passed.");
  if ( !pop->sa_params )
    die("ga_population_set_sa_parameters() must be called prior to ga_population_get_sa_temperature()");

  return pop->sa_params->temperature;
  }


/**********************************************************************
  ga_population_set_sa_parameters()
  synopsis:     Sets the simulated annealling parameters for a
                population.
  parameters:
  return:
  last updated: 11 Oct 2002
 **********************************************************************/

void ga_population_set_sa_parameters( population              *pop,
                                      GAsa_accept             sa_accept,
                                      const double            initial_temp,
                                      const double            final_temp,
                                      const double            temp_step,
                                      const int               temp_freq )
  {

  if ( !pop ) die("Null pointer to population structure passed.");
  if ( !sa_accept ) die("Null pointer to GAsa_accept callback passed.");

  plog( LOG_VERBOSE,
        "Population's SA parameters: inital_temp = %f final_temp = %f temp_step = %f temp_freq = %d",
        initial_temp, final_temp, temp_step, temp_freq );

  if (pop->sa_params == NULL)
    pop->sa_params = (ga_sa_t *)s_malloc(sizeof(ga_sa_t));

  pop->sa_params->sa_accept = sa_accept;
  pop->sa_params->initial_temp = initial_temp;
  pop->sa_params->final_temp = final_temp;
  pop->sa_params->temp_step = temp_step;
  pop->sa_params->temp_freq = temp_freq;
  pop->sa_params->temperature = 0.0;	/* Current temperature. */

  return;
  }


/**********************************************************************
  ga_sa()
  synopsis:	Performs optimisation on the passed entity by using a
                simplistic simulated annealling protocol.  The local
                search and fitness evaluations are performed using the
                standard mutation and evaluation callback mechanisms,
                respectively.

                The passed entity will have its data overwritten.  The
                remainder of the population will be let untouched.  Note
                that it is safe to pass a NULL initial structure, in
                which case a random starting structure wil be generated,
                however the final solution will not be available to the
                caller in any obvious way.

                Custom cooling schemes may be introduced by using
                ga_population_set_sa_temperature() from within
                an iteration_hook callback.
  parameters:
  return:
  last updated:	18 Feb 2005
 **********************************************************************/

int ga_sa(	population		*pop,
                entity			*initial,
                const int		max_iterations )
  {
  int		iteration=0;		/* Current iteration number. */
  entity	*putative;		/* Current solution. */
  entity	*best;			/* Current solution. */
  entity	*tmp;			/* Used to swap working solutions. */

/* Checks. */
  if (!pop) die("NULL pointer to population structure passed.");
  if (!pop->evaluate) die("Population's evaluation callback is undefined.");
  if (!pop->mutate) die("Population's mutation callback is undefined.");
  if (!pop->sa_params) die("ga_population_set_sa_params(), or similar, must be used prior to ga_sa().");

/* Prepare working entities. */
  putative = ga_get_free_entity(pop);
  best = ga_get_free_entity(pop);

/* Do we need to generate a random starting solution? */
  if (!initial)
    {
    plog(LOG_VERBOSE, "Will perform simulated annealling with random starting solution.");

    initial = ga_get_free_entity(pop);
    ga_entity_seed(pop, best);
    }
  else
    {
    plog(LOG_VERBOSE, "Will perform simulated annealling with specified starting solution.");
    ga_entity_copy(pop, best, initial);
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
  pop->sa_params->temperature = pop->sa_params->initial_temp;

  while ( (pop->iteration_hook?pop->iteration_hook(iteration, best):TRUE) &&
           iteration<max_iterations )
    {
    iteration++;

    if (pop->sa_params->temp_freq == -1)
      {
      pop->sa_params->temperature = pop->sa_params->initial_temp
                                  + ((double)iteration/max_iterations)
                                  * (pop->sa_params->final_temp-pop->sa_params->initial_temp);
      }
    else
      {
      if (    pop->sa_params->temperature > pop->sa_params->final_temp
           && iteration%pop->sa_params->temp_freq == 0 )
        {
        pop->sa_params->temperature -= pop->sa_params->temp_step;
        }
      }
    plog(LOG_DEBUG, "%06d temperature = %f", pop->sa_params->temperature);

/*
 * Generate and score a new solution.
 */
  pop->mutate(pop, best, putative);
  pop->evaluate(pop, putative);

/*
 * Use the acceptance criterion to decide whether this new solution should
 * be selected or discarded.
 */
  if ( pop->sa_params->sa_accept(pop, best, putative) )
    {
    tmp = best;
    best = putative;
    putative = tmp;
    }

/*
 * Save the current best solution in the initial entity, if this
 * is now the best found so far.
 */
  if ( initial->fitness<best->fitness )
    {
    ga_entity_blank(pop, initial);
    ga_entity_copy(pop, initial, best);
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
  ga_entity_dereference(pop, best);
  ga_entity_dereference(pop, putative);

  return iteration;
  }


