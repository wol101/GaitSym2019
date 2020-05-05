/**********************************************************************
  ga_gradient.h
 **********************************************************************

  ga_gradient - Gradient methods for comparison and local search.
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

  Synopsis:	Gradient methods for comparison and local search.

 **********************************************************************/

#ifndef GA_GRADIENT_H_INCLUDED
#define GA_GRADIENT_H_INCLUDED

/*
 * Includes.
 */
#include "gaul.h"

/*
 * Prototypes.
 */
FUNCPROTO void ga_population_set_gradient_parameters( population		*pop,
                                        const GAto_double	to_double,
                                        const GAfrom_double	from_double,
                                        const GAgradient	gradient,
					const int		dimensions,
					const double		step_size);
FUNCPROTO int ga_steepestascent(    population              *pop,
		entity                  *initial,
	        const int               max_iterations );
FUNCPROTO int ga_steepestascent_double(    population              *pop,
		entity                  *initial,
	        const int               max_iterations );

#endif	/* GA_GRADIENT_H_INCLUDED */

