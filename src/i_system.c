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
//
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include <sys/time.h>
#include <unistd.h>

#include "doomdef.h"
#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"

#include "d_net.h"
#include "g_game.h"

#include "i_system.h"




int mb_used = 6;


void
I_Tactile
( int on,
  int off,
  int total )
{
  // UNUSED.
  on = off = total = 0;
}

ticcmd_t  emptycmd;
ticcmd_t* I_BaseTiccmd(void)
{
  return &emptycmd;
}


int  I_GetHeapSize (void)
{
  return mb_used * 1024 * 1024;
}

uint8_t* I_ZoneBase (int*  size)
{
  *size = mb_used * 1024 * 1024;
  return (uint8_t*) malloc (*size);
}



//
// I_GetTime
// returns time in 1/70th second tics
//
int  I_GetTime (void)
{
  struct timeval  tp;
  struct timezone tzp;
  int     newtics;
  static int    basetime = 0;

  gettimeofday(&tp, &tzp);
  if (!basetime)
  {
    basetime = tp.tv_sec;
  }
  newtics = (tp.tv_sec - basetime) * TICRATE + tp.tv_usec * TICRATE / 1000000;
  return newtics;
}



//
// I_Init
//
void I_Init (void)
{
  I_InitSound();
  //  I_InitGraphics();
}

//
// I_Quit
//
void I_Quit (void)
{
  D_QuitNetGame ();
  I_ShutdownSound();
  I_ShutdownMusic();
  M_SaveDefaults ();
  I_ShutdownGraphics();
  exit(0);
}

void I_WaitVBL(int count)
{
#ifdef SGI
  sginap(1);
#else
#ifdef SUN
  sleep(0);
#else
  usleep (count * (1000000 / 70) );
#endif
#endif
}

void I_Error(const char* format, ...)
{
  va_list ap;

  // Message first.
  va_start(ap, format);
  fprintf(stderr, "Error: ");
  vfprintf(stderr, format, ap);
  fprintf(stderr, "\n");
  va_end(ap);

  fflush(stderr);

  // Shutdown. Here might be other errors.
  if (demorecording)
  {
    G_CheckDemoStatus();
  }

  D_QuitNetGame();
  I_ShutdownGraphics();

#if defined(DEBUG)
  abort();
#else
  exit(EXIT_FAILURE);
#endif
}
