/**********************************************************************
  ga_deterministiccrowding.c
 **********************************************************************

  ga_deterministiccrowding - Deterministic crowding.
  Copyright ©2003-2005, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:     Deterministic crowding.

 **********************************************************************/

#include "gaul/ga_deterministiccrowding.h"

/**********************************************************************
  ga_population_set_deterministiccrowding_parameters()
  synopsis:     Sets the deterministic crowding parameters for a
                population.
  parameters:	population *pop		Population to set parameters of.
                const GAcompare		Callback to compare two entities.
  return:	none
  last updated: 20 May 2003
 **********************************************************************/

void ga_population_set_deterministiccrowding_parameters( population		*pop,
                                                         const GAcompare	compare )
  {

  if ( !pop ) die("Null pointer to population structure passed.");
  if ( !compare ) die("Null pointer to GAcompare callback passed.");

  plog( LOG_VERBOSE, "Population's deterministic crowding parameters set" );

  if (pop->dc_params == NULL)
    pop->dc_params = (ga_dc_t *)s_malloc(sizeof(ga_dc_t));

  pop->dc_params->compare = compare;

  return;
  }


/**********************************************************************
  ga_deterministiccrowding()
  synopsis:	Performs optimisation of the given population by a
                method known as determinstic crowding.
                ga_genesis(), or equivalent, must be called prior to
                this function.
                This approach is useful when you desire a
                significant amount of diversity in the resulting
                population.
                This was designed as a niching algorithm rather than
                an optimisation algorithm.
                During a generation, children potentially replace
                their parents as soon as they are created, rather
                than replacing them at the end of the generation.
                This differs slightly from the canonical
                deterministic crowding algorithm.
  parameters:
  return:
  last updated:	18 Feb 2005
 **********************************************************************/

int ga_deterministiccrowding(	population		*pop,
                                const int		max_generations )
  {
  int		generation=0;		/* Current generation number. */
  int		*permutation, *ordered;	/* Arrays of entities. */
  entity	*mother, *father;	/* Current entities. */
  entity	*son, *daughter, *entity;	/* Current entities. */
  int		i;			/* Loop variable over entities. */
  double	dist1, dist2;		/* Genetic or phenomic distances. */
  int		rank;			/* Rank of entity in population. */

/* Checks. */
  if (!pop)
    die("NULL pointer to population structure passed.");
  if (!pop->dc_params)
    die("ga_population_set_deterministiccrowding_params(), or similar, must be used prior to ga_deterministiccrowding().");

  if (!pop->evaluate) die("Population's evaluation callback is undefined.");
  if (!pop->mutate) die("Population's mutation callback is undefined.");
  if (!pop->crossover) die("Population's crossover callback is undefined.");

  if (!pop->dc_params->compare) die("Population's comparison callback is undefined.");

  plog(LOG_VERBOSE, "The evolution by deterministic crowding has begun!");

  pop->generation = 0;

/*
 * Score the initial population members.
 */
  if (pop->size < pop->stable_size)
    gaul_population_fill(pop, pop->stable_size - pop->size);

  for (i=0; i<pop->size; i++)
    {
    if (pop->entity_iarray[i]->fitness == GA_MIN_FITNESS)
      pop->evaluate(pop, pop->entity_iarray[i]);
    }

  sort_population(pop);
  ga_genocide_by_fitness(pop, GA_MIN_FITNESS);


/*
 * Prepare arrays to store permutations.
 */
  permutation = (int *)s_malloc(sizeof(int)*pop->size);
  ordered = (int *)s_malloc(sizeof(int)*pop->size);
  for (i=0; i<pop->size;i++)
    ordered[i]=i;

  plog( LOG_VERBOSE,
        "Prior to the first generation, population has fitness scores between %f and %f",
        pop->entity_iarray[0]->fitness,
        pop->entity_iarray[pop->size-1]->fitness );

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

    plog(LOG_DEBUG,
              "Population size is %d at start of generation %d",
              pop->orig_size, generation );

    sort_population(pop);

    random_int_permutation(pop->orig_size, ordered, permutation);

    for ( i=0; i<pop->orig_size; i++ )
      {
      mother = pop->entity_iarray[i];
      father = pop->entity_iarray[permutation[i]];

/*
 * Crossover step.
 */
      plog(LOG_VERBOSE, "Crossover between %d (rank %d fitness %f) and %d (rank %d fitness %f)",
           ga_get_entity_id(pop, mother),
           ga_get_entity_rank(pop, mother), mother->fitness,
           ga_get_entity_id(pop, father),
           ga_get_entity_rank(pop, father), father->fitness);

      son = ga_get_free_entity(pop);
      daughter = ga_get_free_entity(pop);
      pop->crossover(pop, mother, father, daughter, son);

/*
 * Mutation step.
 */
      if (random_boolean_prob(pop->mutation_ratio))
        {
        plog(LOG_VERBOSE, "Mutation of %d (rank %d)",
             ga_get_entity_id(pop, daughter),
             ga_get_entity_rank(pop, daughter) );

        entity = ga_get_free_entity(pop);
        pop->mutate(pop, daughter, entity);
        ga_entity_dereference(pop, daughter);
        daughter = entity;
        }

      if (random_boolean_prob(pop->mutation_ratio))
        {
        plog(LOG_VERBOSE, "Mutation of %d (rank %d)",
             ga_get_entity_id(pop, son),
             ga_get_entity_rank(pop, son) );

        entity = ga_get_free_entity(pop);
        pop->mutate(pop, son, entity);
        ga_entity_dereference(pop, son);
        son = entity;
        }

/*
 * Apply environmental adaptations, score entities, sort entities, etc.
 * FIXME: Currently no adaptation.
 */
      pop->evaluate(pop, daughter);
      pop->evaluate(pop, son);

/*
 * Evaluate similarities.
 */
      dist1 = pop->dc_params->compare(pop, mother, daughter) + pop->dc_params->compare(pop, father, son);
      dist2 = pop->dc_params->compare(pop, mother, son) + pop->dc_params->compare(pop, father, daughter);

/*
 * Determine which entities will survive, and kill the others.
 */
      if (dist1 < dist2)
        {
        rank = ga_get_entity_rank(pop, daughter);
        if (daughter->fitness < mother->fitness)
          {
          entity = pop->entity_iarray[i];
          pop->entity_iarray[i] = pop->entity_iarray[rank];
          pop->entity_iarray[rank] = entity;
          }
        ga_entity_dereference_by_rank(pop, rank);

        rank = ga_get_entity_rank(pop, son);
        if (son->fitness < father->fitness)
          {
          entity = pop->entity_iarray[permutation[i]];
          pop->entity_iarray[permutation[i]] = pop->entity_iarray[rank];
          pop->entity_iarray[rank] = entity;
          }
        ga_entity_dereference_by_rank(pop, rank);
        }
      else
        {
        rank = ga_get_entity_rank(pop, son);
        if (son->fitness < mother->fitness)
          {
          entity = pop->entity_iarray[i];
          pop->entity_iarray[i] = pop->entity_iarray[rank];
          pop->entity_iarray[rank] = entity;
          }
        ga_entity_dereference_by_rank(pop, rank);

        rank = ga_get_entity_rank(pop, daughter);
        if (daughter->fitness < father->fitness)
          {
          entity = pop->entity_iarray[permutation[i]];
          pop->entity_iarray[permutation[i]] = pop->entity_iarray[rank];
          pop->entity_iarray[rank] = entity;
          }
        ga_entity_dereference_by_rank(pop, rank);
        }
      }

/*
 * Use callback.
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


