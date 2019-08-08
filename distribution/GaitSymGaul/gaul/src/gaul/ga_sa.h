/**********************************************************************
  ga_sa.h
 **********************************************************************

  ga_sa - A simulated annealling algorithm for comparison and search.
  Copyright ©2002-2005, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:	A simulated annealling algorithm for comparison and search.

 **********************************************************************/

#ifndef GA_SA_H_INCLUDED
#define GA_SA_H_INCLUDED

/*
 * Includes.
 */
#include "gaul.h"

/*
 * Prototypes.
 */
FUNCPROTO boolean ga_sa_boltzmann_acceptance(population *pop, entity *original, entity *putative);
FUNCPROTO boolean ga_sa_linear_acceptance(population *pop, entity *original, entity *putative);
FUNCPROTO void ga_population_set_sa_temperature(population *pop, const double temp);
FUNCPROTO double ga_population_get_sa_temperature(population *pop);
FUNCPROTO void ga_population_set_sa_parameters(population *pop, GAsa_accept sa_accept, const double initial_temp, const double final_temp, const double temp_step, const int temp_freq);
FUNCPROTO int ga_sa(population *pop, entity *initial, const int max_iterations);

#endif	/* GA_SA_H_INCLUDED */

