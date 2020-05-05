/**********************************************************************
  ga_compare.c
 **********************************************************************

  ga_compare - Entity comparison routines.
  Copyright ©2003-2004, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:     Routines for comparing entities.

		These routines return a distance between two entities.

 **********************************************************************/

#include "gaul/ga_core.h"

/**********************************************************************
  ga_compare_char_hamming()
  synopsis:	Compares two char-array genomes and returns their
		hamming distance.
  parameters:	population *pop	Population of entities (you may use
			differing populations if they are "compatible")
		entity *alpha	Test entity.
		entity *beta	Test entity.
  return:	Returns Hamming distance between two entities' genomes.
  last updated:	21 May 2003
 **********************************************************************/

double ga_compare_char_hamming(population *pop, entity *alpha, entity *beta)
  {
  int		i,j;		/* Loop variable over all chromosomes, alleles. */
  int		dist=0;		/* Genomic distance. */
  char		*a, *b;		/* Pointers to chromosomes. */

  /* Checks */
  if (!alpha || !beta) die("Null pointer to entity structure passed");

  for (i=0; i<pop->num_chromosomes; i++)
    {
    a = (char *)(alpha->chromosome[i]);
    b = (char *)(beta->chromosome[i]);

    for (j=0; j<pop->len_chromosomes; j++)
      {
      dist += abs((int)a[j]-b[j]);
      }
    }

  return (double) dist;
  }


/**********************************************************************
  ga_compare_char_euclidean()
  synopsis:	Compares two char-array genomes and returns their
		euclidean distance.
  parameters:	population *pop	Population of entities (you may use
			differing populations if they are "compatible")
		entity *alpha	Test entity.
		entity *beta	Test entity.
  return:	Returns Euclidean distance between two entities' genomes.
  last updated:	25 Aug 2004
 **********************************************************************/

double ga_compare_char_euclidean(population *pop, entity *alpha, entity *beta)
  {
  int		i,j;			/* Loop variable over all chromosomes, alleles. */
  double	sqdistsum=0.0;		/* Genomic distance. */
  char		*a, *b;			/* Pointers to chromosomes. */

  /* Checks */
  if (!alpha || !beta) die("Null pointer to entity structure passed");

  for (i=0; i<pop->num_chromosomes; i++)
    {
    a = (char *)(alpha->chromosome[i]);
    b = (char *)(beta->chromosome[i]);

    for (j=0; j<pop->len_chromosomes; j++)
      {
      sqdistsum += SQU((int)a[j]-b[j]);
      }
    }

  return sqrt(sqdistsum);
  }


/**********************************************************************
  ga_compare_integer_hamming()
  synopsis:	Compares two integer-array genomes and returns their
		hamming distance.
  parameters:	population *pop	Population of entities (you may use
			differing populations if they are "compatible")
		entity *alpha	Test entity.
		entity *beta	Test entity.
  return:	Returns Hamming distance between two entities' genomes.
  last updated:	25 Aug 2004
 **********************************************************************/

double ga_compare_integer_hamming(population *pop, entity *alpha, entity *beta)
  {
  int		i,j;		/* Loop variable over all chromosomes, alleles. */
  int		dist=0;		/* Genomic distance. */
  int		*a, *b;		/* Pointers to chromosomes. */

  /* Checks */
  if (!alpha || !beta) die("Null pointer to entity structure passed");

  for (i=0; i<pop->num_chromosomes; i++)
    {
    a = (int *)(alpha->chromosome[i]);
    b = (int *)(beta->chromosome[i]);

    for (j=0; j<pop->len_chromosomes; j++)
      {
      dist += abs(a[j]-b[j]);
      }
    }

  return (double) dist;
  }


/**********************************************************************
  ga_compare_integer_euclidean()
  synopsis:	Compares two integer-array genomes and returns their
		euclidean distance.
  parameters:	population *pop	Population of entities (you may use
			differing populations if they are "compatible")
		entity *alpha	Test entity.
		entity *beta	Test entity.
  return:	Returns Euclidean distance between two entities' genomes.
  last updated:	25 Aug 2004
 **********************************************************************/

double ga_compare_integer_euclidean(population *pop, entity *alpha, entity *beta)
  {
  int		i,j;			/* Loop variable over all chromosomes, alleles. */
  double	sqdistsum=0.0;		/* Genomic distance. */
  int		*a, *b;			/* Pointers to chromosomes. */

  /* Checks */
  if (!alpha || !beta) die("Null pointer to entity structure passed");

  for (i=0; i<pop->num_chromosomes; i++)
    {
    a = (int *)(alpha->chromosome[i]);
    b = (int *)(beta->chromosome[i]);

    for (j=0; j<pop->len_chromosomes; j++)
      {
      sqdistsum += SQU(a[j]-b[j]);
      }
    }

  return sqrt(sqdistsum);
  }


/**********************************************************************
  ga_compare_double_hamming()
  synopsis:	Compares two double-array genomes and returns their
		hamming distance.
  parameters:	population *pop	Population of entities (you may use
			differing populations if they are "compatible")
		entity *alpha	Test entity.
		entity *beta	Test entity.
  return:	Returns Hamming distance between two entities' genomes.
  last updated:	25 Aug 2004
 **********************************************************************/

double ga_compare_double_hamming(population *pop, entity *alpha, entity *beta)
  {
  int		i,j;		/* Loop variable over all chromosomes, alleles. */
  double	dist=0.0;	/* Genomic distance. */
  double	*a, *b;		/* Pointers to chromosomes. */

  /* Checks */
  if (!alpha || !beta) die("Null pointer to entity structure passed");

  for (i=0; i<pop->num_chromosomes; i++)
    {
    a = (double *)(alpha->chromosome[i]);
    b = (double *)(beta->chromosome[i]);

    for (j=0; j<pop->len_chromosomes; j++)
      {
      dist += ABS(a[j]-b[j]);
      }
    }

  return dist;
  }


/**********************************************************************
  ga_compare_double_euclidean()
  synopsis:	Compares two double-array genomes and returns their
		euclidean distance.
  parameters:	population *pop	Population of entities (you may use
			differing populations if they are "compatible")
		entity *alpha	Test entity.
		entity *beta	Test entity.
  return:	Returns Euclidean distance between two entities' genomes.
  last updated:	25 Aug 2004
 **********************************************************************/

double ga_compare_double_euclidean(population *pop, entity *alpha, entity *beta)
  {
  int		i,j;			/* Loop variable over all chromosomes, alleles. */
  double	sqdistsum=0.0;		/* Genomic distance. */
  double	*a, *b;			/* Pointers to chromosomes. */

  /* Checks */
  if (!alpha || !beta) die("Null pointer to entity structure passed");

  for (i=0; i<pop->num_chromosomes; i++)
    {
    a = (double *)(alpha->chromosome[i]);
    b = (double *)(beta->chromosome[i]);

    for (j=0; j<pop->len_chromosomes; j++)
      {
      sqdistsum += SQU(a[j]-b[j]);
      }
    }

  return sqrt(sqdistsum);
  }


/**********************************************************************
  ga_compare_boolean_hamming()
  synopsis:	Compares two boolean-array genomes and returns their
		hamming distance.
  parameters:	population *pop	Population of entities (you may use
			differing populations if they are "compatible")
		entity *alpha	Test entity.
		entity *beta	Test entity.
  return:	Returns Hamming distance between two entities' genomes.
  last updated:	25 Aug 2004
 **********************************************************************/

double ga_compare_boolean_hamming(population *pop, entity *alpha, entity *beta)
  {
  int		i,j;		/* Loop variable over all chromosomes, alleles. */
  int		dist=0;		/* Genomic distance. */
  boolean	*a, *b;		/* Pointers to chromosomes. */

  /* Checks */
  if (!alpha || !beta) die("Null pointer to entity structure passed");

  for (i=0; i<pop->num_chromosomes; i++)
    {
    a = (boolean *)(alpha->chromosome[i]);
    b = (boolean *)(beta->chromosome[i]);

    for (j=0; j<pop->len_chromosomes; j++)
      {
      dist += (a[j]!=b[j]);
      }
    }

  return (double) dist;
  }


/**********************************************************************
  ga_compare_boolean_euclidean()
  synopsis:	Compares two boolean-array genomes and returns their
		euclidean distance.
  parameters:	population *pop	Population of entities (you may use
			differing populations if they are "compatible")
		entity *alpha	Test entity.
		entity *beta	Test entity.
  return:	Returns Euclidean distance between two entities' genomes.
  last updated:	25 Aug 2004
 **********************************************************************/

double ga_compare_boolean_euclidean(population *pop, entity *alpha, entity *beta)
  {
  int		i,j;			/* Loop variable over all chromosomes, alleles. */
  double	sqdistsum=0.0;		/* Genomic distance. */
  boolean	*a, *b;			/* Pointers to chromosomes. */

  /* Checks */
  if (!alpha || !beta) die("Null pointer to entity structure passed");

  for (i=0; i<pop->num_chromosomes; i++)
    {
    a = (boolean *)(alpha->chromosome[i]);
    b = (boolean *)(beta->chromosome[i]);

    for (j=0; j<pop->len_chromosomes; j++)
      {
      sqdistsum += (a[j]!=b[j]);
      }
    }

  return sqrt(sqdistsum);
  }


/**********************************************************************
  ga_compare_bitstring_hamming()
  synopsis:	Compares two bitstring-array genomes and returns their
		hamming distance.
  parameters:	population *pop	Population of entities (you may use
			differing populations if they are "compatible")
		entity *alpha	Test entity.
		entity *beta	Test entity.
  return:	Returns Hamming distance between two entities' genomes.
  last updated:	25 Aug 2004
 **********************************************************************/

double ga_compare_bitstring_hamming(population *pop, entity *alpha, entity *beta)
  {
  int		i,j;		/* Loop variable over all chromosomes, alleles. */
  int		dist=0;		/* Genomic distance. */
  byte		*a, *b;		/* Pointers to chromosomes. */

  /* Checks */
  if (!alpha || !beta) die("Null pointer to entity structure passed");

  for (i=0; i<pop->num_chromosomes; i++)
    {
    a = (byte *)(alpha->chromosome[i]);
    b = (byte *)(beta->chromosome[i]);

    for (j=0; j<pop->len_chromosomes; j++)
      {
      dist += (ga_bit_get(a,j)!=ga_bit_get(b,j));
      }
    }

  return (double) dist;
  }


/**********************************************************************
  ga_compare_bitstring_euclidean()
  synopsis:	Compares two bitstring-array genomes and returns their
		euclidean distance.
  parameters:	population *pop	Population of entities (you may use
			differing populations if they are "compatible")
		entity *alpha	Test entity.
		entity *beta	Test entity.
  return:	Returns Euclidean distance between two entities' genomes.
  last updated:	25 Aug 2004
 **********************************************************************/

double ga_compare_bitstring_euclidean(population *pop, entity *alpha, entity *beta)
  {
  int		i,j;			/* Loop variable over all chromosomes, alleles. */
  double	sqdistsum=0.0;		/* Genomic distance. */
  byte		*a, *b;			/* Pointers to chromosomes. */

  /* Checks */
  if (!alpha || !beta) die("Null pointer to entity structure passed");

  for (i=0; i<pop->num_chromosomes; i++)
    {
    a = (byte *)(alpha->chromosome[i]);
    b = (byte *)(beta->chromosome[i]);

    for (j=0; j<pop->len_chromosomes; j++)
      {
      sqdistsum += (ga_bit_get(a,j)!=ga_bit_get(b,j));
      }
    }

  return sqrt(sqdistsum);
  }


