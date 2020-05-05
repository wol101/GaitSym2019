/**********************************************************************
  memory_chunks.h
 **********************************************************************

  memory_chunks - Efficient bulk memory allocation.
  Copyright ©2001-2004, Stewart Adcock <stewart@linux-domain.com>
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

  Synopsis:	Header file for memory_chunks.c

 **********************************************************************/

#ifndef MEMORY_CHUNKS_H_INCLUDED
#define MEMORY_CHUNKS_H_INCLUDED

#include "gaul/gaul_util.h"

#include <stdlib.h>
#include <string.h>

#include "gaul/memory_util.h"

#ifndef MEMORY_ALIGN_SIZE
#define MEMORY_ALIGN_SIZE       MAX(sizeof(void *), sizeof(long))
#endif

#ifndef MEMORY_PADDING
#define MEMORY_PADDING	FALSE
#endif

/*
 * Data types.
 */

typedef struct MemChunk_t MemChunk;

/*
 * Prototypes.
 */

FUNCPROTO void		mem_chunk_init_openmp(void);

FUNCPROTO MemChunk	*mem_chunk_new_real(size_t atom_size, unsigned int num_atoms);
FUNCPROTO MemChunk	*mem_chunk_new_unfreeable_real(size_t atom_size, unsigned int num_atoms);
FUNCPROTO boolean		mem_chunk_has_freeable_atoms_real(MemChunk *mem_chunk);
FUNCPROTO boolean		mem_chunk_isempty_real(MemChunk *mem_chunk);
FUNCPROTO void		mem_chunk_destroy_real(MemChunk *mem_chunk);
FUNCPROTO void		*mem_chunk_alloc_real(MemChunk *mem_chunk);
FUNCPROTO void		mem_chunk_free_real(MemChunk *mem_chunk, void *mem);
FUNCPROTO void		mem_chunk_clean_real(MemChunk *mem_chunk);
FUNCPROTO void		mem_chunk_reset_real(MemChunk *mem_chunk);
FUNCPROTO boolean		mem_chunk_test_real(void);
FUNCPROTO boolean		mem_chunk_check_all_bounds_real(MemChunk *mem_chunk);
FUNCPROTO boolean		mem_chunk_check_bounds_real(MemChunk *mem_chunk, void *mem);
FUNCPROTO void		mem_chunk_diagnostics_real(void);

FUNCPROTO MemChunk	*mem_chunk_new_mimic(size_t atom_size, unsigned int num_atoms);
FUNCPROTO MemChunk	*mem_chunk_new_unfreeable_mimic(size_t atom_size, unsigned int num_atoms);
FUNCPROTO boolean		mem_chunk_has_freeable_atoms_mimic(MemChunk *mem_chunk);
FUNCPROTO boolean		mem_chunk_isempty_mimic(MemChunk *mem_chunk);
FUNCPROTO void		mem_chunk_destroy_mimic(MemChunk *mem_chunk);
FUNCPROTO void		*mem_chunk_alloc_mimic(MemChunk *mem_chunk);
FUNCPROTO void		mem_chunk_free_mimic(MemChunk *mem_chunk, void *mem);
FUNCPROTO void		mem_chunk_clean_mimic(MemChunk *mem_chunk);
FUNCPROTO void		mem_chunk_reset_mimic(MemChunk *mem_chunk);
FUNCPROTO boolean		mem_chunk_test_mimic(void);
FUNCPROTO boolean		mem_chunk_check_all_bounds_mimic(MemChunk *mem_chunk);
FUNCPROTO boolean		mem_chunk_check_bounds_mimic(MemChunk *mem_chunk, void *mem);
FUNCPROTO void		mem_chunk_diagnostics_mimic(void);

/*
 * Exposed API.
 */

#ifndef MEMORY_CHUNKS_MIMIC

#define mem_chunk_new(Y,Z)		mem_chunk_new_real((Y), (Z))
#define mem_chunk_new_unfreeable(Y,Z)	mem_chunk_new_unfreeable_real((Y), (Z))
#define mem_chunk_has_freeable_atoms(Z)	mem_chunk_has_freeable_atoms_real((Z))
#define mem_chunk_isempty(Z)		mem_chunk_isempty_real((Z))
#define mem_chunk_destroy(Z)		mem_chunk_destroy_real((Z))
#define mem_chunk_alloc(Z)		mem_chunk_alloc_real((Z))
#define mem_chunk_free(Y,Z)		mem_chunk_free_real((Y), (Z))
#define mem_chunk_clean(Z)		mem_chunk_clean_real((Z))
#define mem_chunk_reset(Z)		mem_chunk_reset_real((Z))
#define mem_chunk_test			mem_chunk_test_real()
#define mem_chunk_check_all_bounds(Z)	mem_chunk_check_all_bounds_real(Z)
#define mem_chunk_check_bounds(Y,Z)	mem_chunk_check_bounds_real((Y), (Z))
#define mem_chunk_diagnostics		mem_chunk_diagnostics_real()

#else

#define mem_chunk_new(Y,Z)		mem_chunk_new_mimic((Y), (Z))
#define mem_chunk_new_unfreeable(Y,Z)	mem_chunk_new_unfreeable_mimic((Y), (Z))
#define mem_chunk_has_freeable_atoms(Z)	mem_chunk_has_freeable_atoms_mimic((Z))
#define mem_chunk_isempty(Z)		mem_chunk_isempty_mimic((Z))
#define mem_chunk_destroy(Z)		mem_chunk_destroy_mimic((Z))
#define mem_chunk_alloc(Z)		mem_chunk_alloc_mimic((Z))
#define mem_chunk_free(Y,Z)		mem_chunk_free_mimic((Y), (Z))
#define mem_chunk_clean(Z)		mem_chunk_clean_mimic((Z))
#define mem_chunk_reset(Z)		mem_chunk_reset_mimic((Z))
#define mem_chunk_test			mem_chunk_test_mimic()
#define mem_chunk_check_all_bounds(Z)	mem_chunk_check_all_bounds_mimic(Z)
#define mem_chunk_check_bounds(Y,Z)	mem_chunk_check_bounds_mimic((Y), (Z))
#define mem_chunk_diagnostics		mem_chunk_diagnostics_mimic()

#endif

#endif /* MEMORY_CHUNKS_H_INCLUDED */

