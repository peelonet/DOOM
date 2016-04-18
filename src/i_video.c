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
//  DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include <SDL/SDL.h>

#include "d_main.h"
#include "i_system.h"
#include "v_video.h"

static boolean initialized = false;
static SDL_Surface* window;
static SDL_Color palette_out[256];

//
//  Translates the key currently in X_event
//

int xlatekey(SDL_keysym* key)
{
  int rc;

  switch (key->sym)
  {
  case XK_Left:
    rc = KEY_LEFTARROW;
    break;
  case XK_Right:
    rc = KEY_RIGHTARROW;
    break;
  case XK_Down:
    rc = KEY_DOWNARROW;
    break;
  case XK_Up:
    rc = KEY_UPARROW;
    break;
  case XK_Escape:
    rc = KEY_ESCAPE;
    break;
  case XK_Return:
    rc = KEY_ENTER;
    break;
  case XK_Tab:
    rc = KEY_TAB;
    break;
  case XK_F1:
    rc = KEY_F1;
    break;
  case XK_F2:
    rc = KEY_F2;
    break;
  case XK_F3:
    rc = KEY_F3;
    break;
  case XK_F4:
    rc = KEY_F4;
    break;
  case XK_F5:
    rc = KEY_F5;
    break;
  case XK_F6:
    rc = KEY_F6;
    break;
  case XK_F7:
    rc = KEY_F7;
    break;
  case XK_F8:
    rc = KEY_F8;
    break;
  case XK_F9:
    rc = KEY_F9;
    break;
  case XK_F10:
    rc = KEY_F10;
    break;
  case XK_F11:
    rc = KEY_F11;
    break;
  case XK_F12:
    rc = KEY_F12;
    break;

  case XK_BackSpace:
  case XK_Delete:
    rc = KEY_BACKSPACE;
    break;

  case XK_Pause:
    rc = KEY_PAUSE;
    break;

  case XK_KP_Equal:
  case XK_equal:
    rc = KEY_EQUALS;
    break;

  case XK_KP_Subtract:
  case XK_minus:
    rc = KEY_MINUS;
    break;

  case XK_Shift_L:
  case XK_Shift_R:
    rc = KEY_RSHIFT;
    break;

  case XK_Control_L:
  case XK_Control_R:
    rc = KEY_RCTRL;
    break;

  case XK_Alt_L:
  case XK_Meta_L:
  case XK_Alt_R:
  case XK_Meta_R:
    rc = KEY_RALT;
    break;

  default:
    if (rc >= XK_space && rc <= XK_asciitilde)
    {
      rc = rc - XK_space + ' ';
    }
    if (rc >= 'A' && rc <= 'Z')
    {
      rc = rc - 'A' + 'a';
    }
    break;
  }

  return rc;

}

void I_ShutdownGraphics()
{
  if (initialized)
  {
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    initialized = false;
  }
}

//
// I_StartFrame
//
void I_StartFrame (void)
{
  // er?

}

static int  lastmousex = 0;
static int  lastmousey = 0;
boolean   mousemoved = false;
boolean   shmFinished;

static void I_GetEvent()
{
  SDL_Event sdl_event;
  event_t doom_event;

  while (SDL_PollEvent(&sdl_event))
  {
    switch (sdl_event.type)
    {
    case SDL_KEYDOWN:
      doom_event.type = ev_keydown;
      doom_event.data1 = xlatekey(&sdl_event.key.keysym);
      D_PostEvent(&doom_event);
      break;

    case SDL_KEYUP:
      doom_event.type = ev_keyup;
      doom_event.data1 = xlatekey(&sdl_event.key.keysym);
      D_PostEvent(&doom_event);
      break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
      doom_event.type = ev_mouse;
      doom_event.data1 = sdl_event.button.button;
      doom_event.data2 = 0;
      doom_event.data3 = 0;
      D_PostEvent(&doom_event);
      break;

    case SDL_MOUSEMOTION:
      doom_event.type = ev_mouse;
      doom_event.data1 = sdl_event.button.button;
      doom_event.data2 = (sdl_event.motion.xrel << 2);
      doom_event.data3 = ((0 - sdl_event.motion.yrel) << 2);
      if (doom_event.data2 || doom_event.data3)
      {
        if (sdl_event.motion.x != SCREENWIDTH / 2 &&
            sdl_event.motion.y != SCREENHEIGHT / 2)
        {
          D_PostEvent(&doom_event);
        }
      }
      break;

    case SDL_QUIT:
      I_Quit();
      break;
    }
  }
}

//
// I_StartTic
//
void I_StartTic()
{
  if (initialized)
  {
    I_GetEvent();
  }
}

//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
  // what is this?
}

//
// I_FinishUpdate
//
void I_FinishUpdate()
{
  if (!initialized || !(SDL_GetAppState() & SDL_APPACTIVE))
  {
    return;
  }

  SDL_LockSurface(window);
  memcpy(window->pixels, screens[0], SCREENWIDTH * SCREENHEIGHT);
  SDL_UnlockSurface(window);

  SDL_Flip(window);
}

//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
  memcpy (scr, screens[0], SCREENWIDTH * SCREENHEIGHT);
}

//
// I_SetPalette
//
void I_SetPalette(const byte* palette)
{
  int i;

  for (i = 0; i < 256; ++i)
  {
    palette_out[i].r = *palette++;
    palette_out[i].g = *palette++;
    palette_out[i].b = *palette++;
  }

  SDL_SetColors(window, palette_out, 0, 256);
}

void I_InitGraphics()
{
  if (initialized)
  {
    return;
  }

  // Needs SDL_INIT_TIMER?
  if (SDL_Init(SDL_INIT_VIDEO))
  {
    I_Error("SDL_Init: %s\n", SDL_GetError());
  }

  // 8-bit RGBA and double buffering.
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 8);

  // Use OpenGL.
  window = SDL_SetVideoMode(
    SCREENWIDTH,
    SCREENHEIGHT,
    8,
    SDL_SWSURFACE | SDL_DOUBLEBUF
  );

  if (!window)
  {
    I_Error("SDL_SetVideoMode: %s\n", SDL_GetError());
  }

  // Grab the mouse and hide the cursor.
  SDL_WM_GrabInput(SDL_GRAB_ON);
  SDL_ShowCursor(0);

  SDL_WM_SetCaption("Doom", 0);

  screens[0] = (unsigned char*) malloc(SCREENWIDTH * SCREENHEIGHT);

  initialized = true;
}
