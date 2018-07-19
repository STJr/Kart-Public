// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2004-2016 by Sonic Team Junior.
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
	} coop;

	struct
	{
		char passed1[29]; // KNUCKLES GOT    / CRAWLA HONCHO
		char passed2[17];             // A CHAOS EMERALD / GOT THEM ALL!
		char passed3[15];             //                   CAN NOW BECOME
		char passed4[SKINNAMESIZE+7]; //                   SUPER CRAWLA HONCHO
		INT32 passedx1;
		INT32 passedx2;
		INT32 passedx3;
		INT32 passedx4;

		y_bonus_t bonus;
		patch_t *bonuspatch;

		patch_t *pscore; // SCORE
		UINT32 score; // fake score

		// Continues
		UINT8 continues;
		patch_t *pcontinues;
		INT32 *playerchar; // Continue HUD
		UINT8 *playercolor;

		UINT8 gotlife; // Number of extra lives obtained
	} spec;*/

	struct
	{
		UINT32 scores[MAXPLAYERS]; // Winner's score
		UINT8 *color[MAXPLAYERS]; // Winner's color #
		INT32 *character[MAXPLAYERS]; // Winner's character #
		INT32 num[MAXPLAYERS]; // Winner's player #
		char *name[MAXPLAYERS]; // Winner's name
		INT32 numplayers; // Number of players being displayed
		char levelstring[62]; // holds levelnames up to 32 characters
		// SRB2kart
		int increase[MAXPLAYERS]; //how much did the score increase by?
		UINT32 val[MAXPLAYERS]; //Gametype-specific value
		UINT8 pos[MAXPLAYERS]; // player positions. used for ties
		boolean rankingsmode; // rankings mode
	} match;

	/*struct
	{
		UINT8 *color[MAXPLAYERS]; // Winner's color #
		INT32 *character[MAXPLAYERS]; // Winner's character #
		INT32 num[MAXPLAYERS]; // Winner's player #
		char name[MAXPLAYERS][9]; // Winner's name
		UINT32 times[MAXPLAYERS];
		UINT32 rings[MAXPLAYERS];
		UINT32 maxrings[MAXPLAYERS];
		UINT32 monitors[MAXPLAYERS];
		UINT32 scores[MAXPLAYERS];
		UINT32 points[MAXPLAYERS];
		INT32 numplayers; // Number of players being displayed
		char levelstring[40]; // holds levelnames up to 32 characters
	} competition;*/

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

//static void Y_AwardCoopBonuses(void);
//static void Y_AwardSpecialStageBonus(void);
static void Y_CalculateTournamentPoints(void); // SRB2kart
static void Y_MakeRankingsTable(void); // SRB2Kart also

//static void Y_CalculateCompetitionWinners(void);
//static void Y_CalculateTimeRaceWinners(void);
static void Y_CalculateMatchWinners(void);
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
} y_voteclient;

static y_votelvlinfo levelinfo[4];
static y_voteclient voteclient;
static INT32 votetic;
static INT32 voteendtic = -1;
static patch_t *cursor = NULL;
static patch_t *cursor1 = NULL;
static patch_t *cursor2 = NULL;
static patch_t *cursor3 = NULL;
static patch_t *cursor4 = NULL;
static patch_t *randomlvl = NULL;

static void Y_UnloadVoteData(void);

// Stuff copy+pasted from st_stuff.c
/*static INT32 SCX(INT32 x)
{
	return FixedInt(FixedMul(x<<FRACBITS, vid.fdupx));
}
static INT32 SCY(INT32 z)
{
	return FixedInt(FixedMul(z<<FRACBITS, vid.fdupy));
}*/

#define ST_DrawNumFromHud(h,n)        V_DrawTallNum(SCX(hudinfo[h].x), SCY(hudinfo[h].y), V_NOSCALESTART, n)
#define ST_DrawPadNumFromHud(h,n,q)   V_DrawPaddedTallNum(SCX(hudinfo[h].x), SCY(hudinfo[h].y), V_NOSCALESTART, n, q)
#define ST_DrawPatchFromHud(h,p)      V_DrawScaledPatch(SCX(hudinfo[h].x), SCY(hudinfo[h].y), V_NOSCALESTART, p)

//
// Y_IntermissionDrawer
//
// Called by D_Display. Nothing is modified here; all it does is draw.
// Neat concept, huh?
//
void Y_IntermissionDrawer(void)
{
	// Bonus loops
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
		V_DrawFadeScreen(0xFF00, 16);

	if (!splitscreen)
		whiteplayer = demoplayback ? displayplayer : consoleplayer;

	if (cons_menuhighlight.value)
		hilicol = cons_menuhighlight.value;
	else if (modeattacking)
		hilicol = V_ORANGEMAP;
	else
		hilicol = ((intertype == int_race) ? V_SKYMAP : V_REDMAP);

	if (sorttic != -1 && intertic > sorttic)
	{
		INT32 count = (intertic - sorttic);

		if (count < 8)
			x -= ((count * vid.width) / (8 * vid.dupx));
		else
		{
			if (!data.match.rankingsmode)
				Y_MakeRankingsTable();
			if (count == 8)
				goto dotimer;
			else if (count < 16)
				x += (((16 - count) * vid.width) / (8 * vid.dupx));
		}
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
		INT32 y = 48;
		char name[MAXPLAYERNAME+1];
		const char *timeheader = (data.match.rankingsmode ? "RANK" : (intertype == int_race ? "TIME" : "SCORE"));

		// draw the level name
		V_DrawCenteredString(-4 + x + BASEVIDWIDTH/2, 20, 0, data.match.levelstring);
		V_DrawFill(x, 42, 312, 1, 0);

		if (data.match.numplayers > 8)
		{
			V_DrawFill(160, 32, 1, 152, 0);

			V_DrawCenteredString(x+6+(BASEVIDWIDTH/2), 32, hilicol, "#");
			V_DrawString(x+36+(BASEVIDWIDTH/2), 32, hilicol, "NAME");

			V_DrawRightAlignedString(x+152, 32, hilicol, timeheader);
		}

		V_DrawCenteredString(x+6, 32, hilicol, "#");
		V_DrawString(x+36, 32, hilicol, "NAME");

		V_DrawRightAlignedString(x+(BASEVIDWIDTH/2)+152, 32, hilicol, timeheader);

		for (i = 0; i < data.match.numplayers; i++)
		{
			if (data.match.num[i] != MAXPLAYERS && playeringame[data.match.num[i]] && !players[data.match.num[i]].spectator)
			{
				char strtime[10];

				V_DrawCenteredString(x+6, y, 0, va("%d", data.match.pos[i]));

				if (data.match.color[i] == 0)
					V_DrawSmallScaledPatch(x+16, y-4, 0,faceprefix[*data.match.character[i]]);
				else
				{
					UINT8 *colormap = R_GetTranslationColormap(*data.match.character[i], *data.match.color[i], GTC_CACHE);
					V_DrawSmallMappedPatch(x+16, y-4, 0,faceprefix[*data.match.character[i]], colormap);
				}

				if (data.match.numplayers > 9)
					strlcpy(name, data.match.name[i], 6);
				else
					STRBUFCPY(name, data.match.name[i]);

				V_DrawString(x+36, y,
					((data.match.num[i] == whiteplayer)
						? hilicol|V_ALLOWLOWERCASE
						: V_ALLOWLOWERCASE),
					name);

				if (data.match.rankingsmode)
				{
					if (data.match.increase[i] > 9)
						snprintf(strtime, sizeof strtime, "(+%02d)", data.match.increase[i]);
					else
						snprintf(strtime, sizeof strtime, "(+  %d)", data.match.increase[i]);

					if (data.match.numplayers > 8)
						V_DrawRightAlignedString(x+120, y, 0, strtime);
					else
						V_DrawRightAlignedString(x+120+BASEVIDWIDTH/2, y, 0, strtime);

					snprintf(strtime, sizeof strtime, "%d", data.match.scores[i]-data.match.increase[i]);

					if (data.match.numplayers > 8)
						V_DrawRightAlignedString(x+152, y, 0, strtime);
					else
						V_DrawRightAlignedString(x+152+BASEVIDWIDTH/2, y, 0, strtime);
				}
				else
				{
					if (data.match.val[i] == (UINT32_MAX-1))
					{
						if (data.match.numplayers > 8)
							V_DrawRightAlignedThinString(x+152, y-1, 0, "NO CONTEST");
						else
							V_DrawRightAlignedThinString(x+152+BASEVIDWIDTH/2, y-1, 0, "NO CONTEST");
					}
					else
					{
						if (intertype == int_race)
						{
							snprintf(strtime, sizeof strtime, "%i:%02i.%02i", G_TicsToMinutes(data.match.val[i], true),
							G_TicsToSeconds(data.match.val[i]), G_TicsToCentiseconds(data.match.val[i]));
							strtime[sizeof strtime - 1] = '\0';

							if (data.match.numplayers > 8)
								V_DrawRightAlignedString(x+152, y, 0, strtime);
							else
								V_DrawRightAlignedString(x+152+BASEVIDWIDTH/2, y, 0, strtime);
						}
						else
						{
							if (data.match.numplayers > 8)
								V_DrawRightAlignedString(x+152, y, 0, va("%i", data.match.val[i]));
							else
								V_DrawRightAlignedString(x+152+BASEVIDWIDTH/2, y, 0, va("%i", data.match.val[i]));
						}
					}
				}
			}
			else
				data.match.num[i] = MAXPLAYERS;

			y += 16;

			if (y > 176)
			{
				y = 48;
				x += BASEVIDWIDTH/2;
			}
		}
	}
	/*else if (intertype == int_match)
	{
		INT32 y = 48;
		char name[MAXPLAYERNAME+1];
		hilicol = (cons_menuhighlight.value) ? cons_menuhighlight.value : V_REDMAP;

		// draw the level name
		V_DrawCenteredString(-4 + x + BASEVIDWIDTH/2, 20, 0, data.match.levelstring);
		V_DrawFill(x, 42, 312, 1, 0);

		if (data.match.numplayers > 9)
		{
			V_DrawFill(160, 32, 1, 152, 0);

			V_DrawRightAlignedString(x+152, 32, hilicol, "SCORE");

			V_DrawCenteredString(x+(BASEVIDWIDTH/2)+6, 32, hilicol, "#");
			V_DrawString(x+(BASEVIDWIDTH/2)+36, 32, hilicol, "NAME");
		}

		V_DrawCenteredString(x+6, 32, hilicol, "#");
		V_DrawString(x+36, 32, hilicol, "NAME");

		V_DrawRightAlignedString(x+(BASEVIDWIDTH/2)+152, 32, hilicol, "SCORE");

		for (i = 0; i < data.match.numplayers; i++)
		{
			V_DrawCenteredString(x+6, y, 0, va("%d", data.match.pos[i]));

			if (playeringame[data.match.num[i]])
			{
				// Draw the back sprite, it looks ugly if we don't
				V_DrawSmallScaledPatch(x+16, y-4, 0, livesback);

				if (data.match.color[i] == 0)
					V_DrawSmallScaledPatch(x+16, y-4, 0,faceprefix[*data.match.character[i]]);
				else
				{
					UINT8 *colormap = R_GetTranslationColormap(*data.match.character[i], *data.match.color[i], GTC_CACHE);
					V_DrawSmallMappedPatch(x+16, y-4, 0,faceprefix[*data.match.character[i]], colormap);
				}

				if (data.match.numplayers > 9)
					strlcpy(name, data.match.name[i], 9);
				else
					STRBUFCPY(name, data.match.name[i]);

				V_DrawString(x+36, y, V_ALLOWLOWERCASE, name);

				if (data.match.numplayers > 9)
					V_DrawRightAlignedString(x+152, y, 0, va("%i", data.match.scores[i]));
				else
					V_DrawRightAlignedString(x+152+BASEVIDWIDTH/2, y, 0, va("%u", data.match.scores[i]));
			}

			y += 16;

			if (y > 176)
			{
				y = 48;
				x += BASEVIDWIDTH/2;
			}
		}
	}*/
	/*else if (intertype == int_ctf || intertype == int_teammatch)
	{
		INT32 x = 4, y = 0;
		INT32 redplayers = 0, blueplayers = 0;
		char name[MAXPLAYERNAME+1];

		// Show the team flags and the team score at the top instead of "RESULTS"
		V_DrawSmallScaledPatch(128 - SHORT(data.match.blueflag->width)/4, 2, 0, data.match.blueflag);
		V_DrawCenteredString(128, 16, 0, va("%u", bluescore));

		V_DrawSmallScaledPatch(192 - SHORT(data.match.redflag->width)/4, 2, 0, data.match.redflag);
		V_DrawCenteredString(192, 16, 0, va("%u", redscore));

		// draw the level name
		V_DrawCenteredString(BASEVIDWIDTH/2, 24, 0, data.match.levelstring);
		V_DrawFill(4, 42, 312, 1, 0);

		//vert. line
		V_DrawFill(160, 32, 1, 152, 0);

		//strings at the top of the list
		V_DrawCenteredString(x+6, 32, V_YELLOWMAP, "#");
		V_DrawCenteredString(x+(BASEVIDWIDTH/2)+6, 32, V_YELLOWMAP, "#");

		V_DrawString(x+36, 32, V_YELLOWMAP, "NAME");
		V_DrawString(x+(BASEVIDWIDTH/2)+36, 32, V_YELLOWMAP, "NAME");

		V_DrawRightAlignedString(x+152, 32, V_YELLOWMAP, "SCORE");
		V_DrawRightAlignedString(x+(BASEVIDWIDTH/2)+152, 32, V_YELLOWMAP, "SCORE");

		for (i = 0; i < data.match.numplayers; i++)
		{
			if (playeringame[data.match.num[i]] && !(data.match.spectator[i]))
			{
				UINT8 *colormap = R_GetTranslationColormap(*data.match.character[i], *data.match.color[i], GTC_CACHE);

				if (*data.match.color[i] == SKINCOLOR_RED) //red
				{
					if (redplayers++ > 9)
						continue;
					x = 4 + (BASEVIDWIDTH/2);
					y = (redplayers * 16) + 32;
					V_DrawCenteredString(x+6, y, 0, va("%d", redplayers));
				}
				else if (*data.match.color[i] == SKINCOLOR_BLUE) //blue
				{
					if (blueplayers++ > 9)
						continue;
					x = 4;
					y = (blueplayers * 16) + 32;
					V_DrawCenteredString(x+6, y, 0, va("%d", blueplayers));
				}
				else
					continue;

				// Draw the back sprite, it looks ugly if we don't
				V_DrawSmallScaledPatch(x+16, y-4, 0, livesback);

				//color is ALWAYS going to be 6/7 here, no need to check if it's nonzero.
				V_DrawSmallMappedPatch(x+16, y-4, 0,faceprefix[*data.match.character[i]], colormap);

				strlcpy(name, data.match.name[i], 9);

				V_DrawString(x+36, y, V_ALLOWLOWERCASE, name);

				V_DrawRightAlignedString(x+152, y, 0, va("%u", data.match.scores[i]));
			}
		}
	}
	else if (intertype == int_classicrace)
	{
		INT32 x = 4;
		INT32 y = 48;
		UINT32 ptime, pring, pmaxring, pmonitor, pscore;
		char sstrtime[10];

		// draw the level name
		V_DrawCenteredString(BASEVIDWIDTH/2, 8, 0, data.competition.levelstring);
		V_DrawFill(4, 42, 312, 1, 0);

		V_DrawCenteredString(x+6, 32, V_YELLOWMAP, "#");
		V_DrawString(x+36, 32, V_YELLOWMAP, "NAME");
		// Time
		V_DrawRightAlignedString(x+160, 32, V_YELLOWMAP, "TIME");

		// Rings
		V_DrawThinString(x+168, 31, V_YELLOWMAP, "RING");

		// Total rings
		V_DrawThinString(x+191, 22, V_YELLOWMAP, "TOTAL");
		V_DrawThinString(x+196, 31, V_YELLOWMAP, "RING");

		// Monitors
		V_DrawThinString(x+223, 22, V_YELLOWMAP, "ITEM");
		V_DrawThinString(x+229, 31, V_YELLOWMAP, "BOX");

		// Score
		V_DrawRightAlignedString(x+288, 32, V_YELLOWMAP, "SCORE");

		// Points
		V_DrawRightAlignedString(x+312, 32, V_YELLOWMAP, "PT");

		for (i = 0; i < data.competition.numplayers; i++)
		{
			ptime = (data.competition.times[i] & ~0x80000000);
			pring = (data.competition.rings[i] & ~0x80000000);
			pmaxring = (data.competition.maxrings[i] & ~0x80000000);
			pmonitor = (data.competition.monitors[i] & ~0x80000000);
			pscore = (data.competition.scores[i] & ~0x80000000);

			V_DrawCenteredString(x+6, y, 0, va("%d", i+1));

			if (playeringame[data.competition.num[i]])
			{
				// Draw the back sprite, it looks ugly if we don't
				V_DrawSmallScaledPatch(x+16, y-4, 0, livesback);

				if (data.competition.color[i] == 0)
					V_DrawSmallScaledPatch(x+16, y-4, 0,faceprefix[*data.competition.character[i]]);
				else
				{
					UINT8 *colormap = R_GetTranslationColormap(*data.competition.character[i], *data.competition.color[i], GTC_CACHE);
					V_DrawSmallMappedPatch(x+16, y-4, 0,faceprefix[*data.competition.character[i]], colormap);
				}

				// already constrained to 8 characters
				V_DrawString(x+36, y, V_ALLOWLOWERCASE, data.competition.name[i]);

				if (players[data.competition.num[i]].pflags & PF_TIMEOVER)
					snprintf(sstrtime, sizeof sstrtime, "Time Over");
				else if (players[data.competition.num[i]].lives <= 0)
					snprintf(sstrtime, sizeof sstrtime, "Game Over");
				else
					snprintf(sstrtime, sizeof sstrtime, "%i:%02i.%02i", G_TicsToMinutes(ptime, true),
							G_TicsToSeconds(ptime), G_TicsToCentiseconds(ptime));

				sstrtime[sizeof sstrtime - 1] = '\0';
				// Time
				V_DrawRightAlignedThinString(x+160, y-1, ((data.competition.times[i] & 0x80000000) ? V_YELLOWMAP : 0), sstrtime);
				// Rings
				V_DrawRightAlignedThinString(x+188, y-1, V_MONOSPACE|((data.competition.rings[i] & 0x80000000) ? V_YELLOWMAP : 0), va("%u", pring));
				// Total rings
				V_DrawRightAlignedThinString(x+216, y-1, V_MONOSPACE|((data.competition.maxrings[i] & 0x80000000) ? V_YELLOWMAP : 0), va("%u", pmaxring));
				// Monitors
				V_DrawRightAlignedThinString(x+244, y-1, V_MONOSPACE|((data.competition.monitors[i] & 0x80000000) ? V_YELLOWMAP : 0), va("%u", pmonitor));
				// Score
				V_DrawRightAlignedThinString(x+288, y-1, V_MONOSPACE|((data.competition.scores[i] & 0x80000000) ? V_YELLOWMAP : 0), va("%u", pscore));
				// Final Points
				V_DrawRightAlignedString(x+312, y, V_YELLOWMAP, va("%d", data.competition.points[i]));
			}

			y += 16;

			if (y > 176)
				break;
		}
	}*/

dotimer:
	if (timer)
		V_DrawCenteredString(BASEVIDWIDTH/2, 188, hilicol,
			va("start in %d seconds", timer/TICRATE));

	// Make it obvious that scrambling is happening next round.
	if (cv_scrambleonchange.value && cv_teamscramble.value && (intertic/TICRATE % 2 == 0))
		V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT/2, V_YELLOWMAP, M_GetText("Teams will be scrambled next round!"));
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

	// Check for pause or menu up in single player
	if (paused || P_AutoPause())
		return;

	intertic++;

	// Team scramble code for team match and CTF.
	// Don't do this if we're going to automatically scramble teams next round.
	if (G_GametypeHasTeams() && cv_teamscramble.value && !cv_scrambleonchange.value && server)
	{
		// If we run out of time in intermission, the beauty is that
		// the P_Ticker() team scramble code will pick it up.
		if ((intertic % (TICRATE/7)) == 0)
			P_DoTeamscrambling();
	}

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

	if (endtic != -1)
		return; // tally is done

	/* // SRB2kart
	if (intertype == int_coop) // coop or single player, normal level
	{
		INT32 i;
		UINT32 oldscore = data.coop.score;
		boolean skip = false;
		boolean anybonuses = false;

		if (!intertic) // first time only
			S_ChangeMusicInternal("lclear", false); // don't loop it

		if (intertic < TICRATE) // one second pause before tally begins
			return;

		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i] && (players[i].cmd.buttons & BT_BRAKE || players[i].cmd.buttons & BT_ACCELERATE))
				skip = true;

		// bonuses count down by 222 each tic
		for (i = 0; i < 4; ++i)
		{
			if (!data.coop.bonuses[i].points)
				continue;

			data.coop.bonuses[i].points -= 222;
			data.coop.total += 222;
			data.coop.score += 222;
			if (data.coop.bonuses[i].points < 0 || skip == true) // too far?
			{
				data.coop.score += data.coop.bonuses[i].points;
				data.coop.total += data.coop.bonuses[i].points;
				data.coop.bonuses[i].points = 0;
			}
			if (data.coop.bonuses[i].points > 0)
				anybonuses = true;
		}

		if (!anybonuses)
		{
			endtic = intertic + 3*TICRATE; // 3 second pause after end of tally
			S_StartSound(NULL, sfx_chchng); // cha-ching!

			// Update when done with tally
			if ((!modifiedgame || savemoddata) && !(netgame || multiplayer) && !demoplayback)
			{
				if (M_UpdateUnlockablesAndExtraEmblems(false))
					S_StartSound(NULL, sfx_ncitem);

				G_SaveGameData(false);
			}
		}
		else if (!(intertic & 1))
			S_StartSound(NULL, sfx_ptally); // tally sound effect

		if (data.coop.gotlife > 0 && (skip == true || data.coop.score % 50000 < oldscore % 50000)) // just passed a 50000 point mark
		{
			// lives are already added since tally is fake, but play the music
			P_PlayLivesJingle(NULL);
			--data.coop.gotlife;
		}
	}
	else if (intertype == int_spec) // coop or single player, special stage
	{
		INT32 i;
		UINT32 oldscore = data.spec.score;
		boolean skip = false;
		static INT32 tallydonetic = 0;

		if (!intertic) // first time only
		{
			S_ChangeMusicInternal("lclear", false); // don't loop it
			tallydonetic = 0;
		}

		if (intertic < TICRATE) // one second pause before tally begins
			return;

		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i] && (players[i].cmd.buttons & BT_BRAKE || players[i].cmd.buttons & BT_ACCELERATE))
				skip = true;

		if (tallydonetic != 0)
		{
			if (intertic > tallydonetic)
			{
				endtic = intertic + 4*TICRATE; // 4 second pause after end of tally
				S_StartSound(NULL, sfx_flgcap); // cha-ching!
			}
			return;
		}

		// ring bonus counts down by 222 each tic
		data.spec.bonus.points -= 222;
		data.spec.score += 222;
		if (data.spec.bonus.points < 0 || skip == true) // went too far
		{
			data.spec.score += data.spec.bonus.points;
			data.spec.bonus.points = 0;
		}

		if (!data.spec.bonus.points)
		{
			if (data.spec.continues & 0x80) // don't set endtic yet!
				tallydonetic = intertic + (3*TICRATE)/2;
			else // okay we're good.
				endtic = intertic + 4*TICRATE; // 4 second pause after end of tally

			S_StartSound(NULL, sfx_chchng); // cha-ching!

			// Update when done with tally
			if ((!modifiedgame || savemoddata) && !(netgame || multiplayer) && !demoplayback)
			{
				if (M_UpdateUnlockablesAndExtraEmblems(false))
					S_StartSound(NULL, sfx_ncitem);

				G_SaveGameData(false);
			}
		}
		else if (!(intertic & 1))
			S_StartSound(NULL, sfx_ptally); // tally sound effect

		if (data.spec.gotlife > 0 && (skip == true || data.spec.score % 50000 < oldscore % 50000)) // just passed a 50000 point mark
		{
			// lives are already added since tally is fake, but play the music
			P_PlayLivesJingle(NULL);
			--data.spec.gotlife;
		}
	}
	if (intertype == int_timeattack)
	{
		if (!intertic)
			endtic = intertic + 10*TICRATE; // 10 second pause after end of tally
	}*/
	else if (intertype == int_race || intertype == int_match)
	{
		INT32 q=0,r=0;
		boolean kaching = true;

		/* // SRB2kart - removed temporarily.
		if (!intertic) {
			if (!((music_playing == "krwin") // Win
			|| (music_playing == "krok") // Ok
			|| (music_playing == "krlose"))) // Lose
				S_ChangeMusicInternal("racent", true); // Backup Plan
		}*/

		if (intertic < TICRATE || intertic & 1)
			return;

		if (data.match.rankingsmode && intertic > sorttic+(2*TICRATE))
		{
			for (q = 0; q < data.match.numplayers; q++)
			{
				if (data.match.increase[q]) {
					data.match.increase[q]--;
					r++;
					if (data.match.increase[q])
						kaching = false;
				}
			}

			if (r)
				S_StartSound(NULL, (kaching ? sfx_chchng : sfx_ptally));
			else
				endtic = intertic + 3*TICRATE; // 3 second pause after end of tally
		}

		if (modeattacking)
			endtic = intertic + 8*TICRATE; // 8 second pause after end of tally
		else if (netgame || multiplayer)
		{
			if (sorttic == -1)
				sorttic = intertic + max((cv_inttime.value/2)-2, 2)*TICRATE; // 8 second pause after match results
		}
	}
	/*else if (intertype == int_match) //|| intertype == int_ctf || intertype == int_teammatch) // match
	{
		if (!intertic) // first time only
			S_ChangeMusicInternal("racent", true); // loop it

		// If a player has left or joined, recalculate scores.
		//if (data.match.numplayers != D_NumPlayers())
			//Y_CalculateMatchWinners();
	}*/
	/*else if (intertype == int_race || intertype == int_classicrace) // race
	{
		if (!intertic) // first time only
			S_ChangeMusicInternal("racent", true); // loop it

		// Don't bother recalcing for race. It doesn't make as much sense.
	}*/
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
		CONS_Printf(M_GetText("\x82" "Earned %hu emblem%s for Record Attack records.\n"), (UINT16)earnedEmblems, earnedEmblems > 1 ? "s" : "");

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

		/*
		if (G_IsSpecialStage(gamemap))
			intertype = (maptol & TOL_NIGHTS) ? int_nightsspec : int_spec;
		else
			intertype = (maptol & TOL_NIGHTS) ? int_nights : int_coop;
		*/
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
		else
		{
			timer = cv_inttime.value*TICRATE;

			if (!timer)
				timer = 1;
		}

		/* // SRB2kart
		if (gametype == GT_COOP)
		{
			// Nights intermission is single player only
			// Don't add it here
			if (G_IsSpecialStage(gamemap))
				intertype = int_spec;
			else
				intertype = int_coop;
		}
		else */
		if (gametype == GT_TEAMMATCH)
			intertype = int_teammatch;
		else if (gametype == GT_MATCH
		 || gametype == GT_TAG
		 || gametype == GT_HIDEANDSEEK)
			intertype = int_match;
		else if (gametype == GT_RACE)
			intertype = int_race;
		else if (gametype == GT_COMPETITION)
			intertype = int_classicrace;
		else if (gametype == GT_CTF)
			intertype = int_ctf;
	}

	// We couldn't display the intermission even if we wanted to.
	// But we still need to give the players their score bonuses, dummy.
	//if (dedicated) return;

	// This should always exist, but just in case...
	if(!mapheaderinfo[prevmap])
		P_AllocMapHeader(prevmap);

	switch (intertype)
	{
		/*case int_nights:
			// Can't fail
			//G_SetNightsRecords();

			// Check records
			{
				UINT8 earnedEmblems = M_CheckLevelEmblems();
				if (earnedEmblems)
					CONS_Printf(M_GetText("\x82" "Earned %hu emblem%s for NiGHTS records.\n"), (UINT16)earnedEmblems, earnedEmblems > 1 ? "s" : "");
			}

			// fall back into the coop intermission for now
			intertype = int_timeattack;
		case int_timeattack: // coop or single player, normal level // SRB2kart 230117 - replaced int_coop
		{
			// award time and ring bonuses
			// Y_AwardCoopBonuses();

			// setup time data
			data.coop.tics = players[consoleplayer].realtime;

			if ((!modifiedgame || savemoddata) && !multiplayer && !demoplayback)
			{
				// Update visitation flags
				mapvisited[gamemap-1] |= MV_BEATEN;
				if (ALL7EMERALDS(emeralds))
					mapvisited[gamemap-1] |= MV_ALLEMERALDS;

				if (modeattacking == ATTACKING_RECORD)
					Y_UpdateRecordReplays();
			}

			for (i = 0; i < 4; ++i)
				data.coop.bonuspatches[i] = W_CachePatchName(data.coop.bonuses[i].patch, PU_STATIC);
			data.coop.ptotal = W_CachePatchName("YB_TOTAL", PU_STATIC);

			// get act number
			data.coop.ttlnum = W_CachePatchName("TTL01", PU_STATIC);

			// get background patches
			widebgpatch = W_CachePatchName("INTERSCW", PU_STATIC);
			bgpatch = W_CachePatchName("INTERSCR", PU_STATIC);

			// grab an interscreen if appropriate
			if (mapheaderinfo[gamemap-1]->interscreen[0] != '#')
			{
				interpic = W_CachePatchName(mapheaderinfo[gamemap-1]->interscreen, PU_STATIC);
				useinterpic = true;
				usebuffer = false;
			}
			else
			{
				useinterpic = false;
#ifdef HWRENDER
				if (rendermode == render_opengl)
					usebuffer = true; // This needs to be here for OpenGL, otherwise usebuffer is never set to true for it, and thus there's no screenshot in the intermission
#endif
			}
			usetile = false;

			// set up the "got through act" message according to skin name
			// too long so just show "YOU GOT THROUGH THE ACT"
			if (strlen(skins[players[consoleplayer].skin].realname) > 13)
			{
				strcpy(data.coop.passed1, "YOU GOT");
				strcpy(data.coop.passed2, (strlen(mapheaderinfo[prevmap]->actnum) > 0) ? "THROUGH ACT" : "THROUGH THE ACT");
			}
			// long enough that "X GOT" won't fit so use "X PASSED THE ACT"
			else if (strlen(skins[players[consoleplayer].skin].realname) > 8)
			{
				strcpy(data.coop.passed1, skins[players[consoleplayer].skin].realname);
				strcpy(data.coop.passed2, (strlen(mapheaderinfo[prevmap]->actnum) > 0) ? "PASSED ACT" : "PASSED THE ACT");
			}
			// length is okay for normal use
			else
			{
				snprintf(data.coop.passed1, sizeof data.coop.passed1, "%s GOT",
					skins[players[consoleplayer].skin].realname);
				strcpy(data.coop.passed2, (strlen(mapheaderinfo[prevmap]->actnum) > 0) ? "THROUGH ACT" : "THROUGH THE ACT");
			}

			// set X positions
			if (strlen(mapheaderinfo[prevmap]->actnum) > 0)
			{
				data.coop.passedx1 = 62 + (176 - V_LevelNameWidth(data.coop.passed1))/2;
				data.coop.passedx2 = 62 + (176 - V_LevelNameWidth(data.coop.passed2))/2;
			}
			else
			{
				data.coop.passedx1 = (BASEVIDWIDTH - V_LevelNameWidth(data.coop.passed1))/2;
				data.coop.passedx2 = (BASEVIDWIDTH - V_LevelNameWidth(data.coop.passed2))/2;
			}
			// The above value is not precalculated because it needs only be computed once
			// at the start of intermission, and precalculating it would preclude mods
			// changing the font to one of a slightly different width.
			break;
		}

		// SRB2kart 230117 - removed
		case int_nightsspec:
			if (modeattacking && stagefailed)
			{
				// Nuh-uh.  Get out of here.
				Y_EndIntermission();
				Y_FollowIntermission();
				break;
			}
			if (!stagefailed)
				G_SetNightsRecords();

			// Check records
			{
				UINT8 earnedEmblems = M_CheckLevelEmblems();
				if (earnedEmblems)
					CONS_Printf(M_GetText("\x82" "Earned %hu emblem%s for NiGHTS records.\n"), (UINT16)earnedEmblems, earnedEmblems > 1 ? "s" : "");
			}

			// fall back into the special stage intermission for now
			intertype = int_spec;
			// FALLTHRU
		case int_spec: // coop or single player, special stage
		{
			// Update visitation flags?
			if ((!modifiedgame || savemoddata) && !multiplayer && !demoplayback)
			{
				if (!stagefailed)
					mapvisited[gamemap-1] |= MV_BEATEN;
			}

			// give out ring bonuses
			Y_AwardSpecialStageBonus();

			data.spec.bonuspatch = W_CachePatchName(data.spec.bonus.patch, PU_STATIC);
			data.spec.pscore = W_CachePatchName("YB_SCORE", PU_STATIC);
			data.spec.pcontinues = W_CachePatchName("YB_CONTI", PU_STATIC);

			// get background tile
			bgtile = W_CachePatchName("SPECTILE", PU_STATIC);

			// grab an interscreen if appropriate
			if (mapheaderinfo[gamemap-1]->interscreen[0] != '#')
			{
				interpic = W_CachePatchName(mapheaderinfo[gamemap-1]->interscreen, PU_STATIC);
				useinterpic = true;
			}
			else
				useinterpic = false;

			// tile if using the default background
			usetile = !useinterpic;

			// get special stage specific patches
			if (!stagefailed && ALL7EMERALDS(emeralds))
			{
				data.spec.cemerald = W_CachePatchName("GOTEMALL", PU_STATIC);
				data.spec.headx = 70;
				data.spec.nowsuper = players[consoleplayer].skin
					? NULL : W_CachePatchName("NOWSUPER", PU_STATIC);
			}
			else
			{
				data.spec.cemerald = W_CachePatchName("CEMERALD", PU_STATIC);
				data.spec.headx = 48;
				data.spec.nowsuper = NULL;
			}

			// Super form stuff (normally blank)
			data.spec.passed3[0] = '\0';
			data.spec.passed4[0] = '\0';

			// Super form stuff (normally blank)
			data.spec.passed3[0] = '\0';
			data.spec.passed4[0] = '\0';

			// set up the "got through act" message according to skin name
			if (stagefailed)
			{
				strcpy(data.spec.passed2, "SPECIAL STAGE");
				data.spec.passed1[0] = '\0';
			}
			else if (ALL7EMERALDS(emeralds))
			{
				snprintf(data.spec.passed1,
					sizeof data.spec.passed1, "%s",
					skins[players[consoleplayer].skin].realname);
				data.spec.passed1[sizeof data.spec.passed1 - 1] = '\0';
				strcpy(data.spec.passed2, "GOT THEM ALL!");

				if (skins[players[consoleplayer].skin].flags & SF_SUPER)
				{
					strcpy(data.spec.passed3, "CAN NOW BECOME");
					snprintf(data.spec.passed4,
						sizeof data.spec.passed4, "SUPER %s",
						skins[players[consoleplayer].skin].realname);
					data.spec.passed4[sizeof data.spec.passed4 - 1] = '\0';
				}
			}
			else
			{
				if (strlen(skins[players[consoleplayer].skin].realname) <= SKINNAMESIZE-5)
				{
					snprintf(data.spec.passed1,
						sizeof data.spec.passed1, "%s GOT",
						skins[players[consoleplayer].skin].realname);
					data.spec.passed1[sizeof data.spec.passed1 - 1] = '\0';
				}
				else
					strcpy(data.spec.passed1, "YOU GOT");
				strcpy(data.spec.passed2, "A CHAOS EMERALD");
			}
			data.spec.passedx1 = (BASEVIDWIDTH - V_LevelNameWidth(data.spec.passed1))/2;
			data.spec.passedx2 = (BASEVIDWIDTH - V_LevelNameWidth(data.spec.passed2))/2;
			data.spec.passedx3 = (BASEVIDWIDTH - V_LevelNameWidth(data.spec.passed3))/2;
			data.spec.passedx4 = (BASEVIDWIDTH - V_LevelNameWidth(data.spec.passed4))/2;
			break;
		}*/

		case int_match:
		{
			// Calculate who won
			Y_CalculateMatchWinners();
			S_ChangeMusicInternal("racent", true); // loop it
			break;
		}
		case int_race: // (time-only race)
		{
			if ((!modifiedgame || savemoddata) && !multiplayer && !demoplayback) // remove this once we have a proper time attack screen
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
			Y_CalculateTournamentPoints();
			break;
		}

		/*case int_teammatch:
		case int_ctf:
		{
			// Calculate who won
			Y_CalculateMatchWinners();

			// set up the levelstring
			if (strlen(mapheaderinfo[prevmap]->actnum) > 0)
				snprintf(data.match.levelstring,
					sizeof data.match.levelstring,
					"%.32s * %s *",
					mapheaderinfo[prevmap]->lvlttl, mapheaderinfo[prevmap]->actnum);
			else
				snprintf(data.match.levelstring,
					sizeof data.match.levelstring,
					"* %.32s *",
					mapheaderinfo[prevmap]->lvlttl);

			data.match.levelstring[sizeof data.match.levelstring - 1] = '\0';

			if (intertype == int_ctf)
			{
				data.match.redflag = rflagico;
				data.match.blueflag = bflagico;
			}
			else // team match
			{
				data.match.redflag = rmatcico;
				data.match.blueflag = bmatcico;
			}

			bgtile = W_CachePatchName("SRB2BACK", PU_STATIC);
			usetile = true;
			useinterpic = false;
			break;
		}

		case int_classicrace: // classic (full race)
		{
			// find out who won
			Y_CalculateCompetitionWinners();

			// set up the levelstring
			if (strlen(mapheaderinfo[prevmap]->actnum) > 0)
				snprintf(data.competition.levelstring,
					sizeof data.competition.levelstring,
					"%.32s * %s *",
					mapheaderinfo[prevmap]->lvlttl, mapheaderinfo[prevmap]->actnum);
			else
				snprintf(data.competition.levelstring,
					sizeof data.competition.levelstring,
					"* %.32s *",
					mapheaderinfo[prevmap]->lvlttl);

			data.competition.levelstring[sizeof data.competition.levelstring - 1] = '\0';

			// get background tile
			bgtile = W_CachePatchName("SRB2BACK", PU_STATIC);
			usetile = true;
			useinterpic = false;
			break;
		}*/

		case int_none:
		default:
			break;
	}

	if (intertype == int_race || intertype == int_match)
	{
		// set up the levelstring
		if (strlen(mapheaderinfo[prevmap]->zonttl) > 0)
		{
			if (strlen(mapheaderinfo[prevmap]->actnum) > 0)
				snprintf(data.match.levelstring,
					sizeof data.match.levelstring,
					"* %.32s %.32s %s *",
					mapheaderinfo[prevmap]->lvlttl, mapheaderinfo[prevmap]->zonttl, mapheaderinfo[prevmap]->actnum);
			else
				snprintf(data.match.levelstring,
					sizeof data.match.levelstring,
					"* %.32s %.32s *",
					mapheaderinfo[prevmap]->lvlttl, mapheaderinfo[prevmap]->zonttl);
		}
		else
		{
			if (strlen(mapheaderinfo[prevmap]->actnum) > 0)
				snprintf(data.match.levelstring,
					sizeof data.match.levelstring,
					"* %.32s %s *",
					mapheaderinfo[prevmap]->lvlttl, mapheaderinfo[prevmap]->actnum);
			else
				snprintf(data.match.levelstring,
					sizeof data.match.levelstring,
					"* %.32s *",
					mapheaderinfo[prevmap]->lvlttl);
		}

		data.match.levelstring[sizeof data.match.levelstring - 1] = '\0';

		//bgtile = W_CachePatchName("SRB2BACK", PU_STATIC);
		usetile = useinterpic = false;
		usebuffer = true;
	}
}

/*
//
// Y_CalculateTimeRaceWinners
//
static void Y_CalculateTimeRaceWinners(void)
{
	INT32 i, j;
	boolean completed[MAXPLAYERS];

	// Initialize variables

	for (i = 0; i < MAXPLAYERS; i++)
		data.match.scores[i] = INT32_MAX;

	memset(data.match.color, 0, sizeof (data.match.color));
	memset(data.match.character, 0, sizeof (data.match.character));
	memset(data.match.spectator, 0, sizeof (data.match.spectator));
	memset(completed, 0, sizeof (completed));
	data.match.numplayers = 0;
	i = j = 0;

	for (j = 0; j < MAXPLAYERS; j++)
	{
		if (!playeringame[j])
			continue;

		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!playeringame[i])
				continue;

			if (players[i].realtime <= data.match.scores[data.match.numplayers] && completed[i] == false)
			{
				data.match.scores[data.match.numplayers] = players[i].realtime;
				data.match.color[data.match.numplayers] = &players[i].skincolor;
				data.match.character[data.match.numplayers] = &players[i].skin;
				data.match.name[data.match.numplayers] = player_names[i];
				data.match.num[data.match.numplayers] = i;
			}
		}
		completed[data.match.num[data.match.numplayers]] = true;
		data.match.numplayers++;
	}
}
*/

//
// Y_CalculateCompetitionWinners
//
/*static void Y_CalculateCompetitionWinners(void)
{
	INT32 i, j;
	boolean bestat[5];
	boolean completed[MAXPLAYERS];
	INT32 winner; // shortcut

	UINT32 points[MAXPLAYERS];
	UINT32 times[MAXPLAYERS];
	UINT32 rings[MAXPLAYERS];
	UINT32 maxrings[MAXPLAYERS];
	UINT32 monitors[MAXPLAYERS];
	UINT32 scores[MAXPLAYERS];

	memset(data.competition.points, 0, sizeof (data.competition.points));
	memset(points, 0, sizeof (points));
	memset(completed, 0, sizeof (completed));

	// Award points.
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i])
			continue;

		for (j = 0; j < 5; j++)
			bestat[j] = true;

		times[i]    = players[i].realtime;
		rings[i]    = (UINT32)max(players[i].health-1, 0);
		maxrings[i] = (UINT32)players[i].totalring;
		monitors[i] = (UINT32)players[i].numboxes;
		scores[i]   = (UINT32)min(players[i].score, 99999990);

		for (j = 0; j < MAXPLAYERS; j++)
		{
			if (!playeringame[j] || j == i)
				continue;

			if (players[i].realtime <= players[j].realtime)
				points[i]++;
			else
				bestat[0] = false;

			if (max(players[i].health-1, 0) >= max(players[j].health-1, 0))
				points[i]++;
			else
				bestat[1] = false;

			if (players[i].totalring >= players[j].totalring)
				points[i]++;
			else
				bestat[2] = false;

			if (players[i].numboxes >= players[j].numboxes)
				points[i]++;
			else
				bestat[3] = false;

			if (players[i].score >= players[j].score)
				points[i]++;
			else
				bestat[4] = false;
		}

		// Highlight best scores
		if (bestat[0])
			times[i] |= 0x80000000;
		if (bestat[1])
			rings[i] |= 0x80000000;
		if (bestat[2])
			maxrings[i] |= 0x80000000;
		if (bestat[3])
			monitors[i] |= 0x80000000;
		if (bestat[4])
			scores[i] |= 0x80000000;
	}

	// Now we go through and set the data.competition struct properly
	data.competition.numplayers = 0;
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i])
			continue;

		winner = 0;

		for (j = 0; j < MAXPLAYERS; j++)
		{
			if (!playeringame[j])
				continue;

			if (points[j] >= data.competition.points[data.competition.numplayers] && completed[j] == false)
			{
				data.competition.points[data.competition.numplayers] = points[j];
				data.competition.num[data.competition.numplayers] = winner = j;
			}
		}
		// We know this person won this spot, now let's set everything appropriately
		data.competition.times[data.competition.numplayers] = times[winner];
		data.competition.rings[data.competition.numplayers] = rings[winner];
		data.competition.maxrings[data.competition.numplayers] = maxrings[winner];
		data.competition.monitors[data.competition.numplayers] = monitors[winner];
		data.competition.scores[data.competition.numplayers] = scores[winner];

		snprintf(data.competition.name[data.competition.numplayers], 9, "%s", player_names[winner]);
		data.competition.name[data.competition.numplayers][8] = '\0';

		data.competition.color[data.competition.numplayers] = &players[winner].skincolor;
		data.competition.character[data.competition.numplayers] = &players[winner].skin;

		completed[winner] = true;
		data.competition.numplayers++;
	}
}

// ============
// COOP BONUSES
// ============

//
// Y_SetNullBonus
// No bonus in this slot, but we need to set some things anyway.
//
static void Y_SetNullBonus(player_t *player, y_bonus_t *bstruct)
{
	(void)player;
	memset(bstruct, 0, sizeof(y_bonus_t));
	strncpy(bstruct->patch, "MISSING", sizeof(bstruct->patch));
}

//
// Y_SetTimeBonus
//
static void Y_SetTimeBonus(player_t *player, y_bonus_t *bstruct)
{
	INT32 secs, bonus;

	strncpy(bstruct->patch, "YB_TIME", sizeof(bstruct->patch));
	bstruct->display = true;

	// calculate time bonus
	secs = player->realtime / TICRATE;*/
#if 0
	if      (secs <  30) /*   :30 */ bonus = 100000;
	else if (secs <  45) /*   :45 */ bonus = 50000;
	else if (secs <  60) /*  1:00 */ bonus = 10000;
	else if (secs <  90) /*  1:30 */ bonus = 5000;
	else if (secs < 120) /*  2:00 */ bonus = 4000;
	else if (secs < 180) /*  3:00 */ bonus = 3000;
	else if (secs < 240) /*  4:00 */ bonus = 2000;
	else if (secs < 300) /*  5:00 */ bonus = 1000;
	else if (secs < 360) /*  6:00 */ bonus = 500;
	else if (secs < 420) /*  7:00 */ bonus = 400;
	else if (secs < 480) /*  8:00 */ bonus = 300;
	else if (secs < 540) /*  9:00 */ bonus = 200;
	else if (secs < 600) /* 10:00 */ bonus = 100;
	else  /* TIME TAKEN: TOO LONG */ bonus = 0;
#endif
	/*bstruct->points = bonus;
}

//
// Y_SetRingBonus
//
static void Y_SetRingBonus(player_t *player, y_bonus_t *bstruct)
{
	strncpy(bstruct->patch, "YB_RING", sizeof(bstruct->patch));
	bstruct->display = true;
	bstruct->points = max(0, (player->health-1) * 100);
}

//
// Y_SetLinkBonus
//

static void Y_SetLinkBonus(player_t *player, y_bonus_t *bstruct) // SRB2kart - unused.
{
	strncpy(bstruct->patch, "YB_LINK", sizeof(bstruct->patch));
	bstruct->display = true;
	bstruct->points = max(0, (player->maxlink - 1) * 100);
}


//
// Y_SetGuardBonus
//
static void Y_SetGuardBonus(player_t *player, y_bonus_t *bstruct)
{
	INT32 bonus;
	strncpy(bstruct->patch, "YB_GUARD", sizeof(bstruct->patch));
	bstruct->display = true;

	if      (player->timeshit == 0) bonus = 10000;
	else if (player->timeshit == 1) bonus = 5000;
	else if (player->timeshit == 2) bonus = 1000;
	else if (player->timeshit == 3) bonus = 500;
	else if (player->timeshit == 4) bonus = 100;
	else                            bonus = 0;
	bstruct->points = bonus;
}

//
// Y_SetPerfectBonus
//
static void Y_SetPerfectBonus(player_t *player, y_bonus_t *bstruct)
{
	INT32 i;

	(void)player;
	memset(bstruct, 0, sizeof(y_bonus_t));
	strncpy(bstruct->patch, "YB_PERFE", sizeof(bstruct->patch));

	if (data.coop.gotperfbonus == -1)
	{
		INT32 sharedringtotal = 0;
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!playeringame[i]) continue;
			sharedringtotal += players[i].health - 1;
		}
		if (!sharedringtotal || sharedringtotal < nummaprings)
			data.coop.gotperfbonus = 0;
		else
			data.coop.gotperfbonus = 1;
	}
	if (!data.coop.gotperfbonus)
		return;

	bstruct->display = true;
	bstruct->points = 50000;
}

// This list can be extended in the future with SOC/Lua, perhaps.
typedef void (*bonus_f)(player_t *, y_bonus_t *);
bonus_f bonuses_list[4][4] = {
	{
		Y_SetNullBonus,
		Y_SetNullBonus,
		Y_SetNullBonus,
		Y_SetNullBonus,
	},
	{
		Y_SetNullBonus,
		Y_SetTimeBonus,
		Y_SetRingBonus,
		Y_SetPerfectBonus,
	},
	{
		Y_SetNullBonus,
		Y_SetGuardBonus,
		Y_SetRingBonus,
		Y_SetNullBonus,
	},
	{
		Y_SetNullBonus,
		Y_SetGuardBonus,
		Y_SetRingBonus,
		Y_SetPerfectBonus,
	},
};


  // SRB2kart 230117 - Replaced with Y_CalculateTournamentPoints
//
// Y_AwardCoopBonuses
//
// Awards the time and ring bonuses.
//
static void Y_AwardCoopBonuses(void)
{
	INT32 i, j, bonusnum, oldscore, ptlives;
	y_bonus_t localbonuses[4];

	// set score/total first
	data.coop.total = 0;
	data.coop.score = players[consoleplayer].score;
	data.coop.gotperfbonus = -1;
	memset(data.coop.bonuses, 0, sizeof(data.coop.bonuses));
	memset(data.coop.bonuspatches, 0, sizeof(data.coop.bonuspatches));

	for (i = 0; i < MAXPLAYERS; ++i)
	{
		if (!playeringame[i] || players[i].lives < 1) // not active or game over
			bonusnum = 0; // all null
		else
			bonusnum = mapheaderinfo[prevmap]->bonustype + 1; // -1 is none

		oldscore = players[i].score;

		for (j = 0; j < 4; ++j) // Set bonuses
		{
			(bonuses_list[bonusnum][j])(&players[i], &localbonuses[j]);
			players[i].score += localbonuses[j].points;
		}

		ptlives = (!ultimatemode && !modeattacking) ? max((players[i].score/50000) - (oldscore/50000), 0) : 0;
		if (ptlives)
			P_GivePlayerLives(&players[i], ptlives);

		if (i == consoleplayer)
		{
			data.coop.gotlife = ptlives;
			M_Memcpy(&data.coop.bonuses, &localbonuses, sizeof(data.coop.bonuses));
		}
	}

	// Just in case the perfect bonus wasn't checked.
	if (data.coop.gotperfbonus < 0)
		data.coop.gotperfbonus = 0;
}

//
// Y_AwardSpecialStageBonus
//
// Gives a ring bonus only.
static void Y_AwardSpecialStageBonus(void)
{
	INT32 i, oldscore, ptlives;
	y_bonus_t localbonus;

	data.spec.score = players[consoleplayer].score;
	memset(&data.spec.bonus, 0, sizeof(data.spec.bonus));
	data.spec.bonuspatch = NULL;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		oldscore = players[i].score;

		if (!playeringame[i] || players[i].lives < 1) // not active or game over
			Y_SetNullBonus(&players[i], &localbonus);
		else if (useNightsSS) // Link instead of Score
			Y_SetLinkBonus(&players[i], &localbonus);
		else
			Y_SetRingBonus(&players[i], &localbonus);
		players[i].score += localbonus.points;

		// grant extra lives right away since tally is faked
		ptlives = (!ultimatemode && !modeattacking) ? max((players[i].score/50000) - (oldscore/50000), 0) : 0;
		if (ptlives)
			P_GivePlayerLives(&players[i], ptlives);

		if (i == consoleplayer)
		{
			M_Memcpy(&data.spec.bonus, &localbonus, sizeof(data.spec.bonus));

			data.spec.gotlife = ptlives;

			// Continues related
			data.spec.continues = min(players[i].continues, 8);
			if (players[i].gotcontinue)
				data.spec.continues |= 0x80;
			data.spec.playercolor = &players[i].skincolor;
			data.spec.playerchar = &players[i].skin;
		}
	}
}
*/

//
// Y_CalculateTournamentPoints
//
static void Y_CalculateTournamentPoints(void)
{
	INT32 i, j;
	boolean completed[MAXPLAYERS];
	INT32 numplayersingame = 0;

	// Initialize variables
	data.match.rankingsmode = false;
	for (j = 0; j < MAXPLAYERS; j++)
		data.match.val[j] = UINT32_MAX;
	memset(data.match.color, 0, sizeof (data.match.color));
	memset(data.match.character, 0, sizeof (data.match.character));
	memset(data.match.increase, 0, sizeof (data.match.increase));
	memset(completed, 0, sizeof (completed));
	data.match.numplayers = 0;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i] || players[i].spectator)
			continue;

		numplayersingame++;
	}

	for (j = 0; j < numplayersingame; j++)
	{
		for (i = 0; i < MAXPLAYERS; i++)
		{
			UINT32 timeval;

			if (!playeringame[i] || players[i].spectator || completed[i])
				continue;

			timeval = ((players[i].pflags & PF_TIMEOVER || players[i].realtime == UINT32_MAX)
					? (UINT32_MAX-1) : players[i].realtime);

			if (timeval < data.match.val[data.match.numplayers])
			{
				data.match.val[data.match.numplayers] = timeval;
				data.match.num[data.match.numplayers] = i;
			}
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

		if (!(players[i].pflags & PF_TIMEOVER))
			data.match.increase[data.match.numplayers] = numplayersingame - data.match.pos[data.match.numplayers];
		players[i].score += data.match.increase[data.match.numplayers];
		data.match.scores[data.match.numplayers] = players[i].score;

		data.match.numplayers++;
	}
}

//
// Y_CalculateMatchWinners
//
static void Y_CalculateMatchWinners(void)
{
	INT32 i, j;
	boolean completed[MAXPLAYERS];
	INT32 numplayersingame = 0;

	// Initialize variables
	data.match.rankingsmode = false;
	for (j = 0; j < MAXPLAYERS; j++)
		data.match.val[j] = UINT32_MAX;
	memset(data.match.color, 0, sizeof (data.match.color));
	memset(data.match.character, 0, sizeof (data.match.character));
	memset(data.match.increase, 0, sizeof (data.match.increase));
	memset(completed, 0, sizeof (completed));
	data.match.numplayers = 0;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i] || players[i].spectator)
			continue;

		numplayersingame++;
	}

	for (j = 0; j < numplayersingame; j++)
	{
		for (i = 0; i < MAXPLAYERS; i++)
		{
			UINT32 marescoreval;

			if (!playeringame[i] || players[i].spectator || completed[i])
				continue;

			marescoreval = ((players[i].pflags & PF_TIMEOVER)
					? (UINT32_MAX-1) : players[i].marescore);

			if (data.match.val[data.match.numplayers] == UINT32_MAX
			|| (!(players[i].pflags & PF_TIMEOVER) && marescoreval > data.match.val[data.match.numplayers]))
			{
				data.match.val[data.match.numplayers] = marescoreval;
				data.match.num[data.match.numplayers] = i;
			}
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

		if (!(players[i].pflags & PF_TIMEOVER))
			data.match.increase[data.match.numplayers] = numplayersingame - data.match.pos[data.match.numplayers];
		players[i].score += data.match.increase[data.match.numplayers];
		data.match.scores[data.match.numplayers] = players[i].score;

		data.match.numplayers++;
	}
}

//
// Y_MakeRankingsTable
//
static void Y_MakeRankingsTable(void)
{
	INT32 i, j;
	boolean completed[MAXPLAYERS];
	INT32 numplayersingame = 0;

	// Initialize variables
	data.match.rankingsmode = true;
	for (j = 0; j < MAXPLAYERS; j++)
		data.match.scores[j] = UINT32_MAX;
	memset(data.match.color, 0, sizeof (data.match.color));
	memset(data.match.character, 0, sizeof (data.match.character));
	memset(completed, 0, sizeof (completed));
	data.match.numplayers = 0;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i] || players[i].spectator)
			continue;

		numplayersingame++;
	}

	for (j = 0; j < numplayersingame; j++)
	{
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!playeringame[i] || players[i].spectator || completed[i])
				continue;

			if (data.match.scores[data.match.numplayers] == UINT32_MAX || players[i].score > data.match.scores[data.match.numplayers])
			{
				data.match.scores[data.match.numplayers] = players[i].score;
				data.match.num[data.match.numplayers] = i;
			}
		}

		i = data.match.num[data.match.numplayers];

		completed[i] = true;

		data.match.color[data.match.numplayers] = &players[i].skincolor;
		data.match.character[data.match.numplayers] = &players[i].skin;
		data.match.name[data.match.numplayers] = player_names[i];

		if (data.match.numplayers && (data.match.scores[data.match.numplayers] == data.match.scores[data.match.numplayers-1]))
			data.match.pos[data.match.numplayers] = data.match.pos[data.match.numplayers-1];
		else
			data.match.pos[data.match.numplayers] = data.match.numplayers+1;

		data.match.numplayers++;
	}

	snprintf(data.match.levelstring,
		sizeof data.match.levelstring,
		"* Total Rankings *");
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
// Y_EndGame
//
// Why end the game?
// Because Y_FollowIntermission and F_EndCutscene would
// both do this exact same thing *in different ways* otherwise,
// which made it so that you could only unlock Ultimate mode
// if you had a cutscene after the final level and crap like that.
// This function simplifies it so only one place has to be updated
// when something new is added.
void Y_EndGame(void)
{
	// Only do evaluation and credits in coop games.
	if (gametype == GT_COOP)
	{
		if (nextmap == 1102-1) // end game with credits
		{
			F_StartCredits();
			return;
		}
		if (nextmap == 1101-1) // end game with evaluation
		{
			F_StartGameEvaluation();
			return;
		}
	}

	// 1100 or competitive multiplayer, so go back to title screen.
	D_StartTitle();
}

//
// Y_FollowIntermission
//
static void Y_FollowIntermission(void)
{
	if (modeattacking)
	{
		M_EndModeAttackRun();
		return;
	}

	if (nextmap < 1100-1)
	{
		// normal level
		G_AfterIntermission();
		return;
	}

	// Start a custom cutscene if there is one.
	if (mapheaderinfo[gamemap-1]->cutscenenum && !modeattacking)
	{
		F_StartCustomCutscene(mapheaderinfo[gamemap-1]->cutscenenum-1, false, false);
		return;
	}

	Y_EndGame();
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

	if (rendermode == render_none)
		return;

	if (votetic >= voteendtic && voteendtic != -1)
		return;

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
		char str[40];
		patch_t *pic;
		UINT8 sizeadd = selected[i], j, color;

		if (i == 3)
		{
			snprintf(str, sizeof str, "%.32s", "RANDOM");
			str[sizeof str - 1] = '\0';
			pic = randomlvl;
		}
		else
		{
			strcpy(str, levelinfo[i].str);
			pic = levelinfo[i].pic;
		}

		if (selected[i])
		{
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
							p = secondarydisplayplayer;
							break;
						case 2:
							thiscurs = cursor3;
							p = thirddisplayplayer;
							break;
						case 3:
							thiscurs = cursor4;
							p = fourthdisplayplayer;
							break;
						default:
							thiscurs = cursor1;
							p = displayplayer;
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

			V_DrawSmallScaledPatch(BASEVIDWIDTH-100, y, V_SNAPTORIGHT, pic);
			V_DrawRightAlignedThinString(BASEVIDWIDTH-20, 40+y, V_SNAPTORIGHT, str);
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
			V_DrawTinyScaledPatch(BASEVIDWIDTH-60, y, V_SNAPTORIGHT, pic);
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

			if (votes[i] == 3 && (i != pickedvote || voteendtic == -1))
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

			V_DrawTinyScaledPatch(x, y, V_SNAPTOLEFT, pic);
			if (levelinfo[votes[i]].gts)
			{
				V_DrawDiag(x, y, 8, V_SNAPTOLEFT|31);
				V_DrawDiag(x, y, 6, V_SNAPTOLEFT|levelinfo[votes[i]].gtc);
			}

			if (players[i].skincolor == 0)
				V_DrawSmallScaledPatch(x+24, y+9, V_SNAPTOLEFT, faceprefix[players[i].skin]);
			else
			{
				UINT8 *colormap = R_GetTranslationColormap(players[i].skin, players[i].skincolor, GTC_CACHE);
				V_DrawSmallMappedPatch(x+24, y+9, V_SNAPTOLEFT, faceprefix[players[i].skin], colormap);
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
		INT32 hilicol;
		if (cons_menuhighlight.value)
			hilicol = cons_menuhighlight.value;
		else if (gametype == GT_RACE)
			hilicol = V_SKYMAP;
		else //if (gametype == GT_MATCH)
			hilicol = V_REDMAP;
		V_DrawCenteredString(BASEVIDWIDTH/2, 188, hilicol|V_SNAPTOBOTTOM,
			va("Vote ends in %d seconds", timer/TICRATE));
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
	if (!splitscreen && pick == consoleplayer)
		S_StartSound(NULL, sfx_yeeeah);
	else
		S_StartSound(NULL, sfx_kc48);

	nextmap = votelevels[level][0];
	if (gametype != votelevels[level][1])
	{
		INT16 lastgametype = gametype;
		gametype = votelevels[level][1];
		D_GameTypeChanged(lastgametype);
		forceresetplayers = true;
	}
}

//
// Y_VoteTicker
//
// Vote screen thinking :eggthinking:
//
void Y_VoteTicker(void)
{
	INT32 i;

	if (paused || P_AutoPause())
		return;

	votetic++;

	if (votetic == voteendtic)
	{
		Y_UnloadVoteData(); // Y_EndVote resets voteendtic too early apparently, causing the game to try to render patches that we just unloaded...
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
				Y_UnloadVoteData();
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
								if (M_RandomChance(FRACUNIT/1024)) // Let it cheat occasionally~
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
					p = secondarydisplayplayer;
					break;
				case 2:
					p = thirddisplayplayer;
					break;
				case 3:
					p = fourthdisplayplayer;
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

	for (i = 0; i < 4; i++)
	{
		lumpnum_t lumpnum;
		//INT16 j;

		// set up the str
		if (strlen(mapheaderinfo[votelevels[i][0]]->zonttl) > 0)
		{
			if (strlen(mapheaderinfo[votelevels[i][0]]->actnum) > 0)
				snprintf(levelinfo[i].str,
					sizeof levelinfo[i].str,
					"%.32s %.32s %s",
					mapheaderinfo[votelevels[i][0]]->lvlttl, mapheaderinfo[votelevels[i][0]]->zonttl, mapheaderinfo[votelevels[i][0]]->actnum);
			else
				snprintf(levelinfo[i].str,
					sizeof levelinfo[i].str,
					"%.32s %.32s",
					mapheaderinfo[votelevels[i][0]]->lvlttl, mapheaderinfo[votelevels[i][0]]->zonttl);
		}
		else
		{
			if (strlen(mapheaderinfo[votelevels[i][0]]->actnum) > 0)
				snprintf(levelinfo[i].str,
					sizeof levelinfo[i].str,
					"%.32s %s",
					mapheaderinfo[votelevels[i][0]]->lvlttl, mapheaderinfo[votelevels[i][0]]->actnum);
			else
				snprintf(levelinfo[i].str,
					sizeof levelinfo[i].str,
					"%.32s",
					mapheaderinfo[votelevels[i][0]]->lvlttl);
		}

		levelinfo[i].str[sizeof levelinfo[i].str - 1] = '\0';

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
	if (pick == -1) // No other votes? We gotta get out of here, then!
	{
		timer = 0;
		Y_UnloadVoteData();
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

			if (votecompare == -1)
			{
				votecompare = votes[i];
				endtype = 1;
			}
			else if (votes[i] != votecompare)
				endtype = 2;
		}

		if (endtype == 0) // Might as well put this here, too.
		{
			timer = 0;
			Y_UnloadVoteData();
			Y_FollowIntermission();
			return;
		}
		else if (endtype == 1) // Only one unique vote, so just end it immediately.
		{
			voteendtic = votetic + (5*TICRATE);
			S_ChangeMusicInternal("voteeb", false);
			Y_VoteStops(pick, level);
		}
		else
			S_ChangeMusicInternal("voteea", true);
	}

	deferredlevel = level;
	pickedvote = pick;
	timer = 0;
}
