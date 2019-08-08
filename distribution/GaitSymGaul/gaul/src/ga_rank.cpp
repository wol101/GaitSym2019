/**********************************************************************
  ga_rank.c
 **********************************************************************

  ga_rank - Entity comparison routines.
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

  Synopsis:     Routines for comparing entities.

		These routines return -1, 0, or 1 depending upon
		how the entities should ranked.
		(optionally) 0 if ranking is equal or indeterminant.
		1 if alpha entity ranks higher.
		-1 if beta entity ranks higher.

 **********************************************************************/

#include "gaul/ga_core.h"

/**********************************************************************
  ga_rank_fitness()
  synopsis:	Compares two entities and returns their relative
		ranking.
  parameters:	population *alphapop	Population A
		entity *alpha		Test entity A.
		population *betapop 	Population B
		entity *beta		Test entity B.
  return:	Relative rank.
  last updated:	24 Feb 2005
 **********************************************************************/

int ga_rank_fitness(	population *alphapop, entity *alpha,
			population *betapop, entity *beta )
  {

  /* Checks */
  if (!alphapop || !betapop) die("Null pointer to population structure passed");
  if (!alpha || !beta) die("Null pointer to entity structure passed");

  return (alpha->fitness-beta->fitness)>0.0;
  }


