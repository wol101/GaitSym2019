/**********************************************************************
  timer_util.h
 **********************************************************************

  timer_util - Useful timer routines with S-Lang intrinsics.
  Copyright ©2002-2003, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:     Timer routines with S-Lang intrinsics which are
                helpful for code analysis.

 **********************************************************************/

#ifndef TIMER_UTIL_H_INCLUDED
#define TIMER_UTIL_H_INCLUDED

#include "gaul/gaul_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "gaul/log_util.h"
#include "gaul/table_util.h"

/*
 * Timer structure.
 */
typedef struct
  {
  clock_t       begin_clock, save_clock;
  time_t        begin_time, save_time;
  } chrono_t;

/*
 * Prototypes.
 */
FUNCPROTO void	timer_diagnostics(void);
FUNCPROTO void	timer_start(chrono_t *t);
FUNCPROTO double	timer_check(chrono_t *t);

/*
 * SLang intrinsic function with equivalent functionality.
 */
#if HAVE_SLANG==1
FUNCPROTO int	timer_new_slang(void);
FUNCPROTO void	timer_free_slang(int *t_handle);
FUNCPROTO void	timer_start_slang(int *t_handle);
FUNCPROTO double	timer_check_slang(int *t_handle);
#endif

#endif /* TIMER_UTIL_H_INCLUDED */

