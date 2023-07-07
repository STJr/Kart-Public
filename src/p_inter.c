// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2018 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  p_inter.c
/// \brief Handling interactions (i.e., collisions)

#include "doomdef.h"
#include "i_system.h"
#include "am_map.h"
#include "g_game.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"
#include "r_main.h"
#include "st_stuff.h"
#include "hu_stuff.h"
#include "lua_hook.h"
#include "m_cond.h" // unlockables, emblems, etc
#include "m_cheat.h" // objectplace
#include "m_misc.h"
#include "v_video.h" // video flags for CEchos
#include "k_kart.h" // SRB2kart

// CTF player names
#define CTFTEAMCODE(pl) pl->ctfteam ? (pl->ctfteam == 1 ? "\x85" : "\x84") : ""
#define CTFTEAMENDCODE(pl) pl->ctfteam ? "\x80" : ""

// CTF player names
#define CTFTEAMCODE(pl) pl->ctfteam ? (pl->ctfteam == 1 ? "\x85" : "\x84") : ""
#define CTFTEAMENDCODE(pl) pl->ctfteam ? "\x80" : ""

void P_ForceFeed(const player_t *player, INT32 attack, INT32 fade, tic_t duration, INT32 period)
{
	BasicFF_t Basicfeed;
	if (!player)
		return;
	Basicfeed.Duration = (UINT32)(duration * (100L/TICRATE));
	Basicfeed.ForceX = Basicfeed.ForceY = 1;
	Basicfeed.Gain = 25000;
	Basicfeed.Magnitude = period*10;
	Basicfeed.player = player;
	/// \todo test FFB
	P_RampConstant(&Basicfeed, attack, fade);
}

void P_ForceConstant(const BasicFF_t *FFInfo)
{
	JoyFF_t ConstantQuake;
	if (!FFInfo || !FFInfo->player)
		return;
	ConstantQuake.ForceX    = FFInfo->ForceX;
	ConstantQuake.ForceY    = FFInfo->ForceY;
	ConstantQuake.Duration  = FFInfo->Duration;
	ConstantQuake.Gain      = FFInfo->Gain;
	ConstantQuake.Magnitude = FFInfo->Magnitude;
	if (FFInfo->player == &players[consoleplayer])
		I_Tactile(ConstantForce, &ConstantQuake);
	else if (splitscreen && FFInfo->player == &players[displayplayers[1]])
		I_Tactile2(ConstantForce, &ConstantQuake);
	else if (splitscreen > 1 && FFInfo->player == &players[displayplayers[2]])
		I_Tactile3(ConstantForce, &ConstantQuake);
	else if (splitscreen > 2 && FFInfo->player == &players[displayplayers[3]])
		I_Tactile4(ConstantForce, &ConstantQuake);
}
void P_RampConstant(const BasicFF_t *FFInfo, INT32 Start, INT32 End)
{
	JoyFF_t RampQuake;
	if (!FFInfo || !FFInfo->player)
		return;
	RampQuake.ForceX    = FFInfo->ForceX;
	RampQuake.ForceY    = FFInfo->ForceY;
	RampQuake.Duration  = FFInfo->Duration;
	RampQuake.Gain      = FFInfo->Gain;
	RampQuake.Magnitude = FFInfo->Magnitude;
	RampQuake.Start     = Start;
	RampQuake.End       = End;
	if (FFInfo->player == &players[consoleplayer])
		I_Tactile(ConstantForce, &RampQuake);
	else if (splitscreen && FFInfo->player == &players[displayplayers[1]])
		I_Tactile2(ConstantForce, &RampQuake);
	else if (splitscreen > 1 && FFInfo->player == &players[displayplayers[2]])
		I_Tactile3(ConstantForce, &RampQuake);
	else if (splitscreen > 2 && FFInfo->player == &players[displayplayers[3]])
		I_Tactile4(ConstantForce, &RampQuake);
}


//
// GET STUFF
//

//
// P_CanPickupItem
//
// Returns true if the player is in a state where they can pick up items.
//
boolean P_CanPickupItem(player_t *player, UINT8 weapon)
{
	if (player->exiting || mapreset)
		return false;

	/*if (G_BattleGametype() && player->kartstuff[k_bumper] <= 0) // No bumpers in Match
        return false;*/

	if (weapon)
	{
		// Item slot already taken up
		if (weapon == 2)
		{
			// Invulnerable
			if (player->powers[pw_flashing] > 0
				|| player->kartstuff[k_spinouttimer] > 0
				|| player->kartstuff[k_squishedtimer] > 0
				|| player->kartstuff[k_invincibilitytimer] > 0
				|| player->kartstuff[k_growshrinktimer] > 0
				|| player->kartstuff[k_hyudorotimer] > 0)
				return false;

			// Already have fake
			if (player->kartstuff[k_roulettetype] == 2
				|| player->kartstuff[k_eggmanexplode])
				return false;
		}
		else
		{
			// Item-specific timer going off
			if (player->kartstuff[k_stealingtimer] || player->kartstuff[k_stolentimer]
				|| player->kartstuff[k_growshrinktimer] > 0 || player->kartstuff[k_rocketsneakertimer]
				|| player->kartstuff[k_eggmanexplode])
				return false;

			// Item slot already taken up
			if (player->kartstuff[k_itemroulette]
				|| (weapon != 3 && player->kartstuff[k_itemamount])
				|| player->kartstuff[k_itemheld])
				return false;

			if (weapon == 3 && player->kartstuff[k_itemtype] == KITEM_THUNDERSHIELD)
				return false; // No stacking thunder shields!
		}
	}

	return true;
}

//
// P_DoNightsScore
//
// When you pick up some items in nights, it displays
// a score sign, and awards you some drill time.
//
void P_DoNightsScore(player_t *player)
{
	mobj_t *dummymo;

	if (player->exiting)
		return; // Don't do any fancy shit for failures.

	dummymo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z+player->mo->height/2, MT_NIGHTSCORE);
	if (player->bot)
		player = &players[consoleplayer];

	if (G_IsSpecialStage(gamemap)) // Global link count? Maybe not a good idea...
	{
		INT32 i;
		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i])
			{
				if (++players[i].linkcount > players[i].maxlink)
					players[i].maxlink = players[i].linkcount;
				players[i].linktimer = 2*TICRATE;
			}
	}
	else // Individual link counts
	{
		if (++player->linkcount > player->maxlink)
			player->maxlink = player->linkcount;
		player->linktimer = 2*TICRATE;
	}

	if (player->linkcount < 10)
	{
		if (player->bonustime)
		{
			P_AddPlayerScore(player, player->linkcount*20);
			P_SetMobjState(dummymo, dummymo->info->xdeathstate+player->linkcount-1);
		}
		else
		{
			P_AddPlayerScore(player, player->linkcount*10);
			P_SetMobjState(dummymo, dummymo->info->spawnstate+player->linkcount-1);
		}
	}
	else
	{
		if (player->bonustime)
		{
			P_AddPlayerScore(player, 200);
			P_SetMobjState(dummymo, dummymo->info->xdeathstate+9);
		}
		else
		{
			P_AddPlayerScore(player, 100);
			P_SetMobjState(dummymo, dummymo->info->spawnstate+9);
		}
	}

	// Hoops are the only things that should add to your drill meter
	//player->drillmeter += TICRATE;
	dummymo->momz = FRACUNIT;
	dummymo->fuse = 3*TICRATE;

	// What?! NO, don't use the camera! Scale up instead!
	//P_InstaThrust(dummymo, R_PointToAngle2(dummymo->x, dummymo->y, camera[0].x, camera[0].y), 3*FRACUNIT);
	dummymo->scalespeed = FRACUNIT/25;
	dummymo->destscale = 2*FRACUNIT;
}

/** Takes action based on a ::MF_SPECIAL thing touched by a player.
  * Actually, this just checks a few things (heights, toucher->player, no
  * objectplace, no dead or disappearing things)
  *
  * The special thing may be collected and disappear, or a sound may play, or
  * both.
  *
  * \param special     The special thing.
  * \param toucher     The player's mobj.
  * \param heightcheck Whether or not to make sure the player and the object
  *                    are actually touching.
  */
void P_TouchSpecialThing(mobj_t *special, mobj_t *toucher, boolean heightcheck)
{
	player_t *player;
	INT32 i;

	if (objectplacing)
		return;

	I_Assert(special != NULL);
	I_Assert(toucher != NULL);

	// Dead thing touching.
	// Can happen with a sliding player corpse.
	if (toucher->health <= 0)
		return;

	if (heightcheck)
	{
		/*if (special->type == MT_FLINGEMERALD) // little hack here...
		{ // flingemerald sprites are low to the ground, so extend collision radius down some.
			if (toucher->z > (special->z + special->height))
				return;
			if (special->z - special->height > (toucher->z + toucher->height))
				return;
		}
		else*/
		{
			if (toucher->momz < 0) {
				if (toucher->z + toucher->momz > special->z + special->height)
					return;
			} else if (toucher->z > special->z + special->height)
				return;
			if (toucher->momz > 0) {
				if (toucher->z + toucher->height + toucher->momz < special->z)
					return;
			} else if (toucher->z + toucher->height < special->z)
				return;
		}
	}

	if (special->health <= 0)
		return;

	player = toucher->player;
	I_Assert(player != NULL); // Only players can touch stuff!

	if (player->spectator)
		return;

	// Ignore eggman in "ouchie" mode
	if (special->flags & MF_BOSS && special->flags2 & MF2_FRET)
		return;

	if (LUAh_TouchSpecial(special, toucher) || P_MobjWasRemoved(special))
		return;

	if (special->flags & MF_BOSS)
	{
		/*if (special->type == MT_BLACKEGGMAN)
		{
			P_DamageMobj(toucher, special, special, 1); // ouch
			return;
		}

		if (((player->pflags & PF_NIGHTSMODE) && (player->pflags & PF_DRILLING))
		|| (player->pflags & (PF_JUMPED|PF_SPINNING|PF_GLIDING))
		|| player->powers[pw_invulnerability] || player->powers[pw_super]) // Do you possess the ability to subdue the object?
		{
			if (P_MobjFlip(toucher)*toucher->momz < 0)
				toucher->momz = -toucher->momz;
			toucher->momx = -toucher->momx;
			toucher->momy = -toucher->momy;
			P_DamageMobj(special, toucher, toucher, 1);
		}
		else if (((toucher->z < special->z && !(toucher->eflags & MFE_VERTICALFLIP))
		|| (toucher->z + toucher->height > special->z + special->height && (toucher->eflags & MFE_VERTICALFLIP)))
		&& player->charability == CA_FLY
		&& (player->powers[pw_tailsfly]
		|| (toucher->state >= &states[S_PLAY_SPC1] && toucher->state <= &states[S_PLAY_SPC4]))) // Tails can shred stuff with his propeller.
		{
			toucher->momz = -toucher->momz/2;

			P_DamageMobj(special, toucher, toucher, 1);
		}
		// SRB2kart - Removed: No more fly states
		else*/
			P_DamageMobj(toucher, special, special, 1);

		return;
	}
	else if ((special->flags & MF_ENEMY) && !(special->flags & MF_MISSILE)
		&& (special->type != MT_SPRINGSHELL)) // Kart: prevent random hits from these things
	{
		P_DamageMobj(toucher, special, special, 1);
		return;
	}
	else if (special->flags & MF_FIRE)
	{
		P_DamageMobj(toucher, special, special, 1);
		return;
	}
	else
	{
	// We now identify by object type, not sprite! Tails 04-11-2001
	switch (special->type)
	{
		case MT_MEMENTOSTP:	 // Mementos teleport
			// Teleport player to the other teleporter (special->target). We'll assume there's always only ever 2.
			if (!special->target)
				return;	// foolproof crash prevention check!!!!!

			P_SetOrigin(player->mo, special->target->x, special->target->y, special->target->z + (48<<FRACBITS));
			player->mo->angle = special->target->angle;
			P_SetObjectMomZ(player->mo, 12<<FRACBITS, false);
			P_InstaThrust(player->mo, player->mo->angle, 20<<FRACBITS);
			return;
		case MT_FLOATINGITEM: // SRB2kart
			if (!P_CanPickupItem(player, 3) || (player->kartstuff[k_itemamount] && player->kartstuff[k_itemtype] != special->threshold))
				return;

			if (G_BattleGametype() && player->kartstuff[k_bumper] <= 0)
				return;

			player->kartstuff[k_itemtype] = special->threshold;
			player->kartstuff[k_itemamount] += special->movecount;
			if (player->kartstuff[k_itemamount] > 255)
				player->kartstuff[k_itemamount] = 255;

			S_StartSound(special, special->info->deathsound);

			P_SetTarget(&special->tracer, toucher);
			special->flags2 |= MF2_NIGHTSPULL;
			special->destscale = mapobjectscale>>4;
			special->scalespeed <<= 1;

			special->flags &= ~MF_SPECIAL;
			return;
		case MT_RANDOMITEM:			// SRB2kart
			if (!P_CanPickupItem(player, 1))
				return;

			if (G_BattleGametype() && player->kartstuff[k_bumper] <= 0)
			{
				if (player->kartstuff[k_comebackmode] || player->kartstuff[k_comebacktimer])
					return;
				player->kartstuff[k_comebackmode] = 1;
			}

			special->momx = special->momy = special->momz = 0;
			P_SetTarget(&special->target, toucher);
			P_KillMobj(special, toucher, toucher);
			break;
		case MT_EGGMANITEM_SHIELD: // SRB2kart
		case MT_EGGMANITEM:
			if ((special->target == toucher || special->target == toucher->target) && (special->threshold > 0))
				return;

			if (special->health <= 0 || toucher->health <= 0)
				return;

			if (!P_CanPickupItem(player, 2))
				return;

			if (G_BattleGametype() && player->kartstuff[k_bumper] <= 0)
			{
				if (player->kartstuff[k_comebackmode] || player->kartstuff[k_comebacktimer])
					return;
				player->kartstuff[k_comebackmode] = 2;
			}
			else
			{
				K_DropItems(player); //K_StripItems(player);
				//K_StripOther(player);
				player->kartstuff[k_itemroulette] = 1;
				player->kartstuff[k_roulettetype] = 2;
			}

#if 0
			// Eggbox snipe!
			if (special->type == MT_EGGMANITEM && special->health > 1)
				S_StartSound(toucher, sfx_bsnipe);
#endif

			{
				mobj_t *poof = P_SpawnMobj(special->x, special->y, special->z, MT_EXPLODE);
				S_StartSound(poof, special->info->deathsound);
			}

			if (special->target && special->target->player)
			{
				if (G_RaceGametype() || special->target->player->kartstuff[k_bumper] > 0)
					player->kartstuff[k_eggmanblame] = special->target->player-players;
				else
					player->kartstuff[k_eggmanblame] = player-players;

				if (special->target->hnext == special)
				{
					P_SetTarget(&special->target->hnext, NULL);
					special->target->player->kartstuff[k_eggmanheld] = 0;
				}
			}

			P_RemoveMobj(special);
			return;
		case MT_KARMAHITBOX:
			if (!special->target->player)
				return;
			if (player == special->target->player)
				return;
			if (player->kartstuff[k_bumper] <= 0)
				return;
			if (special->target->player->exiting || player->exiting)
				return;

			if (special->target->player->kartstuff[k_comebacktimer]
				|| special->target->player->kartstuff[k_spinouttimer]
				|| special->target->player->kartstuff[k_squishedtimer])
				return;

			if (!special->target->player->kartstuff[k_comebackmode])
			{
				if (player->kartstuff[k_growshrinktimer] || player->kartstuff[k_squishedtimer]
					|| player->kartstuff[k_hyudorotimer] || player->kartstuff[k_spinouttimer]
					|| player->kartstuff[k_invincibilitytimer] || player->powers[pw_flashing])
					return;
				else
				{
					mobj_t *boom = P_SpawnMobj(special->target->x, special->target->y, special->target->z, MT_BOOMEXPLODE);
					UINT8 ptadd = (K_IsPlayerWanted(player) ? 2 : 1);

					boom->scale = special->target->scale;
					boom->destscale = special->target->scale;
					boom->momz = 5*FRACUNIT;
					if (special->target->color)
						boom->color = special->target->color;
					else
						boom->color = SKINCOLOR_KETCHUP;
					S_StartSound(boom, special->info->attacksound);

					if (player->kartstuff[k_bumper] == 1) // If you have only one bumper left, and see if it's a 1v1
					{
						INT32 numingame = 0;

						for (i = 0; i < MAXPLAYERS; i++)
						{
							if (!playeringame[i] || players[i].spectator || players[i].kartstuff[k_bumper] <= 0)
								continue;
							numingame++;
						}

						if (numingame <= 2) // If so, then an extra karma point so they are 100% certain to switch places; it's annoying to end matches with a bomb kill
							ptadd++;
					}

					special->target->player->kartstuff[k_comebackpoints] += ptadd;

					if (ptadd > 1)
						special->target->player->kartstuff[k_yougotem] = 2*TICRATE;

					if (special->target->player->kartstuff[k_comebackpoints] >= 2)
						K_StealBumper(special->target->player, player, true);

					special->target->player->kartstuff[k_comebacktimer] = comebacktime;

					K_ExplodePlayer(player, special->target, special);
				}
			}
			else if (special->target->player->kartstuff[k_comebackmode] == 1 && P_CanPickupItem(player, 1))
			{
				mobj_t *poof = P_SpawnMobj(special->x, special->y, special->z, MT_EXPLODE);
				S_StartSound(poof, special->info->seesound);

				// Karma fireworks
				for (i = 0; i < 5; i++)
				{
					mobj_t *firework = P_SpawnMobj(special->x, special->y, special->z, MT_KARMAFIREWORK);
					firework->momx = (special->target->momx + toucher->momx) / 2;
					firework->momy = (special->target->momy + toucher->momy) / 2;
					firework->momz = (special->target->momz + toucher->momz) / 2;
					P_Thrust(firework, FixedAngle((72*i)<<FRACBITS), P_RandomRange(1,8)*special->scale);
					P_SetObjectMomZ(firework, P_RandomRange(1,8)*special->scale, false);
					firework->color = special->target->color;
				}

				special->target->player->kartstuff[k_comebackmode] = 0;
				special->target->player->kartstuff[k_comebackpoints]++;

				if (special->target->player->kartstuff[k_comebackpoints] >= 2)
					K_StealBumper(special->target->player, player, true);
				special->target->player->kartstuff[k_comebacktimer] = comebacktime;

				player->kartstuff[k_itemroulette] = 1;
				player->kartstuff[k_roulettetype] = 1;
			}
			else if (special->target->player->kartstuff[k_comebackmode] == 2 && P_CanPickupItem(player, 2))
			{
				mobj_t *poof = P_SpawnMobj(special->x, special->y, special->z, MT_EXPLODE);
				UINT8 ptadd = 1; // No WANTED bonus for tricking

				S_StartSound(poof, special->info->seesound);

				if (player->kartstuff[k_bumper] == 1) // If you have only one bumper left, and see if it's a 1v1
				{
					INT32 numingame = 0;

					for (i = 0; i < MAXPLAYERS; i++)
					{
						if (!playeringame[i] || players[i].spectator || players[i].kartstuff[k_bumper] <= 0)
							continue;
						numingame++;
					}

					if (numingame <= 2) // If so, then an extra karma point so they are 100% certain to switch places; it's annoying to end matches with a fake kill
						ptadd++;
				}

				special->target->player->kartstuff[k_comebackmode] = 0;
				special->target->player->kartstuff[k_comebackpoints] += ptadd;

				if (ptadd > 1)
					special->target->player->kartstuff[k_yougotem] = 2*TICRATE;

				if (special->target->player->kartstuff[k_comebackpoints] >= 2)
					K_StealBumper(special->target->player, player, true);

				special->target->player->kartstuff[k_comebacktimer] = comebacktime;

				K_DropItems(player); //K_StripItems(player);
				//K_StripOther(player);

				player->kartstuff[k_itemroulette] = 1;
				player->kartstuff[k_roulettetype] = 2;

				if (special->target->player->kartstuff[k_eggmanblame] >= 0
				&& special->target->player->kartstuff[k_eggmanblame] < MAXPLAYERS
				&& playeringame[special->target->player->kartstuff[k_eggmanblame]]
				&& !players[special->target->player->kartstuff[k_eggmanblame]].spectator)
					player->kartstuff[k_eggmanblame] = special->target->player->kartstuff[k_eggmanblame];
				else
					player->kartstuff[k_eggmanblame] = -1;

				special->target->player->kartstuff[k_eggmanblame] = -1;
			}
			return;
		case MT_SPB:
			if ((special->target == toucher || special->target == toucher->target) && (special->threshold > 0))
				return;

			if (special->health <= 0 || toucher->health <= 0)
				return;

			if (player->spectator)
				return;

			if (special->tracer && !P_MobjWasRemoved(special->tracer) && toucher == special->tracer)
			{
				mobj_t *spbexplode;

				if (player->kartstuff[k_invincibilitytimer] > 0 || player->kartstuff[k_growshrinktimer] > 0 || player->kartstuff[k_hyudorotimer] > 0)
				{
					//player->powers[pw_flashing] = 0;
					K_DropHnextList(player);
					K_StripItems(player);
				}

				S_StopSound(special); // Don't continue playing the gurgle or the siren

				spbexplode = P_SpawnMobj(toucher->x, toucher->y, toucher->z, MT_SPBEXPLOSION);
				spbexplode->extravalue1 = 1; // Tell K_ExplodePlayer to use extra knockback
				if (special->target && !P_MobjWasRemoved(special->target))
					P_SetTarget(&spbexplode->target, special->target);

				P_RemoveMobj(special);
			}
			else
				K_SpinPlayer(player, special->target, 0, special, false);
			return;
		/*case MT_EERIEFOG:
			special->frame &= ~FF_TRANS80;
			special->frame |= FF_TRANS90;
			return;*/
		case MT_SMK_MOLE:
			if (special->target && !P_MobjWasRemoved(special->target))
				return;

			if (special->health <= 0 || toucher->health <= 0)
				return;

			if (!player->mo || player->spectator)
				return;

			// kill
			if (player->kartstuff[k_invincibilitytimer] > 0 || player->kartstuff[k_growshrinktimer] > 0)
			{
				P_KillMobj(special, toucher, toucher);
				return;
			}

			// no interaction
			if (player->powers[pw_flashing] > 0 || player->kartstuff[k_hyudorotimer] > 0
				|| player->kartstuff[k_squishedtimer] > 0 || player->kartstuff[k_spinouttimer] > 0)
				return;

			// attach to player!
			P_SetTarget(&special->target, toucher);
			S_StartSound(special, sfx_s1a2);
			return;
		case MT_CDUFO: // SRB2kart
			if (special->fuse || !P_CanPickupItem(player, 1) || (G_BattleGametype() && player->kartstuff[k_bumper] <= 0))
				return;

			player->kartstuff[k_itemroulette] = 1;
			player->kartstuff[k_roulettetype] = 1;

			// Karma fireworks
			for (i = 0; i < 5; i++)
			{
				mobj_t *firework = P_SpawnMobj(special->x, special->y, special->z, MT_KARMAFIREWORK);
				firework->momx = toucher->momx;
				firework->momy = toucher->momy;
				firework->momz = toucher->momz;
				P_Thrust(firework, FixedAngle((72*i)<<FRACBITS), P_RandomRange(1,8)*special->scale);
				P_SetObjectMomZ(firework, P_RandomRange(1,8)*special->scale, false);
				firework->color = toucher->color;
			}

			S_StartSound(toucher, sfx_cdfm73); // they don't make this sound in the original game but it's nice to have a "reward" for good play

			//special->momx = special->momy = special->momz = 0;
			special->momz = -(3*special->scale)/2;
			//P_SetTarget(&special->target, toucher);
			special->fuse = 2*TICRATE;
			break;
		case MT_BALLOON: // SRB2kart
			P_SetObjectMomZ(toucher, 20<<FRACBITS, false);
			break;
		case MT_TUMBLEGEM:
		case MT_TUMBLECOIN:
			{
				SINT8 flip = P_MobjFlip(special);
				if ((toucher->momx || toucher->momy) && (flip * special->momz <= 0))
				{
					special->momx = toucher->momx;
					special->momy = toucher->momy;
					special->momz = flip * max(P_AproxDistance(toucher->momx, toucher->momy) / 4, FixedMul(special->info->speed, special->scale));
					if (flip * toucher->momz > 0)
						special->momz += toucher->momz / 8;
					if ((statenum_t)(special->state-states) != special->info->seestate)
						P_SetMobjState(special, special->info->seestate);
					S_StartSound(special, special->info->activesound);
				}
			}
			return;

// ***************************************** //
// Rings, coins, spheres, weapon panels, etc //
// ***************************************** //
		case MT_REDTEAMRING:
			if (player->ctfteam != 1)
				return;
			/* FALLTHRU */
		case MT_BLUETEAMRING: // Yes, I'm lazy. Oh well, deal with it.
			if (special->type == MT_BLUETEAMRING && player->ctfteam != 2)
				return;
			/* FALLTHRU */
		case MT_RING:
		case MT_FLINGRING:
			if (!(P_CanPickupItem(player, 0)))
				return;

			special->momx = special->momy = special->momz = 0;
			P_GivePlayerRings(player, 1);

			if ((maptol & TOL_NIGHTS) && special->type != MT_FLINGRING)
				P_DoNightsScore(player);
			break;

		case MT_COIN:
		case MT_FLINGCOIN:
			if (!(P_CanPickupItem(player, 0)))
				return;

			special->momx = special->momy = 0;
			P_GivePlayerRings(player, 1);

			if ((maptol & TOL_NIGHTS) && special->type != MT_FLINGCOIN)
				P_DoNightsScore(player);
			break;
		case MT_BLUEBALL:
			if (!(P_CanPickupItem(player, 0)))
				return;

			P_GivePlayerRings(player, 1);

			special->momx = special->momy = special->momz = 0;
			special->destscale = FixedMul(8*FRACUNIT, special->scale);
			if (states[special->info->deathstate].tics > 0)
				special->scalespeed = FixedDiv(FixedDiv(special->destscale, special->scale), states[special->info->deathstate].tics<<FRACBITS);
			else
				special->scalespeed = 4*FRACUNIT/5;

			if (maptol & TOL_NIGHTS)
				P_DoNightsScore(player);
			break;
		case MT_AUTOPICKUP:
		case MT_BOUNCEPICKUP:
		case MT_SCATTERPICKUP:
		case MT_GRENADEPICKUP:
		case MT_EXPLODEPICKUP:
		case MT_RAILPICKUP:
			if (!(P_CanPickupItem(player, 1)))
				return;

			// Give the power and ringweapon
			if (special->info->mass >= pw_infinityring && special->info->mass <= pw_railring)
			{
				INT32 pindex = special->info->mass - (INT32)pw_infinityring;

				player->powers[special->info->mass] += (UINT16)special->info->reactiontime;
				player->ringweapons |= 1 << (pindex-1);

				if (player->powers[special->info->mass] > rw_maximums[pindex])
					player->powers[special->info->mass] = rw_maximums[pindex];
			}
			break;

		// Ammo pickups
		case MT_INFINITYRING:
		case MT_AUTOMATICRING:
		case MT_BOUNCERING:
		case MT_SCATTERRING:
		case MT_GRENADERING:
		case MT_EXPLOSIONRING:
		case MT_RAILRING:
			if (!(P_CanPickupItem(player, 1)))
				return;

			if (special->info->mass >= pw_infinityring && special->info->mass <= pw_railring)
			{
				INT32 pindex = special->info->mass - (INT32)pw_infinityring;

				player->powers[special->info->mass] += (UINT16)special->health;
				if (player->powers[special->info->mass] > rw_maximums[pindex])
					player->powers[special->info->mass] = rw_maximums[pindex];
			}
			break;

// ***************************** //
// Gameplay related collectibles //
// ***************************** //
		// Special Stage Token
		case MT_EMMY:
			if (player->bot)
				return;
			tokenlist += special->health;

			if (ALL7EMERALDS(emeralds)) // Got all 7
			{
				P_GivePlayerRings(player, 50);
				nummaprings += 50; // no cheating towards Perfect!
			}
			else
				token++;

			if (special->tracer) // token BG
				P_RemoveMobj(special->tracer);
			break;

		// Emerald Hunt
		case MT_EMERHUNT:
			if (player->bot)
				return;

			if (hunt1 == special)
				hunt1 = NULL;
			else if (hunt2 == special)
				hunt2 = NULL;
			else if (hunt3 == special)
				hunt3 = NULL;

			if (!hunt1 && !hunt2 && !hunt3)
			{
				for (i = 0; i < MAXPLAYERS; i++)
				{
					if (!playeringame[i] || players[i].spectator)
						continue;

					players[i].exiting = raceexittime+1;
				}
				S_StartSound(NULL, sfx_lvpass);
			}
			break;

		// Collectible emeralds
		case MT_EMERALD1:
		case MT_EMERALD2:
		case MT_EMERALD3:
		case MT_EMERALD4:
		case MT_EMERALD5:
		case MT_EMERALD6:
		case MT_EMERALD7:
			if (player->bot)
				return;

			if (special->threshold)
				player->powers[pw_emeralds] |= special->info->speed;
			else
				emeralds |= special->info->speed;

			if (special->target && special->target->type == MT_EMERALDSPAWN)
			{
				if (special->target->target)
					P_SetTarget(&special->target->target, NULL);

				special->target->threshold = 0;

				P_SetTarget(&special->target, NULL);
			}
			break;

		// Power stones / Match emeralds
		case MT_FLINGEMERALD:
			if (!(P_CanPickupItem(player, 1)) || player->tossdelay)
				return;

			player->powers[pw_emeralds] |= special->threshold;
			break;

		// Secret emblem thingy
		case MT_EMBLEM:
			{
				if (demo.playback || player->bot)
					return;
				emblemlocations[special->health-1].collected = true;

				M_UpdateUnlockablesAndExtraEmblems(false);

				G_SaveGameData(false);
				break;
			}

		// CTF Flags
		case MT_REDFLAG:
		case MT_BLUEFLAG:
			if (player->bot)
				return;
			if (player->powers[pw_flashing] || player->tossdelay)
				return;
			if (!special->spawnpoint)
				return;
			if (special->fuse == 1)
				return;
//			if (special->momz > 0)
//				return;
			{
				UINT8 flagteam = (special->type == MT_REDFLAG) ? 1 : 2;
				const char *flagtext;
				char flagcolor;
				char plname[MAXPLAYERNAME+4];

				if (special->type == MT_REDFLAG)
				{
					flagtext = M_GetText("Red flag");
					flagcolor = '\x85';
				}
				else
				{
					flagtext = M_GetText("Blue flag");
					flagcolor = '\x84';
				}
				snprintf(plname, sizeof(plname), "%s%s%s",
						 CTFTEAMCODE(player),
						 player_names[player - players],
						 CTFTEAMENDCODE(player));

				if (player->ctfteam == flagteam) // Player is on the same team as the flag
				{
					// Ignore height, only check x/y for now
					// avoids stupid problems with some flags constantly returning
					if (special->x>>FRACBITS != special->spawnpoint->x
					    || special->y>>FRACBITS != special->spawnpoint->y)
					{
						special->fuse = 1;
						special->flags2 |= MF2_JUSTATTACKED;

						if (!P_PlayerTouchingSectorSpecial(player, 4, 2 + flagteam))
						{
							CONS_Printf(M_GetText("%s returned the %c%s%c to base.\n"), plname, flagcolor, flagtext, 0x80);

							// The fuse code plays this sound effect
							//if (players[consoleplayer].ctfteam == player->ctfteam)
							//	S_StartSound(NULL, sfx_hoop1);
						}
					}
				}
				else if (player->ctfteam) // Player is on the other team (and not a spectator)
				{
					UINT16 flagflag   = (special->type == MT_REDFLAG) ? GF_REDFLAG : GF_BLUEFLAG;
					mobj_t **flagmobj = (special->type == MT_REDFLAG) ? &redflag : &blueflag;

					if (player->powers[pw_super])
						return;

					player->gotflag |= flagflag;
					CONS_Printf(M_GetText("%s picked up the %c%s%c!\n"), plname, flagcolor, flagtext, 0x80);
					(*flagmobj) = NULL;
					// code for dealing with abilities is handled elsewhere now
					break;
				}
			}
			return;

// ********************************** //
// NiGHTS gameplay items and powerups //
// ********************************** //
		/*case MT_NIGHTSDRONE:
			if (player->bot)
				return;
			if (player->exiting)
				return;
			if (player->bonustime)
			{
				if (G_IsSpecialStage(gamemap)) //After-mare bonus time/emerald reward in special stages.
				{
					// only allow the player with the emerald in-hand to leave.
					if (toucher->tracer && toucher->tracer->target
					&& toucher->tracer->target->type == MT_GOTEMERALD)
					{
					}
					else // Make sure that SOMEONE has the emerald, at least!
					{
						for (i = 0; i < MAXPLAYERS; i++)
							if (playeringame[i] && players[i].playerstate == PST_LIVE
							&& players[i].mo->tracer && players[i].mo->tracer->target
							&& players[i].mo->tracer->target->type == MT_GOTEMERALD)
								return;
						// Well no one has an emerald, so exit anyway!
					}
					P_GiveEmerald(false);
					// Don't play Ideya sound in special stage mode
				}
				else
					S_StartSound(toucher, special->info->activesound);
			}
			else //Initial transformation. Don't allow second chances in special stages!
			{
				if (player->pflags & PF_NIGHTSMODE)
					return;

				S_StartSound(toucher, sfx_supert);
			}
			if (!(netgame || multiplayer) && !(player->pflags & PF_NIGHTSMODE))
				P_SetTarget(&special->tracer, toucher);
			P_NightserizePlayer(player, special->health); // Transform!
			return;
		case MT_NIGHTSLOOPHELPER:
			// One second delay
			if (special->fuse < toucher->fuse - TICRATE)
			{
				thinker_t *th;
				mobj_t *mo2;
				INT32 count;
				fixed_t x,y,z, gatherradius;
				angle_t d;
				statenum_t sparklestate = S_NULL;

				if (special->target != toucher) // These ain't your helpers, pal!
					return;

				x = special->x>>FRACBITS;
				y = special->y>>FRACBITS;
				z = special->z>>FRACBITS;
				count = 1;

				// scan the remaining thinkers
				for (th = thinkercap.next; th != &thinkercap; th = th->next)
				{
					if (th->function.acp1 != (actionf_p1)P_MobjThinker)
						continue;

					mo2 = (mobj_t *)th;

					if (mo2 == special)
						continue;

					// Not our stuff!
					if (mo2->target != toucher)
						continue;

					if (mo2->type == MT_NIGHTSPARKLE)
						mo2->tics = 1;
					else if (mo2->type == MT_NIGHTSLOOPHELPER)
					{
						if (mo2->fuse >= special->fuse)
						{
							count++;
							x += mo2->x>>FRACBITS;
							y += mo2->y>>FRACBITS;
							z += mo2->z>>FRACBITS;
						}
						P_RemoveMobj(mo2);
					}
				}
				x = (x/count)<<FRACBITS;
				y = (y/count)<<FRACBITS;
				z = (z/count)<<FRACBITS;
				gatherradius = P_AproxDistance(P_AproxDistance(special->x - x, special->y - y), special->z - z);
				P_RemoveMobj(special);

				if (player->powers[pw_nights_superloop])
				{
					gatherradius *= 2;
					sparklestate = mobjinfo[MT_NIGHTSPARKLE].seestate;
				}

				if (gatherradius < 30*FRACUNIT) // Player is probably just sitting there.
					return;

				for (d = 0; d < 16; d++)
					P_SpawnParaloop(x, y, z, gatherradius, 16, MT_NIGHTSPARKLE, sparklestate, d*ANGLE_22h, false);

				S_StartSound(toucher, sfx_prloop);

				// Now we RE-scan all the thinkers to find close objects to pull
				// in from the paraloop. Isn't this just so efficient?
				for (th = thinkercap.next; th != &thinkercap; th = th->next)
				{
					if (th->function.acp1 != (actionf_p1)P_MobjThinker)
						continue;

					mo2 = (mobj_t *)th;

					if (P_AproxDistance(P_AproxDistance(mo2->x - x, mo2->y - y), mo2->z - z) > gatherradius)
						continue;

					if (mo2->flags & MF_SHOOTABLE)
					{
						P_DamageMobj(mo2, toucher, toucher, 1);
						continue;
					}

					// Make these APPEAR!
					// Tails 12-15-2003
					if (mo2->flags & MF_NIGHTSITEM)
					{
						// Requires Bonus Time
						if ((mo2->flags2 & MF2_STRONGBOX) && !player->bonustime)
							continue;

						if (!(mo2->flags & MF_SPECIAL) && mo2->health)
						{
							P_SetMobjState(mo2, mo2->info->seestate);
							mo2->flags |= MF_SPECIAL;
							mo2->flags &= ~MF_NIGHTSITEM;
							S_StartSound(toucher, sfx_hidden);
							continue;
						}
					}

					if (!(mo2->type == MT_NIGHTSWING || mo2->type == MT_RING || mo2->type == MT_COIN
					   || mo2->type == MT_BLUEBALL))
						continue;

					// Yay! The thing's in reach! Pull it in!
					mo2->flags |= MF_NOCLIP|MF_NOCLIPHEIGHT;
					mo2->flags2 |= MF2_NIGHTSPULL;
					P_SetTarget(&mo2->tracer, toucher);
				}
			}
			return;
		case MT_EGGCAPSULE:
			if (player->bot)
				return;

			// make sure everything is as it should be, THEN take rings from players in special stages
			if (player->pflags & PF_NIGHTSMODE && !toucher->target)
				return;

			if (player->mare != special->threshold) // wrong mare
				return;

			if (special->reactiontime > 0) // capsule already has a player attacking it, ignore
				return;

			if (G_IsSpecialStage(gamemap) && !player->exiting)
			{ // In special stages, share rings. Everyone gives up theirs to the player who touched the capsule
				for (i = 0; i < MAXPLAYERS; i++)
					if (playeringame[i] && (&players[i] != player) && players[i].mo->health > 1)
					{
						toucher->health += players[i].mo->health-1;
						player->health = toucher->health;
						players[i].mo->health = 1;
						players[i].health = players[i].mo->health;
					}
			}

			if (!(player->health > 1) || player->exiting)
				return;

			// Mark the player as 'pull into the capsule'
			P_SetTarget(&player->capsule, special);
			special->reactiontime = (player-players)+1;
			P_SetTarget(&special->target, NULL);

			// Clear text
			player->texttimer = 0;
			return;*/
		case MT_NIGHTSBUMPER:
			// Don't trigger if the stage is ended/failed
			if (player->exiting)
				return;

			if (player->bumpertime < TICRATE/4)
			{
				S_StartSound(toucher, special->info->seesound);
				if (player->pflags & PF_NIGHTSMODE)
				{
					player->bumpertime = TICRATE/2;
					if (special->threshold > 0)
						player->flyangle = (special->threshold*30)-1;
					else
						player->flyangle = special->threshold;

					player->speed = FixedMul(special->info->speed, special->scale);
					// Potentially causes axis transfer failures.
					// Also rarely worked properly anyway.
					//P_UnsetThingPosition(player->mo);
					//player->mo->x = special->x;
					//player->mo->y = special->y;
					//P_SetThingPosition(player->mo);
					toucher->z = special->z+(special->height/4);
				}
				else // More like a spring
				{
					angle_t fa;
					fixed_t xspeed, yspeed;
					const fixed_t speed = FixedMul(FixedDiv(special->info->speed*FRACUNIT,75*FRACUNIT), FixedSqrt(FixedMul(toucher->scale,special->scale)));

					player->bumpertime = TICRATE/2;

					P_UnsetThingPosition(toucher);
					toucher->x = special->x;
					toucher->y = special->y;
					P_SetThingPosition(toucher);
					toucher->z = special->z+(special->height/4);

					if (special->threshold > 0)
						fa = (FixedAngle(((special->threshold*30)-1)*FRACUNIT)>>ANGLETOFINESHIFT) & FINEMASK;
					else
						fa = 0;

					xspeed = FixedMul(FINECOSINE(fa),speed);
					yspeed = FixedMul(FINESINE(fa),speed);

					P_InstaThrust(toucher, special->angle, xspeed/10);
					toucher->momz = yspeed/11;

					toucher->angle = special->angle;

					if (player == &players[consoleplayer])
						localangle[0] = toucher->angle;
					else if (player == &players[displayplayers[1]])
						localangle[1] = toucher->angle;
					else if (player == &players[displayplayers[2]])
						localangle[2] = toucher->angle;
					else if (player == &players[displayplayers[3]])
						localangle[3] = toucher->angle;

					P_ResetPlayer(player);

					P_SetPlayerMobjState(toucher, S_KART_STND1); // SRB2kart - was S_PLAY_FALL1
				}
			}
			return;
		/*case MT_NIGHTSSUPERLOOP:
			if (player->bot || !(player->pflags & PF_NIGHTSMODE))
				return;
			if (!G_IsSpecialStage(gamemap))
				player->powers[pw_nights_superloop] = (UINT16)special->info->speed;
			else
			{
				for (i = 0; i < MAXPLAYERS; i++)
					if (playeringame[i] && players[i].pflags & PF_NIGHTSMODE)
						players[i].powers[pw_nights_superloop] = (UINT16)special->info->speed;
				if (special->info->deathsound != sfx_None)
					S_StartSound(NULL, special->info->deathsound);
			}

			// CECHO showing you what this item is
			if (player == &players[displayplayers[0]] || G_IsSpecialStage(gamemap))
			{
				HU_SetCEchoFlags(V_AUTOFADEOUT);
				HU_SetCEchoDuration(4);
				HU_DoCEcho(M_GetText("\\\\\\\\\\\\\\\\Super Paraloop"));
			}
			break;
		case MT_NIGHTSDRILLREFILL:
			if (player->bot || !(player->pflags & PF_NIGHTSMODE))
				return;
			if (!G_IsSpecialStage(gamemap))
				player->drillmeter = special->info->speed;
			else
			{
				for (i = 0; i < MAXPLAYERS; i++)
					if (playeringame[i] && players[i].pflags & PF_NIGHTSMODE)
						players[i].drillmeter = special->info->speed;
				if (special->info->deathsound != sfx_None)
					S_StartSound(NULL, special->info->deathsound);
			}

			// CECHO showing you what this item is
			if (player == &players[displayplayers[0]] || G_IsSpecialStage(gamemap))
			{
				HU_SetCEchoFlags(V_AUTOFADEOUT);
				HU_SetCEchoDuration(4);
				HU_DoCEcho(M_GetText("\\\\\\\\\\\\\\\\Drill Refill"));
			}
			break;
		case MT_NIGHTSHELPER:
			if (player->bot || !(player->pflags & PF_NIGHTSMODE))
				return;
			if (!G_IsSpecialStage(gamemap))
			{
				// A flicky orbits us now
				mobj_t *flickyobj = P_SpawnMobj(toucher->x, toucher->y, toucher->z + toucher->info->height, MT_NIGHTOPIANHELPER);
				P_SetTarget(&flickyobj->target, toucher);

				player->powers[pw_nights_helper] = (UINT16)special->info->speed;
			}
			else
			{
				mobj_t *flickyobj;
				for (i = 0; i < MAXPLAYERS; i++)
					if (playeringame[i] && players[i].mo && players[i].pflags & PF_NIGHTSMODE) {
						players[i].powers[pw_nights_helper] = (UINT16)special->info->speed;
						flickyobj = P_SpawnMobj(players[i].mo->x, players[i].mo->y, players[i].mo->z + players[i].mo->info->height, MT_NIGHTOPIANHELPER);
						P_SetTarget(&flickyobj->target, players[i].mo);
					}
				if (special->info->deathsound != sfx_None)
					S_StartSound(NULL, special->info->deathsound);
			}

			// CECHO showing you what this item is
			if (player == &players[displayplayers[0]] || G_IsSpecialStage(gamemap))
			{
				HU_SetCEchoFlags(V_AUTOFADEOUT);
				HU_SetCEchoDuration(4);
				HU_DoCEcho(M_GetText("\\\\\\\\\\\\\\\\Nightopian Helper"));
			}
			break;
		case MT_NIGHTSEXTRATIME:
			if (player->bot || !(player->pflags & PF_NIGHTSMODE))
				return;
			if (!G_IsSpecialStage(gamemap))
			{
				player->nightstime += special->info->speed;
				player->startedtime += special->info->speed;
				P_RestoreMusic(player);
			}
			else
			{
				for (i = 0; i < MAXPLAYERS; i++)
					if (playeringame[i] && players[i].pflags & PF_NIGHTSMODE)
					{
						players[i].nightstime += special->info->speed;
						players[i].startedtime += special->info->speed;
						P_RestoreMusic(&players[i]);
					}
				if (special->info->deathsound != sfx_None)
					S_StartSound(NULL, special->info->deathsound);
			}

			// CECHO showing you what this item is
			if (player == &players[displayplayers[0]] || G_IsSpecialStage(gamemap))
			{
				HU_SetCEchoFlags(V_AUTOFADEOUT);
				HU_SetCEchoDuration(4);
				HU_DoCEcho(M_GetText("\\\\\\\\\\\\\\\\Extra Time"));
			}
			break;
		case MT_NIGHTSLINKFREEZE:
			if (player->bot || !(player->pflags & PF_NIGHTSMODE))
				return;
			if (!G_IsSpecialStage(gamemap))
			{
				player->powers[pw_nights_linkfreeze] = (UINT16)special->info->speed;
				player->linktimer = 2*TICRATE;
			}
			else
			{
				for (i = 0; i < MAXPLAYERS; i++)
					if (playeringame[i] && players[i].pflags & PF_NIGHTSMODE)
					{
						players[i].powers[pw_nights_linkfreeze] += (UINT16)special->info->speed;
						players[i].linktimer = 2*TICRATE;
					}
				if (special->info->deathsound != sfx_None)
					S_StartSound(NULL, special->info->deathsound);
			}

			// CECHO showing you what this item is
			if (player == &players[displayplayers[0]] || G_IsSpecialStage(gamemap))
			{
				HU_SetCEchoFlags(V_AUTOFADEOUT);
				HU_SetCEchoDuration(4);
				HU_DoCEcho(M_GetText("\\\\\\\\\\\\\\\\Link Freeze"));
			}
			break;*/
		case MT_NIGHTSWING:
			if (G_IsSpecialStage(gamemap) && useNightsSS)
			{ // Pseudo-ring.
				S_StartSound(toucher, special->info->painsound);
				player->totalring++;
			}
			else
				S_StartSound(toucher, special->info->activesound);

			P_DoNightsScore(player);
			break;
		case MT_HOOPCOLLIDE:
			// This produces a kind of 'domino effect' with the hoop's pieces.
			for (; special->hprev != NULL; special = special->hprev); // Move to the first sprite in the hoop
			i = 0;
			for (; special->type == MT_HOOP; special = special->hnext)
			{
				special->fuse = 11;
				special->movedir = i;
				special->extravalue1 = special->target->extravalue1;
				special->extravalue2 = special->target->extravalue2;
				special->target->threshold = 4242;
				i++;
			}
			// Make the collision detectors disappear.
			{
				mobj_t *hnext;
				for (; special != NULL; special = hnext)
				{
					hnext = special->hnext;
					P_RemoveMobj(special);
				}
			}

			P_DoNightsScore(player);

			// Hoops are the only things that should add to the drill meter
			// Also, one tic's worth of drill is too much.
			if (G_IsSpecialStage(gamemap))
			{
				for (i = 0; i < MAXPLAYERS; i++)
					if (playeringame[i] && players[i].pflags & PF_NIGHTSMODE)
						players[i].drillmeter += TICRATE/2;
			}
			else if (player->bot)
				players[consoleplayer].drillmeter += TICRATE/2;
			else
				player->drillmeter += TICRATE/2;

			// Play hoop sound -- pick one depending on the current link.
			if (player->linkcount <= 5)
				S_StartSound(toucher, sfx_hoop1);
			else if (player->linkcount <= 10)
				S_StartSound(toucher, sfx_hoop2);
			else
				S_StartSound(toucher, sfx_hoop3);
			return;

// ***** //
// Mario //
// ***** //
		case MT_SHELL:
			if (special->state == &states[S_SHELL]) // Resting anim
			{
				// Kick that sucker around!
				special->angle = toucher->angle;
				P_InstaThrust(special, special->angle, FixedMul(special->info->speed, special->scale));
				S_StartSound(toucher, sfx_mario2);
				P_SetMobjState(special, S_SHELL1);
				P_SetTarget(&special->target, toucher);
				special->threshold = (3*TICRATE)/2;
			}
			return;
		case MT_AXE:
			{
				line_t junk;
				thinker_t  *th;
				mobj_t *mo2;

				if (player->bot)
					return;

				junk.tag = 649;
				EV_DoElevator(&junk, bridgeFall, false);

				// scan the remaining thinkers to find koopa
				for (th = thinkercap.next; th != &thinkercap; th = th->next)
				{
					if (th->function.acp1 != (actionf_p1)P_MobjThinker)
						continue;

					mo2 = (mobj_t *)th;
					if (mo2->type == MT_KOOPA)
					{
						mo2->momz = 5*FRACUNIT;
						break;
					}
				}
			}
			break;
		case MT_FIREFLOWER:
			if (player->bot)
				return;
			player->powers[pw_shield] |= SH_FIREFLOWER;
			toucher->color = SKINCOLOR_WHITE;
			G_GhostAddColor(player - players, GHC_FIREFLOWER);
			break;

// *************** //
// Misc touchables //
// *************** //
		case MT_STARPOST:
			if (player->bot)
				return;
			// SRB2kart - 150117
			if (player->exiting) //STOP MESSING UP MY STATS FASDFASDF
			{
				player->kartstuff[k_starpostwp] = player->kartstuff[k_waypoint];
				return;
			}
			//
			// SRB2kart: make sure the player will have enough checkpoints to touch
			if (circuitmap && special->health >= ((numstarposts/2) + player->starpostnum))
			{
				// blatant reuse of a variable that's normally unused in circuit
				if (!player->tossdelay)
				{
					S_StartSound(toucher, sfx_s26d);

					if (netgame && cv_antigrief.value)
					{
						player->grieftime += TICRATE;
					}
				}

				player->tossdelay = 3;
				return;
			}

			// We could technically have 91.1 Star Posts. 90 is cleaner.
			if (special->health > 90)
			{
				CONS_Debug(DBG_GAMELOGIC, "Bad Starpost Number!\n");
				return;
			}

			if (player->starpostnum >= special->health)
				return; // Already hit this post

			// Save the player's time and position.
			player->starposttime = player->realtime; //this makes race mode's timers work correctly whilst not affecting sp -x
			//player->starposttime = leveltime;
			player->starpostx = toucher->x>>FRACBITS;
			player->starposty = toucher->y>>FRACBITS;
			player->starpostz = special->z>>FRACBITS;
			player->starpostangle = special->angle;
			player->starpostnum = special->health;
			player->kartstuff[k_starpostflip] = special->spawnpoint->options & MTF_OBJECTFLIP;	// store flipping
			player->grieftime = 0;

			//S_StartSound(toucher, special->info->painsound);
			return;

		case MT_FAKEMOBILE:
			{
				fixed_t touchx, touchy, touchspeed;
				angle_t angle;

				if (P_AproxDistance(toucher->x-special->x, toucher->y-special->y) >
					P_AproxDistance((toucher->x-toucher->momx)-special->x, (toucher->y-toucher->momy)-special->y))
				{
					touchx = toucher->x + toucher->momx;
					touchy = toucher->y + toucher->momy;
				}
				else
				{
					touchx = toucher->x;
					touchy = toucher->y;
				}

				angle = R_PointToAngle2(special->x, special->y, touchx, touchy);
				touchspeed = P_AproxDistance(toucher->momx, toucher->momy);

				toucher->momx = P_ReturnThrustX(special, angle, touchspeed);
				toucher->momy = P_ReturnThrustY(special, angle, touchspeed);
				toucher->momz = -toucher->momz;
				if (player->pflags & PF_GLIDING)
				{
					player->pflags &= ~(PF_GLIDING|PF_JUMPED);
					P_SetPlayerMobjState(toucher, S_KART_STND1); // SRB2kart - was S_PLAY_FALL1
				}

				// Play a bounce sound?
				S_StartSound(toucher, special->info->painsound);
			}
			return;

		case MT_BLACKEGGMAN_GOOPFIRE:
			if (!player->powers[pw_flashing]) // SRB2kart
			{
				toucher->momx = 0;
				toucher->momy = 0;

				if (toucher->momz != 0)
					special->momz = toucher->momz;

				player->powers[pw_ingoop] = 2;

				if (player->pflags & PF_ITEMHANG)
				{
					P_SetTarget(&toucher->tracer, NULL);
					player->pflags &= ~PF_ITEMHANG;
				}

				P_ResetPlayer(player);

				if (special->target && special->target->state == &states[S_BLACKEGG_SHOOT1])
				{
					if (special->target->health <= 2 && P_RandomChance(FRACUNIT/2))
						P_SetMobjState(special->target, special->target->info->missilestate);
					else
						P_SetMobjState(special->target, special->target->info->raisestate);
				}
			}
			else
				player->powers[pw_ingoop] = 0;
			return;
		case MT_EGGSHIELD:
			{
				fixed_t touchx, touchy, touchspeed;
				angle_t angle;

				if (P_AproxDistance(toucher->x-special->x, toucher->y-special->y) >
					P_AproxDistance((toucher->x-toucher->momx)-special->x, (toucher->y-toucher->momy)-special->y))
				{
					touchx = toucher->x + toucher->momx;
					touchy = toucher->y + toucher->momy;
				}
				else
				{
					touchx = toucher->x;
					touchy = toucher->y;
				}

				angle = R_PointToAngle2(special->x, special->y, touchx, touchy) - special->angle;
				touchspeed = P_AproxDistance(toucher->momx, toucher->momy);

				// Blocked by the shield?
				if (!(angle > ANGLE_90 && angle < ANGLE_270))
				{
					toucher->momx = P_ReturnThrustX(special, special->angle, touchspeed);
					toucher->momy = P_ReturnThrustY(special, special->angle, touchspeed);
					toucher->momz = -toucher->momz;
					if (player->pflags & PF_GLIDING)
					{
						player->pflags &= ~(PF_GLIDING|PF_JUMPED);
						P_SetPlayerMobjState(toucher, S_KART_STND1); // SRB2kart - was S_PLAY_FALL1
					}

					// Play a bounce sound?
					S_StartSound(toucher, special->info->painsound);
					return;
				}
				else if (((player->pflags & PF_NIGHTSMODE) && (player->pflags & PF_DRILLING)) || (player->pflags & (PF_JUMPED|PF_SPINNING|PF_GLIDING))
						|| player->powers[pw_invulnerability] || player->powers[pw_super]) // Do you possess the ability to subdue the object?
				{
					// Shatter the shield!
					toucher->momx = -toucher->momx/2;
					toucher->momy = -toucher->momy/2;
					toucher->momz = -toucher->momz;
					break;
				}
			}
			return;

		case MT_BIGTUMBLEWEED:
		case MT_LITTLETUMBLEWEED:
			if (toucher->momx || toucher->momy)
			{
				special->momx = toucher->momx;
				special->momy = toucher->momy;
				special->momz = P_AproxDistance(toucher->momx, toucher->momy)/4;

				if (toucher->momz > 0)
					special->momz += toucher->momz/8;

				P_SetMobjState(special, special->info->seestate);
			}
			return;
		case MT_SMALLMACECHAIN:
		case MT_BIGMACECHAIN:
			// Is this the last link in the chain?
			if (toucher->momz > 0 || !(special->flags2 & MF2_AMBUSH)
				|| (player->pflags & PF_ITEMHANG) || (player->pflags & PF_MACESPIN))
				return;

			if (toucher->z > special->z + special->height/2)
				return;

			if (toucher->z + toucher->height/2 < special->z)
				return;

			if (player->powers[pw_flashing])
				return;

			P_ResetPlayer(player);
			P_SetTarget(&toucher->tracer, special);

			if (special->target && (special->target->type == MT_SPINMACEPOINT || special->target->type == MT_HIDDEN_SLING))
			{
				player->pflags |= PF_MACESPIN;
				S_StartSound(toucher, sfx_spin);
				P_SetPlayerMobjState(toucher, S_KART_STND1); // SRB2kart - was S_PLAY_ATK1
			}
			else
				player->pflags |= PF_ITEMHANG;

			// Can't jump first frame
			player->pflags |= PF_JUMPSTASIS;
			return;
		case MT_BIGMINE:
		case MT_BIGAIRMINE:
			// Spawn explosion!
			P_SpawnMobj(special->x, special->y, special->z, special->info->mass);
			P_RadiusAttack(special, special, special->info->damage);
			S_StartSound(special, special->info->deathsound);
			P_SetMobjState(special, special->info->deathstate);
			return;
		case MT_SPECIALSPIKEBALL:
			if (!(!useNightsSS && G_IsSpecialStage(gamemap))) // Only for old special stages
			{
				P_DamageMobj(toucher, special, special, 1);
				return;
			}

			if (player->powers[pw_invulnerability] || player->powers[pw_flashing]
			|| (player->powers[pw_super] && !(ALL7EMERALDS(player->powers[pw_emeralds]))))
				return;
			if (player->powers[pw_shield] || player->bot)  //If One-Hit Shield
			{
				P_RemoveShield(player);
				S_StartSound(toucher, sfx_shldls); // Ba-Dum! Shield loss.
			}
			else
			{
				P_PlayRinglossSound(toucher);
				if (toucher->health > 10)
					toucher->health -= 10;
				else
					toucher->health = 1;
				player->health = toucher->health;
			}

			P_DoPlayerPain(player, special, NULL);
			return;
		case MT_EGGMOBILE2_POGO:
			// sanity checks
			if (!special->target || !special->target->health)
				return;
			// Goomba Stomp'd!
			if (special->target->momz < 0)
			{
				P_DamageMobj(toucher, special, special->target, 1);
				//special->target->momz = -special->target->momz;
				special->target->momx = special->target->momy = 0;
				special->target->momz = 0;
				special->target->flags |= MF_NOGRAVITY;
				P_SetMobjState(special->target, special->info->raisestate);
				S_StartSound(special->target, special->info->activesound);
				P_RemoveMobj(special);
			}
			return;

		case MT_EXTRALARGEBUBBLE:
			return; // SRB2kart - don't need bubbles mucking with the player
			if ((player->powers[pw_shield] & SH_NOSTACK) == SH_ELEMENTAL)
				return;
			if (maptol & TOL_NIGHTS)
				return;
			if (mariomode)
				return;
			else if (toucher->eflags & MFE_VERTICALFLIP)
			{
				if (special->z+special->height < toucher->z + toucher->height / 3
				 || special->z+special->height > toucher->z + (toucher->height*2/3))
					return; // Only go in the mouth
			}
			else if (special->z < toucher->z + toucher->height / 3
				|| special->z > toucher->z + (toucher->height*2/3))
				return; // Only go in the mouth

			/* // SRB2kart - Can't drown.
			// Eaten by player!
			if (player->powers[pw_underwater] && player->powers[pw_underwater] <= 12*TICRATE + 1)
				P_RestoreMusic(player);

			if (player->powers[pw_underwater] < underwatertics + 1)
				player->powers[pw_underwater] = underwatertics + 1;
			*/

			/*
			if (!player->climbing)
			{
				P_SetPlayerMobjState(toucher, S_PLAY_GASP);
				P_ResetPlayer(player);
			}
			*/

			toucher->momx = toucher->momy = toucher->momz = 0;
			break;

		case MT_WATERDROP:
			if (special->state == &states[special->info->spawnstate])
			{
				special->z = toucher->z+toucher->height-FixedMul(8*FRACUNIT, special->scale);
				special->momz = 0;
				special->flags |= MF_NOGRAVITY;
				P_SetMobjState (special, special->info->deathstate);
				S_StartSound (special, special->info->deathsound+(P_RandomKey(special->info->mass)));
			}
			return;

		default: // SOC or script pickup
			if (player->bot)
				return;
			P_SetTarget(&special->target, toucher);
			break;
		}
	}

	if (!P_MobjWasRemoved(special))
	{
		S_StartSound(toucher, special->info->deathsound); // was NULL, but changed to player so you could hear others pick up rings
		P_KillMobj(special, NULL, toucher);
	}
}

/** Checks if the level timer is over the timelimit and the round should end,
  * unless you are in overtime. In which case leveltime may stretch out beyond
  * timelimitintics and overtime's status will be checked here each tick.
  * Verify that the value of ::cv_timelimit is greater than zero before
  * calling this function.
  *
  * \sa cv_timelimit, P_CheckPointLimit, P_UpdateSpecials
  */
void P_CheckTimeLimit(void)
{
	INT32 i, k;

	if (!cv_timelimit.value)
		return;

	if (!(multiplayer || netgame))
		return;

	if (G_RaceGametype())
		return;

	if (leveltime < (timelimitintics + starttime))
		return;

	if (gameaction == ga_completed)
		return;

	//Tagmode round end but only on the tic before the
	//XD_EXITLEVEL packet is received by all players.
	/*if (G_TagGametype())
	{
		if (leveltime == (timelimitintics + 1))
		{
			for (i = 0; i < MAXPLAYERS; i++)
			{
				if (!playeringame[i] || players[i].spectator
				 || (players[i].pflags & PF_TAGGED) || (players[i].pflags & PF_TAGIT))
					continue;

				CONS_Printf(M_GetText("%s received double points for surviving the round.\n"), player_names[i]);
				P_AddPlayerScore(&players[i], players[i].score);
			}
		}
	}

	//Optional tie-breaker for Match/CTF
	else*/ if (cv_overtime.value)
	{
		INT32 playerarray[MAXPLAYERS];
		INT32 tempplayer = 0;
		INT32 spectators = 0;
		INT32 playercount = 0;

		//Figure out if we have enough participating players to care.
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (players[i].exiting)
				return;
			if (playeringame[i] && players[i].spectator)
				spectators++;
		}

		if ((D_NumPlayers() - spectators) > 1)
		{
			// Play the starpost sfx after the first second of overtime.
			if (gamestate == GS_LEVEL && (leveltime == (timelimitintics + TICRATE)))
				S_StartSound(NULL, sfx_strpst);

			// Normal Match
			if (!G_GametypeHasTeams())
			{
				//Store the nodes of participating players in an array.
				for (i = 0; i < MAXPLAYERS; i++)
				{
					if (playeringame[i] && !players[i].spectator)
					{
						playerarray[playercount] = i;
						playercount++;
					}
				}

				if (playercount > MAXPLAYERS)
					playercount = MAXPLAYERS;

				//Sort 'em.
				for (i = 1; i < playercount; i++)
				{
					for (k = i; k < playercount; k++)
					{
						if (players[playerarray[i-1]].marescore < players[playerarray[k]].marescore)
						{
							tempplayer = playerarray[i-1];
							playerarray[i-1] = playerarray[k];
							playerarray[k] = tempplayer;
						}
					}
				}

				//End the round if the top players aren't tied.
				if (players[playerarray[0]].marescore == players[playerarray[1]].marescore)
					return;
			}
			else
			{
				//In team match and CTF, determining a tie is much simpler. =P
				if (redscore == bluescore)
					return;
			}
		}
	}

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i] || players[i].spectator)
			continue;
		if (players[i].exiting)
			return;
		P_DoPlayerExit(&players[i]);
	}

	/*if (server)
		SendNetXCmd(XD_EXITLEVEL, NULL, 0);*/
}

/** Checks if a player's score is over the pointlimit and the round should end.
  * Verify that the value of ::cv_pointlimit is greater than zero before
  * calling this function.
  *
  * \sa cv_pointlimit, P_CheckTimeLimit, P_UpdateSpecials
  */
void P_CheckPointLimit(void)
{
	INT32 i;

	if (!cv_pointlimit.value)
		return;

	if (!(multiplayer || netgame))
		return;

	if (G_RaceGametype())
		return;

	// pointlimit is nonzero, check if it's been reached by this player
	/*if (G_GametypeHasTeams())
	{
		// Just check both teams
		if ((UINT32)cv_pointlimit.value <= redscore || (UINT32)cv_pointlimit.value <= bluescore)
		{
			if (server)
				SendNetXCmd(XD_EXITLEVEL, NULL, 0);
		}
	}
	else*/
	{
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!playeringame[i] || players[i].spectator)
				continue;

			if ((UINT32)cv_pointlimit.value <= players[i].marescore)
			{
				for (i = 0; i < MAXPLAYERS; i++) // AAAAA nested loop using the same iteration variable ;;
				{
					if (!playeringame[i] || players[i].spectator)
						continue;
					if (players[i].exiting)
						return;
					P_DoPlayerExit(&players[i]);
				}

				/*if (server)
					SendNetXCmd(XD_EXITLEVEL, NULL, 0);*/
				return; // good thing we're leaving the function immediately instead of letting the loop get mangled!
			}
		}
	}
}

/*Checks for untagged remaining players in both tag derivitave modes.
 *If no untagged players remain, end the round.
 *Also serves as error checking if the only IT player leaves.*/
/*void P_CheckSurvivors(void)
{
	INT32 i;
	INT32 survivors = 0;
	INT32 taggers = 0;
	INT32 spectators = 0;
	INT32 survivorarray[MAXPLAYERS];

	if (!D_NumPlayers()) //no players in the game, no check performed.
		return;

	for (i=0; i < MAXPLAYERS; i++) //figure out counts of taggers, survivors and spectators.
	{
		if (playeringame[i])
		{
			if (players[i].spectator)
				spectators++;
			else if (players[i].pflags & PF_TAGIT)
				taggers++;
			else if (!(players[i].pflags & PF_TAGGED))
			{
				survivorarray[survivors] = i;
				survivors++;
			}
		}
	}

	if (!taggers) //If there are no taggers, pick a survivor at random to be it.
	{
		// Exception for hide and seek. If a round has started and the IT player leaves, end the round.
		if (gametype == GT_HIDEANDSEEK && (leveltime >= (hidetime * TICRATE)))
		{
			CONS_Printf(M_GetText("The IT player has left the game.\n"));
			if (server)
				SendNetXCmd(XD_EXITLEVEL, NULL, 0);

			return;
		}

		if (survivors)
		{
			INT32 newtagger = survivorarray[P_RandomKey(survivors)];

			CONS_Printf(M_GetText("%s is now IT!\n"), player_names[newtagger]); // Tell everyone who is it!
			players[newtagger].pflags |= PF_TAGIT;

			survivors--; //Get rid of the guy we just made IT.

			//Yeah, we have an eligible tagger, but we may not have anybody for him to tag!
			//If there is only one guy waiting on the game to fill or spectators to enter game, don't bother.
			if (!survivors && (D_NumPlayers() - spectators) > 1)
			{
				CONS_Printf(M_GetText("All players have been tagged!\n"));
				if (server)
					SendNetXCmd(XD_EXITLEVEL, NULL, 0);
			}

			return;
		}

		//If we reach this point, no player can replace the one that was IT.
		//Unless it is one player waiting on a game, end the round.
		if ((D_NumPlayers() - spectators) > 1)
		{
			CONS_Printf(M_GetText("There are no players able to become IT.\n"));
			if (server)
				SendNetXCmd(XD_EXITLEVEL, NULL, 0);
		}

		return;
	}

	//If there are taggers, but no survivors, end the round.
	//Except when the tagger is by himself and the rest of the game are spectators.
	if (!survivors && (D_NumPlayers() - spectators) > 1)
	{
		CONS_Printf(M_GetText("All players have been tagged!\n"));
		if (server)
			SendNetXCmd(XD_EXITLEVEL, NULL, 0);
	}
}*/

// Checks whether or not to end a race netgame.
boolean P_CheckRacers(void)
{
	INT32 i, j, numplayersingame = 0;

	// Check if all the players in the race have finished. If so, end the level.
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i] || players[i].spectator || players[i].exiting || !players[i].lives)
			continue;

		break;
	}

	if (i == MAXPLAYERS) // finished
	{
		racecountdown = exitcountdown = 0;
		return true;
	}

	if (cv_karteliminatelast.value)
	{
		for (j = 0; j < MAXPLAYERS; j++)
		{
			if (!playeringame[j] || players[j].spectator)
				continue;
			numplayersingame++;
		}

		if (numplayersingame > 1 && nospectategrief > 0 && numplayersingame >= nospectategrief) // prevent spectate griefing
		{
			// check if we just got unlucky and there was only one guy who was a problem
			for (j = i+1; j < MAXPLAYERS; j++)
			{
				if (!playeringame[j] || players[j].spectator || players[j].exiting || !players[j].lives)
					continue;

				break;
			}

			if (j == MAXPLAYERS) // finish anyways, force a time over
			{
				P_DoTimeOver(&players[i]);
				racecountdown = exitcountdown = 0;
				return true;
			}
		}
	}

	if (!racecountdown) // Check to see if the winners have finished, to set countdown.
	{
		UINT8 numingame = 0, numexiting = 0;
		UINT8 winningpos = 1;

		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!playeringame[i] || players[i].spectator)
				continue;
			numingame++;
			if (players[i].exiting)
				numexiting++;
		}

		winningpos = max(1, numingame/2);
		if (numingame % 2) // any remainder?
			winningpos++;

		if (numexiting >= winningpos)
			racecountdown = (((netgame || multiplayer) ? cv_countdowntime.value : 30)*TICRATE) + 1; // 30 seconds to finish, get going!
	}

	return false;
}

/** Kills an object.
  *
  * \param target    The victim.
  * \param inflictor The attack weapon. May be NULL (environmental damage).
  * \param source    The attacker. May be NULL.
  * \todo Cleanup, refactor, split up.
  * \sa P_DamageMobj
  */
void P_KillMobj(mobj_t *target, mobj_t *inflictor, mobj_t *source)
{
	mobjtype_t item;
	mobj_t *mo;

	//if (inflictor && (inflictor->type == MT_SHELL || inflictor->type == MT_FIREBALL))
	//	P_SetTarget(&target->tracer, inflictor);

	if (!useNightsSS && G_IsSpecialStage(gamemap) && target->player && sstimer > 6)
		sstimer = 6; // Just let P_Ticker take care of the rest.

	if (target->flags & (MF_ENEMY|MF_BOSS))
		target->momx = target->momy = target->momz = 0;

	// SRB2kart
	if (target->type != MT_PLAYER && !(target->flags & MF_MONITOR)
		 && !(target->type == MT_ORBINAUT || target->type == MT_ORBINAUT_SHIELD
		 || target->type == MT_JAWZ || target->type == MT_JAWZ_DUD || target->type == MT_JAWZ_SHIELD
		 || target->type == MT_BANANA || target->type == MT_BANANA_SHIELD
		 || target->type == MT_EGGMANITEM || target->type == MT_EGGMANITEM_SHIELD
		 || target->type == MT_BALLHOG || target->type == MT_SPB)) // kart dead items
		target->flags |= MF_NOGRAVITY; // Don't drop Tails 03-08-2000
	else
		target->flags &= ~MF_NOGRAVITY; // lose it if you for whatever reason have it, I'm looking at you shields
	//

	if (target->flags2 & MF2_NIGHTSPULL)
		P_SetTarget(&target->tracer, NULL);

	// dead target is no more shootable
	target->flags &= ~(MF_SHOOTABLE|MF_FLOAT|MF_SPECIAL);
	target->flags2 &= ~(MF2_SKULLFLY|MF2_NIGHTSPULL);
	target->health = 0; // This makes it easy to check if something's dead elsewhere.

	if (LUAh_MobjDeath(target, inflictor, source) || P_MobjWasRemoved(target))
		return;

	// SRB2kart
	// I wish I knew a better way to do this
	if (target->target && target->target->player && target->target->player->mo)
	{
		if (target->target->player->kartstuff[k_eggmanheld] && target->type == MT_EGGMANITEM_SHIELD)
			target->target->player->kartstuff[k_eggmanheld] = 0;

		if (target->target->player->kartstuff[k_itemheld])
		{
			if ((target->type == MT_BANANA_SHIELD && target->target->player->kartstuff[k_itemtype] == KITEM_BANANA) // trail items
				|| (target->type == MT_SSMINE_SHIELD && target->target->player->kartstuff[k_itemtype] == KITEM_MINE)
				|| (target->type == MT_SINK_SHIELD && target->target->player->kartstuff[k_itemtype] == KITEM_KITCHENSINK))
			{
				if (target->movedir != 0 && target->movedir < (UINT16)target->target->player->kartstuff[k_itemamount])
				{
					if (target->target->hnext)
						K_KillBananaChain(target->target->hnext, inflictor, source);
					target->target->player->kartstuff[k_itemamount] = 0;
				}
				else
					target->target->player->kartstuff[k_itemamount]--;
			}
			else if ((target->type == MT_ORBINAUT_SHIELD && target->target->player->kartstuff[k_itemtype] == KITEM_ORBINAUT) // orbit items
				|| (target->type == MT_JAWZ_SHIELD && target->target->player->kartstuff[k_itemtype] == KITEM_JAWZ))
			{
				target->target->player->kartstuff[k_itemamount]--;
				if (target->lastlook != 0)
				{
					K_RepairOrbitChain(target);
				}
			}

			if (target->target->player->kartstuff[k_itemamount] < 0)
				target->target->player->kartstuff[k_itemamount] = 0;

			if (!target->target->player->kartstuff[k_itemamount])
				target->target->player->kartstuff[k_itemheld] = 0;

			if (target->target->hnext == target)
				P_SetTarget(&target->target->hnext, NULL);
		}
	}
	// Above block does not clean up rocket sneakers when a player dies, so we need to do it here target->target is null when using rocket sneakers
	if (target->player)
		K_DropRocketSneaker(target->player);

	// Let EVERYONE know what happened to a player! 01-29-2002 Tails
	if (target->player && !target->player->spectator)
	{
		if (metalrecording) // Ack! Metal Sonic shouldn't die! Cut the tape, end recording!
			G_StopMetalRecording();
		/*if (gametype == GT_MATCH && cv_match_scoring.value == 0 // note, no team match suicide penalty
			&& ((target == source) || (source == NULL && inflictor == NULL) || (source && !source->player)))
		{ // Suicide penalty - Not in Kart
			if (target->player->score >= 50)
				target->player->score -= 50;
			else
				target->player->score = 0;
		}*/

		target->flags2 &= ~MF2_DONTDRAW;
	}

	// if killed by a player
	if (source && source->player)
	{
		if (target->flags & MF_MONITOR || target->type == MT_RANDOMITEM)
		{
			P_SetTarget(&target->target, source);
			source->player->numboxes++;
			if (cv_itemrespawn.value && (netgame || multiplayer))
			{
				target->fuse = cv_itemrespawntime.value*TICRATE + 2; // Random box generation
			}
		}

		// Award Score Tails
		/*{ // Enemies shouldn't award points in Kart
			INT32 score = 0;

			if (maptol & TOL_NIGHTS) // Enemies always worth 200, bosses don't do anything.
			{
				if ((target->flags & MF_ENEMY) && !(target->flags & (MF_MISSILE|MF_BOSS)))
				{
					score = 200;

					if (source->player->bonustime)
						score *= 2;

					// Also, add to the link.
					// I don't know if NiGHTS did this, but
					// Sonic Time Attacked did and it seems like a good enough incentive
					// to make people want to actually dash towards/paraloop enemies
					if (++source->player->linkcount > source->player->maxlink)
						source->player->maxlink = source->player->linkcount;
					source->player->linktimer = 2*TICRATE;
				}
			}
			else
			{
				if (target->flags & MF_BOSS)
					score = 1000;
				else if ((target->flags & MF_ENEMY) && !(target->flags & MF_MISSILE))
				{
					mobj_t *scoremobj;
					UINT32 scorestate = mobjinfo[MT_SCORE].spawnstate;

					scoremobj = P_SpawnMobj(target->x, target->y, target->z + (target->height / 2), MT_SCORE);

					// On ground? No chain starts.
					if (!source->player->powers[pw_invulnerability] && P_IsObjectOnGround(source))
					{
						source->player->scoreadd = 0;
						score = 100;
					}
					// Mario Mode has Mario-like chain point values
					else if (mariomode) switch (++source->player->scoreadd)
					{
						case 1: score = 100;  break;
						case 2: score = 200;  scorestate += 1; break;
						case 3: score = 400;  scorestate += 5; break;
						case 4: score = 800;  scorestate += 6; break;
						case 5: score = 1000; scorestate += 3; break;
						case 6: score = 2000; scorestate += 7; break;
						case 7: score = 4000; scorestate += 8; break;
						case 8: score = 8000; scorestate += 9; break;
						default: // 1up for a chain this long
							if (modeattacking) // but 1ups don't exist in record attack!
							{ // So we just go back to 10k points.
								score = 10000; scorestate += 4; break;
							}
							P_GivePlayerLives(source->player, 1);
							P_PlayLivesJingle(source->player);
							scorestate += 10;
							break;
					}
					// More Sonic-like point system
					else switch (++source->player->scoreadd)
					{
						case 1:  score = 100;   break;
						case 2:  score = 200;   scorestate += 1; break;
						case 3:  score = 500;   scorestate += 2; break;
						case 4: case 5: case 6: case 7: case 8: case 9:
						case 10: case 11: case 12: case 13: case 14:
						         score = 1000;  scorestate += 3; break;
						default: score = 10000; scorestate += 4; break;
					}

					P_SetMobjState(scoremobj, scorestate);
				}
			}

			P_AddPlayerScore(source->player, score);
		}*/
	}

	// if a player avatar dies...
	if (target->player)
	{
		target->flags &= ~(MF_SOLID|MF_SHOOTABLE); // does not block
		P_UnsetThingPosition(target);
		target->flags |= MF_NOBLOCKMAP|MF_NOCLIPHEIGHT;
		P_SetThingPosition(target);

		if (!target->player->bot && !G_IsSpecialStage(gamemap)
		 && G_GametypeUsesLives())
		{
			target->player->lives -= 1; // Lose a life Tails 03-11-2000

			if (target->player->lives <= 0) // Tails 03-14-2000
			{
				if (P_IsLocalPlayer(target->player)/* && target->player == &players[consoleplayer] */)
				{
					S_StopMusic(); // Stop the Music! Tails 03-14-2000
					S_ChangeMusicInternal("gmover", false); // Yousa dead now, Okieday? Tails 03-14-2000
				}
			}
		}
		target->player->playerstate = PST_DEAD;

		if (target->player == &players[consoleplayer])
		{
			// don't die in auto map,
			// switch view prior to dying
			if (automapactive)
				AM_Stop();

			//added : 22-02-98: recenter view for next life...
			localaiming[0] = 0;
		}
		if (target->player == &players[displayplayers[1]])
		{
			// added : 22-02-98: recenter view for next life...
			localaiming[1] = 0;
		}
		if (target->player == &players[displayplayers[2]])
			localaiming[2] = 0;
		if (target->player == &players[displayplayers[3]])
			localaiming[3] = 0;

		//tag deaths handled differently in suicide cases. Don't count spectators!
		/*if (G_TagGametype()
		 && !(target->player->pflags & PF_TAGIT) && (!source || !source->player) && !(target->player->spectator))
		{
			// if you accidentally die before you run out of time to hide, ignore it.
			// allow them to try again, rather than sitting the whole thing out.
			if (leveltime >= hidetime * TICRATE)
			{
				if (gametype == GT_TAG)//suiciding in survivor makes you IT.
				{
					target->player->pflags |= PF_TAGIT;
					CONS_Printf(M_GetText("%s is now IT!\n"), player_names[target->player-players]); // Tell everyone who is it!
					P_CheckSurvivors();
				}
				else
				{
					if (!(target->player->pflags & PF_TAGGED))
					{
						//otherwise, increment the tagger's score.
						//in hide and seek, suiciding players are counted as found.
						INT32 w;

						for (w=0; w < MAXPLAYERS; w++)
						{
							if (players[w].pflags & PF_TAGIT)
								P_AddPlayerScore(&players[w], 1);
						}

						target->player->pflags |= PF_TAGGED;
						CONS_Printf(M_GetText("%s was found!\n"), player_names[target->player-players]);
						P_CheckSurvivors();
					}
				}
			}
		}
		else*/ if (G_BattleGametype())
			K_CheckBumpers();

		target->player->kartstuff[k_pogospring] = 0;
	}

	if (source && target && target->player && source->player)
		P_PlayVictorySound(source); // Killer laughs at you. LAUGHS! BWAHAHAHA!

	// Drop stuff.
	// This determines the kind of object spawned
	// during the death frame of a thing.
	if (!mariomode // Don't show birds, etc. in Mario Mode Tails 12-23-2001
	&& target->flags & MF_ENEMY)
	{
		if (cv_soniccd.value)
			item = MT_SEED;
		else
		{
			INT32 prandom;

			switch (target->type)
			{
				case MT_REDCRAWLA:
				case MT_GOLDBUZZ:
				case MT_SKIM:
				case MT_UNIDUS:
					item = MT_BUNNY;
					break;

				case MT_BLUECRAWLA:
				case MT_JETTBOMBER:
				case MT_GFZFISH:
					item = MT_BIRD;
					break;

				case MT_JETTGUNNER:
				case MT_CRAWLACOMMANDER:
				case MT_REDBUZZ:
				case MT_DETON:
					item = MT_MOUSE;
					break;

				case MT_GSNAPPER:
				case MT_EGGGUARD:
				case MT_SPRINGSHELL:
					item = MT_COW;
					break;

				case MT_MINUS:
				case MT_VULTURE:
				case MT_POINTY:
				case MT_YELLOWSHELL:
					item = MT_CHICKEN;
					break;

				case MT_AQUABUZZ:
					item = MT_REDBIRD;
					break;

				default:
					if (target->info->doomednum)
						prandom = target->info->doomednum%5; // "Random" animal for new enemies.
					else
						prandom = P_RandomKey(5); // No placable object, just use a random number.

					switch(prandom)
					{
						default: item = MT_BUNNY; break;
						case 1: item = MT_BIRD; break;
						case 2: item = MT_MOUSE; break;
						case 3: item = MT_COW; break;
						case 4: item = MT_CHICKEN; break;
					}
					break;
			}
		}

		mo = P_SpawnMobj(target->x, target->y, target->z + (target->height / 2) - FixedMul(mobjinfo[item].height / 2, target->scale), item);
		mo->destscale = target->scale;
		P_SetScale(mo, mo->destscale);
	}
	// Other death animation effects
	else switch(target->type)
	{
		case MT_BOUNCEPICKUP:
		case MT_RAILPICKUP:
		case MT_AUTOPICKUP:
		case MT_EXPLODEPICKUP:
		case MT_SCATTERPICKUP:
		case MT_GRENADEPICKUP:
			P_SetObjectMomZ(target, FRACUNIT, false);
			target->fuse = target->info->damage;
			break;

		case MT_EGGTRAP:
			// Time for birdies! Yaaaaaaaay!
			target->fuse = TICRATE*2;
			break;

		case MT_PLAYER:
			target->momx = target->momy = target->momz = 0;

			if (target->player && target->player->pflags & PF_TIMEOVER)
				break;

			target->fuse = TICRATE*3; // timer before mobj disappears from view (even if not an actual player)
			if (!(source && source->type == MT_NULL && source->threshold == 42)) // Don't jump up when drowning
				P_SetObjectMomZ(target, 14*FRACUNIT, false);

			if (source && source->type == MT_NULL && source->threshold == 42) // drowned
				S_StartSound(target, sfx_drown);
			else if (source && (source->type == MT_SPIKE || (source->type == MT_NULL && source->threshold == 43))) // Spikes
				S_StartSound(target, sfx_spkdth);
			else
				P_PlayDeathSound(target);
			break;

		// SRB2Kart:
		case MT_SMK_ICEBLOCK:
			{
				mobj_t *cur = target->hnext;
				while (cur && !P_MobjWasRemoved(cur))
				{
					P_SetMobjState(cur, S_SMK_ICEBLOCK2);
					cur = cur->hnext;
				}
				target->fuse = 10;
				S_StartSound(target, sfx_s3k80);
			}
			break;

		default:
			break;
	}

	// Enemy drops that ALWAYS occur regardless of mode
	if (target->type == MT_AQUABUZZ) // Additionally spawns breathable bubble for players to get
	{
		if (inflictor && inflictor->player // did a player kill you? Spawn relative to the player so he's bound to get it
		&& P_AproxDistance(inflictor->x - target->x, inflictor->y - target->y) <= inflictor->radius + target->radius + FixedMul(8*FRACUNIT, inflictor->scale) // close enough?
		&& inflictor->z <= target->z + target->height + FixedMul(8*FRACUNIT, inflictor->scale)
		&& inflictor->z + inflictor->height >= target->z - FixedMul(8*FRACUNIT, inflictor->scale))
			mo = P_SpawnMobj(inflictor->x + inflictor->momx, inflictor->y + inflictor->momy, inflictor->z + (inflictor->height / 2) + inflictor->momz, MT_EXTRALARGEBUBBLE);
		else
			mo = P_SpawnMobj(target->x, target->y, target->z, MT_EXTRALARGEBUBBLE);
		mo->destscale = target->scale;
		P_SetScale(mo, mo->destscale);
	}
	else if (target->type == MT_YELLOWSHELL) // Spawns a spring that falls to the ground
	{
		mobjtype_t spawnspring = MT_YELLOWSPRING;
		fixed_t spawnheight = target->z;
		if (!(target->eflags & MFE_VERTICALFLIP))
			spawnheight += target->height;

		mo = P_SpawnMobj(target->x, target->y, spawnheight, spawnspring);
		mo->destscale = target->scale;
		P_SetScale(mo, mo->destscale);

		if (target->flags2 & MF2_OBJECTFLIP)
			mo->flags2 |= MF2_OBJECTFLIP;
	}

	if (target->type == MT_EGGMOBILE3)
	{
		thinker_t *th;
		UINT32 i = 0; // to check how many clones we've removed

		// scan the thinkers to make sure all the old pinch dummies are gone on death
		// this can happen if the boss was hurt earlier than expected
		for (th = thinkercap.next; th != &thinkercap; th = th->next)
		{
			if (th->function.acp1 != (actionf_p1)P_MobjThinker)
				continue;

			mo = (mobj_t *)th;
			if (mo->type == (mobjtype_t)target->info->mass && mo->tracer == target)
			{
				P_RemoveMobj(mo);
				i++;
			}
			if (i == 2) // we've already removed 2 of these, let's stop now
				break;
		}
	}

	if ((target->type == MT_JAWZ || target->type == MT_JAWZ_DUD || target->type == MT_JAWZ_SHIELD) && !(target->flags2 & MF2_AMBUSH))
	{
		target->z += P_MobjFlip(target)*20*target->scale;
	}

	// kill tracer
	if (target->type == MT_FROGGER)
	{
		if (target->tracer && !P_MobjWasRemoved(target->tracer))
			P_KillMobj(target->tracer, inflictor, source);
	}

	if (target->type == MT_FROGGER || target->type == MT_ROBRA_HEAD || target->type == MT_BLUEROBRA_HEAD) // clean hnext list
	{
		mobj_t *cur = target->hnext;
		while (cur && !P_MobjWasRemoved(cur))
		{
			P_KillMobj(cur, inflictor, source);
			cur = cur->hnext;
		}
	}

	// Bounce up on death
	if (target->type == MT_SMK_PIPE || target->type == MT_SMK_MOLE || target->type == MT_SMK_THWOMP)
	{
		target->flags &= (~MF_NOGRAVITY);

		if (target->eflags & MFE_VERTICALFLIP)
			target->z -= target->height;
		else
			target->z += target->height;

		S_StartSound(target, target->info->deathsound);

		P_SetObjectMomZ(target, 8<<FRACBITS, false);
		if (inflictor)
			P_InstaThrust(target, R_PointToAngle2(inflictor->x, inflictor->y, target->x, target->y)+ANGLE_90, 16<<FRACBITS);
	}

	if (target->type == MT_SPIKE && inflictor && target->info->deathstate != S_NULL)
	{
		const fixed_t x=target->x,y=target->y,z=target->z;
		const fixed_t scale=target->scale;
		const boolean flip=(target->eflags & MFE_VERTICALFLIP) == MFE_VERTICALFLIP;
		S_StartSound(target,target->info->deathsound);

		P_SetMobjState(target, target->info->deathstate);
		target->health = 0;
		target->angle = inflictor->angle + ANGLE_90;
		P_UnsetThingPosition(target);
		target->flags = MF_NOCLIP;
		target->x += P_ReturnThrustX(target, target->angle, FixedMul(8*FRACUNIT, target->scale));
		target->y += P_ReturnThrustY(target, target->angle, FixedMul(8*FRACUNIT, target->scale));
		if (flip)
			target->z -= FixedMul(12*FRACUNIT, target->scale);
		else
			target->z += FixedMul(12*FRACUNIT, target->scale);
		P_SetThingPosition(target);
		P_InstaThrust(target,target->angle,FixedMul(2*FRACUNIT, target->scale));
		target->momz = FixedMul(7*FRACUNIT, target->scale);
		if (flip)
			target->momz = -target->momz;

		if (flip)
		{
			target = P_SpawnMobj(x,y,z-FixedMul(12*FRACUNIT, target->scale),MT_SPIKE);
			target->eflags |= MFE_VERTICALFLIP;
		}
		else
			target = P_SpawnMobj(x,y,z+FixedMul(12*FRACUNIT, target->scale),MT_SPIKE);
		P_SetMobjState(target, target->info->deathstate);
		target->health = 0;
		target->angle = inflictor->angle - ANGLE_90;
		target->destscale = scale;
		P_SetScale(target, scale);
		P_UnsetThingPosition(target);
		target->flags = MF_NOCLIP;
		target->x += P_ReturnThrustX(target, target->angle, FixedMul(8*FRACUNIT, target->scale));
		target->y += P_ReturnThrustY(target, target->angle, FixedMul(8*FRACUNIT, target->scale));
		P_SetThingPosition(target);
		P_InstaThrust(target,target->angle,FixedMul(2*FRACUNIT, target->scale));
		target->momz = FixedMul(7*FRACUNIT, target->scale);
		if (flip)
			target->momz = -target->momz;

		if (target->info->xdeathstate != S_NULL)
		{
			target = P_SpawnMobj(x,y,z,MT_SPIKE);
			if (flip)
				target->eflags |= MFE_VERTICALFLIP;
			P_SetMobjState(target, target->info->xdeathstate);
			target->health = 0;
			target->angle = inflictor->angle + ANGLE_90;
			target->destscale = scale;
			P_SetScale(target, scale);
			P_UnsetThingPosition(target);
			target->flags = MF_NOCLIP;
			target->x += P_ReturnThrustX(target, target->angle, FixedMul(8*FRACUNIT, target->scale));
			target->y += P_ReturnThrustY(target, target->angle, FixedMul(8*FRACUNIT, target->scale));
			P_SetThingPosition(target);
			P_InstaThrust(target,target->angle,FixedMul(4*FRACUNIT, target->scale));
			target->momz = FixedMul(6*FRACUNIT, target->scale);
			if (flip)
				target->momz = -target->momz;

			target = P_SpawnMobj(x,y,z,MT_SPIKE);
			if (flip)
				target->eflags |= MFE_VERTICALFLIP;
			P_SetMobjState(target, target->info->xdeathstate);
			target->health = 0;
			target->angle = inflictor->angle - ANGLE_90;
			target->destscale = scale;
			P_SetScale(target, scale);
			P_UnsetThingPosition(target);
			target->flags = MF_NOCLIP;
			target->x += P_ReturnThrustX(target, target->angle, FixedMul(8*FRACUNIT, target->scale));
			target->y += P_ReturnThrustY(target, target->angle, FixedMul(8*FRACUNIT, target->scale));
			P_SetThingPosition(target);
			P_InstaThrust(target,target->angle,FixedMul(4*FRACUNIT, target->scale));
			target->momz = FixedMul(6*FRACUNIT, target->scale);
			if (flip)
				target->momz = -target->momz;
		}
	}
	else if (target->player)
		P_SetPlayerMobjState(target, target->info->deathstate);
	else
#ifdef DEBUG_NULL_DEATHSTATE
		P_SetMobjState(target, S_NULL);
#else
		P_SetMobjState(target, target->info->deathstate);
#endif

	/** \note For player, the above is redundant because of P_SetMobjState (target, S_PLAY_DIE1)
	   in P_DamageMobj()
	   Graue 12-22-2003 */
}

static inline void P_NiGHTSDamage(mobj_t *target, mobj_t *source)
{
	player_t *player = target->player;
	tic_t oldnightstime = player->nightstime;

	if (!player->powers[pw_flashing]
		&& !(player->pflags & PF_GODMODE))
	{
		angle_t fa;

		player->angle_pos = player->old_angle_pos;
		player->speed /= 5;
		player->flyangle += 180; // Shuffle's BETTERNIGHTSMOVEMENT?
		player->flyangle %= 360;

		if (G_RaceGametype())
			player->drillmeter -= 5*20;
		else
		{
			if (source && source->player)
			{
				if (player->nightstime > 20*TICRATE)
					player->nightstime -= 20*TICRATE;
				else
					player->nightstime = 1;
			}
			else
			{
				if (player->nightstime > 5*TICRATE)
					player->nightstime -= 5*TICRATE;
				else
					player->nightstime = 1;
			}
		}

		if (player->pflags & PF_TRANSFERTOCLOSEST)
		{
			target->momx = -target->momx;
			target->momy = -target->momy;
		}
		else
		{
			fa = player->old_angle_pos>>ANGLETOFINESHIFT;

			target->momx = FixedMul(FINECOSINE(fa),target->target->radius);
			target->momy = FixedMul(FINESINE(fa),target->target->radius);
		}

		player->powers[pw_flashing] = K_GetKartFlashing(player);
		P_SetMobjState(target->tracer, S_NIGHTSHURT1);
		S_StartSound(target, sfx_nghurt);

		if (oldnightstime > 10*TICRATE
			&& player->nightstime < 10*TICRATE)
		{
			//S_StartSound(NULL, sfx_timeup); // that creepy "out of time" music from NiGHTS. Dummied out, as some on the dev team thought it wasn't Sonic-y enough (Mystic, notably). Uncomment to restore. -SH
			S_ChangeMusicInternal("drown",false);
		}
	}
}

static inline boolean P_TagDamage(mobj_t *target, mobj_t *inflictor, mobj_t *source, INT32 damage)
{
	player_t *player = target->player;
	(void)damage; //unused parm

	// If flashing or invulnerable, ignore the tag,
	if (player->powers[pw_flashing] || player->powers[pw_invulnerability])
		return false;

	// Ignore IT players shooting each other, unless friendlyfire is on.
	if ((player->pflags & PF_TAGIT && !(cv_friendlyfire.value &&
		source && source->player && source->player->pflags & PF_TAGIT)))
		return false;

	// Don't allow any damage before the round starts.
	if (leveltime <= hidetime * TICRATE)
		return false;

	// Don't allow players on the same team to hurt one another,
	// unless cv_friendlyfire is on.
	if (!cv_friendlyfire.value && (player->pflags & PF_TAGIT) == (source->player->pflags & PF_TAGIT))
	{
		if (!(inflictor->flags & MF_FIRE))
			P_GivePlayerRings(player, 1);
		if (inflictor->flags2 & MF2_BOUNCERING)
			inflictor->fuse = 0; // bounce ring disappears at -1 not 0
		return false;
	}

	// The tag occurs so long as you aren't shooting another tagger with friendlyfire on.
	/*if (source->player->pflags & PF_TAGIT && !(player->pflags & PF_TAGIT))
	{
		P_AddPlayerScore(source->player, 1); //award points to tagger.

		if (gametype == GT_TAG) //survivor
		{
			player->pflags |= PF_TAGIT; //in survivor, the player becomes IT and helps hunt down the survivors.
			CONS_Printf(M_GetText("%s is now IT!\n"), player_names[player-players]); // Tell everyone who is it!
		}
		else
		{
			player->pflags |= PF_TAGGED; //in hide and seek, the player is tagged and stays stationary.
			CONS_Printf(M_GetText("%s was found!\n"), player_names[player-players]); // Tell everyone who is it!
		}

		//checks if tagger has tagged all players, if so, end round early.
		P_CheckSurvivors();
	}*/

	P_DoPlayerPain(player, source, inflictor);

	// Check for a shield
	if (player->powers[pw_shield])
	{
		P_RemoveShield(player);
		S_StartSound(target, sfx_shldls);
		return true;
	}

	if (target->health <= 1) // Death
	{
		P_PlayDeathSound(target);
		P_PlayVictorySound(source); // Killer laughs at you! LAUGHS! BWAHAHAHHAHAA!!
	}
	else if (target->health > 1) // Ring loss
	{
		P_PlayRinglossSound(target);
		P_PlayerRingBurst(player, player->mo->health - 1);
	}

	if (inflictor && ((inflictor->flags & MF_MISSILE) || inflictor->player) && player->powers[pw_super] && ALL7EMERALDS(player->powers[pw_emeralds]))
	{
		player->health -= 10;
		if (player->health < 2)
			player->health = 2;
		target->health = player->health;
	}
	else
		player->health = target->health = 1;

	return true;
}

static inline boolean P_PlayerHitsPlayer(mobj_t *target, mobj_t *inflictor, mobj_t *source, INT32 damage)
{
	player_t *player = target->player;

	// You can't kill yourself, idiot... // Unless it's Mario kart. Which it is. In this mod. All the time.
	//if (source == target)
	//	return false;

	// In COOP/RACE/CHAOS, you can't hurt other players unless cv_friendlyfire is on
	// ...But in SRB2kart, you can!
	//if (!cv_friendlyfire.value && (G_RaceGametype()))
	//	return false;

	// Tag handling
	if (G_TagGametype())
		return P_TagDamage(target, inflictor, source, damage);
	else if (G_GametypeHasTeams()) // CTF + Team Match
	{
		// Don't allow players on the same team to hurt one another,
		// unless cv_friendlyfire is on.
		if (!cv_friendlyfire.value && target->player->ctfteam == source->player->ctfteam)
		{
			if (!(inflictor->flags & MF_FIRE))
				P_GivePlayerRings(target->player, 1);
			if (inflictor->flags2 & MF2_BOUNCERING)
				inflictor->fuse = 0; // bounce ring disappears at -1 not 0

			return false;
		}
	}

	// Add pity.
	if (!player->powers[pw_flashing] && !player->powers[pw_invulnerability] && !player->powers[pw_super]
	&& source->player->score > player->score)
		player->pity++;

	return true;
}

static void P_KillPlayer(player_t *player, mobj_t *source, INT32 damage)
{
	player->pflags &= ~(PF_CARRIED|PF_SLIDING|PF_ITEMHANG|PF_MACESPIN|PF_ROPEHANG|PF_NIGHTSMODE);

	// Burst weapons and emeralds in Match/CTF only
	if (source && (G_BattleGametype()))
	{
		P_PlayerRingBurst(player, player->health - 1);
		P_PlayerEmeraldBurst(player, false);
	}

	// Get rid of shield
	player->powers[pw_shield] = SH_NONE;
	player->mo->color = player->skincolor;
	player->mo->colorized = false;

	// Get rid of emeralds
	player->powers[pw_emeralds] = 0;

	P_ForceFeed(player, 40, 10, TICRATE, 40 + min(damage, 100)*2);

	P_ResetPlayer(player);

	P_SetPlayerMobjState(player->mo, player->mo->info->deathstate);

	/*if (gametype == GT_CTF && (player->gotflag & (GF_REDFLAG|GF_BLUEFLAG)))
	{
		P_PlayerFlagBurst(player, false);
		if (source && source->player)
		{
			// Award no points when players shoot each other when cv_friendlyfire is on.
			if (!G_GametypeHasTeams() || !(source->player->ctfteam == player->ctfteam && source != player->mo))
				P_AddPlayerScore(source->player, 1);
		}
	}
	if (source && source->player && !player->powers[pw_super]) //don't score points against super players
	{
		// Award no points when players shoot each other when cv_friendlyfire is on.
		if (!G_GametypeHasTeams() || !(source->player->ctfteam == player->ctfteam && source != player->mo))
			P_AddPlayerScore(source->player, 1);
	}

	// If the player was super, tell them he/she ain't so super nomore.
	if (gametype != GT_COOP && player->powers[pw_super])
	{
		S_StartSound(NULL, sfx_s3k66); //let all players hear it.
		HU_SetCEchoFlags(0);
		HU_SetCEchoDuration(5);
		HU_DoCEcho(va("%s\\is no longer super.\\\\\\\\", player_names[player-players]));
	}*/

	if (player->pflags & PF_TIMEOVER)
	{
		mobj_t *boom;
		player->mo->flags |= (MF_NOGRAVITY|MF_NOCLIP);
		player->mo->flags2 |= MF2_DONTDRAW;
		boom = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_FZEROBOOM);
		boom->scale = player->mo->scale;
		boom->angle = player->mo->angle;
		P_SetTarget(&boom->target, player->mo);
	}

	if (G_BattleGametype())
	{
		if (player->kartstuff[k_bumper] > 0)
		{
			if (player->kartstuff[k_bumper] == 1)
			{
				mobj_t *karmahitbox = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_KARMAHITBOX); // Player hitbox is too small!!
				P_SetTarget(&karmahitbox->target, player->mo);
				karmahitbox->destscale = player->mo->scale;
				P_SetScale(karmahitbox, player->mo->scale);
				CONS_Printf(M_GetText("%s lost all of their bumpers!\n"), player_names[player-players]);
			}
			player->kartstuff[k_bumper]--;
			if (K_IsPlayerWanted(player))
				K_CalculateBattleWanted();
		}

		K_CheckBumpers();
	}
}

void P_RemoveShield(player_t *player)
{
	if (player->powers[pw_shield] & SH_FORCE)
	{ // Multi-hit
		if ((player->powers[pw_shield] & 0xFF) == 0)
			player->powers[pw_shield] &= ~SH_FORCE;
		else
			player->powers[pw_shield]--;
	}
	else if ((player->powers[pw_shield] & SH_NOSTACK) == SH_NONE)
	{ // Second layer shields
		player->powers[pw_shield] = SH_NONE;
		// Reset fireflower
		if (!player->powers[pw_super])
		{
			player->mo->color = player->skincolor;
			G_GhostAddColor((INT32) (player - players), GHC_NORMAL);
		}
	}
	else if ((player->powers[pw_shield] & SH_NOSTACK) == SH_BOMB) // Give them what's coming to them!
	{
		P_BlackOw(player); // BAM!
		player->pflags |= PF_JUMPDOWN;
	}
	else
		player->powers[pw_shield] = player->powers[pw_shield] & SH_STACK;
}

/*
static void P_ShieldDamage(player_t *player, mobj_t *inflictor, mobj_t *source, INT32 damage) // SRB2kart - unused.
{
	// Must do pain first to set flashing -- P_RemoveShield can cause damage
	P_DoPlayerPain(player, source, inflictor);

	P_RemoveShield(player);

	P_ForceFeed(player, 40, 10, TICRATE, 40 + min(damage, 100)*2);

	if (source && (source->type == MT_SPIKE || (source->type == MT_NULL && source->threshold == 43))) // spikes
		S_StartSound(player->mo, sfx_spkdth);
	else
		S_StartSound (player->mo, sfx_shldls); // Ba-Dum! Shield loss.

	if (gametype == GT_CTF && (player->gotflag & (GF_REDFLAG|GF_BLUEFLAG)))
	{
		P_PlayerFlagBurst(player, false);
		if (source && source->player)
		{
			// Award no points when players shoot each other when cv_friendlyfire is on.
			if (!G_GametypeHasTeams() || !(source->player->ctfteam == player->ctfteam && source != player->mo))
				P_AddPlayerScore(source->player, 25);
		}
	}
	if (source && source->player && !player->powers[pw_super]) //don't score points against super players
	{
		// Award no points when players shoot each other when cv_friendlyfire is on.
		if (!G_GametypeHasTeams() || !(source->player->ctfteam == player->ctfteam && source != player->mo))
			P_AddPlayerScore(source->player, cv_match_scoring.value == 1 ? 25 : 50);
	}
}
*/

static void P_RingDamage(player_t *player, mobj_t *inflictor, mobj_t *source, INT32 damage)
{
	//const UINT8 scoremultiply = ((K_IsWantedPlayer(player) && !trapitem) : 2 ? 1);

	if (!(inflictor && ((inflictor->flags & MF_MISSILE) || inflictor->player) && player->powers[pw_super] && ALL7EMERALDS(player->powers[pw_emeralds])))
	{
		P_DoPlayerPain(player, source, inflictor);

		P_ForceFeed(player, 40, 10, TICRATE, 40 + min(damage, 100)*2);

		if (source && (source->type == MT_SPIKE || (source->type == MT_NULL && source->threshold == 43))) // spikes
			S_StartSound(player->mo, sfx_spkdth);
	}

	/*if (source && source->player && !player->powers[pw_super]) //don't score points against super players
	{
		// Award no points when players shoot each other when cv_friendlyfire is on.
		if (!G_GametypeHasTeams() || !(source->player->ctfteam == player->ctfteam && source != player->mo))
			P_AddPlayerScore(source->player, scoremultiply);
	}

	if (gametype == GT_CTF && (player->gotflag & (GF_REDFLAG|GF_BLUEFLAG)))
	{
		P_PlayerFlagBurst(player, false);
		if (source && source->player)
		{
			// Award no points when players shoot each other when cv_friendlyfire is on.
			if (!G_GametypeHasTeams() || !(source->player->ctfteam == player->ctfteam && source != player->mo))
				P_AddPlayerScore(source->player, scoremultiply);
		}
	}*/

	// Ring loss sound plays despite hitting spikes
	P_PlayRinglossSound(player->mo); // Ringledingle!
}

/** Damages an object, which may or may not be a player.
  * For melee attacks, source and inflictor are the same.
  *
  * \param target    The object being damaged.
  * \param inflictor The thing that caused the damage: creature, missile,
  *                  gargoyle, and so forth. Can be NULL in the case of
  *                  environmental damage, such as slime or crushing.
  * \param source    The creature or person responsible. For example, if a
  *                  player is hit by a ring, the player who shot it. In some
  *                  cases, the target will go after this object after
  *                  receiving damage. This can be NULL.
  * \param damage    Amount of damage to be dealt. 10000 is instant death.
  * \return True if the target sustained damage, otherwise false.
  * \todo Clean up this mess, split into multiple functions.
  * \todo Get rid of the magic number 10000.
  * \sa P_KillMobj
  */
boolean P_DamageMobj(mobj_t *target, mobj_t *inflictor, mobj_t *source, INT32 damage)
{
	player_t *player;
	boolean force = false;

	if (objectplacing)
		return false;

	if (target->health <= 0)
		return false;

	// Spectator handling
	if (netgame)
	{
		if (damage == 42000 && target->player && target->player->spectator)
			damage = 10000;
		else if (target->player && target->player->spectator)
			return false;

		if (source && source->player && source->player->spectator)
			return false;
	}

	// Everything above here can't be forced.
	if (!metalrecording)
	{
		UINT8 shouldForce = LUAh_ShouldDamage(target, inflictor, source, damage);
		if (P_MobjWasRemoved(target))
			return (shouldForce == 1); // mobj was removed
		if (shouldForce == 1)
			force = true;
		else if (shouldForce == 2)
			return false;
	}

	if (!force)
	{
		if (!(target->flags & MF_SHOOTABLE))
			return false; // shouldn't happen...

		if (target->type == MT_BLACKEGGMAN)
			return false;

		// Make sure that boxes cannot be popped by enemies, red rings, etc.
		if (target->flags & MF_MONITOR && ((!source || !source->player || source->player->bot) || (inflictor && !inflictor->player)))
			return false;
	}

	if (target->flags2 & MF2_SKULLFLY)
		target->momx = target->momy = target->momz = 0;

	if (!force)
	{
		// Special case for team ring boxes
		if (target->type == MT_REDRINGBOX && !(source->player->ctfteam == 1))
			return false;

		if (target->type == MT_BLUERINGBOX && !(source->player->ctfteam == 2))
			return false;
	}

	// Special case for Crawla Commander
	if (target->type == MT_CRAWLACOMMANDER)
	{
		if (!force && target->fuse) // Invincible
			return false;

		if (LUAh_MobjDamage(target, inflictor, source, damage) || P_MobjWasRemoved(target))
			return true;

		if (target->health > 1)
		{
			if (target->info->painsound)
				S_StartSound(target, target->info->painsound);

			target->fuse = TICRATE/2;
			target->flags2 |= MF2_FRET;
		}
		else
		{
			target->flags |= MF_NOGRAVITY;
			target->fuse = 0;
		}

		target->momx = target->momy = target->momz = 0;

		P_InstaThrust(target, target->angle-ANGLE_180, FixedMul(5*FRACUNIT, target->scale));
	}
	else if (target->flags & MF_BOSS)
	{
		if (!force && target->flags2 & MF2_FRET) // Currently flashing from being hit
			return false;

		if (LUAh_MobjDamage(target, inflictor, source, damage) || P_MobjWasRemoved(target))
			return true;

		if (target->health > 1)
			target->flags2 |= MF2_FRET;
	}
	else if (target->flags & MF_ENEMY)
	{
		if (LUAh_MobjDamage(target, inflictor, source, damage) || P_MobjWasRemoved(target))
			return true;
	}

	player = target->player;

	if (player) // Player is the target
	{
		if (!force)
		{
			if (player->exiting)
				return false;

			if (!(target->player->pflags & (PF_NIGHTSMODE|PF_NIGHTSFALL)) && (maptol & TOL_NIGHTS))
				return false;
		}

		if (player->pflags & PF_NIGHTSMODE) // NiGHTS damage handling
		{
			if (!force)
			{
				if (source == target)
					return false; // Don't hit yourself with your own paraloop, baka
				if (source && source->player && !cv_friendlyfire.value
				&& (gametype == GT_COOP
				|| (G_GametypeHasTeams() && target->player->ctfteam == source->player->ctfteam)))
					return false; // Don't run eachother over in special stages and team games and such
			}
			if (LUAh_MobjDamage(target, inflictor, source, damage))
				return true;
			P_NiGHTSDamage(target, source); // -5s :(
			return true;
		}

		if (LUAh_MobjDamage(target, inflictor, source, damage))
			return true;

		if (!force && inflictor && (inflictor->flags & MF_FIRE))
		{
			if ((player->powers[pw_shield] & SH_NOSTACK) == SH_ELEMENTAL)
				return false; // Invincible to fire objects

			if (G_RaceGametype() && source && source->player)
				return false; // Don't get hurt by fire generated from friends.
		}

		// Sudden-Death mode
		if (source && source->type == MT_PLAYER)
		{
			if ((G_BattleGametype()) && cv_suddendeath.value
				&& !player->powers[pw_flashing] && !player->powers[pw_invulnerability])
				damage = 10000;
		}

		// Player hits another player
		if (!force && source && source->player)
		{
			if (!P_PlayerHitsPlayer(target, inflictor, source, damage))
				return false;
		}

		if (!force && player->pflags & PF_GODMODE)
			return false;

		// Instant-Death
		if (damage == 10000)
			P_KillPlayer(player, source, damage);
		else if (player->kartstuff[k_invincibilitytimer] > 0 || player->kartstuff[k_growshrinktimer] > 0 || player->powers[pw_flashing])
		{
			if (!force)	// shoulddamage bypasses all of that.
			{
				K_DoInstashield(player);
				return false;
			}
		}
		else
		{
			if (inflictor && (inflictor->type == MT_ORBINAUT || inflictor->type == MT_ORBINAUT_SHIELD
				|| inflictor->type == MT_JAWZ || inflictor->type == MT_JAWZ_SHIELD || inflictor->type == MT_JAWZ_DUD
				|| inflictor->type == MT_SMK_THWOMP || inflictor->player))
			{
				player->kartstuff[k_sneakertimer] = 0;
				K_SpinPlayer(player, source, 1, inflictor, false);
				damage = player->mo->health - 1;
				P_RingDamage(player, inflictor, source, damage);
				P_PlayerRingBurst(player, 5);
				if (P_IsLocalPlayer(player))
				{
					quake.intensity = 32*FRACUNIT;
					quake.time = 5;
				}
			}
			else
			{
				K_SpinPlayer(player, source, 0, inflictor, false);
			}
			return true;
		}
		/* // SRB2kart - don't need these
		else if (metalrecording)
		{
			if (!inflictor)
				inflictor = source;
			if (inflictor && inflictor->flags & MF_ENEMY)
			{ // Metal Sonic destroy enemy !!
				P_KillMobj(inflictor, NULL, target);
				return false;
			}
			else if (inflictor && inflictor->flags & MF_MISSILE)
				return false; // Metal Sonic walk through flame !!
			else
			{ // Oh no! Metal Sonic is hit !!
				P_ShieldDamage(player, inflictor, source, damage);
				return true;
			}
		}
		else if (player->powers[pw_invulnerability] || player->powers[pw_flashing] // ignore bouncing & such in invulnerability
			|| (player->powers[pw_super] && !(ALL7EMERALDS(player->powers[pw_emeralds]) && inflictor && ((inflictor->flags & MF_MISSILE) || inflictor->player))))
		{
			if (force || (inflictor && (inflictor->flags & MF_MISSILE)
				&& (inflictor->flags2 & MF2_SUPERFIRE)
				&& player->powers[pw_super]))
			{
#ifdef HAVE_BLUA
				if (!LUAh_MobjDamage(target, inflictor, source, damage))
#endif
					P_SuperDamage(player, inflictor, source, damage);
				return true;
			}
			else
				return false;
		}
#ifdef HAVE_BLUA
		else if (LUAh_MobjDamage(target, inflictor, source, damage))
			return true;
#endif
		else if (!player->powers[pw_super] && (player->powers[pw_shield] || player->bot))  //If One-Hit Shield
		{
			P_ShieldDamage(player, inflictor, source, damage);
			damage = 0;
		}
		else if (player->mo->health > 1) // No shield but have rings.
		{
			damage = player->mo->health - 1;
			P_RingDamage(player, inflictor, source, damage);
		}
		else // No shield, no rings, no invincibility.
		{
			// To reduce griefing potential, don't allow players to be killed
			// by friendly fire. Spilling their rings and other items is enough.
			if (force || !(G_GametypeHasTeams()
				&& source && source->player && (source->player->ctfteam == player->ctfteam)
				&& cv_friendlyfire.value))
			{
				damage = 1;
				P_KillPlayer(player, source, damage);
			}
			else
			{
				damage = 0;
				P_ShieldDamage(player, inflictor, source, damage);
			}
		}
		*/

		if (inflictor && ((inflictor->flags & MF_MISSILE) || inflictor->player) && player->powers[pw_super] && ALL7EMERALDS(player->powers[pw_emeralds]))
		{
			if (player->powers[pw_shield])
			{
				P_RemoveShield(player);
				return true;
			}
			else
			{
				player->health -= (10 * (1 << (INT32)(player->powers[pw_super] / 10500)));
				if (player->health < 2)
					player->health = 2;
			}

			if (gametype == GT_CTF && (player->gotflag & (GF_REDFLAG|GF_BLUEFLAG)))
				P_PlayerFlagBurst(player, false);
		}
		else
		{
			player->health -= damage; // mirror mobj health here
			if (damage < 10000)
			{
				target->player->powers[pw_flashing] = K_GetKartFlashing(target->player);
				if (damage > 0) // don't spill emeralds/ammo/panels for shield damage
					P_PlayerRingBurst(player, damage);
			}
		}

		if (player->health < 0)
			player->health = 0;

		P_ForceFeed(player, 40, 10, TICRATE, 40 + min(damage, 100)*2);
	}

	// Killing dead. Just for kicks.
	// Require source and inflictor be player.  Don't hurt for firing rings.
	if (cv_killingdead.value && (source && source->player) && (inflictor && inflictor->player) && P_RandomChance(5*FRACUNIT/16))
		P_DamageMobj(source, target, target, 1);

	// do the damage
	if (player && player->powers[pw_super] && ALL7EMERALDS(player->powers[pw_emeralds]) && inflictor && ((inflictor->flags & MF_MISSILE) || inflictor->player))
	{
		target->health -= (10 * (1 << (INT32)(player->powers[pw_super] / 10500)));
		if (target->health < 2)
			target->health = 2;
	}
	else
		target->health -= damage;

	if (source && source->player && target)
		G_GhostAddHit((INT32) (source->player - players), target);

	if (target->health <= 0)
	{
		P_KillMobj(target, inflictor, source);
		return true;
	}

	if (player)
	{
		if (!(player->powers[pw_super] && ALL7EMERALDS(player->powers[pw_emeralds])))
			P_ResetPlayer(target->player);
	}
	else
		switch (target->type)
		{
		case MT_EGGMOBILE2: // egg slimer
			if (target->health < target->info->damage) // in pinch phase
			{
				P_SetMobjState(target, target->info->meleestate); // go to pinch pain state
				break;
			}
			/* FALLTHRU */
		default:
			P_SetMobjState(target, target->info->painstate);
			break;
		}

	if (!P_MobjWasRemoved(target))
	{
		target->reactiontime = 0; // we're awake now...

		if (source && source != target)
		{
			// if not intent on another player,
			// chase after this one
			P_SetTarget(&target->target, source);
			if (target->state == &states[target->info->spawnstate] && target->info->seestate != S_NULL)
			{
				if (player)
				{
					if (!(player->powers[pw_super] && ALL7EMERALDS(player->powers[pw_emeralds])))
						P_SetPlayerMobjState(target, target->info->seestate);
				}
				else
					P_SetMobjState(target, target->info->seestate);
			}
		}
	}

	return true;
}

/** Spills an injured player's rings.
  *
  * \param player    The player who is losing rings.
  * \param num_rings Number of rings lost. A maximum of 32 rings will be
  *                  spawned.
  * \sa P_PlayerFlagBurst
  */
void P_PlayerRingBurst(player_t *player, INT32 num_rings)
{
	INT32 i;
	mobj_t *mo;
	angle_t fa;
	fixed_t ns;
	fixed_t z;

	// Better safe than sorry.
	if (!player)
		return;

	// Never have health in kart I think
	if (player->mo->health <= 1)
		num_rings = 5;

	if (num_rings > 32 && !(player->pflags & PF_NIGHTSFALL))
		num_rings = 32;

	if (player->powers[pw_emeralds])
		P_PlayerEmeraldBurst(player, false);

	// Spill weapons first
	if (player->ringweapons)
		P_PlayerWeaponPanelBurst(player);

	// Spill the ammo
	P_PlayerWeaponAmmoBurst(player);

	// There's no ring spilling in kart, so I'm hijacking this for the same thing as TD
	for (i = 0; i < num_rings; i++)
	{
		INT32 objType = mobjinfo[MT_FLINGENERGY].reactiontime;

		z = player->mo->z;
		if (player->mo->eflags & MFE_VERTICALFLIP)
			z += player->mo->height - mobjinfo[objType].height;

		mo = P_SpawnMobj(player->mo->x, player->mo->y, z, objType);

		mo->fuse = 8*TICRATE;
		P_SetTarget(&mo->target, player->mo);

		mo->destscale = player->mo->scale;
		P_SetScale(mo, player->mo->scale);

		// Angle offset by player angle, then slightly offset by amount of rings
		fa = ((i*FINEANGLES/16) + (player->mo->angle>>ANGLETOFINESHIFT) - ((num_rings-1)*FINEANGLES/32)) & FINEMASK;

		// Make rings spill out around the player in 16 directions like SA, but spill like Sonic 2.
		// Technically a non-SA way of spilling rings. They just so happen to be a little similar.
		if (player->pflags & PF_NIGHTSFALL)
		{
			ns = FixedMul(((i*FRACUNIT)/16)+2*FRACUNIT, mo->scale);
			mo->momx = FixedMul(FINECOSINE(fa),ns);

			if (!(twodlevel || (player->mo->flags2 & MF2_TWOD)))
				mo->momy = FixedMul(FINESINE(fa),ns);

			P_SetObjectMomZ(mo, 8*FRACUNIT, false);
			mo->fuse = 20*TICRATE; // Adjust fuse for NiGHTS
		}
		else
		{
			fixed_t momxy, momz; // base horizonal/vertical thrusts

			if (i > 15)
			{
				momxy = 3*FRACUNIT;
				momz = 4*FRACUNIT;
			}
			else
			{
				momxy = 28*FRACUNIT;
				momz = 3*FRACUNIT;
			}

			ns = FixedMul(momxy, mo->scale);
			mo->momx = FixedMul(FINECOSINE(fa),ns);

			if (!(twodlevel || (player->mo->flags2 & MF2_TWOD)))
				mo->momy = FixedMul(FINESINE(fa),ns);

			ns = momz;
			P_SetObjectMomZ(mo, ns, false);

			if (i & 1)
				P_SetObjectMomZ(mo, ns, true);
		}
		if (player->mo->eflags & MFE_VERTICALFLIP)
			mo->momz *= -1;
	}

	player->losstime += 10*TICRATE;

	if (P_IsObjectOnGround(player->mo))
		player->pflags &= ~PF_NIGHTSFALL;

	return;
}

void P_PlayerWeaponPanelBurst(player_t *player)
{
	mobj_t *mo;
	angle_t fa;
	fixed_t ns;
	INT32 i;
	fixed_t z;

	INT32 num_weapons = M_CountBits((UINT32)player->ringweapons, NUM_WEAPONS-1);
	UINT16 ammoamt = 0;

	for (i = 0; i < num_weapons; i++)
	{
		mobjtype_t weptype = 0;
		powertype_t power = 0;

		if (player->ringweapons & RW_BOUNCE) // Bounce
		{
			weptype = MT_BOUNCEPICKUP;
			player->ringweapons &= ~RW_BOUNCE;
			power = pw_bouncering;
		}
		else if (player->ringweapons & RW_RAIL) // Rail
		{
			weptype = MT_RAILPICKUP;
			player->ringweapons &= ~RW_RAIL;
			power = pw_railring;
		}
		else if (player->ringweapons & RW_AUTO) // Auto
		{
			weptype = MT_AUTOPICKUP;
			player->ringweapons &= ~RW_AUTO;
			power = pw_automaticring;
		}
		else if (player->ringweapons & RW_EXPLODE) // Explode
		{
			weptype = MT_EXPLODEPICKUP;
			player->ringweapons &= ~RW_EXPLODE;
			power = pw_explosionring;
		}
		else if (player->ringweapons & RW_SCATTER) // Scatter
		{
			weptype = MT_SCATTERPICKUP;
			player->ringweapons &= ~RW_SCATTER;
			power = pw_scatterring;
		}
		else if (player->ringweapons & RW_GRENADE) // Grenade
		{
			weptype = MT_GRENADEPICKUP;
			player->ringweapons &= ~RW_GRENADE;
			power = pw_grenadering;
		}

		if (!weptype) // ???
			continue;

		if (player->powers[power] >= mobjinfo[weptype].reactiontime)
			ammoamt = (UINT16)mobjinfo[weptype].reactiontime;
		else
			ammoamt = player->powers[power];

		player->powers[power] -= ammoamt;

		z = player->mo->z;
		if (player->mo->eflags & MFE_VERTICALFLIP)
			z += player->mo->height - mobjinfo[weptype].height;

		mo = P_SpawnMobj(player->mo->x, player->mo->y, z, weptype);
		mo->reactiontime = ammoamt;
		mo->flags2 |= MF2_DONTRESPAWN;
		mo->flags &= ~(MF_NOGRAVITY|MF_NOCLIPHEIGHT);
		P_SetTarget(&mo->target, player->mo);
		mo->fuse = 12*TICRATE;
		mo->destscale = player->mo->scale;
		P_SetScale(mo, player->mo->scale);

		// Angle offset by player angle
		fa = ((i*FINEANGLES/16) + (player->mo->angle>>ANGLETOFINESHIFT)) & FINEMASK;

		// Make rings spill out around the player in 16 directions like SA, but spill like Sonic 2.
		// Technically a non-SA way of spilling rings. They just so happen to be a little similar.

		// >16 ring type spillout
		ns = FixedMul(3*FRACUNIT, mo->scale);
		mo->momx = FixedMul(FINECOSINE(fa),ns);

		if (!(twodlevel || (player->mo->flags2 & MF2_TWOD)))
			mo->momy = FixedMul(FINESINE(fa),ns);

		P_SetObjectMomZ(mo, 4*FRACUNIT, false);

		if (i & 1)
			P_SetObjectMomZ(mo, 4*FRACUNIT, true);
	}
}

void P_PlayerWeaponAmmoBurst(player_t *player)
{
	mobj_t *mo;
	angle_t fa;
	fixed_t ns;
	INT32 i = 0;
	fixed_t z;

	mobjtype_t weptype = 0;
	powertype_t power = 0;

	while (true)
	{
		if (player->powers[pw_bouncering])
		{
			weptype = MT_BOUNCERING;
			power = pw_bouncering;
		}
		else if (player->powers[pw_railring])
		{
			weptype = MT_RAILRING;
			power = pw_railring;
		}
		else if (player->powers[pw_infinityring])
		{
			weptype = MT_INFINITYRING;
			power = pw_infinityring;
		}
		else if (player->powers[pw_automaticring])
		{
			weptype = MT_AUTOMATICRING;
			power = pw_automaticring;
		}
		else if (player->powers[pw_explosionring])
		{
			weptype = MT_EXPLOSIONRING;
			power = pw_explosionring;
		}
		else if (player->powers[pw_scatterring])
		{
			weptype = MT_SCATTERRING;
			power = pw_scatterring;
		}
		else if (player->powers[pw_grenadering])
		{
			weptype = MT_GRENADERING;
			power = pw_grenadering;
		}
		else
			break; // All done!

		z = player->mo->z;
		if (player->mo->eflags & MFE_VERTICALFLIP)
			z += player->mo->height - mobjinfo[weptype].height;

		mo = P_SpawnMobj(player->mo->x, player->mo->y, z, weptype);
		mo->health = player->powers[power];
		mo->flags2 |= MF2_DONTRESPAWN;
		mo->flags &= ~(MF_NOGRAVITY|MF_NOCLIPHEIGHT);
		P_SetTarget(&mo->target, player->mo);

		player->powers[power] = 0;
		mo->fuse = 12*TICRATE;

		mo->destscale = player->mo->scale;
		P_SetScale(mo, player->mo->scale);

		// Angle offset by player angle
		fa = ((i*FINEANGLES/16) + (player->mo->angle>>ANGLETOFINESHIFT)) & FINEMASK;

		// Spill them!
		ns = FixedMul(2*FRACUNIT, mo->scale);
		mo->momx = FixedMul(FINECOSINE(fa), ns);

		if (!(twodlevel || (player->mo->flags2 & MF2_TWOD)))
			mo->momy = FixedMul(FINESINE(fa),ns);

		P_SetObjectMomZ(mo, 3*FRACUNIT, false);

		if (i & 1)
			P_SetObjectMomZ(mo, 3*FRACUNIT, true);

		i++;
	}
}

//
// P_PlayerEmeraldBurst
//
// Spills ONLY emeralds.
//
void P_PlayerEmeraldBurst(player_t *player, boolean toss)
{
	INT32 i;
	angle_t fa;
	fixed_t ns;
	fixed_t z = 0, momx = 0, momy = 0;

	// Better safe than sorry.
	if (!player)
		return;

	// Spill power stones
	if (player->powers[pw_emeralds])
	{
		INT32 num_stones = 0;

		if (player->powers[pw_emeralds] & EMERALD1)
			num_stones++;
		if (player->powers[pw_emeralds] & EMERALD2)
			num_stones++;
		if (player->powers[pw_emeralds] & EMERALD3)
			num_stones++;
		if (player->powers[pw_emeralds] & EMERALD4)
			num_stones++;
		if (player->powers[pw_emeralds] & EMERALD5)
			num_stones++;
		if (player->powers[pw_emeralds] & EMERALD6)
			num_stones++;
		if (player->powers[pw_emeralds] & EMERALD7)
			num_stones++;

		for (i = 0; i < num_stones; i++)
		{
			INT32 stoneflag = 0;
			statenum_t statenum = S_CEMG1;
			mobj_t *mo;

			if (player->powers[pw_emeralds] & EMERALD1)
			{
				stoneflag = EMERALD1;
				statenum = S_CEMG1;
			}
			else if (player->powers[pw_emeralds] & EMERALD2)
			{
				stoneflag = EMERALD2;
				statenum = S_CEMG2;
			}
			else if (player->powers[pw_emeralds] & EMERALD3)
			{
				stoneflag = EMERALD3;
				statenum = S_CEMG3;
			}
			else if (player->powers[pw_emeralds] & EMERALD4)
			{
				stoneflag = EMERALD4;
				statenum = S_CEMG4;
			}
			else if (player->powers[pw_emeralds] & EMERALD5)
			{
				stoneflag = EMERALD5;
				statenum = S_CEMG5;
			}
			else if (player->powers[pw_emeralds] & EMERALD6)
			{
				stoneflag = EMERALD6;
				statenum = S_CEMG6;
			}
			else if (player->powers[pw_emeralds] & EMERALD7)
			{
				stoneflag = EMERALD7;
				statenum = S_CEMG7;
			}

			if (!stoneflag) // ???
				continue;

			player->powers[pw_emeralds] &= ~stoneflag;

			if (toss)
			{
				fa = player->mo->angle>>ANGLETOFINESHIFT;

				z = player->mo->z + player->mo->height;
				if (player->mo->eflags & MFE_VERTICALFLIP)
					z -= mobjinfo[MT_FLINGEMERALD].height + player->mo->height;
				ns = FixedMul(8*FRACUNIT, player->mo->scale);
			}
			else
			{
				fa = ((255 / num_stones) * i) * FINEANGLES/256;

				z = player->mo->z + (player->mo->height / 2);
				if (player->mo->eflags & MFE_VERTICALFLIP)
					z -= mobjinfo[MT_FLINGEMERALD].height;
				ns = FixedMul(4*FRACUNIT, player->mo->scale);
			}

			momx = FixedMul(FINECOSINE(fa), ns);

			if (!(twodlevel || (player->mo->flags2 & MF2_TWOD)))
				momy = FixedMul(FINESINE(fa),ns);
			else
				momy = 0;

			mo = P_SpawnMobj(player->mo->x, player->mo->y, z, MT_FLINGEMERALD);
			mo->health = 1;
			mo->threshold = stoneflag;
			mo->flags2 |= (MF2_DONTRESPAWN|MF2_SLIDEPUSH);
			mo->flags &= ~(MF_NOGRAVITY|MF_NOCLIPHEIGHT);
			P_SetTarget(&mo->target, player->mo);
			mo->fuse = 12*TICRATE;
			P_SetMobjState(mo, statenum);

			mo->momx = momx;
			mo->momy = momy;

			P_SetObjectMomZ(mo, 3*FRACUNIT, false);

			if (player->mo->eflags & MFE_VERTICALFLIP)
				mo->momz = -mo->momz;

			if (toss)
				player->tossdelay = 2*TICRATE;
		}
	}
}

/** Makes an injured or dead player lose possession of the flag.
  *
  * \param player The player with the flag, about to lose it.
  * \sa P_PlayerRingBurst
  */
void P_PlayerFlagBurst(player_t *player, boolean toss)
{
	mobj_t *flag;
	mobjtype_t type;

	if (!(player->gotflag & (GF_REDFLAG|GF_BLUEFLAG)))
		return;

	if (player->gotflag & GF_REDFLAG)
		type = MT_REDFLAG;
	else
		type = MT_BLUEFLAG;

	flag = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, type);

	if (player->mo->eflags & MFE_VERTICALFLIP)
		flag->z += player->mo->height - flag->height;

	if (toss)
		P_InstaThrust(flag, player->mo->angle, FixedMul(6*FRACUNIT, player->mo->scale));
	else
	{
		angle_t fa = P_RandomByte()*FINEANGLES/256;
		flag->momx = FixedMul(FINECOSINE(fa), FixedMul(6*FRACUNIT, player->mo->scale));
		if (!(twodlevel || (player->mo->flags2 & MF2_TWOD)))
			flag->momy = FixedMul(FINESINE(fa), FixedMul(6*FRACUNIT, player->mo->scale));
	}

	flag->momz = FixedMul(8*FRACUNIT, player->mo->scale);
	if (player->mo->eflags & MFE_VERTICALFLIP)
		flag->momz = -flag->momz;

	if (type == MT_REDFLAG)
		flag->spawnpoint = rflagpoint;
	else
		flag->spawnpoint = bflagpoint;

	flag->fuse = cv_flagtime.value * TICRATE;
	P_SetTarget(&flag->target, player->mo);

	// Flag text
	{
		char plname[MAXPLAYERNAME+4];
		const char *flagtext;
		char flagcolor;

		snprintf(plname, sizeof(plname), "%s%s%s",
				 CTFTEAMCODE(player),
				 player_names[player - players],
				 CTFTEAMENDCODE(player));

		if (type == MT_REDFLAG)
		{
			flagtext = M_GetText("Red flag");
			flagcolor = '\x85';
		}
		else
		{
			flagtext = M_GetText("Blue flag");
			flagcolor = '\x84';
		}

		if (toss)
			CONS_Printf(M_GetText("%s tossed the %c%s%c.\n"), plname, flagcolor, flagtext, 0x80);
		else
			CONS_Printf(M_GetText("%s dropped the %c%s%c.\n"), plname, flagcolor, flagtext, 0x80);
	}

	player->gotflag = 0;

	// Pointers set for displaying time value and for consistency restoration.
	if (type == MT_REDFLAG)
		redflag = flag;
	else
		blueflag = flag;

	if (toss)
		player->tossdelay = 2*TICRATE;

	return;
}
