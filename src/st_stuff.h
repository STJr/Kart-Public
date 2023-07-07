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
/// \file  st_stuff.h
/// \brief Status bar header

#ifndef __STSTUFF_H__
#define __STSTUFF_H__

#include "doomtype.h"
#include "d_event.h"
#include "d_player.h"
#include "r_defs.h"

//
// STATUS BAR
//

// Called by main loop.
void ST_Ticker(void);

// Called when naming a replay.
void ST_DrawDemoTitleEntry(void);

#ifdef HAVE_DISCORDRPC
// Called when you have Discord asks
void ST_AskToJoinEnvelope(void);
#endif

// Called by main loop.
void ST_Drawer(void);

// Called when the console player is spawned on each level.
void ST_Start(void);

// Called by startup code.
void ST_Init(void);

// Called by G_Responder() when pressing F12 while viewing a demo.
void ST_changeDemoView(void);

void ST_UnloadGraphics(void);
void ST_LoadGraphics(void);

// face load graphics, called when skin changes
void ST_LoadFaceGraphics(char *rankstr, char *wantstr, char *mmapstr, INT32 playernum);
void ST_ReloadSkinFaceGraphics(void);
#ifdef DELFILE
void ST_UnLoadFaceGraphics(INT32 skinnum);
#endif

void ST_doPaletteStuff(void);

// return if player a is in the same team as player b
boolean ST_SameTeam(player_t *a, player_t *b);

//--------------------
// status bar overlay
//--------------------

extern boolean st_overlay; // sb overlay on or off when fullscreen
extern INT32 st_palette; // 0 is default, any others are special palettes.

extern lumpnum_t st_borderpatchnum;
// patches, also used in intermission
extern patch_t *tallnum[10];
extern patch_t *sboscore;
extern patch_t *sbotime;
extern patch_t *sbocolon;
extern patch_t *sboperiod;
extern patch_t *facerankprefix[MAXSKINS]; // ranking
extern patch_t *facewantprefix[MAXSKINS]; // wanted
extern patch_t *facemmapprefix[MAXSKINS]; // minimap
extern patch_t *livesback;
extern patch_t *ngradeletters[7];

/** HUD location information (don't move this comment)
  */
typedef struct
{
	INT32 x, y;
} hudinfo_t;

typedef enum
{
	HUD_LIVESNAME,
	HUD_LIVESPIC,
	HUD_LIVESNUM,
	HUD_LIVESX,

	HUD_RINGS,
	HUD_RINGSSPLIT,
	HUD_RINGSNUM,
	HUD_RINGSNUMSPLIT,

	HUD_SCORE,
	HUD_SCORENUM,

	HUD_TIME,
	HUD_TIMESPLIT,
	HUD_MINUTES,
	HUD_MINUTESSPLIT,
	HUD_TIMECOLON,
	HUD_TIMECOLONSPLIT,
	HUD_SECONDS,
	HUD_SECONDSSPLIT,
	HUD_TIMETICCOLON,
	HUD_TICS,

	HUD_SS_TOTALRINGS,
	HUD_SS_TOTALRINGS_SPLIT,

	HUD_GETRINGS,
	HUD_GETRINGSNUM,
	HUD_TIMELEFT,
	HUD_TIMELEFTNUM,
	HUD_TIMEUP,
	HUD_HUNTPICS,
	HUD_GRAVBOOTSICO,
	HUD_LAP,

	NUMHUDITEMS
} hudnum_t;

extern hudinfo_t hudinfo[NUMHUDITEMS];

extern UINT16 objectsdrawn;

#endif
