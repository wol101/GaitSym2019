/**********************************************************************
  ga_intrinsics.c
 **********************************************************************

  ga_intrinics - Genetic algorithm routine intrinsics.
  Copyright ©2000-2004, Stewart Adcock <stewart@linux-domain.com>
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
		and performing GA operations using S-Lang scripting.

		Internally, and in the public C interface, pointers
		are used to identify the population and entity
		structures.  However, in the scripting interface these
		pointers are unusable, so identifing integers are
		used instead.

  To do: 	More error checking.
		Needs some tidying.
		Add any missing wrappers.
		In particular, need facility for mating/mutating entities.
		Functions for defining heuristic search algorithm parameters.

 **********************************************************************/

#include "gaul/ga_intrinsics.h"

#if HAVE_SLANG==1

/**********************************************************************
  ga_population_new_slang()
  synopsis:	Allocates and initialises a new population structure,
		and assigns a new population id to it.
  parameters:
  return:	unsigned int	population id for this new structure.
  last updated: 13 Feb 2002
 **********************************************************************/

static int ga_population_new_slang(	int *stable_size,
				int *num_chromosome,
				int *len_chromosome)
  {
  return ga_get_population_id(ga_population_new(*stable_size, *num_chromosome, *len_chromosome));
  }


/**********************************************************************
  ga_entity_seed_slang()
  synopsis:	Fills a population structure with (hopefully) vaguely
		reasonable, starting genes.  Most 'real' work is done
		in a user-specified function.
  parameters:	none
  return:	boolean success.
  last updated: 22/01/01
 **********************************************************************/

static int ga_entity_seed_slang(int *pop_id, int *id)
  {
  population	*pop;			/* Active population structure. */

  pop = ga_get_population_from_id(*pop_id);

  return ga_entity_seed(pop, ga_get_entity_from_id(pop, *id));
  }


/**********************************************************************
  ga_entity_id_from_rank_slang()
  synopsis:	Finds an entity's id from it's rank.
  parameters:	
  return:	
  last updated: 22/01/01
 **********************************************************************/

static int ga_entity_id_from_rank_slang(int *pop_id, int *rank)
  {
  return ga_get_entity_id_from_rank(
               ga_get_population_from_id(*pop_id), *rank );
  }


/**********************************************************************
  ga_entity_rank_from_id_slang()
  synopsis:	Finds an entity's id from it's rank.
  parameters:	
  return:	
  last updated: 22/01/01
 **********************************************************************/

static int ga_entity_rank_from_id_slang(int *pop_id, int *id)
  {
  return ga_get_entity_rank_from_id(
               ga_get_population_from_id(*pop_id), *id );
  }


/**********************************************************************
  ga_population_seed_slang()
  synopsis:	Fills a population structure with vaguely reasonable,
		random, starting genes.  Most 'real' work is done in
		a user-specified function.
  parameters:	none
  return:	boolean success.
  last updated: 22/01/01
 **********************************************************************/

static int ga_population_seed_slang(int *pop_id)
  {
  return ga_population_seed(ga_get_population_from_id(*pop_id));
  }


/**********************************************************************
  ga_population_write_slang()
  synopsis:	Writes entire population and it's genetic data to disk.
  parameters:
  return:
  last updated: 30 May 2002
 **********************************************************************/

static int ga_population_write_slang(int *pop_id, char *fname)
  {
  return ga_population_write(ga_get_population_from_id(*pop_id), fname);
  }


/**********************************************************************
  ga_population_read_slang()
  synopsis:	Reads entire population and it's genetic data back
		from disk.
  parameters:
  return:
  last updated: 22/01/01
 **********************************************************************/

static int ga_population_read_slang(char *fname)
  {
  return ga_get_population_id(ga_population_read(fname));
  }


/**********************************************************************
  ga_entity_write_slang()
  synopsis:	Writes an entity to disk.
  parameters:
  return:
  last updated: 30 May 2002
 **********************************************************************/

static int ga_entity_write_slang(int *pop_id, int *entity_id, char *fname)
  {
  population	*pop;

  pop = ga_get_population_from_id(*pop_id);

  return ga_entity_write(pop, ga_get_entity_from_id(pop, *entity_id), fname);
  }


/**********************************************************************
  ga_entity_read_slang()
  synopsis:	Reads an entity from disk.
  parameters:
  return:
  last updated: 30 May 2002
 **********************************************************************/

static int ga_entity_read_slang(int *pop_id, char *fname)
  {
  population	*pop;

  pop = ga_get_population_from_id(*pop_id);

  return ga_get_entity_id(pop, ga_entity_read(pop, fname));
  }


/**********************************************************************
  ga_entity_kill_slang()
  synopsis:	Marks an entity structure as unused (dereferences it).
		Any contents of entities data field are freed.
  parameters:
  return:
  last updated: 22/01/01
 **********************************************************************/

static int ga_entity_kill_slang(int *pop_id, int *id)
  {
  population	*pop;			/* Active population structure. */

  pop = ga_get_population_from_id(*pop_id);

  return ga_entity_dereference_by_rank(
                pop, ga_get_entity_rank_from_id(pop, *id) );
  }


/**********************************************************************
  ga_entity_erase_slang()
  synopsis:	Clears the entity's data.
		Equivalent to an optimised pair of
		ga_entity_kill_slang() and ga_entity_new_slang() calls.
		Chromosomes are guarenteed to be intact, but may be
		overwritten by user.
		Also decrements population size.
  parameters:
  return:
  last updated:	22/01/01
 **********************************************************************/

static int ga_entity_erase_slang(int *pop_id, int *id)
  {
  population	*pop;			/* Active population structure. */

  pop = ga_get_population_from_id(*pop_id);

  ga_entity_blank(pop, ga_get_entity_from_id(pop, *id));

  return TRUE;
  }


/**********************************************************************
  ga_entity_new_slang()
  synopsis:	Returns handle of a new, unused, entity structure.
		Also increments population size.
  parameters:
  return:
  last updated:	22/01/01
 **********************************************************************/

static int ga_entity_new_slang(int *pop_id)
  {
  population	*pop;			/* Active population structure. */

  pop = ga_get_population_from_id(*pop_id);

  return ga_get_entity_id(pop, ga_get_free_entity(pop));
  }


/**********************************************************************
  ga_entity_clone_slang()
  synopsis:	Returns to a new entity structure with the genes and
		structural data copied from the parent.
		Increments population size also.
  parameters:
  return:
  last updated:	22/01/01
 **********************************************************************/

static int ga_entity_clone_slang(int *pop_id, int *parent)
  {
  entity	*child;						/* Destination entity. */
  population	*pop = ga_get_population_from_id(*pop_id);	/* Population. */

  child = ga_get_free_entity(pop);
  ga_entity_copy(pop, child, ga_get_entity_from_id(pop, *parent));

  return ga_get_entity_id(pop, child);
  }


/**********************************************************************
  ga_entity_copy_chromosome_slang()
  synopsis:	Copy genetic data between entity structures.
  parameters:
  return:
  last updated: 29 Nov 2001
 **********************************************************************/

static int ga_entity_copy_chromosome_slang(int *pop_id, int *parent, int *child, int *chromosome)
  {
  population	*pop;			/* Active population structure. */

  pop = ga_get_population_from_id(*pop_id);

  return ga_entity_copy_chromosome(pop,
                                   ga_get_entity_from_id(pop, *child),
                                   ga_get_entity_from_id(pop, *parent),
                                   *chromosome);
  }


/**********************************************************************
  ga_entity_copy_all_chromosomes_slang()
  synopsis:	Copy genetic data between entity structures.
  parameters:
  return:
  last updated: 29 Nov 2001
 **********************************************************************/

static int ga_entity_copy_all_chromosomes_slang(int *pop_id, int *parent, int *child)
  {
  population	*pop;			/* Active population structure. */

  pop = ga_get_population_from_id(*pop_id);

  return ga_entity_copy_all_chromosomes(pop,
                                        ga_get_entity_from_id(pop, *child),
                                        ga_get_entity_from_id(pop, *parent));
  }


/**********************************************************************
  ga_entity_migrate_slang()
  synopsis:	Copy entity from one population into another - does not
		delete the original.  The caller should do that, if
		required.
		FIXME: Need check to confirm that populations are
		compatible.
  parameters:
  return:
  last updated:	14/02/01
 **********************************************************************/

static int ga_entity_migrate_slang(int *srcpop_id, int *destpop_id, int *jacques)
  {
  population	*srcpop;	/* Original population. */
  population	*destpop;	/* Destination population. */
  entity	*jack;		/* Migrated entity. */

  srcpop = ga_get_population_from_id(*srcpop_id);
  destpop = ga_get_population_from_id(*destpop_id);

  jack = ga_get_free_entity(destpop);
  ga_entity_copy(srcpop, jack, ga_get_entity_from_id(srcpop, *jacques));

  return ga_get_entity_id(destpop, jack);
  }


#if 0
/**********************************************************************
  ga_singlepoint_crossover_chromosome()
  synopsis:	`Mates' two chromosomes by single-point crossover.
  parameters:
  return:
  last updated: 18/10/00
 **********************************************************************/

boolean ga_singlepoint_crossover_chromosome(int *father, int *mother, int *son, int *daughter)
  {
  }


/**********************************************************************
  ga_crossover_chromosome_singlepoints_slang()
  synopsis:	`Mates' two genotypes by single-point crossover of
		each chromosome.
  parameters:
  return:
  last updated: 14/02/01
 **********************************************************************/

int ga_crossover_chromosome_singlepoints_slang(int *pop_id,
                     int *father, int *mother, int *son, int *daughter)
  {
  population	*pop;			/* Active population structure. */

  pop = ga_get_population_from_id(*pop_id);

  ga_crossover_chromosome_singlepoints(pop,
                      ga_get_entity_from_id(pop, *father),
                      ga_get_entity_from_id(pop, *mother),
                      ga_get_entity_from_id(pop, *son),
                      ga_get_entity_from_id(pop, *daughter) );

  return TRUE;
  }


/**********************************************************************
  ga_crossover_chromosome_mixing_slang()
  synopsis:	`Mates' two genotypes by crossover (chromosome mixing).
		Keeps all chromosomes intact, and therefore will not
		need to recreate any structural data.
  parameters:
  return:
  last updated:	14/02/01
 **********************************************************************/

static int ga_crossover_chromosome_mixing_slang(int *pop_id,
                     int *father, int *mother, int *son, int *daughter)
  {
  population    *pop;                   /* Active population structure. */

  pop = ga_get_population_from_id(*pop_id);

  ga_crossover_chromosome_mixing(pop,
                      ga_get_entity_from_id(pop, *father),
                      ga_get_entity_from_id(pop, *mother),
                      ga_get_entity_from_id(pop, *son),
                      ga_get_entity_from_id(pop, *daughter) );

  return TRUE;
  }
#endif


/**********************************************************************
  ga_entity_score_slang()
  synopsis:	Score entity.
  parameters:
  return:
  last updated: 07/02/01
 **********************************************************************/

static int ga_entity_score_slang(int *pop_id, int *joe)
  {
  population	*pop;			/* Active population structure. */

  pop = ga_get_population_from_id(*pop_id);

  if (!pop->evaluate) die("Scoring function not defined.");

/*  return pop->evaluate(pop, ga_get_entity_from_id(pop, *joe));*/
  pop->evaluate(pop, ga_get_entity_from_id(pop, *joe));

/*
  plog(LOG_DEBUG, "Return from pop->evaluate().\n");
*/

  return TRUE;
  }


/**********************************************************************
  ga_evolution_slang()
  synopsis:	Wrapper around the main genetic algorithm routine.
		It performs a GA-based optimisation on the specified
		population.
  parameters:
  return:
  last updated:	11 Jun 2002
 **********************************************************************/

static int ga_evolution_slang(	int	*pop,
			int	*max_generations )
  {
  ga_evolution( ga_get_population_from_id(*pop), *max_generations );

  return TRUE;
  }


/**********************************************************************
  ga_evolution_forked_slang()
  synopsis:	Wrapper around the main genetic algorithm routine.
		It performs a GA-based optimisation on the specified
		population.
  parameters:
  return:
  last updated:	11 Jun 2002
 **********************************************************************/

static int ga_evolution_forked_slang(	int	*pop,
			int	*max_generations )
  {
  ga_evolution_forked( ga_get_population_from_id(*pop), *max_generations );

  return TRUE;
  }


/**********************************************************************
  ga_evolution_threaded_slang()
  synopsis:	Wrapper around the main genetic algorithm routine.
		It performs a GA-based optimisation on the specified
		population.
  parameters:
  return:
  last updated:	19 Aug 2003
 **********************************************************************/

static int ga_evolution_threaded_slang(	int	*pop,
			int	*max_generations )
  {
  ga_evolution_threaded( ga_get_population_from_id(*pop), *max_generations );

  return TRUE;
  }


#if 0
/**********************************************************************
  ga_evolution_mpi_slang()
  synopsis:	Wrapper around the main genetic algorithm routine.
		It performs a GA-based optimisation on the specified
		population.
  parameters:
  return:
  last updated:	19 Aug 2003
 **********************************************************************/

static int ga_evolution_mpi_slang(	int	*pop,
			int	*max_generations )
  {
  ga_evolution_mpi( ga_get_population_from_id(*pop), *max_generations );

  return TRUE;
  }
#endif


/**********************************************************************
  ga_population_set_parameters_slang()
  synopsis:	Sets the GA parameters for a population.
  parameters:
  return:
  last updated:	19 Aug 2003
 **********************************************************************/

static int ga_population_set_parameters_slang( int	*pop,
					 int	*scheme,
					 int	*elitism,
					 double	*crossover,
					 double	*mutation,
					 double	*migration)
  {

  ga_population_set_parameters( ga_get_population_from_id(*pop),
                     (ga_scheme_type) *scheme, (ga_elitism_type) *elitism,
                     *crossover, *mutation, *migration );

  return TRUE;
  }


/**********************************************************************
  ga_population_get_size_slang()
  synopsis:	Access population's size field.
  parameters:
  return:
  last updated:	22/01/01
 **********************************************************************/

static int ga_population_get_size_slang(int *pop)
  {
  return ((population*) ga_get_population_from_id(*pop))->size;
  }


/**********************************************************************
  ga_population_get_maxsize_slang()
  synopsis:	Access population's max_size field.
  parameters:
  return:
  last updated:	22/01/01
 **********************************************************************/

static int ga_population_get_maxsize_slang(int *pop)
  {
  return ((population*) ga_get_population_from_id(*pop))->max_size;
  }


/**********************************************************************
  ga_population_get_stablesize_slang()
  synopsis:	Access population's stable_size field.
  parameters:
  return:
  last updated:	22/01/01
 **********************************************************************/

static int ga_population_get_stablesize_slang(int *pop)
  {
  return ((population*) ga_get_population_from_id(*pop))->stable_size;
  }


/**********************************************************************
  ga_population_set_stablesize_slang()
  synopsis:	Alter population's stable_size field.
		This should be used with care.
  parameters:
  return:
  last updated:	27/03/01
 **********************************************************************/

static int ga_population_set_stablesize_slang(int *pop, int *size)
  {
  ((population*) ga_get_population_from_id(*pop))->stable_size = *size;
  return TRUE;
  }


/**********************************************************************
  ga_population_get_crossoverratio_slang()
  synopsis:	Access population's crossover_ratio field.
  parameters:
  return:
  last updated:	23/04/01
 **********************************************************************/

static double ga_population_get_crossoverratio_slang(int *pop)
  {
  return ((population*) ga_get_population_from_id(*pop))->crossover_ratio;
  }


/**********************************************************************
  ga_population_set_crossoverratio_slang()
  synopsis:	Alter population's crossover_ratio field.
  parameters:
  return:
  last updated:	23/04/01
 **********************************************************************/

static int ga_population_set_crossoverratio_slang(int *pop, double *value)
  {
  ((population*) ga_get_population_from_id(*pop))->crossover_ratio = *value;
  return TRUE;
  }


/**********************************************************************
  ga_population_get_mutationratio_slang()
  synopsis:	Access population's mutation_ratio field.
  parameters:
  return:
  last updated:	23/04/01
 **********************************************************************/

static double ga_population_get_mutationratio_slang(int *pop)
  {
  return ((population*) ga_get_population_from_id(*pop))->mutation_ratio;
  }


/**********************************************************************
  ga_population_set_mutationratio_slang()
  synopsis:	Alter population's mutation_ratio field.
  parameters:
  return:
  last updated:	23/04/01
 **********************************************************************/

static int ga_population_set_mutationratio_slang(int *pop, double *value)
  {
  ((population*) ga_get_population_from_id(*pop))->mutation_ratio = *value;
  return TRUE;
  }


/**********************************************************************
  ga_population_get_migrationratio_slang()
  synopsis:	Access population's migration_ratio field.
  parameters:
  return:
  last updated:	23/04/01
 **********************************************************************/

static int ga_population_get_migrationratio_slang(int *pop)
  {
  return ((population*) ga_get_population_from_id(*pop))->migration_ratio;
  }


/**********************************************************************
  ga_population_set_migrationratio_slang()
  synopsis:	Alter population's migration_ratio field.
  parameters:
  return:
  last updated:	23/04/01
 **********************************************************************/

static int ga_population_set_migrationratio_slang(int *pop, int *value)
  {
  ((population*) ga_get_population_from_id(*pop))->migration_ratio = *value;
  return TRUE;
  }


/**********************************************************************
  ga_population_set_scheme_slang()
  synopsis:	Alter population's evolutionary class field.
  parameters:
  return:
  last updated:	20 Sep 2002
 **********************************************************************/

static int ga_population_set_scheme_slang(int *pop, int *value)
  {
  ((population*) ga_get_population_from_id(*pop))->scheme = *value;
  return TRUE;
  }


/**********************************************************************
  ga_population_set_elitism_slang()
  synopsis:	Alter population's elitism mode field.
  parameters:
  return:
  last updated:	20 Sep 2002
 **********************************************************************/

static int ga_population_set_elitism_slang(int *pop, int *value)
  {
  ((population*) ga_get_population_from_id(*pop))->elitism = *value;
  return TRUE;
  }


/**********************************************************************
  ga_population_get_chromosomenum_slang()
  synopsis:	Access population's num_chromosomes field.
  parameters:
  return:
  last updated:	22/01/01
 **********************************************************************/

static int ga_population_get_chromosomenum_slang(int *pop)
  {
  return ((population*) ga_get_population_from_id(*pop))->num_chromosomes;
  }


/**********************************************************************
  ga_population_get_chromosomelen_slang()
  synopsis:	Access population's len_chromosomes field.
  parameters:
  return:
  last updated:	22/01/01
 **********************************************************************/

static int ga_population_get_chromosomelen_slang(int *pop)
  {
  return ((population*) ga_get_population_from_id(*pop))->len_chromosomes;
  }


/**********************************************************************
  ga_population_get_generation_slang()
  synopsis:	Access population's generation field.
  parameters:
  return:
  last updated:	20 Mar 2003
 **********************************************************************/

static int ga_population_get_generation_slang(int *pop)
  {
  return ((population*) ga_get_population_from_id(*pop))->generation;
  }


/**********************************************************************
  ga_entity_get_fitness_slang()
  synopsis:	Access entity's fitness field.
  parameters:
  return:
  last updated:	22/01/01
 **********************************************************************/

static double ga_entity_get_fitness_slang(int *pop, int *id)
  {
  return ga_get_entity_from_id(ga_get_population_from_id(*pop), *id)->fitness;
  }


/**********************************************************************
  ga_entity_isallocated_slang()
  synopsis:	Determine whether the given entity id is used.
  parameters:
  return:
  last updated:	18 Mar 2002
 **********************************************************************/

static int ga_entity_isallocated_slang(int *pop, int *id)
  {
  return ga_get_entity_from_id(ga_get_population_from_id(*pop), *id) != NULL;
  }


/**********************************************************************
  ga_extinction_slang()
  synopsis:	Purge all memory used by a population.
  parameters:
  return:
  last updated:	22/01/01
 **********************************************************************/

static int ga_extinction_slang(int *pop)
  {
  return ga_extinction( ga_get_population_from_id(*pop) );
  }


/**********************************************************************
  ga_genocide_slang()
  synopsis:	Kill population members.
  parameters:
  return:
  last updated:	11/01/01
 **********************************************************************/

static int ga_genocide_slang(int *pop, int *target_size)
  {
  return ga_genocide( ga_get_population_from_id(*pop), *target_size );
  }


/**********************************************************************
  ga_genocide_by_fitness_slang()
  synopsis:	Kill population members.
  parameters:
  return:
  last updated:	01 Jul 2004
 **********************************************************************/

static int ga_genocide_by_fitness_slang(int *pop, double *target_fitness)
  {
  return ga_genocide( ga_get_population_from_id(*pop), *target_fitness );
  }


/**********************************************************************
  ga_allele_search_slang()
  synopsis:	Wrapper around ga_allele_search() for the scripted API.
  parameters:
  return:	Index of best solution found (A new entity).
  last updated:	18 Mar 2002
 **********************************************************************/

static int ga_allele_search_slang(	int	*pop_id,
				int	*chromosomeid,
				int 	*point,
				int	*min_val,
				int	*max_val,
				int	*entity_id )
  {
  entity	*initial, *final;	/* Initial and final solutions. */
  population	*pop;			/* Active population structure. */

  pop = ga_get_population_from_id(*pop_id);

  initial = ga_get_entity_from_id(pop, *entity_id);

  final = ga_allele_search( pop,
                            *chromosomeid, *point, *min_val, *max_val,
                            initial );

  return ga_get_entity_id(pop, final);
  }


#if 0
/**********************************************************************
  ga_metropolis_slang()
  synopsis:	Wrapper around ga_metropolis_mutation() for the
		scripted API.
  parameters:
  return:	Index of best solution found (A new entity).
  last updated:	18 Mar 2002
 **********************************************************************/

int ga_metropolis_slang(	int	*pop_id,
				int	*entity_id,
				int	*num_iterations,
				int 	*temperature)
  {
  entity	*initial, *final;	/* Initial and final solutions. */
  population	*pop;			/* Active population structure. */

  pop = ga_get_population_from_id(*pop_id);

  initial = ga_get_entity_from_id(pop, *entity_id);

  final = ga_metropolis_mutation( pop, initial,
                         *num_iterations, *temperature );

  return ga_get_entity_id(pop, final);
  }
#endif


/**********************************************************************
  ga_sa_slang()
  synopsis:	Wrapper around ga_sa()
		for the scripted API.
  parameters:
  return:	Index of best solution found (A new entity).
  last updated:	06 Nov 2002
 **********************************************************************/

static int ga_sa_slang(	int	*pop_id,
			int	*entity_id,
			int	*max_iterations )
  {
  entity	*initial;	/* Solution to optimise. */
  population	*pop;		/* Active population structure. */
  int		num_iter;	/* Number of iterations performed. */

  pop = ga_get_population_from_id(*pop_id);

  initial = ga_get_entity_from_id(pop, *entity_id);

  num_iter = ga_sa( pop, initial, *max_iterations );

  return num_iter;
  }


/**********************************************************************
  ga_simpex_slang()
  synopsis:	Wrapper around ga_simplex()
		for the scripted API.
  parameters:
  return:	Index of best solution found (A new entity).
  last updated:	06 Nov 2002
 **********************************************************************/

static int ga_simplex_slang(	int	*pop_id,
			int	*entity_id,
			int	*max_iterations )
  {
  entity	*initial;	/* Solution to optimise. */
  population	*pop;		/* Active population structure. */
  int		num_iter;	/* Number of iterations performed. */

  pop = ga_get_population_from_id(*pop_id);

  initial = ga_get_entity_from_id(pop, *entity_id);

  num_iter = ga_simplex( pop, initial, *max_iterations );

  return num_iter;
  }


/**********************************************************************
  ga_tabu_slang()
  synopsis:	Wrapper around ga_tabu()
		for the scripted API.
  parameters:
  return:	Index of best solution found (A new entity).
  last updated:	06 Nov 2002
 **********************************************************************/

static int ga_tabu_slang(	int	*pop_id,
			int	*entity_id,
			int	*max_iterations )
  {
  entity	*initial;	/* Solution to optimise. */
  population	*pop;		/* Active population structure. */
  int		num_iter;	/* Number of iterations performed. */

  pop = ga_get_population_from_id(*pop_id);

  initial = ga_get_entity_from_id(pop, *entity_id);

  num_iter = ga_tabu( pop, initial, *max_iterations );

  return num_iter;
  }


/**********************************************************************
  ga_randomsearch_slang()
  synopsis:	Wrapper around ga_randomsearch()
		for the scripted API.
  parameters:
  return:	Index of best solution found (A new entity).
  last updated:	06 Nov 2002
 **********************************************************************/

static int ga_random_search_slang(	int	*pop_id,
			int	*entity_id,
			int	*max_iterations )
  {
  entity	*initial;	/* Solution to optimise. */
  population	*pop;		/* Active population structure. */
  int		num_iter;	/* Number of iterations performed. */

  pop = ga_get_population_from_id(*pop_id);

  initial = ga_get_entity_from_id(pop, *entity_id);

  num_iter = ga_random_search( pop, initial, *max_iterations );

  return num_iter;
  }


/**********************************************************************
  ga_search_slang()
  synopsis:	Wrapper around ga_search()
		for the scripted API.
  parameters:
  return:	Index of best solution found (A new entity).
  last updated:	08 Nov 2002
 **********************************************************************/

static int ga_search_slang(	int	*pop_id,
			int	*entity_id )
  {
  entity	*initial;	/* Solution to optimise. */
  population	*pop;		/* Active population structure. */
  int		num_iter;	/* Number of iterations performed. */

  pop = ga_get_population_from_id(*pop_id);

  initial = ga_get_entity_from_id(pop, *entity_id);

  num_iter = ga_search( pop, initial );

  return num_iter;
  }


/**********************************************************************
  ga_nahc_slang()
  synopsis:	Wrapper around ga_next_ascent_hillclimbing() for
		scripted API.
  parameters:
  return:	Index of best solution found (A new entity).
  last updated:	06 Nov 2002
 **********************************************************************/

static int ga_nahc_slang(	int	*pop_id,
			int	*entity_id,
			int	*num_iterations )
  {
  entity	*initial;	/* Solution to optimise. */
  population	*pop;		/* Active population structure. */
  int		num_iter;	/* Number of iterations performed. */

  pop = ga_get_population_from_id(*pop_id);

  initial = ga_get_entity_from_id(pop, *entity_id);

  num_iter = ga_next_ascent_hillclimbing( pop, initial, *num_iterations);

  return num_iter;
  }


/**********************************************************************
  ga_rahc_slang()
  synopsis:	Wrapper around ga_random_ascent_hillclimbing() for
		scripted API.
  parameters:
  return:	Index of best solution found (A new entity).
  last updated:	06 Nov 2002
 **********************************************************************/

static int ga_rahc_slang(	int	*pop_id,
			int	*entity_id,
			int	*num_iterations )
  {
  entity	*initial;	/* Solution to optimise. */
  population	*pop;		/* Active population structure. */
  int		num_iter;	/* Number of iterations performed. */

  pop = ga_get_population_from_id(*pop_id);

  initial = ga_get_entity_from_id(pop, *entity_id);

  num_iter = ga_random_ascent_hillclimbing( pop, initial, *num_iterations );

  return num_iter;
  }


/**********************************************************************
  ga_population_score_and_sort_slang()
  synopsis:	Wrapper around ga_population_score_and_sort() for
		scripted API.  Recommended for use after reading
		the population from disk.
  parameters:	Population handle.
  return:	Success.
  last updated:	28/02/01
 **********************************************************************/

static int ga_population_score_and_sort_slang(int *pop_id)
  {
  return ga_population_score_and_sort(ga_get_population_from_id(*pop_id));
  }


/**********************************************************************
  ga_population_sort_slang()
  synopsis:	Wrapper around ga_population_sort() for
		scripted API.
  parameters:	Population handle.
  return:	Success.
  last updated:	20 May 2002
 **********************************************************************/

static int ga_population_sort_slang(int *pop_id)
  {

  sort_population(ga_get_population_from_id(*pop_id));

  return TRUE;
  }


#if 0
/**********************************************************************
  ga_slang_seed()
  synopsis:	Handle S-Lang based seeding operator.
  parameters:
  return:
  last updated:	13 Jun 2002
 **********************************************************************/

static boolean ga_slang_seed(population *pop, entity *adam)
  {

  plog(LOG_FIXME, "ga_slang_seed() is not implemented.");

  return TRUE;
  }


/**********************************************************************
  ga_slang_select_one()
  synopsis:	Handle S-Lang based single selection operator.
  parameters:
  return:
  last updated:	24/04/01
 **********************************************************************/

static boolean ga_slang_select_one(population *pop, entity **mother)
  {

  plog(LOG_FIXME, "ga_slang_select_one() is not implemented.");

  return TRUE;
  }


/**********************************************************************
  ga_slang_select_two()
  synopsis:	Handle S-Lang based double selection operator.
  parameters:
  return:
  last updated:	24/04/01
 **********************************************************************/

static boolean ga_slang_select_two(population *pop, entity **mother, entity **father)
  {

  plog(LOG_FIXME, "ga_slang_select_two() is not implemented.");

  return TRUE;
  }


/**********************************************************************
  ga_slang_adapt()
  synopsis:	Handle S-Lang based adaption operator.
  parameters:
  return:
  last updated:	24/04/01
 **********************************************************************/

static entity *ga_slang_adapt(population *pop, entity *child)
  {
  entity	*adult;		/* Optimised entity. */

/* Checks */
  if (!pop) die("Null pointer to population structure passed.");
  if (!child) die("Null pointer to entity structure passed.");

  adult = ga_get_free_entity(pop);

  if ( SLang_run_hooks( "adapt_hook", 3,
                        ga_get_population_id(pop), 
                        ga_get_entity_id(pop, child), 
                        ga_get_entity_id(pop, adult)) == -1 )
    die("Error calling S-Lang function \"adapt_hook\".");

  return adult;
  }


/**********************************************************************
  ga_slang_crossover()
  synopsis:	Handle S-Lang based crossover operator.
  parameters:
  return:
  last updated:	24/04/01
 **********************************************************************/

static void ga_slang_crossover(population *pop, entity *father, entity *mother, entity *daughter, entity *son)
  {

/* Checks */
  if (!pop) die("Null pointer to population structure passed.");
  if (!father || !mother || !son || !daughter)
    die("Null pointer to entity structure passed.");

  if ( SLang_run_hooks( "crossover_hook", 5,
                        ga_get_population_id(pop), 
                        ga_get_entity_id(pop, father), 
                        ga_get_entity_id(pop, mother), 
                        ga_get_entity_id(pop, son), 
                        ga_get_entity_id(pop, daughter)) == -1 )
    die("Error calling S-Lang function \"crossover_hook\".");

  return;
  }


/**********************************************************************
  ga_slang_mutate()
  synopsis:	Handle S-Lang based mutation operator.
  parameters:
  return:
  last updated:	24/04/01
 **********************************************************************/

static void ga_slang_mutate(population *pop, entity *father, entity *son)
  {

/* Checks */
  if (!pop) die("Null pointer to population structure passed.");
  if (!father || !son) die("Null pointer to entity structure passed.");

#if 0
/*
 * Duplicate the initial data.
 */
  for (i=0; i<pop->num_chromosomes; i++)
    {
    memcpy(son->chromosome[i], father->chromosome[i], pop->len_chromosomes*sizeof(int));
    ga_copy_data(pop, son, father, i);
    }
#endif

  if ( SLang_run_hooks( "mutate_hook", 3,
                        ga_get_population_id(pop), 
                        ga_get_entity_id(pop, father), 
                        ga_get_entity_id(pop, son)) == -1 )
    die("Error calling S-Lang function \"mutate_hook\".");

  return;
  }


/**********************************************************************
  ga_slang_replace()
  synopsis:	Handle S-Lang based replacement operator.
  parameters:
  return:
  last updated:	08 Apr 2003
 **********************************************************************/

static void ga_slang_replace(population *pop, entity *child)
  {

/* Checks */
  if (!pop) die("Null pointer to population structure passed.");
  if (!child) die("Null pointer to entity structure passed.");

  if ( SLang_run_hooks( "replace_hook", 2,
                        ga_get_population_id(pop), 
                        ga_get_entity_id(pop, child)) == -1 )
    die("Error calling S-Lang function \"replace_hook\".");

  return;
  }
#endif
#endif


/**********************************************************************
  ga_intrinsic_sladd()
  synopsis:	Register the S-Lang intrinsics.
  parameters:	none
  return:	success/failure.
  last updated:	18 Mar 2003
 **********************************************************************/

#if HAVE_SLANG==0
boolean ga_intrinsic_sladd(void)
  {
  plog(LOG_WARNING, "No S-Lang support compiled into GAUL.");
  return TRUE;
  }
#else

boolean ga_intrinsic_sladd(void)
  {
  static double	fitnessmin=GA_MIN_FITNESS;      /* Minimum fitness. */
  static int	schemes[7]={GA_SCHEME_DARWIN, GA_SCHEME_LAMARCK_PARENTS, GA_SCHEME_LAMARCK_CHILDREN, GA_SCHEME_LAMARCK_ALL, GA_SCHEME_BALDWIN_PARENTS, GA_SCHEME_BALDWIN_CHILDREN, GA_SCHEME_BALDWIN_ALL};
  static int	elitism[5]={GA_ELITISM_UNKNOWN, GA_ELITISM_PARENTS_SURVIVE, GA_ELITISM_ONE_PARENT_SURVIVES, GA_ELITISM_PARENTS_DIE, GA_ELITISM_RESCORE_PARENTS};

  if (  SLadd_intrinsic_variable("GA_SCHEME_DARWIN", &(schemes[0]), SLANG_INT_TYPE, TRUE)
     || SLadd_intrinsic_variable("GA_SCHEME_LAMARCK_PARENTS", &(schemes[1]), SLANG_INT_TYPE, TRUE)
     || SLadd_intrinsic_variable("GA_SCHEME_LAMARCK_CHILDREN", &(schemes[2]), SLANG_INT_TYPE, TRUE)
     || SLadd_intrinsic_variable("GA_SCHEME_LAMARCK_ALL", &(schemes[3]), SLANG_INT_TYPE, TRUE)
     || SLadd_intrinsic_variable("GA_SCHEME_BALDWIN_PARENTS", &(schemes[4]), SLANG_INT_TYPE, TRUE)
     || SLadd_intrinsic_variable("GA_SCHEME_BALDWIN_CHILDREN", &(schemes[5]), SLANG_INT_TYPE, TRUE)
     || SLadd_intrinsic_variable("GA_SCHEME_BALDWIN_ALL", &(schemes[6]), SLANG_INT_TYPE, TRUE)
     || SLadd_intrinsic_variable("GA_ELITISM_UNKNOWN", &(elitism[0]), SLANG_INT_TYPE, TRUE)
     || SLadd_intrinsic_variable("GA_ELITISM_PARENTS_SURVIVE", &(elitism[1]), SLANG_INT_TYPE, TRUE)
     || SLadd_intrinsic_variable("GA_ELITISM_ONE_PARENT_SURVIVES", &(elitism[2]), SLANG_INT_TYPE, TRUE)
     || SLadd_intrinsic_variable("GA_ELITISM_PARENTS_DIE", &(elitism[3]), SLANG_INT_TYPE, TRUE)
     || SLadd_intrinsic_variable("GA_ELITISM_RESCORE_PARENTS", &(elitism[4]), SLANG_INT_TYPE, TRUE)
     || SLadd_intrinsic_variable("GA_FITNESS_MIN", &fitnessmin, SLANG_DOUBLE_TYPE, TRUE)
     ) return FALSE;

  if (
         SLadd_intrinsic_function("ga_population_new",
            (FVOID_STAR) ga_population_new_slang, SLANG_INT_TYPE, 3,
            SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_entity_seed",
            (FVOID_STAR) ga_entity_seed_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_entity_id_from_rank",
            (FVOID_STAR) ga_entity_id_from_rank_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_entity_rank_from_id",
            (FVOID_STAR) ga_entity_rank_from_id_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_population_seed",
            (FVOID_STAR) ga_population_seed_slang, SLANG_INT_TYPE, 1,
            SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_population_write",
            (FVOID_STAR) ga_population_write_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_STRING_TYPE)
      || SLadd_intrinsic_function("ga_population_read",
            (FVOID_STAR) ga_population_read_slang, SLANG_INT_TYPE, 1,
            SLANG_STRING_TYPE)
      || SLadd_intrinsic_function("ga_entity_write",
            (FVOID_STAR) ga_entity_write_slang, SLANG_INT_TYPE, 3,
            SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_STRING_TYPE)
      || SLadd_intrinsic_function("ga_entity_read",
            (FVOID_STAR) ga_entity_read_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_STRING_TYPE)
      || SLadd_intrinsic_function("ga_entity_kill",
            (FVOID_STAR) ga_entity_kill_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_entity_erase",
            (FVOID_STAR) ga_entity_erase_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_entity_new",
            (FVOID_STAR) ga_entity_new_slang, SLANG_INT_TYPE, 1,
            SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_entity_clone",
            (FVOID_STAR) ga_entity_clone_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_entity_copy_chromosome",
            (FVOID_STAR) ga_entity_copy_chromosome_slang, SLANG_INT_TYPE, 4,
            SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_entity_copy_all_chromosomes",
            (FVOID_STAR) ga_entity_copy_all_chromosomes_slang, SLANG_INT_TYPE, 3,
            SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_evolution",
            (FVOID_STAR) ga_evolution_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_evolution_forked",
            (FVOID_STAR) ga_evolution_forked_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_evolution_threaded",
            (FVOID_STAR) ga_evolution_threaded_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_INT_TYPE)
/*
      || SLadd_intrinsic_function("ga_evolution_mpi",
            (FVOID_STAR) ga_evolution_mpi_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_INT_TYPE)
*/
      || SLadd_intrinsic_function("ga_population_set_parameters",
            (FVOID_STAR) ga_population_set_parameters_slang, SLANG_INT_TYPE, 6,
            SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_DOUBLE_TYPE, SLANG_DOUBLE_TYPE, SLANG_DOUBLE_TYPE)
      || SLadd_intrinsic_function("ga_population_get_size",
            (FVOID_STAR) ga_population_get_size_slang, SLANG_INT_TYPE, 1,
            SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_population_get_maxsize",
            (FVOID_STAR) ga_population_get_maxsize_slang, SLANG_INT_TYPE, 1,
            SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_population_get_stablesize",
            (FVOID_STAR) ga_population_get_stablesize_slang, SLANG_INT_TYPE, 1,
            SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_population_set_stablesize",
            (FVOID_STAR) ga_population_set_stablesize_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_population_get_crossoverratio",
            (FVOID_STAR) ga_population_get_crossoverratio_slang, SLANG_DOUBLE_TYPE, 1,
            SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_population_set_crossoverratio",
            (FVOID_STAR) ga_population_set_crossoverratio_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_DOUBLE_TYPE)
      || SLadd_intrinsic_function("ga_population_get_mutationratio",
            (FVOID_STAR) ga_population_get_mutationratio_slang, SLANG_DOUBLE_TYPE, 1,
            SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_population_set_mutationratio",
            (FVOID_STAR) ga_population_set_mutationratio_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_DOUBLE_TYPE)
      || SLadd_intrinsic_function("ga_population_get_migrationratio",
            (FVOID_STAR) ga_population_get_migrationratio_slang, SLANG_DOUBLE_TYPE, 1,
            SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_population_set_migrationratio",
            (FVOID_STAR) ga_population_set_migrationratio_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_DOUBLE_TYPE)
      || SLadd_intrinsic_function("ga_population_set_scheme",
            (FVOID_STAR) ga_population_set_scheme_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_population_set_elitism",
            (FVOID_STAR) ga_population_set_elitism_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_population_get_chromosomenum",
            (FVOID_STAR) ga_population_get_chromosomenum_slang, SLANG_INT_TYPE, 1,
            SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_population_get_chromosomelen",
            (FVOID_STAR) ga_population_get_chromosomelen_slang, SLANG_INT_TYPE, 1,
            SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_population_get_generation",
            (FVOID_STAR) ga_population_get_generation_slang, SLANG_INT_TYPE, 1,
            SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_entity_get_fitness",
            (FVOID_STAR) ga_entity_get_fitness_slang, SLANG_DOUBLE_TYPE, 2,
            SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_entity_isallocated",
            (FVOID_STAR) ga_entity_isallocated_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_extinction",
            (FVOID_STAR) ga_extinction_slang, SLANG_INT_TYPE, 1,
            SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_genocide",
            (FVOID_STAR) ga_genocide_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_genocide_by_fitness",
            (FVOID_STAR) ga_genocide_by_fitness_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_DOUBLE_TYPE)
      || SLadd_intrinsic_function("ga_allele_search",
            (FVOID_STAR) ga_allele_search_slang, SLANG_INT_TYPE, 6,
            SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_sa",
            (FVOID_STAR) ga_sa_slang, SLANG_INT_TYPE, 3,
            SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_tabu",
            (FVOID_STAR) ga_tabu_slang, SLANG_INT_TYPE, 3,
            SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_simplex",
            (FVOID_STAR) ga_simplex_slang, SLANG_INT_TYPE, 3,
            SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_nahc",
            (FVOID_STAR) ga_nahc_slang, SLANG_INT_TYPE, 3,
            SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_rahc",
            (FVOID_STAR) ga_rahc_slang, SLANG_INT_TYPE, 3,
            SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_random_search",
            (FVOID_STAR) ga_random_search_slang, SLANG_INT_TYPE, 3,
            SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_search",
            (FVOID_STAR) ga_search_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_population_sort",
            (FVOID_STAR) ga_population_sort_slang, SLANG_INT_TYPE, 1,
            SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_population_score_and_sort",
            (FVOID_STAR) ga_population_score_and_sort_slang, SLANG_INT_TYPE, 1,
            SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_entity_score",
            (FVOID_STAR) ga_entity_score_slang, SLANG_INT_TYPE, 2,
            SLANG_INT_TYPE, SLANG_INT_TYPE)
      || SLadd_intrinsic_function("ga_entity_migrate",
            (FVOID_STAR) ga_entity_migrate_slang, SLANG_INT_TYPE, 3,
            SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_INT_TYPE)
     ) return FALSE;

  return TRUE;
  }

#endif

