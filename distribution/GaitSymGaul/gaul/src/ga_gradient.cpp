/**********************************************************************
  ga_gradient.c
 **********************************************************************

  ga_gradient - Gradient methods for comparison and local search.
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

  Synopsis:     Gradient methods for comparison and local search.

                Routines for local search and optimisation using
                non-evolutionary methods.  These methods are all
                first-order, that is, they require first derivatives.

                Note that, these algorithms require that chromosomes
                may be reversibly mapped to arrays of double-precision
                floating-point array chromsomes.  If this is not
                possible then, hmmm, tough luck.

                You might want to think carefully about your convergence
                criteria.

  References:

 **********************************************************************/

#include "gaul/ga_gradient.h"

/**********************************************************************
  ga_population_set_gradient_parameters()
  synopsis:     Sets the gradient-search parameters for a population.
  parameters:	population *pop		Population to set parameters of.
                const GAto_double	Map chromosomal data to array of doubles.
                const GAfrom_double	Map array of doubles to chromosomal data.
                const int		Number of dimensions for double array (Needn't match dimensions of chromosome.)
  return:	none
  last updated: 19 Nov 2002
 **********************************************************************/

void ga_population_set_gradient_parameters( population		*pop,
                                        const GAto_double	to_double,
                                        const GAfrom_double	from_double,
                                        const GAgradient	gradient,
                                        const int		dimensions,
                                        const double		step_size)
  {

  if ( !pop ) die("Null pointer to population structure passed.");
/*
  if ( !to_double ) die("Null pointer to GAto_double callback passed.");
  if ( !from_double ) die("Null pointer to GAfrom_double callback passed.");
*/

  plog( LOG_VERBOSE, "Population's gradient methods parameters set" );

  if (pop->gradient_params == NULL)
    pop->gradient_params = (ga_gradient_t *)s_malloc(sizeof(ga_gradient_t));

  pop->gradient_params->to_double = to_double;
  pop->gradient_params->from_double = from_double;
  pop->gradient_params->gradient = gradient;
  pop->gradient_params->step_size = step_size;
  pop->gradient_params->dimensions = dimensions;
  pop->gradient_params->alpha = 0.5;	/* Step-size scale-down factor. */
  pop->gradient_params->beta = 1.2;	/* Step-size scale-up factor. */

  return;
  }


/**********************************************************************
  ga_steepestascent()
  synopsis:	Performs optimisation on the passed entity by using a
                steepest ascents method (i.e. steepest descent, except
                maximising the fitness function).
                The passed entity will have its data overwritten.  The
                remainder of the population will be let untouched.
                Note that it is safe to pass a NULL initial structure,
                in which case a random starting structure wil be
                generated, however the final solution will not be
                available to the caller in any obvious way.
  parameters:
  return:
  last updated:	18 Feb 2005
 **********************************************************************/

int ga_steepestascent(	population	*pop,
                        entity		*current,
                        const int	max_iterations )
  {
  int		iteration=0;		/* Current iteration number. */
  int		i;			/* Index into arrays. */
  double	*current_d;		/* Current iteration solution array. */
  double	*current_g;		/* Current iteration gradient array. */
  entity	*putative;		/* New solution. */
  double	*putative_d;		/* New solution array. */
  entity	*tmpentity;		/* Used to swap working solutions. */
  double	*tmpdoubleptr;		/* Used to swap working solutions. */
  double	*buffer;		/* Storage for double arrays. */
  double	step_size;		/* Current step size. */
  double	grms;			/* Current RMS gradient. */
  boolean	force_terminate=FALSE;	/* Force optimisation to terminate. */

/*
 * Checks.
 */
  if (!pop) die("NULL pointer to population structure passed.");
  if (!pop->evaluate) die("Population's evaluation callback is undefined.");
  if (!pop->gradient_params) die("ga_population_set_gradient_params(), or similar, must be used prior to ga_gradient().");
  if (!pop->gradient_params->to_double) die("Population's genome to double callback is undefined.");
  if (!pop->gradient_params->from_double) die("Population's genome from double callback is undefined.");
  if (!pop->gradient_params->gradient) die("Population's first derivatives callback is undefined.");

/*
 * Prepare working entity and double arrays.
 */
  buffer = (double *)s_malloc(sizeof(double)*pop->gradient_params->dimensions*3);

  current_d = buffer;
  putative_d = &(buffer[pop->gradient_params->dimensions]);
  current_g = &(buffer[pop->gradient_params->dimensions*2]);

  putative = ga_get_free_entity(pop);

/* Do we need to generate a random starting solution? */
  if (current==NULL)
    {
    plog(LOG_VERBOSE, "Will perform gradient search with random starting solution.");

    current = ga_get_free_entity(pop);
    ga_entity_seed(pop, current);
    }
  else
    {
    plog(LOG_VERBOSE, "Will perform gradient search with specified starting solution.");
    }

/*
 * Get initial fitness and derivatives.
 */
  pop->evaluate(pop, current);
  pop->gradient_params->to_double(pop, current, current_d);

  grms = pop->gradient_params->gradient(pop, current, current_d, current_g);

  plog( LOG_VERBOSE,
        "Prior to the first iteration, the current solution has fitness score of %f and a RMS gradient of %f",
         current->fitness, grms );

/*
 * Adjust step size based on gradient.
 * This scales the step size according to the initial gradient so that the
 * calculation doesn't blow-up completely.
 */
/*  step_size=(pop->gradient_params->dimensions*pop->gradient_params->step_size)/grms;*/
  step_size=pop->gradient_params->step_size;

/*
 * Do all the iterations:
 *
 * Stop when (a) max_iterations reached, or
 *           (b) "pop->iteration_hook" returns FALSE.
 * The iteration hook could evaluate the RMS gradient, or the maximum component
 * of the gradient, or any other termination criteria that may be desirable.
 */
  while ( force_terminate==FALSE &&
          (pop->iteration_hook?pop->iteration_hook(iteration, current):TRUE) &&
          iteration<max_iterations )
    {
    iteration++;

    for( i=0; i<pop->gradient_params->dimensions; i++ )
      putative_d[i]=current_d[i]+step_size*current_g[i];

    pop->gradient_params->from_double(pop, putative, putative_d);
    pop->evaluate(pop, putative);

#if GA_DEBUG>2
    printf("DEBUG: current_d = %f %f %f %f\n", current_d[0], current_d[1], current_d[2], current_d[3]);
    printf("DEBUG: current_g = %f %f %f %f grms = %f\n", current_g[0], current_g[1], current_g[2], current_g[3], grms);
    printf("DEBUG: putative_d = %f %f %f %f fitness = %f\n", putative_d[0], putative_d[1], putative_d[2], putative_d[3], putative->fitness);
#endif

    if ( current->fitness > putative->fitness )
      {	/* New solution is worse. */

      do
        {
        step_size *= pop->gradient_params->alpha;
        /*printf("DEBUG: step_size = %e\n", step_size);*/

        for( i=0; i<pop->gradient_params->dimensions; i++ )
          putative_d[i]=current_d[i]+step_size*current_g[i];

        pop->gradient_params->from_double(pop, putative, putative_d);
        pop->evaluate(pop, putative);

#if GA_DEBUG>2
        printf("DEBUG: putative_d = %f %f %f %f fitness = %f\n", putative_d[0], putative_d[1], putative_d[2], putative_d[3], putative->fitness);
#endif
        } while( current->fitness > putative->fitness && step_size > ApproxZero);

      if (step_size <= ApproxZero && grms <= ApproxZero) force_terminate=TRUE;
      }
    else
      {	/* New solution is an improvement. */
      step_size *= pop->gradient_params->beta;
#if GA_DEBUG>2
      printf("DEBUG: step_size = %e\n", step_size);
#endif
      }

/* Store improved solution. */
    tmpentity = current;
    current = putative;
    putative = tmpentity;

    tmpdoubleptr = current_d;
    current_d = putative_d;
    putative_d = tmpdoubleptr;

    grms = pop->gradient_params->gradient(pop, current, current_d, current_g);

/*
 * Use the iteration callback.
 */
    plog( LOG_VERBOSE,
          "After iteration %d, the current solution has fitness score of %f and RMS gradient of %f (step_size = %f)",
          iteration, current->fitness, grms, step_size );

    }	/* Iteration loop. */

/*
 * Cleanup.
 */
  ga_entity_dereference(pop, putative);

  s_free(buffer);

  return iteration;
  }


/**********************************************************************
  ga_steepestascent_double()
  synopsis:	Performs optimisation on the passed entity by using a
                steepest ascents method (i.e. steepest descent, except
                maximising the fitness function).
                The passed entity will have its data overwritten.  The
                remainder of the population will be let untouched.
                Note that it is safe to pass a NULL initial structure,
                in which case a random starting structure wil be
                generated, however the final solution will not be
                available to the caller in any obvious way.

                Only double chromosomes may be used in this optimised
                version of the algorithm.
  parameters:
  return:
  last updated:	14 Apr 2005
 **********************************************************************/

int ga_steepestascent_double(	population	*pop,
                        entity		*current,
                        const int	max_iterations )
  {
  int		iteration=0;		/* Current iteration number. */
  int		i;			/* Index into arrays. */
  double	*current_g;		/* Current iteration gradient array. */
  entity	*putative;		/* New solution. */
  entity	*tmpentity;		/* Used to swap working solutions. */
  double	step_size;		/* Current step size. */
  double	grms;			/* Current RMS gradient. */
  boolean	force_terminate=FALSE;	/* Force optimisation to terminate. */

/*
 * Checks.
 */
  if (!pop) die("NULL pointer to population structure passed.");
  if (!pop->evaluate) die("Population's evaluation callback is undefined.");
  if (!pop->gradient_params) die("ga_population_set_gradient_params(), or similar, must be used prior to ga_gradient().");
  if (!pop->gradient_params->gradient) die("Population's first derivatives callback is undefined.");

/*
 * Prepare working entity and gradient array.
 */
  current_g = (double *)s_malloc(sizeof(double)*pop->len_chromosomes);

  putative = ga_get_free_entity(pop);

/* Do we need to generate a random starting solution? */
  if (current==NULL)
    {
    plog(LOG_VERBOSE, "Will perform gradient search with random starting solution.");

    current = ga_get_free_entity(pop);
    ga_entity_seed(pop, current);
    }
  else
    {
    plog(LOG_VERBOSE, "Will perform gradient search with specified starting solution.");
    }

/*
 * Get initial fitness and derivatives.
 */
  pop->evaluate(pop, current);

  grms = pop->gradient_params->gradient(pop, current, (double *)current->chromosome[0], current_g);

  plog( LOG_VERBOSE,
        "Prior to the first iteration, the current solution has fitness score of %f and a RMS gradient of %f",
         current->fitness, grms );

/*
 * Adjust step size based on gradient.
 * This scales the step size according to the initial gradient so that the
 * calculation doesn't blow-up completely.
 */
/*  step_size=(pop->len_chromosomes*pop->gradient_params->step_size)/grms;*/
  step_size=pop->gradient_params->step_size;

/*
 * Do all the iterations:
 *
 * Stop when (a) max_iterations reached, or
 *           (b) "pop->iteration_hook" returns FALSE.
 * The iteration hook could evaluate the RMS gradient, or the maximum component
 * of the gradient, or any other termination criteria that may be desirable.
 */
  while ( force_terminate==FALSE &&
          (pop->iteration_hook?pop->iteration_hook(iteration, current):TRUE) &&
          iteration<max_iterations )
    {
    iteration++;

    for( i=0; i<pop->len_chromosomes; i++ )
      ((double *)putative->chromosome[0])[i]=((double *)current->chromosome[0])[i]+step_size*current_g[i];

    pop->evaluate(pop, putative);

#if GA_DEBUG>2
    printf("DEBUG: current_d = %f %f %f %f\n", current_d[0], current_d[1], current_d[2], current_d[3]);
    printf("DEBUG: current_g = %f %f %f %f grms = %f\n", current_g[0], current_g[1], current_g[2], current_g[3], grms);
    printf("DEBUG: putative_d = %f %f %f %f fitness = %f\n", putative_d[0], putative_d[1], putative_d[2], putative_d[3], putative->fitness);
#endif

    if ( current->fitness > putative->fitness )
      {	/* New solution is worse. */

      do
        {
        step_size *= pop->gradient_params->alpha;
        /*printf("DEBUG: step_size = %e\n", step_size);*/

        for( i=0; i<pop->len_chromosomes; i++ )
          ((double *)putative->chromosome[0])[i]=((double *)current->chromosome[0])[i]+step_size*current_g[i];

        pop->evaluate(pop, putative);
        } while( current->fitness > putative->fitness && step_size > ApproxZero);

      if (step_size <= ApproxZero && grms <= ApproxZero) force_terminate=TRUE;
      }
    else
      {	/* New solution is an improvement. */
      step_size *= pop->gradient_params->beta;
#if GA_DEBUG>2
      printf("DEBUG: step_size = %e\n", step_size);
#endif
      }

/* Store improved solution. */
    tmpentity = current;
    current = putative;
    putative = tmpentity;

    grms = pop->gradient_params->gradient(pop, current, (double *)current->chromosome[0], current_g);

/*
 * Use the iteration callback.
 */
    plog( LOG_VERBOSE,
          "After iteration %d, the current solution has fitness score of %f and RMS gradient of %f (step_size = %f)",
          iteration, current->fitness, grms, step_size );

    }	/* Iteration loop. */

/*
 * Cleanup.
 */
  ga_entity_dereference(pop, putative);

  return iteration;
  }

