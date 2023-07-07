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
/// \file  p_telept.c
/// \brief Teleportation

#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "r_state.h"
#include "s_sound.h"
#include "r_main.h"
#include "r_fps.h"

/**	\brief	The P_MixUp function

	\param	thing	mobj_t to mix up
	\param	x	new x pos
	\param	y	new y pos
	\param	z	new y pos
	\param	angle	new angle to look at

	\return	void


*/
void P_MixUp(mobj_t *thing, fixed_t x, fixed_t y, fixed_t z, angle_t angle,
			INT16 starpostx, INT16 starposty, INT16 starpostz,
			INT32 starpostnum, tic_t starposttime, angle_t starpostangle,
			INT32 flags2)
{
	const INT32 takeflags2 = MF2_TWOD|MF2_OBJECTFLIP;
	UINT8 i;

	// the move is ok,
	// so link the thing into its new position
	P_UnsetThingPosition(thing);

	// Remove touching_sectorlist from mobj.
	if (sector_list)
	{
		P_DelSeclist(sector_list);
		sector_list = NULL;
	}

	thing->x = x;
	thing->y = y;
	thing->z = z;

	if (thing->player)
	{
		if (thing->eflags & MFE_VERTICALFLIP)
			thing->player->viewz = thing->z + thing->height - thing->player->viewheight;
		else
			thing->player->viewz = thing->z + thing->player->viewheight;

		if (!thing->tracer)
			thing->reactiontime = TICRATE/2; // don't move for about half a second

		// absolute angle position
		if (thing == players[consoleplayer].mo)
			localangle[0] = angle;
		else if (splitscreen)
		{
			for (i = 1; i <= splitscreen; i++)
			{
				if (thing == players[displayplayers[i]].mo)
				{
					localangle[i] = angle;
					break;
				}
			}
		}

		// move chasecam at new player location
		for (i = 0; i <= splitscreen; i++)
		{
			if (thing->player == &players[displayplayers[i]] && camera[i].chase)
				P_ResetCamera(thing->player, &camera[i]);
		}

		// don't run in place after a teleport
		thing->player->cmomx = thing->player->cmomy = 0;
		thing->player->rmomx = thing->player->rmomy = 0;
		if (!thing->tracer)
			thing->player->speed = 0;

		// Starpost information
		thing->player->starpostx = starpostx;
		thing->player->starposty = starposty;
		thing->player->starpostz = starpostz;
		thing->player->starposttime = starposttime;
		thing->player->starpostangle = starpostangle;
		thing->player->starpostnum = starpostnum;

		P_ResetPlayer(thing->player);
		P_SetPlayerMobjState(thing, S_KART_STND1); // SRB2kart - was S_PLAY_STND

		P_FlashPal(thing->player, PAL_MIXUP, 10);
	}

	thing->angle = angle;

	thing->momx = thing->momy = thing->momz = 0;

	thing->flags2 = (thing->flags2 & ~takeflags2) | (flags2 & takeflags2);
}

/**	\brief	The P_Teleport function

	\param	thing	mobj_t to teleport
	\param	x	new x pos
	\param	y	new y pos
	\param	z	new y pos
	\param	angle	new angle to look at

	\return	if true, the thing "teleported"


*/
boolean P_Teleport(mobj_t *thing, fixed_t x, fixed_t y, fixed_t z, angle_t angle, boolean flash, boolean dontstopmove)
{
	UINT8 i;

	if (!P_SetOrigin(thing, x, y, z))
		return false;

	thing->angle = angle;

	if (!dontstopmove)
		thing->momx = thing->momy = thing->momz = 0;
	else // Change speed to match direction
		P_InstaThrust(thing, thing->angle, P_AproxDistance(thing->momx, thing->momy));

	if (thing->player)
	{
		if (thing->eflags & MFE_VERTICALFLIP)
			thing->player->viewz = thing->z + thing->height - thing->player->viewheight;
		else
			thing->player->viewz = thing->z + thing->player->viewheight;

		if (!dontstopmove)
			thing->reactiontime = TICRATE/2; // don't move for about half a second

		// absolute angle position
		// move chasecam at new player location
		for (i = 0; i <= splitscreen; i++)
		{
			if (thing->player == &players[displayplayers[i]])
			{
				localangle[i] = angle;
				if (camera[i].chase)
					P_ResetCamera(thing->player, &camera[i]);
				R_ResetViewInterpolation(i + 1);
				break;
			}
		}

		// don't run in place after a teleport
		if (!dontstopmove)
		{
			INT32 p;
			// Search for any players you might be carrying, so you can get them off before they end up being taken with you!
			for (p = 0; p < MAXPLAYERS; p++)
				if (playeringame[p] && players[p].mo && players[p].pflags & PF_CARRIED && players[p].mo->tracer == thing)
				{
					players[p].pflags &= ~PF_CARRIED;
					break;
				}
			thing->player->cmomx = thing->player->cmomy = 0;
			thing->player->rmomx = thing->player->rmomy = 0;
			thing->player->speed = 0;
			P_ResetPlayer(thing->player);
			P_SetPlayerMobjState(thing, S_KART_STND1); // SRB2kart - was S_PLAY_STND
		}

		if (flash)
			P_FlashPal(thing->player, PAL_MIXUP, 10);
	}

	return true;
}
