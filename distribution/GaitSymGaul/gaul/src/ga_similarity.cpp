/**********************************************************************
  ga_similarity.c
 **********************************************************************

  ga_similarity - Genetic algorithm genome/chromosome comparison routines.
  Copyright ©2001-2004, Stewart Adcock <stewart@linux-domain.com>
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

		Definitions:

		I define a pair of alleles to be matching if they are
		identical and not matching if the are different - even
		if the differing values are equivalent (e.g. if the
		modulus is significant rather than the actual value.)

		It is probably best not to use the following functions,
		i.e. they should be deprecated and are likely to be
		deleted soon:
		boolean ga_compare_genome()
		int ga_count_match_alleles()
		double ga_genome_hamming_similarity()
		double ga_genome_euclidian_similarity()

  References:	A general reference that I've found useful in the past
		but maybe is tricky to obtain, but is:
		Bradshaw J., "Introduction to Tversky similarity
		measure", MUG '97 - 11th annual Daylight user group
		meeting.

		Interested parties are additionally directed toward
		the documentation:
		http://gaul.sourceforge.net/similarity.pdf

  To do:	Equivalent functions for the other chromosome types.

 **********************************************************************/

#include "gaul/ga_core.h"

/**********************************************************************
  ga_compare_genome()
  synopsis:	Compares two genotypes.  Simple memcmp() comparison of
		the chromosomes.
  parameters:	population *pop	Population of entities (you may use
			differing populations if they are "compatible")
		entity *alpha	Test entity.
		entity *beta	Test entity.
  return:	Returns TRUE if all chromosomes are identical.
  last updated:	19/12/00
 **********************************************************************/

#ifdef COMPILE_DEPRECATED_FUNCTIONS
boolean ga_compare_genome(population *pop, entity *alpha, entity *beta)
  {
  int		i;		/* Loop variable over all chromosomes. */

  /* Checks */
  if (!alpha || !beta) die("Null pointer to entity structure passed");

  for (i=0; i<pop->num_chromosomes; i++)
    {
/*
 * FIXME: Here is a kludge.
 * This version is more efficient, but invalid when chromosomes have
 * differing lengths.  When differing length chromosomes are properly handled
 * this code should be changed.
*/
    if ( memcmp( alpha->chromosome[i], beta->chromosome[i], pop->len_chromosomes*sizeof(int) ) !=0 )
      return FALSE;
    }

  return TRUE;
  }
#endif


/**********************************************************************
  ga_count_match_alleles()
  synopsis:	Compares two chromosomes and counts matching alleles.
  parameters:	const int length	Chromosome length.
		const int *alpha	Alpha chromosome.
		const int *beta		Beta chromosome.
  return:	Returns number of matching alleles.
  last updated:	13/02/01
 **********************************************************************/

#ifdef COMPILE_DEPRECATED_FUNCTIONS

int ga_count_match_alleles(const int length, const int *alpha, const int *beta)
  {
  int		i;		/* Loop variable over all alleles. */
  int		count=0;	/* Number of matching alleles. */

  /* Checks. */
  if (!alpha || !beta) die("Null pointer to chromosome passed");

  for (i=0; i<length; i++)
    if (alpha[i] == beta[i]) count++;

  return count;
  }
#endif


/**********************************************************************
  ga_genome_hamming_similarity()
  synopsis:	Measures the Hamming coefficient of the genomes of
		a pair of entities.
  parameters:	population *pop	Population of entities (you may use
			differing populations if they are "compatible")
		entity *alpha	Test entity.
		entity *beta	Test entity.
  return:	double	similarity
  last updated:	13/02/01
 **********************************************************************/

#ifdef COMPILE_DEPRECATED_FUNCTIONS

double ga_genome_hamming_similarity(population *pop, entity *alpha, entity *beta)
  {
  int		i;			/* Loop over chromosomes. */
  int		match=0;		/* Count of matching alleles. */
  int		length, total_length=0;	/* Number of alleles. */

  /* Checks */
  if (!alpha || !beta) die("Null pointer to entity structure passed");

  for (i=0; i<pop->num_chromosomes; i++)
    {
/*
 * FIXME: Here is a kludge.
 * This version is more efficient, but invalid when chromosomes have
 * differing lengths.  When differing length chromosomes are properly handled
 * this code should be changed.
*/
    length = pop->len_chromosomes;

    total_length += length;
    match += ga_count_match_alleles(length, alpha->chromosome[i], beta->chromosome[i]);
    }

  return (double) match/total_length;
  }
#endif


/**********************************************************************
  ga_genome_euclidian_similarity()
  synopsis:	Measures the Euclidean coefficient of the genomes of
		a pair of entities.
  parameters:	population *pop	Population of entities (you may use
			differing populations if they are "compatible")
		entity *alpha	Test entity.
		entity *beta	Test entity.
  return:	double	similarity
  last updated:	13/02/01
 **********************************************************************/

#ifdef COMPILE_DEPRECATED_FUNCTIONS

double ga_genome_euclidian_similarity(population *pop, entity *alpha, entity *beta)
  {
  return sqrt(ga_genome_hamming_similarity(pop, alpha, beta));
  }
#endif


/**********************************************************************
  ga_similarity_bitstring_count_1_alleles()
  synopsis:	Counts "1"s in a bitstring chromosome.
  parameters:	const population *pop	Population.
		const entity *alpha	entity containing alpha chromosome.
		const int chromosomeid	Index of chromosome to consider.
  return:	Returns number of alleles with value "1".
  last updated:	16 May 2002
 **********************************************************************/

int ga_similarity_bitstring_count_1_alleles( const population *pop,
                                      const entity *alpha, const int chromosomeid )
  {
  int		i;		/* Loop variable over all alleles. */
  int		count=0;	/* Number of matching alleles. */
  byte		*a;		/* Comparison bitstring. */

  /* Checks. */
  if (!pop) die("Null pointer to population structure passed");
  if (!alpha) die("Null pointer to entity structure passed");
  if (chromosomeid<0 || chromosomeid>=pop->num_chromosomes)
    die("Invalid chromosome index passed");

  a = (byte*)(alpha->chromosome[chromosomeid]);

  for ( i=0; i<pop->len_chromosomes; i++ )
    {
    if (ga_bit_get( a, i ) == 1) count++;
    }

  return count;
  }


/**********************************************************************
  ga_similarity_bitstring_count_match_alleles()
  synopsis:	Compares two bitstring chromosomes and counts matching alleles.
  parameters:	const population *pop	Population.
		const entity *alpha	entity containing alpha chromosome.
		const entity *beta	entity containing beta chromosome.
		const int chromosomeid	Index of chromosome to consider.
  return:	Returns number of matching alleles.
  last updated:	14 May 2002
 **********************************************************************/

int ga_similarity_bitstring_count_match_alleles( const population *pop,
                                      const entity *alpha, const entity *beta,
                                      const int chromosomeid )
  {
  int		i;		/* Loop variable over all alleles. */
  int		count=0;	/* Number of matching alleles. */
  byte		*a, *b;		/* Comparison bitstrings. */

  /* Checks. */
  if (!pop) die("Null pointer to population structure passed");
  if (!alpha || !beta) die("Null pointer to entity structure passed");
  if (chromosomeid<0 || chromosomeid>=pop->num_chromosomes) die("Invalid chromosome index passed");

  a = (byte*)(alpha->chromosome[chromosomeid]);
  b = (byte*)(beta->chromosome[chromosomeid]);

  for ( i=0; i<pop->len_chromosomes; i++ )
    {
    if (ga_bit_get( a, i ) == ga_bit_get( b, i )) count++;
    }

  return count;
  }


/**********************************************************************
  ga_similarity_bitstring_count_and_alleles()
  synopsis:	Compares two bitstring chromosomes and counts alleles
		which both have value of "1".  i.e. Count bits set
		after AND operation.
  parameters:	const population *pop	Population.
		const entity *alpha	entity containing alpha chromosome.
		const entity *beta	entity containing beta chromosome.
		const int chromosomeid	Index of chromosome to consider.
  return:	Returns number of alleles set in both bitstrings.
  last updated:	14 May 2002
 **********************************************************************/

int ga_similarity_bitstring_count_and_alleles( const population *pop,
                                      const entity *alpha, const entity *beta,
                                      const int chromosomeid )
  {
  int		i;		/* Loop variable over all alleles. */
  int		count=0;	/* Number of matching alleles. */
  byte		*a, *b;		/* Comparison bitstrings. */

  /* Checks. */
  if (!pop) die("Null pointer to population structure passed");
  if (!alpha || !beta) die("Null pointer to entity structure passed");
  if (chromosomeid<0 || chromosomeid>=pop->num_chromosomes) die("Invalid chromosome index passed");

  a = (byte*)(alpha->chromosome[chromosomeid]);
  b = (byte*)(beta->chromosome[chromosomeid]);

  for ( i=0; i<pop->len_chromosomes; i++ )
    {
    if (ga_bit_get( a, i ) && ga_bit_get( b, i )) count++;
    }

  return count;
  }


/**********************************************************************
  ga_similarity_integer_count_match_alleles()
  synopsis:	Compares two chromosomes and counts matching alleles.
  parameters:	const population *pop	Population.
		const entity *alpha	entity containing alpha chromosome.
		const entity *beta	entity containing beta chromosome.
		const int chromosomeid	Index of chromosome to consider.
  return:	Returns number of matching alleles.
  last updated:	14 May 2002
 **********************************************************************/

int ga_similarity_integer_count_match_alleles( const population *pop,
                                      const entity *alpha, const entity *beta,
                                      const int chromosomeid )
  {
  int		i;		/* Loop variable over all alleles. */
  int		count=0;	/* Number of matching alleles. */
  int		*a, *b;		/* Comparison integer arrays. */

  /* Checks. */
  if (!pop) die("Null pointer to population structure passed");
  if (!alpha || !beta) die("Null pointer to entity structure passed");
  if (chromosomeid<0 || chromosomeid>=pop->num_chromosomes) die("Invalid chromosome index passed");

  a = (int*)(alpha->chromosome[chromosomeid]);
  b = (int*)(beta->chromosome[chromosomeid]);

  for (i=0; i<pop->len_chromosomes; i++)
    if (a[i] == b[i]) count++;

  return count;
  }


/**********************************************************************
  ga_similarity_bitstring_tanimoto()
  synopsis:	Compares the chromosomes of two entities.
  parameters:	const population *pop	Population.
		const entity *alpha	entity containing alpha chromosome.
		const entity *beta	entity containing beta chromosome.
  return:	Returns Tanimoto similarity coefficient.
  last updated:	16 May 2002
 **********************************************************************/

double ga_similarity_bitstring_tanimoto(const population *pop,
                                      const entity *alpha, const entity *beta)
  {
  int           i;              /* Loop variable over all chromosomes. */
  int		a=0, b=0;	/* Number of ones in the individual entities' chromosomes. */
  int		n=0;		/* Number of ones both entities' chromosomes. */

  /* Checks. */
  if (!pop) die("Null pointer to population structure passed");
  if (!alpha || !beta) die("Null pointer to entity structure passed");

  for (i=0; i<pop->num_chromosomes; i++)
    {
    n += ga_similarity_bitstring_count_and_alleles( pop, alpha, beta, i );
    a += ga_similarity_bitstring_count_1_alleles( pop, alpha, i );
    b += ga_similarity_bitstring_count_1_alleles( pop, beta, i );
    }

  return (double) n/(a+b-n);
  }


/**********************************************************************
  ga_similarity_bitstring_dice()
  synopsis:	Compares the chromosomes of two entities.
  parameters:	const population *pop	Population.
		const entity *alpha	entity containing alpha chromosome.
		const entity *beta	entity containing beta chromosome.
  return:	Returns Dice similarity coefficient.
  last updated:	16 May 2002
 **********************************************************************/

double ga_similarity_bitstring_dice(const population *pop,
                                      const entity *alpha, const entity *beta)
  {
  int           i;              /* Loop variable over all chromosomes. */
  int		a=0, b=0;	/* Number of ones in the individual entities' chromosomes. */
  int		n=0;		/* Number of ones both entities' chromosomes. */

  /* Checks. */
  if (!pop) die("Null pointer to population structure passed");
  if (!alpha || !beta) die("Null pointer to entity structure passed");

  for (i=0; i<pop->num_chromosomes; i++)
    {
    n += ga_similarity_bitstring_count_and_alleles( pop, alpha, beta, i );
    a += ga_similarity_bitstring_count_1_alleles( pop, alpha, i );
    b += ga_similarity_bitstring_count_1_alleles( pop, beta, i );
    }

  return (double) 2*n/(a+b);
  }


/**********************************************************************
  ga_similarity_bitstring_euclidean()
  synopsis:	Compares the chromosomes of two entities.
  parameters:	const population *pop	Population.
		const entity *alpha	entity containing alpha chromosome.
		const entity *beta	entity containing beta chromosome.
  return:	Returns Euclidean similarity coefficient.
  last updated:	16 May 2002
 **********************************************************************/

double ga_similarity_bitstring_euclidean(const population *pop,
                                      const entity *alpha, const entity *beta)
  {
  int           i;              /* Loop variable over all chromosomes. */
  int		a=0, b=0;	/* Number of ones in the individual entities' chromosomes. */
  int		n=0;		/* Number of ones both entities' chromosomes. */

  /* Checks. */
  if (!pop) die("Null pointer to population structure passed");
  if (!alpha || !beta) die("Null pointer to entity structure passed");

  for (i=0; i<pop->num_chromosomes; i++)
    {
    n += ga_similarity_bitstring_count_and_alleles( pop, alpha, beta, i );
    a += ga_similarity_bitstring_count_1_alleles( pop, alpha, i );
    b += ga_similarity_bitstring_count_1_alleles( pop, beta, i );
    }

  return 1.0 - sqrt((double)(a+b-2*n))/pop->len_chromosomes;
  }


/**********************************************************************
  ga_similarity_bitstring_hamming()
  synopsis:	Compares the chromosomes of two entities.
  parameters:	const population *pop	Population.
		const entity *alpha	entity containing alpha chromosome.
		const entity *beta	entity containing beta chromosome.
  return:	Returns Hamming similarity coefficient.
  last updated:	16 May 2002
 **********************************************************************/

double ga_similarity_bitstring_hamming(const population *pop,
                                      const entity *alpha, const entity *beta)
  {
  int           i;              /* Loop variable over all chromosomes. */
  int		a=0, b=0;	/* Number of ones in the individual entities' chromosomes. */
  int		n=0;		/* Number of ones both entities' chromosomes. */

  /* Checks. */
  if (!pop) die("Null pointer to population structure passed");
  if (!alpha || !beta) die("Null pointer to entity structure passed");

  for (i=0; i<pop->num_chromosomes; i++)
    {
    n += ga_similarity_bitstring_count_and_alleles( pop, alpha, beta, i );
    a += ga_similarity_bitstring_count_1_alleles( pop, alpha, i );
    b += ga_similarity_bitstring_count_1_alleles( pop, beta, i );
    }

  return 1.0 - (a+b-2*n)/pop->len_chromosomes;
  }


/**********************************************************************
  ga_similarity_bitstring_cosine()
  synopsis:	Compares the chromosomes of two entities.
  parameters:	const population *pop	Population.
		const entity *alpha	entity containing alpha chromosome.
		const entity *beta	entity containing beta chromosome.
  return:	Returns Cosine similarity coefficient.
  last updated:	16 May 2002
 **********************************************************************/

double ga_similarity_bitstring_cosine(const population *pop,
                                      const entity *alpha, const entity *beta)
  {
  int           i;              /* Loop variable over all chromosomes. */
  int		a=0, b=0;	/* Number of ones in the individual entities' chromosomes. */
  int		n=0;		/* Number of ones both entities' chromosomes. */

  /* Checks. */
  if (!pop) die("Null pointer to population structure passed");
  if (!alpha || !beta) die("Null pointer to entity structure passed");

  for (i=0; i<pop->num_chromosomes; i++)
    {
    n += ga_similarity_bitstring_count_and_alleles( pop, alpha, beta, i );
    a += ga_similarity_bitstring_count_1_alleles( pop, alpha, i );
    b += ga_similarity_bitstring_count_1_alleles( pop, beta, i );
    }

  if (a==0 || b==0) return 0;

  return n/sqrt((double)(a*b));
  }


/**********************************************************************
  ga_similarity_double_count_match_alleles()
  synopsis:	Compares two "double" chromosomes and counts matching
		alleles.  A match is defined to be when
		x+GA_TINY_DOUBLE>y>x-GA_TINY_DOUBLE
  parameters:	const population *pop	Population.
		const entity *alpha	entity containing alpha chromosome.
		const entity *beta	entity containing beta chromosome.
		const int chromosomeid	Index of chromosome to consider.
  return:	Returns number of matching alleles.
  last updated:	23 May 2002
 **********************************************************************/

int ga_similarity_double_count_match_alleles( const population *pop,
                                      const entity *alpha, const entity *beta,
                                      const int chromosomeid )
  {
  int		i;		/* Loop variable over all alleles. */
  int		count=0;	/* Number of matching alleles. */
  double	*a, *b;		/* Comparison chromosomes. */

  /* Checks. */
  if (!pop) die("Null pointer to population structure passed");
  if (!alpha || !beta) die("Null pointer to entity structure passed");
  if (chromosomeid<0 || chromosomeid>=pop->num_chromosomes) die("Invalid chromosome index passed");

  a = (double*)(alpha->chromosome[chromosomeid]);
  b = (double*)(beta->chromosome[chromosomeid]);

  for ( i=0; i<pop->len_chromosomes; i++ )
    {
    if (a[i]+GA_TINY_DOUBLE>b[i] && b[i]>a[i]-GA_TINY_DOUBLE) count++;
    }

  return count;
  }


/**********************************************************************
  ga_similarity_double_tanimoto()
  synopsis:	Compares the chromosomes of two entities.
  parameters:	const population *pop	Population.
		const entity *alpha	entity containing alpha chromosome.
		const entity *beta	entity containing beta chromosome.
  return:	Returns Tanimoto similarity coefficient.
		Range: -1/3 to +1
  last updated:	01 Apr 2004
 **********************************************************************/

double ga_similarity_double_tanimoto(const population *pop,
                                      const entity *alpha, const entity *beta)
  {
  int           i, j;			/* Loop variable over all chromosomes, alleles. */
  double	ab=0.0, aa=0.0, bb=0.0;	/* Components of the similarity equation. */
  double	*a, *b;		/* Comparison chromosomes. */

  /* Checks. */
  if (!pop) die("Null pointer to population structure passed");
  if (!alpha || !beta) die("Null pointer to entity structure passed");

  for (i=0; i<pop->num_chromosomes; i++)
    {
    a = (double*)(alpha->chromosome[i]);
    b = (double*)(beta->chromosome[i]);

    for (j=0; j<pop->len_chromosomes; j++)
       {
       aa += a[j]*a[j];
       ab += a[j]*b[j];
       bb += b[j]*b[j];
       }
    }

  return ab/(aa+bb-ab);
  }


/**********************************************************************
  ga_similarity_double_dice()
  synopsis:	Compares the chromosomes of two entities.
  parameters:	const population *pop	Population.
		const entity *alpha	entity containing alpha chromosome.
		const entity *beta	entity containing beta chromosome.
  return:	Returns Dice similarity coefficient.
		Range: -1 to +1
  last updated:	01 Apr 2004
 **********************************************************************/

double ga_similarity_double_dice(const population *pop,
                                      const entity *alpha, const entity *beta)
  {
  int           i, j;			/* Loop variable over all chromosomes, alleles. */
  double	ab=0.0, aa=0.0, bb=0.0;	/* Components of the similarity equation. */
  double	*a, *b;			/* Comparison chromosomes. */

  /* Checks. */
  if (!pop) die("Null pointer to population structure passed");
  if (!alpha || !beta) die("Null pointer to entity structure passed");

  for (i=0; i<pop->num_chromosomes; i++)
    {
    a = (double*)(alpha->chromosome[i]);
    b = (double*)(beta->chromosome[i]);

    for (j=0; j<pop->len_chromosomes; j++)
       {
       aa += a[j]*a[j];
       ab += a[j]*b[j];
       bb += b[j]*b[j];
       }
    }

  return (2*ab)/(aa+bb);
  }


/**********************************************************************
  ga_similarity_double_cosine()
  synopsis:	Compares the chromosomes of two entities.
  parameters:	const population *pop	Population.
		const entity *alpha	entity containing alpha chromosome.
		const entity *beta	entity containing beta chromosome.
  return:	Returns Cosine similarity coefficient.
		Range: -1 to +1
  last updated:	01 Apr 2004
 **********************************************************************/

double ga_similarity_double_cosine(const population *pop,
                                      const entity *alpha, const entity *beta)
  {
  int           i, j;			/* Loop variable over all chromosomes, alleles. */
  double	ab=0.0, aa=0.0, bb=0.0;	/* Components of the similarity equation. */
  double	*a, *b;			/* Comparison chromosomes. */

  /* Checks. */
  if (!pop) die("Null pointer to population structure passed");
  if (!alpha || !beta) die("Null pointer to entity structure passed");

  for (i=0; i<pop->num_chromosomes; i++)
    {
    a = (double*)(alpha->chromosome[i]);
    b = (double*)(beta->chromosome[i]);

    for (j=0; j<pop->len_chromosomes; j++)
       {
       aa += a[j]*a[j];
       ab += a[j]*b[j];
       bb += b[j]*b[j];
       }
    }

  return ab/sqrt(aa+bb);
  }


