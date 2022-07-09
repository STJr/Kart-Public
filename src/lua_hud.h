// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2014-2016 by John "JTE" Muniz.
// Copyright (C) 2014-2018 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  lua_hud.h
/// \brief HUD enable/disable flags for Lua scripting

#ifndef __LUA_HUD_H__
#define __LUA_HUD_H__

#include "lua_hudlib_drawlist.h"

enum hud {
	hud_stagetitle = 0,
	hud_textspectator,

	hud_time,
	hud_gametypeinfo,
	hud_minimap,
	hud_item,
	hud_position,
	hud_check,			// "CHECK" f-zero indicator
	hud_minirankings,	// Rankings to the left
	hud_battlebumpers,	// mini rankings battle bumpers.
	hud_battlefullscreen,	// battle huge text (WAIT, WIN, LOSE ...) + karma comeback time
	hud_battlecomebacktimer,	// comeback timer in battlefullscreen. separated for ease of use.
	hud_wanted,
	hud_speedometer,
	hud_freeplay,
	hud_rankings,		// Tab rankings

	hud_MAX
};

extern boolean hud_running;

boolean LUA_HudEnabled(enum hud option);

void LUAh_GameHUD(player_t *stplyr, huddrawlist_h list);
void LUAh_ScoresHUD(huddrawlist_h list);

#endif // __LUA_HUD_H__
