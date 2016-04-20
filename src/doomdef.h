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
//  Internally used data structures for virtually everything,
//   key definitions, lots of other stuff.
//
//-----------------------------------------------------------------------------

#ifndef __DOOMDEF__
#define __DOOMDEF__

//
// Global parameters/defines.
//
// DOOM version
enum { VERSION =  110 };

/**
 * Game mode handling: Identify IWAD version to handle IWAD depended animations
 * etc.
 */
typedef enum
{
  /** DOOM 1 shareware, E1 M9. */
  GAME_MODE_SHAREWARE,
  /** DOOM 1 registered, E3, M27. */
  GAME_MODE_REGISTERED,
  /** DOOM 2 retail, E1 M34. */
  GAME_MODE_COMMERCIAL,
  /** DOOM 2 german edition not handled. */
  GAME_MODE_RETAIL,
  /** Well, no IWAD found. */
  GAME_MODE_INDETERMINED
} GameMode;

/**
 * Mission packs: Might be useful for TC stuff?
 */
typedef enum
{
  /** DOOM 1. */
  GAME_MISSION_DOOM,
  /** DOOM 2. */
  GAME_MISSION_DOOM2,
  /** TNT mission pack. */
  GAME_MISSION_PACK_TNT,
  /** Plutonia pack. */
  GAME_MISSION_PACK_PLUTONIA,
  GAME_MISSION_NONE
} GameMission;

/**
 * Identify language to use, software localization.
 */
typedef enum
{
  LANGUAGE_ENGLISH,
  LANGUAGE_FRENCH,
  LANGUAGE_GERMAN,
  LANGUAGE_UNKNOWN
} Language;

// If rangecheck is undefined,
// most parameter validation debugging code will not be compiled
#define RANGECHECK

//
// For resize of screen, at start of game.
// It will not work dynamically, see visplanes.
//
#define BASE_WIDTH    320

// It is educational but futile to change this
//  scaling e.g. to 2. Drawing of status bar,
//  menues etc. is tied to the scale implied
//  by the graphics.
#define SCREEN_MUL    1
#define INV_ASPECT_RATIO  0.625 // 0.75, ideally

// Defines suck. C sucks.
// C++ might sucks for OOP, but it sure is a better C.
// So there.
#define SCREENWIDTH  320
//SCREEN_MUL*BASE_WIDTH //320
#define SCREENHEIGHT 200
//(int)(SCREEN_MUL*BASE_WIDTH*INV_ASPECT_RATIO) //200




// The maximum number of players, multiplayer/networking.
#define MAXPLAYERS    4

// State updates, number of tics / second.
#define TICRATE   35

/**
 * The current state of the game: Whether we are playing, gazing at the
 * intermission screen, the game final animation or a demo.
 */
typedef enum
{
  GAME_STATE_LEVEL,
  GAME_STATE_INTERMISSION,
  GAME_STATE_FINALE,
  GAME_STATE_DEMOSCREEN
} GameState;

//
// Difficulty/skill settings/filters.
//

// Skill flags.
#define MTF_EASY    1
#define MTF_NORMAL    2
#define MTF_HARD    4

// Deaf monsters/do not react to sound.
#define MTF_AMBUSH    8

typedef enum
{
  sk_baby,
  sk_easy,
  sk_medium,
  sk_hard,
  sk_nightmare
} skill_t;




//
// Key cards.
//
typedef enum
{
  it_bluecard,
  it_yellowcard,
  it_redcard,
  it_blueskull,
  it_yellowskull,
  it_redskull,

  NUMCARDS

} card_t;



// The defined weapons,
//  including a marker indicating
//  user has not changed weapon.
typedef enum
{
  wp_fist,
  wp_pistol,
  wp_shotgun,
  wp_chaingun,
  wp_missile,
  wp_plasma,
  wp_bfg,
  wp_chainsaw,
  wp_supershotgun,

  NUMWEAPONS,

  // No pending weapon change.
  wp_nochange

} weapontype_t;


// Ammunition types defined.
typedef enum
{
  am_clip,  // Pistol / chaingun ammo.
  am_shell, // Shotgun / double barreled shotgun.
  am_cell,  // Plasma rifle, BFG.
  am_misl,  // Missile launcher.
  NUMAMMO,
  am_noammo // Unlimited for chainsaw / fist.

} ammotype_t;


// Power up artifacts.
typedef enum
{
  pw_invulnerability,
  pw_strength,
  pw_invisibility,
  pw_ironfeet,
  pw_allmap,
  pw_infrared,
  NUMPOWERS

} powertype_t;



//
// Power up durations,
//  how many seconds till expiration,
//  assuming TICRATE is 35 ticks/second.
//
typedef enum
{
  INVULNTICS  = (30 * TICRATE),
  INVISTICS = (60 * TICRATE),
  INFRATICS = (120 * TICRATE),
  IRONTICS  = (60 * TICRATE)

} powerduration_t;




//
// DOOM keyboard definition.
// This is the stuff configured by Setup.Exe.
// Most key data are simple ascii (uppercased).
//
#define KEY_RIGHTARROW  0xae
#define KEY_LEFTARROW 0xac
#define KEY_UPARROW 0xad
#define KEY_DOWNARROW 0xaf
#define KEY_ESCAPE  27
#define KEY_ENTER 13
#define KEY_TAB   9
#define KEY_F1    (0x80+0x3b)
#define KEY_F2    (0x80+0x3c)
#define KEY_F3    (0x80+0x3d)
#define KEY_F4    (0x80+0x3e)
#define KEY_F5    (0x80+0x3f)
#define KEY_F6    (0x80+0x40)
#define KEY_F7    (0x80+0x41)
#define KEY_F8    (0x80+0x42)
#define KEY_F9    (0x80+0x43)
#define KEY_F10   (0x80+0x44)
#define KEY_F11   (0x80+0x57)
#define KEY_F12   (0x80+0x58)

#define KEY_BACKSPACE 127
#define KEY_PAUSE 0xff

#define KEY_EQUALS  0x3d
#define KEY_MINUS 0x2d

#define KEY_RSHIFT  (0x80+0x36)
#define KEY_RCTRL (0x80+0x1d)
#define KEY_RALT  (0x80+0x38)

#define KEY_LALT  KEY_RALT

// New keys:
#define KEY_CAPSLOCK    (0x80+0x3a)
#define KEY_SCRLCK      (0x80+0x46)

#define KEY_HOME        (0x80+0x47)
#define KEY_END         (0x80+0x4f)
#define KEY_PGUP        (0x80+0x49)
#define KEY_PGDN        (0x80+0x51)
#define KEY_INS         (0x80+0x52)
#define KEY_DEL         (0x80+0x53)

#define KEYP_0          0
#define KEYP_1          KEY_END
#define KEYP_2          KEY_DOWNARROW
#define KEYP_3          KEY_PGDN
#define KEYP_4          KEY_LEFTARROW
#define KEYP_5          '5'
#define KEYP_6          KEY_RIGHTARROW
#define KEYP_7          KEY_HOME
#define KEYP_8          KEY_UPARROW
#define KEYP_9          KEY_PGUP

#define KEYP_DIVIDE     '/'
#define KEYP_PLUS       '+'
#define KEYP_MINUS      '-'
#define KEYP_MULTIPLY   '*'
#define KEYP_PERIOD     0
#define KEYP_EQUALS     KEY_EQUALS
#define KEYP_ENTER      KEY_ENTER

#endif          // __DOOMDEF__
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
