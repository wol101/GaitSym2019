/**********************************************************************
  ga_replace.c
 **********************************************************************

  ga_replace - Genetic algorithm replacement operators.
  Copyright ©2000-2003, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:     Routines for performing GA replacement operations.

		These functions should duplicate user data where
		appropriate.

  To do:	Replace parents if less fit.
		Replace most similar entities.
		Replace 'oldest' entities.

 **********************************************************************/

#include "gaul/ga_core.h"

/**********************************************************************
  ga_replace_by_fitness()
  synopsis:	Replace by fitness.  (i.e. New entity gets inserted
		into population, according to fitness, and least fit
		entity dies.)
  parameters:
  return:
  last updated: 11 Apr 2002
 **********************************************************************/

void ga_replace_by_fitness(population *pop, entity *child)
  {
  int		i, j;		/* Loop over entities. */
  entity	*tmp;		/* For swapping. */

  /* Find child's current rank, which will be somewhere near the bottom. */
  i=pop->size;
  do
    {
    i--;
    } while (i>=pop->orig_size && !(child == pop->entity_iarray[i]));

  if (i<pop->orig_size) die("Dodgy replacement requested.");

  if (child->fitness >= pop->entity_iarray[pop->orig_size-1]->fitness)
    {
    tmp = pop->entity_iarray[pop->orig_size-1];
    pop->entity_iarray[pop->orig_size-1] = pop->entity_iarray[i];
    pop->entity_iarray[i] = tmp;

    /* Shuffle entity to rightful location. */
    j = pop->orig_size-1;
    while (j>0 && pop->entity_iarray[j]->fitness > pop->entity_iarray[j-1]->fitness)
      {
      tmp = pop->entity_iarray[j];
      pop->entity_iarray[j] = pop->entity_iarray[j-1];
      pop->entity_iarray[j-1] = tmp;
      j--;
      }

    /* Rank of replaced entity. */
    i = pop->orig_size-1;
    }

  /* Kill off child/replaced entity. */
  ga_entity_dereference_by_rank(pop, i);

  return;
  }


