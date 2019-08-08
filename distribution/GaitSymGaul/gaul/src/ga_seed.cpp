/**********************************************************************
  ga_seed.c
 **********************************************************************

  ga_seed - Genetic algorithm genome initialisation operators.
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

  Synopsis:     Routines for performing GA seeding operations.

                Seeding operations generate genetic data by some
                non-evolutionary means.  Typically, this is often
                just random generation.

 **********************************************************************/

#include "gaul/ga_core.h"

/**********************************************************************
  ga_seed_boolean_random()
  synopsis:	Seed genetic data for a single entity with a boolean
                chromosome by randomly setting each bit.
  parameters:	population *pop
                entity *adam
  return:	success
  last updated: 15/05/01
 **********************************************************************/

boolean ga_seed_boolean_random(population *pop, entity *adam)
  {
  int		chromo;		/* Index of chromosome to seed */
  int		point;		/* Index of allele to seed */

/* Checks. */
  if (!pop) die("Null pointer to population structure passed.");
  if (!adam) die("Null pointer to entity structure passed.");

/* Seeding. */
  for (chromo=0; chromo<pop->num_chromosomes; chromo++)
    {
    for (point=0; point<pop->len_chromosomes; point++)
      {
      ((boolean *)adam->chromosome[chromo])[point] = random_boolean();
      }
    }

  return TRUE;
  }


/**********************************************************************
  ga_seed_boolean_zero()
  synopsis:	Seed genetic data for a single entity with a boolean
                chromosome by setting each bit to zero.
  parameters:	population *pop
                entity *adam
  return:	success
  last updated: 30 Jun 2003
 **********************************************************************/

boolean ga_seed_boolean_zero(population *pop, entity *adam)
  {
  int		chromo;		/* Index of chromosome to seed */
  int		point;		/* Index of allele to seed */

/* Checks. */
  if (!pop) die("Null pointer to population structure passed.");
  if (!adam) die("Null pointer to entity structure passed.");

/* Seeding. */
  for (chromo=0; chromo<pop->num_chromosomes; chromo++)
    {
    for (point=0; point<pop->len_chromosomes; point++)
      {
      ((boolean *)adam->chromosome[chromo])[point] = 0;
      }
    }

  return TRUE;
  }


/**********************************************************************
  ga_seed_integer_random()
  synopsis:	Seed genetic data for a single entity with an integer
                chromosome by randomly setting each allele.
  parameters:	population *pop
                entity *adam
  return:	success
  last updated: 15/05/01
 **********************************************************************/

boolean ga_seed_integer_random(population *pop, entity *adam)
  {
  int		chromo;		/* Index of chromosome to seed */
  int		point;		/* Index of allele to seed */

/* Checks. */
  if (!pop) die("Null pointer to population structure passed.");
  if (!adam) die("Null pointer to entity structure passed.");

/* Seeding. */
  for (chromo=0; chromo<pop->num_chromosomes; chromo++)
    {
    for (point=0; point<pop->len_chromosomes; point++)
      {
      ((int *)adam->chromosome[chromo])[point] =
        random_int_range(pop->allele_min_integer,pop->allele_max_integer);
      }
    }

  return TRUE;
  }


/**********************************************************************
  ga_seed_integer_zero()
  synopsis:	Seed genetic data for a single entity with an integer
                chromosome by setting each allele to zero.
  parameters:	population *pop
                entity *adam
  return:	success
  last updated: 15/05/01
 **********************************************************************/

boolean ga_seed_integer_zero(population *pop, entity *adam)
  {
  int		chromo;		/* Index of chromosome to seed */
  int		point;		/* Index of allele to seed */

/* Checks. */
  if (!pop) die("Null pointer to population structure passed.");
  if (!adam) die("Null pointer to entity structure passed.");

/* Seeding. */
  for (chromo=0; chromo<pop->num_chromosomes; chromo++)
    {
    for (point=0; point<pop->len_chromosomes; point++)
      {
      ((int *)adam->chromosome[chromo])[point] = 0;
      }
    }

  return TRUE;
  }


/**********************************************************************
  ga_seed_char_random()
  synopsis:	Seed genetic data for a single entity with a character
                chromosome by randomly setting each allele.
  parameters:	population *pop
                entity *adam
  return:	success
  last updated: 16/06/01
 **********************************************************************/

boolean ga_seed_char_random(population *pop, entity *adam)
  {
  int		chromo;		/* Index of chromosome to seed */
  int		point;		/* Index of allele to seed */

/* Checks. */
  if (!pop) die("Null pointer to population structure passed.");
  if (!adam) die("Null pointer to entity structure passed.");

/* Seeding. */
  for (chromo=0; chromo<pop->num_chromosomes; chromo++)
    {
    for (point=0; point<pop->len_chromosomes; point++)
      {
      ((char *)adam->chromosome[chromo])[point]
            = random_int(CHAR_MAX-CHAR_MIN)+CHAR_MIN;
      }
    }

  return TRUE;
  }


/**********************************************************************
  ga_seed_double_random()
  synopsis:	Seed genetic data for a single entity with a double-
                precision floating-point chromosome by randomly
                setting each allele.
  parameters:	population *pop
                entity *adam
  return:	success
  last updated: 16/06/01
 **********************************************************************/

boolean ga_seed_double_random(population *pop, entity *adam)
  {
  int		chromo;		/* Index of chromosome to seed */
  int		point;		/* Index of allele to seed */

/* Checks. */
  if (!pop) die("Null pointer to population structure passed.");
  if (!adam) die("Null pointer to entity structure passed.");

/* Seeding. */
  for (chromo=0; chromo<pop->num_chromosomes; chromo++)
    {
    for (point=0; point<pop->len_chromosomes; point++)
      {
      ((double *)adam->chromosome[chromo])[point] =
        random_double_range(pop->allele_min_double,pop->allele_max_double);
      }
    }

  return TRUE;
  }


/**********************************************************************
  ga_seed_double_random_unit_gaussian()
  synopsis:	Seed genetic data for a single entity with a double-
                precision floating-point chromosome by randomly
                setting each allele using a unit gaussian distribution.
  parameters:	population *pop
                entity *adam
  return:	success
  last updated: 02 Jul 2003
 **********************************************************************/

boolean ga_seed_double_random_unit_gaussian(population *pop, entity *adam)
  {
  int		chromo;		/* Index of chromosome to seed */
  int		point;		/* Index of allele to seed */

/* Checks. */
  if (!pop) die("Null pointer to population structure passed.");
  if (!adam) die("Null pointer to entity structure passed.");

/* Seeding. */
  for (chromo=0; chromo<pop->num_chromosomes; chromo++)
    {
    for (point=0; point<pop->len_chromosomes; point++)
      {
      ((double *)adam->chromosome[chromo])[point] = random_unit_gaussian();
      }
    }

  return TRUE;
  }


/**********************************************************************
  ga_seed_double_zero()
  synopsis:
  parameters:
  return:
  last updated: 16/06/01
 **********************************************************************/

boolean ga_seed_double_zero(population *pop, entity *adam)
  {
  int		chromo;		/* Index of chromosome to seed */
  int		point;		/* Index of allele to seed */

/* Checks. */
  if (!pop) die("Null pointer to population structure passed.");
  if (!adam) die("Null pointer to entity structure passed.");

/* Seeding. */
  for (chromo=0; chromo<pop->num_chromosomes; chromo++)
    {
    for (point=0; point<pop->len_chromosomes; point++)
      {
      ((double *)adam->chromosome[chromo])[point] = 0.0;
      }
    }

  return TRUE;
  }


/**********************************************************************
  ga_seed_printable_random()
  synopsis:
  parameters:
  return: last updated: 16/06/01
 **********************************************************************/

boolean ga_seed_printable_random(population *pop, entity *adam)
  {
  int		chromo;		/* Index of chromosome to seed */
  int		point;		/* Index of allele to seed */

/* Checks. */
  if (!pop) die("Null pointer to population structure passed.");
  if (!adam) die("Null pointer to entity structure passed.");

/* Seeding. */
  for (chromo=0; chromo<pop->num_chromosomes; chromo++)
    {
    for (point=0; point<pop->len_chromosomes; point++)
      {
      ((char *)adam->chromosome[chromo])[point]
            = random_int('~'-' ')+' ';
      }
    }

  return TRUE;
  }


/**********************************************************************
  ga_seed_bitstring_random()
  synopsis:	Randomly seed bitstring chromosomes.
  parameters:	population *pop
                entity *adam
  return:	success
  last updated: 30/06/01
 **********************************************************************/

boolean ga_seed_bitstring_random(population *pop, entity *adam)
  {
  int		chromo;		/* Index of chromosome to seed */
  int		point;		/* Index of allele to seed */

/* Checks. */
  if (!pop) die("Null pointer to population structure passed.");
  if (!adam) die("Null pointer to entity structure passed.");

/* Seeding. */
  for (chromo=0; chromo<pop->num_chromosomes; chromo++)
    {
    for (point=0; point<pop->len_chromosomes; point++)
      {
      ga_bit_randomize((byte *)adam->chromosome[chromo],point);
      }
    }

  return TRUE;
  }


/**********************************************************************
  ga_seed_bitstring_zero()
  synopsis:	Seed bitstring chromosomes with zeros.
  parameters:	population *pop
                entity *adam
  return:	success
  last updated: 10 Aug 2004
 **********************************************************************/

boolean ga_seed_bitstring_zero(population *pop, entity *adam)
  {
  int		chromo;		/* Index of chromosome to seed */
  int		point;		/* Index of allele to seed */

/* Checks. */
  if (!pop) die("Null pointer to population structure passed.");
  if (!adam) die("Null pointer to entity structure passed.");

/* Seeding. */
  for (chromo=0; chromo<pop->num_chromosomes; chromo++)
    {
    for (point=0; point<pop->len_chromosomes; point++)
      {
      ga_bit_clear((byte *)adam->chromosome[chromo],point);
      }
    }

  return TRUE;
  }


