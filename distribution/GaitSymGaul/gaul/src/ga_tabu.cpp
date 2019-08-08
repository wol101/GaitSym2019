/**********************************************************************
  ga_tabu.c
 **********************************************************************

  ga_tabu - A tabu-search algorithm for comparison and local search.
  Copyright ©2002-2005, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:     A tabu-search algorithm for comparison and local search.

                A novel population-based tabu-search is also provided.
                (Or at least it will be!)

 **********************************************************************/

#include "gaul/ga_tabu.h"

/**********************************************************************
  ga_tabu_check_integer()
  synopsis:     Compares two solutions with integer chromosomes and
                returns TRUE if, and only if, they are exactly
                identical.
  parameters:
  return:
  last updated: 10 Oct 2002
 **********************************************************************/

boolean ga_tabu_check_integer(  population      *pop,
                                entity          *putative,
                                entity          *tabu)
{
    int         i;              /* Loop variable over chromosomes. */
    int         j;              /* Loop variable over alleles. */
    int         *a, *b;         /* Comparison integer arrays. */

    /* Checks. */
    if ( !pop ) die("Null pointer to population structure passed.");
    if ( !putative || !tabu ) die("Null pointer to entity structure passed.");

    for (i=0; i<pop->num_chromosomes; i++)
    {
        a = (int*)(putative->chromosome[i]);
        b = (int*)(tabu->chromosome[i]);

        for (j=0; j<pop->len_chromosomes; j++)
            if (a[j] != b[j]) return FALSE;
    }

    return TRUE;
}


/**********************************************************************
  ga_tabu_check_char()
  synopsis:     Compares two solutions with character chromosomes and
                returns TRUE if, and only if, they are exactly
                identical.
  parameters:
  return:
  last updated: 14 Oct 2002
 **********************************************************************/

boolean ga_tabu_check_char(     population      *pop,
                            entity              *putative,
                            entity              *tabu)
{
    int         i;              /* Loop variable over chromosomes. */
    int         j;              /* Loop variable over alleles. */
    char        *a, *b;         /* Comparison char arrays. */

    /* Checks. */
    if ( !pop ) die("Null pointer to population structure passed.");
    if ( !putative || !tabu ) die("Null pointer to entity structure passed.");

    for (i=0; i<pop->num_chromosomes; i++)
    {
        a = (char*)(putative->chromosome[i]);
        b = (char*)(tabu->chromosome[i]);

        for (j=0; j<pop->len_chromosomes; j++)
            if (a[j] != b[j]) return FALSE;
    }

    return TRUE;
}


/**********************************************************************
  ga_tabu_check_boolean()
  synopsis:     Compares two solutions with boolean chromosomes and
                returns TRUE if, and only if, they are exactly
                identical.
  parameters:
  return:
  last updated: 14 Oct 2002
 **********************************************************************/

boolean ga_tabu_check_boolean(  population      *pop,
                                entity          *putative,
                                entity          *tabu)
{
    int         i;              /* Loop variable over chromosomes. */
    int         j;              /* Loop variable over alleles. */
    boolean     *a, *b;         /* Comparison boolean arrays. */

    /* Checks. */
    if ( !pop ) die("Null pointer to population structure passed.");
    if ( !putative || !tabu ) die("Null pointer to entity structure passed.");

    for (i=0; i<pop->num_chromosomes; i++)
    {
        a = (boolean*)(putative->chromosome[i]);
        b = (boolean*)(tabu->chromosome[i]);

        for (j=0; j<pop->len_chromosomes; j++)
            if (a[j] != b[j]) return FALSE;
    }

    return TRUE;
}


/**********************************************************************
  ga_tabu_check_double()
  synopsis:     Compares two solutions with double chromosomes and
                returns TRUE if, and only if, allele pair difference
                is less than TINY.
  parameters:
  return:
  last updated: 14 Oct 2002
 **********************************************************************/

boolean ga_tabu_check_double(   population      *pop,
                                entity          *putative,
                                entity          *tabu)
{
    int         i;              /* Loop variable over chromosomes. */
    int         j;              /* Loop variable over alleles. */
    double      *a, *b;         /* Comparison double arrays. */

    /* Checks. */
    if ( !pop ) die("Null pointer to population structure passed.");
    if ( !putative || !tabu ) die("Null pointer to entity structure passed.");

    for (i=0; i<pop->num_chromosomes; i++)
    {
        a = (double*)(putative->chromosome[i]);
        b = (double*)(tabu->chromosome[i]);

        for (j=0; j<pop->len_chromosomes; j++)
            if (a[j] < b[j]-TINY || a[j] > b[j]+TINY) return FALSE;
    }

    return TRUE;
}


/**********************************************************************
  ga_tabu_check_bitstring()
  synopsis:     Compares two solutions with bitstring chromosomes and
                returns TRUE if, and only if, all alleles are exactly
                the same.
  parameters:
  return:
  last updated: 14 Oct 2002
 **********************************************************************/

boolean ga_tabu_check_bitstring( population     *pop,
                                 entity         *putative,
                                 entity         *tabu)
{
    int         i;              /* Loop variable over chromosomes. */
    int         j;              /* Loop variable over alleles. */
    byte        *a, *b;         /* Comparison bitstrings. */

    /* Checks. */
    if ( !pop ) die("Null pointer to population structure passed.");
    if ( !putative || !tabu ) die("Null pointer to entity structure passed.");

    for (i=0; i<pop->num_chromosomes; i++)
    {
        a = (byte*)(putative->chromosome[i]);
        b = (byte*)(tabu->chromosome[i]);

        for (j=0; j<pop->len_chromosomes; j++)
            if (ga_bit_get( a, i ) != ga_bit_get( b, i )) return FALSE;
    }

    return TRUE;
}


/**********************************************************************
  gaul_check_tabu_list()
  synopsis:     Checks the tabu list verses the putative solutions and
                chooses an acceptable solution.  Returns -1 if all
                putative solutions are tabu.
  parameters:
  return:
  last updated: 14 Oct 2002
 **********************************************************************/

static int gaul_check_tabu_list(        population      *pop,
                                    entity              **putative,
                                    entity              **tabu)
{
    int         i;              /* Loop variable over putative solutions. */
    int         j;              /* Loop variable over tabu list. */
    boolean     is_tabu;        /* Whether current solution is tabu. */

    for (i=0; i<pop->tabu_params->search_count; i++)
    {
        is_tabu = FALSE;
        for (j=0; j<pop->tabu_params->list_length && tabu[j]!=NULL && is_tabu==FALSE; j++)
        {
            is_tabu = pop->tabu_params->tabu_accept(pop,putative[i],tabu[j]);
        }
        if (is_tabu==FALSE)
        {       /* This solution is not tabu. */
            return i;
        }
    }

    /* All solutions are tabu. */
    return -1;
}


/**********************************************************************
  ga_population_set_tabu_parameters()
  synopsis:     Sets the tabu-search parameters for a population.
  parameters:
  return:
  last updated: 09 Oct 2002
 **********************************************************************/

void ga_population_set_tabu_parameters( population              *pop,
                                        GAtabu_accept           tabu_accept,
                                        const int               list_length,
                                        const int               search_count)
{

    if ( !pop ) die("Null pointer to population structure passed.");
    if ( !tabu_accept ) die("Null pointer to GAtabu_accept callback passed.");

    plog( LOG_VERBOSE,
          "Population's tabu-search parameters: list_length = %d search_count = %d",
          list_length, search_count );

    if (pop->tabu_params == NULL)
        pop->tabu_params = (ga_tabu_t *)s_malloc(sizeof(ga_tabu_t));

    pop->tabu_params->tabu_accept = tabu_accept;
    pop->tabu_params->list_length = list_length;
    pop->tabu_params->search_count = search_count;

    return;
}


/**********************************************************************
  ga_tabu()
  synopsis:     Performs optimisation on the passed entity by using a
                simplistic tabu-search.  The local search and fitness
                evaluations are performed using the standard mutation
                and evaluation callback mechanisms, respectively.
                The passed entity will have its data overwritten.  The
                remainder of the population will be let untouched.
                Note that it is safe to pass a NULL initial structure,
                in which case a random starting structure wil be
                generated, however the final solution will not be
                available to the caller in any obvious way.
  parameters:
  return:
  last updated: 18 Feb 2005
 **********************************************************************/

int ga_tabu(    population              *pop,
                entity                  *initial,
                const int               max_iterations )
{
    int         iteration=0;            /* Current iteration number. */
    int         i, j;                   /* Index into putative solution array. */
    entity      *best;                  /* Current best solution. */
    entity      **putative;             /* Current working solutions. */
    entity      *tmp;                   /* Used to swap working solutions. */
    entity      **tabu_list;            /* Tabu list. */
    int         tabu_list_pos=0;        /* Index into the tabu list. */

    /* Checks. */
    if (!pop) die("NULL pointer to population structure passed.");
    if (!pop->evaluate) die("Population's evaluation callback is undefined.");
    if (!pop->mutate) die("Population's mutation callback is undefined.");
    if (!pop->rank) die("Population's ranking callback is undefined.");
    if (!pop->tabu_params) die("ga_population_set_tabu_params(), or similar, must be used prior to ga_tabu().");
    if (!pop->tabu_params->tabu_accept) die("Population's tabu acceptance callback is undefined.");

    /* Prepare working entities. */
    best = ga_get_free_entity(pop);     /* The best solution so far. */
    putative = (entity **)s_malloc(sizeof(entity *)*pop->tabu_params->search_count);

    for (i=0; i<pop->tabu_params->search_count; i++)
    {
        putative[i] = ga_get_free_entity(pop);    /* The 'working' solutions. */
    }

    /* Allocate and clear an array for the tabu list. */
    tabu_list = (entity **)s_malloc(sizeof(vpointer)*pop->tabu_params->list_length);

    for (i=0; i<pop->tabu_params->list_length; i++)
    {
        tabu_list[i] = NULL;
    }

    /* Do we need to generate a random starting solution? */
    if (!initial)
    {
        plog(LOG_VERBOSE, "Will perform tabu-search with random starting solution.");

        initial = ga_get_free_entity(pop);
        ga_entity_seed(pop, best);
    }
    else
    {
        plog(LOG_VERBOSE, "Will perform tabu-search with specified starting solution.");
        ga_entity_copy(pop, best, initial);
    }

    /*
 * Ensure that initial solution is scored.
 */
    if (best->fitness==GA_MIN_FITNESS) pop->evaluate(pop, best);

    plog( LOG_VERBOSE,
          "Prior to the first iteration, the current solution has fitness score of %f",
          best->fitness );

    /*
 * Do all the iterations:
 *
 * Stop when (a) max_iterations reached, or
 *           (b) "pop->iteration_hook" returns FALSE.
 */
    while ( (pop->iteration_hook?pop->iteration_hook(iteration, best):TRUE) &&
            iteration<max_iterations )
    {
        iteration++;

        /*
 * Generate and score new solutions.
 */
        for (i=0; i<pop->tabu_params->search_count; i++)
        {
            pop->mutate(pop, best, putative[i]);
            pop->evaluate(pop, putative[i]);
        }

        /*
 * Sort new solutions (putative[0] will have highest rank).
 * We assume that there are only a small(ish) number of
 * solutions and, therefore, a simple bubble sort is adequate.
 */
        for (i=1; i<pop->tabu_params->search_count; i++)
        {
            for (j=pop->tabu_params->search_count-1; j>=i; j--)
            {
                if ( pop->rank(pop, putative[j], pop, putative[j-1]) > 0 )
                {       /* Perform a swap. */
                    tmp = putative[j];
                    putative[j] = putative[j-1];
                    putative[j-1] = tmp;
                }
            }
        }

        /*
 * Save best solution if it is an improvement, otherwise
 * select the best non-tabu solution (if any).
 * If appropriate, update the tabu list.
 */
        if ( pop->rank(pop, putative[0], pop, best) > 0 )
        {
            tmp = best;
            best = putative[0];
            putative[0] = tmp;
            if (tabu_list[tabu_list_pos] == NULL)
            {
                tabu_list[tabu_list_pos] = ga_entity_clone(pop, best);
            }
            else
            {
                ga_entity_blank(pop, tabu_list[tabu_list_pos]);
                ga_entity_copy(pop, tabu_list[tabu_list_pos], best);
            }
            tabu_list_pos++;
            if (tabu_list_pos >= pop->tabu_params->list_length)
                tabu_list_pos=0;
        }
        else
        {
            if ( -1 < (j = gaul_check_tabu_list(pop, putative, tabu_list)) )
            {
                tmp = best;
                best = putative[j];
                putative[j] = tmp;
                if (tabu_list[tabu_list_pos] == NULL)
                {
                    tabu_list[tabu_list_pos] = ga_entity_clone(pop, best);
                }
                else
                {
                    ga_entity_blank(pop, tabu_list[tabu_list_pos]);
                    ga_entity_copy(pop, tabu_list[tabu_list_pos], best);
                }
                tabu_list_pos++;
                if (tabu_list_pos >= pop->tabu_params->list_length)
                    tabu_list_pos=0;
            }
        }

        /*
 * Save the current best solution in the initial entity, if this
 * is now the best found so far.
 */
        if ( pop->rank(pop, best, pop, initial) > 0 )
        {
            ga_entity_blank(pop, initial);
            ga_entity_copy(pop, initial, best);
        }

        /*
 * Use the iteration callback.
 */
        plog( LOG_VERBOSE,
              "After iteration %d, the current solution has fitness score of %f",
              iteration,
              best->fitness );

    }   /* Iteration loop. */

    /*
 * Cleanup.
 */
    ga_entity_dereference(pop, best);

    for (i=0; i<pop->tabu_params->search_count; i++)
    {
        ga_entity_dereference(pop, putative[i]);
    }

    for (i=0; i<pop->tabu_params->list_length; i++)
    {
        if (tabu_list[i] != NULL)
            ga_entity_dereference(pop, tabu_list[i]);
    }

    s_free(putative);
    s_free(tabu_list);

    return iteration;
}


