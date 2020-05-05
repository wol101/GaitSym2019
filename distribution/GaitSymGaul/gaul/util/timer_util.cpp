/**********************************************************************
  timer_util.c
 **********************************************************************

  timer_util - Useful timer routines with S-Lang intrinsics.
  Copyright ©2002-2003, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:	Timer routines with S-Lang intrinsics which are
		helpful for code analysis.

		These functions are thread-safe.

  Bugs:		Note that the user time will be incorrect after about
		72 minutes.

 **********************************************************************/

#include "gaul/timer_util.h"

/**********************************************************************
  timer_diagnostics()
  synopsis:	Display diagnostic information.
  parameters:	none
  return:	none
  last updated:	13 Mar 2002
 **********************************************************************/

void timer_diagnostics(void)
  {
  printf("=== Timer diagnostics ========================================\n");
  printf("Version:                   %s\n", GA_VERSION_STRING);
  printf("Build date:                %s\n", GA_BUILD_DATE_STRING);
  printf("Compilation machine characteristics:\n%s\n", GA_UNAME_STRING);

  printf("--------------------------------------------------------------\n");
  printf("structure                  sizeof\n");
  printf("chrono_t                   %lu\n", (unsigned long) sizeof(chrono_t));
  printf("==============================================================\n");

  return;
  }


/**********************************************************************
  timer_start()
  synopsis:	Set or reset timer.
  parameters:	chrono_t	t	The timer
  return:	none
  last updated:	06 Feb 2002
 **********************************************************************/

void timer_start(chrono_t *t)
  {
  t->begin_clock = t->save_clock = clock();
  t->begin_time = t->save_time = time(NULL);

  plog(LOG_NORMAL, "Timer started: %d", t->begin_clock);

  return;
  }


/**********************************************************************
  timer_check()
  synopsis:	Read timer.
  parameters:	chrono_t	t	The timer
  return:	user time in seconds.
  last updated:	06 Jun 2003
 **********************************************************************/

double timer_check(chrono_t *t)
  {
  double        user_time, real_time;
  clock_t	cclock = clock();
  time_t	ctime = time(NULL);

  plog(LOG_NORMAL, "Timer checked: %d", cclock);

  user_time = (cclock - t->save_clock) / (double) CLOCKS_PER_SEC;
  real_time = difftime(ctime, t->save_time);
  t->save_clock = ctime;
  t->save_time = cclock;

  plog(LOG_NORMAL, "User time: %f seconds.", user_time);
  plog(LOG_NORMAL, "Real time: %f seconds.", real_time);

  return user_time;
  }


/**********************************************************************
  SLang intrinsic wrappers.
  We can't use pointers from S-Lang, and structures are a pain in the
  posterior, so we use a 
 **********************************************************************/

#if HAVE_SLANG==1

THREAD_LOCK_DEFINE_STATIC(chrono_table_lock);
static TableStruct *chrono_table=NULL;        /* Table of timer handles. */

/**********************************************************************
  timer_new_slang()
  synopsis:	Allocate a new timer timer.
  parameters:	none
  return:	timer handle
  last updated:	07 Feb 2002
 **********************************************************************/

int timer_new_slang(void)
  {
  chrono_t	*t=s_malloc(sizeof(chrono_t));
  int		t_handle;

  THREAD_LOCK(chrono_table_lock);
  if (chrono_table==NULL) chrono_table=table_new();

  t_handle = table_add(chrono_table, (vpointer) t);
  THREAD_UNLOCK(chrono_table_lock);

  return (int) t_handle;
  }


/**********************************************************************
  timer_new_slang()
  synopsis:	Allocate a new timer timer.
  parameters:	none
  return:	timer handle
  last updated:	07 Feb 2002
 **********************************************************************/

void timer_free_slang(int *t_handle)
  {
  chrono_t       *t;

  THREAD_LOCK(chrono_table_lock);
  t = table_remove_index(chrono_table, *t_handle);
  THREAD_UNLOCK(chrono_table_lock);

  s_free(t);

  return;
  }


/**********************************************************************
  timer_start_slang()
  synopsis:	Set or reset timer.
  parameters:	chrono_t	t	The timer
  return:	none
  last updated:	07 Feb 2002
 **********************************************************************/

void timer_start_slang(int *t_handle)
  {
  chrono_t       *t;

  t = (chrono_t *) table_get_data(chrono_table, *t_handle);

  timer_start(t);

  return;
  }


/**********************************************************************
  timer_check_slang()
  synopsis:	Check timer.
  parameters:	chrono_t	t	The timer
  return:	user time in seconds.
  last updated:	07 Feb 2002
 **********************************************************************/

double timer_check_slang(int *t_handle)
  {
  chrono_t       *t;

  t = (chrono_t *) table_get_data(chrono_table, *t_handle);

  return timer_check(t);
  }

#endif	/* HAVE_SLANG==1 */

