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
// Revision 1.3  1997/01/29 20:10
// DESCRIPTION:
//  Preparation of data for rendering,
//  generation of lookups, caching, retrieval by name.
//
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "i_system.h"
#include "z_zone.h"

#include "m_swap.h"

#include "w_wad.h"

#include "doomdef.h"
#include "m_fixed.h"
#include "r_local.h"
#include "p_local.h"

#include "doomstat.h"
#include "r_sky.h"

#include "r_data.h"

//
// Graphics.
// DOOM graphics for walls and sprites
// is stored in vertical runs of opaque pixels (posts).
// A column is composed of zero or more posts,
// a patch or sprite is composed of zero or more columns.
//



//
// Texture definition.
// Each texture is composed of one or more patches,
// with patches being lumps stored in the WAD.
// The lumps are referenced by number, and patched
// into the rectangular texture space using origin
// and possibly other attributes.
//
typedef struct
{
  int16_t originx;
  int16_t originy;
  int16_t patch;
  int16_t stepdir;
  int16_t colormap;
} mappatch_t;

/**
 * Texture definition.
 *
 * A DOOM wall texture is a list of patches which are to be combined in a
 * predefined order.
 */
typedef struct
{
  char        name[8];
  int8_t      masked;
  int16_t     width;
  int16_t     height;
  int         obsolete;
  int16_t     patchcount;
  mappatch_t  patches[1];
} MapTexture;


// A single patch from a texture definition,
//  basically a rectangular area within
//  the texture rectangle.
typedef struct
{
  // Block origin (allways UL),
  // which has allready accounted
  // for the internal origin of the patch.
  int32_t originx;
  int32_t originy;
  int32_t patch;
} texpatch_t;


// A maptexturedef_t describes a rectangular texture,
//  which is composed of one or more mappatch_t structures
//  that arrange graphic patches.
typedef struct
{
  // Keep name for switch changing, etc.
  char  name[8];
  int16_t width;
  int16_t height;

  // All the patches[patchcount]
  //  are drawn back to front into the cached texture.
  int16_t patchcount;
  texpatch_t  patches[1];

} texture_t;



int   firstflat;
int   lastflat;
int   numflats;

int   firstpatch;
int   lastpatch;
int   numpatches;

int   firstspritelump;
int   lastspritelump;
int   numspritelumps;

int   numtextures;
texture_t** textures;


int*      texturewidthmask;
// needed for texture pegging
fixed_t*    textureheight;
int*      texturecompositesize;
short**     texturecolumnlump;
unsigned short**  texturecolumnofs;
uint8_t**      texturecomposite;

// for global animation
int*    flattranslation;
int*    texturetranslation;

// needed for pre rendering
fixed_t*  spritewidth;
fixed_t*  spriteoffset;
fixed_t*  spritetopoffset;

lighttable_t*  colormaps;


//
// MAPTEXTURE_T CACHING
// When a texture is first needed,
//  it counts the number of composite columns
//  required in the texture and allocates space
//  for a column directory and any new columns.
// The directory will simply point inside other patches
//  if there is only one patch in a given column,
//  but any columns with multiple patches
//  will have new column_ts generated.
//



//
// R_DrawColumnInCache
// Clip and draw a column
//  from a patch into a cached post.
//
void
R_DrawColumnInCache
( column_t* patch,
  uint8_t*   cache,
  int   originy,
  int   cacheheight )
{
  int   count;
  int   position;
  uint8_t* source;

  while (patch->topdelta != 0xff)
  {
    source = (uint8_t*)patch + 3;
    count = patch->length;
    position = originy + patch->topdelta;

    if (position < 0)
    {
      count += position;
      position = 0;
    }

    if (position + count > cacheheight)
    {
      count = cacheheight - position;
    }

    if (count > 0)
    {
      memcpy (cache + position, source, count);
    }

    patch = (column_t*)(  (uint8_t*)patch + patch->length + 4);
  }
}



//
// R_GenerateComposite
// Using the texture definition,
//  the composite texture is created from the patches,
//  and each column is cached.
//
void R_GenerateComposite (int texnum)
{
  uint8_t*   block;
  texture_t*    texture;
  texpatch_t*   patch;
  patch_t*    realpatch;
  int     x;
  int     x1;
  int     x2;
  int     i;
  column_t*   patchcol;
  short*    collump;
  unsigned short* colofs;

  texture = textures[texnum];

  block = Z_Malloc (texturecompositesize[texnum],
                    PU_STATIC,
                    &texturecomposite[texnum]);

  collump = texturecolumnlump[texnum];
  colofs = texturecolumnofs[texnum];

  // Composite the columns together.
  patch = texture->patches;

  for (i = 0 , patch = texture->patches;
       i < texture->patchcount;
       i++, patch++)
  {
    realpatch = W_CacheLumpNum (patch->patch, PU_CACHE);
    x1 = patch->originx;
    x2 = x1 + SHORT(realpatch->width);

    if (x1 < 0)
    {
      x = 0;
    }
    else
    {
      x = x1;
    }

    if (x2 > texture->width)
    {
      x2 = texture->width;
    }

    for ( ; x < x2 ; x++)
    {
      // Column does not have multiple patches?
      if (collump[x] >= 0)
      {
        continue;
      }

      patchcol = (column_t*)((uint8_t*)realpatch
                             + LONG(realpatch->columnofs[x - x1]));
      R_DrawColumnInCache (patchcol,
                           block + colofs[x],
                           patch->originy,
                           texture->height);
    }

  }

  // Now that the texture has been built in column cache,
  //  it is purgable from zone memory.
  Z_ChangeTag (block, PU_CACHE);
}



//
// R_GenerateLookup
//
void R_GenerateLookup (int texnum)
{
  texture_t*    texture;
  uint8_t*   patchcount; // patchcount[texture->width]
  texpatch_t*   patch;
  patch_t*    realpatch;
  int     x;
  int     x1;
  int     x2;
  int     i;
  short*    collump;
  unsigned short* colofs;

  texture = textures[texnum];

  // Composited texture not created yet.
  texturecomposite[texnum] = 0;

  texturecompositesize[texnum] = 0;
  collump = texturecolumnlump[texnum];
  colofs = texturecolumnofs[texnum];

  // Now count the number of columns
  //  that are covered by more than one patch.
  // Fill in the lump / offset, so columns
  //  with only a single patch are all done.
  patchcount = (uint8_t*) Z_Malloc(texture->width, PU_STATIC, &patchcount);
  memset (patchcount, 0, texture->width);
  patch = texture->patches;

  for (i = 0 , patch = texture->patches;
       i < texture->patchcount;
       i++, patch++)
  {
    realpatch = W_CacheLumpNum (patch->patch, PU_CACHE);
    x1 = patch->originx;
    x2 = x1 + SHORT(realpatch->width);

    if (x1 < 0)
    {
      x = 0;
    }
    else
    {
      x = x1;
    }

    if (x2 > texture->width)
    {
      x2 = texture->width;
    }
    for ( ; x < x2 ; x++)
    {
      patchcount[x]++;
      collump[x] = patch->patch;
      colofs[x] = LONG(realpatch->columnofs[x - x1]) + 3;
    }
  }

  for (x = 0 ; x < texture->width ; x++)
  {
    if (!patchcount[x])
    {
      printf ("R_GenerateLookup: column without a patch (%s)\n",
              texture->name);
      return;
    }
    // I_Error ("R_GenerateLookup: column without a patch");

    if (patchcount[x] > 1)
    {
      // Use the cached block.
      collump[x] = -1;
      colofs[x] = texturecompositesize[texnum];

      if (texturecompositesize[texnum] > 0x10000 - texture->height)
      {
        I_Error ("R_GenerateLookup: texture %i is >64k",
                 texnum);
      }

      texturecompositesize[texnum] += texture->height;
    }
  }
}




//
// R_GetColumn
//
uint8_t*
R_GetColumn
( int   tex,
  int   col )
{
  int   lump;
  int   ofs;

  col &= texturewidthmask[tex];
  lump = texturecolumnlump[tex][col];
  ofs = texturecolumnofs[tex][col];

  if (lump > 0)
  {
    return (uint8_t*)W_CacheLumpNum(lump, PU_CACHE) + ofs;
  }

  if (!texturecomposite[tex])
  {
    R_GenerateComposite (tex);
  }

  return texturecomposite[tex] + ofs;
}




//
// R_InitTextures
// Initializes the texture list
//  with the textures from the world map.
//
void R_InitTextures (void)
{
  MapTexture* mtexture;
  texture_t*    texture;
  mappatch_t*   mpatch;
  texpatch_t*   patch;

  int     i;
  int     j;

  int*    maptex;
  int*    maptex2;
  int*    maptex1;

  char    name[9];
  char*   names;
  char*   name_p;

  int*    patchlookup;

  int     totalwidth;
  int     nummappatches;
  int     offset;
  int     maxoff;
  int     maxoff2;
  int     numtextures1;
  int     numtextures2;

  int*    directory;

  int     temp1;
  int     temp2;
  int     temp3;


  // Load the patch names from pnames.lmp.
  name[8] = 0;
  names = W_CacheLumpName ("PNAMES", PU_STATIC);
  nummappatches = LONG ( *((int*)names) );
  name_p = names + 4;
  patchlookup = (int*) malloc(nummappatches * sizeof(int));
  //patchlookup = Z_Malloc(nummappatches * sizeof(*patchlookup), PU_STATIC, NULL);

  for (i = 0 ; i < nummappatches ; i++)
  {
    strncpy (name, name_p + i * 8, 8);
    patchlookup[i] = W_CheckNumForName (name);
  }
  Z_Free (names);

  // Load the map texture definitions from textures.lmp.
  // The data is contained in one or two lumps,
  //  TEXTURE1 for shareware, plus TEXTURE2 for commercial.
  maptex = maptex1 = W_CacheLumpName ("TEXTURE1", PU_STATIC);
  numtextures1 = LONG(*maptex);
  maxoff = W_LumpLength (W_GetNumForName ("TEXTURE1"));
  directory = maptex + 1;

  if (W_CheckNumForName ("TEXTURE2") != -1)
  {
    maptex2 = W_CacheLumpName ("TEXTURE2", PU_STATIC);
    numtextures2 = LONG(*maptex2);
    maxoff2 = W_LumpLength (W_GetNumForName ("TEXTURE2"));
  }
  else
  {
    maptex2 = NULL;
    numtextures2 = 0;
    maxoff2 = 0;
  }
  numtextures = numtextures1 + numtextures2;

  textures = Z_Malloc(numtextures * sizeof(*textures), PU_STATIC, NULL);
  texturecolumnlump = Z_Malloc(numtextures * sizeof(*texturecolumnlump), PU_STATIC, NULL);
  texturecolumnofs = Z_Malloc(numtextures * sizeof(*texturecolumnofs), PU_STATIC, NULL);
  texturecomposite = Z_Malloc(numtextures * sizeof(*texturecomposite), PU_STATIC, NULL);
  texturecompositesize = Z_Malloc(numtextures * sizeof(*texturecompositesize), PU_STATIC, NULL);
  texturewidthmask = Z_Malloc(numtextures * sizeof(*texturewidthmask), PU_STATIC, NULL);
  textureheight = Z_Malloc(numtextures * sizeof(*textureheight), PU_STATIC, NULL);

  totalwidth = 0;

  //  Really complex printing shit...
  temp1 = W_GetNumForName ("S_START");  // P_???????
  temp2 = W_GetNumForName ("S_END") - 1;
  temp3 = ((temp2 - temp1 + 63) / 64) + ((numtextures + 63) / 64);
  printf("[");
  for (i = 0; i < temp3; i++)
  {
    printf(" ");
  }
  printf("         ]");
  for (i = 0; i < temp3; i++)
  {
    printf("\x8");
  }
  printf("\x8\x8\x8\x8\x8\x8\x8\x8\x8\x8");

  for (i = 0 ; i < numtextures ; i++, directory++)
  {
    if (!(i & 63))
    {
      printf (".");
    }

    if (i == numtextures1)
    {
      // Start looking in second texture file.
      maptex = maptex2;
      maxoff = maxoff2;
      directory = maptex + 1;
    }

    offset = LONG(*directory);

    if (offset > maxoff)
    {
      I_Error ("R_InitTextures: bad texture directory");
    }

    mtexture = (MapTexture*) ( (uint8_t*)maptex + offset);

    texture = textures[i] =
                Z_Malloc (sizeof(texture_t)
                          + sizeof(texpatch_t) * (SHORT(mtexture->patchcount) - 1),
                          PU_STATIC, 0);

    texture->width = SHORT(mtexture->width);
    texture->height = SHORT(mtexture->height);
    texture->patchcount = SHORT(mtexture->patchcount);

    memcpy (texture->name, mtexture->name, sizeof(texture->name));
    mpatch = &mtexture->patches[0];
    patch = &texture->patches[0];

    for (j = 0 ; j < texture->patchcount ; j++, mpatch++, patch++)
    {
      patch->originx = SHORT(mpatch->originx);
      patch->originy = SHORT(mpatch->originy);
      patch->patch = patchlookup[SHORT(mpatch->patch)];
      if (patch->patch == -1)
      {
        I_Error ("R_InitTextures: Missing patch in texture %s",
                 texture->name);
      }
    }
    texturecolumnlump[i] = Z_Malloc(texture->width * sizeof(short), PU_STATIC, NULL);
    texturecolumnofs[i] = Z_Malloc(texture->width * sizeof(unsigned short), PU_STATIC, NULL);

    j = 1;
    while (j * 2 <= texture->width)
    {
      j <<= 1;
    }

    texturewidthmask[i] = j - 1;
    textureheight[i] = texture->height << FRACBITS;

    totalwidth += texture->width;
  }

  Z_Free (maptex1);
  if (maptex2)
  {
    Z_Free (maptex2);
  }

  // Precalculate whatever possible.
  for (i = 0 ; i < numtextures ; i++)
  {
    R_GenerateLookup (i);
  }

  // Create translation table for global animation.
  texturetranslation = Z_Malloc ((numtextures + 1) * sizeof(*texturetranslation), PU_STATIC, NULL);

  for (i = 0 ; i < numtextures ; i++)
  {
    texturetranslation[i] = i;
  }
}



//
// R_InitFlats
//
void R_InitFlats (void)
{
  int   i;

  firstflat = W_GetNumForName ("F_START") + 1;
  lastflat = W_GetNumForName ("F_END") - 1;
  numflats = lastflat - firstflat + 1;

  // Create translation table for global animation.
  flattranslation = Z_Malloc ((numflats + 1) * sizeof(*flattranslation), PU_STATIC, 0);

  for (i = 0 ; i < numflats ; i++)
  {
    flattranslation[i] = i;
  }
}


//
// R_InitSpriteLumps
// Finds the width and hoffset of all sprites in the wad,
//  so the sprite does not need to be cached completely
//  just for having the header info ready during rendering.
//
void R_InitSpriteLumps (void)
{
  int   i;
  patch_t* patch;

  firstspritelump = W_GetNumForName ("S_START") + 1;
  lastspritelump = W_GetNumForName ("S_END") - 1;

  numspritelumps = lastspritelump - firstspritelump + 1;
  spritewidth = Z_Malloc(numspritelumps * sizeof(*spritewidth), PU_STATIC, NULL);
  spriteoffset = Z_Malloc(numspritelumps * sizeof(*spriteoffset), PU_STATIC, NULL);
  spritetopoffset = Z_Malloc(numspritelumps * sizeof(*spritetopoffset), PU_STATIC, NULL);

  for (i = 0 ; i < numspritelumps ; i++)
  {
    if (!(i & 63))
    {
      printf (".");
    }

    patch = W_CacheLumpNum (firstspritelump + i, PU_CACHE);
    spritewidth[i] = SHORT(patch->width) << FRACBITS;
    spriteoffset[i] = SHORT(patch->leftoffset) << FRACBITS;
    spritetopoffset[i] = SHORT(patch->topoffset) << FRACBITS;
  }
}

void R_InitColormaps()
{
  const int lump = W_GetNumForName("COLORMAP");

  // Load in the light tables, 256 byte align tables.
  colormaps = (lighttable_t*) W_CacheLumpNum(lump, PU_STATIC);
}

//
// R_InitData
// Locates all the lumps
//  that will be used by all views
// Must be called after W_Init.
//
void R_InitData (void)
{
  R_InitTextures ();
  printf ("\nInitTextures");
  R_InitFlats ();
  printf ("\nInitFlats");
  R_InitSpriteLumps ();
  printf ("\nInitSprites");
  R_InitColormaps ();
  printf ("\nInitColormaps");
}

/**
 * Retrieval, get a flat number for a flat name.
 */
int R_FlatNumForName(const char* name)
{
  const int index = W_CheckNumForName(name);

  if (index < 0)
  {
    I_Error("R_FlatNumForName: %s not found", name);
  }

  return index - firstflat;
}

/**
 * Check whether texture is available. Filter out NoTexture indicator.
 */
int R_CheckTextureNumForName(const char* name)
{
  // "NoTexture" marker.
  if (name[0] == '-')
  {
    return 0;
  }
  for (int i = 0; i < numtextures; ++i)
  {
    if (!strncasecmp(textures[i]->name, name, 8))
    {
      return i;
    }
  }

  return -1;
}

/**
 * Calls R_CheckTextureNumForName, aborts with error message.
 */
int R_TextureNumForName(const char* name)
{
  const int index = R_CheckTextureNumForName(name);

  if (index < 0)
  {
    I_Error("R_TextureNumForName: %s not found", name);
  }

  return index;
}

//
// R_PrecacheLevel
// Preloads all relevant graphics for the level.
//
int   flatmemory;
int   texturememory;
int   spritememory;

void R_PrecacheLevel (void)
{
  char*   flatpresent;
  char*   texturepresent;
  char*   spritepresent;

  int     i;
  int     j;
  int     k;
  int     lump;

  texture_t*    texture;
  thinker_t*    th;
  SpriteFrame*  sf;

  if (demoplayback)
  {
    return;
  }

  // Precache flats.
  flatpresent = (char*) Z_Malloc(numflats, PU_STATIC, NULL);
  memset (flatpresent, 0, numflats);

  for (i = 0 ; i < numsectors ; i++)
  {
    flatpresent[sectors[i].floorpic] = 1;
    flatpresent[sectors[i].ceilingpic] = 1;
  }

  flatmemory = 0;

  for (i = 0 ; i < numflats ; i++)
  {
    if (flatpresent[i])
    {
      lump = firstflat + i;
      flatmemory += lumpinfo[lump].size;
      W_CacheLumpNum(lump, PU_CACHE);
    }
  }

  // Precache textures.
  texturepresent = (char*) Z_Malloc(numtextures, PU_STATIC, NULL);
  memset (texturepresent, 0, numtextures);

  for (i = 0 ; i < numsides ; i++)
  {
    texturepresent[sides[i].toptexture] = 1;
    texturepresent[sides[i].midtexture] = 1;
    texturepresent[sides[i].bottomtexture] = 1;
  }

  // Sky texture is always present.
  // Note that F_SKY1 is the name used to
  //  indicate a sky floor/ceiling as a flat,
  //  while the sky texture is stored like
  //  a wall texture, with an episode dependend
  //  name.
  texturepresent[skytexture] = 1;

  texturememory = 0;
  for (i = 0 ; i < numtextures ; i++)
  {
    if (!texturepresent[i])
    {
      continue;
    }

    texture = textures[i];

    for (j = 0 ; j < texture->patchcount ; j++)
    {
      lump = texture->patches[j].patch;
      texturememory += lumpinfo[lump].size;
      W_CacheLumpNum(lump , PU_CACHE);
    }
  }

  // Precache sprites.
  spritepresent = (char*) Z_Malloc(numsprites, PU_STATIC, NULL);
  memset (spritepresent, 0, numsprites);

  for (th = thinkercap.next ; th != &thinkercap ; th = th->next)
  {
    if (th->function.acp1 == (actionf_p1)P_MobjThinker)
    {
      spritepresent[((mobj_t*)th)->sprite] = 1;
    }
  }

  spritememory = 0;
  for (i = 0 ; i < numsprites ; i++)
  {
    if (!spritepresent[i])
    {
      continue;
    }

    for (j = 0 ; j < sprites[i].numframes ; j++)
    {
      sf = &sprites[i].spriteframes[j];
      for (k = 0 ; k < 8 ; k++)
      {
        lump = firstspritelump + sf->lump[k];
        spritememory += lumpinfo[lump].size;
        W_CacheLumpNum(lump , PU_CACHE);
      }
    }
  }
}




