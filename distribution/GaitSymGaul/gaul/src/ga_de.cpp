/**********************************************************************
  ga_de.c
 **********************************************************************

  ga_de - Differential Evolution.
  Copyright ©2005, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:     Differential Evolution.

                The DE algorithm was originally conceived by Rainer
                Storn and Ken Price.  The GAUL implementation is
                based in part on their "de36.c" reference source code.
                See http://www.icsi.berkeley.edu/~storn/code.html

                You may notice that this code includes equivalents of
                all of the original DE strategies along with a
                selection of additional strateties.

 **********************************************************************/

#include "gaul/ga_de.h"

/**********************************************************************
  ga_population_set_differentialevolution_parameters()
  synopsis:     Sets the differential evolution parameters for a
                population.
  parameters:	population *pop		Population to set parameters of.
                const GAcompare		Callback to compare two entities.
  return:	none
  last updated: 12 Apr 2005
 **********************************************************************/

void ga_population_set_differentialevolution_parameters( population *pop,
                                                         const ga_de_strategy_type strategy,
                                                         const ga_de_crossover_type crossover,
                                                         const int num_perturbed,
                                                         const double weighting_min,
                                                         const double weighting_max,
                                                         const double crossover_factor )
  {

  if ( !pop ) die("Null pointer to population structure passed.");

  plog( LOG_VERBOSE, "Population's differential evolution parameters set" );

  if (pop->de_params == NULL)
    pop->de_params = (ga_de_t *)s_malloc(sizeof(ga_de_t));

  pop->de_params->strategy = strategy;
  pop->de_params->crossover_method = crossover;
  pop->de_params->num_perturbed = num_perturbed;
  pop->de_params->weighting_min = weighting_min;
  pop->de_params->weighting_max = weighting_max;
  pop->de_params->crossover_factor = crossover_factor;

  return;
  }


/*
 * Pick an number of random entities by moving their index to the
 * beginning of the permutation array.
 * This method is a lot more efficient than the original algorithm's
 * approach - especially for small population sizes.
 */

static void _gaul_pick_random_entities(int *permutation, int num, int size, int avoid)
  {
  int		j;		/* Loop variable over picked numbers. */
  int		pos, tmp;	/* Current indices. */

  for (j=0; j<num; j++)
    {
    do
      {
      pos = j+random_int(size-j);
      } while (permutation[pos] == avoid);

    tmp = permutation[j];
    permutation[j] = permutation[pos];
    permutation[pos] = tmp;
    }

  return;
  }


/**********************************************************************
  ga_differentialevolution()
  synopsis:	Performs differential evolution.
  parameters:
  return:
  last updated:	12 Apr 2005
 **********************************************************************/

int ga_differentialevolution(	population		*pop,
                                const int		max_generations )
  {
  int		generation=0;		/* Current generation number. */
  int		i;			/* Loop variable over entities. */
  int		best;			/* Index of best entity. */
  int		*permutation;		/* Permutation array for random selections. */
  entity	*tmpentity;		/* New entity. */
  int		L, n;			/* Allele indices. */
  double	weighting_factor;	/* Weighting multiplier. */

/* Checks. */
  if (!pop)
    die("NULL pointer to population structure passed.");
  if (!pop->de_params)
    die("ga_population_set_differentialevolution_params(), or similar, must be used prior to ga_differentialevolution().");

  if (!pop->evaluate) die("Population's evaluation callback is undefined.");
  if (!pop->rank) die("Population's ranking callback is undefined.");
  if (pop->stable_size < 6) die("Population's stable size is too small.  (Must be at least 6)");
  if ( pop->de_params->crossover_factor < 0.0 ||
       pop->de_params->crossover_factor > 1.0 )
    die("Invalid crossover_factor.");

  plog(LOG_VERBOSE, "The differential evolution has begun!");

  pop->generation = 0;

/*
 * Score the initial population members.
 */
  if (pop->size < pop->stable_size)
    gaul_population_fill(pop, pop->stable_size - pop->size);

  if (pop->entity_iarray[0]->fitness == GA_MIN_FITNESS)
    pop->evaluate(pop, pop->entity_iarray[0]);

#pragma omp parallel for \
   shared(pop) private(i) \
   schedule(static)
  for (i=0; i<pop->size; i++)
    {
    if (pop->entity_iarray[i]->fitness == GA_MIN_FITNESS)
      pop->evaluate(pop, pop->entity_iarray[i]);
    }

/*
 * Prepare arrays to store permutations.
 */
  permutation = (int *)s_malloc(sizeof(int)*pop->size);
  for (i=0; i<pop->size; i++)
    permutation[i]=i;

/*
 * Do all the generations:
 *
 * Stop when (a) max_generations reached, or
 *           (b) "pop->generation_hook" returns FALSE.
 */
  while ( (pop->generation_hook?pop->generation_hook(generation, pop):TRUE) &&
           generation<max_generations )
    {
    generation++;
    pop->generation = generation;
    pop->orig_size = pop->size;

    plog(LOG_VERBOSE,
              "Population size is %d at start of generation %d",
              pop->orig_size, generation );

/*
 * Determine weighting factor.
 */
    if (pop->de_params->weighting_min == pop->de_params->weighting_max)
      {
      weighting_factor = pop->de_params->weighting_min;
      }
    else
      {
      weighting_factor = random_double_range(pop->de_params->weighting_min, pop->de_params->weighting_max);
      }

/*
 * Find best solution.
 */
    best = 0;

    if (pop->rank == ga_rank_fitness)
      {
      for (i=1; i<pop->size; i++)
        {
        if (pop->entity_iarray[i]->fitness > pop->entity_iarray[best]->fitness)
          best = i;
        }
      }
    else
      {
      for (i=1; i<pop->size; i++)
        {
        if ( pop->rank(pop, pop->entity_iarray[i],
             pop, pop->entity_iarray[best]) > 0 )
          best = i;
        }
      }

    plog(LOG_VERBOSE,
              "Best fitness is %f at start of generation %d",
              pop->entity_iarray[best]->fitness, generation );

#pragma omp parallel for \
   if (GAUL_DETERMINISTIC_OPENMP==0) \
   shared(pop) private(i) \
   schedule(static)
    for (i=0; i<pop->orig_size; i++)
      {

      tmpentity = ga_entity_clone(pop, pop->entity_iarray[i]);
      n = random_int(pop->len_chromosomes);

/*
 * Note that the following code may appear bloated due to excessive
 * extraction of branches from loops.
 * However, this yields much more efficient code (particularly for larger
 * chromosomes) on less-than-cutting-edge compilers.
 */

      if (pop->de_params->crossover_method == GA_DE_CROSSOVER_BINOMIAL)
        {
        if (pop->de_params->strategy == GA_DE_STRATEGY_BEST)
          {
          if (pop->de_params->num_perturbed == 1)
            { /* DE/best/1/bin */

            _gaul_pick_random_entities(permutation, 2, pop->orig_size, i);

            ((double *)tmpentity->chromosome[0])[n] =
              ((double *)pop->entity_iarray[best]->chromosome[0])[n]
              + weighting_factor*(((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
                                - ((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]);

            for (L=1; L<pop->len_chromosomes; L++)
              {
              if ( random_boolean() )
                ((double *)tmpentity->chromosome[0])[n] =
                  ((double *)pop->entity_iarray[best]->chromosome[0])[n]
                  + weighting_factor*(((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
                                    - ((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]);

              n = (n+1)%pop->len_chromosomes;
              }

            }
          else if (pop->de_params->num_perturbed == 2)
            { /* DE/best/2/bin */

            _gaul_pick_random_entities(permutation, 4, pop->orig_size, i);

            ((double *)tmpentity->chromosome[0])[n] =
              ((double *)pop->entity_iarray[best]->chromosome[0])[n]
              + weighting_factor*(((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
                                + ((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]
                                - ((double *)pop->entity_iarray[permutation[2]]->chromosome[0])[n]
                                - ((double *)pop->entity_iarray[permutation[3]]->chromosome[0])[n]);

            for (L=1; L<pop->len_chromosomes; L++)
              {
              if ( random_boolean() )
                ((double *)tmpentity->chromosome[0])[n] =
                  ((double *)pop->entity_iarray[best]->chromosome[0])[n]
                  + weighting_factor*(((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
                                    + ((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]
                                    - ((double *)pop->entity_iarray[permutation[2]]->chromosome[0])[n]
                                    - ((double *)pop->entity_iarray[permutation[3]]->chromosome[0])[n]);

              n = (n+1)%pop->len_chromosomes;
              }

            }
          else if (pop->de_params->num_perturbed == 3)
            { /* DE/best/3/exp */

            _gaul_pick_random_entities(permutation, 6, pop->orig_size, i);

            ((double *)tmpentity->chromosome[0])[n] =
              ((double *)pop->entity_iarray[best]->chromosome[0])[n]
              + weighting_factor*(((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
                                + ((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]
                                + ((double *)pop->entity_iarray[permutation[2]]->chromosome[0])[n]
                                - ((double *)pop->entity_iarray[permutation[3]]->chromosome[0])[n]
                                - ((double *)pop->entity_iarray[permutation[4]]->chromosome[0])[n]
                                - ((double *)pop->entity_iarray[permutation[5]]->chromosome[0])[n]);

            for (L=1; L<pop->len_chromosomes; L++)
              {
              if ( random_boolean() )
                ((double *)tmpentity->chromosome[0])[n] =
                  ((double *)pop->entity_iarray[best]->chromosome[0])[n]
                  + weighting_factor*(((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
                                    + ((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]
                                    + ((double *)pop->entity_iarray[permutation[2]]->chromosome[0])[n]
                                    - ((double *)pop->entity_iarray[permutation[3]]->chromosome[0])[n]
                                    - ((double *)pop->entity_iarray[permutation[4]]->chromosome[0])[n]
                                    - ((double *)pop->entity_iarray[permutation[5]]->chromosome[0])[n]);


              n = (n+1)%pop->len_chromosomes;
              }

            }
          else
            {
            die("Invalid differential evolution selection number.");
            }
          }
        else if (pop->de_params->strategy == GA_DE_STRATEGY_RAND)
          {
          if (pop->de_params->num_perturbed == 1)
            { /* DE/rand/1/bin */
            _gaul_pick_random_entities(permutation, 3, pop->orig_size, i);

            ((double *)tmpentity->chromosome[0])[n] =
              ((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
              + weighting_factor*(((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]
                                - ((double *)pop->entity_iarray[permutation[2]]->chromosome[0])[n]);

            for (L=1; L<pop->len_chromosomes; L++)
              {
              if ( random_boolean() )
                ((double *)tmpentity->chromosome[0])[n] =
                  ((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
                  + weighting_factor*(((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]
                                    - ((double *)pop->entity_iarray[permutation[2]]->chromosome[0])[n]);

              n = (n+1)%pop->len_chromosomes;
              }

            }
          else if (pop->de_params->num_perturbed == 2)
            { /* DE/rand/2/bin */
            _gaul_pick_random_entities(permutation, 5, pop->orig_size, i);

            ((double *)tmpentity->chromosome[0])[n] =
              ((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
              + weighting_factor*(((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]
                                + ((double *)pop->entity_iarray[permutation[2]]->chromosome[0])[n]
                                - ((double *)pop->entity_iarray[permutation[3]]->chromosome[0])[n]
                                - ((double *)pop->entity_iarray[permutation[4]]->chromosome[0])[n]);

            for (L=1; L<pop->len_chromosomes; L++)
              {
              if ( random_boolean() )
                ((double *)tmpentity->chromosome[0])[n] =
                  ((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
                  + weighting_factor*(((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]
                                    + ((double *)pop->entity_iarray[permutation[2]]->chromosome[0])[n]
                                    - ((double *)pop->entity_iarray[permutation[3]]->chromosome[0])[n]
                                    - ((double *)pop->entity_iarray[permutation[4]]->chromosome[0])[n]);

              n = (n+1)%pop->len_chromosomes;
              }

            }
          else if (pop->de_params->num_perturbed == 3)
            { /* DE/rand/3/bin */
            _gaul_pick_random_entities(permutation, 7, pop->orig_size, i);

            ((double *)tmpentity->chromosome[0])[n] =
              ((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
              + weighting_factor*(((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]
                                + ((double *)pop->entity_iarray[permutation[2]]->chromosome[0])[n]
                                + ((double *)pop->entity_iarray[permutation[3]]->chromosome[0])[n]
                                - ((double *)pop->entity_iarray[permutation[4]]->chromosome[0])[n]
                                - ((double *)pop->entity_iarray[permutation[5]]->chromosome[0])[n]
                                - ((double *)pop->entity_iarray[permutation[6]]->chromosome[0])[n]);

            for (L=1; L<pop->len_chromosomes; L++)
              {
              if ( random_boolean() )
                ((double *)tmpentity->chromosome[0])[n] =
                  ((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
                  + weighting_factor*(((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]
                                    + ((double *)pop->entity_iarray[permutation[2]]->chromosome[0])[n]
                                    + ((double *)pop->entity_iarray[permutation[3]]->chromosome[0])[n]
                                    - ((double *)pop->entity_iarray[permutation[4]]->chromosome[0])[n]
                                    - ((double *)pop->entity_iarray[permutation[5]]->chromosome[0])[n]
                                    - ((double *)pop->entity_iarray[permutation[6]]->chromosome[0])[n]);

              n = (n+1)%pop->len_chromosomes;
              }

            }
          else
            {
            die("Invalid differential evolution selection number.");
            }
          }
        else if (pop->de_params->strategy == GA_DE_STRATEGY_RANDTOBEST)
          {
          if (pop->de_params->num_perturbed == 1)
            { /* DE/rand-to-best/1/bin */
            _gaul_pick_random_entities(permutation, 2, pop->orig_size, i);

            ((double *)tmpentity->chromosome[0])[n] +=
              weighting_factor*(((double *)pop->entity_iarray[best]->chromosome[0])[n]
                              - ((double *)tmpentity->chromosome[0])[n]
                              + ((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
                              - ((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]);

            for (L=1; L<pop->len_chromosomes; L++)
              {
              if ( random_boolean() )
                ((double *)tmpentity->chromosome[0])[n] +=
                  weighting_factor*(((double *)pop->entity_iarray[best]->chromosome[0])[n]
                                  - ((double *)tmpentity->chromosome[0])[n]
                                  + ((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
                                  - ((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]);

              n = (n+1)%pop->len_chromosomes;
              }

            }
          else if (pop->de_params->num_perturbed == 2)
            { /* DE/rand-to-best/2/bin */
            _gaul_pick_random_entities(permutation, 4, pop->orig_size, i);

            ((double *)tmpentity->chromosome[0])[n] +=
              weighting_factor*(((double *)pop->entity_iarray[best]->chromosome[0])[n]
                              - ((double *)tmpentity->chromosome[0])[n]
                              + ((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
                              + ((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]
                              - ((double *)pop->entity_iarray[permutation[2]]->chromosome[0])[n]
                              - ((double *)pop->entity_iarray[permutation[3]]->chromosome[0])[n]);

            for (L=1; L<pop->len_chromosomes; L++)
              {
              if ( random_boolean() )
                ((double *)tmpentity->chromosome[0])[n] +=
                  weighting_factor*(((double *)pop->entity_iarray[best]->chromosome[0])[n]
                                  - ((double *)tmpentity->chromosome[0])[n]
                                  + ((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
                                  + ((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]
                                  - ((double *)pop->entity_iarray[permutation[2]]->chromosome[0])[n]
                                  - ((double *)pop->entity_iarray[permutation[3]]->chromosome[0])[n]);

              n = (n+1)%pop->len_chromosomes;
              }

            }
          else
            {
            die("Invalid differential evolution selection number.");
            }
          }
        else
          {
          die("Unknown differential evolution strategy.");
          }

        }
      else
        { /* pop->de_params->crossover_method == GA_DE_CROSSOVER_EXPONENTIAL */
        if (pop->de_params->strategy == GA_DE_STRATEGY_BEST)
          {
          if (pop->de_params->num_perturbed == 1)
            { /* DE/best/1/exp */

            _gaul_pick_random_entities(permutation, 2, pop->orig_size, i);

            L = 0;
            do
              {
              ((double *)tmpentity->chromosome[0])[n] =
                ((double *)pop->entity_iarray[best]->chromosome[0])[n]
                + weighting_factor*(((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
                                  - ((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]);

              n = (n+1)%pop->len_chromosomes;
              L++;
              } while(random_boolean_prob(pop->de_params->crossover_factor) && (L < pop->len_chromosomes));

            }
          else if (pop->de_params->num_perturbed == 2)
            { /* DE/best/2/exp */

            _gaul_pick_random_entities(permutation, 4, pop->orig_size, i);

            L = 0;
            do
              {
              ((double *)tmpentity->chromosome[0])[n] =
                ((double *)pop->entity_iarray[best]->chromosome[0])[n]
                + weighting_factor*(((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
                                  + ((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]
                                  - ((double *)pop->entity_iarray[permutation[2]]->chromosome[0])[n]
                                  - ((double *)pop->entity_iarray[permutation[3]]->chromosome[0])[n]);

              n = (n+1)%pop->len_chromosomes;
              L++;
              } while(random_boolean_prob(pop->de_params->crossover_factor) && (L < pop->len_chromosomes));
            }
          else if (pop->de_params->num_perturbed == 3)
            { /* DE/best/3/exp */

            _gaul_pick_random_entities(permutation, 6, pop->orig_size, i);

            L = 0;
            do
              {
              ((double *)tmpentity->chromosome[0])[n] =
                ((double *)pop->entity_iarray[best]->chromosome[0])[n]
                + weighting_factor*(((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
                                  + ((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]
                                  + ((double *)pop->entity_iarray[permutation[2]]->chromosome[0])[n]
                                  - ((double *)pop->entity_iarray[permutation[3]]->chromosome[0])[n]
                                  - ((double *)pop->entity_iarray[permutation[4]]->chromosome[0])[n]
                                  - ((double *)pop->entity_iarray[permutation[5]]->chromosome[0])[n]);

              n = (n+1)%pop->len_chromosomes;
              L++;
              } while(random_boolean_prob(pop->de_params->crossover_factor) && (L < pop->len_chromosomes));
            }
          else
            {
            die("Invalid differential evolution selection number.");
            }

          }
        else if (pop->de_params->strategy == GA_DE_STRATEGY_RAND)
          {
          if (pop->de_params->num_perturbed == 1)
            { /* DE/rand/1/exp (DE1) */

            _gaul_pick_random_entities(permutation, 3, pop->orig_size, i);

            L = 0;
            do
              {
              ((double *)tmpentity->chromosome[0])[n] =
                ((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
                + weighting_factor*(((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]
                                  - ((double *)pop->entity_iarray[permutation[2]]->chromosome[0])[n]);

              n = (n+1)%pop->len_chromosomes;
              L++;
              } while(random_boolean_prob(pop->de_params->crossover_factor) && (L < pop->len_chromosomes));

            }
          else if (pop->de_params->num_perturbed == 2)
            { /* DE/rand/2/exp */

            _gaul_pick_random_entities(permutation, 5, pop->orig_size, i);

            L = 0;
            do
              {
              ((double *)tmpentity->chromosome[0])[n] =
                ((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
                + weighting_factor*(((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]
                                  + ((double *)pop->entity_iarray[permutation[2]]->chromosome[0])[n]
                                  - ((double *)pop->entity_iarray[permutation[3]]->chromosome[0])[n]
                                  - ((double *)pop->entity_iarray[permutation[4]]->chromosome[0])[n]);

              n = (n+1)%pop->len_chromosomes;
              L++;
              } while(random_boolean_prob(pop->de_params->crossover_factor) && (L < pop->len_chromosomes));

            }
          else if (pop->de_params->num_perturbed == 3)
            { /* DE/rand/3/exp */

            _gaul_pick_random_entities(permutation, 7, pop->orig_size, i);

            L = 0;
            do
              {
              ((double *)tmpentity->chromosome[0])[n] =
                ((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
                + weighting_factor*(((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]
                                  + ((double *)pop->entity_iarray[permutation[2]]->chromosome[0])[n]
                                  + ((double *)pop->entity_iarray[permutation[3]]->chromosome[0])[n]
                                  - ((double *)pop->entity_iarray[permutation[4]]->chromosome[0])[n]
                                  - ((double *)pop->entity_iarray[permutation[5]]->chromosome[0])[n]
                                  - ((double *)pop->entity_iarray[permutation[6]]->chromosome[0])[n]);

              n = (n+1)%pop->len_chromosomes;
              L++;
              } while(random_boolean_prob(pop->de_params->crossover_factor) && (L < pop->len_chromosomes));

            }
          else
            {
            die("Invalid differential evolution selection number.");
            }

          }
        else if (pop->de_params->strategy == GA_DE_STRATEGY_RANDTOBEST)
          {
          if (pop->de_params->num_perturbed == 1)
            { /* DE/rand-to-best/1/exp */

            _gaul_pick_random_entities(permutation, 2, pop->orig_size, i);

            L = 0;
            do
              {
              ((double *)tmpentity->chromosome[0])[n] +=
                weighting_factor*(((double *)pop->entity_iarray[best]->chromosome[0])[n]
                                - ((double *)tmpentity->chromosome[0])[n]
                                + ((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
                                - ((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]);

              n = (n+1)%pop->len_chromosomes;
              L++;
              } while(random_boolean_prob(pop->de_params->crossover_factor) && (L < pop->len_chromosomes));

            }
          else if (pop->de_params->num_perturbed == 2)
            { /* DE/rand-to-best/2/exp */

            _gaul_pick_random_entities(permutation, 4, pop->orig_size, i);

            L = 0;
            do
              {
              ((double *)tmpentity->chromosome[0])[n] +=
                weighting_factor*(((double *)pop->entity_iarray[best]->chromosome[0])[n]
                                - ((double *)tmpentity->chromosome[0])[n]
                                + ((double *)pop->entity_iarray[permutation[0]]->chromosome[0])[n]
                                + ((double *)pop->entity_iarray[permutation[1]]->chromosome[0])[n]
                                - ((double *)pop->entity_iarray[permutation[2]]->chromosome[0])[n]
                                - ((double *)pop->entity_iarray[permutation[3]]->chromosome[0])[n]);

              n = (n+1)%pop->len_chromosomes;
              L++;
              } while(random_boolean_prob(pop->de_params->crossover_factor) && (L < pop->len_chromosomes));

            }
          else
            {
            die("Invalid differential evolution selection number.");
            }

          }
        else
          {
          die("Unknown differential evolution strategy.");
          }

        }

/*
 * Evaluate new solution and restore the former chromosome values
 * if this new solution is not an improvement.
 */
      if ( !pop->evaluate(pop, tmpentity) ||
         ( pop->rank == ga_rank_fitness && pop->entity_iarray[i]->fitness > tmpentity->fitness ) ||
         ( pop->rank != ga_rank_fitness && pop->rank(pop, tmpentity, pop, pop->entity_iarray[i]) < 0 ) )
        {
/*printf("DEBUG: old = %f > new = %f\n", pop->entity_iarray[i]->fitness, tmpentity->fitness);*/
        ga_entity_blank(pop, tmpentity);
        ga_entity_copy(pop, tmpentity, pop->entity_iarray[i]);
        }

      }

/*
 * Eliminate the original population members.
 */
    while (pop->orig_size > 0)
      {
      pop->orig_size--;
      ga_entity_dereference_by_rank(pop, pop->orig_size);
      }

/*
 * End of generation.
 */
    plog(LOG_VERBOSE,
          "After generation %d, population has fitness scores between %f and %f",
          generation,
          pop->entity_iarray[0]->fitness,
          pop->entity_iarray[pop->size-1]->fitness );

    }	/* Generation loop. */

/*
 * Ensure final ordering of population is correct.
 */
  sort_population(pop);

  return generation;
  }


