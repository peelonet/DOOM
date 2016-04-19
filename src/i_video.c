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
#include <stdbool.h>
#include <ctype.h>

#include <SDL/SDL.h>

#include "d_main.h"
#include "i_system.h"
#include "v_video.h"

static bool initialized = false;
static SDL_Surface* window;
static SDL_Color palette_out[256];

/**
 * Translates SDL key symbol.
 */
static int translate_key(SDL_keysym* key)
{
  switch (key->sym)
  {
  case SDLK_BACKSPACE:
    return KEY_BACKSPACE;
  case SDLK_TAB:
    return KEY_TAB;
  case SDLK_PAUSE:
    return KEY_PAUSE;
  case SDLK_ESCAPE:
    return KEY_ESCAPE;
  case SDLK_MINUS:
    return KEY_MINUS;

  case SDLK_INSERT:
    return KEY_INS;
  case SDLK_HOME:
    return KEY_HOME;
  case SDLK_END:
    return KEY_END;
  case SDLK_PAGEUP:
    return KEY_PGUP;
  case SDLK_PAGEDOWN:
    return KEY_PGDN;
  case SDLK_EQUALS:
    return KEY_EQUALS;
  case SDLK_UP:
    return KEY_UPARROW;
  case SDLK_DOWN:
    return KEY_DOWNARROW;
  case SDLK_RIGHT:
    return KEY_RIGHTARROW;
  case SDLK_LEFT:
    return KEY_LEFTARROW;

  case SDLK_F1:
    return KEY_F1;
  case SDLK_F2:
    return KEY_F2;
  case SDLK_F3:
    return KEY_F3;
  case SDLK_F4:
    return KEY_F4;
  case SDLK_F5:
    return KEY_F5;
  case SDLK_F6:
    return KEY_F6;
  case SDLK_F7:
    return KEY_F7;
  case SDLK_F8:
    return KEY_F8;
  case SDLK_F9:
    return KEY_F9;
  case SDLK_F10:
    return KEY_F10;
  case SDLK_F11:
    return KEY_F11;
  case SDLK_F12:
    return KEY_F12;

  case SDLK_LSHIFT:
  case SDLK_RSHIFT:
    return KEY_RSHIFT;

  case SDLK_LCTRL:
  case SDLK_RCTRL:
    return KEY_RCTRL;

  case SDLK_LALT:
  case SDLK_RALT:
  case SDLK_LMETA:
  case SDLK_RMETA:
    return KEY_RALT;

  case SDLK_KP0:
    return KEYP_0;
  case SDLK_KP1:
    return KEYP_1;
  case SDLK_KP2:
    return KEYP_2;
  case SDLK_KP3:
    return KEYP_3;
  case SDLK_KP4:
    return KEYP_4;
  case SDLK_KP5:
    return KEYP_5;
  case SDLK_KP6:
    return KEYP_6;
  case SDLK_KP7:
    return KEYP_7;
  case SDLK_KP8:
    return KEYP_8;
  case SDLK_KP9:
    return KEYP_9;
  case SDLK_KP_PERIOD:
    return KEYP_PERIOD;
  case SDLK_KP_DIVIDE:
    return KEYP_DIVIDE;
  case SDLK_KP_MULTIPLY:
    return KEYP_MULTIPLY;
  case SDLK_KP_MINUS:
    return KEYP_MINUS;
  case SDLK_KP_PLUS:
    return KEYP_PLUS;
  case SDLK_KP_ENTER:
    return KEYP_ENTER;
  case SDLK_KP_EQUALS:
    return KEYP_EQUALS;

  case SDLK_CAPSLOCK:
    return KEY_CAPSLOCK;
  case SDLK_SCROLLOCK:
    return KEY_SCRLCK;

  default:
    return tolower(key->sym);
  }
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

bool   mousemoved = false;
bool   shmFinished;

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
      doom_event.data1 = translate_key(&sdl_event.key.keysym);
      D_PostEvent(&doom_event);
      break;

    case SDL_KEYUP:
      doom_event.type = ev_keyup;
      doom_event.data1 = translate_key(&sdl_event.key.keysym);
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
void I_ReadScreen (uint8_t* scr)
{
  memcpy (scr, screens[0], SCREENWIDTH * SCREENHEIGHT);
}

//
// I_SetPalette
//
void I_SetPalette(const uint8_t* palette)
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
