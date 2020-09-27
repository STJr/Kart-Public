// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2004-2018 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  y_inter.c
/// \brief Tally screens, or "Intermissions" as they were formally called in Doom

#include "doomdef.h"
#include "doomstat.h"
#include "d_main.h"
#include "f_finale.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_net.h"
#include "i_video.h"
#include "p_tick.h"
#include "r_defs.h"
#include "r_things.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "v_video.h"
#include "w_wad.h"
#include "y_inter.h"
#include "z_zone.h"
#include "m_menu.h"
#include "m_misc.h"
#include "i_system.h"
#include "p_setup.h"

#include "r_local.h"
#include "p_local.h"

#include "m_cond.h" // condition sets

#include "m_random.h" // M_RandomKey
#include "g_input.h" // PLAYER1INPUTDOWN
#include "k_kart.h" // colortranslations
#include "console.h" // cons_menuhighlight
#include "lua_hook.h" // IntermissionThinker hook

#ifdef HWRENDER
#include "hardware/hw_main.h"
#endif

#ifdef PC_DOS
#include <stdio.h> // for snprintf
int	snprintf(char *str, size_t n, const char *fmt, ...);
//int	vsnprintf(char *str, size_t n, const char *fmt, va_list ap);
#endif

typedef struct
{
	char patch[9];
	 INT32 points;
	UINT8 display;
} y_bonus_t;

typedef union
{
	/*struct
	{
		char passed1[21]; // KNUCKLES GOT    / CRAWLA HONCHO
		char passed2[16]; // THROUGH THE ACT / PASSED THE ACT
		INT32 passedx1;
		INT32 passedx2;

		y_bonus_t bonuses[4];
		patch_t *bonuspatches[4];

		SINT8 gotperfbonus; // Used for visitation flags.

		UINT32 score, total; // fake score, total
		UINT32 tics; // time

		patch_t *ttlnum; // act number being displayed
		patch_t *ptotal; // TOTAL
		UINT8 gotlife; // Number of extra lives obtained
	} coop;*/

	struct
	{
		UINT8 *color[MAXPLAYERS]; // Winner's color #
		INT32 *character[MAXPLAYERS]; // Winner's character #
		INT32 num[MAXPLAYERS]; // Winner's player #
		char *name[MAXPLAYERS]; // Winner's name
		INT32 numplayers; // Number of players being displayed
		char levelstring[64]; // holds levelnames up to 64 characters
		// SRB2kart
		UINT8 increase[MAXPLAYERS]; // how much did the score increase by?
		UINT8 jitter[MAXPLAYERS]; // wiggle
		UINT32 val[MAXPLAYERS]; // Gametype-specific value
		UINT8 pos[MAXPLAYERS]; // player positions. used for ties
		boolean rankingsmode; // rankings mode
		boolean encore; // encore mode
	} match;
} y_data;

static y_data data;

// graphics
static patch_t *bgpatch = NULL;     // INTERSCR
static patch_t *widebgpatch = NULL; // INTERSCW
static patch_t *bgtile = NULL;      // SPECTILE/SRB2BACK
static patch_t *interpic = NULL;    // custom picture defined in map header
static boolean usetile;
boolean usebuffer = false;
static boolean useinterpic;
static INT32 timer;

static INT32 intertic;
static INT32 endtic = -1;
static INT32 sorttic = -1;

intertype_t intertype = int_none;

static void Y_FollowIntermission(void);
static void Y_UnloadData(void);

// SRB2Kart: voting stuff
// Level images
typedef struct
{
	char str[62];
	UINT8 gtc;
	const char *gts;
	patch_t *pic;
	boolean encore;
} y_votelvlinfo;

// Clientside & splitscreen player info.
typedef struct
{
	SINT8 selection;
	UINT8 delay;
} y_voteplayer;

typedef struct
{
	y_voteplayer playerinfo[4];
	UINT8 ranim;
	UINT8 rtics;
	UINT8 roffset;
	UINT8 rsynctime;
	UINT8 rendoff;
	boolean loaded;
} y_voteclient;

static y_votelvlinfo levelinfo[5];
static y_voteclient voteclient;
static INT32 votetic;
static INT32 voteendtic = -1;
static patch_t *cursor = NULL;
static patch_t *cursor1 = NULL;
static patch_t *cursor2 = NULL;
static patch_t *cursor3 = NULL;
static patch_t *cursor4 = NULL;
static patch_t *randomlvl = NULL;
static patch_t *rubyicon = NULL;

static void Y_UnloadVoteData(void);

//
// SRB2Kart - Y_CalculateMatchData and ancillary functions
//
static void Y_CompareRace(INT32 i)
{
	UINT32 val = ((players[i].pflags & PF_TIMEOVER || players[i].realtime == UINT32_MAX)
		? (UINT32_MAX-1) : players[i].realtime);

	if (!(val < data.match.val[data.match.numplayers]))
		return;

	data.match.val[data.match.numplayers] = val;
	data.match.num[data.match.numplayers] = i;
}

static void Y_CompareBattle(INT32 i)
{
	UINT32 val = ((players[i].pflags & PF_TIMEOVER)
			? (UINT32_MAX-1) : players[i].marescore);

	if (!(data.match.val[data.match.numplayers] == UINT32_MAX
	|| (!(players[i].pflags & PF_TIMEOVER) && val > data.match.val[data.match.numplayers])))
		return;

	data.match.val[data.match.numplayers] = val;
	data.match.num[data.match.numplayers] = i;
}

static void Y_CompareRank(INT32 i)
{
	UINT8 increase = ((data.match.increase[i] == UINT8_MAX) ? 0 : data.match.increase[i]);
	if (!(data.match.val[data.match.numplayers] == UINT32_MAX || (players[i].score - increase) > data.match.val[data.match.numplayers]))
		return;

	data.match.val[data.match.numplayers] = (players[i].score - increase);
	data.match.num[data.match.numplayers] = i;
}

static void Y_CalculateMatchData(UINT8 rankingsmode, void (*comparison)(INT32))
{
	INT32 i, j;
	boolean completed[MAXPLAYERS];
	INT32 numplayersingame = 0;

	// Initialize variables
	if (rankingsmode > 1)
		;
	else if ((data.match.rankingsmode = (boolean)rankingsmode))
	{
		sprintf(data.match.levelstring, "* Total Rankings *");
		data.match.encore = false;
	}
	else
	{
		// set up the levelstring
		if (mapheaderinfo[prevmap]->levelflags & LF_NOZONE)
		{
			if (mapheaderinfo[prevmap]->actnum[0])
				snprintf(data.match.levelstring,
					sizeof data.match.levelstring,
					"* %s %s *",
					mapheaderinfo[prevmap]->lvlttl, mapheaderinfo[prevmap]->actnum);
			else
				snprintf(data.match.levelstring,
					sizeof data.match.levelstring,
					"* %s *",
					mapheaderinfo[prevmap]->lvlttl);
		}
		else
		{
			const char *zonttl = (mapheaderinfo[prevmap]->zonttl[0] ? mapheaderinfo[prevmap]->zonttl : "Zone");
			if (mapheaderinfo[prevmap]->actnum[0])
				snprintf(data.match.levelstring,
					sizeof data.match.levelstring,
					"* %s %s %s *",
					mapheaderinfo[prevmap]->lvlttl, zonttl, mapheaderinfo[prevmap]->actnum);
			else
				snprintf(data.match.levelstring,
					sizeof data.match.levelstring,
					"* %s %s *",
					mapheaderinfo[prevmap]->lvlttl, zonttl);
		}

		data.match.levelstring[sizeof data.match.levelstring - 1] = '\0';

		data.match.encore = encoremode;

		memset(data.match.jitter, 0, sizeof (data.match.jitter));
	}

	for (i = 0; i < MAXPLAYERS; i++)
	{
		data.match.val[i] = UINT32_MAX;

		if (!playeringame[i] || players[i].spectator)
		{
			data.match.increase[i] = UINT8_MAX;
			continue;
		}

		if (!rankingsmode)
			data.match.increase[i] = UINT8_MAX;

		numplayersingame++;
	}

	memset(data.match.color, 0, sizeof (data.match.color));
	memset(data.match.character, 0, sizeof (data.match.character));
	memset(completed, 0, sizeof (completed));
	data.match.numplayers = 0;

	for (j = 0; j < numplayersingame; j++)
	{
		INT32 nump = ((G_RaceGametype() && nospectategrief > 0) ? nospectategrief : numplayersingame);

		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!playeringame[i] || players[i].spectator || completed[i])
				continue;

			comparison(i);
		}

		i = data.match.num[data.match.numplayers];

		completed[i] = true;

		data.match.color[data.match.numplayers] = &players[i].skincolor;
		data.match.character[data.match.numplayers] = &players[i].skin;
		data.match.name[data.match.numplayers] = player_names[i];

		if (data.match.numplayers && (data.match.val[data.match.numplayers] == data.match.val[data.match.numplayers-1]))
			data.match.pos[data.match.numplayers] = data.match.pos[data.match.numplayers-1];
		else
			data.match.pos[data.match.numplayers] = data.match.numplayers+1;

		if (!rankingsmode && !(players[i].pflags & PF_TIMEOVER) && (data.match.pos[data.match.numplayers] < nump))
		{
			data.match.increase[i] = nump - data.match.pos[data.match.numplayers];
			players[i].score += data.match.increase[i];
		}

		if (demo.recording && !rankingsmode)
			G_WriteStanding(
				data.match.pos[data.match.numplayers],
				data.match.name[data.match.numplayers],
				*data.match.character[data.match.numplayers],
				*data.match.color[data.match.numplayers],
				data.match.val[data.match.numplayers]
			);

		data.match.numplayers++;
	}
}

//
// Y_IntermissionDrawer
//
// Called by D_Display. Nothing is modified here; all it does is draw. (SRB2Kart: er, about that...)
// Neat concept, huh?
//
void Y_IntermissionDrawer(void)
{
	INT32 i, whiteplayer = MAXPLAYERS, x = 4, hilicol = V_YELLOWMAP; // fallback

	if (intertype == int_none || rendermode == render_none)
		return;

	if (!usebuffer)
		V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);

	if (useinterpic)
		V_DrawScaledPatch(0, 0, 0, interpic);
	else if (!usetile)
	{
		if (rendermode == render_soft && usebuffer)
			VID_BlitLinearScreen(screens[1], screens[0], vid.width*vid.bpp, vid.height, vid.width*vid.bpp, vid.rowbytes);
#ifdef HWRENDER
		else if(rendermode != render_soft && usebuffer)
		{
			HWR_DrawIntermissionBG();
		}
#endif
		else
		{
			if (widebgpatch && rendermode == render_soft && vid.width / vid.dupx == 400)
				V_DrawScaledPatch(0, 0, V_SNAPTOLEFT, widebgpatch);
			else
				V_DrawScaledPatch(0, 0, 0, bgpatch);
		}
	}
	else
		V_DrawPatchFill(bgtile);

	if (usebuffer) // Fade everything out
		V_DrawFadeScreen(0xFF00, 22);

	if (!splitscreen)
		whiteplayer = demo.playback ? displayplayers[0] : consoleplayer;

	if (cons_menuhighlight.value)
		hilicol = cons_menuhighlight.value;
	else if (modeattacking)
		hilicol = V_ORANGEMAP;
	else
		hilicol = ((intertype == int_race) ? V_SKYMAP : V_REDMAP);

	if (sorttic != -1 && intertic > sorttic && !demo.playback)
	{
		INT32 count = (intertic - sorttic);

		if (count < 8)
			x -= ((count * vid.width) / (8 * vid.dupx));
		else if (count == 8)
			goto dotimer;
		else if (count < 16)
			x += (((16 - count) * vid.width) / (8 * vid.dupx));
	}

	// SRB2kart 290117 - compeltely replaced this block.
	/*if (intertype == int_timeattack)
	{
		// draw time
		ST_DrawPatchFromHud(HUD_TIME, sbotime);
		if (cv_timetic.value)
			ST_DrawNumFromHud(HUD_SECONDS, data.coop.tics);
		else
		{
			INT32 seconds, minutes, tictrn;

			seconds = G_TicsToSeconds(data.coop.tics);
			minutes = G_TicsToMinutes(data.coop.tics, true);
			tictrn  = G_TicsToCentiseconds(data.coop.tics);

			ST_DrawNumFromHud(HUD_MINUTES, minutes); // Minutes
			ST_DrawPatchFromHud(HUD_TIMECOLON, sbocolon); // Colon
			ST_DrawPadNumFromHud(HUD_SECONDS, seconds, 2); // Seconds

			// SRB2kart - pulled from old coop block, just in case we need it
			// we should show centiseconds on the intermission screen too, if the conditions are right.
			if (modeattacking || cv_timetic.value == 2)
			{
				ST_DrawPatchFromHud(HUD_TIMETICCOLON, sboperiod); // Period
				ST_DrawPadNumFromHud(HUD_TICS, tictrn, 2); // Tics
			}

			ST_DrawPatchFromHud(HUD_TIMETICCOLON, sboperiod); // Period
			ST_DrawPadNumFromHud(HUD_TICS, tictrn, 2); // Tics
		}

		// draw the "got through act" lines and act number
		V_DrawLevelTitle(data.coop.passedx1, 49, 0, data.coop.passed1);
		V_DrawLevelTitle(data.coop.passedx2, 49+V_LevelNameHeight(data.coop.passed2)+2, 0, data.coop.passed2);

		if (strlen(mapheaderinfo[prevmap]->actnum) > 0)
			V_DrawScaledPatch(244, 57, 0, data.coop.ttlnum);

		//if (gottimebonus && endtic != -1)
		//	V_DrawCenteredString(BASEVIDWIDTH/2, 172, V_YELLOWMAP, "TIME BONUS UNLOCKED!");
	}
	else*/ if (intertype == int_race || intertype == int_match)
	{
#define NUMFORNEWCOLUMN 8
		INT32 y = 41, gutter = ((data.match.numplayers > NUMFORNEWCOLUMN) ? 0 : (BASEVIDWIDTH/2));
		INT32 dupadjust = (vid.width/vid.dupx), duptweak = (dupadjust - BASEVIDWIDTH)/2;
		const char *timeheader;

		if (data.match.rankingsmode)
			timeheader = "RANK";
		else
			timeheader = (intertype == int_race ? "TIME" : "SCORE");

		// draw the level name
		V_DrawCenteredString(-4 + x + BASEVIDWIDTH/2, 12, 0, data.match.levelstring);
		V_DrawFill((x-3) - duptweak, 34, dupadjust-2, 1, 0);

		if (data.match.encore)
			V_DrawCenteredString(-4 + x + BASEVIDWIDTH/2, 12-8, hilicol, "ENCORE MODE");

		if (data.match.numplayers > NUMFORNEWCOLUMN)
		{
			V_DrawFill(x+156, 24, 1, 158, 0);
			V_DrawFill((x-3) - duptweak, 182, dupadjust-2, 1, 0);

			V_DrawCenteredString(x+6+(BASEVIDWIDTH/2), 24, hilicol, "#");
			V_DrawString(x+36+(BASEVIDWIDTH/2), 24, hilicol, "NAME");

			V_DrawRightAlignedString(x+152, 24, hilicol, timeheader);
		}

		V_DrawCenteredString(x+6, 24, hilicol, "#");
		V_DrawString(x+36, 24, hilicol, "NAME");

		V_DrawRightAlignedString(x+(BASEVIDWIDTH/2)+152, 24, hilicol, timeheader);

		for (i = 0; i < data.match.numplayers; i++)
		{
			boolean dojitter = data.match.jitter[data.match.num[i]];
			data.match.jitter[data.match.num[i]] = 0;

			if (data.match.num[i] != MAXPLAYERS && playeringame[data.match.num[i]] && !players[data.match.num[i]].spectator)
			{
				char strtime[MAXPLAYERNAME+1];

				if (dojitter)
					y--;

				V_DrawCenteredString(x+6, y, 0, va("%d", data.match.pos[i]));

				if (data.match.color[i])
				{
					UINT8 *colormap = R_GetTranslationColormap(*data.match.character[i], *data.match.color[i], GTC_CACHE);
					V_DrawMappedPatch(x+16, y-4, 0, facerankprefix[*data.match.character[i]], colormap);
				}

				if (data.match.num[i] == whiteplayer)
				{
					UINT8 cursorframe = (intertic / 4) % 8;
					V_DrawScaledPatch(x+16, y-4, 0, W_CachePatchName(va("K_CHILI%d", cursorframe+1), PU_CACHE));
				}

				STRBUFCPY(strtime, data.match.name[i]);

				if (data.match.numplayers > NUMFORNEWCOLUMN)
					V_DrawThinString(x+36, y-1, ((data.match.num[i] == whiteplayer) ? hilicol : 0)|V_ALLOWLOWERCASE|V_6WIDTHSPACE, strtime);
				else
					V_DrawString(x+36, y, ((data.match.num[i] == whiteplayer) ? hilicol : 0)|V_ALLOWLOWERCASE, strtime);

				if (data.match.rankingsmode)
				{
					if (data.match.increase[data.match.num[i]] != UINT8_MAX)
					{
						if (data.match.increase[data.match.num[i]] > 9)
							snprintf(strtime, sizeof strtime, "(+%02d)", data.match.increase[data.match.num[i]]);
						else
							snprintf(strtime, sizeof strtime, "(+  %d)", data.match.increase[data.match.num[i]]);

						if (data.match.numplayers > NUMFORNEWCOLUMN)
							V_DrawRightAlignedThinString(x+135+gutter, y-1, V_6WIDTHSPACE, strtime);
						else
							V_DrawRightAlignedString(x+120+gutter, y, 0, strtime);
					}

					snprintf(strtime, sizeof strtime, "%d", data.match.val[i]);

					if (data.match.numplayers > NUMFORNEWCOLUMN)
						V_DrawRightAlignedThinString(x+152+gutter, y-1, V_6WIDTHSPACE, strtime);
					else
						V_DrawRightAlignedString(x+152+gutter, y, 0, strtime);
				}
				else
				{
					if (data.match.val[i] == (UINT32_MAX-1))
						V_DrawRightAlignedThinString(x+152+gutter, y-1, (data.match.numplayers > NUMFORNEWCOLUMN ? V_6WIDTHSPACE : 0), "NO CONTEST.");
					else
					{
						if (intertype == int_race)
						{
							snprintf(strtime, sizeof strtime, "%i'%02i\"%02i", G_TicsToMinutes(data.match.val[i], true),
							G_TicsToSeconds(data.match.val[i]), G_TicsToCentiseconds(data.match.val[i]));
							strtime[sizeof strtime - 1] = '\0';

							if (data.match.numplayers > NUMFORNEWCOLUMN)
								V_DrawRightAlignedThinString(x+152+gutter, y-1, V_6WIDTHSPACE, strtime);
							else
								V_DrawRightAlignedString(x+152+gutter, y, 0, strtime);
						}
						else
						{
							if (data.match.numplayers > NUMFORNEWCOLUMN)
								V_DrawRightAlignedThinString(x+152+gutter, y-1, V_6WIDTHSPACE, va("%i", data.match.val[i]));
							else
								V_DrawRightAlignedString(x+152+gutter, y, 0, va("%i", data.match.val[i]));
						}
					}
				}

				if (dojitter)
					y++;
			}
			else
				data.match.num[i] = MAXPLAYERS; // this should be the only field setting in this function

			y += 18;

			if (i == NUMFORNEWCOLUMN-1)
			{
				y = 41;
				x += BASEVIDWIDTH/2;
			}
#undef NUMFORNEWCOLUMN
		}
	}

dotimer:
	if (timer)
	{
		char *string;
		INT32 tickdown = (timer+1)/TICRATE;

		if (multiplayer && demo.playback)
			string = va("Replay ends in %d", tickdown);
		else
			string = va("%s starts in %d", cv_advancemap.string, tickdown);

		V_DrawCenteredString(BASEVIDWIDTH/2, 188, hilicol,
			string);
	}

	if ((demo.recording || demo.savemode == DSM_SAVED) && !demo.playback)
		switch (demo.savemode)
		{
		case DSM_NOTSAVING:
			V_DrawRightAlignedThinString(BASEVIDWIDTH - 2, 2, V_SNAPTOTOP|V_SNAPTORIGHT|V_ALLOWLOWERCASE|hilicol, "Look Backward: Save replay");
			break;

		case DSM_SAVED:
			V_DrawRightAlignedThinString(BASEVIDWIDTH - 2, 2, V_SNAPTOTOP|V_SNAPTORIGHT|V_ALLOWLOWERCASE|hilicol, "Replay saved!");
			break;

		case DSM_TITLEENTRY:
			ST_DrawDemoTitleEntry();
			break;

		default: // Don't render any text here
			break;
		}

	// Make it obvious that scrambling is happening next round.
	if (cv_scrambleonchange.value && cv_teamscramble.value && (intertic/TICRATE % 2 == 0))
		V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT/2, hilicol, M_GetText("Teams will be scrambled next round!"));
}

//
// Y_Ticker
//
// Manages fake score tally for single player end of act, and decides when intermission is over.
//
void Y_Ticker(void)
{
	if (intertype == int_none)
		return;

	if (demo.recording)
	{
		if (demo.savemode == DSM_NOTSAVING && InputDown(gc_lookback, 1))
			demo.savemode = DSM_TITLEENTRY;

		if (demo.savemode == DSM_WILLSAVE || demo.savemode == DSM_WILLAUTOSAVE)
			G_SaveDemo();
	}

	// Check for pause or menu up in single player
	if (paused || P_AutoPause())
		return;

#ifdef HAVE_BLUA
	LUAh_IntermissionThinker();
#endif

	intertic++;

	// Team scramble code for team match and CTF.
	// Don't do this if we're going to automatically scramble teams next round.
	/*if (G_GametypeHasTeams() && cv_teamscramble.value && !cv_scrambleonchange.value && server)
	{
		// If we run out of time in intermission, the beauty is that
		// the P_Ticker() team scramble code will pick it up.
		if ((intertic % (TICRATE/7)) == 0)
			P_DoTeamscrambling();
	}*/

	// multiplayer uses timer (based on cv_inttime)
	if (timer)
	{
		if (!--timer)
		{
			Y_EndIntermission();
			Y_FollowIntermission();
			return;
		}
	}
	// single player is hardcoded to go away after awhile
	else if (intertic == endtic)
	{
		Y_EndIntermission();
		Y_FollowIntermission();
		return;
	}

	if (intertic < TICRATE || intertic & 1 || endtic != -1)
		return;

	if (intertype == int_race || intertype == int_match)
	{
		if (netgame || multiplayer)
		{
			if (sorttic == -1)
				sorttic = intertic + max((cv_inttime.value/2)-2, 2)*TICRATE; // 8 second pause after match results
			else if (!(multiplayer && demo.playback)) // Don't advance to rankings in replays
			{
				if (!data.match.rankingsmode && (intertic >= sorttic + 8))
					Y_CalculateMatchData(1, Y_CompareRank);

				if (data.match.rankingsmode && intertic > sorttic+16+(2*TICRATE))
				{
					INT32 q=0,r=0;
					boolean kaching = true;

					for (q = 0; q < data.match.numplayers; q++)
					{
						if (data.match.num[q] == MAXPLAYERS
						|| !data.match.increase[data.match.num[q]]
						|| data.match.increase[data.match.num[q]] == UINT8_MAX)
							continue;

						r++;
						data.match.jitter[data.match.num[q]] = 1;
						if (--data.match.increase[data.match.num[q]])
							kaching = false;
					}

					if (r)
					{
						S_StartSound(NULL, (kaching ? sfx_chchng : sfx_ptally));
						Y_CalculateMatchData(2, Y_CompareRank);
					}
					else
						endtic = intertic + 3*TICRATE; // 3 second pause after end of tally
				}
			}
		}
		else
			endtic = intertic + 8*TICRATE; // 8 second pause after end of tally
	}
}

//
// Y_UpdateRecordReplays
//
// Update replay files/data, etc. for Record Attack
// See G_SetNightsRecords for NiGHTS Attack.
//
static void Y_UpdateRecordReplays(void)
{
	const size_t glen = strlen(srb2home)+1+strlen("replay")+1+strlen(timeattackfolder)+1+strlen("MAPXX")+1;
	char *gpath;
	char lastdemo[256], bestdemo[256];
	UINT8 earnedEmblems;

	// Record new best time
	if (!mainrecords[gamemap-1])
		G_AllocMainRecordData(gamemap-1);

	if ((mainrecords[gamemap-1]->time == 0) || (players[consoleplayer].realtime < mainrecords[gamemap-1]->time))
		mainrecords[gamemap-1]->time = players[consoleplayer].realtime;

	if ((mainrecords[gamemap-1]->lap == 0) || (bestlap < mainrecords[gamemap-1]->lap))
		mainrecords[gamemap-1]->lap = bestlap;

	// Save demo!
	bestdemo[255] = '\0';
	lastdemo[255] = '\0';
	G_SetDemoTime(players[consoleplayer].realtime, bestlap);
	G_CheckDemoStatus();

	I_mkdir(va("%s"PATHSEP"replay", srb2home), 0755);
	I_mkdir(va("%s"PATHSEP"replay"PATHSEP"%s", srb2home, timeattackfolder), 0755);

	if ((gpath = malloc(glen)) == NULL)
		I_Error("Out of memory for replay filepath\n");

	sprintf(gpath,"%s"PATHSEP"replay"PATHSEP"%s"PATHSEP"%s", srb2home, timeattackfolder, G_BuildMapName(gamemap));
	snprintf(lastdemo, 255, "%s-%s-last.lmp", gpath, cv_chooseskin.string);

	if (FIL_FileExists(lastdemo))
	{
		UINT8 *buf;
		size_t len = FIL_ReadFile(lastdemo, &buf);

		snprintf(bestdemo, 255, "%s-%s-time-best.lmp", gpath, cv_chooseskin.string);
		if (!FIL_FileExists(bestdemo) || G_CmpDemoTime(bestdemo, lastdemo) & 1)
		{ // Better time, save this demo.
			if (FIL_FileExists(bestdemo))
				remove(bestdemo);
			FIL_WriteFile(bestdemo, buf, len);
			CONS_Printf("\x83%s\x80 %s '%s'\n", M_GetText("NEW RECORD TIME!"), M_GetText("Saved replay as"), bestdemo);
		}

		snprintf(bestdemo, 255, "%s-%s-lap-best.lmp", gpath, cv_chooseskin.string);
		if (!FIL_FileExists(bestdemo) || G_CmpDemoTime(bestdemo, lastdemo) & (1<<1))
		{ // Better lap time, save this demo.
			if (FIL_FileExists(bestdemo))
				remove(bestdemo);
			FIL_WriteFile(bestdemo, buf, len);
			CONS_Printf("\x83%s\x80 %s '%s'\n", M_GetText("NEW RECORD LAP!"), M_GetText("Saved replay as"), bestdemo);
		}

		//CONS_Printf("%s '%s'\n", M_GetText("Saved replay as"), lastdemo);

		Z_Free(buf);
	}
	free(gpath);

	// Check emblems when level data is updated
	if ((earnedEmblems = M_CheckLevelEmblems()))
		CONS_Printf(M_GetText("\x82" "Earned %hu medal%s for Record Attack records.\n"), (UINT16)earnedEmblems, earnedEmblems > 1 ? "s" : "");

	if (M_UpdateUnlockablesAndExtraEmblems(false))
		S_StartSound(NULL, sfx_ncitem);

	// SRB2Kart - save here so you NEVER lose your earned times/medals.
	G_SaveGameData(false);

	// Update timeattack menu's replay availability.
	CV_AddValue(&cv_nextmap, 1);
	CV_AddValue(&cv_nextmap, -1);
}

//
// Y_StartIntermission
//
// Called by G_DoCompleted. Sets up data for intermission drawer/ticker.
//
void Y_StartIntermission(void)
{
	intertic = -1;

#ifdef PARANOIA
	if (endtic != -1)
		I_Error("endtic is dirty");
#endif

	if (!multiplayer)
	{
		timer = 0;

		/* // srb2kart: time attack tally is UGLY rn
		if (modeattacking)
			intertype = int_timeattack;
		else
		*/
			intertype = int_race;
	}
	else
	{
		if (cv_inttime.value == 0 && gametype == GT_COOP)
			timer = 0;
		else if (demo.playback) // Override inttime (which is pulled from the replay anyway
			timer = 10*TICRATE;
		else
		{
			timer = cv_inttime.value*TICRATE;

			if (!timer)
				timer = 1;
		}

		if (gametype == GT_MATCH)
			intertype = int_match;
		else //if (gametype == GT_RACE)
			intertype = int_race;
	}

	// We couldn't display the intermission even if we wanted to.
	// But we still need to give the players their score bonuses, dummy.
	//if (dedicated) return;

	// This should always exist, but just in case...
	if(!mapheaderinfo[prevmap])
		P_AllocMapHeader(prevmap);

	switch (intertype)
	{
		case int_match:
		{
			// Calculate who won
			Y_CalculateMatchData(0, Y_CompareBattle);
			if (cv_inttime.value > 0)
				S_ChangeMusicInternal("racent", true); // loop it
			break;
		}
		case int_race: // (time-only race)
		{
			if (!majormods && !multiplayer && !demo.playback) // remove this once we have a proper time attack screen
			{
				// Update visitation flags
				mapvisited[gamemap-1] |= MV_BEATEN;
				if (ALL7EMERALDS(emeralds))
					mapvisited[gamemap-1] |= MV_ALLEMERALDS;
				/*if (ultimatemode)
					mapvisited[gamemap-1] |= MV_ULTIMATE;
				if (data.coop.gotperfbonus)
					mapvisited[gamemap-1] |= MV_PERFECT;*/

				if (modeattacking == ATTACKING_RECORD)
					Y_UpdateRecordReplays();
			}

			// Calculate who won
			Y_CalculateMatchData(0, Y_CompareRace);
			break;
		}

		case int_none:
		default:
			break;
	}

	//if (intertype == int_race || intertype == int_match)
	{
		//bgtile = W_CachePatchName("SRB2BACK", PU_STATIC);
		usetile = useinterpic = false;
		usebuffer = true;
	}
}

// ======

//
// Y_EndIntermission
//
void Y_EndIntermission(void)
{
	Y_UnloadData();

	endtic = -1;
	sorttic = -1;
	intertype = int_none;
	usebuffer = false;
}

//
// Y_FollowIntermission
//
static void Y_FollowIntermission(void)
{
	// This handles whether to play a post-level cutscene, end the game,
	// or simply go to the next level.
	// No need to duplicate the code here!
	G_AfterIntermission();
}

#define UNLOAD(x) Z_ChangeTag(x, PU_CACHE); x = NULL

//
// Y_UnloadData
//
static void Y_UnloadData(void)
{
	// In hardware mode, don't Z_ChangeTag a pointer returned by W_CachePatchName().
	// It doesn't work and is unnecessary.
	if (rendermode != render_soft)
		return;

	// unload the background patches
	UNLOAD(bgpatch);
	UNLOAD(widebgpatch);
	UNLOAD(bgtile);
	UNLOAD(interpic);

	/*switch (intertype)
	{
		case int_coop:
			// unload the coop and single player patches
			UNLOAD(data.coop.ttlnum);
			UNLOAD(data.coop.bonuspatches[3]);
			UNLOAD(data.coop.bonuspatches[2]);
			UNLOAD(data.coop.bonuspatches[1]);
			UNLOAD(data.coop.bonuspatches[0]);
			UNLOAD(data.coop.ptotal);
			break;
		case int_spec:
			// unload the special stage patches
			//UNLOAD(data.spec.cemerald);
			//UNLOAD(data.spec.nowsuper);
			UNLOAD(data.spec.bonuspatch);
			UNLOAD(data.spec.pscore);
			UNLOAD(data.spec.pcontinues);
			break;
		case int_match:
		case int_race:
		default:
			//without this default,
			//int_none, int_tag, int_chaos, and int_classicrace
			//are not handled
			break;
	}*/
}

// SRB2Kart: Voting!

//
// Y_VoteDrawer
//
// Draws the voting screen!
//
void Y_VoteDrawer(void)
{
	INT32 i, x, y = 0, height = 0;
	UINT8 selected[4];
	fixed_t rubyheight = 0;

	if (rendermode == render_none)
		return;

	if (votetic >= voteendtic && voteendtic != -1)
		return;

	if (!voteclient.loaded)
		return;

	{
		angle_t rubyfloattime = (ANGLE_MAX/NEWTICRATE)*(votetic % NEWTICRATE);
		rubyheight = FINESINE(rubyfloattime>>ANGLETOFINESHIFT);
	}

	V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);

	if (widebgpatch && rendermode == render_soft && vid.width / vid.dupx > 320)
		V_DrawScaledPatch(((vid.width/2) / vid.dupx) - (SHORT(widebgpatch->width)/2),
							(vid.height / vid.dupy) - SHORT(widebgpatch->height),
							V_SNAPTOTOP|V_SNAPTOLEFT, widebgpatch);
	else
		V_DrawScaledPatch(((vid.width/2) / vid.dupx) - (SHORT(bgpatch->width)/2), // Keep the width/height adjustments, for screens that are less wide than 320(?)
							(vid.height / vid.dupy) - SHORT(bgpatch->height),
							V_SNAPTOTOP|V_SNAPTOLEFT, bgpatch);

	for (i = 0; i < 4; i++) // First, we need to figure out the height of this thing...
	{
		UINT8 j;
		selected[i] = 0; // Initialize

		for (j = 0; j <= splitscreen; j++)
		{
			if (voteclient.playerinfo[j].selection == i)
				selected[i]++;
		}

		if (selected[i])
			height += 50;
		else
			height += 25;

		if (i < 3)
			height += 5-splitscreen;
	}

	y = (200-height)/2;
	for (i = 0; i < 4; i++)
	{
		const char *str;
		patch_t *pic;
		UINT8 j, color;

		if (i == 3)
		{
			str = "RANDOM";
			pic = randomlvl;
		}
		else
		{
			str = levelinfo[i].str;
			pic = levelinfo[i].pic;
		}

		if (selected[i])
		{
			UINT8 sizeadd = selected[i];

			for (j = 0; j <= splitscreen; j++) // another loop for drawing the selection backgrounds in the right order, grumble grumble..
			{
				INT32 handy = y;
				UINT8 p;
				UINT8 *colormap;
				patch_t *thiscurs;

				if (voteclient.playerinfo[j].selection != i)
					continue;

				if (!splitscreen)
				{
					thiscurs = cursor;
					p = consoleplayer;
					color = levelinfo[i].gtc;
					colormap = NULL;
				}
				else
				{
					switch (j)
					{
						case 1:
							thiscurs = cursor2;
							p = displayplayers[1];
							break;
						case 2:
							thiscurs = cursor3;
							p = displayplayers[2];
							break;
						case 3:
							thiscurs = cursor4;
							p = displayplayers[3];
							break;
						default:
							thiscurs = cursor1;
							p = displayplayers[0];
							break;
					}

					color = colortranslations[players[p].skincolor][7];
					colormap = R_GetTranslationColormap(TC_DEFAULT, players[p].skincolor, GTC_CACHE);
				}

				if (votes[p] != -1 || players[p].spectator)
					continue;

				handy += 6*(3-splitscreen) + (13*j);
				V_DrawMappedPatch(BASEVIDWIDTH-124, handy, V_SNAPTORIGHT, thiscurs, colormap);

				if (votetic % 10 < 4)
					V_DrawFill(BASEVIDWIDTH-100-sizeadd, y-sizeadd, 80+(sizeadd*2), 50+(sizeadd*2), 120|V_SNAPTORIGHT);
				else
					V_DrawFill(BASEVIDWIDTH-100-sizeadd, y-sizeadd, 80+(sizeadd*2), 50+(sizeadd*2), color|V_SNAPTORIGHT);

				sizeadd--;
			}

			if (!levelinfo[i].encore)
				V_DrawSmallScaledPatch(BASEVIDWIDTH-100, y, V_SNAPTORIGHT, pic);
			else
			{
				V_DrawFixedPatch((BASEVIDWIDTH-20)<<FRACBITS, (y)<<FRACBITS, FRACUNIT/2, V_FLIP|V_SNAPTORIGHT, pic, 0);
				V_DrawFixedPatch((BASEVIDWIDTH-60)<<FRACBITS, ((y+25)<<FRACBITS) - (rubyheight<<1), FRACUNIT, V_SNAPTORIGHT, rubyicon, NULL);
			}

			V_DrawRightAlignedThinString(BASEVIDWIDTH-21, 40+y, V_SNAPTORIGHT|V_6WIDTHSPACE, str);

			if (levelinfo[i].gts)
			{
				INT32 w = V_ThinStringWidth(levelinfo[i].gts, V_SNAPTORIGHT)+1;
				V_DrawFill(BASEVIDWIDTH-100, y+10, w+1, 2, V_SNAPTORIGHT|31);
				V_DrawFill(BASEVIDWIDTH-100, y, w, 11, V_SNAPTORIGHT|levelinfo[i].gtc);
				V_DrawDiag(BASEVIDWIDTH-100+w+1, y, 12, V_SNAPTORIGHT|31);
				V_DrawDiag(BASEVIDWIDTH-100+w, y, 11, V_SNAPTORIGHT|levelinfo[i].gtc);
				V_DrawThinString(BASEVIDWIDTH-99, y+1, V_SNAPTORIGHT, levelinfo[i].gts);
			}

			y += 50;
		}
		else
		{
			if (!levelinfo[i].encore)
				V_DrawTinyScaledPatch(BASEVIDWIDTH-60, y, V_SNAPTORIGHT, pic);
			else
			{
				V_DrawFixedPatch((BASEVIDWIDTH-20)<<FRACBITS, y<<FRACBITS, FRACUNIT/4, V_FLIP|V_SNAPTORIGHT, pic, 0);
				V_DrawFixedPatch((BASEVIDWIDTH-40)<<FRACBITS, (y<<FRACBITS) + (25<<(FRACBITS-1)) - rubyheight, FRACUNIT/2, V_SNAPTORIGHT, rubyicon, NULL);
			}

			if (levelinfo[i].gts)
			{
				V_DrawDiag(BASEVIDWIDTH-60, y, 8, V_SNAPTORIGHT|31);
				V_DrawDiag(BASEVIDWIDTH-60, y, 6, V_SNAPTORIGHT|levelinfo[i].gtc);
			}
			y += 25;
		}

		y += 5-splitscreen;
	}

	x = 20;
	y = 10;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (dedicated && i == 0) // While leaving blank spots for non-existent players is largely intentional, the first spot *always* being blank looks a tad silly :V
			continue;

		if ((playeringame[i] && !players[i].spectator) && votes[i] != -1)
		{
			patch_t *pic;

			if (votes[i] >= 3 && (i != pickedvote || voteendtic == -1))
				pic = randomlvl;
			else
				pic = levelinfo[votes[i]].pic;

			if (!timer && i == voteclient.ranim)
			{
				V_DrawScaledPatch(x-18, y+9, V_SNAPTOLEFT, cursor);
				if (voteendtic != -1 && !(votetic % 4))
					V_DrawFill(x-1, y-1, 42, 27, 120|V_SNAPTOLEFT);
				else
					V_DrawFill(x-1, y-1, 42, 27, levelinfo[votes[i]].gtc|V_SNAPTOLEFT);
			}

			if (!levelinfo[votes[i]].encore)
				V_DrawTinyScaledPatch(x, y, V_SNAPTOLEFT, pic);
			else
			{
				V_DrawFixedPatch((x+40)<<FRACBITS, (y)<<FRACBITS, FRACUNIT/4, V_SNAPTOLEFT|V_FLIP, pic, 0);
				V_DrawFixedPatch((x+20)<<FRACBITS, (y<<FRACBITS) + (25<<(FRACBITS-1)) - rubyheight, FRACUNIT/2, V_SNAPTOLEFT, rubyicon, NULL);
			}

			if (levelinfo[votes[i]].gts)
			{
				V_DrawDiag(x, y, 8, V_SNAPTOLEFT|31);
				V_DrawDiag(x, y, 6, V_SNAPTOLEFT|levelinfo[votes[i]].gtc);
			}

			if (players[i].skincolor)
			{
				UINT8 *colormap = R_GetTranslationColormap(players[i].skin, players[i].skincolor, GTC_CACHE);
				V_DrawMappedPatch(x+24, y+9, V_SNAPTOLEFT, facerankprefix[players[i].skin], colormap);
			}

			if (!splitscreen && i == consoleplayer)
			{
				UINT8 cursorframe = (votetic / 4) % 8;
				V_DrawScaledPatch(x+24, y+9, V_SNAPTOLEFT, W_CachePatchName(va("K_CHILI%d", cursorframe+1), PU_CACHE));
			}
		}

		y += 30;

		if (y > BASEVIDHEIGHT-40)
		{
			x += 60;
			y = 10;
		}
	}

	if (timer)
	{
		INT32 hilicol, tickdown = (timer+1)/TICRATE;
		if (cons_menuhighlight.value)
			hilicol = cons_menuhighlight.value;
		else if (gametype == GT_RACE)
			hilicol = V_SKYMAP;
		else //if (gametype == GT_MATCH)
			hilicol = V_REDMAP;
		V_DrawCenteredString(BASEVIDWIDTH/2, 188, hilicol,
			va("Vote ends in %d", tickdown));
	}
}

//
// Y_VoteStop
//
// Vote screen's selection stops moving
//
SINT8 deferredlevel = 0;
static void Y_VoteStops(SINT8 pick, SINT8 level)
{
	nextmap = votelevels[level][0];

	if (level == 4)
		S_StartSound(NULL, sfx_noooo2); // gasp
	else if (mapheaderinfo[nextmap] && (mapheaderinfo[nextmap]->menuflags & LF2_HIDEINMENU))
		S_StartSound(NULL, sfx_noooo1); // this is bad
	else if (netgame && P_IsLocalPlayer(&players[pick]))
		S_StartSound(NULL, sfx_yeeeah); // yeeeah!
	else
		S_StartSound(NULL, sfx_kc48); // just a cool sound

	if (gametype != votelevels[level][1])
	{
		INT16 lastgametype = gametype;
		gametype = votelevels[level][1];
		D_GameTypeChanged(lastgametype);
		forceresetplayers = true;
	}

	deferencoremode = (levelinfo[level].encore);
}

//
// Y_VoteTicker
//
// Vote screen thinking :eggthinking:
//
void Y_VoteTicker(void)
{
	INT32 i;

	if (paused || P_AutoPause() || !voteclient.loaded)
		return;

#ifdef HAVE_BLUA
	LUAh_VoteThinker();
#endif

	votetic++;

	if (votetic == voteendtic)
	{
		Y_EndVote();
		Y_FollowIntermission();
		return;
	}

	for (i = 0; i < MAXPLAYERS; i++) // Correct votes as early as possible, before they're processed by the game at all
	{
		if (!playeringame[i] || players[i].spectator)
			votes[i] = -1; // Spectators are the lower class, and have effectively no voice in the government. Democracy sucks.
		else if (pickedvote != -1 && votes[i] == -1)
			votes[i] = 3; // Slow people get random
	}

	if (server && pickedvote != -1 && votes[pickedvote] == -1) // Uh oh! The person who got picked left! Recalculate, quick!
		D_PickVote();

	if (!votetic)
		S_ChangeMusicInternal("vote", true);

	if (timer)
		timer--;

	if (pickedvote != -1)
	{
		timer = 0;
		voteclient.rsynctime++;

		if (voteendtic == -1)
		{
			UINT8 tempvotes[MAXPLAYERS];
			UINT8 numvotes = 0;

			for (i = 0; i < MAXPLAYERS; i++)
			{
				if (votes[i] == -1)
					continue;
				tempvotes[numvotes] = i;
				numvotes++;
			}

			if (numvotes < 1) // Whoops! Get outta here.
			{
				Y_EndVote();
				Y_FollowIntermission();
				return;
			}

			voteclient.rtics--;

			if (voteclient.rtics <= 0)
			{
				voteclient.roffset++;
				voteclient.rtics = min(20, (3*voteclient.roffset/4)+5);
				S_StartSound(NULL, sfx_kc39);
			}

			if (voteclient.rendoff == 0 || voteclient.roffset < voteclient.rendoff)
				voteclient.ranim = tempvotes[((pickedvote + voteclient.roffset) % numvotes)];

			if (voteclient.roffset >= 20)
			{
				if (voteclient.rendoff == 0)
				{
					if (voteclient.rsynctime % 51 == 0) // Song is 1.45 seconds long (sorry @ whoever wants to replace it in a music wad :V)
					{
						for (i = 5; i >= 3; i--) // Find a suitable place to stop
						{
							if (tempvotes[((pickedvote + voteclient.roffset + i) % numvotes)] == pickedvote)
							{
								voteclient.rendoff = voteclient.roffset+i;
								if (M_RandomChance(FRACUNIT/32)) // Let it cheat occasionally~
									voteclient.rendoff++;
								S_ChangeMusicInternal("voteeb", false);
								break;
							}
						}
					}
				}
				else if (voteclient.roffset >= voteclient.rendoff)
				{
					voteendtic = votetic + (3*TICRATE);
					Y_VoteStops(pickedvote, deferredlevel);
				}
			}
		}
		else
			voteclient.ranim = pickedvote;
	}
	else
	{
		if (votetic < 3*(NEWTICRATE/7)) // give it some time before letting you control it :V
			return;

		for (i = 0; i <= splitscreen; i++)
		{
			UINT8 p;
			boolean pressed = false;

			switch (i)
			{
				case 1:
					p = displayplayers[1];
					break;
				case 2:
					p = displayplayers[2];
					break;
				case 3:
					p = displayplayers[3];
					break;
				default:
					p = consoleplayer;
					break;
			}

			if (voteclient.playerinfo[i].delay)
				voteclient.playerinfo[i].delay--;

			if ((playeringame[p] && !players[p].spectator)
				&& !voteclient.playerinfo[i].delay
				&& pickedvote == -1 && votes[p] == -1)
			{
				if (InputDown(gc_aimforward, i+1) || JoyAxis(AXISAIM, i+1) < 0)
				{
					voteclient.playerinfo[i].selection--;
					pressed = true;
				}

				if ((InputDown(gc_aimbackward, i+1) || JoyAxis(AXISAIM, i+1) > 0) && !pressed)
				{
					voteclient.playerinfo[i].selection++;
					pressed = true;
				}

				if (voteclient.playerinfo[i].selection < 0)
					voteclient.playerinfo[i].selection = 3;
				if (voteclient.playerinfo[i].selection > 3)
					voteclient.playerinfo[i].selection = 0;

				if ((InputDown(gc_accelerate, i+1) || JoyAxis(AXISMOVE, i+1) > 0) && !pressed)
				{
					D_ModifyClientVote(voteclient.playerinfo[i].selection, i);
					pressed = true;
				}
			}

			if (pressed)
			{
				S_StartSound(NULL, sfx_kc4a);
				voteclient.playerinfo[i].delay = NEWTICRATE/7;
			}
		}

		if (server)
		{
			if (timer == 0)
			{
				for (i = 0; i < MAXPLAYERS; i++)
				{
					if ((playeringame[i] && !players[i].spectator) && votes[i] == -1)
						votes[i] = 3;
				}
			}
			else
			{
				for (i = 0; i < MAXPLAYERS; i++)
				{
					if ((playeringame[i] && !players[i].spectator) && votes[i] == -1)
						return;
				}
			}

			timer = 0;
			if (voteendtic == -1)
				D_PickVote();
		}
	}
}

//
// Y_StartVote
//
// MK online style voting screen, appears after intermission
//
void Y_StartVote(void)
{
	INT32 i = 0;

	votetic = -1;

#ifdef PARANOIA
	if (voteendtic != -1)
		I_Error("voteendtic is dirty");
#endif

	widebgpatch = W_CachePatchName(((gametype == GT_MATCH) ? "BATTLSCW" : "INTERSCW"), PU_STATIC);
	bgpatch = W_CachePatchName(((gametype == GT_MATCH) ? "BATTLSCR" : "INTERSCR"), PU_STATIC);
	cursor = W_CachePatchName("M_CURSOR", PU_STATIC);
	cursor1 = W_CachePatchName("P1CURSOR", PU_STATIC);
	cursor2 = W_CachePatchName("P2CURSOR", PU_STATIC);
	cursor3 = W_CachePatchName("P3CURSOR", PU_STATIC);
	cursor4 = W_CachePatchName("P4CURSOR", PU_STATIC);
	randomlvl = W_CachePatchName("RANDOMLV", PU_STATIC);
	rubyicon = W_CachePatchName("RUBYICON", PU_STATIC);

	timer = cv_votetime.value*TICRATE;
	pickedvote = -1;

	for (i = 0; i < 3; i++)
	{
		voteclient.playerinfo[i].selection = 0;
		voteclient.playerinfo[i].delay = 0;
	}

	voteclient.ranim = 0;
	voteclient.rtics = 1;
	voteclient.roffset = 0;
	voteclient.rsynctime = 0;
	voteclient.rendoff = 0;

	for (i = 0; i < MAXPLAYERS; i++)
		votes[i] = -1;

	for (i = 0; i < 5; i++)
	{
		lumpnum_t lumpnum;

		// set up the encore
		levelinfo[i].encore = (votelevels[i][1] & 0x80);
		votelevels[i][1] &= ~0x80;

		// set up the str
		if (i == 4)
			levelinfo[i].str[0] = '\0';
		else
		{
			// set up the levelstring
			if (mapheaderinfo[votelevels[i][0]]->levelflags & LF_NOZONE || !mapheaderinfo[votelevels[i][0]]->zonttl[0])
			{
				if (mapheaderinfo[votelevels[i][0]]->actnum[0])
					snprintf(levelinfo[i].str,
						sizeof levelinfo[i].str,
						"%s %s",
						mapheaderinfo[votelevels[i][0]]->lvlttl, mapheaderinfo[votelevels[i][0]]->actnum);
				else
					snprintf(levelinfo[i].str,
						sizeof levelinfo[i].str,
						"%s",
						mapheaderinfo[votelevels[i][0]]->lvlttl);
			}
			else
			{
				if (mapheaderinfo[votelevels[i][0]]->actnum[0])
					snprintf(levelinfo[i].str,
						sizeof levelinfo[i].str,
						"%s %s %s",
						mapheaderinfo[votelevels[i][0]]->lvlttl, mapheaderinfo[votelevels[i][0]]->zonttl, mapheaderinfo[votelevels[i][0]]->actnum);
				else
					snprintf(levelinfo[i].str,
						sizeof levelinfo[i].str,
						"%s %s",
						mapheaderinfo[votelevels[i][0]]->lvlttl, mapheaderinfo[votelevels[i][0]]->zonttl);
			}

			levelinfo[i].str[sizeof levelinfo[i].str - 1] = '\0';
		}

		// set up the gtc and gts
		levelinfo[i].gtc = G_GetGametypeColor(votelevels[i][1]);
		if (i == 2 && votelevels[i][1] != votelevels[0][1])
			levelinfo[i].gts = gametype_cons_t[votelevels[i][1]].strvalue;
		else
			levelinfo[i].gts = NULL;

		// set up the pic
		lumpnum = W_CheckNumForName(va("%sP", G_BuildMapName(votelevels[i][0]+1)));
		if (lumpnum != LUMPERROR)
			levelinfo[i].pic = W_CachePatchName(va("%sP", G_BuildMapName(votelevels[i][0]+1)), PU_STATIC);
		else
			levelinfo[i].pic = W_CachePatchName("BLANKLVL", PU_STATIC);
	}

	voteclient.loaded = true;
}

//
// Y_EndVote
//
void Y_EndVote(void)
{
	Y_UnloadVoteData();
	voteendtic = -1;
}

//
// Y_UnloadVoteData
//
static void Y_UnloadVoteData(void)
{
	voteclient.loaded = false;

	if (rendermode != render_soft)
		return;

	UNLOAD(widebgpatch);
	UNLOAD(bgpatch);
	UNLOAD(cursor);
	UNLOAD(cursor1);
	UNLOAD(cursor2);
	UNLOAD(cursor3);
	UNLOAD(cursor4);
	UNLOAD(randomlvl);
	UNLOAD(rubyicon);

	UNLOAD(levelinfo[4].pic);
	UNLOAD(levelinfo[3].pic);
	UNLOAD(levelinfo[2].pic);
	UNLOAD(levelinfo[1].pic);
	UNLOAD(levelinfo[0].pic);
}

//
// Y_SetupVoteFinish
//
void Y_SetupVoteFinish(SINT8 pick, SINT8 level)
{
	if (!voteclient.loaded)
		return;

	if (pick == -1) // No other votes? We gotta get out of here, then!
	{
		Y_EndVote();
		Y_FollowIntermission();
		return;
	}

	if (pickedvote == -1)
	{
		INT32 i;
		SINT8 votecompare = -1;
		INT32 endtype = 0;

		voteclient.rsynctime = 0;

		for (i = 0; i < MAXPLAYERS; i++)
		{
			if ((playeringame[i] && !players[i].spectator) && votes[i] == -1)
				votes[i] = 3;

			if (votes[i] == -1 || endtype > 1) // Don't need to go on
				continue;

			if (level == 4)
			{
				votes[i] = 4;
				continue;
			}

			if (endtype == 2)
				continue;

			if (votecompare == -1)
			{
				votecompare = votes[i];
				endtype = 1;
			}
			else if (votes[i] != votecompare)
				endtype = 2;
		}

		if (level == 4 || endtype == 1) // Only one unique vote, so just end it immediately.
		{
			voteendtic = votetic + (5*TICRATE);
			S_ChangeMusicInternal("voteeb", false);
			Y_VoteStops(pick, level);
		}
		else if (endtype == 0) // Might as well put this here, too.
		{
			Y_EndVote();
			Y_FollowIntermission();
			return;
		}
		else
			S_ChangeMusicInternal("voteea", true);
	}

	deferredlevel = level;
	pickedvote = pick;
	timer = 0;
}
