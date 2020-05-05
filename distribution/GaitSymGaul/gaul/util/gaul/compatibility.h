/**********************************************************************
  compatibility.h
 **********************************************************************

  compatibility - Compatibility/Portability stuff.
  Copyright ©2000-2004, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:	Compatibility/Portability stuff.

 **********************************************************************/

#ifndef COMPATIBILITY_H_INCLUDED
#define COMPATIBILITY_H_INCLUDED

#include "gaul/gaul_util.h"

/* Includes */
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>		/* usleep, fcntl */
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include "gaul/memory_util.h"

/*
 * Debugging
 */
#ifndef COMPAT_DEBUG
#define COMPAT_DEBUG   2
#endif

/*
 * Prototypes
 */

#if HAVE_IPOW != 1
FUNCPROTO int ipow(int n, int e);
#endif

#if HAVE_DPOW != 1
FUNCPROTO double dpow(double n, int e);
#endif

#if HAVE_STRCHR != 1
# if HAVE_INDEX == 1
#  define strchr index
# else
FUNCPROTO char *strchr(const char *str, int c);
# endif
#endif

#if HAVE_STRLEN != 1
FUNCPROTO size_t strlen(const char *str);
#endif

#if HAVE_STRCMP != 1
FUNCPROTO int strcmp(const char *str1, const char *str2);
#endif

#if HAVE_STRNCMP != 1
FUNCPROTO int strncmp(const char *str1, const char *str2, size_t len);
#endif

#if HAVE_STRCPY != 1
FUNCPROTO char *strcpy(char *str1, const char *str2);
#endif

#if HAVE_STRNCPY != 1
FUNCPROTO char *strncpy(char *str1, const char *str2, size_t len);
#endif

#if HAVE_STRPBRK != 1
FUNCPROTO char *strpbrk(const char *s, const char *accept);
#endif

#if HAVE_STRSEP != 1
FUNCPROTO char *strsep(char **str, const char *delim);
#endif

#if HAVE_STRCASECMP != 1
FUNCPROTO int strcasecmp(const char *str0, const char *str1);
#endif

#if HAVE_STRNCASECMP != 1
FUNCPROTO int strncasecmp(const char *str0, const char *str1, size_t n);
#endif

#if HAVE_USLEEP != 1
FUNCPROTO void usleep(unsigned long usec);
#endif

#if HAVE_MEMCPY != 1
/* Some systems, such as SunOS do have BCOPY instead. */
# if HAVE_BCOPY == 1
#  define memcpy(A, B, C) bcopy((B), (A), (C))
# else
FUNCPROTO void memcpy(char *dest, const char *src, size_t len);
# endif
#endif

#if HAVE_MEMCMP != 1
/* Some systems, such as SunOS do have BCMP instead. */
# if HAVE_BCMP == 1
#  define memcmp(A, B, C) bcmp((B), (A), (C))
# else
FUNCPROTO int memcmp(const void *src1, const void *src2, size_t n);
# endif
#endif

#if HAVE_STRDUP != 1
FUNCPROTO char *strdup(const char *str);
#endif

#if 0
#if HAVE_STRNDUP != 1
FUNCPROTO char *strndup(const char *str, size_t n);
#endif
#endif

#if HAVE_DIEF != 1
/*
 * HAVE_DIEF is set in "SAA_header.h", not "config.h"
 */
FUNCPROTO void dief(const char *format, ...);
#endif

#if HAVE_WAITPID != 1 && !defined( W32_CRIPPLED )
/* FIXME: Need to add a Windows compatible version of this function. */
FUNCPROTO pid_t waitpid(pid_t pid, int *pstatus, int options);
#endif

#if HAVE_MIN != 1
FUNCPROTO int min(int a, int b);
#endif

#if HAVE_MAX != 1
FUNCPROTO int max(int a, int b);
#endif

#if HAVE_SINCOS != 1
FUNCPROTO void sincos( double radians, double *s, double *c );
#endif

#if HAVE_GETHOSTNAME != 1
FUNCPROTO int gethostname(char *name, size_t len);
#endif

#endif /* COMPATIBILITY_H_INCLUDED */


