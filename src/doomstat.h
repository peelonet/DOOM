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
//   All the global variables that store the internal state.
//   Theoretically speaking, the internal state of the engine
//    should be found by looking at the variables collected
//    here, and every relevant module will have to include
//    this header file.
//   In practice, things are a bit messy.
//
//-----------------------------------------------------------------------------
#ifndef __D_STATE__
#define __D_STATE__

// We need globally shared data structures,
//  for defining the global state variables.
#include "doomdata.h"
#include "d_net.h"

// We need the playr data structure as well.
#include "d_player.h"

// ------------------------
// Command line parameters.
//
extern  bool nomonsters; // checkparm of -nomonsters
extern  bool respawnparm;  // checkparm of -respawn
extern  bool fastparm; // checkparm of -fast

extern  bool devparm;  // DEBUG: launched with -devparm



// -----------------------------------------------------
// Game Mode - identify IWAD as shareware, retail etc.
//
extern GameMode gamemode;
extern GameMission gamemission;

// Set if homebrew PWAD stuff has been added.
extern  bool modifiedgame;


// -------------------------------------------
// Language.
extern Language language;


// -------------------------------------------
// Selected skill type, map etc.
//

// Defaults for menu, methinks.
extern Skill startskill;
extern  int             startepisode;
extern  int   startmap;

extern  bool   autostart;

// Selected by user.
extern Skill gameskill;
extern  int   gameepisode;
extern  int   gamemap;

// Nightmare mode flag, single player.
extern  bool         respawnmonsters;

// Netgame? Only true if >1 player.
extern  bool netgame;

// Flag: true only if started as net deathmatch.
// An enum might handle altdeath/cooperative better.
extern int deathmatch;

// -------------------------
// Internal parameters for sound rendering.
// These have been taken from the DOS version,
//  but are not (yet) supported with Linux
//  (e.g. no sound volume adjustment with menu.

// These are not used, but should be (menu).
// From m_menu.c:
//  Sound FX volume has default, 0 - 15
//  Music volume has default, 0 - 15
// These are multiplied by 8.
extern int snd_SfxVolume;      // maximum volume for sound
extern int snd_MusicVolume;    // maximum volume for music

// Current music/sfx card - index useless
//  w/o a reference LUT in a sound module.
// Ideally, this would use indices found
//  in: /usr/include/linux/soundcard.h
extern int snd_MusicDevice;
extern int snd_SfxDevice;
// Config file? Same disclaimer as above.
extern int snd_DesiredMusicDevice;
extern int snd_DesiredSfxDevice;


// -------------------------
// Status flags for refresh.
//

// Depending on view size - no status bar?
// Note that there is no way to disable the
//  status bar explicitely.
extern  bool statusbaractive;

extern  bool automapactive;  // In AutoMap mode?
extern  bool menuactive; // Menu overlayed?
extern  bool paused;   // Game Pause?


extern  bool   viewactive;

extern  bool   nodrawers;
extern  bool   noblit;

extern  int   viewwindowx;
extern  int   viewwindowy;
extern  int   viewheight;
extern  int   viewwidth;
extern  int   scaledviewwidth;






// This one is related to the 3-screen display mode.
// ANG90 = left side, ANG270 = right
extern  int viewangleoffset;

// Player taking events, and displaying.
extern  int consoleplayer;
extern  int displayplayer;


// -------------------------------------
// Scores, rating.
// Statistics on a given map, for intermission.
//
extern  int totalkills;
extern  int totalitems;
extern  int totalsecret;

// Timer, for scores.
extern  int levelstarttic;  // gametic at level start
extern  int leveltime;  // tics in game play for par



// --------------------------------------
// DEMO playback/recording related stuff.
// No demo, there is a human player in charge?
// Disable save/end game?
extern  bool usergame;

//?
extern  bool demoplayback;
extern  bool demorecording;

// Quit after playing a demo from cmdline.
extern  bool   singledemo;

extern GameState gamestate;

//-----------------------------
// Internal parameters, fixed.
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.



extern  int   gametic;


// Bookkeeping on players - state.
extern  player_t  players[MAXPLAYERS];

// Alive? Disconnected?
extern  bool   playeringame[MAXPLAYERS];


// Player spawn spots for deathmatch.
#define MAX_DM_STARTS   10
extern  mapthing_t      deathmatchstarts[MAX_DM_STARTS];
extern  mapthing_t* deathmatch_p;

// Player spawn spots.
extern  mapthing_t      playerstarts[MAXPLAYERS];

// Intermission stats.
// Parameters for world map / intermission.
extern  wbstartstruct_t   wminfo;


// LUT of ammunition limits for each kind.
// This doubles with BackPack powerup item.
extern  int   maxammo[NUMAMMO];





//-----------------------------------------
// Internal parameters, used for engine.
//

// File handling stuff.
extern  char    basedefault[1024];

// if true, load all graphics at level load
extern  bool         precache;


// wipegamestate can be set to -1
//  to force a wipe on the next draw
extern GameState wipegamestate;

extern  int             mouseSensitivity;
//?
// debug flag to cancel adaptiveness
extern  bool         singletics;

extern  int             bodyqueslot;



// Needed to store the number of the dummy sky flat.
// Used for rendering,
//  as well as tracking projectiles etc.
extern int    skyflatnum;



// Netgame stuff (buffers and pointers, i.e. indices).

// This is ???
extern  doomcom_t*  doomcom;

// This points inside doomcom.
extern  doomdata_t* netbuffer;


extern  ticcmd_t  localcmds[BACKUPTICS];
extern  int   rndindex;

extern  int   maketic;
extern  int             nettics[MAXNETNODES];

extern  ticcmd_t        netcmds[MAXPLAYERS][BACKUPTICS];
extern  int   ticdup;



#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
