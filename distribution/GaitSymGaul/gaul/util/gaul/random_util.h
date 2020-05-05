/**********************************************************************
  random_util.h
 **********************************************************************

  random_util - Random number utility routines.
  Copyright ©2000-2002, Stewart Adcock <stewart@linux-domain.com>
  All rights reserved.

  The latest version of this program should be available at:
  http://www.stewart-adcock.co.uk/

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

  Synopsis:	Random number utility routines.

 **********************************************************************/

#ifndef RANDOM_UTIL_H_INCLUDED
#define RANDOM_UTIL_H_INCLUDED

#include "gaul/gaul_util.h"

#include <float.h>
#if HAVE_LIMITS_H==1
#include <limits.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
 * Debugging.
 */
#ifndef RANDOM_DEBUG
# ifdef DEBUG
#  define RANDOM_DEBUG DEBUG
# else
#  define RANDOM_DEBUG 0
# endif
#endif

/*
 * Constants.
 */
#ifdef UINT_MAX
/* UINT_MAX comes from limits.h */
#define RANDOM_RAND_MAX		(unsigned int) UINT_MAX
#else
#define RANDOM_RAND_MAX         (unsigned int) 0xFFFFFFFF      /* Maximum 32-bit unsigned int */
#endif

/*
 * Type definitions.
 * sizeof(random_state) should be 64*sizeof(int) which is
 * hopefully, fairly, optimal.
 */
#define RANDOM_NUM_STATE_VALS	57
typedef struct random_state_t
  {
  unsigned int	v[RANDOM_NUM_STATE_VALS];
  int		j, k, x;
  } random_state;

/*
 * Function prototypes.
 */

FUNCPROTO unsigned int	 random_rand(void);
FUNCPROTO void	random_seed(const unsigned int seed);
FUNCPROTO void	random_tseed(void);
FUNCPROTO void	random_init(void);
FUNCPROTO boolean	random_isinit(void);
FUNCPROTO char	*random_get_state_str(void);
FUNCPROTO unsigned int random_get_state_str_len(void);
FUNCPROTO void	random_set_state_str(char *state);
FUNCPROTO random_state	random_get_state(void);
FUNCPROTO void	random_set_state(random_state state);

FUNCPROTO boolean	random_boolean(void);
FUNCPROTO boolean	random_boolean_prob(const double prob);

FUNCPROTO unsigned int	random_int(const unsigned int max);
FUNCPROTO int	random_int_range(const int min, const int max);

FUNCPROTO double	random_double_full(void);
FUNCPROTO double	random_double(const double max);
FUNCPROTO double	random_double_range(const double min, const double max);
FUNCPROTO double	random_double_1(void);
FUNCPROTO double	random_unit_uniform(void);
FUNCPROTO double	random_gaussian(const double mean, const double stddev);
FUNCPROTO double	random_unit_gaussian(void);
FUNCPROTO void	random_diagnostics(void);
FUNCPROTO boolean	random_test(void);

FUNCPROTO float	random_float_full(void);
FUNCPROTO float	random_float(const float max);
FUNCPROTO float	random_float_range(const float min, const float max);
FUNCPROTO float	random_float_1(void);
FUNCPROTO float	random_float_unit_uniform(void);
FUNCPROTO float	random_float_gaussian(const float mean, const float stddev);
FUNCPROTO float	random_float_unit_gaussian(void);
FUNCPROTO float	random_float_cauchy(void);
FUNCPROTO float	random_float_exponential(void);

FUNCPROTO void	random_int_permutation(const int size, int *iarray, int *oarray);

#define random_int_full	random_rand

#if HAVE_SLANG==1

/* These functions don't need wrappers:
FUNCPROTO void	random_init(void)
FUNCPROTO boolean	random_isinit(void)
FUNCPROTO unsigned int	random_get_state_str(void);
FUNCPROTO char	*random_get_state_str_len(void);
FUNCPROTO void	random_set_state_str(char *state);
FUNCPROTO double	random_double_1(void);
FUNCPROTO boolean	random_boolean(void)
FUNCPROTO double	random_unit_uniform(void)
FUNCPROTO double	random_unit_gaussian(void)
FUNCPROTO void	random_diagnostics(void)
FUNCPROTO boolean	random_test(void)
*/
/* These functions aren't defined as intrinsics anyway:
State fetching/setting stuff.
*/

FUNCPROTO int	random_rand_wrapper(void);
FUNCPROTO void	random_seed_wrapper(int *seed);
FUNCPROTO boolean	random_boolean_prob_wrapper(double *prob);
FUNCPROTO int	random_int_wrapper(int *max);
FUNCPROTO int	random_int_range_wrapper(int *min, int *max);
FUNCPROTO double	random_double_wrapper(double *max);
FUNCPROTO double	random_double_range_wrapper(double *min, double *max);
FUNCPROTO double	random_gaussian_wrapper(double *mean, double *stddev);

#endif	/* HAVE_SLANG==1 */

#endif	/* RANDOM_UTIL_H_INCLUDED */

