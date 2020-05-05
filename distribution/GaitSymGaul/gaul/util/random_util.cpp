/**********************************************************************
  random_util.c
 **********************************************************************

  random_util - Pseudo-random number generation routines.
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

  Synopsis:	Portable routines for generating pseudo random numbers.

		Why use these instead of the system routines?
		(a) Useful interface functions. (can specify ranges or
		specific distributions)
		(b) System independant. (Generated numbers are
		reproducible across system types.)
		(c) Enables saving and restoring state.

		SLang intrinsic function wrappers are provided if the
		HAVE_SLANG constant is set to 1.

		The algorithm I selected is the Mitchell and Moore
		variant of the standard additive number generator.
		The required array of numbers is populated by a
		using single seed value by using a linear
		congruential pseudo random number generator.

		This routines have been tested and provide the same
		output (within the limits of computational precision)
		on (at least) the following platforms:

		o Intel x86 (Linux, OpenBSD, FreeBSD)
		o AMD x86-64 (Linux, FreeBSD)
		o IBM Power3 (AIX)
		o IBM PowerPC (Linux)
		o MIPS R4K, R10K, R12K (IRIX)
		o Alpha EV56 (Tru64), EV7 (Linux)
		o SPARC Ultra-4 (Solaris)

		This code should be thread safe.

		For OpenMP code, USE_OPENMP must be defined and 
		random_init() or random_seed() must be called BY A
		SINGLE THREAD prior to any other function.
		
  References:	Standard additive number generator and the linear
		congruential algorithm:
		Knuth D.E., "The Art of Computer Programming",
		Vol 2, 2nd ed. (1998)

		General information:
		Press W., Flannery B.P., Teukolsky S.A., Vetterling W.T.,
		"Numerical Recipes in C: The Art of Scientific Computing"
		Cambridge University Press, 2nd ed. (1992)

  Usage:	o First call random_init().

		o Call random_seed() to seed the PRNG (like srand()).
		  Alternatively, random_tseed() to use system clock for seed.

		o random_rand() is a rand() replacement, which is available
		  for use but I would recommend the wrapper functions:
		  random_int(), random_int_range() etc.

		o random_get_state() and random_set_state() may be used
		  to set, save, restore, and query the current state.

		These functions can be tested by compiling with
		something like:
		gcc -o testrand random_util.c -DRANDOM_UTIL_TEST

  To do:	Gaussian/Boltzmann distributions.
		Need a proper test of randomness.
		Properly implement stack of states.
		I could do with some handy documentation.

 **********************************************************************/

#include "gaul/random_util.h"

/*
 * PRNG constants.  Don't mess with these values willie-nillie.
 */
#define RANDOM_MM_ALPHA	55
#define RANDOM_MM_BETA	24
#define RANDOM_LC_ALPHA	3
#define RANDOM_LC_BETA	257
#define RANDOM_LC_GAMMA	RANDOM_RAND_MAX

/*
 * Global state variable stack.
 * (Implemented using singly-linked list.)
 */
/*
static GSList	*state_list=NULL;
*/
static random_state	current_state;
static boolean		is_initialised=FALSE;

THREAD_LOCK_DEFINE_STATIC(random_state_lock);

/**********************************************************************
 random_rand()
 Synopsis:	Replacement for the standard rand().
		Returns a new pseudo-random value from the sequence, in
		the range 0 to RANDOM_RAND_MAX inclusive, and updates
		global state for next call.  size should be non-zero,
		and state should be initialized.
  parameters:
  return:	none
  last updated:	30/12/00
 **********************************************************************/

unsigned int random_rand(void)
  {
  unsigned int val;

  if (!is_initialised) die("Neither random_init() or random_seed() have been called.");

  THREAD_LOCK(random_state_lock);

  val = (current_state.v[current_state.j]+current_state.v[current_state.k])
        & RANDOM_RAND_MAX;

  current_state.x = (current_state.x+1) % RANDOM_NUM_STATE_VALS;
  current_state.j = (current_state.j+1) % RANDOM_NUM_STATE_VALS;
  current_state.k = (current_state.k+1) % RANDOM_NUM_STATE_VALS;
  current_state.v[current_state.x] = val;

  THREAD_UNLOCK(random_state_lock);

  return val;
  } 


/**********************************************************************
  random_seed()
  synopsis:	Set seed for pseudo random number generator.
		Uses the linear congruential algorithm to fill
		state array.
  parameters:	const unsigned int seed		Seed value.
  return:	none
  last updated: 04 May 2004
 **********************************************************************/

void random_seed(const unsigned int seed)
  { 
  int	i; 

#if USE_OPENMP == 1
  if (is_initialised == FALSE)
    {
    omp_init_lock(&random_state_lock);
    is_initialised = TRUE;
    }
#else
  is_initialised = TRUE;
#endif

  THREAD_LOCK(random_state_lock);

  current_state.v[0]=(seed & RANDOM_RAND_MAX);

  for(i=1; i<RANDOM_NUM_STATE_VALS; i++)
    current_state.v[i] = (RANDOM_LC_ALPHA * current_state.v[i-1]
                          + RANDOM_LC_BETA) & RANDOM_RAND_MAX;

  current_state.j = 0;
  current_state.k = RANDOM_MM_ALPHA-RANDOM_MM_BETA;
  current_state.x = RANDOM_MM_ALPHA-0;

  THREAD_UNLOCK(random_state_lock);

  return;
  } 


/**********************************************************************
  random_tseed()
  synopsis:	Set seed for pseudo random number generator from
		the system time.
  parameters:
  return:	none
  last updated:	28/01/01
 **********************************************************************/

void random_tseed(void)
  { 
  random_seed((unsigned int) (time(NULL)%RANDOM_RAND_MAX));

  return;
  } 


/*************************************************************************
  random_get_state()
  synopsis:	Retrieve current state.  This can be used for saving
		the current state.
  parameters:	none
  return:	random_state state
  last updated:	16/05/00
*************************************************************************/

random_state random_get_state(void)
  {
  return current_state;
  }


/*************************************************************************
  random_set_state()
  synopsis:	Replace current state with specified state.
		This can be used for restoring a saved state.
  parameters:	random_state state
  return:	none
  last updated:	16/05/00
*************************************************************************/

void random_set_state(random_state state)
  {
  current_state = state;

  return;
  }


/*************************************************************************
  random_get_state_str()
  synopsis:	Retrieve current state encoded as a static string.
		This can be used for saving the current state.
  parameters:	none
  return:	char *
  last updated:	28/01/01
*************************************************************************/

char *random_get_state_str(void)
  {
  return (char *) &current_state;
  }


/*************************************************************************
  random_get_state_str_len()
  synopsis:	Retrieve the length of the string encoded current state.
		This can be used for saving the current state.
  parameters:	none
  return:	char *
  last updated:	28/01/01
*************************************************************************/

unsigned int random_get_state_str_len(void)
  {
  return (unsigned int) sizeof(random_state);
  }


/*************************************************************************
  random_set_state_str()
  synopsis:	Replace current state with specified state.
		This can be used for restoring a saved state.
  parameters:	char *
  return:	none
  last updated:	28/01/01
*************************************************************************/

void random_set_state_str(char *state)
  {
  /* This causes potential unaligned trap on Alpha CPUs. */
  current_state = *((random_state *)state);

  return;
  }


/**********************************************************************
  random_init()
  synopsis:	Initialise random number generators.  Should be
		called prior to any of these functions being used.
		Seeds the PRNG with a seed of 1.
  parameters:	none
  return:	none
  last updated:	30 May 2002
 **********************************************************************/

void random_init(void)
  {
  random_seed(1);

#if RANDOM_DEBUG>0
  printf("DEBUG: Random number routines initialised.\n");
#endif

  return;
  }


/**********************************************************************
  random_isinit()
  synopsis:	Whether these routines have been initialised.
  parameters:
  return:	none
  last updated:	30/12/00
 **********************************************************************/

boolean random_isinit(void)
  {
  return is_initialised;
  }


/**********************************************************************
  PRNG interface routines.
 **********************************************************************/

/**********************************************************************
  random_boolean()
  synopsis:	Return TRUE 50% of the time.
  parameters:
  return:	none
  last updated:	16/05/00
 **********************************************************************/

boolean random_boolean(void)
  {
  return (random_rand() <= RANDOM_RAND_MAX/2);
  }


/**********************************************************************
  random_boolean_prob()
  synopsis:	Return TRUE (prob*100)% of the time.
  parameters:
  return:	TRUE or FALSE.
  last updated:	16/05/00
 **********************************************************************/

boolean random_boolean_prob(const double prob)
  {
  return (random_rand() <= (unsigned int)(prob*(double)RANDOM_RAND_MAX));
  }


/**********************************************************************
  random_int()
  synopsis:	Return a random integer between 0 and (N-1) inclusive.
  parameters:
  return:
  last updated:	05 Jun 2003
 **********************************************************************/

unsigned int random_int(const unsigned int max)
  {
/*
  return (int)((double)random_rand()*max/RANDOM_RAND_MAX);
*/
  if (max==0)
    return 0;
  else
    return random_rand()%max;
  }


/**********************************************************************
  random_int_range()
  synopsis:	Return a random integer between min and max-1 inclusive.
  parameters:
  return:
  last updated:	05 Jun 2003
 **********************************************************************/

int random_int_range(const int min, const int max)
  {
/*
  return (random_rand()*(max-min)/RANDOM_RAND_MAX + min);
*/
  if (max-min==0)
    return max;
  else
    return min + (random_rand()%(max-min));
  }


/**********************************************************************
  random_double_full()
  synopsis:	Return a random double within the allowed range.
  parameters:
  return:
  last updated:	16/06/01
 **********************************************************************/

double random_double_full(void)
  {
  return ( ((double)random_rand()/(double)RANDOM_RAND_MAX)*
                (DBL_MAX-DBL_MIN)+DBL_MIN );
  }


/**********************************************************************
  random_double()
  synopsis:	Return a random double within the specified range.
  parameters:
  return:
  last updated:	22/01/01
 **********************************************************************/

double random_double(const double max)
  {
  return ( max*(((double)random_rand())/(double)RANDOM_RAND_MAX) );
  }


/**********************************************************************
  random_double_range()
  synopsis:	Return a random double within the specified range.
  parameters:
  return:
  last updated:	07/06/00
 **********************************************************************/

double random_double_range(const double min, const double max)
  {
  return ( (max-min)*(((double)random_rand())/(double)RANDOM_RAND_MAX) + min );
  }


/**********************************************************************
  random_double_1()
  synopsis:	Return a random double within the range -1.0=>r>1.0
  parameters:
  return:
  last updated:	21/02/01
 **********************************************************************/

double random_double_1(void)
  {
  return ( 2.0*(((double)random_rand())/(double)RANDOM_RAND_MAX) - 1.0 );
  }


/**********************************************************************
  random_float_full()
  synopsis:	Return a random float within the allowed range.
  parameters:
  return:
  last updated:	4 Dec 2001
 **********************************************************************/

float random_float_full(void)
  {
  return ( ((float)random_rand()/(float)RANDOM_RAND_MAX)*
                (DBL_MAX-DBL_MIN)+DBL_MIN );
  }


/**********************************************************************
  random_float()
  synopsis:	Return a random float within the specified range.
  parameters:
  return:
  last updated:	4 Dec 2001
 **********************************************************************/

float random_float(const float max)
  {
  return ( max*(((float)random_rand())/(float)RANDOM_RAND_MAX) );
  }


/**********************************************************************
  random_float_range()
  synopsis:	Return a random float within the specified range.
  parameters:
  return:
  last updated:	4 Dec 2001
 **********************************************************************/

float random_float_range(const float min, const float max)
  {
  return ( (max-min)*(((float)random_rand())/(float)RANDOM_RAND_MAX) + min );
  }


/**********************************************************************
  random_float_1()
  synopsis:	Return a random float within the range -1.0=>r>1.0
  parameters:
  return:
  last updated:	4 Dec 2001
 **********************************************************************/

float random_float_1(void)
  {
  return ( 2.0*(((float)random_rand())/(float)RANDOM_RAND_MAX) - 1.0 );
  }


/**********************************************************************
  random_float_unit_uniform()
  synopsis:	Return a pseudo-random number with a uniform
		distribution in the range 0.0=>r>1.0
  parameters:
  return:
  last updated:	4 Dec 2001
 **********************************************************************/

float random_float_unit_uniform(void)
  {
  return ( (((float)random_rand())/(float)RANDOM_RAND_MAX) );
  }


/**********************************************************************
  random_unit_uniform()
  synopsis:	Return a pseudo-random number with a uniform
		distribution in the range 0.0=>r>1.0
  parameters:
  return:
  last updated:	11/01/01
 **********************************************************************/

double random_unit_uniform(void)
  {
  return ( (((double)random_rand())/(double)RANDOM_RAND_MAX) );
  }


/**********************************************************************
  random_float_gaussian()
  synopsis:	Return a pseudo-random number with a normal
		distribution with a given mean and standard devaiation.
  parameters:
  return:
  last updated:	4 Dec 2001
 **********************************************************************/

float random_float_gaussian(const float mean, const float stddev)
  {
  float	q,u,v,x,y;

/* 
 * Generate P = (u,v) uniform in rectangular acceptance region.
 */
  do
    {
    u = 1.0-random_float_unit_uniform();	/* in range 0.0>u>=1.0 */
    v = 1.7156 * (0.5 - random_float_unit_uniform());	/* in range 0.8578>v>=0.8578 */

/* Evaluate the quadratic form. */
    x = u - 0.449871;
    y = fabs(v) + 0.386595;
    q = x * x + y * (0.19600 * y - 0.25472 * x);

/*
 * Accept P if inside inner ellipse.
 * Reject P if outside outer ellipse, or outside acceptance region.
 */
    } while ((q >= 0.27597) && ((q > 0.27846) || (v * v > -4.0 * log(u) * u * u)));

/* Return ratio of P's coordinates as the normal deviate. */
  return (mean + 2.0 * stddev * v / u);	/* I'm not entirely sure why this *2.0 factor is needed! */
  }


/**********************************************************************
  random_float_unit_gaussian()
  synopsis:	Random number with normal distribution, average 0.0,
		deviation 1.0
  parameters:
  return:
  last updated:	07 Jan 2002
 **********************************************************************/

float random_float_unit_gaussian(void)
  {
  float			r, u, v, fac;
  static boolean	set = FALSE;
  static float		dset;

  if (set)
    {
    set = FALSE;
    return dset;
    }

  do
    {
    u = 2.0 * random_float_unit_uniform() - 1.0;
    v = 2.0 * random_float_unit_uniform() - 1.0;
    r = u*u + v*v;
    } while (r >= 1.0);

  fac = sqrt(-2.0 * log(r) / r);
  dset = v*fac;

  return u*fac;
  }

/* The original (thread-safe) version was this: */
#if 0
float random_float_unit_gaussian(void)
  {
  float	r, u, v;

  do
    {
    u = 2.0 * random_float_unit_uniform() - 1.0;
    v = 2.0 * random_float_unit_uniform() - 1.0;
    r = u*u + v*v;
    } while (r >= 1.0);

  return u*sqrt(-2.0 * log(r) / r);
  }
#endif


/**********************************************************************
  random_float_cauchy()
  synopsis:	Random number with a Cauchy/Lorentzian distribution.
  parameters:
  return:
  last updated:	4 Dec 2001
 **********************************************************************/

float random_float_cauchy(void)
  {
  return tan(random_float_range(-PI/2,PI/2));
  }


/**********************************************************************
  random_float_exponential()
  synopsis:	Random number with an exponential distribution, mean
		of 1.0.
  parameters:
  return:
  last updated:	4 Dec 2001
 **********************************************************************/

float random_float_exponential(void)
  {
  return -log(random_float_unit_uniform());
  }


/**********************************************************************
  random_gaussian()
  synopsis:	Return a pseudo-random number with a normal
		distribution with a given mean and standard devaiation.
  parameters:
  return:
  last updated:	11/01/01
 **********************************************************************/

/*
  Kinda based on: (But optimised quite a bit)

  ALGORITHM 712, COLLECTED ALGORITHMS FROM ACM.
  THIS WORK PUBLISHED IN TRANSACTIONS ON MATHEMATICAL SOFTWARE,
  VOL. 18, NO. 4, DECEMBER, 1992, PP. 434-435.
  The algorithm uses the ratio of uniforms method of A.J. Kinderman
  and J.F. Monahan augmented with quadratic bounding curves.
 */
double random_gaussian(const double mean, const double stddev)
  {
  double	q,u,v,x,y;

/* 
 * Generate P = (u,v) uniform in rectangular acceptance region.
 */
  do
    {
    u = 1.0-random_unit_uniform();	/* in range 0.0>u>=1.0 */
    v = 1.7156 * (0.5 - random_unit_uniform());	/* in range 0.8578>v>=0.8578 */

/* Evaluate the quadratic form. */
    x = u - 0.449871;
    y = fabs(v) + 0.386595;
    q = x * x + y * (0.19600 * y - 0.25472 * x);

/*
 * Accept P if inside inner ellipse.
 * Reject P if outside outer ellipse, or outside acceptance region.
 */
    } while ((q >= 0.27597) && ((q > 0.27846) || (v * v > -4.0 * log(u) * u * u)));

/* Return ratio of P's coordinates as the normal deviate. */
  return (mean + 2.0 * stddev * v / u);	/* I'm not entirely sure why this *2.0 factor is needed! */
  }


#if 0
/* Random number with normal distribution, average 0, deviation 1.
   From Numerical Recipes. */
double random_unit_gaussian(void)
  {
  static boolean	set = FALSE;
  static double		dset;
  double		fac, r, u, v;

  if (!set)
    {
    do
      {
      u = 2.0 * random_unit_uniform() - 1.0;
      v = 2.0 * random_unit_uniform() - 1.0;
      r = u*u + v*v;
      } while (r >= 1.0);

    fac = sqrt(-2.0 * log(r) / r);
    dset = u * fac;
    set = TRUE;

    return v * fac;
    }
  else
    {
    set = FALSE;
    return dset;
    }
  }
#endif


/**********************************************************************
  random_unit_gaussian()
  synopsis:	Random number with normal distribution, average 0.0,
		deviation 1.0
  parameters:
  return:
  last updated:	07 Jan 2002
 **********************************************************************/

double random_unit_gaussian(void)
  {
  double		r, u, v, fac;
  static boolean	set = FALSE;
  static double		dset;

  if (set)
    {
    set = FALSE;
    return dset;
    }

  do
    {
    u = 2.0 * random_unit_uniform() - 1.0;
    v = 2.0 * random_unit_uniform() - 1.0;
    r = u*u + v*v;
    } while (r >= 1.0);

  fac = sqrt(-2.0 * log(r) / r);
  dset = v*fac;

  return u*fac;
  }

/* The original (thread-safe) version was this: */
#if 0
double random_unit_gaussian(void)
  {
  double	r, u, v;

  do
    {
    u = 2.0 * random_unit_uniform() - 1.0;
    v = 2.0 * random_unit_uniform() - 1.0;
    r = u*u + v*v;
    } while (r >= 1.0);

  return u*sqrt(-2.0 * log(r) / r);
  }
#endif


/**********************************************************************
  random_cauchy()
  synopsis:	Random number with a Cauchy/Lorentzian distribution.
  parameters:	none
  return:	double	Random value.
  last updated:	30/04/01
 **********************************************************************/

double random_cauchy(void)
  {
  return tan(random_double_range(-PI/2,PI/2));
  }


/**********************************************************************
  random_exponential()
  synopsis:	Random number with an exponential distribution, mean
		of 1.0.
  parameters:	none
  return:	double	Random value.
  last updated:	30/04/01
 **********************************************************************/

double random_exponential(void)
  {
  return -log(random_unit_uniform());
  }


/**********************************************************************
  random_int_permutation()
  synopsis:	Randomize an array of integers.
  parameters:	int	size
		int	*iarray		Source array.
		int	*oarray		Destination array.
  return:	none
  last updated:	14 Mar 2002
 **********************************************************************/

void random_int_permutation(const int size, int *iarray, int *oarray)
  {
  int		i,j=0;		/* Loop variables over arrays. */
  int		pos;		/* Randomly selected index. */
  
  if (!iarray || !oarray) die("NULL pointer to int array passed.");

  for (i=size-1; i>0; i--)
    {
    pos = random_int(i);
    oarray[j++] = iarray[pos];
    iarray[pos] = iarray[i];
    }

  oarray[j] = iarray[0];

  return;
  }


/**********************************************************************
  random_diagnostics()
  synopsis:	Diagnostics.
  parameters:	none
  return:	none
  last updated:	13 Mar 2002
 **********************************************************************/

void random_diagnostics(void)
  {
  int	i;	/* Loop over PRNG array. */

  printf("=== PRNG routines diagnostic information =====================\n");
  printf("Version:                   %s\n", GA_VERSION_STRING);
  printf("Build date:                %s\n", GA_BUILD_DATE_STRING);
  printf("Compilation machine characteristics:\n%s\n", GA_UNAME_STRING);
  printf("--------------------------------------------------------------\n");
  printf("RANDOM_DEBUG:              %d\n", RANDOM_DEBUG);
  printf("RANDOM_RAND_MAX:           %u\n", RANDOM_RAND_MAX);
  printf("RANDOM_NUM_STATE_VALS:     %d\n", RANDOM_NUM_STATE_VALS);
#if HAVE_SLANG==1
    printf("HAVE_SLANG:                TRUE\n");
#else
    printf("HAVE_SLANG:                FALSE\n");
#endif
  printf("--------------------------------------------------------------\n");
  printf("structure                  sizeof\n");
  printf("random_state:              %lu\n", (unsigned long) sizeof(random_state));
  printf("--------------------------------------------------------------\n");

  if (is_initialised)
    {
    printf("Current state\n");
    printf("j: %d k: %d x: %d v[%d]:\n",
           current_state.j, current_state.k, current_state.x, RANDOM_NUM_STATE_VALS);
    for (i=0; i<RANDOM_NUM_STATE_VALS; i++) printf("%u ", current_state.v[i]);
    printf("\n");
    }
  else
    {
    printf("Not initialised.\n");
    }

  printf("==============================================================\n");

  return;
  }


/**********************************************************************
  random_test()
  synopsis:	Testing.
  parameters:
  return:	TRUE if all tests successful.
  last updated:	30/12/00
 **********************************************************************/

#define NUM_BINS 200
#define NUM_SAMPLES 1000000
#define NUM_CHISQ 20

boolean random_test(void)
  {
  unsigned int	i, j, k;		/* Loop variables. */
  double	r;			/* Pseudo-random number. */
  long		bins[NUM_BINS];		/* Bin. */
  double	sum=0,sumsq=0;		/* Stats. */
  int		numtrue=0, numfalse=0;	/* Count booleans. */
  unsigned int	rchi=100;		/* Number of bins in chisq test. */
  unsigned int	nchi=1000;		/* Number of samples in chisq test. */
  double	chisq;			/* Chisq error. */
  double	elimit = 2*sqrt((double)rchi);	/* Chisq error limit. */
  double	nchi_rchi = (double)nchi / (double)rchi;	/* Speed calculation. */
  FILE		*rfile=NULL;		/* Handle for file of random integers. */

  random_init();

  printf("Testing random numbers.\n");

/*
 * Uniform Distribution.
 */
  printf("Uniform distribution.  Mean should be about 0.5.\n");

  for (i=0;i<NUM_BINS;i++) bins[i] = 0;

  for (i=0;i<NUM_SAMPLES;i++)
    {
    r = random_unit_uniform();
    if (r >= 0.0 && r < 1.0)
      {
      bins[(int)(r*NUM_BINS)]++;
      sum += r;
      sumsq += SQU(r);
      }
    else
      {
      printf("Number generated out of range 0.0<=r<1.0.\n");
      }
    }
  printf("Mean = %f\n", sum / NUM_SAMPLES);
  printf("Standard deviation = %f\n", (sumsq - sum*sum/NUM_SAMPLES)/NUM_SAMPLES);

  for (i=0;i<NUM_BINS;i++)
    printf("%5.3f %ld\n", i/(double)NUM_BINS, bins[i]);

/*
 * Gaussian Distribution.
 */
  printf("Gaussian distribution.  Mean should be about 0.45.  Standard deviation should be about 0.05.\n");

  sum=0;
  sumsq=0;

  for (i=0;i<NUM_BINS;i++) bins[i] = 0;

  for (i=0;i<NUM_SAMPLES;i++)
    {
    r = random_gaussian(0.45,0.05);
    if (r >= 0.0 && r < 1.0)
      {
      bins[(int)(r*NUM_BINS)]++;
      sum += r;
      sumsq += SQU(r);
      }
    else
      {
      printf("Number generated out of range 0.0<=r<1.0.\n");
      }
    }
  printf("Mean = %f\n", sum / NUM_SAMPLES);
  printf("Standard deviation = %f\n", (sumsq - sum*sum/NUM_SAMPLES)/NUM_SAMPLES);

  for (i=0;i<NUM_BINS;i++)
    printf("%5.3f %ld\n", i/(double)NUM_BINS, bins[i]);

/*
 * Unit Gaussian Distribution.
 */
  printf("Gaussian distribution.  Mean should be about 0.0.  Standard deviation should be about 1.0.\n");

  sum=0;
  sumsq=0;

  for (i=0;i<NUM_BINS;i++) bins[i] = 0;

  for (i=0;i<NUM_SAMPLES;i++)
    {
    r = random_unit_gaussian();
    if (r >= -5.0 && r < 5.0)
      {
      bins[(int)((r+5.0)*NUM_BINS)/10]++;
      sum += r;
      sumsq += SQU(r);
      }
    else
      {
      printf("Number generated out of range -5.0<=r<5.0.\n");
      }
    }
  printf("Mean = %f\n", sum / NUM_SAMPLES);
  printf("Standard deviation = %f\n", (sumsq - sum*sum/NUM_SAMPLES)/NUM_SAMPLES);

  for (i=0;i<NUM_BINS;i++)
    printf("%5.3f %ld\n", -5.0+10*i/(double)NUM_BINS, bins[i]);

/*
 * Random Boolean.
 */
  printf("Random Booleans.  Two counts should be approximately equal.\n");

  for (i=0;i<NUM_SAMPLES;i++)
    {
    if ( random_boolean() )
      numtrue++;
    else
      numfalse++;
    }
  printf("TRUE/FALSE = %d/%d\n", numtrue, numfalse);

/*
 * Random int.
 */
  printf("Random Integers.  The distribution should be approximately uniform.\n");

  for (i=0;i<NUM_BINS;i++) bins[i] = 0;

  for (i=0;i<NUM_SAMPLES;i++)
    bins[random_int(NUM_BINS)]++;

  for (i=0;i<NUM_BINS;i++)
    printf("%u %ld\n", i, bins[i]);

/*
 * Chi squared test.  This is the standard basic test for randomness of a PRNG.
 * We would expect any moethod to fail about about one out of ten runs.
 * The error is r*t/N - N and should be within 2*sqrt(r) of r.
 */

  printf("Chi Squared Test of Random Integers.  We would expect a couple of failures.\n");

  if (rchi>NUM_BINS) die("Internal error: too few bins.");

  for (j=0;j<NUM_CHISQ;j++)
    {
    printf("Run %u. chisq should be within %f of %u.\n", j, elimit, rchi);
    for(k=0; k<10; k++)
      {
      memset(bins, 0, rchi*sizeof(long));
      chisq = 0.0;

      for(i=0; i<nchi; i++)
        bins[random_int(rchi)]++;

      for(i=0; i<rchi; i++)
        chisq += SQU((double)bins[i] - nchi_rchi);

      chisq /= nchi_rchi;

      printf("chisq = %f - %s\n", chisq, fabs(chisq - rchi)>elimit?"FAILED":"PASSED");
      }
    }

  printf("Creating file (\"randtest.dat\") of 5000 random integer numbers for external analysis.\n");

  rfile = fopen("randtest.dat", "w");

  for (i=0; i<5000; i++)
    {
    r = (double) random_rand();
    fprintf(rfile, "%f %f\n",
            /*i, r,*/
            (double)i/(double)5000, r/(double)RANDOM_RAND_MAX);
    }

  fclose(rfile);

  return TRUE;
  }


/**********************************************************************
  SLang intrinsic wrapper functions.

  FIXME: Problem with unsigned vs. signed ints.
 **********************************************************************/
#if HAVE_SLANG==1

/* These function don't need wrappers:
void random_tseed(void)
void random_init(void)
boolean random_is_init(void)
boolean random_boolean(void)
double random_unit_uniform(void)
double random_unit_gaussian(void)
void random_diagnostics(void)
boolean random_test(void)
*/

int random_rand_wrapper(void)
  {
  return (int) random_rand();
  } 


void random_seed_wrapper(int *seed)
  { 
  random_seed((unsigned int) *seed);
  return;
  } 


#if 0
THREAD_LOCK_DEFINE_STATIC(state_table_lock);
TableStruct     *state_table=NULL;        /* Table for state handles. */

int random_get_state_wrapper(void)
  {
  int	stateid;	/* State handle. */

  THREAD_LOCK(state_table_lock);
  if (!state_table) state_table=table_new();

  stateid = table_add(state_table, (vpointer) current_state);	/* FIXME: Need to copy state. */
  THREAD_UNLOCK(state_table_lock);

  return stateid;
  }


void random_set_state_wrapper(int stateid)
  {
  random_state	state;	/* State to restore. */

  THREAD_LOCK(state_table_lock);
  state = table_get_data(state_table, stateid);
  THREAD_UNLOCK(state_table_lock);

  if (!state) die("Unmatched state handle.");

  random_set_state(state);

  return;
  }
#endif


boolean random_boolean_prob_wrapper(double *prob)
  {
  return random_boolean_prob(*prob);
  }


int random_int_wrapper(int *max)
  {
  return (int) random_int((unsigned int) *max);
  }


int random_int_range_wrapper(int *min, int *max)
  {
  return random_int_range(*min, *max);
  }


double random_double_wrapper(double *max)
  {
  return random_double(*max);
  }


double random_double_range_wrapper(double *min, double *max)
  {
  return random_double_range(*min, *max);
  }


double random_gaussian_wrapper(double *mean, double *stddev)
  {
  return random_gaussian(*mean, *stddev);
  }

#endif
