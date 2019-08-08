/**********************************************************************
  ga_simplex.c
 **********************************************************************

  ga_simplex - A simplex search algorithm for comparison and local search.
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

  Synopsis:     A simplex search algorithm for comparison and local search.

                Note that, this algorithm requires that chromosomes
                may be reversibly mapped to arrays of double-precision
                floating-point array chromsomes.  If this is not
                possible then, hmmm, tough luck.

                You might want to think carefully about your convergence
                criteria.

  References:   Press, Flannery, Teukolsky, and Vetterling,
                "Numerical Recipes in C:  The Art of Scientific Computing"
                Cambridge University Press, 2nd edition (1992) pp. 408-412.

                Nelder, J.A., and Mead, R. Computer Journal, 7:308-313 (1965)

                Yarbro, L.A., and Deming, S.N. Analytica Chim. Acta,
                73:391-398 (1974)

  To do:        Make alpha, beta and gamma parameters.

 **********************************************************************/

#include "gaul/ga_simplex.h"

/**********************************************************************
  ga_population_set_simplex_parameters()
  synopsis:     Sets the simplex-search parameters for a population.
  parameters:   population *pop         Population to set parameters of.
                const int               Number of dimensions for double array (Needn't match dimensions of chromosome.)
                const double            Initial step size.
                const GAto_double       Map chromosomal data to array of doubles.
                const GAfrom_double     Map array of doubles to chromosomal data.
  return:       none
  last updated: 29 Mar 2004
 **********************************************************************/

void ga_population_set_simplex_parameters( population           *pop,
                                           const int            dimensions,
                                           const double         step,
                                           const GAto_double    to_double,
                                           const GAfrom_double  from_double)
{

    if ( !pop ) die("Null pointer to population structure passed.");
/*
    if ( !to_double ) die("Null pointer to GAto_double callback passed.");
    if ( !from_double ) die("Null pointer to GAfrom_double callback passed.");
*/

    plog( LOG_VERBOSE, "Population's simplex-search parameters set" );

    if (pop->simplex_params == NULL)
        pop->simplex_params = (ga_simplex_t *)s_malloc(sizeof(ga_simplex_t));

    pop->simplex_params->to_double = to_double;
    pop->simplex_params->from_double = from_double;
    pop->simplex_params->dimensions = dimensions;

    pop->simplex_params->step = step;   /* range: >0, 1=unit step randomisation, higher OK. */

    pop->simplex_params->alpha = 1.50;  /* range: 0=no extrap, 1=unit step extrap, higher OK. */
    pop->simplex_params->beta = 0.75;   /* range: 0=no contraction, 1=full contraction. */
    pop->simplex_params->gamma = 0.25;  /* range: 0=no contraction, 1=full contraction. */

    return;
}


/**********************************************************************
  ga_simplex()
  synopsis:     Performs optimisation on the passed entity by using a
                simplistic simplex-search.  The fitness evaluations
                are performed using the standard and evaluation
                callback mechanism.
                The passed entity will have its data overwritten.  The
                remainder of the population will be let untouched.
                Note that it is safe to pass a NULL initial structure,
                in which case a random starting structure will be
                generated, however the final solution will not be
                available to the caller in any obvious way.
  parameters:
  return:
  last updated: 18 Feb 2005
 **********************************************************************/

int ga_simplex( population              *pop,
                entity                  *initial,
                const int               max_iterations )
{
    int         iteration=0;            /* Current iteration number. */
    int         i, j;                   /* Index into putative solution array. */
    entity      **putative;             /* Current working solutions. */
    entity      *new1, *new2;           /* New putative solutions. */
    entity      *tmpentity;             /* Used to swap working solutions. */
    double      *tmpdoubleptr;          /* Used to swap working solutions. */
    int         num_points;             /* Number of search points. */
    double      **putative_d, *putative_d_buffer;/* Storage for double arrays. */
    double      *average;               /* Vector average of solutions. */
    double      *new1_d, *new2_d;       /* New putative solutions. */
    int         first=0, last;          /* Indices into solution arrays. */
    boolean     done=FALSE;             /* Whether the shuffle sort is complete. */
    boolean     did_replace;            /* Whether worst solution was replaced. */
    boolean     restart_needed;         /* Whether the search needs restarting. */

/*
 * Checks.
 */
    if (!pop) die("NULL pointer to population structure passed.");
    if (!pop->evaluate) die("Population's evaluation callback is undefined.");
    if (!pop->simplex_params) die("ga_population_set_simplex_params(), or similar, must be used prior to ga_simplex().");
    if (!pop->simplex_params->to_double) die("Population's genome to double callback is undefined.");
    if (!pop->simplex_params->from_double) die("Population's genome from double callback is undefined.");

/*
 * Prepare working entities and double arrays.
 * The space for the average and new arrays are allocated simultaneously.
 */
    num_points = pop->simplex_params->dimensions+1;
    putative = (entity **)s_malloc(sizeof(entity *)*num_points);
    putative_d = (double **)s_malloc(sizeof(double *)*num_points);
    putative_d_buffer = (double *)s_malloc(sizeof(double)*pop->simplex_params->dimensions*num_points*3);

    putative_d[0] = putative_d_buffer;
    average = &(putative_d_buffer[num_points*pop->simplex_params->dimensions]);
    new1_d = &(putative_d_buffer[(num_points+1)*pop->simplex_params->dimensions]);
    new2_d = &(putative_d_buffer[(num_points+2)*pop->simplex_params->dimensions]);

    for (i=1; i<num_points; i++)
    {
        putative[i] = ga_get_free_entity(pop);    /* The 'working' solutions. */
        putative_d[i] = &(putative_d_buffer[i*pop->simplex_params->dimensions]);
    }

    new1 = ga_get_free_entity(pop);
    new2 = ga_get_free_entity(pop);

    /* Do we need to generate a random starting solution? */
    if (!initial)
    {
        plog(LOG_VERBOSE, "Will perform simplex search with random starting solution.");

        putative[0] = ga_get_free_entity(pop);
        ga_entity_seed(pop, putative[0]);
        initial = ga_get_free_entity(pop);
    }
    else
    {
        plog(LOG_VERBOSE, "Will perform simplex search with specified starting solution.");

        putative[0] = ga_get_free_entity(pop);
        ga_entity_copy(pop, putative[0], initial);
    }

/*
 * Generate sample points.
 * Ensure that these initial solutions are scored.
 *
 * NOTE: Only perturb each solution by one dimension, by an
 * amount specified by the step parameter; it might be better to perturb
 * all dimensions and/or by a randomized amount.
 */
#pragma omp parallel \
    if (GAUL_DETERMINISTIC_OPENMP==0) \
    shared(pop,num_points,putative_d,putative) private(i,j)
    {
#pragma omp single \
    nowait
        pop->simplex_params->to_double(pop, putative[0], putative_d[0]);
        pop->evaluate(pop, putative[0]);

#pragma omp for \
    schedule(static) nowait
        for (i=1; i<num_points; i++)
        {
            for (j=0; j<pop->simplex_params->dimensions; j++)
            {
                putative_d[i][j] = putative_d[0][j] + random_double_range(-pop->simplex_params->step,pop->simplex_params->step);
                if (putative_d[i][j] > pop->allele_max_double) putative_d[i][j] = pop->allele_max_double;
                if (putative_d[i][j] < pop->allele_min_double) putative_d[i][j] = pop->allele_min_double;
            }

            pop->simplex_params->from_double(pop, putative[i], putative_d[i]);
            pop->evaluate(pop, putative[i]);
        }
    }   /* End of parallel block. */

/*
 * Sort the initial solutions by fitness.
 * We use a bi-directional bubble sort algorithm (which is
 * called shuffle sort, apparently).
 */
    last = pop->simplex_params->dimensions-1;
    while (done == FALSE && first < last)
    {
        for (j = last ; j > first ; j--)
        {
            if ( putative[j]->fitness > putative[j-1]->fitness )
            {   /* Swap! */
                tmpentity = putative[j];
                putative[j] = putative[j-1];
                putative[j-1] = tmpentity;
                tmpdoubleptr = putative_d[j];
                putative_d[j] = putative_d[j-1];
                putative_d[j-1] = tmpdoubleptr;
            }
        }
        first++;    /* The first one is definitely correct now. */

        done = TRUE;

        for (j = first ; j < last ; j++)
        {
            if ( putative[j]->fitness < putative[j+1]->fitness )
            {   /* Swap! */
                tmpentity = putative[j];
                putative[j] = putative[j+1];
                putative[j+1] = tmpentity;
                tmpdoubleptr = putative_d[j];
                putative_d[j] = putative_d[j+1];
                putative_d[j+1] = tmpdoubleptr;
                done = FALSE;
            }
        }
        last--;     /* The last one is definitely correct now. */
    }

    plog( LOG_VERBOSE,
          "Prior to the first iteration, the current solution has fitness score of %f",
          putative[0]->fitness );

/*
 * Do all the iterations:
 *
 * Stop when (a) max_iterations reached, or
 *           (b) "pop->iteration_hook" returns FALSE.
 */
    while ( (pop->iteration_hook?pop->iteration_hook(iteration, putative[0]):TRUE) &&
            iteration<max_iterations )
    {
        iteration++;

/*
 * Compute the vector average of all solutions except the least fit.
 * Exploration will proceed along the vector from the least fit point
 * to that vector average.
 */
        for (j = 0; j < pop->simplex_params->dimensions; j++)
        {
            average[j] = 0.0;

            for (i = 0; i < num_points-1; i++)
            {
                average[j] += putative_d[i][j];
            }

            average[j] /= num_points-1;
        }

/*
 * Check for convergence and restart if needed.
 * Reduce step, alpha, beta and gamma each time this happens.
 */
        restart_needed = TRUE;
        plog(LOG_DEBUG, "average = ");
        for (j = 0; j < pop->simplex_params->dimensions; j++)
        {
            if ( average[j]-TINY > putative_d[pop->simplex_params->dimensions][j] ||
                 average[j]+TINY < putative_d[pop->simplex_params->dimensions][j] )
                restart_needed = FALSE;

            plog(LOG_DEBUG, "%d %f ", j, average[j]/pop->simplex_params->dimensions);
        }


        if (restart_needed != FALSE)
        {
            plog(LOG_DEBUG, "restarting search.");
            pop->simplex_params->step *= 0.50;
            pop->simplex_params->alpha *= 0.75;
            pop->simplex_params->beta *= 0.75;
            pop->simplex_params->gamma *= 0.75;

            for (i=1; i<num_points; i++)
            {
                for (j=0; j<pop->simplex_params->dimensions; j++)
                {
                    putative_d[i][j] = putative_d[0][j] + random_double_range(-pop->simplex_params->step,pop->simplex_params->step);
                    if (putative_d[i][j] > pop->allele_max_double) putative_d[i][j] = pop->allele_max_double;
                    if (putative_d[i][j] < pop->allele_min_double) putative_d[i][j] = pop->allele_min_double;
                }

                pop->simplex_params->from_double(pop, putative[i], putative_d[i]);
                pop->evaluate(pop, putative[i]);
            }
        }

/*
 * Simplex reflection - Extrapolate by a factor alpha away from worst point.
 */
        for (j = 0; j < pop->simplex_params->dimensions; j++)
        {
            new1_d[j] = (1.0 + pop->simplex_params->alpha) * average[j] -
                    pop->simplex_params->alpha * putative_d[num_points-1][j];
        }

/*
 * Evaluate the function at this reflected point.
 */
        pop->simplex_params->from_double(pop, new1, new1_d);
        pop->evaluate(pop, new1);

        if (new1->fitness > putative[0]->fitness)
        {
/*
 * The new solution is fitter than the previously fittest solution, so attempt an
 * additional extrapolation by a factor alpha.
 */
            plog(LOG_DEBUG, "new1 (%f) is fitter than p0 ( %f )", new1->fitness, putative[0]->fitness);

            for (j = 0; j < pop->simplex_params->dimensions; j++)
                new2_d[j] = (1.0 + pop->simplex_params->alpha) * new1_d[j] -
                        pop->simplex_params->alpha * putative_d[num_points-1][j];

            pop->simplex_params->from_double(pop, new2, new2_d);
            pop->evaluate(pop, new2);

            if (new2->fitness > putative[0]->fitness)
            {
/*
 * This additional extrapolation succeeded, so replace the least fit solution
 * by inserting new solution in correct position.
 */
                plog(LOG_DEBUG, "new2 (%f) is fitter than p0 ( %f )", new2->fitness, putative[0]->fitness);

                tmpentity = putative[pop->simplex_params->dimensions];
                tmpdoubleptr = putative_d[pop->simplex_params->dimensions];

                for (j = pop->simplex_params->dimensions; j > 0; j--)
                {
                    putative[j]=putative[j-1];
                    putative_d[j]=putative_d[j-1];
                }

                putative[0] = new2;
                putative_d[0] = new2_d;

                new2 = tmpentity;
                new2_d = tmpdoubleptr;
            }
            else
            {
/*
 * This additional extrapolation failed, so use the original
 * reflected solution.
 */
                tmpentity = putative[pop->simplex_params->dimensions];
                tmpdoubleptr = putative_d[pop->simplex_params->dimensions];

                for (j = pop->simplex_params->dimensions; j > 0; j--)
                {
                    putative[j]=putative[j-1];
                    putative_d[j]=putative_d[j-1];
                }

                putative[0] = new1;
                putative_d[0] = new1_d;

                new1 = tmpentity;
                new1_d = tmpdoubleptr;
            }
        }
        else if (new1->fitness < putative[pop->simplex_params->dimensions-1]->fitness)
        {
/*
 * The reflected point is worse than the second-least fit.
 */
            plog(LOG_DEBUG, "new1 (%f) is less fit than p(n-1) ( %f )", new1->fitness, putative[pop->simplex_params->dimensions-1]->fitness);

            did_replace = FALSE;

            if (new1->fitness > putative[pop->simplex_params->dimensions]->fitness)
            {
/*
 * It is better than the least fit, so use it to replace the
 * least fit.
 */
                plog(LOG_DEBUG, "but fitter than p(n) ( %f )", putative[pop->simplex_params->dimensions]->fitness);
                did_replace = TRUE;

                tmpentity = putative[pop->simplex_params->dimensions];
                tmpdoubleptr = putative_d[pop->simplex_params->dimensions];

                putative[pop->simplex_params->dimensions] = new1;
                putative_d[pop->simplex_params->dimensions] = new1_d;

                new1 = tmpentity;
                new1_d = tmpdoubleptr;
            }
/*
 * Perform a contraction of the simplex along one dimension, away from worst point.
 */
            for (j = 0; j < pop->simplex_params->dimensions; j++)
                new1_d[j] = (1.0 - pop->simplex_params->beta) * average[j] +
                        pop->simplex_params->beta * putative_d[num_points-1][j];

            pop->simplex_params->from_double(pop, new1, new1_d);
            pop->evaluate(pop, new1);

            if (new1->fitness > putative[pop->simplex_params->dimensions]->fitness)
            {
/*
 * The contraction gave an improvement, so accept it by
 * inserting the new solution at the correct position.
 */
                did_replace = TRUE;

                plog(LOG_DEBUG, "contracted new1 (%f) is fitter than p(n) ( %f )", new1->fitness, putative[pop->simplex_params->dimensions]->fitness);

                i = 0;
                while (putative[i]->fitness > new1->fitness) i++;

                tmpentity = putative[pop->simplex_params->dimensions];
                tmpdoubleptr = putative_d[pop->simplex_params->dimensions];

                for (j = pop->simplex_params->dimensions; j > i; j--)
                {
                    putative[j]=putative[j-1];
                    putative_d[j]=putative_d[j-1];
                }

                putative[i] = new1;
                putative_d[i] = new1_d;

                new1 = tmpentity;
                new1_d = tmpdoubleptr;
            }

            if (did_replace == FALSE)
            {
/*
 * The new solution is worse than the previous worse.  So, contract
 * toward the average point.
 */
                plog(LOG_DEBUG, "new1 (%f) is worse than all.", new1->fitness);

                for (i = 1; i < num_points; i++)
                {
                    for (j = 0; j < pop->simplex_params->dimensions; j++)
                        putative_d[i][j] = average[j] +
                                pop->simplex_params->gamma * (putative_d[i][j] - average[j]);

                    pop->simplex_params->from_double(pop, putative[i], putative_d[i]);
                    pop->evaluate(pop, putative[i]);
                }

/*
 * Alternative is to contact toward the most fit point.
        for (i = 1; i < num_points; i++)
          {
          for (j = 0; j < pop->simplex_params->dimensions; j++)
            putative_d[i][j] = putative_d[0][j] +
                               pop->simplex_params->gamma * (putative_d[i][j] - putative_d[0][j]);

          pop->simplex_params->from_double(pop, putative[i], putative_d[i]);
          pop->evaluate(pop, putative[i]);
          }
*/

            }

        }
        else
        {
/*
 * The reflection gave a solution which was better than the worst two
 * solutions, but worse than the best solution.
 * Replace the old worst solution by inserting the new solution at the
 * correct position.
 */
            plog(LOG_DEBUG, "new1 (%f) is fitter than worst 2", new1->fitness);
            for (j=0; j < pop->simplex_params->dimensions; j++)
                plog(LOG_DEBUG, "%d fitness = %f", j, putative[j]->fitness);


            i = 0;
            while (putative[i]->fitness > new1->fitness) i++;

            plog(LOG_DEBUG, "new1 inserted at position %d\n", i);

            tmpentity = putative[pop->simplex_params->dimensions];
            tmpdoubleptr = putative_d[pop->simplex_params->dimensions];

            for (j = pop->simplex_params->dimensions; j > i; j--)
            {
                putative[j]=putative[j-1];
                putative_d[j]=putative_d[j-1];
            }

            putative[i] = new1;
            putative_d[i] = new1_d;

            new1 = tmpentity;
            new1_d = tmpdoubleptr;
        }

/*
 * Use the iteration callback.
 */
        plog( LOG_VERBOSE,
              "After iteration %d, the current solution has fitness score of %f",
              iteration,
              putative[0]->fitness );

    }   /* Iteration loop. */

/*
 * Store best solution.
 */
    ga_entity_copy(pop, initial, putative[0]);

/*
 * Cleanup.
 */
    ga_entity_dereference(pop, new1);
    ga_entity_dereference(pop, new2);

    for (i=0; i<num_points; i++)
    {
        ga_entity_dereference(pop, putative[i]);
    }

    s_free(putative);
    s_free(putative_d);
    s_free(putative_d_buffer);

    return iteration;
}


/**********************************************************************
  ga_simplex_double()
  synopsis:     Performs optimisation on the passed entity by using a
                simplistic simplex-search.  The fitness evaluations
                are performed using the standard and evaluation
                callback mechanism.
                The passed entity will have its data overwritten.  The
                remainder of the population will be let untouched.
                Note that it is safe to pass a NULL initial structure,
                in which case a random starting structure will be
                generated, however the final solution will not be
                available to the caller in any obvious way.

                Only double chromosomes may be used in this optimised
                version of the algorithm.
  parameters:
  return:
  last updated: 13 Apr 2005
 **********************************************************************/

int ga_simplex_double(  population              *pop,
                        entity                  *initial,
                        const int               max_iterations )
{
    int         iteration=0;            /* Current iteration number. */
    int         i, j;                   /* Index into putative solution array. */
    entity      **putative;             /* Current working solutions. */
    entity      *new1, *new2;           /* New putative solutions. */
    entity      *tmpentity;             /* Used to swap working solutions. */
    int         num_points;             /* Number of search points. */
    double      *average;               /* Vector average of solutions. */
    int         first=0, last;          /* Indices into solution arrays. */
    boolean     done=FALSE;             /* Whether the shuffle sort is complete. */
    boolean     did_replace;            /* Whether worst solution was replaced. */
    boolean     restart_needed;         /* Whether the search needs restarting. */

/*
 * Checks.
 */
    if (!pop) die("NULL pointer to population structure passed.");
    if (!pop->evaluate) die("Population's evaluation callback is undefined.");
    if (!pop->simplex_params) die("ga_population_set_simplex_params(), or similar, must be used prior to ga_simplex().");

/*
 * Prepare working entities and double arrays.
 * The space for the average and new arrays are allocated simultaneously.
 */
    num_points = pop->len_chromosomes+1;
    putative = (entity **)s_malloc(sizeof(entity *)*num_points);
    average = (double *)s_malloc(sizeof(double)*pop->len_chromosomes);

    for (i=1; i<num_points; i++)
    {
        putative[i] = ga_get_free_entity(pop);    /* The 'working' solutions. */
    }

    new1 = ga_get_free_entity(pop);
    new2 = ga_get_free_entity(pop);

    /* Do we need to generate a random starting solution? */
    if (!initial)
    {
        plog(LOG_VERBOSE, "Will perform simplex search with random starting solution.");

        putative[0] = ga_get_free_entity(pop);
        ga_entity_seed(pop, putative[0]);
        initial = ga_get_free_entity(pop);
    }
    else
    {
        plog(LOG_VERBOSE, "Will perform simplex search with specified starting solution.");

        putative[0] = ga_get_free_entity(pop);
        ga_entity_copy(pop, putative[0], initial);
    }

/*
 * Generate sample points.
 * Ensure that these initial solutions are scored.
 *
 * NOTE: Only perturb each solution by one dimension, by an
 * amount specified by the step parameter; it might be better to perturb
 * all dimensions and/or by a randomized amount.
 */
#pragma omp parallel \
    if (GAUL_DETERMINISTIC_OPENMP==0) \
    shared(pop,num_points,putative) private(i,j)
    {
#pragma omp single \
    nowait
        pop->evaluate(pop, putative[0]);

#pragma omp for \
    schedule(static) nowait
        for (i=1; i<num_points; i++)
        {
            for (j=0; j<pop->len_chromosomes; j++)
            {
                ((double *)putative[i]->chromosome[0])[j] = ((double *)putative[0]->chromosome[0])[j] + random_double_range(-pop->simplex_params->step,pop->simplex_params->step);
                if (((double *)putative[i]->chromosome[0])[j] > pop->allele_max_double) ((double *)putative[i]->chromosome[0])[j] = pop->allele_max_double;
                if (((double *)putative[i]->chromosome[0])[j] < pop->allele_min_double) ((double *)putative[i]->chromosome[0])[j] = pop->allele_min_double;
            }

            pop->evaluate(pop, putative[i]);
        }
    }   /* End of parallel block. */

/*
 * Sort the initial solutions by fitness.
 * We use a bi-directional bubble sort algorithm (which is
 * called shuffle sort, apparently).
 */
    last = pop->len_chromosomes-1;
    while (done == FALSE && first < last)
    {
        for (j = last ; j > first ; j--)
        {
            if ( putative[j]->fitness > putative[j-1]->fitness )
            {   /* Swap! */
                tmpentity = putative[j];
                putative[j] = putative[j-1];
                putative[j-1] = tmpentity;
            }
        }
        first++;    /* The first one is definitely correct now. */

        done = TRUE;

        for (j = first ; j < last ; j++)
        {
            if ( putative[j]->fitness < putative[j+1]->fitness )
            {   /* Swap! */
                tmpentity = putative[j];
                putative[j] = putative[j+1];
                putative[j+1] = tmpentity;
                done = FALSE;
            }
        }
        last--;     /* The last one is definitely correct now. */
    }

    plog( LOG_VERBOSE,
          "Prior to the first iteration, the current solution has fitness score of %f",
          putative[0]->fitness );

/*
 * Do all the iterations:
 *
 * Stop when (a) max_iterations reached, or
 *           (b) "pop->iteration_hook" returns FALSE.
 */
    while ( (pop->iteration_hook?pop->iteration_hook(iteration, putative[0]):TRUE) &&
            iteration<max_iterations )
    {
        iteration++;

/*
 * Compute the vector average of all solutions except the least fit.
 * Exploration will proceed along the vector from the least fit point
 * to that vector average.
 */
        for (j = 0; j < pop->len_chromosomes; j++)
        {
            average[j] = 0.0;

            for (i = 0; i < num_points-1; i++)
                average[j] += ((double *)putative[i]->chromosome[0])[j];

            average[j] /= num_points-1;
        }

/*
 * Check for convergence and restart if needed.
 * Reduce step, alpha, beta and gamma each time this happens.
 */
        restart_needed = TRUE;
        plog(LOG_DEBUG, "average = ");

        for (j = 0; j < pop->len_chromosomes; j++)
        {
            if ( average[j]-TINY > ((double *)putative[pop->len_chromosomes]->chromosome[0])[j] ||
                 average[j]+TINY < ((double *)putative[pop->len_chromosomes]->chromosome[0])[j] )
                restart_needed = FALSE;

            plog(LOG_DEBUG, "%f ", average[j]/pop->len_chromosomes);
        }
        plog(LOG_DEBUG, "");

        if (restart_needed != FALSE)
        {
            plog(LOG_DEBUG, "restarting search.");
            pop->simplex_params->step *= 0.50;
            pop->simplex_params->alpha *= 0.75;
            pop->simplex_params->beta *= 0.75;
            pop->simplex_params->gamma *= 0.75;

            for (i=1; i<num_points; i++)
            {
                for (j=0; j<pop->len_chromosomes; j++)
                {
                    ((double *)putative[i]->chromosome[0])[j] = ((double *)putative[0]->chromosome[0])[j] + random_double_range(-pop->simplex_params->step,pop->simplex_params->step);
                    if (((double *)putative[i]->chromosome[0])[j] > pop->allele_max_double) ((double *)putative[i]->chromosome[0])[j] = pop->allele_max_double;
                    if (((double *)putative[i]->chromosome[0])[j] < pop->allele_min_double) ((double *)putative[i]->chromosome[0])[j] = pop->allele_min_double;
                }

                pop->evaluate(pop, putative[i]);
            }
        }

/*
 * Simplex reflection - Extrapolate by a factor alpha away from worst point.
 */
        for (j = 0; j < pop->len_chromosomes; j++)
            ((double *)new1->chromosome[0])[j]
                = (1.0 + pop->simplex_params->alpha) * average[j] -
                pop->simplex_params->alpha * ((double *)putative[num_points-1]->chromosome[0])[j];

/*
 * Evaluate the function at this reflected point.
 */
        pop->evaluate(pop, new1);

        if (new1->fitness > putative[0]->fitness)
        {
/*
 * The new solution is fitter than the previously fittest solution, so attempt an
 * additional extrapolation by a factor alpha.
 */
            plog(LOG_DEBUG, "new1 (%f) is fitter than p0 ( %f )", new1->fitness, putative[0]->fitness);

            for (j = 0; j < pop->len_chromosomes; j++)
                ((double *)new2->chromosome[0])[j]
                    = (1.0 + pop->simplex_params->alpha) * ((double *)new1->chromosome[0])[j] -
                    pop->simplex_params->alpha * ((double *)putative[num_points-1]->chromosome[0])[j];

            pop->evaluate(pop, new2);

            if (new2->fitness > putative[0]->fitness)
            {
/*
 * This additional extrapolation succeeded, so replace the least fit solution
 * by inserting new solution in correct position.
 */
                plog(LOG_DEBUG, "new2 (%f) is fitter than p0 ( %f )", new2->fitness, putative[0]->fitness);

                tmpentity = putative[pop->len_chromosomes];

                for (j = pop->len_chromosomes; j > 0; j--)
                    putative[j]=putative[j-1];

                putative[0] = new2;

                new2 = tmpentity;
            }
            else
            {
/*
 * This additional extrapolation failed, so use the original
 * reflected solution.
 */
                tmpentity = putative[pop->len_chromosomes];

                for (j = pop->len_chromosomes; j > 0; j--)
                {
                    putative[j]=putative[j-1];
                }

                putative[0] = new1;

                new1 = tmpentity;
            }
        }
        else if (new1->fitness < putative[pop->len_chromosomes-1]->fitness)
        {
/*
 * The reflected point is worse than the second-least fit.
 */
            plog(LOG_DEBUG, "new1 (%f) is less fit than p(n-1) ( %f )", new1->fitness, putative[pop->len_chromosomes-1]->fitness);

            did_replace = FALSE;

            if (new1->fitness > putative[pop->len_chromosomes]->fitness)
            {
/*
 * It is better than the least fit, so use it to replace the
 * least fit.
 */
                plog(LOG_DEBUG, "but fitter than p(n) ( %f )", putative[pop->len_chromosomes]->fitness);

                did_replace = TRUE;

                tmpentity = putative[pop->len_chromosomes];

                putative[pop->len_chromosomes] = new1;

                new1 = tmpentity;
            }
/*
 * Perform a contraction of the simplex along one dimension, away from worst point.
 */
            for (j = 0; j < pop->len_chromosomes; j++)
                ((double *)new1->chromosome[0])[j]
                    = (1.0 - pop->simplex_params->beta) * average[j] +
                    pop->simplex_params->beta * ((double *)putative[num_points-1]->chromosome[0])[j];

            pop->evaluate(pop, new1);

            if (new1->fitness > putative[pop->len_chromosomes]->fitness)
            {
/*
 * The contraction gave an improvement, so accept it by
 * inserting the new solution at the correct position.
 */
                did_replace = TRUE;

                plog(LOG_DEBUG, "contracted new1 (%f) is fitter than p(n) ( %f )", new1->fitness, putative[pop->len_chromosomes]->fitness);

                i = 0;
                while (putative[i]->fitness > new1->fitness) i++;

                tmpentity = putative[pop->len_chromosomes];

                for (j = pop->len_chromosomes; j > i; j--)
                    putative[j]=putative[j-1];

                putative[i] = new1;

                new1 = tmpentity;
            }

            if (did_replace == FALSE)
            {
/*
 * The new solution is worse than the previous worse.  So, contract
 * toward the average point.
 */
                plog(LOG_DEBUG, "new1 (%f) is worse than all.", new1->fitness);

                for (i = 1; i < num_points; i++)
                {
                    for (j = 0; j < pop->len_chromosomes; j++)
                        ((double *)putative[i]->chromosome[0])[j]
                            = average[j] +
                            pop->simplex_params->gamma
                            * (((double *)putative[i]->chromosome[0])[j] - average[j]);

                    pop->evaluate(pop, putative[i]);
                }

/*
 * Alternative is to contact toward the most fit point.
        for (i = 1; i < num_points; i++)
          {
          for (j = 0; j < pop->len_chromosomes; j++)
            ((double *)putative[i]->chromosome[0])[j]
                = ((double *)putative[0]->chromosome[0])[j] +
                  pop->simplex_params->gamma
                    * (((double *)putative[i]->chromosome[0])[j] - ((double *)putative[0]->chromosome[0])[j]);

          pop->evaluate(pop, putative[i]);
          }
*/

            }

        }
        else
        {
/*
 * The reflection gave a solution which was better than the worst two
 * solutions, but worse than the best solution.
 * Replace the old worst solution by inserting the new solution at the
 * correct position.
 */
      plog(LOG_DEBUG, "new1 (%f) is fitter than worst 2\n", new1->fitness);
      for (j=0; j < pop->len_chromosomes; j++)
        plog(LOG_DEBUG, "%d fitness = %f\n", j, putative[j]->fitness);


            i = 0;
            while (putative[i]->fitness > new1->fitness) i++;

            plog(LOG_DEBUG, "new1 inserted at position %d\n", i);

            tmpentity = putative[pop->len_chromosomes];

            for (j = pop->len_chromosomes; j > i; j--)
                putative[j]=putative[j-1];

            putative[i] = new1;

            new1 = tmpentity;
        }

/*
 * Use the iteration callback.
 */
        plog( LOG_VERBOSE,
              "After iteration %d, the current solution has fitness score of %f",
              iteration,
              putative[0]->fitness );

    }   /* Iteration loop. */

/*
 * Store best solution.
 */
    ga_entity_copy(pop, initial, putative[0]);

/*
 * Cleanup.
 */
    ga_entity_dereference(pop, new1);
    ga_entity_dereference(pop, new2);

    for (i=0; i<num_points; i++)
    {
        ga_entity_dereference(pop, putative[i]);
    }

    s_free(putative);

    return iteration;
}

