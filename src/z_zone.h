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
// DESCRIPTION:
//      Zone Memory Allocation, perhaps NeXT ObjectiveC inspired.
//  Remark: this was the only stuff that, according
//   to John Carmack, might have been useful for
//   Quake.
//
//---------------------------------------------------------------------



#ifndef __Z_ZONE__
#define __Z_ZONE__

#include <stdio.h>

//
// ZONE MEMORY
// PU - purge tags.
// Tags < 100 are not overwritten until freed.
#define PU_STATIC   1 // static entire execution time
#define PU_SOUND    2 // static while playing
#define PU_MUSIC    3 // static while playing
#define PU_FREE   4
#define PU_LEVEL    50  // static until level exited
#define PU_LEVSPEC    51      // a special thinker in a level
// Tags >= 100 are purgable whenever needed.
#define PU_PURGELEVEL 100
#define PU_CACHE    101


void  Z_Init (void);
void* Z_Malloc (int size, int tag, void* ptr);
void    Z_Free (void* ptr);
void    Z_FreeTags (int lowtag, int hightag);
void    Z_DumpHeap (int lowtag, int hightag);
void    Z_FileDumpHeap (FILE* f);
void    Z_CheckHeap (void);
void    Z_ChangeTag2 (void* ptr, int tag, const char* filename, int line);
int     Z_FreeMemory (void);

//
// This is used to get the local FILE:LINE info from CPP
// prior to really call the function in question.
//
#define Z_ChangeTag(p, t) \
  Z_ChangeTag2((p), (t), __FILE__, __LINE__)

#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
