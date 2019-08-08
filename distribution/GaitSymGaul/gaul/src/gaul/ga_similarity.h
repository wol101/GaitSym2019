/**********************************************************************
  ga_similarity.h
 **********************************************************************

  ga_similarity - Genetic algorithm genome comparison routines.
  Copyright ©2001-2003, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:     Routines for comparing genomes/chromosomes.

 **********************************************************************/

#ifndef GA_SIMILARITY_H_INCLUDED
#define GA_SIMILARITY_H_INCLUDED

/*
 * Includes
 */
#include "gaul.h"

/*
 * Deprecated Prototypes
 */
boolean	ga_compare_genome(population *pop, entity *alpha, entity *beta);
double	ga_genome_euclidean_similarity(population *pop, entity *alpha, entity *beta);
int	ga_count_match_alleles(const int length, const int *alpha, const int *beta);
double	ga_genome_hamming_similarity(population *pop, entity *alpha, entity *beta);

/*
 * Prototypes.
 */
FUNCPROTO double	ga_similarity_bitstring_tanimoto(const population *pop,
                                  const entity *alpha, const entity *beta);
FUNCPROTO double	ga_similarity_bitstring_dice(const population *pop,
                                  const entity *alpha, const entity *beta);
FUNCPROTO double	ga_similarity_bitstring_hamming(const population *pop,
                                  const entity *alpha, const entity *beta);
FUNCPROTO double	ga_similarity_bitstring_euclidean(const population *pop,
                                  const entity *alpha, const entity *beta);
FUNCPROTO double	ga_similarity_bitstring_cosine(const population *pop,
                                  const entity *alpha, const entity *beta);

FUNCPROTO double	ga_similarity_double_tanimoto(const population *pop,
                                  const entity *alpha, const entity *beta);
FUNCPROTO double	ga_similarity_double_dice(const population *pop,
                                      const entity *alpha, const entity *beta);
FUNCPROTO double	ga_similarity_double_cosine(const population *pop,
                                      const entity *alpha, const entity *beta);

#endif	/* GA_SIMILARITY_H_INCLUDED */
