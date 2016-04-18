// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//  Zone Memory Allocation. Neat.
//
//-----------------------------------------------------------------------------
#include "z_zone.h"
#include "i_system.h"
#include "doomdef.h"

//
// ZONE MEMORY ALLOCATION
//
// There is never any space between memblocks,
//  and there will never be two contiguous free memblocks.
// The rover can be left pointing at a non-empty block.
//
// It is of no value to free a cachable block,
//  because it will get overwritten automatically if needed.
//

static const int ZONEID = 0x1d4a11;
static const int MINFRAGMENT = 64;

typedef struct memblock_s
{
  /** Including the header and possibly tiny fragments. */
  int size;
  /** NULL if a free block. */
  void** user;
  /** Purgelevel. */
  int tag;
  /** Should be ZONEID. */
  int id;
  struct memblock_s* next;
  struct memblock_s* prev;
} memblock_t;

typedef struct
{
  int size;
  memblock_t blocklist;
  memblock_t* rover;
} memzone_t;

static memzone_t* mainzone;

void Z_Init()
{
  memblock_t* block;
  int size = 0;

  mainzone = (memzone_t*) I_ZoneBase(&size);
  mainzone->size = size;

  // Set the entire zone to one free block.
  mainzone->blocklist.next = mainzone->blocklist.prev = block
    = (memblock_t*) ((byte*) mainzone + sizeof(memzone_t));

  mainzone->blocklist.user = (void*) mainzone;
  mainzone->blocklist.tag = PU_STATIC;
  mainzone->rover = block;

  block->prev = block->next = &mainzone->blocklist;

  // NULL indicates a free block.
  block->user = NULL;

  block->size = mainzone->size - sizeof(memzone_t);
}

void Z_Free(void* ptr)
{
  memblock_t* block = (memblock_t*) ((byte*) ptr - sizeof(memblock_t));
  memblock_t* other;

  if (block->id != ZONEID)
  {
    I_Error("Z_Free: Freed a pointer without ZONEID.");
  }

  if (block->tag != PU_FREE && block->user)
  {
    // Clear the user's mark.
    *block->user = NULL;
  }

  // Mark as free.
  block->user = NULL;
  block->tag = PU_FREE;
  block->id = 0;

  other = block->prev;
  if (other->tag == PU_FREE)
  {
    // Merge with previous free block.
    other->size += block->size;
    other->next = block->next;
    other->next->prev = other;

    if (block == mainzone->rover)
    {
      mainzone->rover = other;
    }

    block = other;
  }

  other = block->next;
  if (other->tag == PU_FREE)
  {
    // Merge the next free block onto the end.
    block->size += other->size;
    block->next = other->next;
    block->next->prev = other;

    if (other == mainzone->rover)
    {
      mainzone->rover = block;
    }
  }
}

/**
 * You can pass a NULL user if the tag is < PU_PURGELEVEL.
 */
void* Z_Malloc(int size, int tag, void* user)
{
  int extra;
  memblock_t* start;
  memblock_t* rover;
  memblock_t* base;
  void* result;

  size = (size + sizeof(void*) - 1) & ~(sizeof(void*) - 1);

  // Scan through the block list, looking for the first free block of
  // sufficient size, throwing out any purgable blocks along the way.

  // Account for size of block header.
  size += sizeof(memblock_t);

  // If there is a free block behind the rover, back up over them.
  base = mainzone->rover;

  if (base->prev->tag == PU_FREE)
  {
    base = base->prev;
  }

  rover = base;
  start = base->prev;

  do
  {
    if (rover == start)
    {
      // Scanned all the way around the list.
      I_Error("Z_Malloc: Failed on allocation of %d bytes.", size);
    }

    if (rover->tag != PU_FREE)
    {
      if (rover->tag < PU_PURGELEVEL)
      {
        // Hit a block that can't be purged, so move past it.
        base = rover = rover->next;
      }
      else
      {
        // Free the rover block (adding the size to base). The rover can be the
        // base block.
        base = base->prev;
        Z_Free((byte*) rover + sizeof(memblock_t));
        base = base->next;
        rover = base->next;
      }
    }
    else
    {
      rover = rover->next;
    }
  }
  while (base->tag != PU_FREE || base->size < size);

  // Found a block big enough.
  extra = base->size - size;

  if (extra > MINFRAGMENT)
  {
    // There will be a free fragment after the allocated block.
    memblock_t* newblock = (memblock_t*) ((byte*) base + size);

    newblock->size = extra;

    newblock->tag = PU_FREE;
    newblock->user = NULL;
    newblock->prev = base;
    newblock->next = base->next;
    newblock->next->prev = newblock;

    base->next = newblock;
    base->size = size;
  }

  if (!user && tag >= PU_PURGELEVEL)
  {
    I_Error("Z_Malloc: An owner is required for purgable blocks.");
  }

  base->user = user;
  base->tag = tag;

  result = (void*) ((byte*) base + sizeof(memblock_t));

  if (base->user)
  {
    *base->user = result;
  }

  // Next allocation will start looking here.
  mainzone->rover = base->next;

  base->id = ZONEID;

  return result;
}

void Z_FreeTags(int lowtag, int hightag)
{
  memblock_t* block;
  memblock_t* next;

  for (block = mainzone->blocklist.next;
       block != &mainzone->blocklist;
       block = next)
  {
    // Get link before freeing.
    next = block->next;

    // Free block?
    if (block->tag == PU_FREE)
    {
      continue;
    }

    if (block->tag >= lowtag && block->tag <= hightag)
    {
      Z_Free((byte*) block + sizeof(memblock_t));
    }
  }
}

void Z_CheckHeap()
{
  memblock_t* block;

  for (block = mainzone->blocklist.next; ; block = block->next)
  {
    if (block->next == &mainzone->blocklist)
    {
      // All blocks have been hit.
      return;
    }

    if ((byte*) block + block->size != (byte*) block->next)
    {
      I_Error("Z_CheckHeap: Block size does not touch the next block.");
    }

    if (block->next->prev != block)
    {
      I_Error("Z_CheckHeap: Next block doesn't have proper back link.");
    }

    if (block->tag == PU_FREE && block->next->tag == PU_FREE)
    {
      I_Error("Z_CheckHeap: Two consecutive free block.");
    }
  }
}

void Z_ChangeTag2(void* ptr, int tag, const char* filename, int line)
{
  memblock_t* block = (memblock_t*) ((byte*) ptr - sizeof(memblock_t));

  if (block->id != ZONEID)
  {
    I_Error("%s:%d: Z_ChangeTag: Block without a ZONEID.", filename, line);
  }

  if (tag >= PU_PURGELEVEL && !block->user)
  {
    I_Error("%s:%d: Z_ChangeTag: An owner is required.", filename, line);
  }

  block->tag = tag;
}
