/**********************************************************************
  util/gaul/gaul_config_win.h
 **********************************************************************

  gaul_config_win.h - GAUL configuration settings for windows.
  Copyright ©2003-2005, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:	GAUL configuration settings for windows.

        All windows-specific configuration should be done here.

 **********************************************************************/

#ifndef GAUL_CONFIG_WIN_H_INCLUDED
#define GAUL_CONFIG_WIN_H_INCLUDED 1

/**********************************************************************
 * Preparation for Windows DLL generation.
 **********************************************************************/

// wis - changes for Windows application generation
//#ifdef BUILDING_DLL
//#  define FUNCPROTO __declspec (dllexport)
//#else
//#  define FUNCPROTO __declspec (dllimport)
//#endif
//#else
//#  define FUNCPROTO
//#endif
#  define FUNCPROTO
// ~wis

#  if defined __CYGWIN32__ && !defined __CYGWIN__
#    define __CYGWIN__ __CYGWIN32__
#  endif

#define USE_WINDOWS_H 1

#ifndef _MSC_VER
#include <sys/time.h>
#endif
#include <sys/types.h>
#include <limits.h>
#include <string.h>
#include <io.h>

#ifdef _MSC_VER

/*
 * This section should be used for MS VC++.
 */
#  include <time.h>
#if _MSC_VER <= 1800 // Visual Studio 2013 and earlier
#  define vsnprintf _vsnprintf
#  define snprintf _snprintf
#endif
#  include <crtdbg.h>

#else

/*
 * This section should be used for DevC++, and possibly others.
 */
#  include <float.h>
#  include <unistd.h>
#  define HAVE_STDBOOL_H 1
#  define HAVE__BOOL 1

#endif

#if USE_WINDOWS_H == 1
#include <windows.h>
#endif

#ifndef errno
/* Some sytems #define this! */
extern int errno;
#endif

/*
 * These need to be modified for each release:
 */
#define GA_MAJOR_VERSION 0
#define GA_MINOR_VERSION 1848
#define GA_PATCH_VERSION 3
#define GA_VERSION_STRING "0.1848-3"
#define GA_UNAME_STRING "<Unknown windows machine>"
#define GA_BUILD_DATE_STRING "14/04/05"

/*
 * Functions available on this platform:
 */

#define HAVE_MEMCPY	1
#define HAVE_SINCOS	1
#define HAVE_STRDUP	1
#define HAVE_STRNDUP	1
#define HAVE_STRCPY	1
#define HAVE_STRNCPY	1
#define HAVE_STRCMP	1
#define HAVE_STRNCMP	1
#define HAVE_STRCASECMP	1
#define HAVE_STRNCASECMP	1
#define HAVE_USLEEP	1
#define HAVE_INDEX	1
#define HAVE_WAITPID	1
#define HAVE_STRSPN	1
#define HAVE_STRCSPN	1
#define HAVE_STRPBRK	1
#define HAVE_STRSEP	1
#define HAVE_STRLEN	1
#define HAVE_STRNCPY	1
#define HAVE_MEMSET 1
#define HAVE_MEMCMP 1

/*
 * Functions not available.
 */
#define HAVE_IPOW	0
#define HAVE_DPOW	0
#define HAVE_BCOPY	0
#define HAVE_BCMP	0
#define HAVE_SNOOZE	0

/*
 * These are defined if windows.h is included:
 */
#if USE_WINDOWS_H == 1
#define HAVE_MIN	1
#define HAVE_MAX	1
#define HAVE_GETHOSTNAME	1
#else
#define HAVE_MIN	0
#define HAVE_MAX	0
#define HAVE_GETHOSTNAME	0
#endif

/*
 * Default constants:
 * These ones probably shouldn't be changed:
 */

#define W32_CRIPPLED 1
#define AVLTREE_KEY_TYPE void *
#define GA_NUM_PROCESSES_ENVVAR_STRING "GA_NUM_PROCESSES"
#define GA_NUM_THREADS_ENVVAR_STRING "GA_NUM_THREADS"
#define USE_CHROMO_CHUNKS 0
#define USE_OPTIMISED_MEMSET 1

#define MEMORY_ALLOC_SAFE 1
#define MEMORY_ALLOC_DEBUG 0

/*
 * These might need to be changed:
 */

#define HAVE_SLANG 0
#define HAVE_MPI 0
#define HAVE_PTHREADS 0
#define USE_OPENMP 0 // wis OpenMP not yet working
#define GAUL_DETERMINISTIC_OPENMP 0

#endif /* GAUL_CONFIG_WIN_H_INCLUDED */

