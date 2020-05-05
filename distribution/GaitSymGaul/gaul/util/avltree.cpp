/**********************************************************************
  avltree.c
 **********************************************************************

  avltree - AVL tree implementation.
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

  Synopsis:	 AVL trees.
                (Adel'son-Velskii and Landis Tree, or height-balanced
                1-tree (!))

  References:	Just about any datastructure text book.
                I have stubbs & webre (1985, Wadsworth Inc.)

                Much of this code comes from the GLIB gtree.c source.

                Differences to the Balanced Binary Tree code in GLIB:
                  - degenerate keys are stored (I needed this - BUT NOT YET IMPLEMENTED)
                  - opaque key generation (for usability)
                  - primitive key comparison (for rapid comparisons)
                  - traverse in order only (for small footprint)
                  - no search function (a bit useless anyway)
                  - additional avltree_lookup_{lowest|highest}() functions (Useful)
                  - data returned after removing node (Useful - NULL means nothing removed)
                  - avltree_destroy for cleaning+deleteing a tree (useful)
                  - OpenMP code (shared memory parallelisation)

                By default the keys are unsigned longs, but this may
                be overridden at compile time by defining the constant
                AVLTREE_KEY_TYPE.

                This code should be thread safe.

                For OpenMP code, USE_OPENMP must be defined and
                avltree_init_openmp() must be called prior to any other
                function.

                A basic test program may be compiled with something like:
                gcc avltree.c -DAVLTREE_COMPILE_MAIN -g

  To do:	rebuilding tree after changing key/hashing function.
                remove function return success flag.
                convert some recursive functions into iterative functions where sensible (e.g. avltree_node_count)
                Use the memory_chunk allocators.  This should now be safe.

 **********************************************************************/

#include "gaul/avltree.h"

/*
 * Private node data structure.
 */
typedef struct AVLNode_t
  {
  struct AVLNode_t *left;	/* Left subtree. */
  struct AVLNode_t *right;	/* Right subtree. */
  int		balance;	/* Height (left) - height (right). */
  AVLKey	key;		/* The key for this node. */
  vpointer	data;		/* Data stored at this node. */
  } AVLNode;

/*
 * Private function prototypes.
 */
static AVLNode	*avltree_node_new(AVLKey key, vpointer data);
static void	avltree_node_free(AVLNode *node);
static void	avltree_node_delete(AVLNode *node);
static AVLNode	*avltree_node_insert(AVLNode *node, AVLKey key,
                                     vpointer data, boolean *inserted);
static AVLNode	*avltree_node_remove(AVLNode *node,
                                     AVLKey key, vpointer *removed_data);
static AVLNode	*avltree_node_balance(AVLNode *node);
static AVLNode	*avltree_node_remove_leftmost(AVLNode *node,
                                             AVLNode **leftmost);
static AVLNode	*avltree_node_restore_left_balance(AVLNode *node,
                                                     int old_balance);
static AVLNode	*avltree_node_restore_right_balance(AVLNode *node,
                                                     int old_balance);
static vpointer	avltree_node_lookup(AVLNode *node, AVLKey key);
static int	avltree_node_count(AVLNode *node);
static boolean	avltree_node_traverse(AVLNode *node,
                                AVLTraverseFunc traverse_func, vpointer userdata);
static int	avltree_node_height(AVLNode *node);
static AVLNode	*avltree_node_rotate_left(AVLNode *node);
static AVLNode	*avltree_node_rotate_right(AVLNode *node);
static void	avltree_node_check(AVLNode *node);

/*
 * Compilation constants.
 */
#define AVL_NODE_BUFFER_NUM_INCR	16
#define AVL_NODE_BUFFER_SIZE		1024

/*
 * Global variables.
 */
static int		AVLnum_trees = 0;	/* Count number of trees in use. */
static int		buffer_num = -1;
static int		num_buffers = 0;
static int		num_used = AVL_NODE_BUFFER_SIZE;
static AVLNode		**node_buffers = NULL;
static AVLNode		*node_free_list = NULL;

/*
 * Note that a single coarse thread lock is used when a number of
 * less coarse locks might be better.
 */
THREAD_LOCK_DEFINE_STATIC(avltree_node_buffer_lock);
#if USE_OPENMP == 1
static boolean avltree_openmp_initialised = FALSE;
#endif

/*
 * Private functions.
 */

/*
 * Deallocate all memory associated with buffers.  Should
 * never be called outside of a "avltree_node_buffer_lock"
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
  num_used = AVL_NODE_BUFFER_SIZE;

  node_free_list = NULL;

  return;
  }


static AVLNode *avltree_node_new(AVLKey key, vpointer data)
  {
  AVLNode *node;

  THREAD_LOCK(avltree_node_buffer_lock);
/*
 * Find an unused node.  Look for unused node in current buffer, then
 * in the free node list, then allocate new buffer.
 */
  if (num_used < AVL_NODE_BUFFER_SIZE)
    {
    node = &(node_buffers[buffer_num][num_used]);
    num_used++;
    }
  else
    {
    if (node_free_list!=NULL)
      {
      node = node_free_list;
      node_free_list = node->right;
      }
    else
      {
      buffer_num++;

      if (buffer_num == num_buffers)
        {
        num_buffers += AVL_NODE_BUFFER_NUM_INCR;
        node_buffers = (AVLNode **)s_realloc(node_buffers, sizeof(AVLNode *)*num_buffers);
        }

      node_buffers[buffer_num] = (AVLNode *)s_malloc(sizeof(AVLNode)*AVL_NODE_BUFFER_SIZE);

      node = node_buffers[buffer_num];
      num_used = 1;
      }
    }
  THREAD_UNLOCK(avltree_node_buffer_lock);

  node->balance = 0;
  node->left = NULL;
  node->right = NULL;
  node->key = key;
  node->data = data;

  return node;
  }


static void avltree_node_free(AVLNode *node)
  {
  THREAD_LOCK(avltree_node_buffer_lock);
  node->right = node_free_list;
  node_free_list = node;
  THREAD_UNLOCK(avltree_node_buffer_lock);

  return;
  }


static void avltree_node_delete(AVLNode *node)
  {

  if (node!=NULL)
    {
    avltree_node_delete(node->right);
    avltree_node_delete(node->left);
    avltree_node_free(node);
    }

  return;
  }


/*
 * Actually, an iterative version would be preferable...
 */
static void avltree_node_destroy(AVLNode *node, AVLDestructorFunc free_func)
  {

  if (node!=NULL)
    {
    avltree_node_destroy(node->right, free_func);
    avltree_node_destroy(node->left, free_func);
    free_func(node->data);
    avltree_node_free(node);
    }

  return;
  }


/*
 * Systematically search tree with a search function which has the
 * same ordering as the tree.
 * This iterative version is much faster than the equivalent recursive version.
 */
static vpointer avltree_node_ordered_search(AVLNode *node,
                     AVLSearchFunc search_func, vpointer userdata)
  {
  int dir;

  while (node!=NULL)
    {
    dir = (*search_func)(node->data, userdata);

    if (dir<0)
      node=node->left;
    else if (dir>0)
      node=node->right;
    else
      return node->data;
    }

  return NULL;
  }


/*
 * Systematically search tree until AVLMatchFunc returns TRUE.
 * This can be fairly slow!  Don't say I didn't warn you.
 */
static boolean avltree_node_search(AVLNode *node,
                     AVLMatchFunc search_func, vpointer userdata, vpointer *node_data)
  {
  *node_data=node->data;

  if ((*search_func)(*node_data, userdata)) return TRUE;

  if (node->left!=NULL)
    if (avltree_node_search(node->left, search_func, userdata, node_data))
      return TRUE;

  if (node->right!=NULL)
    if (avltree_node_search(node->right, search_func, userdata, node_data))
      return TRUE;

  return FALSE;
  }


static AVLNode *avltree_node_insert(AVLNode *node,
                    AVLKey key, vpointer data, boolean *inserted)
  {
  int old_balance;

  if (!node)
    {
    *inserted = TRUE;
    return avltree_node_new(key, data);
    }

  if (key < node->key)
    {
    if (node->left!=NULL)
      {
      old_balance = node->left->balance;
      node->left = avltree_node_insert(node->left, key, data, inserted);

      if ((old_balance != node->left->balance) && node->left->balance)
        node->balance--;
      }
    else
      {
      *inserted = TRUE;
      node->left = avltree_node_new(key, data);
      node->balance--;
      }
    }
  else if (key > node->key)
    {
    if (node->right!=NULL)
      {
      old_balance = node->right->balance;
      node->right = avltree_node_insert(node->right, key, data, inserted);

      if ((old_balance != node->right->balance) && node->right->balance)
        node->balance++;
      }
    else
      {
      *inserted = TRUE;
      node->right = avltree_node_new(key, data);
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

  if (*inserted!=FALSE && (node->balance < -1 || node->balance > 1))
    node = avltree_node_balance(node);

  return node;
  }


static AVLNode *avltree_node_remove(AVLNode *node,
                            AVLKey key, vpointer *removed_data)
  {
  AVLNode	*new_root=NULL;
  int		old_balance;

  if (!node) return NULL;

  if (key < node->key)
    {
    if (node->left!=NULL)
      {
      old_balance = node->left->balance;
      node->left = avltree_node_remove(node->left, key, removed_data);
      node = avltree_node_restore_left_balance(node, old_balance);
      }
    }
  else if (key > node->key)
    {
    if (node->right!=NULL)
      {
      old_balance = node->right->balance;
      node->right = avltree_node_remove(node->right, key, removed_data);
      node = avltree_node_restore_right_balance(node, old_balance);
      }
    }
  else if (key == node->key)
    {
    AVLNode *removed_node;

    removed_node = node;

    if (!node->right)
      {
      node = node->left;
      }
    else
      {
      old_balance = node->right->balance;
      node->right = avltree_node_remove_leftmost(node->right, &new_root);
      if (new_root==NULL) die("Internal error.  New root node is NULL.");
      new_root->left = node->left;
      new_root->right = node->right;
      new_root->balance = node->balance;
      node = avltree_node_restore_right_balance(new_root, old_balance);
      }

    *removed_data = removed_node->data;
    avltree_node_free(removed_node);
    }

  return node;
  }


static AVLNode *avltree_node_balance(AVLNode *node)
  {

  if (node->balance < -1)
    {
    if (node->left->balance > 0)
      node->left = avltree_node_rotate_left(node->left);
    node = avltree_node_rotate_right(node);
    }
  else if (node->balance > 1)
    {
    if (node->right->balance < 0)
      node->right = avltree_node_rotate_right(node->right);
    node = avltree_node_rotate_left(node);
    }

  return node;
  }


static AVLNode *avltree_node_remove_leftmost(AVLNode  *node,
                             AVLNode **leftmost)
  {
  int old_balance;

  if (!node->left)
    {
    *leftmost = node;
    return node->right;
    }

  old_balance = node->left->balance;
  node->left = avltree_node_remove_leftmost(node->left, leftmost);
  return avltree_node_restore_left_balance(node, old_balance);
  }


static AVLNode *avltree_node_lookup_leftmost(AVLNode *node)
  {
/* Recursive version:
  if (!node->left) return node;
  return avltree_node_lookup_leftmost(node->left);
 */

  while (node->left!=NULL) node = node->left;

  return node;
  }


static AVLNode *avltree_node_lookup_rightmost(AVLNode *node)
  {
/* Recursive version:
  if (!node->right) return node;
  return avltree_node_lookup_rightmost(node->right);
 */

  while (node->right!=NULL) node = node->right;

  return node;
  }


static AVLNode *avltree_node_restore_left_balance(AVLNode	*node,
                                  int		old_balance)
  {
  if ( (!node->left) || ((node->left->balance != old_balance) &&
           (node->left->balance == 0)) )
    {
    node->balance++;
    }

  if (node->balance > 1) return avltree_node_balance(node);

  return node;
  }


static AVLNode *avltree_node_restore_right_balance(AVLNode	*node,
                                   int		old_balance)
  {
  if ( (!node->right) || ((node->right->balance != old_balance) &&
           (node->right->balance == 0)) )
    {
    node->balance--;
    }

  if (node->balance < -1) return avltree_node_balance(node);

  return node;
  }


#if 0
/* Recursive version */
static vpointer avltree_node_lookup(AVLNode *node, AVLKey key)
  {
  if (!node) return NULL;

  if (key < node->key)
    {
    if (node->left) return avltree_node_lookup(node->left, key);
    }
  else if (key > node->key)
    {
    if (node->right) return avltree_node_lookup(node->right, key);
    }
  else
    {	/* key == node->key */
    return node->data;
    }

  return NULL;
  }
#endif


/*
 * Iterative version -- much more efficient than the recursive version.
 */
static vpointer avltree_node_lookup(AVLNode *node, AVLKey key)
  {
  while (node!=NULL && key!=node->key)
    {
    if (key < node->key)
      node = node->left;
    else
      node = node->right;
    }
/*
if (node) printf("Found key %p >", node->data); else printf("Not found key >");
*/

  return node;
  }


/*
 * Recursive node counting routine.
 * Need an iterative version.
 */
static int avltree_node_count(AVLNode *node)
  {
  int count=1;

  if (node->left!=NULL) count += avltree_node_count(node->left);
  if (node->right!=NULL) count += avltree_node_count(node->right);

  return count;
  }


/*
 * Stops when an AVLTraverseFunc() callback returns TRUE.
 */
static boolean avltree_node_traverse(AVLNode *node,
                     AVLTraverseFunc traverse_func, vpointer userdata)
  {
  if (node->left!=NULL)
    {
    if (avltree_node_traverse(node->left, traverse_func, userdata)) return TRUE;
    }

  if ((*traverse_func)(node->key, node->data, userdata)) return TRUE;

  if (node->right!=NULL)
    {
    if (avltree_node_traverse(node->right, traverse_func, userdata)) return TRUE;
    }

  return FALSE;
  }


static int avltree_node_height(AVLNode *node)
  {
  int left_height;
  int right_height;

  if (!node) return 0;

  if (node->left!=NULL)
    left_height = avltree_node_height(node->left);
  else
    left_height = 0;

  if (node->right!=NULL)
    right_height = avltree_node_height(node->right);
  else
    right_height = 0;

  return MAX(left_height, right_height) + 1;
  }


static AVLNode *avltree_node_rotate_left(AVLNode *node)
  {
  AVLNode *right;
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


static AVLNode *avltree_node_rotate_right(AVLNode *node)
  {
  AVLNode *left;
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


static void avltree_node_check(AVLNode *node)
  {
  int left_height;
  int right_height;
  int balance;

  if (node!=NULL)
    {
    if (node->left!=NULL)
      left_height = avltree_node_height(node->left);
    else
      left_height = 0;

    if (node->right!=NULL)
      right_height = avltree_node_height(node->right);
    else
      right_height = 0;

    balance = right_height - left_height;
    if (balance != node->balance)
      dief("avltree_node_check: failed: %d ( %d )", balance, node->balance);

    if (node->left!=NULL)
      avltree_node_check(node->left);
    if (node->right!=NULL)
      avltree_node_check(node->right);
    }

  return;
  }


/*
 * Public Interface:
 */

/*
 * This function must be called before any other functions is OpenMP
 * code is to be used.  Can be safely called when OpenMP code is not
 * being used, and can be safely called more than once.
 */
void avltree_init_openmp(void)
  {

#if USE_OPENMP == 1
  if (avltree_openmp_initialised == FALSE)
    {
    omp_init_lock(&avltree_node_buffer_lock);
    avltree_openmp_initialised = TRUE;
    }
#endif

  return;
  }


AVLTree *avltree_new(AVLKeyFunc key_generate_func)
  {
  AVLTree *tree;

  if (!key_generate_func) return NULL;

  AVLnum_trees++;

  tree = (AVLTree *)s_malloc(sizeof(AVLTree));
  if (!tree) die("Unable to allocate memory.");

  tree->root = NULL;
  tree->key_generate_func = key_generate_func;

  return tree;
  }


void avltree_delete(AVLTree *tree)
  {
  if (!tree) return;

  avltree_node_delete(tree->root);

  s_free(tree);

  AVLnum_trees--;

  THREAD_LOCK(avltree_node_buffer_lock);
  if (AVLnum_trees == 0)
    _destroy_buffers();
  THREAD_UNLOCK(avltree_node_buffer_lock);

  return;
  }


void avltree_destroy(AVLTree *tree, AVLDestructorFunc free_func)
  {
  if (!tree) return;

  if (free_func!=NULL)
    avltree_node_destroy(tree->root, free_func);
  else
    avltree_node_delete(tree->root);

  s_free(tree);

  AVLnum_trees--;

  THREAD_LOCK(avltree_node_buffer_lock);
  if (AVLnum_trees == 0)
    _destroy_buffers();
  THREAD_UNLOCK(avltree_node_buffer_lock);

  return;
  }


boolean avltree_insert(AVLTree *tree, vpointer data)
  {
  boolean	inserted=FALSE;

  if (!tree) return FALSE;
  if (!data) return FALSE;	/* ordered search would barf at this! */

  tree->root = avltree_node_insert(tree->root,
                    tree->key_generate_func(data), data, &inserted);

  return inserted;
  }


vpointer avltree_remove(AVLTree *tree, vpointer data)
  {
  vpointer removed=NULL;

  if (!tree || !tree->root) return NULL;

  tree->root = avltree_node_remove(tree->root, tree->key_generate_func(data), &removed);

  return removed;
  }


vpointer avltree_remove_key(AVLTree *tree, AVLKey key)
  {
  vpointer removed=NULL;

  if (!tree || !tree->root) return NULL;

  tree->root = avltree_node_remove(tree->root, key, &removed);

  return removed;
  }


vpointer avltree_lookup(AVLTree *tree, vpointer data)
  {
  AVLNode	*node;

  if (!tree || !tree->root) return NULL;

  node = (AVLNode *)avltree_node_lookup(tree->root, tree->key_generate_func(data));
  return node?node->data:NULL;
  }


vpointer avltree_lookup_key(AVLTree *tree, AVLKey key)
  {
  AVLNode	*node;

  if (!tree || !tree->root) return NULL;

  node = (AVLNode *)avltree_node_lookup(tree->root, key);
  return node?node->data:NULL;
  }


vpointer avltree_lookup_lowest(AVLTree *tree)
  {
  AVLNode	*node;

  if (!tree || !tree->root) return NULL;

  node = avltree_node_lookup_leftmost(tree->root);
  return node?node->data:NULL;
  }


vpointer avltree_lookup_highest(AVLTree *tree)
  {
  AVLNode	*node;

  if (!tree || !tree->root) return NULL;

  node = avltree_node_lookup_rightmost(tree->root);
  return node?node->data:NULL;
  }


vpointer avltree_ordered_search(AVLTree *tree,
                    AVLSearchFunc search_func, vpointer userdata)
  {
  if (!tree || !tree->root) return NULL;

  return avltree_node_ordered_search(tree->root, search_func, userdata);
  }


vpointer avltree_search(AVLTree *tree, AVLMatchFunc search_func, vpointer userdata)
  {
  vpointer	nodedata=NULL;

  if (!tree || !tree->root) return NULL;

  return avltree_node_search(tree->root, search_func, userdata, &nodedata)?nodedata:NULL;
  }


void avltree_traverse(AVLTree *tree, AVLTraverseFunc traverse_func, vpointer userdata)
  {
  if (!tree || !tree->root) return;

  avltree_node_traverse(tree->root, traverse_func, userdata);

  return;
  }


int avltree_height(AVLTree *tree)
  {
  return (tree!=NULL && tree->root!=NULL)?avltree_node_height(tree->root):0;
  }


int avltree_num_nodes(AVLTree *tree)
  {
  return (tree!=NULL && tree->root!=NULL)?avltree_node_count(tree->root):0;
  }


/*
 * Testing:
 */

void avltree_diagnostics(void)
  {
  printf("=== AVLTree diagnostics ======================================\n");
  printf("Version:                   %s\n", GA_VERSION_STRING);
  printf("Build date:                %s\n", GA_BUILD_DATE_STRING);
  printf("Compilation machine characteristics:\n%s\n", GA_UNAME_STRING);

  printf("--------------------------------------------------------------\n");
  printf("structure                  sizeof\n");
  printf("AVLTree                    %lu\n", (unsigned long) sizeof(AVLTree));
  printf("AVLNode                    %lu\n", (unsigned long) sizeof(AVLNode));
  printf("--------------------------------------------------------------\n");
  printf("Trees in use:              %d\n", AVLnum_trees);
  printf("==============================================================\n");

  return;
  }


static boolean failed = FALSE;

static AVLKey test_avltree_generate(constvpointer data)
  {
/*
 * Simple casting from char to AVLKey... should work ;)
 * (It works when AVLKey is the default unsigned long...)
 */
  return (AVLKey) *((char *)data);
/*
  return (AVLKey) (data);
*/
  }

static boolean test_avltree_traverse(AVLKey key, vpointer data, vpointer userdata)
  {
/* check. */
  if (key != test_avltree_generate(data))
    {
    printf("failure (%ld %ld) ", key, test_avltree_generate(data));
    failed=TRUE;
    }

/* output character. */
  printf("%c ", *((char *)data));
/*
  printf("%c=%ld ", *((char *)data), (long) test_avltree_generate(data));
*/

/* terminate traversal if userdata is non-NULL and character is 'S'. */
  if ((boolean *)userdata!=NULL && *((char *)data)=='S')
    {
    printf("%s ", (char *)userdata);
    return TRUE;
    }

  return FALSE;
  }

#ifdef AVLTREE_COMPILE_MAIN
int main(int argc, char **argv)
#else
boolean avltree_test(void)
#endif
  {
  int		i, j;
  AVLTree	*tree;
  char		chars[62];
  char		chx='x', chX='X', *ch;

  printf("Testing my dodgy AVL tree routines.\n");

  tree = avltree_new(test_avltree_generate);

  i = 0;
  for (j = 0; j < 26; j++, i++)
    {
    chars[i] = 'A' + (char) j;
    avltree_insert(tree, &chars[i]);
    }
  for (j = 0; j < 26; j++, i++)
    {
    chars[i] = 'a' + (char) j;
    avltree_insert(tree, &chars[i]);
    }
  for (j = 0; j < 10; j++, i++)
    {
    chars[i] = '0' + (char) j;
    avltree_insert(tree, &chars[i]);
    }

  printf("height: %d\n", avltree_height(tree));
  printf("num nodes: %d\n", avltree_num_nodes(tree));

  printf("tree: ");
  avltree_traverse(tree, test_avltree_traverse, NULL);
  printf("\n");

  printf("tree to 'S' then foo: ");
  avltree_traverse(tree, test_avltree_traverse, (void *)"foo");
  printf("\n");

  for (i = 0; i < 26; i++)
    if ( !avltree_remove(tree, &chars[i]) ) printf("%c not found.\n", chars[i]);

  printf("height: %d\n", avltree_height(tree));
  printf("num nodes: %d\n", avltree_num_nodes(tree));

  printf("tree: ");
  avltree_traverse(tree, test_avltree_traverse, NULL);
  printf("\n");

  printf("Lookup for 'x': ");
  ch = (char *) avltree_lookup(tree, (vpointer) &chx);
  if (ch) printf("Found '%c'\n", *ch); else printf("Not found.\n");

  printf("Lookup for 'X': ");
  ch = (char *) avltree_lookup(tree, (vpointer) &chX);
  if (ch) printf("Found '%c'\n", *ch); else printf("Not found.\n");

  printf("Tests:         %s\n", failed?"FAILED":"PASSED");

  avltree_delete(tree);

  return failed;
  }

