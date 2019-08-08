/**********************************************************************
  avltree.h
 **********************************************************************

  avltree - AVL Tree implementation.
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

  Synopsis:	Public header for avltree.c

 **********************************************************************/

#ifndef AVLTREE_H_INCLUDED
#define AVLTREE_H_INCLUDED

#include "gaul/gaul_util.h"

#include <stdio.h>
#include <stdlib.h>

#include "gaul/compatibility.h"
#include "gaul/memory_util.h"

/*
 * Define datatype for AVL keys.
 */
#ifdef AVLTREE_KEY_TYPE
typedef AVLTREE_KEY_TYPE AVLKey;
#else
typedef unsigned long AVLKey;
#endif

typedef AVLKey	(*AVLKeyFunc)(constvpointer data);
typedef boolean	(*AVLTraverseFunc)(AVLKey key, vpointer data, vpointer userdata);
typedef boolean	(*AVLMatchFunc)(constvpointer data, vpointer userdata);
typedef int	(*AVLSearchFunc)(constvpointer data, vpointer userdata);
typedef void	(*AVLDestructorFunc)(vpointer data);

typedef struct AVLTree_t
  {
  struct AVLNode_t	*root;  /* opaque from hereonin. */
  AVLKeyFunc		key_generate_func;
  } AVLTree;

/*
 * Prototypes.
 */

FUNCPROTO void		avltree_init_openmp(void);
FUNCPROTO AVLTree		*avltree_new(AVLKeyFunc key_generate_func);
FUNCPROTO void		avltree_delete(AVLTree *tree);
FUNCPROTO void		avltree_destroy(AVLTree *tree, AVLDestructorFunc free_func);
FUNCPROTO boolean		avltree_insert(AVLTree *tree, vpointer data);
FUNCPROTO vpointer	avltree_remove(AVLTree *tree, vpointer data);
FUNCPROTO vpointer	avltree_remove_key(AVLTree *tree, AVLKey key);
FUNCPROTO vpointer	avltree_lookup(AVLTree *tree, vpointer data);
FUNCPROTO vpointer	avltree_lookup_lowest(AVLTree *tree);
FUNCPROTO vpointer	avltree_lookup_highest(AVLTree *tree);
FUNCPROTO vpointer	avltree_lookup_key(AVLTree *tree, AVLKey key);
FUNCPROTO vpointer	avltree_ordered_search(AVLTree *tree,
                         AVLSearchFunc search_func, vpointer userdata);
FUNCPROTO vpointer	avltree_search(AVLTree *tree,
                         AVLMatchFunc search_func, vpointer userdata);
FUNCPROTO void		avltree_traverse(AVLTree *tree,
			 AVLTraverseFunc traverse_func, vpointer userdata);
FUNCPROTO int		avltree_height(AVLTree *tree);
FUNCPROTO int		avltree_num_nodes(AVLTree *tree);
FUNCPROTO void		avltree_diagnostics(void);

#ifndef AVLTREE_COMPILE_MAIN
FUNCPROTO boolean		avltree_test(void);
#endif

#endif /* AVLTREE_H_INCLUDED */
