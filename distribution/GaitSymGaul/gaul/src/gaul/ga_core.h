/**********************************************************************
  ga_core.h
 **********************************************************************

  ga_core - Genetic algorithm routines.
  Copyright ©2000-2005, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:     Routines for handling populations and performing GA
		operations.

 **********************************************************************/

#ifndef GA_CORE_H_INCLUDED
#define GA_CORE_H_INCLUDED

/*
 * Includes.
 */
#include "gaul.h"

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include <math.h>
#include <sys/types.h>
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

/*
 * Debugging
 */
#ifndef GA_DEBUG
# ifdef DEBUG
#  define GA_DEBUG	DEBUG
# else
#  define GA_DEBUG	0
# endif
#endif

/*
 * Specification of number of processes used in
 * multiprocess functions.
 */
#ifndef GA_NUM_PROCESSES_ENVVAR_STRING
#define GA_NUM_PROCESSES_ENVVAR_STRING  "GAUL_NUM_PROCESSES"
#endif
  
#ifndef GA_DEFAULT_NUM_PROCESSES
#define GA_DEFAULT_NUM_PROCESSES        8
#endif

/*
 * Specification of number of threads used in
 * multithreaded functions.
 */
#ifndef GA_NUM_THREADS_ENVVAR_STRING
#define GA_NUM_THREADS_ENVVAR_STRING	"GAUL_NUM_THREADS"
#endif
  
#ifndef GA_DEFAULT_NUM_THREADS
#define GA_DEFAULT_NUM_THREADS		4
#endif

/*
 * Whether simple statistics should be dumped to disk.
 */
#ifndef GA_WRITE_STATS
# if GA_DEBUG > 1
#  define GA_WRITE_STATS	TRUE
# else
#  define GA_WRITE_STATS	FALSE
# endif
#endif

/*
 * Include remainder of this library's headers.
 */
#include "gaul/ga_bitstring.h"
#include "gaul/ga_chromo.h"
#include "gaul/ga_climbing.h"
#include "gaul/ga_de.h"
#include "gaul/ga_deterministiccrowding.h"
#include "gaul/ga_gradient.h"
#include "gaul/ga_optim.h"
#include "gaul/ga_qsort.h"
#include "gaul/ga_randomsearch.h"
#include "gaul/ga_sa.h"
#include "gaul/ga_similarity.h"
#include "gaul/ga_systematicsearch.h"
#include "gaul/ga_simplex.h"
#include "gaul/ga_tabu.h"

/*
 * Compilation constants.
 */
#define GA_BOLTZMANN_FACTOR	(1.38066e-23)
#define GA_TINY_DOUBLE		(1.0e-9)

/*
 * MPI message tags.
 */
#define GA_TAG_NULL			0

#define GA_TAG_NUMENTITIES		101
#define GA_TAG_ENTITYLEN		102
#define GA_TAG_ENTITYFITNESS		103
#define GA_TAG_ENTITYCHROMOSOME		104

#define GA_TAG_POPSTABLESIZE		201
#define GA_TAG_POPCROSSOVER		202
#define GA_TAG_POPMUTATION		203
#define GA_TAG_POPMIGRATION		204
#define GA_TAG_POPALLELEMUTPROB		205
#define GA_TAG_POPALLELEMININT		206
#define GA_TAG_POPALLELEMAXINT		207
#define GA_TAG_POPALLELEMINDOUBLE	208
#define GA_TAG_POPALLELEMAXDOUBLE	209

/*
 * Entity Structure.
 *
 * FIXME: Make opaque i.e. move definition into ga_core.c
 * Should encourage the use of accessor functions rather than directly tweaking
 * the values in this structure manually.
 */
struct entity_t
  {
  double	fitness;	/* Fitness score. */
  vpointer	*chromosome;	/* The chromosomes (the genotype). */
  vpointer	data;		/* User data containing physical properties. (the phenotype) */
  };

/*
 * Tabu-search parameter structure.
 */
typedef struct
  {
  int		list_length;	/* Length of the tabu-list. */
  int		search_count;	/* Number of local searches initiated at each iteration. */
  GAtabu_accept	tabu_accept;	/* Acceptance function. */
  } ga_tabu_t;

/*
 * Simulated Annealling search parameter structure.
 */
typedef struct
  {
  double	initial_temp;	/* Initial temperature. */
  double	final_temp;	/* Final temperature. */
  double	temp_step;	/* Increment of temperature updates. */
  int		temp_freq;	/* Frequency for temperature updates.
				 * (Or, -1 for smooth transition between Ti and Tf) */
  double	temperature;	/* Current temperature. */
  GAsa_accept	sa_accept;	/* Acceptance criterion function. */
  } ga_sa_t;

/*
 * Hill climbing parameter structure.
 */
typedef struct
  {
  GAmutate_allele	mutate_allele;	/* Allele mutation function. */
  } ga_climbing_t;

/*
 * Simplex parameter structure.
 */
typedef struct
  {
  int		dimensions;	/* Size of double array. */
  double	alpha;		/*  (range: 0=no extrap, 1=unit step extrap, higher OK.) */
  double	beta;		/*  (range: 0=no contraction, 1=full contraction.) */
  double	gamma;		/*  (range: 0=no contraction, 1=full contraction.) */
  double	step;		/* Initial randomisation step (range: >0, 1=unit step randomisation, higher OK.) */
  GAto_double	to_double;	/* Convert chromosome to double array. */
  GAfrom_double	from_double;	/* Convert chromosome from double array. */
  } ga_simplex_t;

/*
 * Deterministic crowding parameter structure.
 */
typedef struct
  {
  GAcompare	compare;	/* Compare two entities (either genomic or phenomic space). */
  } ga_dc_t;

/*
 * Differential evolution parameter structure.
 */
typedef struct
  {
  ga_de_strategy_type	strategy;		/* Selection strategy. */
  ga_de_crossover_type	crossover_method;	/* Crossover strategy. */
  int			num_perturbed;		/* Number to perturb. */
  double		crossover_factor;	/* Crossover ratio. */
  double		weighting_min;		/* Minimum crossover weighting factor. */
  double		weighting_max;		/* Maximum crossover weighting factor. */
  } ga_de_t;

/*
 * Gradient methods parameter structure.
 */
typedef struct
  {
  int		dimensions;	/* Size of double array. */
  double	step_size;	/* Step size, (or initial step size). */
  double	alpha;		/* Step size scale-down factor. */
  double	beta;		/* Step size scale-up factor. */
  GAto_double	to_double;	/* Convert chromosome to double array. */
  GAfrom_double	from_double;	/* Convert chromosome from double array. */
  GAgradient	gradient;	/* Return gradients array. */
  } ga_gradient_t;

/*
 * Systematic search parameter structure.
 */
typedef struct
  {
  GAscan_chromosome	scan_chromosome;	/* Allele searching function. */
  int			chromosome_state;	/* Permutation counter. */
  int			allele_state;		/* Permutation counter. */
  } ga_search_t;

/*
 * Probabilistic sampling parameter structure.
 */
typedef struct
  {
  int			**num_states;		/* Number of states for each allele. */
  } ga_sampling_t;

/* 
 * Internal state values for built-in selection operators.
 * Currently used for roulette wheel and SUS selection routines.
 */
typedef struct
  {
  double	mean, stddev, sum;	/* Fitness statistics. */
  double	current_expval;		/* Total of expectancy values. */
  double	minval;			/* Worst fitness value. */
  double	step;			/* Distance between each pointer. */
  double	offset1, offset2;	/* Current pointer offsets. */
  int		marker;			/* The roulette wheel marker. */
  int		num_to_select;		/* Number of individuals to select. */
  int		current1, current2;	/* Currently selected individuals. */
  int		*permutation;		/* Randomly ordered indices. */
  } ga_selectdata_t;


/*
 * Population Structure.
 *
 * FIXME: Make opaque. (I have already written the accessor functions.)
 * IMPORTANT NOTE: If you really must iterate over all entities in
 * a population in external code, loop over entity_iarray... NOT entity_array.
 */
struct population_t
  {
  int		max_size;		/* Current maximum population size. */
  int		stable_size;		/* Requested population size. */
  int		size;			/* Actual population size. */
  int		orig_size;		/* Number of parents (entities at start of generation). */
  int		island;			/* Population's island. */
  int		free_index;		/* Next potentially free entity index. */
  int		generation;		/* For ga_population_get_generation(). */

  MemChunk	*entity_chunk;		/* Buffer for entity structures. */
  entity	**entity_array;		/* The population in id order. */
  entity	**entity_iarray;	/* The population sorted by fitness. */

  int		num_chromosomes;	/* Number of chromosomes in genotype.  FIXME: should be an array of lengths. */
  int		len_chromosomes;	/* Maximum length of each chromosome. */
  
  vpointer	data;			/* User data. */

  int		select_state;		/* Available to selection algorithms. */
  ga_selectdata_t	selectdata;	/* State values for built-in selection operators. */

/*
 * Special parameters for particular built-in GA operators.
 * FIXME: I don't like how this is currently implemented; need a more
 * elegant approach.
 */
  int		allele_min_integer, allele_max_integer;	/* Range for "integer" alleles. */
  double	allele_min_double, allele_max_double;	/* Range for "double" alleles. */

/*
 * Evolutionary parameters.
 */
  double		crossover_ratio;	/* Chance for crossover. */
  double		mutation_ratio;		/* Chance for mutation. */
  double		migration_ratio;	/* Chance for migration. */
  ga_scheme_type	scheme;			/* Evolutionary scheme. */
  ga_elitism_type	elitism;		/* Elitism mode. */

/*
 * Special (aka miscellaneous) parameters.
 */
  double		allele_mutation_prob;	/* Chance for individual alleles to mutate in certain mutation operators. */

/*
 * Non-evolutionary parameters.
 */
  ga_tabu_t		*tabu_params;		/* Parameters for tabu-search. */
  ga_sa_t		*sa_params;		/* Parameters for simulated annealling. */
  ga_climbing_t		*climbing_params;	/* Parameters for hill climbing. */
  ga_simplex_t		*simplex_params;	/* Parameters for simplex search. */
  ga_dc_t		*dc_params;		/* Parameters for deterministic crowding. */
  ga_de_t		*de_params;		/* Parameters for differential evolution. */
  ga_gradient_t		*gradient_params;	/* Parameters for gradient methods. */
  ga_search_t		*search_params;		/* Parameters for systematic search. */
  ga_sampling_t		*sampling_params;	/* Parameters for probabilistic sampling. */

/*
 * The scoring function and the other callbacks are defined here.
 */
  GAgeneration_hook		generation_hook;
  GAiteration_hook		iteration_hook;

  GAdata_destructor		data_destructor;
  GAdata_ref_incrementor	data_ref_incrementor;

  GAchromosome_constructor	chromosome_constructor;
  GAchromosome_destructor	chromosome_destructor;
  GAchromosome_replicate	chromosome_replicate;
  GAchromosome_to_bytes		chromosome_to_bytes;
  GAchromosome_from_bytes	chromosome_from_bytes;
  GAchromosome_to_string	chromosome_to_string;

  GAevaluate			evaluate;
  GAseed			seed;
  GAadapt			adapt;
  GAselect_one			select_one;
  GAselect_two			select_two;
  GAmutate			mutate;
  GAcrossover			crossover;
  GAreplace			replace;
  GArank			rank;

/*
 * Memory handling.
 */
#if USE_CHROMO_CHUNKS == 1
  MemChunk			*chromoarray_chunk;
  MemChunk			*chromo_chunk;
#endif

/*
 * Execution locks.
 */
  THREAD_LOCK_DECLARE(lock);
#if USE_CHROMO_CHUNKS == 1
  THREAD_LOCK_DECLARE(chromo_chunk_lock);
#endif
  };

/*
 * Constant definitions.
 */

/* Define lower bound on fitness. */
#define GA_MIN_FITNESS			-DBL_MAX

/*
 * Define some default values.
 */
#define GA_DEFAULT_CROSSOVER_RATIO	0.9
#define GA_DEFAULT_MUTATION_RATIO	0.1
#define GA_DEFAULT_MIGRATION_RATIO	0.1

/*
 * Define chance of any given allele being mutated in one mutation
 * operation (only for certain mutation functions).
 */
#define GA_DEFAULT_ALLELE_MUTATION_PROB	0.02

/*
 * A private prototype.
 */
boolean gaul_population_fill(population *pop, int num);

#endif	/* GA_CORE_H_INCLUDED */

