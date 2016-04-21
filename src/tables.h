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
//  Lookup tables.
//  Do not try to look them up :-).
//  In the order of appearance:
//
//  int finetangent[4096] - Tangens LUT.
//   Should work with BAM fairly well (12 of 16bit,
//      effectively, by shifting).
//
//  int finesine[10240]   - Sine lookup.
//   Guess what, serves as cosine, too.
//   Remarkable thing is, how to use BAMs with this?
//
//  int tantoangle[2049]  - ArcTan LUT,
//    maps tan(angle) to angle fast. Gotta search.
//
//-----------------------------------------------------------------------------
#ifndef __TABLES__
#define __TABLES__

#define FINEANGLES 8192
#define SLOPERANGE 2048

extern const int FINEMASK;
extern const int ANGLETOFINESHIFT;

extern const int ANG45;
extern const int ANG90;
extern const int ANG180;
extern const int ANG270;

extern const int SLOPEBITS;
extern const int DBITS;

// Effective size is 10240.
extern const fixed_t finesine[5 * FINEANGLES / 4];

// Re-use data, is just PI/2 pahse shift.
extern const fixed_t* finecosine;

// Effective size is 4096.
extern const fixed_t finetangent[FINEANGLES / 2];

typedef uint32_t angle_t;


// Effective size is 2049;
// The +1 size is to handle the case when x==y
//  without additional checking.
extern const angle_t tantoangle[SLOPERANGE + 1];

int SlopeDiv(unsigned int num, unsigned int den);

#endif
