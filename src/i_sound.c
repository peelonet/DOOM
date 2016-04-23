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
//  System interface for sound.
//
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "doomdef.h"
#include "doomstat.h"
#include "sounds.h"
#include "w_wad.h"

//
// SFX API
// Note: this was called by S_Init.
// However, whatever they did in the
// old DPMS based DOS version, this
// were simply dummies in the Linux
// version.
// See soundserver initdata().
//
void I_SetChannels()
{
  // TODO: Implement with SDL_mixer.
}


void I_SetSfxVolume(int volume)
{
  // Identical to DOS.
  // Basically, this should propagate
  //  the menu/config file setting
  //  to the state variable used in
  //  the mixing.
  snd_SfxVolume = volume;
}

// MUSIC API - dummy. Some code from DOS version.
void I_SetMusicVolume(int volume)
{
  // Internal state variable.
  snd_MusicVolume = volume;
  // Now set volume on output device.
  // Whatever( snd_MusciVolume );
}


//
// Retrieve the raw data lump index
//  for a given SFX name.
//
int I_GetSfxLumpNum(sfxinfo_t* sfx)
{
  char namebuf[9];
  sprintf(namebuf, "ds%s", sfx->name);
  return W_GetNumForName(namebuf);
}

//
// Starting a sound means adding it
//  to the current list of active sounds
//  in the internal channels.
// As the SFX info struct contains
//  e.g. a pointer to the raw data,
//  it is ignored.
// As our sound handling does not handle
//  priority, it is ignored.
// Pitching (that is, increased speed of playback)
//  is set, but currently not used by mixing.
//
int I_StartSound (int id, int vol, int sep, int pitch, int priority)
{
  return -1; // TODO: Implement with SDL_mixer.
}

void I_StopSound(int handle)
{
  // TODO: Implement with SDL_mixer.
}

int I_SoundIsPlaying(int handle)
{
  return 0; // TODO: Implement with SDL_mixer.
}

//
// This function loops all active (internal) sound
//  channels, retrieves a given number of samples
//  from the raw sound data, modifies it according
//  to the current (internal) channel parameters,
//  mixes the per channel samples into the global
//  mixbuffer, clamping it to the allowed range,
//  and sets up everything for transferring the
//  contents of the mixbuffer to the (two)
//  hardware channels (left and right, that is).
//
// This function currently supports only 16bit.
//
void I_UpdateSound()
{
  // TODO: Implement with SDL_mixer.
}


//
// This would be used to write out the mixbuffer
//  during each game loop update.
// Updates sound buffer and audio device at runtime.
// It is called during Timer interrupt with SNDINTR.
// Mixing now done synchronous, and
//  only output be done asynchronous?
//
void I_SubmitSound()
{
  // TODO: Implement with SDL_mixer.
}

void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
  // TODO: Implement with SDL_mixer.
}

void I_ShutdownSound()
{
  // TODO: Implement with SDL_mixer.
}

void I_InitSound()
{
  // TODO: Implement with SDL_mixer.
}

//
// MUSIC API.
// Still no music done.
// Remains. Dummies.
//

void I_InitMusic()
{
  // TODO: Implement with SDL_mixer.
}

void I_ShutdownMusic()
{
  // TODO: Implement with SDL_mixer.
}

void I_PlaySong(int handle, int looping)
{
  // TODO: Implement with SDL_mixer.
}

void I_PauseSong(int handle)
{
  // TODO: Implement with SDL_mixer.
}

void I_ResumeSong(int handle)
{
  // TODO: Implement with SDL_mixer.
}

void I_StopSong(int handle)
{
  // TODO: Implement with SDL_mixer.
}

void I_UnRegisterSong(int handle)
{
  // TODO: Implement with SDL_mixer.
}

int I_RegisterSong(void* data)
{
  return 1; // TODO: Implement with SDL_mixer.
}
