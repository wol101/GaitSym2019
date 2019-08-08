/**********************************************************************
  memory_util.c
 **********************************************************************

  memory_util - Usage control wrapper around standard malloc() etc.
  Copyright ©1999-2005, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:	memory_util.c is intended as a general purpose,
                portable, memory tracking package that replaces the
                system calls realloc, calloc and realloc and
                free with "safer" routines while keeping track of all
                the memory currently allocated.
                Additional facilities to enable array overflow checks
                are included.

                The neat thing about all this is that it can sit over
                the top of any other memory debugging library which
                replaces the standard malloc routines, transparently.

                Has an easy-to-use memory chunk implementation.
                (see memory_chunk.c)
                Chunks are created/destroyed using
                mem_chunk_new() and mem_chunk_destroy().
                Individual atoms are created/destroyed using
                mem_chunk_alloc() and mem_chunk_free().

                Compile the code with MEMORY_ALLOC_SAFE for simple
                wrappers around the standard system allocation memory
                routines.  Compile with MEMORY_ALLOC_DEBUG for fully
                auditing wrappers.  Compile with neither for the
                standard routines only.  When MEMORY_ALLOC_DEBUG is
                defined or MEMORY_CHUNKS_MIMIC is not defined, my AVL
                tree implementation is required.

                The tree lookup key (AVLKey) must be capable of
                storing any arbitrary memory address on your machine.

                FAQ:
                Q. Why not just use Purify?
                A. I can't afford it.
                Q. Well, ElectricFence is free - so why not use that?
                A. It is horrendously slow, and a huge memory hog.

                For OpenMP code, USE_OPENMP must be defined and
                memory_init_openmp() must be called prior to any
                other function.

  To do:	A lot!

                Record mem_chunk allocations in the same way as xalloc allocations.

                Don't really deallocate memory - reuse later.  This would enable the possibility of detecting memory access after it has been free-ed.
                Need memory_final_clearup() to deallocate all memory -and- the memory table!
                Use random number generator which does not interfere with system random number generator.
                Code for low memory simulation.  i.e. impose limit on memory allocation.  (I currently use the shell's built-in resource limit for this)

                Need shortened version of check_mptr() - check_new_mptr()
                Use my helga_log functions for logging? (a) would work okay with MP/MT code, and (b) enable logging to a file.

                memory_diagnostics()
                Compile as libmemory.so

                Adapt padding so alignment isn't broken on Alpha processors.

        ???	Optional automatic garbage collection.

        *->	Use helga_log functions for logging.
        *->	Rewrite padding - arbitrary sizes.

        *->	Reintroduce random padding.

                alloca() like function.

                s_malloc0() etc. would be useful.

  Bugs:		Padding causes data to become misaligned on alpha processors.
                These function names aren't strictly allowed under ISO C99!

 **********************************************************************/

#include "gaul/memory_util.h"

/*
 * The memory table structure.
 */
typedef struct mem_record_t
  {
  void		*mptr;		/* the pointer value for this block of memory (may not be real start of memory block) */
  size_t	mem;		/* the total number of bytes in this block (the actual amount incl. padding) */
  size_t	rmem;		/* the amount of memory the calling routine requested, note: rmem<=mem */
  char		label[64];	/* the variable/label name associated with this memory */
  char  	func[64];	/* the function the original allocation routine was from */
  char		file[64];	/* file of memory allocation request  */
/* Get rid of [64]! use pointer instead.  Will also save use of strcpy() */
  int		line;		/* line of memory allocation request */
  char		pad_high[8];	/* chars added to test for overflow */
  char		pad_low[8];	/*  - " -             - " -   underflow */
  size_t	pad_ls;		/* amount of pre-padding */
  size_t	pad_hs;		/* amount of post-padding */
  } mem_record;

/*
 * Global state variables.
 */

THREAD_LOCK_DEFINE_STATIC(memory_mem_record_chunk);
THREAD_LOCK_DEFINE_STATIC(memory_memtree);
#if USE_OPENMP == 1
static boolean memory_openmp_initialised = FALSE;
#endif

static MemChunk *mem_record_chunk=NULL;	/* the memory record buffer. */
static AVLTree	*memtree=NULL;		/* the global memory allocation tree. */
static int	num_mem;		/* the number of entries in the memory table. */
static int	max_mem;		/* the current size of the memory table. */
static int	allocated_mem;		/* the total memory currently allocated. */
static int	most_mem;		/* record of maximum allocated memory. */
static int	memory_verbose=1;	/* level of reporting. */
static int	memory_strict=3;	/* level of strictness for uninitialized memory. */
static int	memory_bounds=0;	/* level of automatic bound checking. */
static int	memory_padding=0;	/* variables for memory padding FIXME: replace with pad_size_low and pad_size_high. */
static int	memory_reset_bv=1;	/* reset padding after memory violations detected. */
static int	memory_count_bv=0;	/* count total number of bounds violations encountered. */
static int	memory_count_if=0;	/* count total number of invalid free calls. */
static size_t	memory_size_pad=sizeof(char)*8;	/* Amount of padding to use. */

static FILE	*memory_log_file=NULL;	/* File handle for log file */
static long	memory_count_malloc=0;	/* count total number of s_malloc() calls. */
static long	memory_count_calloc=0;	/* count total number of s_calloc() calls. */
static long	memory_count_realloc=0;	/* count total number of s_realloc() calls. */
static long	memory_count_strdup=0;	/* count total number of s_strdup() calls. */
static long	memory_count_free=0;	/* count total number of s_free() calls. */

static int	node_count=0;		/* counting tree nodes for debugging. */

/*
 * This function must be called before any other functions is OpenMP
 * code is to be used.  Can be safely called when OpenMP code is not
 * being used, and can be safely called more than once.
 */
void memory_init_openmp(void)
  {

#if USE_OPENMP == 1
  if (memory_openmp_initialised == FALSE)
    {
    omp_init_lock(&memory_mem_record_chunk);
    omp_init_lock(&memory_memtree);
    memory_openmp_initialised = TRUE;
    }
#endif

  return;
  }


/*
 * avltree.c replacements to avoid usage of the local
 * malloc functions.
 */

static AVLKey key_func(constvpointer data)
  {
  return (AVLKey) (((mem_record *)data)->mptr);
  }

/*
static AVLTree *replacement_avltree_new(void)
  {
  AVLTree	*tree;

  if ( !(tree = malloc(sizeof(AVLTree))) )
    die("Unable to malloc memory!");

  tree->root = NULL;
  tree->key_generate_func = key_func;

  return tree;
  }
*/

/*
 * Allocating/Deallocating mem_record structures.
 */

static mem_record *mem_record_new(void)
  {
  mem_record *mr;

  THREAD_LOCK(memory_mem_record_chunk);
  if (!mem_record_chunk) mem_record_chunk = mem_chunk_new(sizeof(mem_record), 1024);

  mr = (mem_record *)mem_chunk_alloc(mem_record_chunk);
  THREAD_UNLOCK(memory_mem_record_chunk);

  num_mem++;

  return mr;
  }


static void mem_record_free(mem_record *mr)
  {
  mem_chunk_free(mem_record_chunk, (vpointer) mr);

  num_mem--;

  return;
  }


static void memtree_new(void)
  {
  if (memtree) die("Memory tree already exists.");

  THREAD_LOCK(memory_memtree);
/*
  memtree = replacement_avltree_new();
*/
  memtree = avltree_new(key_func);
  THREAD_UNLOCK(memory_memtree);

  return;
  }


/*
static void memtree_destroy(void)
  {
  if (!memtree) die("Unable to destroy non-existant tree.");

  THREAD_LOCK(memory_memtree);
  avltree_destroy(memtree);
  THREAD_UNLOCK(memory_memtree);

  return;
  }
*/


/**********************************************************************
  void memory_open_log()
  synopsis:	Write messages to log file.
  parameters:   const char *fname	File name for logfile.
  return:	none
  last updated: 04/01/00
 **********************************************************************/

void memory_open_log(const char *fname)
  {
  if (memory_log_file) fclose(memory_log_file);

  memory_log_file = fopen(fname, "a");

  memory_write_log("Log file opened.");

  return;
  }


/**********************************************************************
  void memory_write_log()
  synopsis:	Write messages to log file, if logfile has been opened.
  parameters:   const char *text	String to write to log file.
  return:	none
  last updated: 04/01/00
 **********************************************************************/

void memory_write_log(const char *text)
  {
  if (memory_log_file)
    {
    time_t t = time(NULL);
    fprintf(memory_log_file, "%s: %s\n", ctime(&t), text);
    }

  return;
  }


/**********************************************************************
  void memory_fwrite_log()
  synopsis:	Write formatted messages to log file, if logfile
                has been opened.
  parameters:   const char *format, args...	Stuff to write to log file.
  return:	none
  last updated: 04/01/00
 **********************************************************************/

void memory_fwrite_log(const char *format, ...)
  {
  va_list	args;
  char		text[2048];
  int		len;

  if (memory_log_file)
    {
    time_t t = time(NULL);

    va_start(args, format);
    vsnprintf(text, 2047, format, args);	/* 2047 so tacking on newline doesn't cause overflow */
    va_end(args);

/* Was it an empty format? */
    if (*text == '\0') return;

/* Tack on a '\n' if necessary. */
    len = strlen(text)-1;
    if (text[len] != '\n')
      {
      text[len++] = '\n';
      text[len] = '\0';
      }

    fprintf(memory_log_file, "%s: %s\n", ctime(&t), text);
    }

  return;
  }


static boolean table_traverse(AVLKey key, vpointer data, vpointer userdata)
  {
  mem_record	*mr=(mem_record *)data;

  node_count++;

  printf("%d: ", node_count);

/* check. */
/*
  if (key != (AVLKey) mr->mptr)
    printf("key failure (%p %p) ", (vpointer) key, (vpointer) ((AVLKey)mr->mptr));
*/

/*  printf(" %s\t %s\t %s\t %d\t %Zd\t %Zd\t (%p)\n",mr->label,mr->func,mr->file,mr->line,mr->mem,mr->rmem,mr->mptr);*/
  printf(" %s\t %s\t %s\t %d\t %lu\t %lu\t (%p)\n",mr->label,mr->func,mr->file,mr->line,(unsigned long)mr->mem,(unsigned long)mr->rmem,mr->mptr);

  return FALSE;
  }


static boolean bounds_traverse(AVLKey key, vpointer data, vpointer userdata)
  {
  mem_record	*mr=(mem_record *)data;

  node_count++;

/* check. */
/*
  if (key != (AVLKey) mr->mptr)
    printf("key failure (%p %p) ", (vpointer) key, (vpointer) ((AVLKey)mr->mptr));
*/

  if (memory_check_bounds(mr->mptr)!=0)
    printf("violation! %s\t %s\t %s\t %d\t %lu\t %lu\t (%p)\n",mr->label,mr->func,mr->file,mr->line,(unsigned long)mr->mem,(unsigned long)mr->rmem,mr->mptr);

/*    printf("violation! %s\t %s\t %s\t %d\t %Zd\t %Zd\t (%p)\n",mr->label,mr->func,mr->file,mr->line,mr->mem,mr->rmem,mr->mptr);*/

  return FALSE;
  }


/**********************************************************************
  match_mptr()
  synopsis:	This is the fundamental memory table check.  Returns
                the pointer to the relevant structure, or NULL.
  parameters:
  return:
  last updated:	30/12/00
 **********************************************************************/

static mem_record *match_mptr(void *mptr)
  {
  return (mem_record *)avltree_lookup_key(memtree, (AVLKey) mptr);
  }


/**********************************************************************
  check_mptr()
  synopsis:	This is the function for validating a new pointer.
                Note that passing current as NULL indicates
                that this pointer is not expected to have a entry,
                i.e. it is new or outside the table scope, e.g. its
                the memory table itself.
                TRUE is returned if pointer's records seem okay.
  parameters:
  return:
  last updated: 01/12/98
 **********************************************************************/

static int check_mptr(void *mptr, mem_record *current)
  {
  mem_record	*i;

/*
 * check for NULL, which should only occur if the system is out
 * of memory.
 */
  if (!mptr)
    {
    printf("ERROR: Pointer is null.  Probably out of memory!\n");
    exit(EXIT_FAILURE);
    }

/*
 * Check whether pointer already exists in the memory table. if it does
 * but in the same position as "current" then everything is ok, i.e. the
 * system might have used to same pointer for the same memory (i.e. it
 * defaulted on memory creation, maybe).  However, if the pointer exists
 * anywhere else then something has gone odd.  s_alloc_debug() or
 * s_free_debug() would get screwed up trying to use this pointer.
 */
  i=match_mptr(mptr);

  if (i==NULL || i==current) return TRUE;

  if (i)
    {
    printf("WARNING: system has duplicated an active table pointer! %p from %s\n", mptr, i->label);
    printf("WARNING: The table is now corrupted! %d records.\n", num_mem);
    }
/*
  else
    {
    printf("WARNING: Unable to match table pointer! %p\n", mptr);
    printf("WARNING: The table is now corrupted! %d records.\n", num_mem);
    }
*/

/* FIXME: reinstate this using tree traversal function.
  for(j=0;j<num_mem;j++)
    printf("%d %p %p\n",j,mptr,mem[j].mptr);
*/

  return FALSE;
  }


/**********************************************************************
  pad_mptr_low() etc.
  synopsis:	Functions to handle padding.
  parameters:
  return:
  last updated: 01/12/98
 **********************************************************************/

static void pad_mptr_low(mem_record *mr)
  {

  if (mr->pad_ls == 0) return;	/* There's nothing to do here. */

  (void *)memcpy(((unsigned char *) mr->mptr)-mr->pad_ls,mr->pad_low,mr->pad_ls);

  return;
  }


static void pad_mptr_high(mem_record *mr)
  {
  unsigned char	*cmptr;

  if (mr->pad_hs==0) return;	/* There's nothing to do here. */

  cmptr=(unsigned char *) mr->mptr;

  (void *)memcpy(cmptr+mr->rmem,mr->pad_high,mr->pad_hs);

  return;
  }


static int check_pad_mptr_low(mem_record *mr)
  {
  unsigned char	*cmptr;

  if (!mr->pad_ls) return 0;

  cmptr=(unsigned char *) mr->mptr;

  return abs( memcmp(cmptr-mr->pad_ls,mr->pad_low,mr->pad_ls) );
  }


static int check_pad_mptr_high(mem_record *mr)
  {
  unsigned char	*cmptr;

  if (!mr->pad_hs) return 0;

  cmptr=(unsigned char *) mr->mptr;

  return abs( memcmp(cmptr+mr->rmem,mr->pad_high,mr->pad_hs) );
  }


static void *bump_down(void *mtemp, int i_size)
  {
  unsigned char *bmptr;

  bmptr = (unsigned char *) mtemp;
  bmptr -= i_size;

  return ((void *) bmptr);
  }


static void *bump_up(void *mtemp, int i_size)
  {
  unsigned char *bmptr;

  bmptr = (unsigned char *) mtemp;
  bmptr += i_size;

  return ((void *) bmptr);
  }


static void free_padded(mem_record *mr)
  {
  unsigned char *bmptr;

  bmptr = (unsigned char*) mr->mptr;
  bmptr -= mr->pad_ls;
  free(bmptr);

  return;
  }


/**********************************************************************
  void *s_alloc_debug()
  synopsis:     My replacement for the standard malloc(), calloc()
                realloc(), and strdup() calls.  I have assummed that
                sizeof(char)==sizeof(byte) which I think is always
                true.
  parameters:	lots.
  return:       None.
  last updated: 19/01/99
 **********************************************************************/

void *s_alloc_debug(memory_alloc_type type, size_t memsize, int numvars, void *mptr,
                const char *name, const char *file, const int line,
                const char *label)
  {
  mem_record	*j=NULL;			/* Current memory record. */
  size_t	size_low, size_high;		/* Padding amount. */
  char		rand_high='a', rand_low='a';	/* Padding value. */
  int		k;				/* Loop variable. */
  void		*mtemp=NULL;			/* Temp. memory pointer. */
  int		len;				/* String length. */

#if MEMORY_DEBUG>2
  printf("Allocation of %d bytes by method %d.\n", (int) memsize, (int) type);
#endif

/* Increment call counter */
  switch ((int) type)
    {
    case (MEMORY_MALLOC):
      memory_count_malloc++;
      break;
    case (MEMORY_CALLOC):
      memory_count_calloc++;
      break;
    case (MEMORY_REALLOC):
      memory_count_realloc++;
      break;
    case (MEMORY_STRDUP):
    case (MEMORY_STRNDUP):
      memory_count_strdup++;
      break;
    default:
      printf("ERROR: s_alloc_debug(): Unknown type %d.\n", (int) type);
      exit(EXIT_FAILURE);
    }

/*
 * Initialise tree if required.
 * Remember that this tree is outside the scope of the memory tracing!
 */
    if (memtree==NULL)
      {
      memtree_new();
      if(!check_mptr(memtree,NULL)) printf("Dodgy memory allocation table!\n");
      }

/* Check padding preference */
  switch (memory_padding)
    {
    case 0:
      size_low=0;
      size_high=0;
      break;
    case 1:
      size_low=memory_size_pad;
      size_high=memory_size_pad;
/*
      rand_high=rand();
      rand_low=rand();
*/
      break;
    case 2:
      size_low=0;
      size_high=memory_size_pad;
/*
      rand_high=rand();
*/
      break;
    case 3:
      size_low=memory_size_pad;
      size_high=0;
/*
      rand_low=rand();
*/
      break;
    default:
      printf("s_alloc_debug(): Unknown memory padding level %d\n", memory_padding);
      exit(EXIT_FAILURE);
    }

  if(memory_bounds==1) memory_check_all_bounds();
  if(memory_bounds==3) memory_check_all_bounds();

/* Check pointer if we are doing a reallocation */
  if(mptr!=NULL && type==MEMORY_REALLOC)
    {
    j=match_mptr(mptr);

/*
printf("Realloc mptr = %p j = %p j->mptr = %p\n", mptr, j, j->mptr);
*/

/*
 * Handle any non-null, non-registered pointers.
 */
    if (memory_strict && j==NULL)
      {
      printf("WARNING: NO match to pointer table for %p \"%s\".\n", mptr, label);
      if (memory_strict==1) { printf("Assigning memory anyway.\n"); j=NULL; mptr=NULL; }
      if (memory_strict==2) { printf("Returning with pointer UNASSIGNED.\n"); return mptr; }
      if (memory_strict==3) { dief("Exiting program.\n"); }
      }

/*
 * Handle special case of realloc(mptr, 0).
 */
    if (memsize==0)
      {
      s_free_debug(mptr, name, file, line);
      return NULL;
      }

    }

/*
 * Add memory by whichever method was requested.
 * Note that realloc() is NULL-safe.
 */
  if ( mptr==NULL || type==MEMORY_STRDUP || type==MEMORY_STRNDUP )
    {
    switch ((int) type)
      {
      case (MEMORY_MALLOC):
#if MEMORY_DEBUG>2
        printf("Malloc of %d + %d + %d bytes.\n", (int) memsize, (int) size_low, (int) size_high);
#endif
        if ( !(mtemp=malloc(memsize+size_low+size_high)) )
          {
          dief("Memory allocation of %lu bytes failed at func=%s file=%s line=%d\n",
                 (unsigned long) memsize, name, file, line);
          }
        break;
      case (MEMORY_CALLOC):
        /* We don't use 'mtemp=calloc(memsize,numvars);',
         * we use malloc instead of calloc */
        if ( !(mtemp=malloc(memsize*numvars+size_low+size_high)) )
          {
          dief("Memory allocation of %lu bytes failed at func=%s file=%s line=%d\n",
                 (unsigned long) numvars*memsize, name, file, line);
          }
        memset(mtemp,0,memsize*numvars+size_low+size_high);
        break;
      case (MEMORY_REALLOC):
        /* We also use malloc instead of realloc */
        if ( !(mtemp=malloc(memsize+size_low+size_high)) )
          {
          dief("Memory allocation of %lu bytes failed at func=%s file=%s line=%d\n",
                 (unsigned long) memsize, name, file, line);
          }
        break;
      case (MEMORY_STRDUP):
        /* And guess what... We also use malloc for strdup.  Wow. */
        if ( !(mtemp=malloc(memsize+size_low+size_high)) )
          {
          dief("Strdup of %lu bytes failed at func=%s file=%s line=%d\n",
                 (unsigned long) memsize, name, file, line);
          }
        memcpy((char *)mtemp+size_low,mptr,memsize);
        break;
      case (MEMORY_STRNDUP):
        /* ...and strndup. */
        len = strlen((const char *)mptr);
        if (memsize > len) memsize=len;
        if ( !(mtemp=malloc(memsize+size_low+size_high)) )
          {
          dief("Strdup of %lu bytes failed at func=%s file=%s line=%d\n",
                 (unsigned long) memsize, name, file, line);
          }
        memcpy((char *)mtemp+size_low,mptr,memsize);
        ((char *)mtemp)[memsize-1] = '\0';
        break;
      default:
        dief("s_alloc_debug(): Unknown type %d.  This is a bad internal error!\n",
               (int) type);
      }

    mtemp=bump_up(mtemp,size_low);

    if (!check_mptr(mtemp,NULL)) printf("Dodgy %s\n",label);

/*
 * Fill in memory record for this block.
 */
    j = mem_record_new();
    j->mptr=mtemp;
    j->mem=memsize*numvars;
    j->rmem=j->mem;
    strncpy(j->label,label,64);
    strncpy(j->func,name,64);
    strncpy(j->file,file,64);
    j->line=line;

    j->pad_ls=size_low;
    j->pad_hs=size_high;
    if(size_high)
      {
      for (k=0; k<size_high; k++) j->pad_high[k]=rand_high;
      pad_mptr_high(j);
      }
    if(size_low)
      {
      for (k=0; k<size_low; k++) j->pad_low[k]=rand_low;
      pad_mptr_low(j);
      }

    allocated_mem+=memsize*numvars;
    if (allocated_mem > most_mem) most_mem = allocated_mem;
/*
printf("INSERT (1) mtemp = %p j = %p j->mptr = %p\n", mtemp, j, j->mptr);
*/
    avltree_insert(memtree, (vpointer)j);
    }
  else
    {
/*
 * There is (or should be) a record of this pointer already, therefore
 * we just update the current entry.
 */
    mtemp=j->mptr;

/* DEBUG: */
    if (!check_mptr(mtemp,j)) printf("Dodgy %s\n",label);

/*
printf("mtemp = %p, j = %p, j->mptr = %p\n", mtemp, j, j->mptr);
*/

/* if new memory is required */
    if(memsize*numvars>j->mem)
      {

      switch ((int) type)
        {
        case (MEMORY_MALLOC):
          /* This case should never happen. */
          printf("s_alloc_debug(): MALLOC with existing pointer requested.  Internal error?\n");
          exit(EXIT_FAILURE);
          break;
        case (MEMORY_CALLOC):
          /* This case should never happen. */
          printf("s_alloc_debug(): CALLOC with existing pointer requested.  Internal error?\n");
          exit(EXIT_FAILURE);
          break;
        case (MEMORY_REALLOC):
          mtemp=bump_down(mtemp,j->pad_ls);
          mtemp=realloc(mtemp,memsize+size_low+size_high);
          if (!mtemp)
            {
            printf("Memory reallocation of %lu bytes failed at func=%s file=%s line=%d\n",
                   (unsigned long) memsize, name, file, line);
            perror("realloc");
            exit(EXIT_FAILURE);
            }
          break;
        case (MEMORY_STRDUP):
          /* This case should never happen. */
          printf("s_alloc_debug(): STRDUP call in wrong bit of code.  Internal error!\n");
          exit(EXIT_FAILURE);
          break;
        case (MEMORY_STRNDUP):
          /* This case should never happen. */
          printf("s_alloc_debug(): STRNDUP call in wrong bit of code.  Internal error!\n");
          exit(EXIT_FAILURE);
          break;
        default:
          printf("s_alloc_debug(): Unknown type %d.\n", (int) type);
          exit(EXIT_FAILURE);
        }

      mtemp=bump_up(mtemp,size_low);

/*
 * Realloc may default to same memory location.  So, I could save a few cycles
 * by not removing the record from the tree and then replacing it again.
 */
      avltree_remove_key(memtree, (AVLKey) j->mptr);
      if (!check_mptr(mtemp,NULL)) printf("Dodgy %s\n", label);

/*
printf("REPLACING j->mptr %p with mtemp %p j %p\n", j->mptr, mtemp, j);
*/

      allocated_mem+=memsize*numvars-j->mem;
      if (allocated_mem > most_mem) most_mem = allocated_mem;
      j->mptr=mtemp;
      j->mem=memsize*numvars;
      strncpy(j->label,label,64);
      strncpy(j->func,name,64);
      strncpy(j->file,file,64);
      j->line=line;
      j->rmem=j->mem;

      j->pad_ls=size_low;
      j->pad_hs=size_high;
      if (size_high)
        {
        for (k=0; k<size_high; k++) j->pad_high[k]=rand_high;
        pad_mptr_high(j);
        }
      if (size_low)
        {
        for (k=0; k<size_low; k++) j->pad_low[k]=rand_low;
        pad_mptr_low(j);
        }

/*
printf("INSERT (2) mtemp = %p j = %p j->mptr = %p\n", mtemp, j, j->mptr);
*/
      avltree_insert(memtree, (vpointer)j);
      }
    else
      {
/* no new memory is required, so just reuse current */
      if (type==MEMORY_CALLOC)
        {
        memset(mtemp,0,memsize*numvars);
        }
      j->rmem=memsize*numvars; /* note rmem!=mem */

      if(j->pad_hs)
        {
        for (k=0; k<size_high; k++) j->pad_high[k]=rand_high;
        pad_mptr_high(j);
        }
      if (j->pad_ls)
        {
        for (k=0; k<size_low; k++) j->pad_low[k]=rand_low;
        pad_mptr_low(j);
        }
      }
    }

  if (memory_verbose>1)
    {
    if (memory_verbose>2)
      {
      printf("%s allocation call from %s, file \"%s\", line %d\n", label, name, file, line);
      }
    printf("s_alloc_debug(): %s has %lu used, %lu allocated, total memory allocated = %d\n",
                label, (unsigned long) j->rmem, (unsigned long) j->mem, allocated_mem);
    }

/*
  printf("DEBUG: tree now contains %d nodes.  %d records.\n", avltree_num_nodes(memtree), num_mem);
  if (avltree_num_nodes(memtree) != num_mem) die("Oh dear.");
*/

  return mtemp;
  }


/**********************************************************************
  void *s_free_debug()
  synopsis:     My replacement for free().
  parameters:   None.
  return:       None.
  last updated: 17/07/00
 **********************************************************************/

void *s_free_debug(void *mptr, const char *name, const char *file, const int line)
  {
  mem_record	*j;	/* Index of current memory record. */

/* Increment call counter */
  memory_count_free++;

/* If set to do so, check for bounds violations */
  if (memory_bounds==2 || memory_bounds==3) memory_check_all_bounds();

/* Do some checks on pointer */
/* Is it non-NULL? */
  if (mptr==NULL)
    {
    printf("WARNING: Passed NULL pointer!\n");
    printf("Not attempting to deallocate memory.\n");
    printf("function \"%s\" file \"%s\" line %d\n", name, file, line);

    memory_count_if++;
    return (NULL);
    }

/* Is it in the table? */
  j=match_mptr(mptr);
  if(j==NULL)
    {
    printf("WARNING: Pointer not in memory table!\n");
    printf("Not attempting to deallocate memory.\n");
    printf("function \"%s\" file \"%s\" line %d\n", name, file, line);

    memory_count_if++;
    return (mptr);
    }

/* Does it have a positive number of bytes associated? */
  if (j->mem < 1)
    {
    printf("WARNING: Pointer has zero bytes associated!\n");
    printf("Not attempting to deallocate memory.\n");
    printf("function \"%s\" file \"%s\" line %d\n", name, file, line);

    memory_count_if++;
    return (mptr);
    }

/*
 * Remove this entry from memory tree.
 */
/*
printf("REMOVING mptr = %p j = %p j->mptr = %p\n", mptr, j, j->mptr);
*/
  avltree_remove_key(memtree, (AVLKey) mptr);
/* DEBUG:
  if (match_mptr(mptr)) printf("WARNING: pointer is still in table.\n");
*/

/*
 * Pointer seems okay, so deallocate this memory.
 */
  free_padded(j);

  allocated_mem-=j->mem;

/* report on deallocation, if required */
  if (memory_verbose>1)
    {
    if (memory_verbose>2)
      {
      printf("deallocation call from %s, file \"%s\", line %d\n", name, file, line);
      printf("orig. \"%s\" allocation call from %s, file \"%s\", line %d\n", j->label, j->func, j->file, j->line);
      }
    printf("s_free_debug(): deallocated %zd bytes successfully, total memory allocated now %d\n", j->mem, allocated_mem);
    }

/*
 * Remove the unused entry from memory record chunk.
 */
  mem_record_free(j);

  return (NULL);	/* Successful deallocation. */
  }


/**********************************************************************
  int memory_total()
  synopsis:     Return the total memory currently allocated through
                these routines.
  parameters:   None.
  return:       int	allocated_mem
  last updated: 07/12/98
 **********************************************************************/

int memory_total(void)
  {
  if (memory_verbose>0)
    printf("Total memory allocated:\t%d bytes.\n", allocated_mem);

  return(allocated_mem);
  }


/**********************************************************************
  void print_memory_alloc_to(void* mptr)
  synopsis:	Print the memory allocated to pointer mptr if is
                listed in the memory table.
  parameters:   pointer.
  return:	none.
  last updated: 07/12/98
 **********************************************************************/

void memory_print_alloc_to(void* mptr)
  {
  mem_record	*j;

  if(!mptr) { printf("Passed pointer is NULL!\n"); return;}

  j=match_mptr(mptr);
  if(j==NULL)
    {
    printf("Requested pointer not found in the memory table!\n");
    return;
    }

  printf("Total memory in %s is %zd, used = %zd\n", j->label, j->mem, j->rmem);

  return;
  }


/**********************************************************************
  void memory_alloc_to()
  synopsis:	Return the number of bytes allocated to pointer mptr
                if it is listed in the memory table.
  parameters:   pointer.
  return:       int.
  last updated: 07/12/98
 **********************************************************************/

int memory_alloc_to(void* mptr)
  {
  mem_record	*j;

  if(!mptr) return 0;

  j=match_mptr(mptr);
  if(j==NULL)
    {
    printf("Requested pointer not found in the memory table!\n");
    return 0;
    }

  return(j->mem);
  }


/**********************************************************************
  void memory_used_mptr()
  synopsis:     Return the number of bytes allocated to a pointer.
  parameters:   pointer.
  return:       int.
  last updated: 07/12/98
 **********************************************************************/

int memory_used_mptr(void *mptr)
  {
  mem_record	*j;

  if (mptr==NULL)
    {
    printf("WARNING: Passed null pointer!\n");
    return 0;
    }

  j=match_mptr(mptr);
  if (j==NULL)
    {
    printf("Requested pointer not found in the memory table!\n");
    return 0;
    }

  return j->rmem;
  }


/**********************************************************************
  void memory_display_status()
  synopsis:     Display information about status of memory allocation
                routines.
  parameters:   None.
  return:       None.
  last updated: 14/08/00
 **********************************************************************/

void memory_display_status(void)
  {

  printf("=== Memory Stats =============================\n");
  printf("Number of entries in memory table:  %d\n", num_mem);
  printf("Number of entries in memory tree:   %d\n", avltree_num_nodes(memtree));
  printf("Current size of memory table:       %d\n", max_mem);
  printf("Current total memory allocated:     %d bytes\n", allocated_mem);
  printf("Maximum total memory allocated:     %d bytes\n", most_mem);
  printf("----------------------------------------------\n");
  printf("Report level:                       %d\n", memory_verbose);
  printf("Strictness level:                   %d\n", memory_strict);
  printf("Padding flag:                       %d\n", memory_padding);
  printf("Size of padding:                    %zd\n", memory_size_pad);
  printf("Bounds check level:                 %d\n", memory_bounds);
  printf("Bounds violation reset flag:        %d\n", memory_reset_bv);
  printf("----------------------------------------------\n");
  printf("Total number of malloc() calls:     %ld\n", memory_count_malloc);
  printf("Total number of calloc() calls:     %ld\n", memory_count_calloc);
  printf("Total number of realloc() calls:    %ld\n", memory_count_realloc);
  printf("Total number of strdup() calls:     %ld\n", memory_count_strdup);
  printf("Total number of free() calls:       %ld\n", memory_count_free);
  printf("----------------------------------------------\n");
  printf("Total number of bounds violations:  %d\n", memory_count_bv);
  printf("Total number of invalid 'frees':    %d\n", memory_count_if);
  printf("==============================================\n");

  return;
  }


/**********************************************************************
  void memory_display_table()
  synopsis:     List all the memory allocated along with the entries
                labels.  WARNING: Table may be very long!
  parameters:   None.
  return:       None.
  last updated: 03/12/98
 **********************************************************************/

void memory_display_table(void)
  {

  if (num_mem==0)
    {
    printf("Memory allocation table is empty.\n");
    }
  else
    {
    node_count=0;
    printf("Memory tree contains %d nodes. (Should contain %d)\n",
           avltree_num_nodes(memtree), num_mem);
    printf("=== Memory Allocation Table ==================\n");
    printf("num  label\t  function\t  file\t  line\t  mem\t  rmem\t  (mptr)\n");
    avltree_traverse(memtree, table_traverse, NULL);
    printf("==============================================\n");
    printf("Counted %d nodes.\n", node_count);
    }

  return;
  }


/**********************************************************************
  void memory_set_mptr_label()
  synopsis:	Sets label for a table entry
  parameters:	pointer
                char *label
  return:	none
  updated:	20/01/99
 **********************************************************************/

void memory_set_mptr_label(void *mptr, char *label)
  {
  mem_record	*j;

  j=match_mptr(mptr);
  if(j==NULL)
    {
    printf("Requested pointer not found in memory table.  Can not reassign pointer label.\n");
    return;
    }

  strncpy(j->label,label,64);

  if (memory_verbose>2)
    printf("Label set to \"%s\"\n", label);

  return;
  }


/**********************************************************************
  void memory_set_verbose()
  synopsis:     Sets memory reporting level.
                0 = Only reasonably severe errors reported.
                3 = Full reporting.
                1,2 = Somewhere inbetween.
  parameters:   New memory reporting level.
  return:       None.
  last updated: 03/12/98
 **********************************************************************/

void memory_set_verbose(int i)
  {
  if (i<0 || i>3) {printf("No reporting level %d.\n",i); return;}

  memory_verbose=i;

  if (memory_verbose > 0) printf("Reporting level set to %d\n", memory_verbose);

  return;
  }


/**********************************************************************
  void memory_set_bounds()
  synopsis:     Sets memory bound checking level.
  parameters:   New memory bound level.
  return:       TRUE on success, otherwise FALSE.
  last updated: 01/12/98
 **********************************************************************/

boolean memory_set_bounds(int i)
  {

  if (memory_verbose > 0)
    {
    switch (memory_bounds)
      {
      case 0:
        printf("Bound Check level set to 0 (only check upon explicit request)\n");
        break;
      case 1:
        printf("Bound Check level set to 1 (check upon memory allocation)\n");
        break;
      case 2:
        printf("Bound Check level set to 2 (check upon memory deallocation)\n");
        break;
      case 3:
        printf("Bound Check level set to 3 (check upon memory allocation or deallocation)\n");
        break;
      default:
        printf("No Bound Check Level %d.\n",i);
        return FALSE;
      }
    }

  memory_bounds = i;

  return TRUE;
  }


/**********************************************************************
  void memory_set_strict()
  synopsis:     Sets memory strictness level which determines action
                when garbage pointers encountered
  parameters:   New memory strictness level.
  return:       None.
  last updated: 01/12/98
 **********************************************************************/

void memory_set_strict(int i)
  {
  if (i<0 || i>3) {printf("No Memory Strictness Level %d.\n", i); return;}

  memory_strict=i;
  if (memory_strict==0) printf("Memory Strictness set to Zero= garbage pointers treated as NULL\n");
  if (memory_strict==1) printf("Memory Strictness set to One= warn then garbage pointers treated as NULL\n");
  if (memory_strict==2) printf("Memory Strictness set to Two= garbage pointers prevent memory assignment\n");
  if (memory_strict==3) printf("Memory Strictness set to Three= garbage pointers terminate program\n");

  return;
  }


/**********************************************************************
  int memory_check_all_bounds()
  synopsis:     Checks memory padding for all entries in table.
  parameters:   None.
  return:       None.
  last updated: 03/12/98
 **********************************************************************/

int memory_check_all_bounds(void)
  {
  int	k=0;
/*
  int	i, j;
*/

  if (memory_verbose>2) printf("Checking memory bounds.\n");
  if (memory_reset_bv==0) memory_count_bv = 0;

  avltree_traverse(memtree, bounds_traverse, NULL);

#if 0

  for(i=0;i<num_mem;i++)
    {
    j=memory_check_bounds(mem[i].mptr);

    if (memory_verbose>0)
      {
      switch (j)
        {
        case (-1):
          printf("Pointer wasn't found in memory table.\n");
          break;
        case (0):
          if (memory_verbose>2) printf("Pointer passed bounds check.\n");
          break;
        case (1):
          printf("low bounds violation found, value=\"");
          for (k=0; k<mem[i].pad_ls; k+=sizeof(char)) printf("%c", mem[i].pad_low[k]);
          printf("\"\nentry=%d name=%s func=%s file=%s line=%d\n", i, mem[i].label, mem[i].func, mem[i].file, mem[i].line);
          break;
        case (2):
          printf("high bounds violation found, value=\"");
          for (k=0; k<mem[i].pad_hs; k+=sizeof(char)) printf("%c", mem[i].pad_high[k]);
          printf("\"\nentry=%d name=%s func=%s file=%s line=%d\n", i, mem[i].label, mem[i].func, mem[i].file, mem[i].line);
          break;
        case (3):
          printf("high and low bounds violations found, low value=\"");
          for (k=0; k<mem[i].pad_ls; k+=sizeof(char)) printf("%c", mem[i].pad_low[k]);
          printf("\", high value=\"");
          for (k=0; k<mem[i].pad_hs; k+=sizeof(char)) printf("%c", mem[i].pad_high[k]);
          printf("\"\nentry=%d name=%s func=%s file=%s line=%d\n", i, mem[i].label, mem[i].func, mem[i].file, mem[i].line);
          break;
        default:
          printf("WARNING: internal error in memory_util.c\n");
        }
      }

    if (j>0) k++;
    }
#endif

  if (k==1)
    {
    printf("WARNING: A bounds violation has been detected.\n");
    }
  else if (k>1)
    {
    printf("WARNING: %d bounds violations have been detected.\n",k);
    }
  else if(memory_verbose>2)
    {
    printf("Memory bounds are undisturbed.\n");
    }

  return k;
  }


/**********************************************************************
  int memory_check_bounds()
  synopsis:     Checks memory padding.  Resets padding, if required.
  parameters:   pointer in memory table.
  return:       None.
  last updated: 03/12/98
 **********************************************************************/

int memory_check_bounds(void* mptr)
  {
  mem_record	*j;
  int		m,n;

  j=match_mptr(mptr);
  if (j==NULL)
    {
    printf("WARNING: Pointer not found in the memory table!\n");
    return (-1);
    }

/* Update violations count, and reset padding, if appropriate. */
  m=check_pad_mptr_low(j);
  if (m)
    {
    memory_count_bv++;
    if (memory_reset_bv) pad_mptr_low(j);
    }

  n=check_pad_mptr_high(j);
    {
    memory_count_bv++;
    if (memory_reset_bv) pad_mptr_high(j);
    }

  return (2*n+m);
  }


/**********************************************************************
  void memory_set_padding()
  synopsis:     Sets memory padding level.
  parameters:   New memory padding level.
  return:       None.
  last updated: 01/12/98
 **********************************************************************/

void memory_set_padding(int i)
  {
  if (i > 0 && i < 4) memory_padding = i; else memory_padding = 0;

  if (memory_padding == 0) printf("memory padding turned off\n");
  if (memory_padding == 1) printf("memory now to be padded, high and low\n");
  if (memory_padding == 2) printf("memory now to be padded, high \n");
  if (memory_padding == 3) printf("memory now to be padded, low\n");

  return;
  }


/**********************************************************************
  void *s_malloc_safe()
  synopsis:     Wrapper around system's malloc() function.  Will
                never return upon failure so there is no need to
                check the return value.
  parameters:
  return:
  last updated:	14/08/00
 **********************************************************************/

void *s_malloc_safe(	size_t size,
                        const char *funcname, const char *filename, const int linenum)
  {
  void	*ptr;	/* Pointer to new memory */

  memory_count_malloc++;

  if (size==0)
    {
    printf("WARNING: Memory allocation of 0 bytes requested at func=%s file=%s line=%d\n",
           funcname, filename, linenum);
    return NULL;
    }

  if ( !(ptr = malloc(size)) )
    {
    printf("Memory allocation of %lu bytes failed at func=%s file=%s line=%d\n",
           (unsigned long) size, funcname, filename, linenum);
    perror("malloc");
    exit(EXIT_FAILURE);
    }

  return ptr;
  }


/**********************************************************************
  void *s_calloc_safe()
  synopsis:     Wrapper around system's calloc() function.  Will
                never return upon failure so there is no need to
                check the return value.
  parameters:
  return:
  last updated:	14/08/00
 **********************************************************************/

void *s_calloc_safe(	size_t num, size_t size,
                        const char *funcname, const char *filename, const int linenum)
  {
  void	*ptr;	/* Pointer to new memory */

  memory_count_calloc++;

  if (size==0 || num==0)
    {
    printf("WARNING: Memory allocation of 0 bytes requested at func=%s file=%s line=%d\n",
           funcname, filename, linenum);
    return NULL;
    }

  if ( !(ptr = calloc(num, size)) )
    {
    printf("Memory allocation of %lu bytes failed at func=%s file=%s line=%d\n",
           (unsigned long) num*size, funcname, filename, linenum);
    perror("calloc");
    exit(EXIT_FAILURE);
    }

  return ptr;
  }


/**********************************************************************
  void *s_realloc_safe()
  synopsis:     Wrapper around system's realloc() function.  Will
                never return upon failure so there is no need to
                check the return value.
  parameters:
  return:
  last updated:	14/08/00
 **********************************************************************/

void *s_realloc_safe(	void *oldptr, size_t size,
                        const char *funcname, const char *filename, const int linenum)
  {
  void	*ptr;	/* Pointer to new memory */

  memory_count_realloc++;

  if ( !(ptr = realloc(oldptr, size)) )
    {
    printf("Memory reallocation of %lu bytes failed at func=%s file=%s line=%d\n",
           (unsigned long) size, funcname, filename, linenum);
    perror("realloc");
    exit(EXIT_FAILURE);
    }

  return ptr;
  }


/**********************************************************************
  char *s_strdup_safe()
  synopsis:     Wrapper around system's strdup() function.  Will
                never return upon failure so there is no need to
                check the return value.
  parameters:
  return:
  last updated:	15/11/00
 **********************************************************************/

char *s_strdup_safe(	const char *src,
                        const char *funcname, const char *filename, const int linenum)
  {
  void		*dest;	/* Pointer to new string */
  size_t	len;	/* String length */

  memory_count_strdup++;

  if (!src)
    {
    printf("WARNING: strdup() of NULL string requested at func=%s file=%s line=%d\n",
           funcname, filename, linenum);
    return NULL;
    }

  len = strlen(src)+1;

/*
  if ( !(dest = strdup(src)) )
    {
    printf("String duplication of %d chars failed at func=%s file=%s line=%d\n",
           len, funcname, filename, linenum);
    perror("strdup");
    exit(EXIT_FAILURE);
    }
*/
  if ( !(dest = malloc(len*sizeof(char))) )
    {
    printf("String duplication of %lu chars failed at func=%s file=%s line=%d\n",
           (unsigned long) len, funcname, filename, linenum);
    perror("strdup");
    exit(EXIT_FAILURE);
    }

  memcpy(dest, src, len*sizeof(char));

  return (char *)dest;
  }


/**********************************************************************
  char *s_strndup_safe()
  synopsis:	strdup()-like function.  String will be null-terminated.
  parameters:
  return:
  last updated:	01/03/01
 **********************************************************************/

char *s_strndup_safe(	const char *src, size_t length,
                        const char *funcname, const char *filename, const int linenum )
  {
  void		*dest;	/* Pointer to new string */
  size_t	len;	/* String length */

  memory_count_strdup++;

  if (!src)
    {
    printf("WARNING: strndup() of NULL string requested at func=%s file=%s line=%d\n",
           funcname, filename, linenum);
    return NULL;
    }

  if (!length)
    {
    printf("WARNING: strndup() of zero-length string requested at func=%s file=%s line=%d\n",
           funcname, filename, linenum);
    return NULL;
    }

  len = strlen(src)+1;

  if (length < len) len = length;

  if ( !(dest = malloc(len*sizeof(char))) )
    {
    printf("String duplication of %lu chars failed at func=%s file=%s line=%d\n",
           (unsigned long) len, funcname, filename, linenum);
    perror("strdup");
    exit(EXIT_FAILURE);
    }

  len--;

  memcpy(dest, src, len*sizeof(char));

  ((char *) dest)[len] = '\0';

  return (char *)dest;
  }


/**********************************************************************
  void s_free_safe()
  synopsis:     Wrapper around system's free() function.  Will
                never return upon failure so there is no need to
                check the return value.
  parameters:
  return:
  last updated:	10/02/05
 **********************************************************************/

void s_free_safe(void *ptr,
                 const char *funcname,
                 const char *filename,
                 const int linenum)
  {
  memory_count_free++;

  if (ptr)
    free(ptr);
  else
    printf("Unable to free NULL pointer at func=%s file=%s line=%d\n",
           funcname, filename, linenum);

  return;
  }

