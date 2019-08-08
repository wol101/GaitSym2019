/**********************************************************************
  memory_chunks.c
 **********************************************************************

  memory_chunks - Efficient bulk memory allocation.
  Copyright ©2001-2005, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:	Efficient bulk memory allocation.

                This code is part of memory_util.c - its own
                integrated implementation of memory chunks.
                It may be used independantly if you feel very brave.

                Define MEMORY_CHUNKS_MIMIC to allow your favourite
                memory debugger to function properly.  However, note
                that by defining MEMORY_CHUNKS_MIMIC a number of memory
                leaks will occur if you rely on mem_chunk_clean(),
                mem_chunk_reset() or mem_chunk_free() to implicitly
                deallocate all memory atoms (which would normally be
                a valid thing to do).

                This is thread safe.

                For OpenMP code, USE_OPENMP must be defined and
                mem_chunk_init_openmp() must be called prior to any
                other function.

  To do:	Padding for array under/overflow checking.
                Observe contents of atoms in the FreeAtom list.

  Known bugs:	High padding may be offset from the real end of the data - so some
                overflows will be missed.
                Much of the tree handling code may now be optimised.

 **********************************************************************/

#include "gaul/memory_chunks.h"

/* MEMORY_ALIGN_SIZE should be set in gaul_config.h */

#define MEMORY_AREA_SIZE 4L

typedef void *Key_t;

typedef struct node_s
  {
  struct node_s *left;		/* Left subtree. */
  struct node_s *right;		/* Right subtree. */
  int		balance;	/* Height (left) - height (right). */
  Key_t		key;		/* The key for this node. */
  vpointer	data;		/* Data stored at this node. */
  } node_t;

typedef struct
  {
  node_t	*root;
  } tree_t;

typedef struct FreeAtom_t
  {
  struct FreeAtom_t *next;
  } FreeAtom;

typedef struct MemArea_t
  {
  struct MemArea_t *next;	/* The next memory area */
  struct MemArea_t *prev;	/* The previous memory area */
  size_t	index;		/* The current index into the "mem" array */
  size_t	free;		/* The number of free bytes in this memory area */
  unsigned int	used;		/* The number of atoms allocated from this area */
  unsigned char	*mem;		/* The mem array from which atoms get allocated. */
  } MemArea;

struct MemChunk_t
  {
  unsigned int	num_mem_areas;		/* The total number of memory areas */
  unsigned int	num_unused_areas;	/* The number of areas that may be deallocated */
  size_t	atom_size;		/* The size of an atom (used in mimic routines) */
  size_t	area_size;		/* The size of a memory area */
  MemArea	*mem_area;		/* The current memory area */
  MemArea	*mem_areas;		/* A list of all the memory areas owned by this chunk */
  MemArea	*free_mem_area;		/* Free area... which is about to be destroyed */
  FreeAtom	*free_atoms;		/* Free atoms list */
  tree_t	*mem_tree;		/* Tree of memory areas sorted by memory address */
  long		num_atoms_alloc;	/* The number of allocated atoms (only used in mimic routines) */
  };


/*
 * Private function prototypes.
 */
static node_t	*node_new(Key_t key, void *data);
static void	node_free(node_t *node);
static void	node_delete(node_t *node);
static node_t	*node_insert(node_t *node, Key_t key,
                                             void *data, boolean *inserted);
static node_t	*node_remove(node_t *node,
                                             Key_t key, void **removed_data);
static node_t	*node_balance(node_t *node);
static node_t	*node_remove_leftmost(node_t *node,
                                                     node_t **leftmost);
static node_t	*node_restore_left_balance(node_t *node,
                                                     int old_balance);
static node_t	*node_restore_right_balance(node_t *node,
                                                     int old_balance);
static int	node_height(node_t *node);
static node_t	*node_rotate_left(node_t *node);
static node_t	*node_rotate_right(node_t *node);

/*
 * Compilation constants.
 */
#define _NODE_BUFFER_NUM_INCR	16
#define _NODE_BUFFER_SIZE		1024

/*
 * Global variables.
 */
static node_t		*node_free_list = NULL;
static int		num_trees = 0;	/* Count number of trees in use. */
static int		buffer_num = -1;
static int		num_buffers = 0;
static int		num_used = _NODE_BUFFER_SIZE;
static node_t		**node_buffers = NULL;

/*
 * Note that a single coarse thread lock is used when a number of
 * less coarse locks might be better.
 */
THREAD_LOCK_DEFINE_STATIC(node_buffer_lock);
#if USE_OPENMP == 1
static boolean mem_chunk_openmp_initialised = FALSE;
#endif

/*
 * This function must be called before any other functions is OpenMP
 * code is to be used.  Can be safely called when OpenMP code is not
 * being used, and can be safely called more than once.
 */
void mem_chunk_init_openmp(void)
  {

#if USE_OPENMP == 1
  if (mem_chunk_openmp_initialised == FALSE)
    {
    omp_init_lock(&node_buffer_lock);
    mem_chunk_openmp_initialised = TRUE;
    }
#endif

  return;
  }



/*
 * Private functions.
 */

/*
 * Deallocate all memory associated with buffers.  Should
 * never be called outside of a "node_buffer_lock"
 * block.
 */
static void _destroy_buffers(void)
  {

  while (buffer_num >= 0)
    {
    s_free(node_buffers[buffer_num]);
    buffer_num--;
    }

  s_free(node_buffers);
  node_buffers = NULL;
  num_buffers = 0;
  num_used = _NODE_BUFFER_SIZE;

  node_free_list = NULL;

  return;
  }


static node_t *node_new(Key_t key, void *data)
  {
  node_t *node;

  THREAD_LOCK(node_buffer_lock);
/*
 * Find an unused node.  Look for unused node in current buffer, then
 * in the free node list, then allocate new buffer.
 */
  if (num_used < _NODE_BUFFER_SIZE)
    {
    node = &(node_buffers[buffer_num][num_used]);
    num_used++;
    }
  else
    {
    if (node_free_list)
      {
      node = node_free_list;
      node_free_list = node->right;
      }
    else
      {
      buffer_num++;

      if (buffer_num == num_buffers)
        {
        num_buffers += _NODE_BUFFER_NUM_INCR;
        node_buffers = (node_t **)s_realloc(node_buffers, sizeof(node_t *)*num_buffers);
        }

      node_buffers[buffer_num] = (node_t *)malloc(sizeof(node_t)*_NODE_BUFFER_SIZE);

      if (!node_buffers[buffer_num]) die("Unable to allocate memory.");

      node = node_buffers[buffer_num];
      num_used = 1;
      }
    }
  THREAD_UNLOCK(node_buffer_lock);

  node->balance = 0;
  node->left = NULL;
  node->right = NULL;
  node->key = key;
  node->data = data;

  return node;
  }


static void node_free(node_t *node)
  {
  THREAD_LOCK(node_buffer_lock);
  node->right = node_free_list;
  node_free_list = node;
  THREAD_UNLOCK(node_buffer_lock);

  return;
  }


static void node_delete(node_t *node)
  {
  if (node)
    {
    node_delete(node->right);
    node_delete(node->left);
    node_free(node);
    }

  return;
  }


/*
 * Systematically search tree with a search function which has the
 * same ordering as the tree.
 * This iterative version is much faster than the equivalent recursive version.
 */
static void *node_ordered_search(node_t *node, void *userdata)
  {

  while (node)
    {
    if (userdata < (void *) ((MemArea *)node->data)->mem)
      node=node->left;
    else if (userdata > (void *) &(((MemArea *)node->data)->mem[((MemArea *)node->data)->index]))
      node=node->right;
    else
      return node->data;
    }

  return NULL;
  }


static node_t *node_insert(node_t *node,
                    Key_t key, void *data, boolean *inserted)
  {
  int old_balance;

  if (!node)
    {
    *inserted = TRUE;
    return node_new(key, data);
    }

  if (key < node->key)
    {
    if (node->left)
      {
      old_balance = node->left->balance;
      node->left = node_insert(node->left, key, data, inserted);

      if ((old_balance != node->left->balance) && node->left->balance)
        node->balance--;
      }
    else
      {
      *inserted = TRUE;
      node->left = node_new(key, data);
      node->balance--;
      }
    }
  else if (key > node->key)
    {
    if (node->right)
      {
      old_balance = node->right->balance;
      node->right = node_insert(node->right, key, data, inserted);

      if ((old_balance != node->right->balance) && node->right->balance)
        node->balance++;
      }
    else
      {
      *inserted = TRUE;
      node->right = node_new(key, data);
      node->balance++;
      }
    }
  else
    {	/* key == node->key */
/*
    *inserted = FALSE;
 */
    printf("WARNING: -- Replaced node -- (Key clash?)\n");

    node->data = data;
    return node;
    }

  if (*inserted && (node->balance < -1 || node->balance > 1))
    node = node_balance(node);

  return node;
  }


static node_t *node_remove(node_t *node,
                            Key_t key, void **removed_data)
  {
  node_t	*new_root;
  int		old_balance;

  if (!node) return NULL;

  if (key < node->key)
    {
    if (node->left)
      {
      old_balance = node->left->balance;
      node->left = node_remove(node->left, key, removed_data);
      node = node_restore_left_balance(node, old_balance);
      }
    }
  else if (key > node->key)
    {
    if (node->right)
      {
      old_balance = node->right->balance;
      node->right = node_remove(node->right, key, removed_data);
      node = node_restore_right_balance(node, old_balance);
      }
    }
  else if (key == node->key)
    {
    node_t *removed_node;

    removed_node = node;

    if (!node->right)
      {
      node = node->left;
      }
    else
      {
      old_balance = node->right->balance;
      node->right = node_remove_leftmost (node->right, &new_root);
      new_root->left = node->left;
      new_root->right = node->right;
      new_root->balance = node->balance;
      node = node_restore_right_balance (new_root, old_balance);
      }

    *removed_data = removed_node->data;
    node_free(removed_node);
    }

  return node;
  }


static node_t *node_balance(node_t *node)
  {
  if (node->balance < -1)
    {
    if (node->left->balance > 0)
      node->left = node_rotate_left(node->left);
    node = node_rotate_right (node);
    }
  else if (node->balance > 1)
    {
    if (node->right->balance < 0)
      node->right = node_rotate_right(node->right);
    node = node_rotate_left (node);
    }

  return node;
  }


static node_t *node_remove_leftmost(node_t  *node,
                             node_t **leftmost)
  {
  int old_balance;

  if (!node->left)
    {
    *leftmost = node;
    return node->right;
    }

  old_balance = node->left->balance;
  node->left = node_remove_leftmost(node->left, leftmost);
  return node_restore_left_balance(node, old_balance);
  }


static node_t *node_restore_left_balance(node_t	*node,
                                  int		old_balance)
  {
  if ( (!node->left) || ((node->left->balance != old_balance) &&
           (node->left->balance == 0)) )
    {
    node->balance++;
    }

  if (node->balance > 1) return node_balance(node);

  return node;
  }


static node_t *node_restore_right_balance(node_t	*node,
                                   int		old_balance)
  {
  if ( (!node->right) || ((node->right->balance != old_balance) &&
           (node->right->balance == 0)) )
    {
    node->balance--;
    }

  if (node->balance < -1) return node_balance(node);

  return node;
  }


static int node_height(node_t *node)
  {
  int left_height;
  int right_height;

  if (!node) return 0;

  if (node->left)
    left_height = node_height(node->left);
  else
    left_height = 0;

  if (node->right)
    right_height = node_height(node->right);
  else
    right_height = 0;

  return MAX(left_height, right_height) + 1;
  }


static node_t *node_rotate_left(node_t *node)
  {
  node_t *right;
  int a_bal;
  int b_bal;

  right = node->right;

  node->right = right->left;
  right->left = node;

  a_bal = node->balance;
  b_bal = right->balance;

  if (b_bal <= 0)
    {
    if (a_bal >= 1) right->balance = b_bal - 1;
    else right->balance = a_bal + b_bal - 2;

    node->balance = a_bal - 1;
    }
  else
    {
    if (a_bal <= b_bal) right->balance = a_bal - 2;
    else right->balance = b_bal - 1;

    node->balance = a_bal - b_bal - 1;
    }

  return right;
  }


static node_t *node_rotate_right(node_t *node)
  {
  node_t *left;
  int a_bal;
  int b_bal;

  left = node->left;

  node->left = left->right;
  left->right = node;

  a_bal = node->balance;
  b_bal = left->balance;

  if (b_bal <= 0)
    {
    if (b_bal > a_bal) left->balance = b_bal + 1;
    else left->balance = a_bal + 2;

    node->balance = a_bal - b_bal + 1;
    }
  else
    {
    if (a_bal <= -1) left->balance = b_bal + 1;
    else left->balance = a_bal + b_bal + 2;

    node->balance = a_bal + 1;
    }

  return left;
  }


static tree_t *tree_new(void)
  {
  tree_t *tree;

  num_trees++;

  if (!(tree = (tree_t *)malloc(sizeof(tree_t))) )
    die("Unable to allocate memory.");

  tree->root = NULL;

  return tree;
  }


static void sdelete(tree_t *tree)
  {
  if (!tree) return;

  node_delete(tree->root);

  s_free(tree);

  num_trees--;

  THREAD_LOCK(node_buffer_lock);
  if (num_trees == 0)
    _destroy_buffers();
  THREAD_UNLOCK(node_buffer_lock);

  return;
  }


static boolean insert(tree_t *tree, void *data)
  {
  boolean	inserted=FALSE;

  if (!data) die("Internal error: Trying to insert NULL data.");
  if (!tree) die("Internal error: Trying to insert into NULL tree.");

  tree->root = node_insert(tree->root, (Key_t) ((MemArea *)data)->mem, data, &inserted);

  return inserted;
  }


static void *remove_data(tree_t *tree, void *data)
  {
  void *removed=NULL;

  if (!tree || !tree->root) return NULL;

  tree->root = node_remove(tree->root, (Key_t) ((MemArea *)data)->mem, &removed);

  return removed;
  }


static void *remove_key(tree_t *tree, Key_t key)
  {
  void *removed=NULL;

  if (!tree || !tree->root) return NULL;

  tree->root = node_remove(tree->root, key, &removed);

  return removed;
  }


static void *ordered_search(tree_t *tree, void *userdata)
  {
  if (!tree || !tree->root) return NULL;

  return node_ordered_search(tree->root, userdata);
  }


/*
 * Padding functions:
 */
#if MEMORY_PADDING==TRUE
static unsigned char *pad_values="abcdefghijklmnopqr";

#define BUMP_DOWN(X)	( (void *) (((unsigned char *)(X))-MEMORY_ALIGN_SIZE) )
#define BUMP_UP(X)	( (void *) (((unsigned char *)(X))+MEMORY_ALIGN_SIZE) )

static void set_pad_low(MemChunk *mem_chunk, void *mem)
  {
  memcpy(((unsigned char *)mem),pad_values,MEMORY_ALIGN_SIZE);

  return;
  }


static void set_pad_high(MemChunk *mem_chunk, void *mem)
  {
  memcpy(((unsigned char *)mem)+mem_chunk->atom_size-MEMORY_ALIGN_SIZE,
            pad_values,MEMORY_ALIGN_SIZE);

  return;
  }


static int check_pad_low(MemChunk *mem_chunk, void *mem)
  {
  return memcmp(((unsigned char *)mem),pad_values,MEMORY_ALIGN_SIZE);
  }


static int check_pad_high(MemChunk *mem_chunk, void *mem)
  {
  return memcmp(((unsigned char *)mem)+mem_chunk->atom_size-MEMORY_ALIGN_SIZE,
                   pad_values,MEMORY_ALIGN_SIZE);
  }

#endif	/* MEMORY_PADDING==TRUE */


boolean mem_chunk_has_freeable_atoms_real(MemChunk *mem_chunk)
  {

  return mem_chunk->mem_tree?TRUE:FALSE;
  }


static MemChunk *_mem_chunk_new(size_t atom_size, unsigned int num_atoms)
  {
  MemChunk	*mem_chunk;

/*
 * Ensure that we don't misalign allocated memory for the user.
 * This also ensures that the minimum atom_size is okay for the
 * FreeAtom list.
 */
  if (atom_size % MEMORY_ALIGN_SIZE > 0)
    {
    atom_size += MEMORY_ALIGN_SIZE - (atom_size % MEMORY_ALIGN_SIZE);
    printf("DEBUG: modified MemChunk atom size.\n");
    }
#if MEMORY_PADDING==TRUE
  atom_size += 2*MEMORY_ALIGN_SIZE;
#endif

  if ( !(mem_chunk = (MemChunk *) malloc(sizeof(MemChunk))) )
    die("Unable to allocate memory.");

  mem_chunk->num_mem_areas = 0;
  mem_chunk->num_unused_areas = 0;
  mem_chunk->mem_area = NULL;
  mem_chunk->free_mem_area = NULL;
  mem_chunk->free_atoms = NULL;
  mem_chunk->mem_areas = NULL;
  mem_chunk->atom_size = atom_size;
  mem_chunk->area_size = atom_size*num_atoms;
  mem_chunk->mem_tree = NULL;

  return mem_chunk;
  }


/*
 * Return TRUE is the memory chunk is empty.
 */
boolean mem_chunk_isempty_real(MemChunk *mem_chunk)
  {

  if (!mem_chunk) die("Null pointer to mem_chunk passed.");

  return mem_chunk->num_mem_areas==mem_chunk->num_unused_areas;
  }


/*
 * Constricted memory chunks: Atoms may not be individually released.
 */
MemChunk *mem_chunk_new_unfreeable_real(size_t atom_size, unsigned int num_atoms)
  {
  MemChunk	*mem_chunk;

  if (atom_size<1) die("Passed atom size is < 1 byte.");
  if (num_atoms<1) die("Passed number of atoms is < 1.");

  mem_chunk = _mem_chunk_new(atom_size, num_atoms);

  return mem_chunk;
  }


MemChunk *mem_chunk_new_real(size_t atom_size, unsigned int num_atoms)
  {
  MemChunk	*mem_chunk;

  if (atom_size<1) die("Passed atom size is < 1 byte.");
  if (num_atoms<1) die("Passed number of atoms is < 1.");

  mem_chunk = _mem_chunk_new(atom_size, num_atoms);
  mem_chunk->mem_tree = tree_new();

  return mem_chunk;
  }


void mem_chunk_destroy_real(MemChunk *mem_chunk)
  {
  MemArea *mem_areas;
  MemArea *temp_area;

  if (!mem_chunk) die("Null pointer to mem_chunk passed.");

  mem_areas = mem_chunk->mem_areas;
  while (mem_areas)
    {
    temp_area = mem_areas;
    mem_areas = mem_areas->next;
    free(temp_area);
    }

  sdelete(mem_chunk->mem_tree);

  free(mem_chunk);

  return;
  }


void *mem_chunk_alloc_real(MemChunk *mem_chunk)
  {
  MemArea *temp_area;
  void *mem;

  if (!mem_chunk) die("Null pointer to mem_chunk passed.");

  while (mem_chunk->free_atoms)
    {
      /* Get the first piece of memory on the "free_atoms" list.
       * We can go ahead and destroy the list node we used to keep
       *  track of it with and to update the "free_atoms" list to
       *  point to its next element.
       */
      mem = mem_chunk->free_atoms;
      mem_chunk->free_atoms = mem_chunk->free_atoms->next;

      /* Determine which area this piece of memory is allocated from */
      temp_area = (MemArea *)ordered_search(mem_chunk->mem_tree, mem);

      /* If the area is unused, then it may be destroyed.
       * We check to see if all of the segments on the free list that
       *  reference this area have been removed. This occurs when
       *  the ammount of free memory is less than the allocatable size.
       * If the chunk should be freed, then we place it in the "free_mem_area".
       * This is so we make sure not to free the memory area here and then
       *  allocate it again a few lines down.
       * If we don't allocate a chunk a few lines down then the "free_mem_area"
       *  will be freed.
       * If there is already a "free_mem_area" then we'll just free this memory area.
       */
      if (temp_area->used==0)
        {
          /* Update the "free" memory available in that area */
          temp_area->free += mem_chunk->atom_size;

          if (temp_area->free == mem_chunk->area_size)
            {
              if (temp_area == mem_chunk->mem_area)
                mem_chunk->mem_area = NULL;

              if (mem_chunk->free_mem_area)
                {
                mem_chunk->num_mem_areas--;

                  if (temp_area->next)
                    temp_area->next->prev = temp_area->prev;
                  if (temp_area->prev)
                    temp_area->prev->next = temp_area->next;
                  if (temp_area == mem_chunk->mem_areas)
                    mem_chunk->mem_areas = mem_chunk->mem_areas->next;

                  if (mem_chunk->mem_tree)
                    {
                    if (!remove_data(mem_chunk->mem_tree, temp_area)) die("Unable to find temp_area.");
                    }

                  free (temp_area);
                }
              else
                mem_chunk->free_mem_area = temp_area;

              mem_chunk->num_unused_areas--;
            }
        }
      else
        {
          /* Update the number of allocated atoms count.
           */
          temp_area->used++;

          /* The area is still in use...return the memory
           */
#if MEMORY_PADDING==TRUE
  set_pad_low(mem_chunk, mem);
  set_pad_high(mem_chunk, mem);
/*
  if (check_pad_low(mem_chunk, mem)!=0) die("LOW MEMORY_PADDING ALREADY CORRUPT!");
  if (check_pad_high(mem_chunk, mem)!=0) die("HIGH MEMORY_PADDING ALREADY CORRUPT!");
*/
  mem = BUMP_UP(mem);
#endif

        return mem;
        }
    }

  /* If there isn't a current memory area or the current memory area is out of
   * space then allocate a new memory area. We'll first check and see if we can
   * use the "free_mem_area".  Otherwise we'll just malloc the memory area.
   */
  if ((!mem_chunk->mem_area) ||
      ((mem_chunk->mem_area->index + mem_chunk->atom_size) > mem_chunk->area_size))
    {
      if (mem_chunk->free_mem_area)
        {
          mem_chunk->mem_area = mem_chunk->free_mem_area;
          mem_chunk->free_mem_area = NULL;
        }
      else
        {
          mem_chunk->mem_area = (MemArea*) malloc(sizeof(MemArea)+
                                                  MEMORY_ALIGN_SIZE-(sizeof(MemArea)%MEMORY_ALIGN_SIZE)+
                                                  mem_chunk->area_size);
          mem_chunk->mem_area->mem = ((unsigned char*) (mem_chunk->mem_area)+
                                     sizeof(MemArea)+
                                     MEMORY_ALIGN_SIZE-
                                     (sizeof(MemArea)%MEMORY_ALIGN_SIZE));

          if (!mem_chunk->mem_area) die("Unable to allocate memory.");

          mem_chunk->num_mem_areas++;
          mem_chunk->mem_area->next = mem_chunk->mem_areas;
          mem_chunk->mem_area->prev = NULL;

          if (mem_chunk->mem_areas)
            mem_chunk->mem_areas->prev = mem_chunk->mem_area;
          mem_chunk->mem_areas = mem_chunk->mem_area;

          if (mem_chunk->mem_tree)
            insert(mem_chunk->mem_tree, mem_chunk->mem_area);
        }

      mem_chunk->mem_area->index = 0;
      mem_chunk->mem_area->free = mem_chunk->area_size;
      mem_chunk->mem_area->used = 0;
    }

/*
 * Get the memory and modify the state variables appropriately.
 */
  mem = (vpointer) &(mem_chunk->mem_area->mem[mem_chunk->mem_area->index]);
  mem_chunk->mem_area->index += mem_chunk->atom_size;
  mem_chunk->mem_area->free -= mem_chunk->atom_size;
  mem_chunk->mem_area->used++;

#if MEMORY_PADDING==TRUE
  set_pad_low(mem_chunk, mem);
  set_pad_high(mem_chunk, mem);
/*
  if (check_pad_low(mem_chunk, mem)!=0) die("LOW MEMORY_PADDING ALREADY CORRUPT!");
  if (check_pad_high(mem_chunk, mem)!=0) die("HIGH MEMORY_PADDING ALREADY CORRUPT!");
*/
  mem = BUMP_UP(mem);
#endif

  return mem;
  }


void mem_chunk_free_real(MemChunk *mem_chunk, void *mem)
  {
  MemArea *temp_area;
  FreeAtom *free_atom;

  if (!mem_chunk) die("Null pointer to mem_chunk passed.");
  if (!mem_chunk->mem_tree) die("MemChunk passed has no freeable atoms.");
  if (!mem) die("NULL pointer passed.");

#if MEMORY_PADDING==TRUE
  mem = BUMP_DOWN(mem);
  if (check_pad_low(mem_chunk, mem)!=0)
    dief("LOW MEMORY_PADDING CORRUPT! (%*s)", MEMORY_ALIGN_SIZE, (unsigned char *)mem);
  if (check_pad_high(mem_chunk, mem)!=0)
    dief("HIGH MEMORY_PADDING CORRUPT!(%*s)", MEMORY_ALIGN_SIZE, (unsigned char *)mem);
#endif

/*
 * Place the memory on the "free_atoms" list.
 */
  free_atom = (FreeAtom*) mem;
  free_atom->next = mem_chunk->free_atoms;
  mem_chunk->free_atoms = free_atom;

  if (!(temp_area = (MemArea *)ordered_search(mem_chunk->mem_tree, mem)) )
    die("Unable to find temp_area.");

  temp_area->used--;

  if (temp_area->used == 0)
    {
    mem_chunk->num_unused_areas++;
    }

  return;
  }


/* This doesn't free the free_area if there is one */
void mem_chunk_clean_real(MemChunk *mem_chunk)
  {
  MemArea *mem_area;
  FreeAtom *prev_free_atom;
  FreeAtom *temp_free_atom;
  void *mem;

  if (!mem_chunk) die("Null pointer to mem_chunk passed.");
  if (!mem_chunk->mem_tree) die("MemChunk passed has no freeable atoms.");

  if (mem_chunk->mem_tree)
    {
    prev_free_atom = NULL;
    temp_free_atom = mem_chunk->free_atoms;

    while (temp_free_atom)
      {
      mem = (vpointer) temp_free_atom;

      if (!(mem_area = (MemArea *)ordered_search(mem_chunk->mem_tree, mem)) ) die("mem_area not found.");

/*
 * If this memory area is unused, delete the area, list node and decrement the free mem.
 */
      if (mem_area->used==0)
        {
        if (prev_free_atom)
          prev_free_atom->next = temp_free_atom->next;
        else
          mem_chunk->free_atoms = temp_free_atom->next;
        temp_free_atom = temp_free_atom->next;

        mem_area->free += mem_chunk->atom_size;
        if (mem_area->free == mem_chunk->area_size)
          {
          mem_chunk->num_mem_areas--;
          mem_chunk->num_unused_areas--;

          if (mem_area->next)
          mem_area->next->prev = mem_area->prev;
          if (mem_area->prev)
            mem_area->prev->next = mem_area->next;
          if (mem_area == mem_chunk->mem_areas)
            mem_chunk->mem_areas = mem_chunk->mem_areas->next;
          if (mem_area == mem_chunk->mem_area)
            mem_chunk->mem_area = NULL;

          if (mem_chunk->mem_tree)
            {
            if (!remove_key(mem_chunk->mem_tree, (Key_t) mem_area))
              die("mem_area not found.");
            }

          free(mem_area);
          }
        }
      else
        {
        prev_free_atom = temp_free_atom;
        temp_free_atom = temp_free_atom->next;
        }
      }
    }

  return;
  }


void mem_chunk_reset_real(MemChunk *mem_chunk)
  {
  MemArea *mem_areas;
  MemArea *temp_area;

  if (!mem_chunk) die("Null pointer to mem_chunk passed.");

  mem_areas = mem_chunk->mem_areas;
  mem_chunk->num_mem_areas = 0;
  mem_chunk->mem_areas = NULL;
  mem_chunk->mem_area = NULL;

  while (mem_areas)
    {
    temp_area = mem_areas;
    mem_areas = mem_areas->next;
    free(temp_area);
    }

  mem_chunk->free_atoms = NULL;

  if (mem_chunk->mem_tree)
    {
    sdelete(mem_chunk->mem_tree);
    mem_chunk->mem_tree = tree_new();
    }

  return;
  }


#if MEMORY_PADDING==TRUE
static int memarea_check_bounds(MemChunk *mem_chunk, MemArea *mem_area)
  {
  int		count = 0;
  unsigned char	*mem;
  int		index = 0;

  while (index < mem_area->index)
    {
    mem = (unsigned char*) &mem_area->mem[index];
    if (check_pad_low(mem_chunk, mem)!=0) count++;
    if (check_pad_high(mem_chunk, mem)!=0) count++;
    index += mem_chunk->atom_size;
    }

  return count;
  }


boolean mem_chunk_check_all_bounds_real(MemChunk *mem_chunk)
  {
  MemArea	*mem_area;
  int		badcount=0;

  if (!mem_chunk) die("Null pointer to mem_chunk passed.");

  mem_area = mem_chunk->mem_areas;

  while (mem_area)
    {
    if (mem_chunk->mem_area->used>0)
      {
      badcount += memarea_check_bounds(mem_chunk, mem_area);
      mem_area = mem_area->next;
      }
    }

  printf("%d pads corrupt or free.\n", badcount);

  return badcount>0;
  }


boolean mem_chunk_check_bounds_real(MemChunk *mem_chunk, void *mem)
  {
  mem = BUMP_DOWN(mem);
  if (check_pad_low(mem_chunk, mem)!=0)
    dief("Low padding corrupt! (%*s)", MEMORY_ALIGN_SIZE, (unsigned char *)mem);
  if (check_pad_high(mem_chunk, mem)!=0)
    dief("High padding corrupt!(%*s)", MEMORY_ALIGN_SIZE, (unsigned char *)mem);

  return TRUE;
  }
#else


boolean mem_chunk_check_all_bounds_real(MemChunk *mem_chunk)
  {
  return TRUE;
  }


boolean mem_chunk_check_bounds_real(MemChunk *mem_chunk, void *mem)
  {
  return TRUE;
  }
#endif


boolean mem_chunk_test_real(void)
  {
  unsigned char	*tmem[10000];
  MemChunk	*tmem_chunk=NULL;
  size_t	atomsize=40;
  int		i, j;

  printf("checking mem chunks...\n");

  tmem_chunk = mem_chunk_new(atomsize, 100);

  printf("alloc*1000...\n");
  for (i = 0; i < 1000; i++)
    {
    tmem[i] = (unsigned char *)mem_chunk_alloc(tmem_chunk);

    *tmem[i] = i%254;
    }

  for (i = 0; i < 1000; i++)
    {
    mem_chunk_check_bounds_real(tmem_chunk, tmem[i]);
    }

  printf("free*500...\n");
  for (i = 0; i < 500; i++)
    {
    mem_chunk_free(tmem_chunk, tmem[i]);
    }

  for (i = 500; i < 1000; i++)
    {
    mem_chunk_check_bounds_real(tmem_chunk, tmem[i]);
    }

  printf("alloc*500...\n");
  for (i = 0; i < 500; i++)
    {
    tmem[i] = (unsigned char *)mem_chunk_alloc(tmem_chunk);

    *tmem[i] = i%254;
    }

  for (i = 0; i < 1000; i++)
    {
    mem_chunk_check_bounds_real(tmem_chunk, tmem[i]);
    }

  printf("free*1000...\n");

  for (i = 0; i < 1000; i++)
    {
    if (*tmem[i] != i%254) die("Uh oh.");

    for (j = i; j<1000; j++)
      mem_chunk_check_bounds_real(tmem_chunk, tmem[j]);

    mem_chunk_free(tmem_chunk, tmem[i]);
    }

  printf("ok.\n");

  return TRUE;
  }


void mem_chunk_diagnostics_real(void)
  {
  printf("=== mem_chunk diagnostics ====================================\n");
  printf("Version:                   %s\n", GA_VERSION_STRING);
  printf("Build date:                %s\n", GA_BUILD_DATE_STRING);
  printf("Compilation machine characteristics:\n%s\n", GA_UNAME_STRING);

  printf("--------------------------------------------------------------\n");
  printf("MEMORY_PADDING:    %s\n", MEMORY_PADDING?"TRUE":"FALSE");
  printf("MEMORY_ALIGN_SIZE  %zd\n", MEMORY_ALIGN_SIZE);
  printf("MEMORY_AREA_SIZE   %ld\n", MEMORY_AREA_SIZE);

  printf("--------------------------------------------------------------\n");
  printf("structure          sizeof\n");
  printf("FreeAtom           %lu\n", (unsigned long) sizeof(FreeAtom));
  printf("MemArea            %lu\n", (unsigned long) sizeof(MemArea));
  printf("MemChunk           %lu\n", (unsigned long) sizeof(MemChunk));
  printf("==============================================================\n");

  return;
  }


/*
 * The following functions mimic the Memory chunk API, but are just
 * wrappers around the system malloc.
 */

/*
 * Return TRUE is the memory chunk is empty.
 */
boolean mem_chunk_isempty_mimic(MemChunk *mem_chunk)
  {

  if (!mem_chunk) die("Null pointer to mem_chunk passed.");

  return mem_chunk->num_atoms_alloc==0;
  }


/*
 * Create new memory chunk.
 */

MemChunk *mem_chunk_new_mimic(size_t atom_size, unsigned int num_atoms)
  {
  MemChunk	*mem_chunk;

  mem_chunk = (MemChunk *)malloc(sizeof(MemChunk));

  if (!mem_chunk) die("Unable to allocate memory.");

  mem_chunk->atom_size = atom_size;
  mem_chunk->num_atoms_alloc = 0;

  return mem_chunk;
  }


MemChunk *mem_chunk_new_unfreeable_mimic(size_t atom_size, unsigned int num_atoms)
  {
  return mem_chunk_new(atom_size, num_atoms);
  }


boolean mem_chunk_has_freeable_atoms_mimic(MemChunk *mem_chunk)
  {
  return TRUE;
  }


void mem_chunk_destroy_mimic(MemChunk *mem_chunk)
  {

  if (!mem_chunk) die("Null pointer to mem_chunk passed.");

  free(mem_chunk);

  return;
  }


void *mem_chunk_alloc_mimic(MemChunk *mem_chunk)
  {
  void	*mem;

  if (!mem_chunk) die("Null pointer to mem_chunk passed.");

  mem_chunk->num_atoms_alloc++;

  mem = malloc(mem_chunk->atom_size);

  if (!mem) die("Unable to allocate memory.");

  return mem;
  }


void mem_chunk_free_mimic(MemChunk *mem_chunk, void *mem)
  {

  if (!mem_chunk) die("Null pointer to mem_chunk passed.");
  if (!mem) die("Null pointer to memory passed.");

  free(mem);

  mem_chunk->num_atoms_alloc--;

  return;
  }


void mem_chunk_clean_mimic(MemChunk *mem_chunk)
  {

  if (!mem_chunk) die("Null pointer to mem_chunk passed.");

  mem_chunk->num_atoms_alloc=0;

  return;
  }


void mem_chunk_reset_mimic(MemChunk *mem_chunk)
  {

  if (!mem_chunk) die("Null pointer to mem_chunk passed.");

  mem_chunk->num_atoms_alloc=0;

  return;
  }


boolean mem_chunk_check_bounds_mimic(MemChunk *mem_chunk, void *mem)
  {
  return TRUE;
  }


boolean mem_chunk_test_mimic(void)
  {
  return TRUE;
  }


void mem_chunk_diagnostics_mimic(void)
  {
  printf("=== mem_chunk diagnostics *** MIMIC *** ======================\n");
  printf("Version:                   %s\n", GA_VERSION_STRING);
  printf("Build date:                %s\n", GA_BUILD_DATE_STRING);
  printf("Compilation machine characteristics:\n%s\n", GA_UNAME_STRING);

  printf("--------------------------------------------------------------\n");
  printf("structure          sizeof\n");
  printf("MemChunk           %lu\n", (unsigned long) sizeof(MemChunk));
  printf("==============================================================\n");

  return;
  }

