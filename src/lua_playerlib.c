// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2012-2016 by John "JTE" Muniz.
// Copyright (C) 2012-2018 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  lua_playerlib.c
/// \brief player object library for Lua scripting

#include "doomdef.h"
#ifdef HAVE_BLUA
#include "fastcmp.h"
#include "p_mobj.h"
#include "d_player.h"
#include "g_game.h"
#include "p_local.h"
#include "d_clisrv.h"

#include "lua_script.h"
#include "lua_libs.h"
#include "lua_hud.h" // hud_running errors
#include "lua_hook.h"	// hook_cmd_running

static int lib_iteratePlayers(lua_State *L)
{
	INT32 i = -1;
	if (lua_gettop(L) < 2)
	{
		//return luaL_error(L, "Don't call players.iterate() directly, use it as 'for player in players.iterate do <block> end'.");
		lua_pushcfunction(L, lib_iteratePlayers);
		return 1;
	}
	lua_settop(L, 2);
	lua_remove(L, 1); // state is unused.
	if (!lua_isnil(L, 1))
		i = (INT32)(*((player_t **)luaL_checkudata(L, 1, META_PLAYER)) - players);
	for (i++; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i])
			continue;
		if (!players[i].mo)
			continue;
		LUA_PushUserdata(L, &players[i], META_PLAYER);
		return 1;
	}
	return 0;
}

static int lib_getPlayer(lua_State *L)
{
	const char *field;
	// i -> players[i]
	if (lua_type(L, 2) == LUA_TNUMBER)
	{
		lua_Integer i = luaL_checkinteger(L, 2);
		if (i < 0 || i >= MAXPLAYERS)
			return luaL_error(L, "players[] index %d out of range (0 - %d)", i, MAXPLAYERS-1);
		if (!playeringame[i])
			return 0;
		if (!players[i].mo)
			return 0;
		LUA_PushUserdata(L, &players[i], META_PLAYER);
		return 1;
	}

	field = luaL_checkstring(L, 2);
	if (fastcmp(field,"iterate"))
	{
		lua_pushcfunction(L, lib_iteratePlayers);
		return 1;
	}
	return 0;
}

// #players -> MAXPLAYERS
static int lib_lenPlayer(lua_State *L)
{
	lua_pushinteger(L, MAXPLAYERS);
	return 1;
}

// Same deal as the three functions above but for displayplayers

static int lib_iterateDisplayplayers(lua_State *L)
{
	INT32 i = -1;
	INT32 temp = -1;
	INT32 iter = 0;

	if (lua_gettop(L) < 2)
	{
		//return luaL_error(L, "Don't call displayplayers.iterate() directly, use it as 'for player in displayplayers.iterate do <block> end'.");
		lua_pushcfunction(L, lib_iterateDisplayplayers);
		return 1;
	}
	lua_settop(L, 2);
	lua_remove(L, 1); // state is unused.
	if (!lua_isnil(L, 1))
	{
		temp = (INT32)(*((player_t **)luaL_checkudata(L, 1, META_PLAYER)) - players);	// get the player # of the last iterated player.

		// @FIXME:
		// I didn't quite find a better way for this; Here, we go back to which player in displayplayers we last iterated to resume the for loop below for this new function call
		// I don't understand enough about how the Lua stacks work to get this to work in possibly a single line.
		// So anyone feel free to correct this!

		for (; iter < MAXSPLITSCREENPLAYERS; iter++)
		{
			if (displayplayers[iter] == temp)
			{
				i = iter;
				break;
			}
		}
	}

	for (i++; i < MAXSPLITSCREENPLAYERS; i++)
	{
		if (i > splitscreen || !playeringame[displayplayers[i]])
			return 0;	// Stop! There are no more players for us to go through. There will never be a player gap in displayplayers.

		if (!players[displayplayers[i]].mo)
			continue;
		LUA_PushUserdata(L, &players[displayplayers[i]], META_PLAYER);
		lua_pushinteger(L, i);	// push this to recall what number we were on for the next function call. I suppose this also means you can retrieve the splitscreen player number with 'for p, n in displayplayers.iterate'!
		return 2;
	}
	return 0;
}

static int lib_getDisplayplayers(lua_State *L)
{
	const char *field;
	// i -> players[i]
	if (lua_type(L, 2) == LUA_TNUMBER)
	{
		lua_Integer i = luaL_checkinteger(L, 2);
		if (i < 0 || i >= MAXSPLITSCREENPLAYERS)
			return luaL_error(L, "displayplayers[] index %d out of range (0 - %d)", i, MAXSPLITSCREENPLAYERS-1);
		if (i > splitscreen)
			return 0;
		if (!playeringame[displayplayers[i]])
			return 0;
		if (!players[displayplayers[i]].mo)
			return 0;
		LUA_PushUserdata(L, &players[displayplayers[i]], META_PLAYER);
		return 1;
	}

	field = luaL_checkstring(L, 2);
	if (fastcmp(field,"iterate"))
	{
		lua_pushcfunction(L, lib_iterateDisplayplayers);
		return 1;
	}
	return 0;
}

// #displayplayers -> MAXSPLITSCREENPLAYERS
static int lib_lenDisplayplayers(lua_State *L)
{
	lua_pushinteger(L, MAXSPLITSCREENPLAYERS);
	return 1;
}

static int player_get(lua_State *L)
{
	player_t *plr = *((player_t **)luaL_checkudata(L, 1, META_PLAYER));
	const char *field = luaL_checkstring(L, 2);

	if (!plr) {
		if (fastcmp(field,"valid")) {
			lua_pushboolean(L, false);
			return 1;
		}
		return LUA_ErrInvalid(L, "player_t");
	}

	if (fastcmp(field,"valid"))
		lua_pushboolean(L, true);
	else if (fastcmp(field,"name"))
		lua_pushstring(L, player_names[plr-players]);
	else if (fastcmp(field,"mo"))
	{
		if (plr->spectator)
			lua_pushnil(L);
		else
			LUA_PushUserdata(L, plr->mo, META_MOBJ);
	}
	else if (fastcmp(field,"cmd"))
		LUA_PushUserdata(L, &plr->cmd, META_TICCMD);
	else if (fastcmp(field,"playerstate"))
		lua_pushinteger(L, plr->playerstate);
	else if (fastcmp(field,"viewz"))
		lua_pushfixed(L, plr->viewz);
	else if (fastcmp(field,"viewheight"))
		lua_pushfixed(L, plr->viewheight);
	/*else if (fastcmp(field,"deltaviewheight"))
		lua_pushfixed(L, plr->deltaviewheight);
	else if (fastcmp(field,"bob"))
		lua_pushfixed(L, plr->bob);*/
	else if (fastcmp(field,"aiming"))
		lua_pushangle(L, plr->aiming);
	else if (fastcmp(field,"health"))
		lua_pushinteger(L, plr->health);
	else if (fastcmp(field,"pity"))
		lua_pushinteger(L, plr->pity);
	else if (fastcmp(field,"currentweapon"))
		lua_pushinteger(L, plr->currentweapon);
	else if (fastcmp(field,"ringweapons"))
		lua_pushinteger(L, plr->ringweapons);
	else if (fastcmp(field,"powers"))
		LUA_PushUserdata(L, plr->powers, META_POWERS);
	else if (fastcmp(field,"kartstuff"))
		LUA_PushUserdata(L, plr->kartstuff, META_KARTSTUFF);
	else if (fastcmp(field,"frameangle"))
		lua_pushangle(L, plr->frameangle);
	else if (fastcmp(field,"pflags"))
		lua_pushinteger(L, plr->pflags);
	else if (fastcmp(field,"panim"))
		lua_pushinteger(L, plr->panim);
	else if (fastcmp(field,"flashcount"))
		lua_pushinteger(L, plr->flashcount);
	else if (fastcmp(field,"flashpal"))
		lua_pushinteger(L, plr->flashpal);
	else if (fastcmp(field,"skincolor"))
		lua_pushinteger(L, plr->skincolor);
	else if (fastcmp(field,"score"))
		lua_pushinteger(L, plr->score);
	else if (fastcmp(field,"dashspeed"))
		lua_pushfixed(L, plr->dashspeed);
	else if (fastcmp(field,"dashtime"))
		lua_pushinteger(L, plr->dashtime);
	// SRB2kart
	else if (fastcmp(field,"kartspeed"))
		lua_pushinteger(L, plr->kartspeed);
	else if (fastcmp(field,"kartweight"))
		lua_pushinteger(L, plr->kartweight);
	//
	else if (fastcmp(field,"charflags"))
		lua_pushinteger(L, plr->charflags);
	else if (fastcmp(field,"lives"))
		lua_pushinteger(L, plr->lives);
	else if (fastcmp(field,"continues"))
		lua_pushinteger(L, plr->continues);
	else if (fastcmp(field,"xtralife"))
		lua_pushinteger(L, plr->xtralife);
	else if (fastcmp(field,"gotcontinue"))
		lua_pushinteger(L, plr->gotcontinue);
	else if (fastcmp(field,"speed"))
		lua_pushfixed(L, plr->speed);
	else if (fastcmp(field,"jumping"))
		lua_pushboolean(L, plr->jumping);
	else if (fastcmp(field,"secondjump"))
		lua_pushinteger(L, plr->secondjump);
	else if (fastcmp(field,"fly1"))
		lua_pushinteger(L, plr->fly1);
	else if (fastcmp(field,"scoreadd"))
		lua_pushinteger(L, plr->scoreadd);
	else if (fastcmp(field,"glidetime"))
		lua_pushinteger(L, plr->glidetime);
	else if (fastcmp(field,"climbing"))
		lua_pushinteger(L, plr->climbing);
	else if (fastcmp(field,"deadtimer"))
		lua_pushinteger(L, plr->deadtimer);
	else if (fastcmp(field,"exiting"))
		lua_pushinteger(L, plr->exiting);
	else if (fastcmp(field,"homing"))
		lua_pushinteger(L, plr->homing);
	else if (fastcmp(field,"skidtime"))
		lua_pushinteger(L, plr->skidtime);
	else if (fastcmp(field,"cmomx"))
		lua_pushfixed(L, plr->cmomx);
	else if (fastcmp(field,"cmomy"))
		lua_pushfixed(L, plr->cmomy);
	else if (fastcmp(field,"rmomx"))
		lua_pushfixed(L, plr->rmomx);
	else if (fastcmp(field,"rmomy"))
		lua_pushfixed(L, plr->rmomy);
	else if (fastcmp(field,"numboxes"))
		lua_pushinteger(L, plr->numboxes);
	else if (fastcmp(field,"totalring"))
		lua_pushinteger(L, plr->totalring);
	else if (fastcmp(field,"realtime"))
		lua_pushinteger(L, plr->realtime);
	else if (fastcmp(field,"laps"))
		lua_pushinteger(L, plr->laps);
	else if (fastcmp(field,"ctfteam"))
		lua_pushinteger(L, plr->ctfteam);
	else if (fastcmp(field,"gotflag"))
		lua_pushinteger(L, plr->gotflag);
	else if (fastcmp(field,"weapondelay"))
		lua_pushinteger(L, plr->weapondelay);
	else if (fastcmp(field,"tossdelay"))
		lua_pushinteger(L, plr->tossdelay);
	else if (fastcmp(field,"starpostx"))
		lua_pushinteger(L, plr->starpostx);
	else if (fastcmp(field,"starposty"))
		lua_pushinteger(L, plr->starposty);
	else if (fastcmp(field,"starpostz"))
		lua_pushinteger(L, plr->starpostz);
	else if (fastcmp(field,"starpostnum"))
		lua_pushinteger(L, plr->starpostnum);
	else if (fastcmp(field,"starposttime"))
		lua_pushinteger(L, plr->starposttime);
	else if (fastcmp(field,"starpostangle"))
		lua_pushangle(L, plr->starpostangle);
	else if (fastcmp(field,"angle_pos"))
		lua_pushangle(L, plr->angle_pos);
	else if (fastcmp(field,"old_angle_pos"))
		lua_pushangle(L, plr->old_angle_pos);
	else if (fastcmp(field,"axis1"))
		LUA_PushUserdata(L, plr->axis1, META_MOBJ);
	else if (fastcmp(field,"axis2"))
		LUA_PushUserdata(L, plr->axis2, META_MOBJ);
	else if (fastcmp(field,"bumpertime"))
		lua_pushinteger(L, plr->bumpertime);
	else if (fastcmp(field,"flyangle"))
		lua_pushinteger(L, plr->flyangle);
	else if (fastcmp(field,"drilltimer"))
		lua_pushinteger(L, plr->drilltimer);
	else if (fastcmp(field,"linkcount"))
		lua_pushinteger(L, plr->linkcount);
	else if (fastcmp(field,"linktimer"))
		lua_pushinteger(L, plr->linktimer);
	else if (fastcmp(field,"anotherflyangle"))
		lua_pushinteger(L, plr->anotherflyangle);
	else if (fastcmp(field,"nightstime"))
		lua_pushinteger(L, plr->nightstime);
	else if (fastcmp(field,"drillmeter"))
		lua_pushinteger(L, plr->drillmeter);
	else if (fastcmp(field,"drilldelay"))
		lua_pushinteger(L, plr->drilldelay);
	else if (fastcmp(field,"bonustime"))
		lua_pushboolean(L, plr->bonustime);
	else if (fastcmp(field,"capsule"))
		LUA_PushUserdata(L, plr->capsule, META_MOBJ);
	else if (fastcmp(field,"mare"))
		lua_pushinteger(L, plr->mare);
	else if (fastcmp(field,"marebegunat"))
		lua_pushinteger(L, plr->marebegunat);
	else if (fastcmp(field,"startedtime"))
		lua_pushinteger(L, plr->startedtime);
	else if (fastcmp(field,"finishedtime"))
		lua_pushinteger(L, plr->finishedtime);
	else if (fastcmp(field,"finishedrings"))
		lua_pushinteger(L, plr->finishedrings);
	else if (fastcmp(field,"marescore"))
		lua_pushinteger(L, plr->marescore);
	else if (fastcmp(field,"lastmarescore"))
		lua_pushinteger(L, plr->lastmarescore);
	else if (fastcmp(field,"lastmare"))
		lua_pushinteger(L, plr->lastmare);
	else if (fastcmp(field,"maxlink"))
		lua_pushinteger(L, plr->maxlink);
	else if (fastcmp(field,"texttimer"))
		lua_pushinteger(L, plr->texttimer);
	else if (fastcmp(field,"textvar"))
		lua_pushinteger(L, plr->textvar);
	else if (fastcmp(field,"lastsidehit"))
		lua_pushinteger(L, plr->lastsidehit);
	else if (fastcmp(field,"lastlinehit"))
		lua_pushinteger(L, plr->lastlinehit);
	else if (fastcmp(field,"losstime"))
		lua_pushinteger(L, plr->losstime);
	else if (fastcmp(field,"timeshit"))
		lua_pushinteger(L, plr->timeshit);
	else if (fastcmp(field,"onconveyor"))
		lua_pushinteger(L, plr->onconveyor);
	else if (fastcmp(field,"awayviewmobj"))
		LUA_PushUserdata(L, plr->awayviewmobj, META_MOBJ);
	else if (fastcmp(field,"awayviewtics"))
		lua_pushinteger(L, plr->awayviewtics);
	else if (fastcmp(field,"awayviewaiming"))
		lua_pushangle(L, plr->awayviewaiming);
	else if (fastcmp(field,"spectator"))
		lua_pushboolean(L, plr->spectator);
	else if (fastcmp(field,"bot"))
		lua_pushinteger(L, plr->bot);
	else if (fastcmp(field,"jointime"))
		lua_pushinteger(L, plr->jointime);
	else if (fastcmp(field,"splitscreenindex"))
		lua_pushinteger(L, plr->splitscreenindex);
#ifdef HWRENDER
	else if (fastcmp(field,"fovadd"))
		lua_pushfixed(L, plr->fovadd);
#endif
	else if (fastcmp(field,"ping"))
		lua_pushinteger(L, playerpingtable[( plr - players )]);
	else {
		lua_getfield(L, LUA_REGISTRYINDEX, LREG_EXTVARS);
		I_Assert(lua_istable(L, -1));
		lua_pushlightuserdata(L, plr);
		lua_rawget(L, -2);
		if (!lua_istable(L, -1)) { // no extra values table
			CONS_Debug(DBG_LUA, M_GetText("'%s' has no extvars table or field named '%s'; returning nil.\n"), "player_t", field);
			return 0;
		}
		lua_getfield(L, -1, field);
		if (lua_isnil(L, -1)) // no value for this field
			CONS_Debug(DBG_LUA, M_GetText("'%s' has no field named '%s'; returning nil.\n"), "player_t", field);
	}

	return 1;
}

#define NOSET luaL_error(L, LUA_QL("player_t") " field " LUA_QS " should not be set directly.", field)
static int player_set(lua_State *L)
{
	player_t *plr = *((player_t **)luaL_checkudata(L, 1, META_PLAYER));
	const char *field = luaL_checkstring(L, 2);
	if (!plr)
		return LUA_ErrInvalid(L, "player_t");

	if (hud_running)
		return luaL_error(L, "Do not alter player_t in HUD rendering code!");

	if (hook_cmd_running)
		return luaL_error(L, "Do not alter player_t in BuildCMD code!");

	if (fastcmp(field,"mo")) {
		mobj_t *newmo = *((mobj_t **)luaL_checkudata(L, 3, META_MOBJ));
		plr->mo->player = NULL; // remove player pointer from old mobj
		(newmo->player = plr)->mo = newmo; // set player pointer for new mobj, and set new mobj as the player's mobj
	}
	else if (fastcmp(field,"cmd"))
		return NOSET;
	else if (fastcmp(field,"playerstate"))
		plr->playerstate = luaL_checkinteger(L, 3);
	else if (fastcmp(field,"viewz"))
		plr->viewz = luaL_checkfixed(L, 3);
	else if (fastcmp(field,"viewheight"))
		plr->viewheight = luaL_checkfixed(L, 3);
	/*else if (fastcmp(field,"deltaviewheight"))
		plr->deltaviewheight = luaL_checkfixed(L, 3);
	else if (fastcmp(field,"bob"))
		plr->bob = luaL_checkfixed(L, 3);*/
	else if (fastcmp(field,"aiming")) {
		plr->aiming = luaL_checkangle(L, 3);
		if (plr == &players[consoleplayer])
			localaiming[0] = plr->aiming;
		else if (plr == &players[displayplayers[1]])
			localaiming[1] = plr->aiming;
		else if (plr == &players[displayplayers[2]])
			localaiming[2] = plr->aiming;
		else if (plr == &players[displayplayers[3]])
			localaiming[3] = plr->aiming;
	}
	else if (fastcmp(field,"health"))
		plr->health = (INT32)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"pity"))
		plr->pity = (SINT8)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"currentweapon"))
		plr->currentweapon = (INT32)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"ringweapons"))
		plr->ringweapons = (INT32)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"powers"))
		return NOSET;
	else if (fastcmp(field,"pflags"))
		plr->pflags = luaL_checkinteger(L, 3);
	else if (fastcmp(field,"panim"))
		plr->panim = luaL_checkinteger(L, 3);
	else if (fastcmp(field,"flashcount"))
		plr->flashcount = (UINT16)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"flashpal"))
		plr->flashpal = (UINT16)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"skincolor"))
	{
		UINT8 newcolor = (UINT8)luaL_checkinteger(L,3);
		if (newcolor >= MAXSKINCOLORS)
			return luaL_error(L, "player.skincolor %d out of range (0 - %d).", newcolor, MAXSKINCOLORS-1);
		plr->skincolor = newcolor;
	}
	else if (fastcmp(field,"score"))
		plr->score = (UINT32)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"dashspeed"))
		plr->dashspeed = luaL_checkfixed(L, 3);
	else if (fastcmp(field,"dashtime"))
		plr->dashtime = (INT32)luaL_checkinteger(L, 3);
	// SRB2kart
	else if (fastcmp(field,"kartstuff"))
		return NOSET;
	else if (fastcmp(field,"frameangle"))
		plr->frameangle = luaL_checkangle(L, 3);
	else if (fastcmp(field,"kartspeed"))
		plr->kartspeed = (UINT8)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"kartweight"))
		plr->kartweight = (UINT8)luaL_checkinteger(L, 3);
	//
	else if (fastcmp(field,"charflags"))
		plr->charflags = (UINT32)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"lives"))
		plr->lives = (SINT8)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"continues"))
		plr->continues = (SINT8)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"xtralife"))
		plr->xtralife = (SINT8)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"gotcontinue"))
		plr->gotcontinue = (UINT8)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"speed"))
		plr->speed = luaL_checkfixed(L, 3);
	else if (fastcmp(field,"jumping"))
		plr->jumping = luaL_checkboolean(L, 3);
	else if (fastcmp(field,"secondjump"))
		plr->secondjump = (UINT8)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"fly1"))
		plr->fly1 = (UINT8)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"scoreadd"))
		plr->scoreadd = (UINT8)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"glidetime"))
		plr->glidetime = (tic_t)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"climbing"))
		plr->climbing = (INT32)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"deadtimer"))
		plr->deadtimer = (INT32)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"exiting"))
		plr->exiting = (tic_t)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"homing"))
		plr->homing = (UINT8)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"skidtime"))
		plr->skidtime = (tic_t)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"cmomx"))
		plr->cmomx = luaL_checkfixed(L, 3);
	else if (fastcmp(field,"cmomy"))
		plr->cmomy = luaL_checkfixed(L, 3);
	else if (fastcmp(field,"rmomx"))
		plr->rmomx = luaL_checkfixed(L, 3);
	else if (fastcmp(field,"rmomy"))
		plr->rmomy = luaL_checkfixed(L, 3);
	else if (fastcmp(field,"numboxes"))
		plr->numboxes = (INT16)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"totalring"))
		plr->totalring = (INT16)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"realtime"))
		plr->realtime = (tic_t)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"laps"))
		plr->laps = (UINT8)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"ctfteam"))
		plr->ctfteam = (INT32)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"gotflag"))
		plr->gotflag = (UINT16)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"weapondelay"))
		plr->weapondelay = (INT32)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"tossdelay"))
		plr->tossdelay = (INT32)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"starpostx"))
		plr->starpostx = (INT16)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"starposty"))
		plr->starposty = (INT16)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"starpostz"))
		plr->starpostz = (INT16)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"starpostnum"))
		plr->starpostnum = (INT32)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"starposttime"))
		plr->starposttime = (tic_t)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"starpostangle"))
		plr->starpostangle = luaL_checkangle(L, 3);
	else if (fastcmp(field,"angle_pos"))
		plr->angle_pos = luaL_checkangle(L, 3);
	else if (fastcmp(field,"old_angle_pos"))
		plr->old_angle_pos = luaL_checkangle(L, 3);
	else if (fastcmp(field,"axis1"))
		P_SetTarget(&plr->axis1, *((mobj_t **)luaL_checkudata(L, 3, META_MOBJ)));
	else if (fastcmp(field,"axis2"))
		P_SetTarget(&plr->axis2, *((mobj_t **)luaL_checkudata(L, 3, META_MOBJ)));
	else if (fastcmp(field,"bumpertime"))
		plr->bumpertime = (tic_t)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"flyangle"))
		plr->flyangle = (INT32)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"drilltimer"))
		plr->drilltimer = (tic_t)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"linkcount"))
		plr->linkcount = (INT32)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"linktimer"))
		plr->linktimer = (tic_t)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"anotherflyangle"))
		plr->anotherflyangle = (INT32)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"nightstime"))
		plr->nightstime = (tic_t)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"drillmeter"))
		plr->drillmeter = (INT32)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"drilldelay"))
		plr->drilldelay = (UINT8)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"bonustime"))
		plr->bonustime = luaL_checkboolean(L, 3);
	else if (fastcmp(field,"capsule"))
	{
		mobj_t *mo = NULL;
		if (!lua_isnil(L, 3))
			mo = *((mobj_t **)luaL_checkudata(L, 3, META_MOBJ));
		P_SetTarget(&plr->capsule, mo);
	}
	else if (fastcmp(field,"mare"))
		plr->mare = (UINT8)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"marebegunat"))
		plr->marebegunat = (tic_t)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"startedtime"))
		plr->startedtime = (tic_t)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"finishedtime"))
		plr->finishedtime = (tic_t)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"finishedrings"))
		plr->finishedrings = (INT16)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"marescore"))
		plr->marescore = (UINT32)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"lastmarescore"))
		plr->lastmarescore = (UINT32)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"lastmare"))
		plr->lastmare = (UINT8)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"maxlink"))
		plr->maxlink = (INT32)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"texttimer"))
		plr->texttimer = (UINT8)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"textvar"))
		plr->textvar = (UINT8)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"lastsidehit"))
		plr->lastsidehit = (INT16)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"lastlinehit"))
		plr->lastlinehit = (INT16)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"losstime"))
		plr->losstime = (tic_t)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"timeshit"))
		plr->timeshit = (UINT8)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"onconveyor"))
		plr->onconveyor = (INT32)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"awayviewmobj"))
	{
		mobj_t *mo = NULL;
		if (!lua_isnil(L, 3))
			mo = *((mobj_t **)luaL_checkudata(L, 3, META_MOBJ));
		P_SetTarget(&plr->awayviewmobj, mo);
	}
	else if (fastcmp(field,"awayviewtics"))
	{
		plr->awayviewtics = (INT32)luaL_checkinteger(L, 3);
		if (plr->awayviewtics && !plr->awayviewmobj) // awayviewtics must ALWAYS have an awayviewmobj set!!
			P_SetTarget(&plr->awayviewmobj, plr->mo); // but since the script might set awayviewmobj immediately AFTER setting awayviewtics, use player mobj as filler for now.
	}
	else if (fastcmp(field,"awayviewaiming"))
		plr->awayviewaiming = luaL_checkangle(L, 3);
	else if (fastcmp(field,"spectator"))
		plr->spectator = lua_toboolean(L, 3);
	else if (fastcmp(field,"bot"))
		return NOSET;
	else if (fastcmp(field,"jointime"))
		plr->jointime = (tic_t)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"splitscreenindex"))
		return NOSET;
#ifdef HWRENDER
	else if (fastcmp(field,"fovadd"))
		plr->fovadd = luaL_checkfixed(L, 3);
#endif
	else {
		lua_getfield(L, LUA_REGISTRYINDEX, LREG_EXTVARS);
		I_Assert(lua_istable(L, -1));
		lua_pushlightuserdata(L, plr);
		lua_rawget(L, -2);
		if (lua_isnil(L, -1)) {
			// This index doesn't have a table for extra values yet, let's make one.
			lua_pop(L, 1);
			CONS_Debug(DBG_LUA, M_GetText("'%s' has no field named '%s'; adding it as Lua data.\n"), "player_t", field);
			lua_newtable(L);
			lua_pushlightuserdata(L, plr);
			lua_pushvalue(L, -2); // ext value table
			lua_rawset(L, -4); // LREG_EXTVARS table
		}
		lua_pushvalue(L, 3); // value to store
		lua_setfield(L, -2, field);
		lua_pop(L, 2);
	}

	return 0;
}

#undef NOSET

static int player_num(lua_State *L)
{
	player_t *plr = *((player_t **)luaL_checkudata(L, 1, META_PLAYER));
	if (!plr)
		return luaL_error(L, "accessed player_t doesn't exist anymore.");
	lua_pushinteger(L, plr-players);
	return 1;
}

// powers, p -> powers[p]
static int power_get(lua_State *L)
{
	UINT16 *powers = *((UINT16 **)luaL_checkudata(L, 1, META_POWERS));
	powertype_t p = luaL_checkinteger(L, 2);
	if (p >= NUMPOWERS)
		return luaL_error(L, LUA_QL("powertype_t") " cannot be %u", p);
	lua_pushinteger(L, powers[p]);
	return 1;
}

// powers, p, value -> powers[p] = value
static int power_set(lua_State *L)
{
	UINT16 *powers = *((UINT16 **)luaL_checkudata(L, 1, META_POWERS));
	powertype_t p = luaL_checkinteger(L, 2);
	UINT16 i = (UINT16)luaL_checkinteger(L, 3);
	if (p >= NUMPOWERS)
		return luaL_error(L, LUA_QL("powertype_t") " cannot be %u", p);
	if (hud_running)
		return luaL_error(L, "Do not alter player_t in HUD rendering code!");
	if (hook_cmd_running)
		return luaL_error(L, "Do not alter player_t in BuildCMD code!");
	powers[p] = i;
	return 0;
}

// #powers -> NUMPOWERS
static int power_len(lua_State *L)
{
	lua_pushinteger(L, NUMPOWERS);
	return 1;
}

// kartstuff, ks -> kartstuff[ks]
static int kartstuff_get(lua_State *L)
{
	INT32 *kartstuff = *((INT32 **)luaL_checkudata(L, 1, META_KARTSTUFF));
	kartstufftype_t ks = luaL_checkinteger(L, 2);
	if (ks >= NUMKARTSTUFF)
		return luaL_error(L, LUA_QL("kartstufftype_t") " cannot be %u", ks);
	lua_pushinteger(L, kartstuff[ks]);
	return 1;
}

// kartstuff, ks, value -> kartstuff[ks] = value
static int kartstuff_set(lua_State *L)
{
	INT32 *kartstuff = *((INT32 **)luaL_checkudata(L, 1, META_KARTSTUFF));
	kartstufftype_t ks = luaL_checkinteger(L, 2);
	INT32 i = (INT32)luaL_checkinteger(L, 3);
	if (ks >= NUMKARTSTUFF)
		return luaL_error(L, LUA_QL("kartstufftype_t") " cannot be %u", ks);
	if (hud_running)
		return luaL_error(L, "Do not alter player_t in HUD rendering code!");
	if (hook_cmd_running)
		return luaL_error(L, "Do not alter player_t in BuildCMD code!");
	kartstuff[ks] = i;
	return 0;
}

// #kartstuff -> NUMKARTSTUFF
static int kartstuff_len(lua_State *L)
{
	lua_pushinteger(L, NUMKARTSTUFF);
	return 1;
}

#define NOFIELD luaL_error(L, LUA_QL("ticcmd_t") " has no field named " LUA_QS, field)

static int ticcmd_get(lua_State *L)
{
	ticcmd_t *cmd = *((ticcmd_t **)luaL_checkudata(L, 1, META_TICCMD));
	const char *field = luaL_checkstring(L, 2);
	if (!cmd)
		return LUA_ErrInvalid(L, "player_t");

	if (fastcmp(field,"forwardmove"))
		lua_pushinteger(L, cmd->forwardmove);
	else if (fastcmp(field,"sidemove"))
		lua_pushinteger(L, cmd->sidemove);
	else if (fastcmp(field,"angleturn"))
		lua_pushinteger(L, cmd->angleturn);
	else if (fastcmp(field,"aiming"))
		lua_pushinteger(L, cmd->aiming);
	else if (fastcmp(field,"buttons"))
		lua_pushinteger(L, cmd->buttons);
	else if (fastcmp(field,"driftturn"))
		lua_pushinteger(L, cmd->driftturn);
	else if (fastcmp(field,"latency"))
		lua_pushinteger(L, cmd->latency);
	else
		return NOFIELD;

	return 1;
}

static int ticcmd_set(lua_State *L)
{
	ticcmd_t *cmd = *((ticcmd_t **)luaL_checkudata(L, 1, META_TICCMD));
	const char *field = luaL_checkstring(L, 2);
	if (!cmd)
		return LUA_ErrInvalid(L, "ticcmd_t");

	if (hud_running)
		return luaL_error(L, "Do not alter player_t in HUD rendering code!");

	if (fastcmp(field,"forwardmove"))
		cmd->forwardmove = (SINT8)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"sidemove"))
		cmd->sidemove = (SINT8)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"angleturn"))
		cmd->angleturn = (INT16)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"aiming"))
		cmd->aiming = (INT16)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"buttons"))
		cmd->buttons = (UINT16)luaL_checkinteger(L, 3);
	else if (fastcmp(field,"driftturn"))
		cmd->driftturn = (INT16)luaL_checkinteger(L, 3);
	else
		return NOFIELD;

	return 0;
}

#undef NOFIELD

int LUA_PlayerLib(lua_State *L)
{
	luaL_newmetatable(L, META_PLAYER);
		lua_pushcfunction(L, player_get);
		lua_setfield(L, -2, "__index");

		lua_pushcfunction(L, player_set);
		lua_setfield(L, -2, "__newindex");

		lua_pushcfunction(L, player_num);
		lua_setfield(L, -2, "__len");
	lua_pop(L,1);

	luaL_newmetatable(L, META_POWERS);
		lua_pushcfunction(L, power_get);
		lua_setfield(L, -2, "__index");

		lua_pushcfunction(L, power_set);
		lua_setfield(L, -2, "__newindex");

		lua_pushcfunction(L, power_len);
		lua_setfield(L, -2, "__len");
	lua_pop(L,1);

	luaL_newmetatable(L, META_KARTSTUFF);
		lua_pushcfunction(L, kartstuff_get);
		lua_setfield(L, -2, "__index");

		lua_pushcfunction(L, kartstuff_set);
		lua_setfield(L, -2, "__newindex");

		lua_pushcfunction(L, kartstuff_len);
		lua_setfield(L, -2, "__len");
	lua_pop(L,1);

	luaL_newmetatable(L, META_TICCMD);
		lua_pushcfunction(L, ticcmd_get);
		lua_setfield(L, -2, "__index");

		lua_pushcfunction(L, ticcmd_set);
		lua_setfield(L, -2, "__newindex");
	lua_pop(L,1);

	lua_newuserdata(L, 0);
		lua_createtable(L, 0, 2);
			lua_pushcfunction(L, lib_getPlayer);
			lua_setfield(L, -2, "__index");

			lua_pushcfunction(L, lib_lenPlayer);
			lua_setfield(L, -2, "__len");
		lua_setmetatable(L, -2);
	lua_setglobal(L, "players");

	// push displayplayers in the same fashion
	lua_newuserdata(L, 0);
		lua_createtable(L, 0, 2);
			lua_pushcfunction(L, lib_getDisplayplayers);
			lua_setfield(L, -2, "__index");

			lua_pushcfunction(L, lib_lenDisplayplayers);
			lua_setfield(L, -2, "__len");
		lua_setmetatable(L, -2);
	lua_setglobal(L, "displayplayers");

	return 0;
}

#endif
