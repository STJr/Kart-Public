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
/// \file  p_tick.c
/// \brief Archiving: SaveGame I/O, Thinker, Ticker

#include "doomstat.h"
#include "g_game.h"
#include "g_input.h"
#include "p_local.h"
#include "z_zone.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "p_polyobj.h"
#include "m_random.h"
#include "lua_script.h"
#include "lua_hook.h"
#include "k_kart.h"
#include "r_main.h"
#include "r_fps.h"
#include "i_video.h" // rendermode

// Object place
#include "m_cheat.h"

tic_t leveltime;

//
// THINKERS
// All thinkers should be allocated by Z_Calloc
// so they can be operated on uniformly.
// The actual structures will vary in size,
// but the first element must be thinker_t.
//

// Both the head and tail of the thinker list.
thinker_t thinkercap;

void Command_Numthinkers_f(void)
{
	INT32 num;
	INT32 count = 0;
	actionf_p1 action;
	thinker_t *think;

	if (gamestate != GS_LEVEL)
	{
		CONS_Printf(M_GetText("You must be in a level to use this.\n"));
		return;
	}

	if (COM_Argc() < 2)
	{
		CONS_Printf(M_GetText("numthinkers <#>: Count number of thinkers\n"));
		CONS_Printf(
			"\t1: P_MobjThinker\n"
			/*"\t2: P_RainThinker\n"
			"\t3: P_SnowThinker\n"*/
			"\t2: P_NullPrecipThinker\n"
			"\t3: T_Friction\n"
			"\t4: T_Pusher\n"
			"\t5: P_RemoveThinkerDelayed\n");
		return;
	}

	num = atoi(COM_Argv(1));

	switch (num)
	{
		case 1:
			action = (actionf_p1)P_MobjThinker;
			CONS_Printf(M_GetText("Number of %s: "), "P_MobjThinker");
			break;
		/*case 2:
			action = (actionf_p1)P_RainThinker;
			CONS_Printf(M_GetText("Number of %s: "), "P_RainThinker");
			break;
		case 3:
			action = (actionf_p1)P_SnowThinker;
			CONS_Printf(M_GetText("Number of %s: "), "P_SnowThinker");
			break;*/
		case 2:
			action = (actionf_p1)P_NullPrecipThinker;
			CONS_Printf(M_GetText("Number of %s: "), "P_NullPrecipThinker");
			break;
		case 3:
			action = (actionf_p1)T_Friction;
			CONS_Printf(M_GetText("Number of %s: "), "T_Friction");
			break;
		case 4:
			action = (actionf_p1)T_Pusher;
			CONS_Printf(M_GetText("Number of %s: "), "T_Pusher");
			break;
		case 5:
			action = (actionf_p1)P_RemoveThinkerDelayed;
			CONS_Printf(M_GetText("Number of %s: "), "P_RemoveThinkerDelayed");
			break;
		default:
			CONS_Printf(M_GetText("That is not a valid number.\n"));
			return;
	}

	for (think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if (think->function.acp1 != action)
			continue;

		count++;
	}

	CONS_Printf("%d\n", count);
}

void Command_CountMobjs_f(void)
{
	thinker_t *th;
	mobjtype_t i;
	INT32 count;

	if (gamestate != GS_LEVEL)
	{
		CONS_Printf(M_GetText("You must be in a level to use this.\n"));
		return;
	}

	if (COM_Argc() >= 2)
	{
		size_t j;
		for (j = 1; j < COM_Argc(); j++)
		{
			i = atoi(COM_Argv(j));
			if (i >= NUMMOBJTYPES)
			{
				CONS_Printf(M_GetText("Object number %d out of range (max %d).\n"), i, NUMMOBJTYPES-1);
				continue;
			}

			count = 0;

			for (th = thinkercap.next; th != &thinkercap; th = th->next)
			{
				if (th->function.acp1 != (actionf_p1)P_MobjThinker)
					continue;

				if (((mobj_t *)th)->type == i)
					count++;
			}

			CONS_Printf(M_GetText("There are %d objects of type %d currently in the level.\n"), count, i);
		}
		return;
	}

	CONS_Printf(M_GetText("Count of active objects in level:\n"));

	for (i = 0; i < NUMMOBJTYPES; i++)
	{
		count = 0;

		for (th = thinkercap.next; th != &thinkercap; th = th->next)
		{
			if (th->function.acp1 != (actionf_p1)P_MobjThinker)
				continue;

			if (((mobj_t *)th)->type == i)
				count++;
		}

		if (count > 0) // Don't bother displaying if there are none of this type!
			CONS_Printf(" * %d: %d\n", i, count);
	}
}

//
// P_InitThinkers
//
void P_InitThinkers(void)
{
	thinkercap.prev = thinkercap.next = &thinkercap;
	waypointcap = NULL;
}

//
// P_AddThinker
// Adds a new thinker at the end of the list.
//
void P_AddThinker(thinker_t *thinker)
{
	thinkercap.prev->next = thinker;
	thinker->next = &thinkercap;
	thinker->prev = thinkercap.prev;
	thinkercap.prev = thinker;

	thinker->references = 0;    // killough 11/98: init reference counter to 0
}

//
// killough 11/98:
//
// Make currentthinker external, so that P_RemoveThinkerDelayed
// can adjust currentthinker when thinkers self-remove.

static thinker_t *currentthinker;

//
// P_RemoveThinkerDelayed()
//
// Called automatically as part of the thinker loop in P_RunThinkers(),
// on nodes which are pending deletion.
//
// If this thinker has no more pointers referencing it indirectly,
// remove it, and set currentthinker to one node preceeding it, so
// that the next step in P_RunThinkers() will get its successor.
//
void P_RemoveThinkerDelayed(void *pthinker)
{
	thinker_t *thinker = pthinker;
	if (!thinker->references)
	{
		{
			/* Remove from main thinker list */
			thinker_t *next = thinker->next;
			/* Note that currentthinker is guaranteed to point to us,
			 * and since we're freeing our memory, we had better change that. So
			 * point it to thinker->prev, so the iterator will correctly move on to
			 * thinker->prev->next = thinker->next */
			(next->prev = currentthinker = thinker->prev)->next = next;
		}
		R_DestroyLevelInterpolators(thinker);
		Z_Free(thinker);
	}
}

//
// P_RemoveThinker
//
// Deallocation is lazy -- it will not actually be freed
// until its thinking turn comes up.
//
// killough 4/25/98:
//
// Instead of marking the function with -1 value cast to a function pointer,
// set the function to P_RemoveThinkerDelayed(), so that later, it will be
// removed automatically as part of the thinker process.
//
void P_RemoveThinker(thinker_t *thinker)
{
#ifdef HAVE_BLUA
	LUA_InvalidateUserdata(thinker);
#endif
	thinker->function.acp1 = P_RemoveThinkerDelayed;
}

/*
 * P_SetTarget
 *
 * This function is used to keep track of pointer references to mobj thinkers.
 * In Doom, objects such as lost souls could sometimes be removed despite
 * their still being referenced. In Boom, 'target' mobj fields were tested
 * during each gametic, and any objects pointed to by them would be prevented
 * from being removed. But this was incomplete, and was slow (every mobj was
 * checked during every gametic). Now, we keep a count of the number of
 * references, and delay removal until the count is 0.
 */

mobj_t *P_SetTarget(mobj_t **mop, mobj_t *targ)
{
	if (*mop)              // If there was a target already, decrease its refcount
		(*mop)->thinker.references--;
if ((*mop = targ) != NULL) // Set new target and if non-NULL, increase its counter
		targ->thinker.references++;
	return targ;
}

//
// P_RunThinkers
//
// killough 4/25/98:
//
// Fix deallocator to stop using "next" pointer after node has been freed
// (a Doom bug).
//
// Process each thinker. For thinkers which are marked deleted, we must
// load the "next" pointer prior to freeing the node. In Doom, the "next"
// pointer was loaded AFTER the thinker was freed, which could have caused
// crashes.
//
// But if we are not deleting the thinker, we should reload the "next"
// pointer after calling the function, in case additional thinkers are
// added at the end of the list.
//
// killough 11/98:
//
// Rewritten to delete nodes implicitly, by making currentthinker
// external and using P_RemoveThinkerDelayed() implicitly.
//
static inline void P_RunThinkers(void)
{
	for (currentthinker = thinkercap.next; currentthinker != &thinkercap; currentthinker = currentthinker->next)
	{
		if (currentthinker->function.acp1)
			currentthinker->function.acp1(currentthinker);
	}
}

//
// P_DoAutobalanceTeams()
//
// Determine if the teams are unbalanced, and if so, move a player to the other team.
//
/*static void P_DoAutobalanceTeams(void)
{
	changeteam_union NetPacket;
	UINT16 usvalue;
	INT32 i=0;
	INT32 red=0, blue=0;
	INT32 redarray[MAXPLAYERS], bluearray[MAXPLAYERS];
	INT32 redflagcarrier = 0, blueflagcarrier = 0;
	INT32 totalred = 0, totalblue = 0;

	NetPacket.value.l = NetPacket.value.b = 0;
	memset(redarray, 0, sizeof(redarray));
	memset(bluearray, 0, sizeof(bluearray));

	// Only do it if we have enough room in the net buffer to send it.
	// Otherwise, come back next time and try again.
	if (sizeof(usvalue) > GetFreeXCmdSize())
		return;

	//We have to store the players in an array with the rest of their team.
	//We can then pick a random player to be forced to change teams.
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i] && players[i].ctfteam)
		{
			if (players[i].ctfteam == 1)
			{
				if (!players[i].gotflag)
				{
					redarray[red] = i; //store the player's node.
					red++;
				}
				else
					redflagcarrier++;
			}
			else
			{
				if (!players[i].gotflag)
				{
					bluearray[blue] = i; //store the player's node.
					blue++;
				}
				else
					blueflagcarrier++;
			}
		}
	}

	totalred = red + redflagcarrier;
	totalblue = blue + blueflagcarrier;

	if ((abs(totalred - totalblue) > cv_autobalance.value))
	{
		if (totalred > totalblue)
		{
			i = M_RandomKey(red);
			NetPacket.packet.newteam = 2;
			NetPacket.packet.playernum = redarray[i];
			NetPacket.packet.verification = true;
			NetPacket.packet.autobalance = true;

			usvalue  = SHORT(NetPacket.value.l|NetPacket.value.b);
			SendNetXCmd(XD_TEAMCHANGE, &usvalue, sizeof(usvalue));
		}

		if (totalblue > totalred)
		{
			i = M_RandomKey(blue);
			NetPacket.packet.newteam = 1;
			NetPacket.packet.playernum = bluearray[i];
			NetPacket.packet.verification = true;
			NetPacket.packet.autobalance = true;

			usvalue  = SHORT(NetPacket.value.l|NetPacket.value.b);
			SendNetXCmd(XD_TEAMCHANGE, &usvalue, sizeof(usvalue));
		}
	}
}

//
// P_DoTeamscrambling()
//
// If a team scramble has been started, scramble one person from the
// pre-made scramble array. Said array is created in TeamScramble_OnChange()
//
void P_DoTeamscrambling(void)
{
	changeteam_union NetPacket;
	UINT16 usvalue;
	NetPacket.value.l = NetPacket.value.b = 0;

	// Only do it if we have enough room in the net buffer to send it.
	// Otherwise, come back next time and try again.
	if (sizeof(usvalue) > GetFreeXCmdSize())
		return;

	if (scramblecount < scrambletotal)
	{
		if (players[scrambleplayers[scramblecount]].ctfteam != scrambleteams[scramblecount])
		{
			NetPacket.packet.newteam = scrambleteams[scramblecount];
			NetPacket.packet.playernum = scrambleplayers[scramblecount];
			NetPacket.packet.verification = true;
			NetPacket.packet.scrambled = true;

			usvalue = SHORT(NetPacket.value.l|NetPacket.value.b);
			SendNetXCmd(XD_TEAMCHANGE, &usvalue, sizeof(usvalue));
		}

		scramblecount++; //Increment, and get to the next player when we come back here next time.
	}
	else
		CV_SetValue(&cv_teamscramble, 0);
}

static inline void P_DoSpecialStageStuff(void)
{
	boolean inwater = false;
	INT32 i;

	// Can't drown in a special stage
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i] || players[i].spectator)
			continue;

		players[i].powers[pw_underwater] = players[i].powers[pw_spacetime] = 0;
	}

	if (sstimer < 15*TICRATE+6 && sstimer > 7 && (mapheaderinfo[gamemap-1]->levelflags & LF_SPEEDMUSIC))
		S_SpeedMusic(1.4f);

	if (sstimer < 7 && sstimer > 0) // The special stage time is up!
	{
		sstimer = 0;
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i])
			{
				players[i].exiting = raceexittime+1;
				players[i].pflags &= ~PF_GLIDING;
			}

			if (i == consoleplayer)
				S_StartSound(NULL, sfx_lose);
		}

		if (mapheaderinfo[gamemap-1]->levelflags & LF_SPEEDMUSIC)
			S_SpeedMusic(1.0f);

		stagefailed = true;
	}

	if (sstimer > 1) // As long as time isn't up...
	{
		UINT32 ssrings = 0;
		// Count up the rings of all the players and see if
		// they've collected the required amount.
		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i])
			{
				ssrings += (players[i].mo->health-1);

				// If in water, deplete timer 6x as fast.
				if ((players[i].mo->eflags & MFE_TOUCHWATER)
					|| (players[i].mo->eflags & MFE_UNDERWATER))
					inwater = true;
			}

		if (ssrings >= totalrings && totalrings > 0)
		{
			// Halt all the players
			for (i = 0; i < MAXPLAYERS; i++)
				if (playeringame[i])
				{
					players[i].mo->momx = players[i].mo->momy = 0;
					players[i].exiting = raceexittime+1;
				}

			sstimer = 0;

			P_GiveEmerald(true);
		}

		// Decrement the timer
		if (!objectplacing)
		{
			if (inwater)
				sstimer -= 6;
			else
				sstimer--;
		}
	}
}

static inline void P_DoTagStuff(void)
{
	INT32 i;

	// tell the netgame who the initial IT person is.
	if (leveltime == TICRATE)
	{
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (players[i].pflags & PF_TAGIT)
			{
				CONS_Printf(M_GetText("%s is now IT!\n"), player_names[i]); // Tell everyone who is it!
				break;
			}
		}
	}

	//increment survivor scores
	if (leveltime % TICRATE == 0 && leveltime > (hidetime * TICRATE))
	{
		INT32 participants = 0;

		for (i=0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i] && !players[i].spectator)
				participants++;
		}

		for (i=0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i] && !players[i].spectator && players[i].playerstate == PST_LIVE
			&& !(players[i].pflags & (PF_TAGIT|PF_TAGGED)))
				//points given is the number of participating players divided by two.
				P_AddPlayerScore(&players[i], participants/2);
		}
	}
}

static inline void P_DoCTFStuff(void)
{
	// Automatic team balance for CTF and team match
	if (leveltime % (TICRATE * 5) == 0) //only check once per five seconds for the sake of CPU conservation.
	{
		// Do not attempt to autobalance and scramble teams at the same time.
		// Only the server should execute this. No verified admins, please.
		if ((cv_autobalance.value && !cv_teamscramble.value) && cv_allowteamchange.value && server)
			P_DoAutobalanceTeams();
	}

	// Team scramble code for team match and CTF.
	if ((leveltime % (TICRATE/7)) == 0)
	{
		// If we run out of time in the level, the beauty is that
		// the Y_Ticker() team scramble code will pick it up.
		if (cv_teamscramble.value && server)
			P_DoTeamscrambling();
	}
}*/

//
// P_Ticker
//
void P_Ticker(boolean run)
{
	INT32 i;

	//Increment jointime even if paused.
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
			++players[i].jointime;

	if (objectplacing)
	{
		if (OP_FreezeObjectplace())
		{
			P_MapStart();
			R_UpdateMobjInterpolators();
			OP_ObjectplaceMovement(&players[0]);
			P_MoveChaseCamera(&players[0], &camera[0], false);
			R_UpdateViewInterpolation();
			P_MapEnd();
			return;
		}
	}

	// Check for pause or menu up in single player
	if (paused || P_AutoPause())
	{
		if (demo.rewinding && leveltime > 0)
		{
			leveltime = (leveltime-1) & ~3;
			G_PreviewRewind(leveltime);
		}
		else if (demo.freecam && democam.cam)	// special case: allow freecam to MOVE during pause!
			P_DemoCameraMovement(democam.cam);

		return;
	}

	for (i = 0; i <= splitscreen; i++)
		postimgtype[i] = postimg_none;

	P_MapStart();

	if (run)
	{
		R_UpdateMobjInterpolators();

		if (demo.recording)
		{
			G_WriteDemoExtraData();
			for (i = 0; i < MAXPLAYERS; i++)
				if (playeringame[i])
					G_WriteDemoTiccmd(&players[i].cmd, i);
		}
		if (demo.playback)
		{

#ifdef DEMO_COMPAT_100
			if (demo.version == 0x0001)
			{
				G_ReadDemoTiccmd(&players[consoleplayer].cmd, 0);
			}
			else
			{
#endif
				G_ReadDemoExtraData();
				for (i = 0; i < MAXPLAYERS; i++)
					if (playeringame[i])
					{
						//@TODO all this throwdir stuff shouldn't be here! But it's added to maintain 1.0.4 compat for now...
						// Remove for 1.1!
						if (players[i].cmd.buttons & BT_FORWARD)
							players[i].kartstuff[k_throwdir] = 1;
						else if (players[i].cmd.buttons & BT_BACKWARD)
							players[i].kartstuff[k_throwdir] = -1;
						else
							players[i].kartstuff[k_throwdir] = 0;

						G_ReadDemoTiccmd(&players[i].cmd, i);
					}
#ifdef DEMO_COMPAT_100
			}
#endif
		}

#ifdef HAVE_BLUA
		LUAh_PreThinkFrame();
#endif

		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i] && players[i].mo && !P_MobjWasRemoved(players[i].mo))
				P_PlayerThink(&players[i]);
	}

	// Keep track of how long they've been playing!
	if (!demo.playback) // Don't increment if a demo is playing.
		totalplaytime++;

	/*if (!useNightsSS && G_IsSpecialStage(gamemap))
		P_DoSpecialStageStuff();

	if (runemeraldmanager)
		P_EmeraldManager(); // Power stone mode*/

	if (run)
	{
		P_RunThinkers();

		// Run any "after all the other thinkers" stuff
		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i] && players[i].mo && !P_MobjWasRemoved(players[i].mo))
				P_PlayerAfterThink(&players[i]);

#ifdef HAVE_BLUA
		LUAh_ThinkFrame();
#endif
	}

	// Run shield positioning
	//P_RunShields();
	P_RunOverlays();

	P_RunShadows();

	P_UpdateSpecials();
	P_RespawnSpecials();

	// Lightning, rain sounds, etc.
	P_PrecipitationEffects();

	if (run)
		leveltime++;

	// as this is mostly used for HUD stuff, add the record attack specific hack to it as well!
	if (!(modeattacking && !demo.playback) || leveltime >= starttime - TICRATE*4)
		timeinmap++;

	/*if (G_TagGametype())
		P_DoTagStuff();

	if (G_GametypeHasTeams())
		P_DoCTFStuff();*/

	if (run)
	{
		if (countdowntimer && --countdowntimer <= 0)
		{
			countdowntimer = 0;
			countdowntimeup = true;
			for (i = 0; i < MAXPLAYERS; i++)
			{
				if (!playeringame[i] || players[i].spectator)
					continue;

				if (!players[i].mo)
					continue;

				P_DamageMobj(players[i].mo, NULL, NULL, 10000);
			}
		}

		if (racecountdown > 1)
			racecountdown--;

		if (exitcountdown > 1)
			exitcountdown--;

		if (indirectitemcooldown > 0)
			indirectitemcooldown--;
		if (hyubgone > 0)
			hyubgone--;

		if (G_RaceGametype())
		{
			K_UpdateSpectateGrief();
		}

		if (G_BattleGametype())
		{
			if (wantedcalcdelay && --wantedcalcdelay <= 0)
				K_CalculateBattleWanted();
		}

		if (quake.time)
		{
			fixed_t ir = quake.intensity>>1;
			/// \todo Calculate distance from epicenter if set and modulate the intensity accordingly based on radius.
			quake.x = M_RandomRange(-ir,ir);
			quake.y = M_RandomRange(-ir,ir);
			quake.z = M_RandomRange(-ir,ir);
			--quake.time;
		}
		else
			quake.x = quake.y = quake.z = 0;

		if (metalplayback)
			G_ReadMetalTic(metalplayback);
		if (metalrecording)
			G_WriteMetalTic(players[consoleplayer].mo);

		if (demo.recording)
		{
			INT32 axis = JoyAxis(AXISLOOKBACK, 1);

			G_WriteAllGhostTics();

			if (cv_recordmultiplayerdemos.value && (demo.savemode == DSM_NOTSAVING || demo.savemode == DSM_WILLAUTOSAVE))
				if (demo.savebutton && demo.savebutton + 3*TICRATE < leveltime && (InputDown(gc_lookback, 1) || (cv_usejoystick.value && axis > 0)))
					demo.savemode = DSM_TITLEENTRY;
		}
		else if (demo.playback) // Use Ghost data for consistency checks.
		{
#ifdef DEMO_COMPAT_100
			if (demo.version == 0x0001)
				G_ConsGhostTic(0);
			else
#endif
			G_ConsAllGhostTics();
		}

		if (modeattacking)
			G_GhostTicker();

		if (mapreset > 1
			&& --mapreset <= 1
			&& server) // Remember: server uses it for mapchange, but EVERYONE ticks down for the animation
				D_MapChange(gamemap, gametype, encoremode, true, 0, false, false);

#ifdef HAVE_BLUA
		LUAh_PostThinkFrame();
#endif
	}

	// Always move the camera.
	for (i = 0; i <= splitscreen; i++)
	{
		if (camera[i].chase)
			P_MoveChaseCamera(&players[displayplayers[i]], &camera[i], false);
	}

	if (run)
	{
		R_UpdateLevelInterpolators();
		R_UpdateViewInterpolation();

		// Hack: ensure newview is assigned every tic.
		// Ensures view interpolation is T-1 to T in poor network conditions
		// We need a better way to assign view state decoupled from game logic
		if (rendermode != render_none)
		{
			for (i = 0; i <= splitscreen; i++)
			{
				player_t *player = &players[displayplayers[i]];
				boolean isSkyVisibleForPlayer = skyVisiblePerPlayer[i];
				if (!player->mo)
					continue;
				if (isSkyVisibleForPlayer && skyboxmo[0] && cv_skybox.value)
				{
					R_SkyboxFrame(player);
				}
				R_SetupFrame(player, (skyboxmo[0] && cv_skybox.value));
			}
		}
	}

	P_MapEnd();

	if (demo.playback)
		G_StoreRewindInfo();

//	Z_CheckMemCleanup();
}

// Abbreviated ticker for pre-loading, calls thinkers and assorted things
void P_PreTicker(INT32 frames)
{
	INT32 i;
	ticcmd_t temptic;

	for (i = 0; i <= splitscreen; i++)
		postimgtype[i] = postimg_none;

	hook_defrosting = frames;

	while (hook_defrosting)
	{
		P_MapStart();

		R_UpdateMobjInterpolators();

#ifdef HAVE_BLUA
		LUAh_PreThinkFrame();
#endif

		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i] && players[i].mo && !P_MobjWasRemoved(players[i].mo))
			{
				// stupid fucking cmd hack
				// if it isn't for this, players can move in preticker time
				// (and disrupt demo recording and other things !!)
				memcpy(&temptic, &players[i].cmd, sizeof(ticcmd_t));
				memset(&players[i].cmd, 0, sizeof(ticcmd_t));
				// correct angle on spawn...
				players[i].cmd.angleturn = temptic.angleturn;

				P_PlayerThink(&players[i]);

				memcpy(&players[i].cmd, &temptic, sizeof(ticcmd_t));
			}

		P_RunThinkers();

		// Run any "after all the other thinkers" stuff
		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i] && players[i].mo && !P_MobjWasRemoved(players[i].mo))
				P_PlayerAfterThink(&players[i]);

#ifdef HAVE_BLUA
		LUAh_ThinkFrame();
#endif

		// Run shield positioning
		//P_RunShields();
		P_RunOverlays();

		P_UpdateSpecials();
		P_RespawnSpecials();

#ifdef HAVE_BLUA
		LUAh_PostThinkFrame();
#endif

		R_UpdateLevelInterpolators();
		R_UpdateViewInterpolation();
		R_ResetViewInterpolation(0);

		P_MapEnd();

		hook_defrosting--;
	}
}
