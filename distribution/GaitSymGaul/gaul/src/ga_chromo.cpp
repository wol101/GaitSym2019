/**********************************************************************
  ga_chromo.c
 **********************************************************************

  ga_chromo - Genetic algorithm chromosome handling routines.
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

  Synopsis:     Routines for handling GAUL's built-in chromosome types.

                The functions required for each chromsome type are:
                ga_chromosome_XXX_allocate() - assign memory.
                ga_chromosome_XXX_deallocate() - free memory.
                ga_chromosome_XXX_replicate() - copy genetic information.
                ga_chromosome_XXX_to_bytes() - for serialization.
                   (Leave max_bytes==0, if no need to free (i.e. static))
                ga_chromosome_XXX_from_bytes() - for deserialization.
                ga_chromosome_XXX_to_string() - Human readable NULL-
                   terminated string.

                The chromosome types are:
                integer - C int.
                boolean - boolean (generally, C _Bool by default).
                double - C double.
                char - C char.
                bitstring - bitstring.
                list - generic linked-list.

  To do:	Will need chromosome comparison functions.

 **********************************************************************/

#include "gaul/ga_chromo.h"

/**********************************************************************
  ga_chromosome_integer_allocate()
  synopsis:	Allocate the chromosomes for an entity.  Initial
                contents are garbage (there is no need to zero them).
  parameters:
  return:
  last updated: 05 Oct 2004
 **********************************************************************/

boolean ga_chromosome_integer_allocate( population *pop,
                                        entity *embryo )
  {
  int		i;		/* Loop variable over all chromosomes */

  if (!pop) die("Null pointer to population structure passed.");
  if (!embryo) die("Null pointer to entity structure passed.");

  if (embryo->chromosome!=NULL)
    die("This entity already contains chromosomes.");

#if USE_CHROMO_CHUNKS == 1
  THREAD_LOCK(pop->chromo_chunk_lock);
  if (!pop->chromo_chunk)
    {
    pop->chromoarray_chunk = mem_chunk_new(pop->num_chromosomes*sizeof(int *), 1024);
    pop->chromo_chunk = mem_chunk_new(pop->num_chromosomes*pop->len_chromosomes*sizeof(int), 2048);
    }

  embryo->chromosome = mem_chunk_alloc(pop->chromoarray_chunk);
  embryo->chromosome[0] = mem_chunk_alloc(pop->chromo_chunk);
  THREAD_UNLOCK(pop->chromo_chunk_lock);
#else
  embryo->chromosome = (void **)s_malloc(pop->num_chromosomes*sizeof(int *));
  embryo->chromosome[0] = s_malloc(pop->num_chromosomes*pop->len_chromosomes*sizeof(int));
#endif

  for (i=1; i<pop->num_chromosomes; i++)
    {
    embryo->chromosome[i] = &(((int *)embryo->chromosome[i-1])[pop->len_chromosomes]);
    }

  return TRUE;
  }


/**********************************************************************
  ga_chromosome_integer_deallocate()
  synopsis:	Deallocate the chromosomes for an entity.
  parameters:
  return:
  last updated: 05 Oct 2004
 **********************************************************************/

void ga_chromosome_integer_deallocate( population *pop,
                                       entity *corpse )
  {

  if (!pop) die("Null pointer to population structure passed.");
  if (!corpse) die("Null pointer to entity structure passed.");

  if (corpse->chromosome==NULL)
    die("This entity already contains no chromosomes.");

#if USE_CHROMO_CHUNKS == 1
  THREAD_LOCK(pop->chromo_chunk_lock);
  mem_chunk_free(pop->chromo_chunk, corpse->chromosome[0]);
  mem_chunk_free(pop->chromoarray_chunk, corpse->chromosome);
  corpse->chromosome=NULL;

  if (mem_chunk_isempty(pop->chromo_chunk))
    {
    mem_chunk_destroy(pop->chromo_chunk);
    mem_chunk_destroy(pop->chromoarray_chunk);
    pop->chromo_chunk = NULL;
    }
  THREAD_UNLOCK(pop->chromo_chunk_lock);
#else
  s_free(corpse->chromosome[0]);
  s_free(corpse->chromosome);
  corpse->chromosome=NULL;
#endif

  return;
  }


/**********************************************************************
  ga_chromosome_integer_replicate()
  synopsis:	Duplicate a chromosome exactly.
  parameters:
  return:
  last updated: 13/06/01
 **********************************************************************/

void ga_chromosome_integer_replicate( const population *pop,
                                      entity *parent, entity *child,
                                      const int chromosomeid )
  {

  if (!pop) die("Null pointer to population structure passed.");
  if (!parent || !child) die("Null pointer to entity structure passed.");
  if (!parent->chromosome || !child->chromosome) die("Entity has no chromsomes.");

  memcpy( child->chromosome[chromosomeid],
          parent->chromosome[chromosomeid],
          pop->len_chromosomes * sizeof(int));

  return;
  }


/**********************************************************************
  ga_chromosome_integer_to_bytes()
  synopsis:	Convert to contiguous form.  In this case, a trivial
                process.
  parameters:
  return:	Number of bytes processed.
  last updated: 1 Feb 2002
 **********************************************************************/

unsigned int ga_chromosome_integer_to_bytes(const population *pop, entity *joe,
                                     byte **bytes, unsigned int *max_bytes)
  {
  int		num_bytes;	/* Actual size of genes. */

  if (!pop) die("Null pointer to population structure passed.");
  if (!joe) die("Null pointer to entity structure passed.");

  if (*max_bytes!=0) die("Internal error.");

  if (!joe->chromosome)
    {
    *bytes = (byte *)"\0";
    return 0;
    }

  num_bytes = pop->len_chromosomes * pop->num_chromosomes *
              sizeof(int);

  *bytes = (byte *)joe->chromosome[0];

  return num_bytes;
  }


/**********************************************************************
  ga_chromosome_integer_from_bytes()
  synopsis:	Convert from contiguous form.  In this case, a trivial
                process.
  parameters:
  return:
  last updated: 1 Feb 2002
 **********************************************************************/

void ga_chromosome_integer_from_bytes(const population *pop, entity *joe, byte *bytes)
  {

  if (!pop) die("Null pointer to population structure passed.");
  if (!joe) die("Null pointer to entity structure passed.");

  if (!joe->chromosome) die("Entity has no chromsomes.");

  memcpy(joe->chromosome[0], bytes,
         pop->len_chromosomes * pop->num_chromosomes * sizeof(int));

  return;
  }


/**********************************************************************
  ga_chromosome_integer_to_string()
  synopsis:	Convert to human readable form.
  parameters:	const population *pop	Population (compatible with entity)
                const entity *joe	Entity to encode as text.
                char *text		Malloc()'ed text buffer, or NULL.
                size_t *textlen		Current size of text buffer.
  return:
  last updated: 19 Aug 2002
 **********************************************************************/

char *ga_chromosome_integer_to_string(
                              const population *pop, const entity *joe,
                              char *text, size_t *textlen)
  {
  int		i, j;		/* Loop over chromosome, alleles. */
  int		k=0;		/* Pointer into 'text'. */
  int		l;		/* Number of appended digits. */

  if (!pop) die("Null pointer to population structure passed.");
  if (!joe) die("Null pointer to entity structure passed.");

/* Ensure that a reasonable amount of memory is allocated. */
  if (!text || *textlen < 8 * pop->len_chromosomes * pop->num_chromosomes)
    {
    *textlen = 8 * pop->len_chromosomes * pop->num_chromosomes;
    text = (char *)s_realloc(text, sizeof(char) * *textlen);
    }

/* Handle empty chromosomes. */
  if (!joe->chromosome)
    {
    text[1] = '\0';
    return text;
    }

  for(i=0; i<pop->num_chromosomes; i++)
    {
    for(j=0; j<pop->len_chromosomes; j++)
      {
      if (*textlen-k<8)
        {
        *textlen *= 2;   /* FIXME: This isn't intelligent. */
        text = (char *)s_realloc(text, sizeof(char) * *textlen);
        }

      l = snprintf(&(text[k]), *textlen-k, "%d ",
                       ((int *)joe->chromosome[i])[j]);

      if (l == -1)
        {	/* Truncation occured. */
        *textlen *= 2;	/* FIXME: This isn't intelligent. */
        text = (char *)s_realloc(text, sizeof(char) * *textlen);
        l = snprintf(&(text[k]), *textlen-k, "%d ",
                       ((int *)joe->chromosome[i])[j]);

        if (l == -1) die("Internal error, string truncated again.");
        }

      k += l;
      }
    }

/* Replace last space character with NULL character. */
  text[k-1]='\0';

  return text;
  }


/**********************************************************************
  ga_chromosome_boolean_allocate()
  synopsis:	Allocate the chromosomes for an entity.  Initial
                contents are garbage (there is no need to zero them).
  parameters:
  return:
  last updated: 13/06/01
 **********************************************************************/

boolean ga_chromosome_boolean_allocate(population *pop, entity *embryo)
  {
  int		i;		/* Loop variable over all chromosomes */

  if (!pop) die("Null pointer to population structure passed.");
  if (!embryo) die("Null pointer to entity structure passed.");

  if (embryo->chromosome!=NULL)
    die("This entity already contains chromosomes.");

  embryo->chromosome = (void **)s_malloc(pop->num_chromosomes*sizeof(boolean *));
  embryo->chromosome[0] = s_malloc(pop->num_chromosomes*pop->len_chromosomes*sizeof(boolean));

  for (i=1; i<pop->num_chromosomes; i++)
    {
    embryo->chromosome[i] = &(((boolean *)embryo->chromosome[i-1])[pop->len_chromosomes]);
    }

  return TRUE;
  }


/**********************************************************************
  ga_chromosome_boolean_deallocate()
  synopsis:	Deallocate the chromosomes for an entity.
  parameters:
  return:
  last updated: 13/06/01
 **********************************************************************/

void ga_chromosome_boolean_deallocate(population *pop, entity *corpse)
  {

  if (!pop) die("Null pointer to population structure passed.");
  if (!corpse) die("Null pointer to entity structure passed.");

  if (corpse->chromosome==NULL)
    die("This entity already contains no chromosomes.");

  s_free(corpse->chromosome[0]);
  s_free(corpse->chromosome);
  corpse->chromosome=NULL;

  return;
  }


/**********************************************************************
  ga_chromosome_boolean_replicate()
  synopsis:	Duplicate a chromosome exactly.
  parameters:
  return:
  last updated: 19 Mar 2002
 **********************************************************************/

void ga_chromosome_boolean_replicate( const population *pop,
                                      entity *parent, entity *child,
                                      const int chromosomeid )
  {

  if (!pop) die("Null pointer to population structure passed.");
  if (!parent || !child) die("Null pointer to entity structure passed.");
  if (!parent->chromosome || !child->chromosome) die("Entity has no chromsomes.");

  memcpy(child->chromosome[chromosomeid], parent->chromosome[chromosomeid],
              pop->len_chromosomes * sizeof(boolean));

  return;
  }


/**********************************************************************
  ga_chromosome_boolean_to_bytes()
  synopsis:	Convert to contiguous form.  In this case, a trivial
                process.  (Note that we could compress the data at this
                point but CPU time is currenty more important to me
                than memory or bandwidth)
  parameters:
  return:
  last updated: 1 Feb 2002
 **********************************************************************/

unsigned int ga_chromosome_boolean_to_bytes(const population *pop, entity *joe,
                                    byte **bytes, unsigned int *max_bytes)
  {
  int		num_bytes;	/* Actual size of genes. */

  if (!pop) die("Null pointer to population structure passed.");
  if (!joe) die("Null pointer to entity structure passed.");

  if (*max_bytes!=0) die("Internal error.");

  if (!joe->chromosome)
    {
    *bytes = (byte *)"\0";
    return 0;
    }

  num_bytes = pop->len_chromosomes * pop->num_chromosomes *
              sizeof(boolean);

  *bytes = (byte *)joe->chromosome[0];

  return num_bytes;
  }


/**********************************************************************
  ga_chromosome_boolean_from_bytes()
  synopsis:	Convert from contiguous form.  In this case, a trivial
                process.
  parameters:
  return:
  last updated: 1 Feb 2002
 **********************************************************************/

void ga_chromosome_boolean_from_bytes(const population *pop, entity *joe, byte *bytes)
  {

  if (!pop) die("Null pointer to population structure passed.");
  if (!joe) die("Null pointer to entity structure passed.");

  if (!joe->chromosome) die("Entity has no chromsomes.");

  memcpy(joe->chromosome[0], bytes,
         pop->len_chromosomes * pop->num_chromosomes * sizeof(boolean));

  return;
  }


/**********************************************************************
  ga_chromosome_boolean_to_string()
  synopsis:	Convert to human readable form.
  parameters:
  return:
  last updated: 19 Aug 2002
 **********************************************************************/

char *ga_chromosome_boolean_to_string(
                              const population *pop, const entity *joe,
                              char *text, size_t *textlen)
  {
  int		i, j;		/* Loop over chromosome, alleles. */
  int		k=0;		/* Pointer into 'text'. */

  if (!pop) die("Null pointer to population structure passed.");
  if (!joe) die("Null pointer to entity structure passed.");

  if (!text || *textlen < pop->len_chromosomes * pop->num_chromosomes + 1)
    {
    *textlen = pop->len_chromosomes * pop->num_chromosomes + 1;
    text = (char *)s_realloc(text, sizeof(char) * *textlen);
    }

  if (!joe->chromosome)
    {
    text[0] = '\0';
    }
  else
    {
    for(i=0; i<pop->num_chromosomes; i++)
      {
      for(j=0; j<pop->len_chromosomes; j++)
        {
        text[k++] = ((boolean *)joe->chromosome[i])[j]?'1':'0';
        }
      }
    text[k] = '\0';
    }

  return text;
  }


/**********************************************************************
  ga_chromosome_double_allocate()
  synopsis:	Allocate the chromosomes for an entity.  Initial
                contents are garbage (there is no need to zero them).
  parameters:
  return:
  last updated: 16/06/01
 **********************************************************************/

boolean ga_chromosome_double_allocate(population *pop, entity *embryo)
  {
  int		i;		/* Loop variable over all chromosomes */

  if (!pop) die("Null pointer to population structure passed.");
  if (!embryo) die("Null pointer to entity structure passed.");

  if (embryo->chromosome!=NULL)
    die("This entity already contains chromosomes.");

  embryo->chromosome = (void **)s_malloc(pop->num_chromosomes*sizeof(double *));
  embryo->chromosome[0] = s_malloc(pop->num_chromosomes*pop->len_chromosomes*sizeof(double));

  for (i=1; i<pop->num_chromosomes; i++)
    {
    embryo->chromosome[i] = &(((double *)embryo->chromosome[i-1])[pop->len_chromosomes]);
    }

  return TRUE;
  }


/**********************************************************************
  ga_chromosome_double_deallocate()
  synopsis:	Deallocate the chromosomes for an entity.
  parameters:
  return:
  last updated: 16/06/01
 **********************************************************************/

void ga_chromosome_double_deallocate(population *pop, entity *corpse)
  {

  if (!pop) die("Null pointer to population structure passed.");
  if (!corpse) die("Null pointer to entity structure passed.");

  if (corpse->chromosome==NULL)
    die("This entity already contains no chromosomes.");

  s_free(corpse->chromosome[0]);
  s_free(corpse->chromosome);
  corpse->chromosome=NULL;

  return;
  }


/**********************************************************************
  ga_chromosome_double_replicate()
  synopsis:	Duplicate a chromosome exactly.
  parameters:
  return:
  last updated: 16/06/01
 **********************************************************************/

void ga_chromosome_double_replicate( const population *pop,
                                      entity *parent, entity *child,
                                      const int chromosomeid )
  {

  if (!pop) die("Null pointer to population structure passed.");
  if (!parent || !child) die("Null pointer to entity structure passed.");
  if (!parent->chromosome || !child->chromosome) die("Entity has no chromsomes.");

  memcpy(child->chromosome[chromosomeid], parent->chromosome[chromosomeid],
              pop->len_chromosomes * sizeof(double));

  return;
  }


/**********************************************************************
  ga_chromosome_double_to_bytes()
  synopsis:	Convert to contiguous form.  In this case, a trivial
                process.  (Note that we could compress the data at this
                point but CPU time is currenty more important to me
                than memory or bandwidth)
  parameters:
  return:
  last updated: 1 Feb 2002
 **********************************************************************/

unsigned int ga_chromosome_double_to_bytes(const population *pop, entity *joe,
                                    byte **bytes, unsigned int *max_bytes)
  {
  int		num_bytes;	/* Actual size of genes. */

  if (!pop) die("Null pointer to population structure passed.");
  if (!joe) die("Null pointer to entity structure passed.");

  if (*max_bytes!=0) die("Internal error.");

  if (!joe->chromosome)
    {
    *bytes = (byte *)"\0";
    return 0;
    }

  num_bytes = pop->len_chromosomes * pop->num_chromosomes *
              sizeof(double);

  *bytes = (byte *)joe->chromosome[0];

  return num_bytes;
  }


/**********************************************************************
  ga_chromosome_double_from_bytes()
  synopsis:	Convert from contiguous form.  In this case, a trivial
                process.
  parameters:
  return:
  last updated: 1 Feb 2002
 **********************************************************************/

void ga_chromosome_double_from_bytes(const population *pop, entity *joe, byte *bytes)
  {

  if (!pop) die("Null pointer to population structure passed.");
  if (!joe) die("Null pointer to entity structure passed.");

  if (!joe->chromosome) die("Entity has no chromsomes.");

  memcpy(joe->chromosome[0], bytes,
         pop->len_chromosomes * pop->num_chromosomes * sizeof(double));

  return;
  }


/**********************************************************************
  ga_chromosome_double_to_string()
  synopsis:	Convert to human readable form.
  parameters:
  return:
  last updated: 19 Aug 2002
 **********************************************************************/

char *ga_chromosome_double_to_string(
                              const population *pop, const entity *joe,
                              char *text, size_t *textlen)
  {
  int		i, j;		/* Loop over chromosome, alleles. */
  int		k=0;		/* Pointer into 'text'. */
  int		l;		/* Number of 'snprintf'ed characters. */

  if (!pop) die("Null pointer to population structure passed.");
  if (!joe) die("Null pointer to entity structure passed.");

  if (!text || *textlen < 10 * pop->len_chromosomes * pop->num_chromosomes)
    {
    *textlen = 10 * pop->len_chromosomes * pop->num_chromosomes;
    text = (char *)s_realloc(text, sizeof(char) * *textlen);
    }

  if (!joe->chromosome)
    {
    text[1] = '\0';
    return text;
    }

  for(i=0; i<pop->num_chromosomes; i++)
    {
    for(j=0; j<pop->len_chromosomes; j++)
      {
      if (*textlen-k<8)
        {
        *textlen *= 2;	/* FIXME: This isn't intelligent. */
        text = (char *)s_realloc(text, sizeof(char) * *textlen);
        }

      l = snprintf(&(text[k]), *textlen-k, "%f ",
                       ((double *)joe->chromosome[i])[j]);

      if (l == -1)
        {	/* Truncation occured. */
        *textlen *= 2;	/* FIXME: This isn't intelligent. */
        text = (char *)s_realloc(text, sizeof(char) * *textlen);
        l = snprintf(&(text[k]), *textlen-k, "%f ",
                       ((double *)joe->chromosome[i])[j]);

        if (l == -1) die("Internal error, string truncated again.");
        }

      k += l;
      }
    }

  text[k-1] = '\0';

  return text;
  }


/**********************************************************************
  ga_chromosome_char_allocate()
  synopsis:	Allocate the chromosomes for an entity.  Initial
                contents are garbage (there is no need to zero them).
  parameters:
  return:
  last updated: 16/06/01
 **********************************************************************/

boolean ga_chromosome_char_allocate(population *pop, entity *embryo)
  {
  int		i;		/* Loop variable over all chromosomes */

  if (!pop) die("Null pointer to population structure passed.");
  if (!embryo) die("Null pointer to entity structure passed.");

  if (embryo->chromosome!=NULL)
    die("This entity already contains chromosomes.");

  embryo->chromosome = (void **)s_malloc(pop->num_chromosomes*sizeof(char *));
  embryo->chromosome[0] = s_malloc(pop->num_chromosomes*pop->len_chromosomes*sizeof(char));

  for (i=1; i<pop->num_chromosomes; i++)
    {
    embryo->chromosome[i] = &(((char *)embryo->chromosome[i-1])[pop->len_chromosomes]);
    }

  return TRUE;
  }


/**********************************************************************
  ga_chromosome_char_deallocate()
  synopsis:	Deallocate the chromosomes for an entity.
  parameters:
  return:
  last updated: 16/06/01
 **********************************************************************/

void ga_chromosome_char_deallocate(population *pop, entity *corpse)
  {

  if (!pop) die("Null pointer to population structure passed.");
  if (!corpse) die("Null pointer to entity structure passed.");

  if (corpse->chromosome==NULL)
    die("This entity already contains no chromosomes.");

/*  ga_entity_dump(pop, corpse);*/

  s_free(corpse->chromosome[0]);
  s_free(corpse->chromosome);
  corpse->chromosome=NULL;

  return;
  }


/**********************************************************************
  ga_chromosome_char_replicate()
  synopsis:	Duplicate a chromosome exactly.
  parameters:
  return:
  last updated: 19 Mar 2002
 **********************************************************************/

void ga_chromosome_char_replicate( const population *pop,
                                   entity *parent, entity *child,
                                   const int chromosomeid )
  {

  if (!pop) die("Null pointer to population structure passed.");
  if (!parent || !child) die("Null pointer to entity structure passed.");
  if (!parent->chromosome || !child->chromosome) die("Entity has no chromsomes.");

  memcpy(child->chromosome[chromosomeid], parent->chromosome[chromosomeid],
              pop->len_chromosomes * sizeof(char));

  return;
  }


/**********************************************************************
  ga_chromosome_char_to_bytes()
  synopsis:	Convert to contiguous form.  In this case, a highly
                trivial process.
  parameters:
  return:
  last updated: 1 Feb 2002
 **********************************************************************/

unsigned int ga_chromosome_char_to_bytes(const population *pop, entity *joe,
                                    byte **bytes, unsigned int *max_bytes)
  {
  int		num_bytes;	/* Actual size of genes. */

  if (!pop) die("Null pointer to population structure passed.");
  if (!joe) die("Null pointer to entity structure passed.");

  if (*max_bytes!=0) die("Internal error.");

  if (!joe->chromosome)
    {
    *bytes = (byte *)"\0";
    return 0;
    }

  num_bytes = pop->len_chromosomes * pop->num_chromosomes * sizeof(char);

  *bytes = (byte *)joe->chromosome[0];

  return num_bytes;
  }


/**********************************************************************
  ga_chromosome_char_from_bytes()
  synopsis:	Convert from contiguous form.  In this case, a trivial
                process.
  parameters:
  return:
  last updated: 1 Feb 2002
 **********************************************************************/

void ga_chromosome_char_from_bytes(const population *pop, entity *joe, byte *bytes)
  {

  if (!pop) die("Null pointer to population structure passed.");
  if (!joe) die("Null pointer to entity structure passed.");

  if (!joe->chromosome) die("Entity has no chromsomes.");

  memcpy(joe->chromosome[0], bytes,
         pop->len_chromosomes * pop->num_chromosomes * sizeof(char));

  return;
  }


/**********************************************************************
  ga_chromosome_char_to_string()
  synopsis:	Convert genetic data into human readable form.
  parameters:
  return:
  last updated: 19 Aug 2002
 **********************************************************************/

char *ga_chromosome_char_to_string(
                              const population *pop, const entity *joe,
                              char *text, size_t *textlen)
  {
  int		i;		/* Loop over chromosome, alleles. */
  int		k=0;		/* Pointer into 'text'. */

  if (!pop) die("Null pointer to population structure passed.");
  if (!joe) die("Null pointer to entity structure passed.");

  if (*textlen < pop->len_chromosomes * pop->num_chromosomes + 1)
    {
    *textlen = pop->len_chromosomes * pop->num_chromosomes + 1;
    text = (char *)s_realloc(text, sizeof(char) * *textlen);
    }

  if (!joe->chromosome)
    {
    text[0] = '\0';
    }
  else
    {
    for(i=0; i<pop->num_chromosomes; i++)
      {
      memcpy(&(text[k]), joe->chromosome[0],
         pop->len_chromosomes * sizeof(char));
      k += pop->len_chromosomes;
      }
    text[k] = '\0';
    }

  return text;
  }


/**********************************************************************
  ga_chromosome_bitstring_allocate()
  synopsis:	Allocate the chromosomes for an entity.  Initial
                contents are garbage (there is no need to zero them).
  parameters:
  return:
  last updated: 30/06/01
 **********************************************************************/

boolean ga_chromosome_bitstring_allocate(population *pop, entity *embryo)
  {
  int		i;		/* Loop variable over all chromosomes */

  if (!pop) die("Null pointer to population structure passed.");
  if (!embryo) die("Null pointer to entity structure passed.");

  if (embryo->chromosome!=NULL)
    die("This entity already contains chromosomes.");

  embryo->chromosome = (void **)s_malloc(pop->num_chromosomes*sizeof(byte *));

  for (i=0; i<pop->num_chromosomes; i++)
    embryo->chromosome[i] = ga_bit_new(pop->len_chromosomes);

  return TRUE;
  }


/**********************************************************************
  ga_chromosome_bitstring_deallocate()
  synopsis:	Deallocate the chromosomes for an entity.
  parameters:
  return:
  last updated: 30/06/01
 **********************************************************************/

void ga_chromosome_bitstring_deallocate(population *pop, entity *corpse)
  {
  int		i;		/* Loop variable over all chromosomes */

  if (!pop) die("Null pointer to population structure passed.");
  if (!corpse) die("Null pointer to entity structure passed.");

  if (corpse->chromosome==NULL)
    die("This entity already contains no chromosomes.");

  for (i=0; i<pop->num_chromosomes; i++)
    ga_bit_free((byte *)corpse->chromosome[i]);

  s_free(corpse->chromosome);
  corpse->chromosome=NULL;

  return;
  }


/**********************************************************************
  ga_chromosome_bitstring_replicate()
  synopsis:	Duplicate a chromosome exactly.
  parameters:
  return:
  last updated: 30/06/01
 **********************************************************************/

void ga_chromosome_bitstring_replicate( const population *pop,
                                      entity *parent, entity *child,
                                      const int chromosomeid )
  {

  if (!pop) die("Null pointer to population structure passed.");
  if (!parent || !child) die("Null pointer to entity structure passed.");
  if (!parent->chromosome || !child->chromosome) die("Entity has no chromsomes.");

  ga_bit_clone( (byte *)child->chromosome[chromosomeid],
                (byte *)parent->chromosome[chromosomeid],
                pop->len_chromosomes );

  return;
  }


/**********************************************************************
  ga_chromosome_bitstring_to_bytes()
  synopsis:	Convert to contiguous form.
  parameters:
  return:
  last updated: 30/06/01
 **********************************************************************/

unsigned int ga_chromosome_bitstring_to_bytes(const population *pop,
                                    entity *joe,
                                     byte **bytes, unsigned int *max_bytes)
  {
  int		num_bytes;	/* Actual size of genes. */
  int		i;		/* Loop variable over all chromosomes */

  if (!pop) die("Null pointer to population structure passed.");
  if (!joe) die("Null pointer to entity structure passed.");

  num_bytes = ga_bit_sizeof(pop->len_chromosomes) * pop->num_chromosomes;

  if (num_bytes>*max_bytes)
    {
    *max_bytes = num_bytes;
    *bytes = (byte *)s_realloc(*bytes, *max_bytes*sizeof(byte));
    /* sizeof(byte) should always be 1 */
    }

  if (!joe->chromosome)
    {
    *bytes = (byte *)0;
    return 0;
    }

  for(i=0; i<pop->num_chromosomes; i++)
    {
    ga_bit_copy( *bytes, (byte *)joe->chromosome[i],
                 i*pop->len_chromosomes, 0,
                 pop->len_chromosomes );
    }

  return num_bytes;
  }


/**********************************************************************
  ga_chromosome_bitstring_from_bytes()
  synopsis:	Convert from contiguous form.  In this case, a trivial
                process.
  parameters:
  return:
  last updated: 13/06/01
 **********************************************************************/

void ga_chromosome_bitstring_from_bytes( const population *pop,
                                         entity *joe,
                                         byte *bytes )
  {
  int		i;		/* Loop variable over all chromosomes */

  if (!pop) die("Null pointer to population structure passed.");
  if (!joe) die("Null pointer to entity structure passed.");

  if (!joe->chromosome) die("Entity has no chromsomes.");

  for(i=0; i<pop->num_chromosomes; i++)
    {
    ga_bit_copy( (byte *)joe->chromosome[i], bytes,
                 0, i*pop->len_chromosomes,
                 pop->len_chromosomes );
    }

  return;
  }


/**********************************************************************
  ga_chromosome_bitstring_to_string()
  synopsis:	Convert to human readable form.
  parameters:
  return:
  last updated: 19 Aug 2002
 **********************************************************************/

char *ga_chromosome_bitstring_to_string(
                              const population *pop, const entity *joe,
                              char *text, size_t *textlen)
  {
  int		i, j;		/* Loop over chromosome, alleles. */
  int		k=0;		/* Pointer into 'text'. */

  if (!pop) die("Null pointer to population structure passed.");
  if (!joe) die("Null pointer to entity structure passed.");

  if (!text || *textlen < pop->len_chromosomes * pop->num_chromosomes + 1)
    {
    *textlen = pop->len_chromosomes * pop->num_chromosomes + 1;
    text = (char *)s_realloc(text, sizeof(char) * *textlen);
    }

  if (!joe->chromosome)
    {
    text[0] = '\0';
    }
  else
    {
    for(i=0; i<pop->num_chromosomes; i++)
      {
      for(j=0; j<pop->len_chromosomes; j++)
        {
        text[k++] = ga_bit_get((byte *)joe->chromosome[i],j)?'1':'0';
        }
      }
    text[k] = '\0';
    }

  return text;
  }


/**********************************************************************
  ga_chromosome_list_allocate()
  synopsis:	Allocate the chromosomes for an entity.  Initial
                contents are set to null.
  parameters:
  return:
  last updated: 05 Oct 2004
 **********************************************************************/

boolean ga_chromosome_list_allocate(population *pop, entity *embryo)
  {
  int		i;		/* Loop variable over all chromosomes */

  if (!pop) die("Null pointer to population structure passed.");
  if (!embryo) die("Null pointer to entity structure passed.");

  if (embryo->chromosome!=NULL)
    die("This entity already contains chromosomes.");

  embryo->chromosome = (void **)s_malloc(pop->num_chromosomes*sizeof(DLList *));

  for (i=0; i<pop->num_chromosomes; i++)
    embryo->chromosome[i] = NULL;

  return TRUE;
  }


/**********************************************************************
  ga_chromosome_list_deallocate()
  synopsis:	Deallocate the chromosomes for an entity.
  parameters:
  return:
  last updated: 05 Oct 2004
 **********************************************************************/

void ga_chromosome_list_deallocate(population *pop, entity *corpse)
  {
  int		i;		/* Loop variable over all chromosomes */

  if (!pop) die("Null pointer to population structure passed.");
  if (!corpse) die("Null pointer to entity structure passed.");

  if (corpse->chromosome==NULL)
    die("This entity already contains no chromosomes.");

  for (i=0; i<pop->num_chromosomes; i++)
    dlink_free_all((DLList *)corpse->chromosome[i]);

  s_free(corpse->chromosome);
  corpse->chromosome=NULL;

  return;
  }


/**********************************************************************
  ga_chromosome_list_replicate()
  synopsis:	Duplicate a chromosome exactly.
                Currently unimplemented at present.
  parameters:
  return:
  last updated: 05 Oct 2004
 **********************************************************************/

void ga_chromosome_list_replicate( const population *pop,
                                      entity *parent, entity *child,
                                      const int chromosomeid )
  {

  if (!pop) die("Null pointer to population structure passed.");
  if (!parent || !child) die("Null pointer to entity structure passed.");
  if (!parent->chromosome || !child->chromosome) die("Entity has no chromsomes.");

  child->chromosome[chromosomeid] = dlink_clone(
                (DLList *)parent->chromosome[chromosomeid] );

  return;
  }


/**********************************************************************
  ga_chromosome_list_to_bytes()
  synopsis:	Convert to contiguous form.
                Currently unimplemented at present.
                FIXME: Need a user-defined callback to implement this
                according to contents of the list.
  parameters:
  return:
  last updated: 05 Oct 2004
 **********************************************************************/

unsigned int ga_chromosome_list_to_bytes(const population *pop,
                                    entity *joe,
                                     byte **bytes, unsigned int *max_bytes)
  {
  int		num_bytes=0;	/* Actual size of genes. */

  if (!pop) die("Null pointer to population structure passed.");
  if (!joe) die("Null pointer to entity structure passed.");

  die("ga_chromosome_list_to_bytes() is not implemented.");

  /* Avoid compiler warnings. */
  **bytes = 0;
  *max_bytes = 0;

  return num_bytes;
  }


/**********************************************************************
  ga_chromosome_list_from_bytes()
  synopsis:	Convert from contiguous form.  In this case, a trivial
                process.
                Currently unimplemented at present.
                FIXME: Need a user-defined callback to implement this
                according to contents of the list.
  parameters:
  return:
  last updated: 05 Oct 2004
 **********************************************************************/

void ga_chromosome_list_from_bytes( const population *pop,
                                         entity *joe,
                                         byte *bytes )
  {

  if (!pop) die("Null pointer to population structure passed.");
  if (!joe) die("Null pointer to entity structure passed.");

  if (!joe->chromosome) die("Entity has no chromsomes.");

  die("ga_chromosome_list_from_bytes() is not implemented.");

  /* Avoid compiler warning. */
  *bytes = 0;

  return;
  }


/**********************************************************************
  ga_chromosome_list_to_string()
  synopsis:	Convert to human readable form.
                Currently unimplemented at present.
                FIXME: Need a user-defined callback to implement this
                according to contents of the list.
  parameters:
  return:
  last updated: 05 Oct 2004
 **********************************************************************/

char *ga_chromosome_list_to_string(
                              const population *pop, const entity *joe,
                              char *text, size_t *textlen)
  {

  if (!pop) die("Null pointer to population structure passed.");
  if (!joe) die("Null pointer to entity structure passed.");

  if (!text || *textlen < 14)
    {
    *textlen = 14;
    text = (char *)s_realloc(text, sizeof(char) * *textlen);
    }

  strncpy(text, "<unavailable>", 14);

  return text;
  }



