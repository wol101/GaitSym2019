/**********************************************************************
  gaul.h
 **********************************************************************

  gaul - Genetic Algorithm Utility Library.
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

  Synopsis:	Public header file for GAUL.

		This file should be included by any code that will
		be linking to libgaul.

 **********************************************************************/

#ifndef GAUL_H_INCLUDED
#define GAUL_H_INCLUDED

/**********************************************************************
 * Include requisite headers.
 **********************************************************************/

#include "gaul/gaul_util.h"	/* General header containing commonly
				   used convenience definitions.
				   This also includes a platform-
				   specific configuration file. */

/*
 * Portable programming utilities.
 */
#include "gaul/compatibility.h"      /* For portability stuff. */
#include "gaul/linkedlist.h"         /* For linked lists. */
#include "gaul/log_util.h"           /* For logging facilities. */
#include "gaul/memory_util.h"        /* Memory handling. */
#include "gaul/random_util.h"        /* For PRNGs. */
#include "gaul/table_util.h"         /* Handling unique integer ids. */


/**********************************************************************
 * Forward declarations.
 **********************************************************************/

/* The entity datatype stores single individuals. */
typedef struct entity_t entity;
/* The population datatype stores single populations. */
typedef struct population_t population;

/**********************************************************************
 * Enumerated types, used to define varients of the GA algorithms.
 **********************************************************************/

/*
 * Evolutionary mode.
 */
typedef enum ga_scheme_type_t
  {
  GA_SCHEME_DARWIN = 0,
  GA_SCHEME_LAMARCK_PARENTS = 1,
  GA_SCHEME_LAMARCK_CHILDREN = 2,
  GA_SCHEME_LAMARCK_ALL = 3,
  GA_SCHEME_BALDWIN_PARENTS = 4,
  GA_SCHEME_BALDWIN_CHILDREN = 8,
  GA_SCHEME_BALDWIN_ALL = 12
  } ga_scheme_type;

/*
 * Elitism mode.
 */
typedef enum ga_elitism_type_t
  {
  GA_ELITISM_UNKNOWN = 0,
  GA_ELITISM_NULL = 0,
  GA_ELITISM_PARENTS_SURVIVE = 1,
  GA_ELITISM_ONE_PARENT_SURVIVES = 2,
  GA_ELITISM_PARENTS_DIE = 3,
  GA_ELITISM_RESCORE_PARENTS = 4
  } ga_elitism_type;

/*
 * Stategies available with Differential Evolution implementation.
 */
typedef enum de_strategy_t
  {
  GA_DE_STRATEGY_UNKNOWN = 0,
  GA_DE_STRATEGY_BEST = 1,
  GA_DE_STRATEGY_RAND = 2,
  GA_DE_STRATEGY_RANDTOBEST = 3
  } ga_de_strategy_type;

typedef enum de_crossover_t
  {
  GA_DE_CROSSOVER_UNKNOWN = 0,
  GA_DE_CROSSOVER_BINOMIAL = 1,
  GA_DE_CROSSOVER_EXPONENTIAL = 2
  } ga_de_crossover_type;

/**********************************************************************
 * Callback function typedefs.
 **********************************************************************/
/*
 * Analysis and termination.
 */
/* GAgeneration_hook is called at the beginning of each generation by
 * all evolutionary functions. */
typedef boolean (*GAgeneration_hook)(const int generation, population *pop);
/* GAiteration_hook is called at the beginning of each iteration by
 * all non-evolutionary functions. */
typedef boolean (*GAiteration_hook)(const int iteration, entity *entity);

/*
 * Phenome (A general purpose data cache) handling.
 */
/* GAdata_destructor is used to deallocate phenomic data. */
typedef void    (*GAdata_destructor)(vpointer data);
/* GAdata_ref_incrementor is used for reference counting of phenomic data. */
typedef void    (*GAdata_ref_incrementor)(vpointer data);

/*
 * Genome handling.
 */
/* GAchromosome_constructor is used to allocate single chromosomes. */
typedef boolean (*GAchromosome_constructor)(population *pop, entity *entity);
/* GAchromosome_destructor is used to deallocate single chromosomes. */
typedef void    (*GAchromosome_destructor)(population *pop, entity *entity);
/* GAchromosome_replicate is used to clone single chromosomes. */
typedef void    (*GAchromosome_replicate)(const population *pop, entity *parent, entity *child, const int chromosomeid);
/* GAchromosome_to_bytes is used to pack genomic data into a
 * contiguous block of memory. */
typedef unsigned int    (*GAchromosome_to_bytes)(const population *pop, entity *joe, byte **bytes, unsigned int *max_bytes);
/* GAchromosome_from_bytes is used to unpack genomic data from a
 * contiguous block of memory. */
typedef void    (*GAchromosome_from_bytes)(const population *pop, entity *joe, byte *bytes);
/* GAchromosome_to_string is used to generate a human readable
 * representation of genomic data. */
typedef char    *(*GAchromosome_to_string)(const population *pop, const entity *joe, char *text, size_t *textlen);

/*
 * GA operations.
 *
 * FIXME: Adaptation prototype should match the mutation prototype so that
 * the adaptation local optimisation algorithms may be used as mutation
 * operators.
 */
/* GAevaluate determines the fitness of an entity. */
typedef boolean (*GAevaluate)(population *pop, entity *entity);
/* GAseed initialises the genomic contents of an entity. */
typedef boolean	(*GAseed)(population *pop, entity *adam);
/* GAadapt optimises/performs learning for an entity. */
typedef entity *(*GAadapt)(population *pop, entity *child);
/* GAselect_one selects a single entity from the population. */
typedef boolean (*GAselect_one)(population *pop, entity **mother);
/* GAselect_two selects a pair of entities from the population. */
typedef boolean (*GAselect_two)(population *pop, entity **mother, entity **father);
/* GAmutate introduces a mutation into an entity. */
typedef void    (*GAmutate)(population *pop, entity *mother, entity *daughter);
/* GAcrossover produces two new sets of chromosomes from two parent sets. */
typedef void    (*GAcrossover)(population *pop, entity *mother, entity *father, entity *daughter, entity *son);
/* GAreplace inserts a new entity into the population. */
typedef void    (*GAreplace)(population *pop, entity *child);
/* GArank Compare two entities and return 1, 0, or -1, if alpha should rank higher,
 * they should have equal rank, or beta should rank higher. */
typedef int	(*GArank)(population *alphapop, entity *alpha, population *betapop, entity *beta);

/*
 * Alternative heuristic search function operations.
 * 
 * GAtabu_accept     - Tabu-search tabu+aspiration criteria.
 * GAsa_accept       - Simulated Annealing acceptance criteria.
 * GAmutate_allele   - Mutate a single, specified, allele.
 * GAto_double       - Map chromosomal data to double-precision float array.
 * GAfrom_double     - Map chromosomal data from double-precision float array.
 * GAgradient        - Return array of gradients.
 * GAscan_chromosome - Produce next permutation of genome.
 * GAcompare         - Compare two entities and return distance.
 */
typedef boolean	(*GAtabu_accept)(population *pop, entity *putative, entity *tabu);
typedef boolean	(*GAsa_accept)(population *pop, entity *current, entity *trial);
typedef boolean	(*GAmutate_allele)(population *pop, entity *parent, entity *child, const int chromosomeid, const int alleleid);
typedef boolean	(*GAto_double)(population *pop, entity *entity, double *darray);
typedef boolean	(*GAfrom_double)(population *pop, entity *entity, double *darray);
typedef double	(*GAgradient)(population *pop, entity *entity, double *darray, double *varray);
typedef boolean	(*GAscan_chromosome)(population *pop, entity *entity, int enumeration_num);
typedef double	(*GAcompare)(population *pop, entity *alpha, entity *beta);

/**********************************************************************
 * Public prototypes.
 **********************************************************************/

/*
 * Functions located in ga_core.c:
 * (Basic entity and population handling)
 */
FUNCPROTO population *ga_population_new(	const int stable_size,
				const int num_chromosome,
				const int len_chromosome);
FUNCPROTO population *ga_population_clone_empty( population *pop );
FUNCPROTO population *ga_population_clone( population *pop );
FUNCPROTO int	ga_get_num_populations(void);
FUNCPROTO population *ga_get_population_from_id(unsigned int id);
FUNCPROTO unsigned int ga_get_population_id(population *pop);
FUNCPROTO unsigned int *ga_get_all_population_ids(void);
FUNCPROTO population **ga_get_all_populations(void);
FUNCPROTO boolean	ga_entity_seed(population *pop, entity *e);
FUNCPROTO boolean ga_population_seed(population *pop);
FUNCPROTO double	ga_entity_evaluate(population *pop, entity *entity);
FUNCPROTO boolean	ga_population_score_and_sort(population *pop);
FUNCPROTO boolean	ga_population_sort(population *pop);
FUNCPROTO int ga_get_entity_rank(population *pop, entity *e);
FUNCPROTO int ga_get_entity_id(population *pop, entity *e);
FUNCPROTO entity *ga_get_entity_from_id(population *pop, const unsigned int id);
FUNCPROTO entity *ga_get_entity_from_rank(population *pop, const unsigned int rank);
FUNCPROTO int	ga_get_entity_rank_from_id(population *pop, int id);
FUNCPROTO int	ga_get_entity_id_from_rank(population *pop, int rank);
FUNCPROTO boolean	ga_entity_dereference_by_rank(population *pop, int rank);
FUNCPROTO boolean ga_entity_dereference(population *p, entity *dying);
FUNCPROTO boolean ga_entity_dereference_by_id(population *pop, int id);
FUNCPROTO void ga_entity_clear_data(population *p, entity *entity, const int chromosome);
FUNCPROTO void ga_entity_blank(population *p, entity *entity);
FUNCPROTO entity *ga_get_free_entity(population *pop);
FUNCPROTO boolean ga_copy_data(population *pop, entity *dest, entity *src, const int chromosome);
FUNCPROTO boolean ga_entity_copy_all_chromosomes(population *pop, entity *dest, entity *src);
FUNCPROTO boolean ga_entity_copy_chromosome(population *pop, entity *dest, entity *src, int chromo);
FUNCPROTO boolean ga_entity_copy(population *pop, entity *dest, entity *src);
FUNCPROTO entity	*ga_entity_clone(population *pop, entity *parent);

FUNCPROTO void ga_population_send_by_mask( population *pop, int dest_node, int num_to_send, boolean *send_mask );
FUNCPROTO void ga_population_send_every( population *pop, int dest_node );
FUNCPROTO void ga_population_append_receive( population *pop, int src_node );
FUNCPROTO population *ga_population_new_receive( int src_node );
FUNCPROTO population *ga_population_receive( int src_node );
FUNCPROTO void ga_population_send( population *pop, int dest_node );
FUNCPROTO void ga_population_send_all( population *pop, int dest_node );

FUNCPROTO entity	*ga_optimise_entity(population *pop, entity *unopt);
FUNCPROTO void	ga_population_set_parameters(  population            *pop,
		                       const ga_scheme_type  scheme,
		                       const ga_elitism_type elitism,
		                       const double          crossover,
		                       const double          mutation,
		                       const double          migration);
FUNCPROTO void	ga_population_set_scheme(      population            *pop,
		                       const ga_scheme_type  scheme);
FUNCPROTO void	ga_population_set_elitism(     population            *pop,
		                       const ga_elitism_type elitism);
FUNCPROTO void	ga_population_set_crossover(   population            *pop,
		                       const double          crossover);
FUNCPROTO void	ga_population_set_mutation(    population            *pop,
		                       const double          mutation);
FUNCPROTO void	ga_population_set_migration(   population            *pop,
		                       const double          migration);
FUNCPROTO void	ga_population_set_allele_mutation_prob(   population            *pop,
		                       const double          prob);
FUNCPROTO void	ga_population_set_allele_min_integer(   population            *pop,
		                       const int          value);
FUNCPROTO void	ga_population_set_allele_max_integer(   population            *pop,
		                       const int          value);
FUNCPROTO void	ga_population_set_allele_min_double(   population            *pop,
		                       const double          value);
FUNCPROTO void	ga_population_set_allele_max_double(   population            *pop,
		                       const double          value);
FUNCPROTO double ga_population_get_crossover(population       *pop);
FUNCPROTO double ga_population_get_mutation(population       *pop);
FUNCPROTO double ga_population_get_migration(population       *pop);
FUNCPROTO double ga_population_get_allele_mutation_prob(population       *pop);
FUNCPROTO int ga_population_get_allele_min_integer(population       *pop);
FUNCPROTO int ga_population_get_allele_max_integer(population       *pop);
FUNCPROTO double ga_population_get_allele_min_double(population       *pop);
FUNCPROTO double ga_population_get_allele_max_double(population       *pop);
FUNCPROTO ga_scheme_type ga_population_get_scheme(population       *pop);
FUNCPROTO ga_elitism_type ga_population_get_elitism(population       *pop);
FUNCPROTO population *ga_transcend(unsigned int id);
FUNCPROTO unsigned int ga_resurect(population *pop);
FUNCPROTO boolean ga_extinction(population *extinct);
FUNCPROTO boolean ga_genocide(population *pop, int target_size);
FUNCPROTO boolean ga_genocide_by_fitness(population *pop, double target_fitness);
FUNCPROTO boolean ga_population_set_data(population *pop, vpointer data);
FUNCPROTO vpointer ga_population_get_data(population *pop);
FUNCPROTO boolean ga_entity_set_data(population *pop, entity *e, SLList *data);
FUNCPROTO SLList	*ga_entity_get_data(population *pop, entity *e);
FUNCPROTO int	ga_population_get_generation(population *pop);

FUNCPROTO double	ga_entity_get_fitness(entity *e);
FUNCPROTO boolean	ga_entity_set_fitness(entity *e, double fitness);
FUNCPROTO int	ga_population_get_stablesize(population *pop);
FUNCPROTO int	ga_population_get_size(population *pop);
FUNCPROTO int	ga_population_get_maxsize(population *pop);
FUNCPROTO boolean	ga_population_set_stablesize(population *pop, int stable_size);

FUNCPROTO int	ga_funclookup_ptr_to_id(void *func);
FUNCPROTO int	ga_funclookup_label_to_id(char *funcname);
FUNCPROTO void	*ga_funclookup_label_to_ptr(char *funcname);
FUNCPROTO void	*ga_funclookup_id_to_ptr(int id);
FUNCPROTO const char *ga_funclookup_id_to_label(int id);
FUNCPROTO void	ga_init_openmp( void );

/*
 * Functions located in ga_io.c:
 * (Disk I/O)
 */
FUNCPROTO boolean ga_population_write(population *pop, char *fname);
FUNCPROTO population *ga_population_read(char *fname);
FUNCPROTO boolean ga_entity_write(population *pop, entity *entity, char *fname);
FUNCPROTO entity *ga_entity_read(population *pop, char *fname);

/*
 * Functions located in ga_select.c:
 * (Selection operators)
 */
FUNCPROTO boolean ga_select_one_random(population *pop, entity **mother);
FUNCPROTO boolean ga_select_two_random(population *pop, entity **mother, entity **father);
FUNCPROTO boolean ga_select_one_every(population *pop, entity **mother);
FUNCPROTO boolean ga_select_two_every(population *pop, entity **mother, entity **father);
FUNCPROTO boolean	ga_select_one_randomrank(population *pop, entity **mother);
FUNCPROTO boolean ga_select_two_randomrank(population *pop, entity **mother, entity **father);
FUNCPROTO boolean ga_select_one_bestof2(population *pop, entity **mother);
FUNCPROTO boolean ga_select_two_bestof2(population *pop, entity **mother, entity **father);
FUNCPROTO boolean ga_select_one_bestof3(population *pop, entity **mother);
FUNCPROTO boolean ga_select_two_bestof3(population *pop, entity **mother, entity **father);
FUNCPROTO boolean	ga_select_one_roulette( population *pop, entity **mother );
FUNCPROTO boolean	ga_select_two_roulette( population *pop, entity **mother, entity **father );
FUNCPROTO boolean	ga_select_one_roulette_rebased( population *pop, entity **mother );
FUNCPROTO boolean	ga_select_two_roulette_rebased( population *pop, entity **mother, entity **father );
FUNCPROTO boolean	ga_select_one_sus( population *pop, entity **mother );
FUNCPROTO boolean	ga_select_two_sus( population *pop, entity **mother, entity **father );
FUNCPROTO boolean	ga_select_one_sussq( population *pop, entity **mother );
FUNCPROTO boolean	ga_select_two_sussq( population *pop, entity **mother, entity **father );
FUNCPROTO boolean	ga_select_one_aggressive( population *pop, entity **mother );
FUNCPROTO boolean	ga_select_two_aggressive( population *pop, entity **mother, entity **father );
FUNCPROTO boolean	ga_select_one_best( population *pop, entity **mother );
FUNCPROTO boolean	ga_select_two_best( population *pop, entity **mother, entity **father );
FUNCPROTO boolean ga_select_one_linearrank( population *pop, entity **mother );
FUNCPROTO boolean ga_select_two_linearrank( population *pop, entity **mother, entity **father );
FUNCPROTO boolean ga_select_one_roundrobin( population *pop, entity **mother );

/*
 * Functions located in ga_crossover.c:
 * (Crossover operators)
 */
FUNCPROTO void	ga_crossover_integer_singlepoints(population *pop, entity *father, entity *mother, entity *son, entity *daughter);
FUNCPROTO void	ga_crossover_integer_doublepoints(population *pop, entity *father, entity *mother, entity *son, entity *daughter);
FUNCPROTO void	ga_crossover_integer_mean(population *pop, entity *father, entity *mother, entity *son, entity *daughter);
FUNCPROTO void	ga_crossover_integer_mixing(population *pop, entity *father, entity *mother, entity *son, entity *daughter);
FUNCPROTO void	ga_crossover_integer_allele_mixing( population *pop,
                                 entity *father, entity *mother,
                                  entity *son, entity *daughter );
FUNCPROTO void	ga_crossover_boolean_singlepoints(population *pop, entity *father, entity *mother, entity *son, entity *daughter);
FUNCPROTO void	ga_crossover_boolean_doublepoints(population *pop, entity *father, entity *mother, entity *son, entity *daughter);
FUNCPROTO void	ga_crossover_boolean_mixing(population *pop, entity *father, entity *mother, entity *son, entity *daughter);
FUNCPROTO void	ga_crossover_boolean_allele_mixing( population *pop,
                                 entity *father, entity *mother,
                                  entity *son, entity *daughter );
FUNCPROTO void	ga_crossover_char_singlepoints( population *pop,
                                     entity *father, entity *mother,
                                          entity *son, entity *daughter );
FUNCPROTO void	ga_crossover_char_doublepoints( population *pop,
                                    entity *father, entity *mother,
                                    entity *son, entity *daughter );
FUNCPROTO void	ga_crossover_char_mixing(population *pop, entity *father, entity *mother, entity *son, entity *daughter);
FUNCPROTO void	ga_crossover_char_allele_mixing( population *pop,
	                                 entity *father, entity *mother,
                                  entity *son, entity *daughter );
FUNCPROTO void	ga_crossover_double_singlepoints( population *pop,
                                       entity *father, entity *mother,
                                       entity *son, entity *daughter );
FUNCPROTO void	ga_crossover_double_doublepoints( population *pop,
                                        entity *father, entity *mother,
                                       entity *son, entity *daughter );
FUNCPROTO void	ga_crossover_double_mixing(population *pop, entity *father, entity *mother, entity *son, entity *daughter);
FUNCPROTO void	ga_crossover_double_mean(population *pop, entity *father, entity *mother, entity *son, entity *daughter);
FUNCPROTO void	ga_crossover_double_allele_mixing( population *pop,
                                entity *father, entity *mother,
                                 entity *son, entity *daughter );
FUNCPROTO void	ga_crossover_bitstring_singlepoints(population *pop, entity *father, entity *mother, entity *son, entity *daughter);
FUNCPROTO void	ga_crossover_bitstring_doublepoints(population *pop, entity *father, entity *mother, entity *son, entity *daughter);
FUNCPROTO void	ga_crossover_bitstring_mixing(population *pop, entity *father, entity *mother, entity *son, entity *daughter);
FUNCPROTO void	ga_crossover_bitstring_allele_mixing( population *pop,
                                entity *father, entity *mother,
                                entity *son, entity *daughter );

/*
 * Functions located in ga_mutate.c:
 * (Mutation operators)
 */
FUNCPROTO void	ga_mutate_integer_singlepoint_drift(population *pop, entity *father, entity *son);
FUNCPROTO void	ga_mutate_integer_singlepoint_randomize(population *pop, entity *father, entity *son);
FUNCPROTO void	ga_mutate_integer_multipoint(population *pop, entity *father, entity *son);
FUNCPROTO void	ga_mutate_integer_allpoint(population *pop, entity *father, entity *son);
FUNCPROTO void	ga_mutate_boolean_singlepoint(population *pop, entity *father, entity *son);
FUNCPROTO void	ga_mutate_boolean_multipoint(population *pop, entity *father, entity *son);
FUNCPROTO void	ga_mutate_char_singlepoint_drift(population *pop, entity *father, entity *son);
FUNCPROTO void	ga_mutate_char_singlepoint_randomize(population *pop, entity *father, entity *son);
FUNCPROTO void	ga_mutate_char_allpoint(population *pop, entity *father, entity *son);
FUNCPROTO void	ga_mutate_char_multipoint(population *pop, entity *father, entity *son);
FUNCPROTO void	ga_mutate_printable_singlepoint_drift(population *pop, entity *father, entity *son);
FUNCPROTO void	ga_mutate_printable_singlepoint_randomize(population *pop, entity *father, entity *son);
FUNCPROTO void	ga_mutate_printable_allpoint(population *pop, entity *father, entity *son);
FUNCPROTO void	ga_mutate_printable_multipoint(population *pop, entity *father, entity *son);
FUNCPROTO void	ga_mutate_bitstring_singlepoint(population *pop, entity *father, entity *son);
FUNCPROTO void	ga_mutate_bitstring_multipoint(population *pop, entity *father, entity *son);
FUNCPROTO void	ga_mutate_double_singlepoint_drift( population *pop,
                                          entity *father, entity *son );
FUNCPROTO void	ga_mutate_double_singlepoint_randomize( population *pop,
                                              entity *father, entity *son );
FUNCPROTO void	ga_mutate_double_multipoint(population *pop, entity *father, entity *son);
FUNCPROTO void	ga_mutate_double_allpoint(population *pop, entity *father, entity *son);

/*
 * Functions located in ga_seed.c:
 * (Genesis operators)
 */
FUNCPROTO boolean	ga_seed_boolean_random(population *pop, entity *adam);
FUNCPROTO boolean ga_seed_boolean_zero(population *pop, entity *adam);
FUNCPROTO boolean	ga_seed_integer_random(population *pop, entity *adam);
FUNCPROTO boolean	ga_seed_integer_zero(population *pop, entity *adam);
FUNCPROTO boolean	ga_seed_double_random(population *pop, entity *adam);
FUNCPROTO boolean	ga_seed_double_zero(population *pop, entity *adam);
FUNCPROTO boolean	ga_seed_double_random_unit_gaussian(population *pop, entity *adam);
FUNCPROTO boolean	ga_seed_char_random(population *pop, entity *adam);
FUNCPROTO boolean	ga_seed_printable_random(population *pop, entity *adam);
FUNCPROTO boolean	ga_seed_bitstring_random(population *pop, entity *adam);
FUNCPROTO boolean	ga_seed_bitstring_zero(population *pop, entity *adam);

/*
 * Functions located in ga_replace.c:
 * (Replacement operators)
 */
FUNCPROTO void	ga_replace_by_fitness(population *pop, entity *child);

/*
 * Functions located in ga_utility.c:
 * (Miscellaneous support functions)
 */
FUNCPROTO void    ga_diagnostics( void ); 
FUNCPROTO int	ga_get_major_version( void );
FUNCPROTO int	ga_get_minor_version( void );
FUNCPROTO int	ga_get_patch_version( void );
/* ga_genesis() is deprecated! Use ga_genesis_integer() instead. */
FUNCPROTO population *ga_genesis( const int               population_size,
                        const int               num_chromo,
                        const int               len_chromo,
                        GAgeneration_hook       generation_hook,
                        GAiteration_hook        iteration_hook,
                        GAdata_destructor       data_destructor,
                        GAdata_ref_incrementor  data_ref_incrementor,
                        GAevaluate              evaluate,
                        GAseed                  seed,
                        GAadapt                 adapt,
                        GAselect_one            select_one,
                        GAselect_two            select_two,
                        GAmutate                mutate,
                        GAcrossover             crossover,
                        GAreplace               replace,
			vpointer		userdata );
/* ga_genesis_int() is deprecated! Use ga_genesis_integer() instead. */
FUNCPROTO population *ga_genesis_int( const int           population_size,
                        const int               num_chromo,
                        const int               len_chromo,
                        GAgeneration_hook       generation_hook,
                        GAiteration_hook        iteration_hook,
                        GAdata_destructor       data_destructor,
                        GAdata_ref_incrementor  data_ref_incrementor,
                        GAevaluate              evaluate,
                        GAseed                  seed,
                        GAadapt                 adapt,
                        GAselect_one            select_one,
                        GAselect_two            select_two,
                        GAmutate                mutate,
                        GAcrossover             crossover,
                        GAreplace               replace,
			vpointer		userdata );
FUNCPROTO population *ga_genesis_integer( const int           population_size,
                        const int               num_chromo,
                        const int               len_chromo,
                        GAgeneration_hook       generation_hook,
                        GAiteration_hook        iteration_hook,
                        GAdata_destructor       data_destructor,
                        GAdata_ref_incrementor  data_ref_incrementor,
                        GAevaluate              evaluate,
                        GAseed                  seed,
                        GAadapt                 adapt,
                        GAselect_one            select_one,
                        GAselect_two            select_two,
                        GAmutate                mutate,
                        GAcrossover             crossover,
                        GAreplace               replace,
			vpointer		userdata );
FUNCPROTO population *ga_genesis_boolean( const int               population_size,
                        const int               num_chromo,
                        const int               len_chromo,
                        GAgeneration_hook       generation_hook,
                        GAiteration_hook        iteration_hook,
                        GAdata_destructor       data_destructor,
                        GAdata_ref_incrementor  data_ref_incrementor,
                        GAevaluate              evaluate,
                        GAseed                  seed,
                        GAadapt                 adapt,
                        GAselect_one            select_one,
                        GAselect_two            select_two,
                        GAmutate                mutate,
                        GAcrossover             crossover,
                        GAreplace               replace,
			vpointer		userdata );
FUNCPROTO population *ga_genesis_char( const int               population_size,
                        const int               num_chromo,
                        const int               len_chromo,
                        GAgeneration_hook       generation_hook,
                        GAiteration_hook        iteration_hook,
                        GAdata_destructor       data_destructor,
                        GAdata_ref_incrementor  data_ref_incrementor,
                        GAevaluate              evaluate,
                        GAseed                  seed,
                        GAadapt                 adapt,
                        GAselect_one            select_one,
                        GAselect_two            select_two,
                        GAmutate                mutate,
                        GAcrossover             crossover,
                        GAreplace               replace,
			vpointer		userdata );
FUNCPROTO population *ga_genesis_double( const int               population_size,
                        const int               num_chromo,
                        const int               len_chromo,
                        GAgeneration_hook       generation_hook,
                        GAiteration_hook        iteration_hook,
                        GAdata_destructor       data_destructor,
                        GAdata_ref_incrementor  data_ref_incrementor,
                        GAevaluate              evaluate,
                        GAseed                  seed,
                        GAadapt                 adapt,
                        GAselect_one            select_one,
                        GAselect_two            select_two,
                        GAmutate                mutate,
                        GAcrossover             crossover,
                        GAreplace               replace,
			vpointer		userdata );
FUNCPROTO population *ga_genesis_bitstring( const int               population_size,
                        const int               num_chromo,
                        const int               len_chromo,
                        GAgeneration_hook       generation_hook,
                        GAiteration_hook        iteration_hook,
                        GAdata_destructor       data_destructor,
                        GAdata_ref_incrementor  data_ref_incrementor,
                        GAevaluate              evaluate,
                        GAseed                  seed,
                        GAadapt                 adapt,
                        GAselect_one            select_one,
                        GAselect_two            select_two,
                        GAmutate                mutate,
                        GAcrossover             crossover,
                        GAreplace               replace,
			vpointer		userdata );
FUNCPROTO entity  *ga_allele_search(      population      *pop,
                                const int       chromosomeid,
                                const int       point,
                                const int       min_val, 
                                const int       max_val, 
                                entity          *initial );
FUNCPROTO void ga_population_dump(population *pop);
FUNCPROTO void ga_entity_dump(population *pop, entity *john);

/*
 * Functions located in ga_stats.c:
 * (Statistics functions)
 */
FUNCPROTO boolean ga_fitness_mean( population *pop, double *average );
FUNCPROTO boolean ga_fitness_mean_stddev( population *pop,
                             double *average, double *stddev );
FUNCPROTO boolean ga_fitness_stats( population *pop,
                          double *max, double *min,
                          double *mean, double *median,
                          double *variance, double *stddev,
                          double *kurtosis, double *skew );

/*
 * Functions located in ga_compare.c:
 * (Entity comparison functions)
 */
FUNCPROTO double ga_compare_char_hamming(population *pop, entity *alpha, entity *beta);
FUNCPROTO double ga_compare_char_euclidean(population *pop, entity *alpha, entity *beta);
FUNCPROTO double ga_compare_integer_hamming(population *pop, entity *alpha, entity *beta);
FUNCPROTO double ga_compare_integer_euclidean(population *pop, entity *alpha, entity *beta);
FUNCPROTO double ga_compare_double_hamming(population *pop, entity *alpha, entity *beta);
FUNCPROTO double ga_compare_double_euclidean(population *pop, entity *alpha, entity *beta);
FUNCPROTO double ga_compare_boolean_hamming(population *pop, entity *alpha, entity *beta);
FUNCPROTO double ga_compare_boolean_euclidean(population *pop, entity *alpha, entity *beta);
FUNCPROTO double ga_compare_bitstring_hamming(population *pop, entity *alpha, entity *beta);
FUNCPROTO double ga_compare_bitstring_euclidean(population *pop, entity *alpha, entity *beta);

/*
 * Functions located in ga_rank.c:
 * (Entity comparison functions)
 */
FUNCPROTO int ga_rank_fitness(population *alphapop, entity *alpha, population *betapop, entity *beta);

/**********************************************************************
 * Include remainder of this library's headers.
 * These should, mostly, contain private definitions etc.
 * But they currently contain almost everything.
 **********************************************************************/

#include "gaul/ga_core.h"		/* Private aspects of GAUL. */

#if HAVE_SLANG==1
#include "gaul/ga_intrinsics.h"         /* GAUL's S-Lang interface. */
#endif

#endif	/* GAUL_H_INCLUDED */

