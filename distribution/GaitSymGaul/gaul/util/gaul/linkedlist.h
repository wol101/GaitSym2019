/**********************************************************************
  linkedlist.h
 **********************************************************************

  linkedlist - Linked list implementation.
  Copyright ©2000-2003, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:	Header file for linked list implementation.

 **********************************************************************/

#ifndef LINKEDLIST_H_INCLUDED
#define LINKEDLIST_H_INCLUDED

#include "gaul/gaul_util.h"

#include "gaul/memory_util.h"

/*
 * Type definitions.
 */

typedef int	(*LLCompareFunc)(constvpointer data1, constvpointer data2);
typedef boolean	(*LLForeachFunc)(vpointer data, vpointer userdata);
typedef void	(*LLDestructorFunc)(vpointer data);

typedef struct DLList_t
  {
  struct DLList_t *next;	/* Next element. */
  struct DLList_t *prev;	/* Prev element. */
  vpointer data;		/* Data stored in this element. */
  } DLList;

typedef struct SLList_t
  {
  struct SLList_t *next;	/* Next element. */
  vpointer data;		/* Data stored in this element. */
  } SLList;


/*
 * Function prototypes.
 */

FUNCPROTO void	linkedlist_init_openmp(void);
FUNCPROTO SLList	*slink_new(void);
FUNCPROTO void	slink_free_all(SLList *list);
FUNCPROTO void	slink_free(SLList *list);
FUNCPROTO SLList	*slink_append(SLList *list, vpointer data);
FUNCPROTO SLList	*slink_prepend(SLList *list, vpointer data);
FUNCPROTO SLList	*slink_insert_next(SLList *list, vpointer data);
FUNCPROTO SLList	*slink_insert_index(SLList *list, vpointer data, int index);
FUNCPROTO SLList	*slink_delete_data(SLList *list, vpointer data);
FUNCPROTO SLList	*slink_delete_all_data(SLList *list, vpointer data);
FUNCPROTO SLList	*slink_delete_link(SLList *list, SLList *link);
FUNCPROTO SLList	*slink_clone(SLList *list);
FUNCPROTO SLList	*slink_reverse(SLList *list);
FUNCPROTO SLList	*slink_nth(SLList *list, const int index);
FUNCPROTO vpointer	slink_nth_data(SLList *list, const int index);
FUNCPROTO SLList	*slink_find(SLList *list, vpointer data);
FUNCPROTO SLList	*slink_find_custom(SLList *list, vpointer data, LLCompareFunc func);
FUNCPROTO int	slink_index_link(SLList *list, SLList *link);
FUNCPROTO int	slink_index_data(SLList *list, vpointer data);
FUNCPROTO SLList	*slink_last(SLList *list);
FUNCPROTO int	slink_size(SLList *list);
FUNCPROTO boolean	slink_foreach(SLList *list, LLForeachFunc func, vpointer userdata);
FUNCPROTO SLList	*slink_sort_merge (SLList      *l1, 
		     SLList      *l2,
		     LLCompareFunc compare_func);
FUNCPROTO SLList	*slink_sort(SLList       *list,
	      LLCompareFunc compare_func);
FUNCPROTO DLList	*dlink_new(void);
FUNCPROTO void	dlink_free_all(DLList *list);
FUNCPROTO void	dlink_free(DLList *list);
FUNCPROTO DLList	*dlink_append(DLList *list, vpointer data);
FUNCPROTO DLList	*dlink_prepend(DLList *list, vpointer data);
FUNCPROTO DLList	*dlink_insert_next(DLList *list, vpointer data);
FUNCPROTO DLList	*dlink_insert_prev(DLList *list, vpointer data);
FUNCPROTO DLList	*dlink_insert_index(DLList	*list,
	       vpointer	 data,
	       int	 index);
FUNCPROTO DLList	*dlink_delete_all_data(DLList *list, vpointer data);
FUNCPROTO DLList	*dlink_delete_data(DLList *list, vpointer data);
FUNCPROTO DLList	*dlink_delete_link(DLList *list, DLList *link);
FUNCPROTO DLList	*dlink_clone(DLList *list);
FUNCPROTO DLList	*dlink_reverse(DLList *list);
FUNCPROTO DLList	*dlink_nth(DLList *list, const int index);
FUNCPROTO DLList	*dlink_pth(DLList *list, const int index);
FUNCPROTO vpointer	dlink_nth_data(DLList *list, const int index);
FUNCPROTO vpointer	dlink_pth_data(DLList *list, const int index);
FUNCPROTO DLList	*dlink_find(DLList *list, vpointer data);
FUNCPROTO DLList	*dlink_find_custom(DLList *list, vpointer data, LLCompareFunc func);
FUNCPROTO int	dlink_index_link(DLList *list, DLList *link);
FUNCPROTO int	dlink_index_data(DLList *list, vpointer data);
FUNCPROTO DLList	*dlink_last(DLList *list);
FUNCPROTO DLList	*dlink_first(DLList *list);
FUNCPROTO int	dlink_size(DLList *list);
FUNCPROTO boolean	dlink_foreach(DLList *list, LLForeachFunc func, vpointer userdata);
FUNCPROTO boolean	dlink_foreach_reverse(DLList *list,
                       LLForeachFunc func, vpointer userdata);
FUNCPROTO DLList	*dlink_sort_merge(DLList       *l1, 
		   DLList       *l2,
		   LLCompareFunc compare_func);
FUNCPROTO DLList	*dlink_sort(DLList       *list,
	     LLCompareFunc compare_func);
FUNCPROTO void linkedlist_diagnostics(void);
#ifndef LINKEDLIST_COMPILE_MAIN
FUNCPROTO boolean linkedlist_test(void);
#endif

#define slink_insert_prev(X,Y)	slink_prepend((X), (Y));
#define slink_next(X)		((X)?(((SLList *)(X))->next):NULL)
#define dlink_next(X)		((X)?(((DLList *)(X))->next):NULL)
#define dlink_prev(X)		((X)?(((DLList *)(X))->prev):NULL)

#define slink_data(X)		((X)?(((SLList *)(X))->data):NULL)
#define dlink_data(X)		((X)?(((DLList *)(X))->data):NULL)

/*
 * glib list emulation stuff.
 *
 * These macro redirections will be used if LINKEDLIST_EMULATE_GLIST is defined.
 */
#ifdef LINKEDLIST_EMULATE_GLIST

#define GSList	SLList
#define GList	DLList

#define g_list_append(X, Y)		dlink_append((X), (Y))
#define g_list_reverse(X)		dlink_reverse((X))
#define g_list_nth(X, Y)		dlink_nth((X), (Y))
#define g_list_nth_data(X, Y)		dlink_nth_data((X), (Y))
#define g_list_position(X, Y)		dlink_index_link((X), (Y))
#define g_list_free(X)			dlink_free_all((X))
#define g_list_insert_sorted(X, Y, Z)	dlink_insert_sorted((X), (Y), (Z))
#define g_list_foreach(X, Y, Z)		dlink_foreach((X), (Y), (Z))
#define g_list_prepend(X, Y)		dlink_prepend((X), (Y))
#define g_list_sort(X, Y)		dlink_sort((X), (Y))
#define g_list_length(X)		dlink_size((X))
#define g_list_next(X)			((X)?(((DLList *)(X))->next):NULL)
#define g_list_prev(X)			((X)?(((DLList *)(X))->prev):NULL)

#define g_slist_append(X, Y)		slink_append((X), (Y))
#define g_slist_reverse(X)		slink_reverse((X))
#define g_slist_nth(X, Y)		slink_nth((X), (Y))
#define g_slist_nth_data(X, Y)		slink_nth_data((X), (Y))
#define g_slist_position(X, Y)		slink_index_link((X), (Y))
#define g_slist_free(X)			slink_free_all((X))
#define g_slist_insert_sorted(X, Y, Z)	slink_insert_sorted((X), (Y), (Z))
#define g_slist_foreach(X, Y, Z)	slink_foreach((X), (Y), (Z))
#define g_slist_prepend(X, Y)		slink_prepend((X), (Y))
#define g_slist_sort(X, Y)		slink_sort((X), (Y))
#define g_slist_length(X)		slink_size((X))
#define g_slist_next(X)			((X)?(((SLList *)(X))->next):NULL)

#endif

#endif /* LINKEDLIST_H_INCLUDED */

