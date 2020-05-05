/**********************************************************************
  memory_util.h
 **********************************************************************

  memory_util - Usage control wrapper around standard malloc() etc.
  Copyright ©1999-2005, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:	Header file for memory_methods

 ********************************************************************/

#ifndef MEMORY_UTIL_H_INCLUDED
#define MEMORY_UTIL_H_INCLUDED

#include "gaul/gaul_util.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "gaul/memory_chunks.h"
#include "gaul/avltree.h"

/*
 * Debugging.
 */
#ifndef MEMORY_DEBUG
# ifdef DEBUG
#  define MEMORY_DEBUG DEBUG
# else
#  define MEMORY_DEBUG 0
# endif
#endif

/* Take advantage of the GNU malloc checking features. */
#ifdef __GNUC__
#if MEMORY_ALLOC_DEBUG==1
#define	MALLOC_CHECK_	2
#endif
#endif

/*
 * Set some defaults.
 */
#ifndef MEMORY_ALLOC_DEBUG
# ifndef MEMORY_ALLOC_SAFE
#  define MEMORY_ALLOC_DEBUG	0
#  define MEMORY_ALLOC_SAFE	1
# else
#  define MEMORY_ALLOC_DEBUG	(0==MEMORY_ALLOC_SAFE)
# endif
#else
# ifndef MEMORY_ALLOC_SAFE
#  define MEMORY_ALLOC_SAFE	(0==MEMORY_ALLOC_DEBUG)
# endif
#endif

/*
 * Definitions for s_alloc_debug()
 */
typedef enum memory_alloc_type_t
  {
  MEMORY_UNKNOWN=0,
  MEMORY_MALLOC, MEMORY_CALLOC, MEMORY_REALLOC,
  MEMORY_STRDUP, MEMORY_STRNDUP
  } memory_alloc_type;

/*
 * Memory allocation macros.
 * Use "MEMORY_ALLOC_DEBUG" and "MEMORY_ALLOC_SAFE" for selecting between
 * these memory routines and the system memory routines.
 * MEMORY_ALLOC_DEBUG = use my replacement functions for malloc() etc.
 * MEMORY_ALLOC_SAFE = use simple success test wrappers around system calls.
 * if both are defined, "MEMORY_ALLOC_DEBUG" over-rides.
 * if neither is defined, standard system functions will be used directly.
 */
#if MEMORY_ALLOC_DEBUG==1
/* Use replacement functions with extensive checking, tracing and profiling. */

#define s_malloc(X)	s_alloc_debug(		MEMORY_MALLOC,		\
						(X),	1,	NULL,	\
						__PRETTY_FUNCTION__,	\
						__FILE__,		\
						__LINE__,		\
						"malloc")
#define s_calloc(X,Y)	s_alloc_debug(		MEMORY_CALLOC,		\
						(X),	(Y),	NULL,	\
						__PRETTY_FUNCTION__,	\
						__FILE__,		\
						__LINE__,		\
						"calloc")
#define s_realloc(X,Y)	s_alloc_debug(		MEMORY_REALLOC,		\
						(Y),	1,	(X),	\
						__PRETTY_FUNCTION__,	\
						__FILE__,		\
						__LINE__,		\
						"realloc")
#define s_malloc_labelled(X, Y)	s_alloc_debug(	MEMORY_MALLOC,		\
						(X),	1,	NULL,	\
						__PRETTY_FUNCTION__,	\
						__FILE__,		\
						__LINE__,		\
						(Y))
#define s_strdup(X)	(char *) s_alloc_debug(	MEMORY_STRDUP,		\
						(size_t) strlen((X)),	\
						1,	(X),		\
						__PRETTY_FUNCTION__,	\
						__FILE__,		\
						__LINE__,		\
						"strdup")
#define s_strndup(X, Y)	(char *) s_alloc_debug(	MEMORY_STRNDUP,		\
						(Y), 1,	(X),		\
						__PRETTY_FUNCTION__,	\
						__FILE__,		\
						__LINE__,		\
						"strndup")
#define s_free(X)	s_free_debug(		(X),			\
						__PRETTY_FUNCTION__,    \
                                                __FILE__,               \
                                                __LINE__)

#else /* MEMORY_ALLOC_DEBUG */
#if MEMORY_ALLOC_SAFE==1
/* Use simple wrappers around system calls. */

#define s_malloc(X)		s_malloc_safe((X),			\
				__PRETTY_FUNCTION__, __FILE__, __LINE__)
#define s_calloc(X,Y)		s_calloc_safe((X),(Y),			\
				__PRETTY_FUNCTION__, __FILE__, __LINE__)
#define s_realloc(X,Y)		s_realloc_safe((X),(Y),			\
				__PRETTY_FUNCTION__, __FILE__, __LINE__)
#define s_malloc_labelled(X, Y)	s_malloc_safe((X),			\
				__PRETTY_FUNCTION__, __FILE__, __LINE__)
#define s_strndup(X, Y)		s_strndup_safe((X), (Y),		\
				__PRETTY_FUNCTION__, __FILE__, __LINE__)
#define s_strdup(X)		s_strdup_safe((X),			\
				__PRETTY_FUNCTION__, __FILE__, __LINE__)
#define s_free(X)		s_free_safe((X),			\
				__PRETTY_FUNCTION__, __FILE__, __LINE__)

#else /* MEMORY_ALLOC_SAFE */
/* Use standard system calls with no wrappers. */

#define s_malloc(X)		malloc((X))
#define s_calloc(X,Y)		calloc((X),(Y))
#define s_realloc(X,Y)		realloc((X),(Y))
#define s_malloc_labelled(X, Y)	malloc((X))
#define s_strdup(X)		strdup((X))
/* Many systems don't have a strndup() function, so we just avoid the problem completely! */
#define s_strndup(X, Y)		s_strndup_safe((X), (Y),		\
				__PRETTY_FUNCTION__, __FILE__, __LINE__)
#define s_free(X)		free((X))

#endif /* MEMORY_ALLOC_SAFE */
#endif /* MEMORY_ALLOC_DEBUG */

/*
 * Prototypes
 */
FUNCPROTO void	memory_init_openmp(void);

FUNCPROTO void	memory_open_log(const char *fname);
FUNCPROTO void	memory_write_log(const char *text);

FUNCPROTO void	memory_display_status(void);
FUNCPROTO void	memory_display_table(void);

FUNCPROTO int	memory_total(void);
FUNCPROTO void	memory_print_alloc_to(void *pnt);
FUNCPROTO int	memory_alloc_to(void *pnt);
FUNCPROTO void	memory_set_pnt_label(void *pnt, char *label);
FUNCPROTO int	memory_check_all_bounds(void);
FUNCPROTO int	memory_check_bounds(void *pnt);
FUNCPROTO void	memory_set_strict(int i);
FUNCPROTO boolean	memory_set_bounds(int i);
FUNCPROTO void	memory_set_verbose(int i);

/*
 * Prototypes for [unrecommended] direct access to functions.
 */

/*
 * Actual Malloc/Calloc/Realloc/Strdup/Free replacements.
 * Debug versions.
 */
FUNCPROTO void	*s_alloc_debug(memory_alloc_type, size_t, int, void*, const char*, const char*, const int, const char*);
FUNCPROTO void	*s_free_debug(void*, const char*, const char*, const int);

/*
 * System Malloc/Calloc/Realloc/Strdup/Free calls with wrappers.
 * Safe versions.
 */
FUNCPROTO void	*s_malloc_safe(size_t, const char*, const char*, const int);
FUNCPROTO void	*s_calloc_safe(size_t, size_t, const char*, const char*, const int);
FUNCPROTO void	*s_realloc_safe(void*, size_t, const char*, const char*, const int);
FUNCPROTO char	*s_strdup_safe(const char*, const char*, const char*, const int);
FUNCPROTO char	*s_strndup_safe(const char*, size_t, const char*, const char*, const int);
FUNCPROTO void	s_free_safe(void*, const char*, const char*, const int);

#endif

