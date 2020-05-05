/**********************************************************************
  ga_optim.h
 **********************************************************************

  ga_optim - Gene-based optimisation routines.
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

#ifndef GA_OPTIM_H_INCLUDED
#define GA_OPTIM_H_INCLUDED

/*
 * Includes
 */
#include "gaul.h"

#if HAVE_MPI==1
#include <mpi.h>
#endif

/*
 * Callback function prototype.
 */
typedef void    (*GAspecificmutate)(int chromo, int point, int *data);

/*
 * Prototypes
 */
FUNCPROTO void	 ga_attach_mpi_slave( population *pop );
FUNCPROTO void	 ga_detach_mpi_slaves(void);

FUNCPROTO int	ga_evolution(	population		*pop,
			const int		max_generations );
FUNCPROTO int	ga_evolution_mp(	population		*pop,
			const int		max_generations );
FUNCPROTO int	ga_evolution_mpi(	population		*pop,
			const int		max_generations );
FUNCPROTO int	ga_evolution_forked(	population		*pop,
			const int		max_generations );
FUNCPROTO int	ga_evolution_threaded(	population		*pop,
			const int		max_generations );
FUNCPROTO int	ga_evolution_steady_state(	population		*pop,
			const int		max_iterations );
FUNCPROTO int	ga_evolution_archipelago( const int num_pops,
                        population              **pops,
                        const int               max_generations );
FUNCPROTO int	ga_evolution_archipelago_forked( const int num_pops,
                        population              **pops,
                        const int               max_generations );
FUNCPROTO int	ga_evolution_archipelago_threaded( const int num_pops,
                        population              **pops,
                        const int               max_generations );
FUNCPROTO int	ga_evolution_archipelago_mp( const int num_pops,
                        population              **pops,
                        const int               max_generations );
FUNCPROTO int	ga_evolution_archipelago_mpi( const int num_pops,
                        population              **pops,
                        const int               max_generations );

#endif	/* GA_OPTIM_H */
