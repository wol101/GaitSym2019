/**********************************************************************
  log_util.c
 **********************************************************************

  log_util - general logging services.
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

  Synopsis:	Portable routines for logging and debug messages.

        These functions can be tested by compiling with
        something like:
        gcc -o testlog log_util.c -DLOG_UTIL_TEST

        These functions are thread-safe.

  To do:	Seperate levels for callback, file, stderr outputs.
        Validate the logging level when set.
        Seperate levels/files/whatever possible for seperate files.
        Option for outputing the time.

 **********************************************************************/

#include "gaul/log_util.h"

/*
 * Global variables.
 *
 * NB/ log_filename and log_callback are meaningless on non-GNU C systems.
 */
static char		*log_filename=NULL;		/* Log filename */
static log_func		log_callback=NULL;		/* Callback function for log */
static enum log_level_type	log_level=LOG_NONE;	/* Logging level */
static boolean		log_date=TRUE;			/* Whether to display date in logs */

THREAD_LOCK_DEFINE_STATIC(gaul_log_callback_lock);
THREAD_LOCK_DEFINE_STATIC(gaul_log_global_lock);
THREAD_LOCK_DEFINE_STATIC(gaul_log_level_lock);

#if HAVE_MPI == 1
static int mpi_get_rank(void)
  {
  int	rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  return rank;
  }
#endif


/**********************************************************************
  log_init()
  synopsis:	Initialise logging facilities.  If func and fname are
        both NULL, use stdout instead.
  parameters:	int	level	Logging level.
        char	*fname	Filename, or NULL.
        log_func	func	Callback function, or NULL.
  return:	none
  last updated:	26 Feb 2002
 **********************************************************************/

void log_init(	enum log_level_type	level,
            char			*fname,
            log_func		func,
            boolean			date)
  {
  char	*oldfname=NULL;

  THREAD_LOCK(gaul_log_global_lock);
  log_level = level;
  if (fname)
    {
    if (log_filename != fname) oldfname = log_filename;
    log_filename = s_strdup(fname);
    }
  log_date = date;
  THREAD_UNLOCK(gaul_log_global_lock);

  THREAD_LOCK(gaul_log_callback_lock);
    log_callback = func;
  THREAD_UNLOCK(gaul_log_callback_lock);

  if (oldfname) s_free(oldfname);

#if HAVE_MPI == 1
  plog(LOG_VERBOSE, "Log started. (parallel with MPI)");
#else
  plog(LOG_VERBOSE, "Log started.");
#endif
  plog(LOG_DEBUG, "Debug output activated.");

  return;
  }


/**********************************************************************
  log_set_level()
  synopsis:	Adjust logging level.
  parameters:	int	level	New logging level.
  return:	none
  last updated:	07/05/00
 **********************************************************************/

void log_set_level(const enum log_level_type level)
  {
  THREAD_LOCK(gaul_log_level_lock);
  log_level = level;
  plog(LOG_VERBOSE, "Log level adjusted to %d.", level);
  THREAD_UNLOCK(gaul_log_level_lock);

  return;
  }


/**********************************************************************
  log_get_level()
  synopsis:	Return logging level.
  parameters:	none
  return:	log_level
  last updated:	28/05/00
 **********************************************************************/

enum log_level_type log_get_level(void)
  {
  return log_level;
  }


/**********************************************************************
  log_set_file()
  synopsis:	Adjust log file.
  parameters:	const char *fname	Filename for output.
  return:	none
  last updated:	31 Jan 2002
 **********************************************************************/

void log_set_file(const char *fname)
  {
  char	*oldfname=NULL;

  THREAD_LOCK(gaul_log_global_lock);
  if (log_filename != fname) oldfname = log_filename;
  log_filename = s_strdup(fname);
  THREAD_UNLOCK(gaul_log_global_lock);

  if (oldfname) s_free(oldfname);

  plog(LOG_VERBOSE, "Log file adjusted to \"%s\".", fname);

  return;
  }


/**********************************************************************
  log_output()
  synopsis:	If log level is appropriate, append message to log
        file.  log_init() should really be called prior
        to the first use of the function, although nothing will
        break if you don't.
  parameters:	int	level	Logging level.
        char		format	Format string.
        ...		Variable args.
  return:       none
  last updated: 21/06/00
 **********************************************************************/

void log_output(	const enum	log_level_type level,
            const char	*func_name,
            const char	*file_name,
            const int	line_num,
            const char	*format, ...)
  {
  va_list	ap;				/* variable args structure */
  char		message[LOG_MAX_LEN];	/* The text to write */
  FILE		*fh;				/* File handle */
/* FIXME: Needs to be more general */
  const char    log_text[7][10] = {"????: ", "FATAL: ", "WARNING: ", "", "", "FIXME: ", "DEBUG: "};
  time_t	t;				/* Time structure */

  t = time(&t);	/* Less than optimal when no time display is required. */
        /* I tried variations on ctime(time()), withou success. */

/*
 * Should message be dropped?
 * This test is only required if this function was called without the macro
 * wrapper - i.e. this is a non-GNU compiler.
 */
#if !defined(__GNUC__) && !defined(__INTEL_COMPILER)
  if (level > log_level) return;
#endif

/* Generate actual message string */
  va_start(ap, format);
  vsnprintf(message, LOG_MAX_LEN, format, ap);
  va_end(ap);

/* Call a callback? */
  THREAD_LOCK(gaul_log_callback_lock);
  if (log_callback)
    log_callback(level, func_name, file_name, line_num, message);
  THREAD_UNLOCK(gaul_log_callback_lock);

/* Write to file? */
  THREAD_LOCK(gaul_log_global_lock);
  if (log_filename)
    {
    if ( !(fh=fopen(log_filename, "a+")) )
      {
      fprintf(stdout, "FATAL: Unable to open logfile \"%s\" for appending.\n", log_filename);
      abort();	/* FIXME: Find more elegant method */
      }

#if HAVE_MPI == 1
    fprintf(fh, "%d: %s%s%s%s\n",
             mpi_get_rank(),
             log_date?"":ctime(&t), log_date?"":" - ",
             log_text[level], message);
#else
    fprintf(fh, "%s%s%s%s\n",
             log_date?"":ctime(&t), log_date?"":" - ",
             log_text[level], message);
#endif

    if (level >= LOG_FIXME)
      fprintf(fh, "   in %s(), \"%s\", line %d\n",
                       func_name, file_name, line_num);

/*    fflush(fh);*/
    fclose(fh);
    }
  THREAD_UNLOCK(gaul_log_global_lock);

/* Write to stdout? */
  if ( !(log_callback || log_filename) )
    {
#if HAVE_MPI == 1
    if (mpi_get_rank() >= 0)
      fprintf(stdout, "%d: %s%s%s%s\n", mpi_get_rank(),
              log_date?"":ctime(&t), log_date?"":" - ",
              log_text[level], message);
    else
#endif
      fprintf(stdout, "%s%s%s%s\n",
              log_date?"":ctime(&t), log_date?"":" - ",
              log_text[level], message);

    if (level >= LOG_FIXME)
      fprintf(stdout, "   in %s(), \"%s\", line %d\n",
                       func_name, file_name, line_num);

    /*fprintf(stderr, "%s%s\n", log_text[level], message);*/
    fflush(stdout);
    }

  return;
  }


#if !( ( defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(_MSC_VER) ) && !defined(__APPLE_CPP__) && !defined(__APPLE_CC__) )
/*
 * This is a reduced form of the above function for non-GNU systems.
 */
void plog(const enum log_level_type level, const char *format, ...)
  {
  va_list       ap;                             /* variable args structure */
  char          message[LOG_MAX_LEN];           /* The text to write */
  const char    log_text[7][10] = {"????: ", "FATAL: ", "WARNING: ", "", "", "FIXME: ", "DEBUG: "};
  time_t        t;                              /* Time structure */

  t = time(&t);

/* Call a callback? */
  THREAD_LOCK(gaul_log_callback_lock);
  if (log_callback)
    log_callback(level, "unknown", "unknown", 0, message);
  THREAD_UNLOCK(gaul_log_callback_lock);

  if ( (level) <= log_level )
    {
    va_start(ap, format);
    vsnprintf(message, LOG_MAX_LEN, format, ap);
    va_end(ap);

#if HAVE_MPI == 1
    if (mpi_get_rank() >= 0)
      printf( "%d: %s%s%s%s\n", mpi_get_rank(),
              log_date?"":ctime(&t), log_date?"":" - ",
              log_text[level], message );
    else
#endif
      printf( "%s%s%s%s\n",
              log_date?"":ctime(&t), log_date?"":" - ",
              log_text[level], message );
    }
  return;
  }
#endif

/*
 * SLang intrinsic wrappers.
 */
#if HAVE_SLANG==1
void log_wrapper(int *level, char *message)
  {
/*
  unsigned int	num = SLang_Num_Function_Args;
*/
  const char	log_text[7][10] = {"?????: ", "FATAL: ", "WARNING: ", "", "", "FIXME: ", "DEBUG: " };
  time_t	t;				/* Time structure. */

  t = time(&t);

/* Call a callback? */
  THREAD_LOCK(gaul_log_callback_lock);
  if (log_callback)
    log_callback(*level, "[SLang]", "unknown", 0, message);
  THREAD_UNLOCK(gaul_log_callback_lock);

  if ( *level <= log_level )
    {
    printf("%s%s%s%s\n",
              log_date?"":ctime(&t), log_date?"":" - ",
              log_text[*level], message );
    }

  return;
  }
#endif	/* HAVE_SLANG==1 */

