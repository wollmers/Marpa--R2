/* This file is a modification of one of the versions of the GNU obstack.c
 * which was LGPL 2.1.  Here is the copyright notice from that file:
 *
 * obstack.c - subroutines used implicitly by object stack macros
 * Copyright (C) 1988, 1989, 1990, 1991, 1992, 1993, 1994, 1996, 1997, 1998,
 * 1999, 2000, 2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.
 * This file is part of the GNU C Library.
 */

/*
 * Copyright 2013 Jeffrey Kegler
 * This file is part of Marpa::R2.  Marpa::R2 is free software: you can
 * redistribute it and/or modify it under the terms of the GNU Lesser
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * Marpa::R2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser
 * General Public License along with Marpa::R2.  If not, see
 * http://www.gnu.org/licenses/.
 */

# include "config.h"

# include "marpa.h"
# include "marpa_ami.h"
# include "marpa_obs.h"

/* Default size is what GNU malloc can fit in a 4096-byte block.
     12 is sizeof (mhead) and 4 is EXTRA from GNU malloc.
     Use the values for range checking, because if range checking is off,
     the extra bytes won't be missed terribly, but if range checking is on
     and we used a larger request, a whole extra 4096 bytes would be
     allocated.

     These number are irrelevant to the new GNU malloc.  I suspect it is
     less sensitive to the size of the request. 
     If malloc were really smart, it would round addresses to DEFAULT_ALIGNMENT.
     But in fact it might be less smart and round addresses to as much as
     WORST_MALLOC_ROUNDING.  So we prepare for it to do that.
     */
#define MALLOC_OVERHEAD ( ALIGN_UP(12, WORST_MALLOC_ROUNDING) + ALIGN_UP(4, WORST_MALLOC_ROUNDING))
#define MINIMUM_CHUNK_SIZE (sizeof(struct marpa_obstack_chunk))
#define DEFAULT_CHUNK_SIZE (4096 - MALLOC_OVERHEAD)

struct marpa_obstack *
marpa__obs_begin (int size)
{
  struct marpa_obstack_chunk *chunk;	/* points to new chunk */
  struct marpa_obstack *h;	/* points to new obstack */
  /* Just enough room for the chunk and obstack headers */

  if (MARPA_OBSTACK_DEBUG)
    {
      /* Use the minimum size if we are debugging */
      size = MINIMUM_CHUNK_SIZE;
    }
  else
    {
      size = MAX (DEFAULT_CHUNK_SIZE, size);
    }
  chunk = my_malloc (size);
  h = &chunk->contents.obstack_header;

  h->chunk = chunk;

  /* The first object can go after the obstack header */
  h->next_free = h->object_base = ((char *) h + sizeof (*h));
  chunk->header.size = h->minimum_chunk_size = size;
  chunk->header.prev = 0;
  return h;
}

/* Allocate a new current chunk for the obstack *H
   on the assumption that LENGTH bytes need to be added
   to the current object, or a new object of length LENGTH allocated.
   Unlike original GNU obstacks, does *NOT*
   copy any partial object from the end of the old chunk
   to the beginning of the new one.  */

void
marpa__obs_newchunk (struct marpa_obstack *h, int length)
{
  struct marpa_obstack_chunk *old_chunk = h->chunk;
  struct marpa_obstack_chunk *new_chunk;
  long  new_size;
  char *object_base;

  /* Compute size for new chunk.
   * Make sure there is enough room for |length|
   * after adjusting alignment.
   */
  new_size = length + offsetof(struct marpa_obstack_chunk, contents) + DEFAULT_ALIGNMENT;
  if (!MARPA_OBSTACK_DEBUG && new_size < h->minimum_chunk_size) {
    new_size = h->minimum_chunk_size;
  }

  /* Allocate and initialize the new chunk.  */
  new_chunk = my_malloc( new_size);
  h->chunk = new_chunk;
  new_chunk->header.prev = old_chunk;
  new_chunk->header.size = new_size;

  object_base = new_chunk->contents.contents;

  h->object_base = object_base;
  h->next_free = h->object_base;
}

/* Free everything in H.  */
void
marpa__obs_free (struct marpa_obstack *h)
{
  struct marpa_obstack_chunk *lp;       /* below addr of any objects in this chunk */
  struct marpa_obstack_chunk *plp;      /* point to previous chunk if any */

  if (!h)
    return;                     /* Return safely if never initialized */
  lp = h->chunk;
  while (lp != 0)
    {
      plp = lp->header.prev;
      my_free (lp);
      lp = plp;
    }
}

/* vim: set expandtab shiftwidth=4: */
