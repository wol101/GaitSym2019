/**********************************************************************
  compatibility.c
 **********************************************************************

  compatibility - Compatibility/Portability stuff.
  Copyright ©2000-2004, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:	Compatibility/portability functions.

                Note that some of these functions are POSIX
                compliant. Some are ANSI compliant.  Some are just
                plain non-standard.

 **********************************************************************/

#include "gaul/compatibility.h"

#if HAVE_IPOW != 1
/*
 * Integer power.
 */
int ipow(int n, int e)
  {
  int	result=1;	/* The answer. */

  while (e>0)
    {
    result*=n;
    e--;
    }

  return result;
  }
#endif	/* HAVE_IPOW */


#if HAVE_DPOW != 1
/*
 * Double to integer power.
 */
double dpow(double n, int e)
  {
  double	result=1.0;	/* The answer. */

  while (e>0)
    {
    result*=n;
    e--;
    }

  return result;
  }
#endif	/* HAVE_DPOW */


#if HAVE_MEMCPY != 1
#if HAVE_BCOPY != 1
/*
 * Copy LEN characters from SRC to DEST
 *
 * Some systems, such as SunOS do have BCOPY instead.
 * In which case this is defined as a macro in the header.
 */
void memcpy(char *dest, const char *src, size_t len)
  {
  char		*dest_p;
  const	char	*src_p;
  int		byte_c;

  if (len <= 0)
    return;

  src_p = src;
  dest_p = dest;

  if (src_p <= dest_p && src_p + (len - 1) >= dest_p)
    {
    /* overlap, must copy right-to-left. */
    src_p += len - 1;
    dest_p += len - 1;
    for (byte_c = 0; byte_c < len; byte_c++)
      *dest_p-- = *src_p--;
    }
  else
    {
    for (byte_c = 0; byte_c < len; byte_c++)
      *dest_p++ = *src_p++;
    }

  return;
  }
#endif /* HAVE_BCOPY */
#endif /* HAVE_MEMCPY */


#if HAVE_STRCHR != 1
#if HAVE_INDEX != 1
/*
 * Find C in STR by searching through the string
 */
char *strchr(const char *str, int c)
  {

  while ( *str != '\0' )
    {
    if (*str == (char)c)
      return (char *)str;
    str++;
    }

  if (c == '\0')
    return (char *)str;

  return NULL;
  }
#endif /* HAVE_INDEX */
#endif /* HAVE_STRCHR */


#if HAVE_STRLEN  != 1
/*
 * Return the length in characters of STR
 */
size_t strlen(const char *str)
  {
  int	len=0;

  while ( *str != '\0' )
    {
    str++;
    len++;
    }

  return len;
  }
#endif /* HAVE_STRLEN */


#if HAVE_STRCMP != 1
/*
 * Compare str1 and str2.
 *
 * It returns -1, 0 or 1 if str1 is found, to be less than, to be
 * equal to, or be greater than str2, respectively.
 */
int strcmp(const char *str1, const char *str2)
  {

  while (*str1 != '\0' && *str1 == *str2)
    {
    str1++;
    str2++;
    }

  return *str1 - *str2;
  }
#endif /* HAVE_STRCMP */


#if HAVE_STRNCMP != 1
/*
 * Compare at most len characters of str1 and str2.
 *
 * It returns -1, 0 or 1 if str1 is found, to be less than, to be
 * equal to, or be greater than str2, respectively.
 */
int strncmp(const char *str1, const char *str2, size_t len)
  {
  int	c=0;

  while ( c < len )
    {
    if (*str1 != *str2 || *str1 == '\0')
      return *str1 - *str2;
    c++;
    str1++;
    str2++;
    }

  return 0;
  }
#endif /* HAVE_STRNCMP */


#if HAVE_STRCPY != 1
/*
 * Copies str2 to str1.
 *
 * Returns str1.
 */
char *strcpy(char *str1, const char *str2)
  {
  char	*str_p;

  str_p = str1;
  while (*str2 != '\0')
    {
    *str_p = *str2;
    str_p++;
    str2++;
    }

  *str_p = '\0';

  return str1;
  }
#endif /* HAVE_STRCPY */


#if HAVE_STRNCPY != 1
/*
 * Copy str2 to str1 until len characters copied, or a null
 * character is found in str2.
 *
 * Returns str1.
 */
char	*strncpy(char *str1, const char *str2, size_t len)
  {
  char		*str1_p, null_reached = FALSE;
  int		len_c;

  for (len_c = 0, str1_p = str1; len_c < len && !null_reached; len_c++, str1_p++, str2++)
    {
    if (null_reached || *str2 == '\0')
      {
      null_reached = TRUE;
      *str1_p = '\0';
      }
    else
      {
      *str1_p = *str2;
      }
    }

  return str1;
  }
#endif /* HAVE_STRNCPY */


#if HAVE_STRPBRK != 1
/*
 * Locate the first occurrence in the string s of any of the characters in the string accept.
 */
char *strpbrk(const char *s, const char *accept)
  {
  const char *s1;
  const char *s2;

  for (s1 = s; *s1 != '\0'; ++s1)
    {
    for (s2 = accept; *s2 != '\0'; ++s2)
      {
      if (*s1 == *s2) return (char *) s1;
      }
    }

  return NULL;
  }
#endif /* HAVE_STRPBRK */


#if HAVE_STRSEP != 1
/*
 * If *str is NULL, return NULL.  Otherwise, this find the first token
 * in the string *str, where tokens are delimited by symbols in the
 * string delim.  This token is terminated with a `\0' character (by
 * overwriting the delimiter) and *str is updated to point past the
 * token.  If no delimiter is found, the token is taken to be the
 * entire string *str, and *str is made NULL.
 *
 * Returns a pointer to the token (i.e returns the original value of *str)
 *
 * The strsep() function was introduced as a replacement for strtok(),
 * which cannot handle empty fields.
 */
char *strsep(char **str, const char *delim)
  {
  char *s = *str, *end;

  if (!s) return NULL;

  end = strpbrk(s, delim);
  if (end) *end++ = '\0';
  *str = end;

  return s;
  }
#endif /* HAVE_STRSEP */


#if HAVE_STRCASECMP != 1
/*
 * Compare strings like strncmp(), but ignoring case.
 */
int strcasecmp(const char *str0, const char *str1)
  {

  while( tolower(*str0)==tolower(*str1) )
    {
    if(*str0=='\0')
      return 0;
    str0++;
    str1++;
    }

  return tolower(*str0)-tolower(*str1);
  }
#endif /* HAVE_STRCASECMP */


#if HAVE_STRNCASECMP != 1
/*
 * Compare strings like strncmp(), but ignoring case.
 * ie, only compares first n chars.
 */

int strncasecmp(const char *str0, const char *str1, size_t n)
  {
  size_t i;

  for(i=0; i<n && tolower(*str0)==tolower(*str1); i++, str0++, str1++)
    if(*str0=='\0')
      return 0;

  return (i==n) ? 0 : (tolower(*str0)-tolower(*str1));
  }
#endif /* HAVE_STRNCASECMP */


#if HAVE_USLEEP != 1
/*
 * Sleep for a specified number of microseconds.
 * -- Should replace with the POSIX standard's nanosleep().
 */

void usleep(unsigned long usec)
  {
#if W32_CRIPPLED == 1
# if USE_WINDOWS_H == 1
  Sleep(usec/1000);		/* Windows sleep is in milliseconds. */
#else
  int	i;
  for (i=0; i<usec*10; i++);	/* A completely dodgy kludge.  Just introduces an arbitrary length delay. */
# endif
#else
  #if HAVE_SNOOZE == 1		/* i.e. BeOS, AtheOS, Syllable. */
  snooze(usec/1000);		/* BeOS sleep is in milliseconds. */
  #else
  struct timeval tv;
  tv.tv_sec=0;
  tv.tv_usec=usec;
  select(0, NULL, NULL, NULL, &tv);
  #endif
#endif

  return;
  }
#endif /* HAVE_USLEEP */


#if HAVE_MEMSET != 1
/*
 * Set LEN characters in STR to character C
 */
#if USE_OPTIMISED_MEMSET != 1
/* Original version.  Must use this on Solaris, by the looks of things. */
void *memset(void *str, int c, size_t len)
  {
  char	*orig = str;

  for (; len > 0; len--, str++) *(char *)str = (char)c;

  return orig;
  }

#else

/* Optimised version */
/*
 * Copyright (c) 1990, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Hibler and Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
void *memset(void *dst0, int c0, size_t bytes)
  {
    size_t t;
    unsigned int c;
    unsigned char *dst;
    const int word_size = sizeof(unsigned int);
    const int word_mask = (sizeof(unsigned int) - 1);

    dst = dst0;

    /* if not enough words for a reasonable speedup, just fill bytes */
    if (bytes < 3 * word_size) {
        while (bytes != 0) {
            *dst++ = c0;
            --bytes;
        }
        return dst0;
    }

    /* fill the whole stamping word */
    if ((c = (unsigned char)c0) != 0) {
        c = c | (c << 8);
#if (SIZEOF_INT > 2)
        c = c | (c << 16);
#endif
#if (SIZEOF_INT > 4)
        c = c | (c << 32);
#endif
    }

    /* align destination by filling in bytes */
    if ((t = (long)dst & word_mask) != 0) {
        t = word_size - t;
        bytes -= t;
        do {
            *dst++ = c0;
        } while (--t != 0);
    }

    /* now fill with words. length was >= 2*words so we know t >= 1 here */
    t = bytes / word_size;
    do {
        *(unsigned int *)dst = c;
        dst += word_size;
    } while (--t != 0);

    /* finish with trailing bytes, if there are bytes left */
    t = bytes & word_mask;
    if (t != 0) {
        do {
            *dst++ = c0;
        } while (--t != 0);
    }

  return dst0;
  }
#endif
#endif /* HAVE_MEMSET */


#if HAVE_MEMCMP != 1
#if HAVE_BCMP != 1
/*
 * Some systems, such as SunOS do have BCMP instead.
 * In which case this is defined as a macro in the header.
 */
int memcmp(const void *src1, const void *src2, size_t n)
  {
  const unsigned char *cp1=src1;
  const unsigned char *cp2=src2;

  while (n-- > 0) if (*cp1++ != *cp2++) return (*--cp1 - *--cp2);

  return 0;
  }
#endif /* HAVE_BCMP */
#endif /* HAVE_MEMCMP */


#if HAVE_STRDUP != 1
char *strdup(const char *str)
  {
  char *new_str;

  if (!str) return NULL;

  new_str = s_malloc(sizeof(char)*(strlen(str)+1));
  strcpy(new_str, str);

  return new_str;
  }
#endif /* HAVE_STRDUP */

#if 0
#if HAVE_STRNDUP != 1
char *strndup(const char *str, size_t n)
  {
  char *new_str=NULL;

  if (str)
    {
    new_str = (char *)s_malloc(sizeof(char)*(n+1));
    strncpy(new_str, str, n);
    new_str[n] = '\0';
    }

  return new_str;
  }
#endif /* HAVE_STRNDUP */
#endif

#if HAVE_DIEF != 1
/*
 * Needed as a function because many compilers don't use vararg macros.
 * HAVE_DIEF is set in "SAA_header.h", not "config.h".
 */
void dief(const char *format, ...)
  {
  va_list       ap;                        /* variable args structure */

  printf("FATAL ERROR: ");
  va_start(ap, format);
  vprintf(format, ap);
  va_end(ap);
  printf("\n");

  abort();
  }
#endif /* HAVE_DIEF */


#if HAVE_WAITPID != 1 && !defined( W32_CRIPPLED )
pid_t waitpid(pid_t pid, int *pstatus, int options)
  {
  pid_t result;

  do
    {
    result = wait(pstatus);
    } while (result >= 0 && result != pid);

  return result;
  }
#endif /* HAVE_WAITPID */


#if HAVE_MIN != 1
int min( int a, int b )
  {
  return a <= b ? a : b;
  }
#endif


#if HAVE_MAX != 1
int max( int a, int b )
  {
  return a >= b ? a : b;
  }
#endif


#if 0 // this breaks ubuntu and is not actually very useful
#if HAVE_SINCOS != 1
/*
 * This is an undocumented GNU extension, which is actually fairly useful.
 */
void sincos( double radians, double *s, double *c )
  {

#if __i368__
  __asm__ ("fsincos" : "=t" (*c), "=u" (*s) : "0" (radians));
#else
  *s = sin(radians);
  *c = cos(radians);
#endif

/*printf("DEBUG: sincos(%f) = %f %f\n", radians, *s, *c);*/

  return;
  }
#endif /* HAVE_SINCOS */
#endif

#if HAVE_GETHOSTNAME != 1
/*
 * gethostname() - get host name

   This function is used to access the host name of the current processor.  It
   returns a NULL-terminated hostname by filling the array name with a length of
   len bytes.  In case the NULL-terminated hostname does not fit, no error is
   returned, but the hostname is truncated.  In the specification, it is
   unspecified whether the truncated hostname will be NULL-terminated, but here
   it is.

   What should happen is that upon success, zero is returned.  On error, -1
   is returned, and errno is set to EINVAL.  But here the hostname is set to
   <unknown>.

  From the manpage:
       EINVAL len is negative or, for sethostname, len is larger than the
              maximum allowed size, or, for gethostname on Linux/i386, len
              is smaller than the actual size.  (In this last case glibc 2.1
              uses ENAMETOOLONG.)

   SUSv2 guarantees that host names are limited to 255 bytes.  POSIX
   1003.1-2001 guarantees that host names (not including the terminating
   NUL) are limited to HOST_NAME_MAX bytes.

 */

int gethostname(char *name, size_t len)
  {

  snprintf(name, len, "<unknown>");

  return TRUE;
  }
#endif /* HAVE_GETHOSTNAME */

