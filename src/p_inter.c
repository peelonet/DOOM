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
//  Handling interactions (i.e., collisions).
//
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#include "doomdef.h"
#include "dstrings.h"
#include "sounds.h"

#include "doomstat.h"

#include "m_random.h"
#include "i_system.h"

#include "am_map.h"

#include "r_defs.h"
#include "r_main.h"
#include "p_local.h"

#include "s_sound.h"

#define BONUSADD  6

/*
 * Power up durations, how many seconds until expiration, assuming TICRATE is
 * 35 ticks/seconds.
 */
static const int POWER_DURATION_INVULNERABILITY = (30 * TICRATE);
static const int POWER_DURATION_INVISIBILITY = (60 * TICRATE);
static const int POWER_DURATION_INFRARED = (120 * TICRATE);
static const int POWER_DURATION_IRONFEET = (60 * TICRATE);

// a weapon is found with two clip loads,
// a big item has five clip loads
int maxammo[NUMAMMO] = {200, 50, 300, 50};
int clipammo[NUMAMMO] = {10, 4, 20, 1};


//
// GET STUFF
//

/**
 * Amount is the number of clip loads, not the invidivual count (0= 1/2 clip).
 * Returns false if the ammo cannot be picked up at all.
 */
static bool P_GiveAmmo(player_t* player, AmmoType type, int amount)
{
  int oldamount;

  if (type == AMMO_TYPE_NOAMMO)
  {
    return false;
  }

  if (!type || type > NUMAMMO)
  {
    I_Error("P_GiveAmmo: bad type %d", type);
  }

  if (player->ammo[type] == player->maxammo[type])
  {
    return false;
  }

  if (amount)
  {
    amount *= clipammo[type];
  } else {
    amount = clipammo[type] / 2;
  }

  if (gameskill == SKILL_BABY || gameskill == SKILL_NIGHTMARE)
  {
    // Give double ammo in trainer mode, you'll need it in nightmare. */
    amount <<= 1;
  }

  oldamount = player->ammo[amount];
  player->ammo[type] += amount;

  if (player->ammo[type] > player->maxammo[type])
  {
    player->ammo[type] = player->maxammo[type];
  }

  // If non zero ammo, don't change up weapons, player was lower on purpose.
  if (oldamount)
  {
    return true;
  }

  // We were down to zero, so select a new weapon. Preferences are not user
  // selectable. (Yet.)
  switch (type)
  {
  case AMMO_TYPE_CLIP:
    if (player->readyweapon == WEAPON_TYPE_FIST)
    {
      if (player->weaponowned[WEAPON_TYPE_CHAINGUN])
      {
        player->pendingweapon = WEAPON_TYPE_CHAINGUN;
      }
      else
      {
        player->pendingweapon = WEAPON_TYPE_PISTOL;
      }
    }
    break;

  case AMMO_TYPE_SHELL:
    if (player->readyweapon == WEAPON_TYPE_FIST
        || player->readyweapon == WEAPON_TYPE_PISTOL)
    {
      if (player->weaponowned[WEAPON_TYPE_SHOTGUN])
      {
        player->pendingweapon = WEAPON_TYPE_SHOTGUN;
      }
    }
    break;

  case AMMO_TYPE_CELL:
    if (player->readyweapon == WEAPON_TYPE_FIST
        || player->readyweapon == WEAPON_TYPE_PISTOL)
    {
      if (player->weaponowned[WEAPON_TYPE_PLASMA])
      {
        player->pendingweapon = WEAPON_TYPE_PLASMA;
      }
    }
    break;

  case AMMO_TYPE_MISSILE:
    if (player->readyweapon == WEAPON_TYPE_FIST)
    {
      if (player->weaponowned[WEAPON_TYPE_MISSILE])
      {
        player->pendingweapon = WEAPON_TYPE_MISSILE;
      }
    }
  default:
    break;
  }

  return true;
}

/**
 * The weapon name may have a MF_DROPPED flag stored in.
 */
static bool P_GiveWeapon(player_t* player, WeaponType weapon, bool dropped)
{
  bool gaveammo = false;
  bool gaveweapon = false;

  if (netgame && deathmatch != 2 && !dropped)
  {
    // Leave placed weapons forever on net games.
    if (player->weaponowned[weapon])
    {
      return false;
    }

    player->bonuscount += BONUSADD;
    player->weaponowned[weapon] = true;

    if (deathmatch)
    {
      P_GiveAmmo(player, weaponinfo[weapon].ammo, 5);
    }
    else
    {
      P_GiveAmmo(player, weaponinfo[weapon].ammo, 2);
    }
    player->pendingweapon = weapon;

    if (player == &players[consoleplayer])
    {
      S_StartSound(NULL, sfx_wpnup);
    }

    return false;
  }

  if (weaponinfo[weapon].ammo != AMMO_TYPE_NOAMMO)
  {
    // Give one clip with a dropped weapon, two clips with a found weapon.
    if (dropped)
    {
      gaveammo = P_GiveAmmo(player, weaponinfo[weapon].ammo, 1);
    }
    else
    {
      gaveammo = P_GiveAmmo(player, weaponinfo[weapon].ammo, 2);
    }
  }

  if (!player->weaponowned[weapon])
  {
    gaveweapon = true;
    player->weaponowned[weapon] = true;
    player->pendingweapon = weapon;
  }

  return gaveweapon || gaveammo;
}

//
// P_GiveBody
// Returns false if the body isn't needed at all
//
bool
P_GiveBody
( player_t* player,
  int   num )
{
  if (player->health >= MAXHEALTH)
  {
    return false;
  }

  player->health += num;
  if (player->health > MAXHEALTH)
  {
    player->health = MAXHEALTH;
  }
  player->mo->health = player->health;

  return true;
}



//
// P_GiveArmor
// Returns false if the armor is worse
// than the current armor.
//
bool
P_GiveArmor
( player_t* player,
  int   armortype )
{
  int   hits;

  hits = armortype * 100;
  if (player->armorpoints >= hits)
  {
    return false;  // don't pick up
  }

  player->armortype = armortype;
  player->armorpoints = hits;

  return true;
}

static void P_GiveCard(player_t* player, Card card)
{
  if (!player->cards[card])
  {
    player->bonuscount = BONUSADD;
    player->cards[card] = true;
  }
}

bool P_GivePower(player_t* player, PowerType power)
{
  if (power == POWER_TYPE_INVULNERABILITY)
  {
    player->powers[power] = POWER_DURATION_INVULNERABILITY;

    return true;
  }

  if (power == POWER_TYPE_INVISIBILITY)
  {
    player->powers[power] = POWER_DURATION_INVISIBILITY;
    player->mo->flags |= MF_SHADOW;

    return true;
  }

  if (power == POWER_TYPE_INFRARED)
  {
    player->powers[power] = POWER_DURATION_INFRARED;

    return true;
  }

  if (power == POWER_TYPE_IRONFEET)
  {
    player->powers[power] = POWER_DURATION_IRONFEET;

    return true;
  }

  if (power == POWER_TYPE_STRENGTH)
  {
    P_GiveBody (player, 100);
    player->powers[power] = 1;

    return true;
  }

  if (player->powers[power])
  {
    return false;  // already got it
  }

  player->powers[power] = 1;

  return true;
}

//
// P_TouchSpecialThing
//
void
P_TouchSpecialThing
( mobj_t* special,
  mobj_t* toucher )
{
  player_t* player;
  int   i;
  fixed_t delta;
  int   sound;

  delta = special->z - toucher->z;

  if (delta > toucher->height
      || delta < -8 * FRACUNIT)
  {
    // out of reach
    return;
  }


  sound = sfx_itemup;
  player = toucher->player;

  // Dead thing touching.
  // Can happen with a sliding player corpse.
  if (toucher->health <= 0)
  {
    return;
  }

  // Identify by sprite.
  switch (special->sprite)
  {
  // armor
  case SPR_ARM1:
    if (!P_GiveArmor (player, 1))
    {
      return;
    }
    player->message = GOTARMOR;
    break;

  case SPR_ARM2:
    if (!P_GiveArmor (player, 2))
    {
      return;
    }
    player->message = GOTMEGA;
    break;

  // bonus items
  case SPR_BON1:
    player->health++;   // can go over 100%
    if (player->health > 200)
    {
      player->health = 200;
    }
    player->mo->health = player->health;
    player->message = GOTHTHBONUS;
    break;

  case SPR_BON2:
    player->armorpoints++;    // can go over 100%
    if (player->armorpoints > 200)
    {
      player->armorpoints = 200;
    }
    if (!player->armortype)
    {
      player->armortype = 1;
    }
    player->message = GOTARMBONUS;
    break;

  case SPR_SOUL:
    player->health += 100;
    if (player->health > 200)
    {
      player->health = 200;
    }
    player->mo->health = player->health;
    player->message = GOTSUPER;
    sound = sfx_getpow;
    break;

  case SPR_MEGA:
    if (gamemode != GAME_MODE_COMMERCIAL)
    {
      return;
    }
    player->health = 200;
    player->mo->health = player->health;
    P_GiveArmor (player, 2);
    player->message = GOTMSPHERE;
    sound = sfx_getpow;
    break;

  // cards
  // leave cards for everyone
  case SPR_BKEY:
    if (!player->cards[CARD_BLUE])
    {
      player->message = GOTBLUECARD;
    }
    P_GiveCard (player, CARD_BLUE);
    if (!netgame)
    {
      break;
    }
    return;

  case SPR_YKEY:
    if (!player->cards[CARD_YELLOW])
    {
      player->message = GOTYELWCARD;
    }
    P_GiveCard (player, CARD_YELLOW);
    if (!netgame)
    {
      break;
    }
    return;

  case SPR_RKEY:
    if (!player->cards[CARD_RED])
    {
      player->message = GOTREDCARD;
    }
    P_GiveCard (player, CARD_RED);
    if (!netgame)
    {
      break;
    }
    return;

  case SPR_BSKU:
    if (!player->cards[CARD_BLUE_SKULL])
    {
      player->message = GOTBLUESKUL;
    }
    P_GiveCard (player, CARD_BLUE_SKULL);
    if (!netgame)
    {
      break;
    }
    return;

  case SPR_YSKU:
    if (!player->cards[CARD_YELLOW_SKULL])
    {
      player->message = GOTYELWSKUL;
    }
    P_GiveCard (player, CARD_YELLOW_SKULL);
    if (!netgame)
    {
      break;
    }
    return;

  case SPR_RSKU:
    if (!player->cards[CARD_RED_SKULL])
    {
      player->message = GOTREDSKULL;
    }
    P_GiveCard (player, CARD_RED_SKULL);
    if (!netgame)
    {
      break;
    }
    return;

  // medikits, heals
  case SPR_STIM:
    if (!P_GiveBody (player, 10))
    {
      return;
    }
    player->message = GOTSTIM;
    break;

  case SPR_MEDI:
    if (!P_GiveBody (player, 25))
    {
      return;
    }

    if (player->health < 25)
    {
      player->message = GOTMEDINEED;
    }
    else
    {
      player->message = GOTMEDIKIT;
    }
    break;


  // power ups
  case SPR_PINV:
    if (!P_GivePower (player, POWER_TYPE_INVULNERABILITY))
    {
      return;
    }
    player->message = GOTINVUL;
    sound = sfx_getpow;
    break;

  case SPR_PSTR:
    if (!P_GivePower (player, POWER_TYPE_STRENGTH))
    {
      return;
    }
    player->message = GOTBERSERK;
    if (player->readyweapon != WEAPON_TYPE_FIST)
    {
      player->pendingweapon = WEAPON_TYPE_FIST;
    }
    sound = sfx_getpow;
    break;

  case SPR_PINS:
    if (!P_GivePower (player, POWER_TYPE_INVISIBILITY))
    {
      return;
    }
    player->message = GOTINVIS;
    sound = sfx_getpow;
    break;

  case SPR_SUIT:
    if (!P_GivePower (player, POWER_TYPE_IRONFEET))
    {
      return;
    }
    player->message = GOTSUIT;
    sound = sfx_getpow;
    break;

  case SPR_PMAP:
    if (!P_GivePower (player, POWER_TYPE_ALLMAP))
    {
      return;
    }
    player->message = GOTMAP;
    sound = sfx_getpow;
    break;

  case SPR_PVIS:
    if (!P_GivePower (player, POWER_TYPE_INFRARED))
    {
      return;
    }
    player->message = GOTVISOR;
    sound = sfx_getpow;
    break;

  // ammo
  case SPR_CLIP:
    if (special->flags & MF_DROPPED)
    {
      if (!P_GiveAmmo (player, AMMO_TYPE_CLIP, 0))
      {
        return;
      }
    }
    else
    {
      if (!P_GiveAmmo (player, AMMO_TYPE_CLIP, 1))
      {
        return;
      }
    }
    player->message = GOTCLIP;
    break;

  case SPR_AMMO:
    if (!P_GiveAmmo (player, AMMO_TYPE_CLIP, 5))
    {
      return;
    }
    player->message = GOTCLIPBOX;
    break;

  case SPR_ROCK:
    if (!P_GiveAmmo (player, AMMO_TYPE_MISSILE, 1))
    {
      return;
    }
    player->message = GOTROCKET;
    break;

  case SPR_BROK:
    if (!P_GiveAmmo (player, AMMO_TYPE_MISSILE, 5))
    {
      return;
    }
    player->message = GOTROCKBOX;
    break;

  case SPR_CELL:
    if (!P_GiveAmmo (player, AMMO_TYPE_CELL, 1))
    {
      return;
    }
    player->message = GOTCELL;
    break;

  case SPR_CELP:
    if (!P_GiveAmmo (player, AMMO_TYPE_CELL, 5))
    {
      return;
    }
    player->message = GOTCELLBOX;
    break;

  case SPR_SHEL:
    if (!P_GiveAmmo (player, AMMO_TYPE_SHELL, 1))
    {
      return;
    }
    player->message = GOTSHELLS;
    break;

  case SPR_SBOX:
    if (!P_GiveAmmo (player, AMMO_TYPE_SHELL, 5))
    {
      return;
    }
    player->message = GOTSHELLBOX;
    break;

  case SPR_BPAK:
    if (!player->backpack)
    {
      for (i = 0 ; i < NUMAMMO ; i++)
      {
        player->maxammo[i] *= 2;
      }
      player->backpack = true;
    }
    for (i = 0 ; i < NUMAMMO ; i++)
    {
      P_GiveAmmo (player, i, 1);
    }
    player->message = GOTBACKPACK;
    break;

  // weapons
  case SPR_BFUG:
    if (!P_GiveWeapon (player, WEAPON_TYPE_BFG, false) )
    {
      return;
    }
    player->message = GOTBFG9000;
    sound = sfx_wpnup;
    break;

  case SPR_MGUN:
    if (!P_GiveWeapon (player, WEAPON_TYPE_CHAINGUN, special->flags & MF_DROPPED) )
    {
      return;
    }
    player->message = GOTCHAINGUN;
    sound = sfx_wpnup;
    break;

  case SPR_CSAW:
    if (!P_GiveWeapon (player, WEAPON_TYPE_CHAINSAW, false) )
    {
      return;
    }
    player->message = GOTCHAINSAW;
    sound = sfx_wpnup;
    break;

  case SPR_LAUN:
    if (!P_GiveWeapon (player, WEAPON_TYPE_MISSILE, false) )
    {
      return;
    }
    player->message = GOTLAUNCHER;
    sound = sfx_wpnup;
    break;

  case SPR_PLAS:
    if (!P_GiveWeapon (player, WEAPON_TYPE_PLASMA, false) )
    {
      return;
    }
    player->message = GOTPLASMA;
    sound = sfx_wpnup;
    break;

  case SPR_SHOT:
    if (!P_GiveWeapon (player, WEAPON_TYPE_SHOTGUN, special->flags & MF_DROPPED ) )
    {
      return;
    }
    player->message = GOTSHOTGUN;
    sound = sfx_wpnup;
    break;

  case SPR_SGN2:
    if (!P_GiveWeapon (player, WEAPON_TYPE_SUPERSHOTGUN, special->flags & MF_DROPPED ) )
    {
      return;
    }
    player->message = GOTSHOTGUN2;
    sound = sfx_wpnup;
    break;

  default:
    I_Error ("P_SpecialThing: Unknown gettable thing");
  }

  if (special->flags & MF_COUNTITEM)
  {
    player->itemcount++;
  }
  P_RemoveMobj (special);
  player->bonuscount += BONUSADD;
  if (player == &players[consoleplayer])
  {
    S_StartSound (NULL, sound);
  }
}


//
// KillMobj
//
void
P_KillMobj
( mobj_t* source,
  mobj_t* target )
{
  mobjtype_t  item;
  mobj_t* mo;

  target->flags &= ~(MF_SHOOTABLE | MF_FLOAT | MF_SKULLFLY);

  if (target->type != MT_SKULL)
  {
    target->flags &= ~MF_NOGRAVITY;
  }

  target->flags |= MF_CORPSE | MF_DROPOFF;
  target->height >>= 2;

  if (source && source->player)
  {
    // count for intermission
    if (target->flags & MF_COUNTKILL)
    {
      source->player->killcount++;
    }

    if (target->player)
    {
      source->player->frags[target->player - players]++;
    }
  }
  else if (!netgame && (target->flags & MF_COUNTKILL) )
  {
    // count all monster deaths,
    // even those caused by other monsters
    players[0].killcount++;
  }

  if (target->player)
  {
    // count environment kills against you
    if (!source)
    {
      target->player->frags[target->player - players]++;
    }

    target->flags &= ~MF_SOLID;
    target->player->playerstate = PST_DEAD;
    P_DropWeapon (target->player);

    if (target->player == &players[consoleplayer]
        && automapactive)
    {
      // don't die in auto map,
      // switch view prior to dying
      AM_Stop ();
    }

  }

  if (target->health < -target->info->spawnhealth
      && target->info->xdeathstate)
  {
    P_SetMobjState (target, target->info->xdeathstate);
  }
  else
  {
    P_SetMobjState (target, target->info->deathstate);
  }
  target->tics -= P_Random() & 3;

  if (target->tics < 1)
  {
    target->tics = 1;
  }

  //  I_StartSound (&actor->r, actor->info->deathsound);


  // Drop stuff.
  // This determines the kind of object spawned
  // during the death frame of a thing.
  switch (target->type)
  {
  case MT_WOLFSS:
  case MT_POSSESSED:
    item = MT_CLIP;
    break;

  case MT_SHOTGUY:
    item = MT_SHOTGUN;
    break;

  case MT_CHAINGUY:
    item = MT_CHAINGUN;
    break;

  default:
    return;
  }

  mo = P_SpawnMobj (target->x, target->y, ONFLOORZ, item);
  mo->flags |= MF_DROPPED;  // special versions of items
}




//
// P_DamageMobj
// Damages both enemies and players
// "inflictor" is the thing that caused the damage
//  creature or missile, can be NULL (slime, etc)
// "source" is the thing to target after taking damage
//  creature or NULL
// Source and inflictor are the same for melee attacks.
// Source can be NULL for slime, barrel explosions
// and other environmental stuff.
//
void
P_DamageMobj
( mobj_t* target,
  mobj_t* inflictor,
  mobj_t* source,
  int     damage )
{
  unsigned  ang;
  int   saved;
  player_t* player;
  fixed_t thrust;
  int   temp;

  if ( !(target->flags & MF_SHOOTABLE) )
  {
    return;  // shouldn't happen...
  }

  if (target->health <= 0)
  {
    return;
  }

  if ( target->flags & MF_SKULLFLY )
  {
    target->momx = target->momy = target->momz = 0;
  }

  player = target->player;
  if (player && gameskill == SKILL_BABY)
  {
    damage >>= 1;  // take half damage in trainer mode
  }


  // Some close combat weapons should not
  // inflict thrust and push the victim out of reach,
  // thus kick away unless using the chainsaw.
  if (inflictor
      && !(target->flags & MF_NOCLIP)
      && (!source
          || !source->player
          || source->player->readyweapon != WEAPON_TYPE_CHAINSAW))
  {
    ang = R_PointToAngle2 ( inflictor->x,
                            inflictor->y,
                            target->x,
                            target->y);

    thrust = damage * (FRACUNIT >> 3) * 100 / target->info->mass;

    // make fall forwards sometimes
    if ( damage < 40
         && damage > target->health
         && target->z - inflictor->z > 64 * FRACUNIT
         && (P_Random () & 1) )
    {
      ang += ANG180;
      thrust *= 4;
    }

    ang >>= ANGLETOFINESHIFT;
    target->momx += FixedMul (thrust, finecosine[ang]);
    target->momy += FixedMul (thrust, finesine[ang]);
  }

  // player specific
  if (player)
  {
    // end of game hell hack
    if (target->subsector->sector->special == 11
        && damage >= target->health)
    {
      damage = target->health - 1;
    }


    // Below certain threshold,
    // ignore damage in GOD mode, or with INVUL power.
    if ( damage < 1000
         && ( (player->cheats & CF_GODMODE)
              || player->powers[POWER_TYPE_INVULNERABILITY] ) )
    {
      return;
    }

    if (player->armortype)
    {
      if (player->armortype == 1)
      {
        saved = damage / 3;
      }
      else
      {
        saved = damage / 2;
      }

      if (player->armorpoints <= saved)
      {
        // armor is used up
        saved = player->armorpoints;
        player->armortype = 0;
      }
      player->armorpoints -= saved;
      damage -= saved;
    }
    player->health -= damage;   // mirror mobj health here for Dave
    if (player->health < 0)
    {
      player->health = 0;
    }

    player->attacker = source;
    player->damagecount += damage;  // add damage after armor / invuln

    if (player->damagecount > 100)
    {
      player->damagecount = 100;  // teleport stomp does 10k points...
    }

    temp = damage < 100 ? damage : 100;

    if (player == &players[consoleplayer])
    {
      I_Tactile (40, 10, 40 + temp * 2);
    }
  }

  // do the damage
  target->health -= damage;
  if (target->health <= 0)
  {
    P_KillMobj (source, target);
    return;
  }

  if ( (P_Random () < target->info->painchance)
       && !(target->flags & MF_SKULLFLY) )
  {
    target->flags |= MF_JUSTHIT;  // fight back!

    P_SetMobjState (target, target->info->painstate);
  }

  target->reactiontime = 0;   // we're awake now...

  if ( (!target->threshold || target->type == MT_VILE)
       && source && source != target
       && source->type != MT_VILE)
  {
    // if not intent on another player,
    // chase after this one
    target->target = source;
    target->threshold = BASETHRESHOLD;
    if (target->state == &states[target->info->spawnstate]
        && target->info->seestate != S_NULL)
    {
      P_SetMobjState (target, target->info->seestate);
    }
  }

}

