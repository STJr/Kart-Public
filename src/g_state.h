// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2018 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  g_state.h
/// \brief SRB2 game states

#ifndef __G_STATE__
#define __G_STATE__

#include "doomtype.h"

// the current state of the game
typedef enum
{
	GS_NULL = 0,        // At beginning.

	// Fadable gamestates
	GS_LEVEL,           // Playing, in a level.
	GS_INTERMISSION,    // Gazing at the intermission screen.
	GS_VOTING,          // SRB2Kart: MP voting screen
	GS_CONTINUING,      // continue screen

	GS_TITLESCREEN,     // title screen
	GS_TIMEATTACK,      // time attack menu
	GS_CREDITS,         // credit sequence
	GS_EVALUATION,      // Evaluation at the end of a game.
	GS_GAMEEND,         // game end sequence

	// Hardcoded fades or other fading methods
	GS_INTRO,           // introduction
	GS_CUTSCENE,        // custom cutscene

	// Not fadable
	GS_DEDICATEDSERVER, // new state for dedicated server
	GS_WAITINGPLAYERS   // waiting for players in a net game
} gamestate_t;

typedef enum
{
	ga_nothing,
	ga_completed,
	ga_worlddone,
	ga_startcont,
	ga_continued,
	ga_startvote,
} gameaction_t;

extern gamestate_t gamestate;
extern UINT8 ultimatemode; // was sk_insane
extern gameaction_t gameaction;

extern boolean botingame;
extern UINT8 botskin, botcolor;

#endif //__G_STATE__
