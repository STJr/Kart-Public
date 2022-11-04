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
/// \file  f_finale.c
/// \brief Title screen, intro, game evaluation, and credits.

#include "doomdef.h"
#include "doomstat.h"
#include "d_main.h"
#include "f_finale.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "r_local.h"
#include "s_sound.h"
#include "i_time.h"
#include "i_video.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"
#include "i_system.h"
#include "i_threads.h"
#include "m_menu.h"
#include "dehacked.h"
#include "g_input.h"
#include "console.h"
#include "m_random.h"
#include "y_inter.h"
#include "m_cond.h"

// Stage of animation:
// 0 = text, 1 = art screen
static INT32 finalecount;
INT32 titlescrollspeed = 5;

static INT32 timetonext; // Delay between screen changes
static INT32 continuetime; // Short delay when continuing

static tic_t animtimer; // Used for some animation timings
static tic_t credbgtimer; // Credits background
static INT32 roidtics; // Asteroid spinning

static tic_t stoptimer;

static boolean keypressed = false;

// (no longer) De-Demo'd Title Screen
#if 0
static UINT8  laststaff = 0;
#endif
static UINT32 demoDelayLeft;
static UINT32 demoIdleLeft;

static patch_t *ttbanner; // SONIC ROBO BLAST 2
static patch_t *ttkart; // *vroom* KART
static patch_t *ttcheckers; // *vroom* KART
static patch_t *ttkflash; // flash screen

static patch_t *driver[2]; // Driving character on the waiting screen
static UINT8 *waitcolormap; // colormap for the spinning character

static void F_SkyScroll(INT32 scrollspeed);

//
// CUTSCENE TEXT WRITING
//
static const char *cutscene_basetext = NULL;
static char cutscene_disptext[1024];
static INT32 cutscene_baseptr = 0;
static INT32 cutscene_writeptr = 0;
static INT32 cutscene_textcount = 0;
static INT32 cutscene_textspeed = 0;
static UINT8 cutscene_boostspeed = 0;
static tic_t cutscene_lasttextwrite = 0;
//
// This alters the text string cutscene_disptext.
// Use the typical string drawing functions to display it.
// Returns 0 if \0 is reached (end of input)
//
static UINT8 F_WriteText(void)
{
	INT32 numtowrite = 1;
	const char *c;
	tic_t ltw = I_GetTime();

	if (cutscene_lasttextwrite == ltw)
		return 1; // singletics prevention
	cutscene_lasttextwrite = ltw;

	if (cutscene_boostspeed)
	{
		// for custom cutscene speedup mode
		numtowrite = 8;
	}
	else
	{
		// Don't draw any characters if the count was 1 or more when we started
		if (--cutscene_textcount >= 0)
			return 1;

		if (cutscene_textspeed < 7)
			numtowrite = 8 - cutscene_textspeed;
	}

	for (;numtowrite > 0;++cutscene_baseptr)
	{
		c = &cutscene_basetext[cutscene_baseptr];
		if (!c || !*c || *c=='#')
			return 0;

		// \xA0 - \xAF = change text speed
		if ((UINT8)*c >= 0xA0 && (UINT8)*c <= 0xAF)
		{
			cutscene_textspeed = (INT32)((UINT8)*c - 0xA0);
			continue;
		}
		// \xB0 - \xD2 = delay character for up to one second (35 tics)
		else if ((UINT8)*c >= 0xB0 && (UINT8)*c <= (0xB0+TICRATE-1))
		{
			cutscene_textcount = (INT32)((UINT8)*c - 0xAF);
			numtowrite = 0;
			continue;
		}

		cutscene_disptext[cutscene_writeptr++] = *c;

		// Ignore other control codes (color)
		if ((UINT8)*c < 0x80)
			--numtowrite;
	}
	// Reset textcount for next tic based on speed
	// if it wasn't already set by a delay.
	if (cutscene_textcount < 0)
	{
		cutscene_textcount = 0;
		if (cutscene_textspeed > 7)
			cutscene_textcount = cutscene_textspeed - 7;
	}
	return 1;
}

static void F_NewCutscene(const char *basetext)
{
	cutscene_basetext = basetext;
	memset(cutscene_disptext,0,sizeof(cutscene_disptext));
	cutscene_writeptr = cutscene_baseptr = 0;
	cutscene_textspeed = 9;
	cutscene_textcount = TICRATE/2;
}

//
// F_SkyScroll
//
static void F_SkyScroll(INT32 scrollspeed)
{
	INT32 x, y, w;
	patch_t *pat, *pat2;
	INT32 anim2 = 0;

	pat = W_CachePatchName("TITLEBG1", PU_CACHE);
	pat2 = W_CachePatchName("TITLEBG2", PU_CACHE);

	w = vid.width / vid.dupx;

	animtimer = ((finalecount*scrollspeed)/16) % SHORT(pat->width);
	anim2 = SHORT(pat2->width) - (((finalecount*scrollspeed)/16) % SHORT(pat2->width));

	// SRB2Kart: F_DrawPatchCol is over-engineered; recoded to be less shitty and error-prone
	if (rendermode != render_none)
	{
		V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 120);

		x = -((INT32)animtimer);
		y = 0;
		while (x < w)
		{
			V_DrawFixedPatch(x*FRACUNIT, y*FRACUNIT, FRACUNIT, V_SNAPTOTOP|V_SNAPTOLEFT, pat, NULL);
			x += SHORT(pat->width);
		}

		x = -anim2;
		y = BASEVIDHEIGHT - SHORT(pat2->height);
		while (x < w)
		{
			V_DrawFixedPatch(x*FRACUNIT, y*FRACUNIT, FRACUNIT, V_SNAPTOBOTTOM|V_SNAPTOLEFT, pat2, NULL);
			x += SHORT(pat2->width);
		}
	}

	W_UnlockCachedPatch(pat);
	W_UnlockCachedPatch(pat2);
}

// =============
//  INTRO SCENE
// =============
#define NUMINTROSCENES 1
INT32 intro_scenenum = 0;
INT32 intro_curtime = 0;

const char *introtext[NUMINTROSCENES];

static tic_t introscenetime[NUMINTROSCENES] =
{
	 4*TICRATE,	// KART KR(eW
};

// custom intros
void F_StartCustomCutscene(INT32 cutscenenum, boolean precutscene, boolean resetplayer);

void F_StartIntro(void)
{
	if (gamestate)
	{
		F_WipeStartScreen();
		V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);
		F_WipeEndScreen();
		F_RunWipe(wipedefs[wipe_level_final], false);
	}

	if (introtoplay)
	{
		if (!cutscenes[introtoplay - 1])
			D_StartTitle();
		else
			F_StartCustomCutscene(introtoplay - 1, false, false);
		return;
	}

	introtext[0] = " #";

	G_SetGamestate(GS_INTRO);
	gameaction = ga_nothing;
	paused = false;
	CON_ToggleOff();
	F_NewCutscene(introtext[0]);

	intro_scenenum = 0;
	finalecount = animtimer = stoptimer = 0;
	roidtics = BASEVIDWIDTH - 64;
	timetonext = introscenetime[intro_scenenum];
	S_StopMusic();
}

//
// F_IntroDrawer
//
void F_IntroDrawer(void)
{
	boolean highres = false;
	INT32 cx = 8, cy = 128;
	patch_t *background = NULL;
	INT32 bgxoffs = 0;

	// DRAW A FULL PIC INSTEAD OF FLAT!
	if (intro_scenenum == 0)
	{
		background = W_CachePatchName("KARTKREW", PU_CACHE);
		highres = true;
	}

	V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 120);

	if (background)
	{
		if (highres)
			V_DrawSmallScaledPatch(bgxoffs, 0, 0, background);
		else
			V_DrawScaledPatch(bgxoffs, 0, 0, background);
	}

	W_UnlockCachedPatch(background);

	V_DrawString(cx, cy, 0, cutscene_disptext);
}

//
// F_IntroTicker
//
void F_IntroTicker(void)
{
	// advance animation
	finalecount++;

	if (finalecount % 3 == 0)
		roidtics--;

	timetonext--;

	if (intro_scenenum == 0)
	{
		if (timetonext <= 0)
		{
			intro_scenenum++;
			if (rendermode != render_none)
			{
				F_WipeStartScreen();
				V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);
				F_WipeEndScreen();
				F_RunWipe(99,true);
			}

			// Stay on black for a bit. =)
			{
				tic_t quittime;
				quittime = I_GetTime() + NEWTICRATE*2; // Shortened the quit time, used to be 2 seconds
				while (quittime > I_GetTime())
				{
					I_Sleep(cv_sleep.value);
					I_UpdateTime(cv_timescale.value);
				}
			}

			D_StartTitle();
			return;
		}
		if (finalecount == 8)
			S_StartSound(NULL, sfx_vroom);
		else if (finalecount == 47)
		{
			// Need to use M_Random otherwise it always uses the same sound
			INT32 rskin = M_RandomKey(numskins);
			UINT8 rtaunt = M_RandomKey(2);
			sfxenum_t rsound = skins[rskin].soundsid[SKSKBST1+rtaunt];
			S_StartSound(NULL, rsound);
		}
	}

	F_WriteText();

	// check for skipping
	if (keypressed)
		keypressed = false;
}

//
// F_IntroResponder
//
boolean F_IntroResponder(event_t *event)
{
	INT32 key = event->data1;

	// remap virtual keys (mouse & joystick buttons)
	switch (key)
	{
		case KEY_MOUSE1:
			key = KEY_ENTER;
			break;
		case KEY_MOUSE1 + 1:
			key = KEY_BACKSPACE;
			break;
		case KEY_JOY1:
		case KEY_JOY1 + 2:
			key = KEY_ENTER;
			break;
		case KEY_JOY1 + 3:
			key = 'n';
			break;
		case KEY_JOY1 + 1:
			key = KEY_BACKSPACE;
			break;
		case KEY_HAT1:
			key = KEY_UPARROW;
			break;
		case KEY_HAT1 + 1:
			key = KEY_DOWNARROW;
			break;
		case KEY_HAT1 + 2:
			key = KEY_LEFTARROW;
			break;
		case KEY_HAT1 + 3:
			key = KEY_RIGHTARROW;
			break;
	}

	if (event->type != ev_keydown && key != 301)
		return false;

	if (key != 27 && key != KEY_ENTER && key != KEY_SPACE && key != KEY_BACKSPACE)
		return false;

	if (keypressed)
		return false;

	keypressed = true;
	return true;
}

// =========
//  CREDITS
// =========
static const char *credits[] = {
	"\1SRB2Kart",
	"\1Credits",
	"",
	"\1Game Design",
	"Sally \"TehRealSalt\" Cochenour",
	"Jeffery \"Chromatian\" Scott",
	"\"VelocitOni\"",
	"",
	"\1Lead Programming",
	"Sally \"TehRealSalt\" Cochenour",
	"Vivian \"toaster\" Grannell",
	"Ronald \"Eidolon\" Kinard",
	"James Robert Roman",
	"Sean \"Sryder\" Ryder",
	"Ehab \"wolfs\" Saeed",
	"\"ZarroTsu\"",
	"",
	"\1Support Programming",
	"\"Lach\"",
	"\"Lat\'\"",
	"AJ \"Tyron\" Martinez",
	"\"Monster Iestyn\"",
	"\"SteelT\"",
	"",
	"\1External Programming",
	"Alam Ed Arias",
	"\"alphaRexJames\"",
	"\"Ashnal\"",
	"\"filpAM\"",
	"\"FlykeSpice\"",
	"\"Hannu Hanhi\"",
	"\"himie\"",
	"\"JugadorXEI\"",
	"\"Kimberly\"",
	"\"Lighto97\"",
	"\"Lonsfor\"",
	"\"mazmazz\"",
	"\"minenice\"",
	"\"Shuffle\"",
	"\"Snu\"",
	"\"X.organic\"",
	"",
	"\1Lead Artists",
	"Desmond \"Blade\" DesJardins",
	"\"VelocitOni\"",
	"",
	"\1Support Artists",
	"Sally \"TehRealSalt\" Cochenour",
	"\"Chengi\"",
	"\"Chrispy\"",
	"Sherman \"CoatRack\" DesJardins",
	"\"DrTapeworm\"",
	"Jesse \"Jeck Jims\" Emerick",
	"Wesley \"Charyb\" Gillebaard",
	"\"Nev3r\"",
	"Vivian \"toaster\" Grannell",
	"James \"SeventhSentinel\" Hall",
	"\"Lat\'\"",
	"\"rairai104n\"",
	"\"Tyrannosaur Chao\"",
	"\"ZarroTsu\"",
	"",
	"\1External Artists",
	"\"1-Up Mason\"",
	"\"DirkTheHusky\"",
	"\"LJSTAR\"",
	"\"MotorRoach\"",
	"\"Ritz\"",
	"\"Rob\"",
	"\"SmithyGNC\"",
	"\"Snu\"",
	"\"Spherallic\"",
	"\"TelosTurntable\"",
	"\"VAdaPEGA\"",
	"\"Virt\"",
	"\"Voltrix\"",
	"",
	"\1Sound Design",
	"James \"SeventhSentinel\" Hall",
	"Sonic Team",
	"\"VAdaPEGA\"",
	"\"VelocitOni\"",
	"",
	"\1Original Music",
	"\"DrTapeworm\"",
	"Wesley \"Charyb\" Gillebaard",
	"James \"SeventhSentinel\" Hall",
	"",
	"\1Lead Level Design",
	"\"Blitz-T\"",
	"Sally \"TehRealSalt\" Cochenour",
	"Desmond \"Blade\" DesJardins",
	"Jeffery \"Chromatian\" Scott",
	"\"Tyrannosaur Chao\"",
	"",
	"\1Support Level Design",
	"\"Chaos Zero 64\"",
	"\"D00D64\"",
	"\"DrTapeworm\"",
	"Paul \"Boinciel\" Clempson",
	"Sherman \"CoatRack\" DesJardins",
	"Vivian \"toaster\" Grannell",
	"\"Gunla\"",
	"James \"SeventhSentinel\" Hall",
	"\"Lat\'\"",
	"\"MK\"",
	"\"Ninferno\"",
	"Sean \"Sryder\" Ryder",
	"\"Ryuspark\"",
	"\"Simsmagic\"",
	"Ivo Solarin",
	"\"SP47\"",
	"\"TG\"",
	"\"Victor Rush Turbo\"",
	"\"ZarroTsu\"",
	"",
	"\1Testing",
	"RKH License holders",
	"The KCS",
	"\"CyberIF\"",
	"\"Dani\"",
	"Karol \"Fooruman\" D""\x1E""browski", // Dąbrowski, <Sryder> accents in srb2 :ytho:
	"\"Virt\"",
	"",
	"\1Special Thanks",
	"SEGA",
	"Sonic Team",
	"SRB2 & Sonic Team Jr. (www.srb2.org)",
	"\"Chaos Zero 64\"",
	"",
	"\1Produced By",
	"Kart Krew",
	"",
	"\1In Memory of",
	"\"Tyler52\"",
	"",
	"",
	"\1Thank you",
	"\1for playing!",
	NULL
};

static struct {
	UINT32 x, y;
	const char *patch;
	UINT8 colorize;
} credits_pics[] = {
	// We don't have time to be fancy, let's just colorize some item sprites :V
	{224, 80+(216* 1), "K_ITJAWZ", SKINCOLOR_CREAMSICLE},
	{224, 80+(216* 2), "K_ITSPB",  SKINCOLOR_GARDEN},
	{224, 80+(216* 3), "K_ITBANA", SKINCOLOR_LILAC},
	{224, 80+(216* 4), "K_ITHYUD", SKINCOLOR_DREAM},
	{224, 80+(216* 5), "K_ITBHOG", SKINCOLOR_TANGERINE},
	{224, 80+(216* 6), "K_ITSHRK", SKINCOLOR_JAWZ},
	{224, 80+(216* 7), "K_ITSHOE", SKINCOLOR_MINT},
	{224, 80+(216* 8), "K_ITGROW", SKINCOLOR_RUBY},
	{224, 80+(216* 9), "K_ITPOGO", SKINCOLOR_SAPPHIRE},
	{224, 80+(216*10), "K_ITRSHE", SKINCOLOR_YELLOW},
	{224, 80+(216*11), "K_ITORB4", SKINCOLOR_DUSK},
	{224, 80+(216*12), "K_ITEGGM", SKINCOLOR_GREEN},
	{224, 80+(216*13), "K_ITMINE", SKINCOLOR_BRONZE},
	{224, 80+(216*14), "K_ITTHNS", SKINCOLOR_RASPBERRY},
	{224, 80+(216*15), "K_ITINV1", SKINCOLOR_GREY},
	// This Tyler52 gag is troublesome
	// Alignment should be ((spaces+1 * 100) + (headers+1 * 38) + (lines * 15))
	// Current max image spacing: (216*17)
	{112, (16*100)+(19*38)+(103*15), "TYLER52", SKINCOLOR_NONE},
	{0, 0, NULL, SKINCOLOR_NONE}
};

void F_StartCredits(void)
{
	G_SetGamestate(GS_CREDITS);

	// Just in case they're open ... somehow
	M_ClearMenus(true);

	// Save the second we enter the credits
	if ((!modifiedgame || savemoddata) && !(netgame || multiplayer) && cursaveslot >= 0)
		G_SaveGame((UINT32)cursaveslot);

	if (creditscutscene)
	{
		F_StartCustomCutscene(creditscutscene - 1, false, false);
		return;
	}

	gameaction = ga_nothing;
	paused = false;
	CON_ToggleOff();
	S_StopMusic();

	S_ChangeMusicInternal("credit", false);
	S_ShowMusicCredit();

	finalecount = 0;
	animtimer = 0;
	timetonext = 2*TICRATE;
}

void F_CreditDrawer(void)
{
	UINT16 i;
	fixed_t y = (80<<FRACBITS) - 5*(animtimer<<FRACBITS)/8;

	V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);

	// Draw background
	V_DrawSciencePatch(0, 0 - FixedMul(32<<FRACBITS, FixedDiv(credbgtimer%TICRATE, TICRATE)), V_SNAPTOTOP, W_CachePatchName("CREDTILE", PU_CACHE), FRACUNIT);

	V_DrawSciencePatch(0, 0 - FixedMul(40<<FRACBITS, FixedDiv(credbgtimer%(TICRATE/2), (TICRATE/2))), V_SNAPTOTOP, W_CachePatchName("CREDZIGZ", PU_CACHE), FRACUNIT);
	V_DrawSciencePatch(320<<FRACBITS, 0 - FixedMul(40<<FRACBITS, FixedDiv(credbgtimer%(TICRATE/2), (TICRATE/2))), V_SNAPTOTOP|V_FLIP, W_CachePatchName("CREDZIGZ", PU_CACHE), FRACUNIT);

	// Draw pictures
	for (i = 0; credits_pics[i].patch; i++)
	{
		UINT8 *colormap = NULL;
		fixed_t sc = FRACUNIT>>1;

		if (credits_pics[i].colorize != SKINCOLOR_NONE)
		{
			colormap = R_GetTranslationColormap(TC_RAINBOW, credits_pics[i].colorize, GTC_MENUCACHE);
			sc = FRACUNIT; // quick hack so I don't have to add another field to credits_pics
		}

		V_DrawFixedPatch(credits_pics[i].x<<FRACBITS, (credits_pics[i].y<<FRACBITS) - 4*(animtimer<<FRACBITS)/5, sc, 0, W_CachePatchName(credits_pics[i].patch, PU_CACHE), colormap);
	}

	// Dim the background
	//V_DrawFadeScreen();

	// Draw credits text on top
	for (i = 0; credits[i]; i++)
	{
		switch(credits[i][0])
		{
		case 0:
			y += 80<<FRACBITS;
			break;
		case 1:
			if (y>>FRACBITS > -20)
				V_DrawCreditString((160 - (V_CreditStringWidth(&credits[i][1])>>1))<<FRACBITS, y, 0, &credits[i][1]);
			y += 30<<FRACBITS;
			break;
		default:
			if (y>>FRACBITS > -10)
				V_DrawStringAtFixed(32<<FRACBITS, y, V_ALLOWLOWERCASE, credits[i]);
			y += 12<<FRACBITS;
			break;
		}
		if (((y>>FRACBITS) * vid.dupy) > vid.height)
			break;
	}

	// RR isn't any time soon as of writing, but v1.4 is expected to be the last v1 release. Let's give it a proper send off.
	if (finalecount)
	{
		const char *goodbyefornow = "See you in ""\x82""Dr. Robotnik's Ring Racers""\x80""!";
		fixed_t lpad = ((vid.width/vid.dupx) - BASEVIDWIDTH)<<FRACBITS;
		fixed_t w = V_StringWidth(goodbyefornow, V_ALLOWLOWERCASE)<<FRACBITS;
		fixed_t x = FixedMul(((BASEVIDWIDTH<<FRACBITS)+w+lpad), ((finalecount-1)<<FRACBITS)/(5*TICRATE)) - w - (lpad/2);
		V_DrawString(x>>FRACBITS, y>>FRACBITS, V_ALLOWLOWERCASE, goodbyefornow); // for some reason DrawStringAtFixed can't tolerate colour codes
	}
}

void F_CreditTicker(void)
{
	// "Simulate" the drawing of the credits so that dedicated mode doesn't get stuck
	UINT16 i;
	fixed_t y = (80<<FRACBITS) - 5*(animtimer<<FRACBITS)/8;

	// Draw credits text on top
	for (i = 0; credits[i]; i++)
	{
		switch(credits[i][0])
		{
			case 0: y += 80<<FRACBITS; break;
			case 1: y += 30<<FRACBITS; break;
			default: y += 12<<FRACBITS; break;
		}
		if (FixedMul(y,vid.dupy) > vid.height)
			break;
	}

	// Do this here rather than in the drawer you doofus! (this is why dedicated mode broke at credits)
	if (!credits[i] && y <= 120<<FRACBITS && !finalecount)
	{
		timetonext = 5*TICRATE+1;
		finalecount = 5*TICRATE;
	}

	if (timetonext)
		timetonext--;
	else
		animtimer++;

	credbgtimer++;

	if (finalecount && --finalecount == 0)
		F_StartGameEvaluation();
}

boolean F_CreditResponder(event_t *event)
{
	INT32 key = event->data1;

	// remap virtual keys (mouse & joystick buttons)
	switch (key)
	{
		case KEY_MOUSE1:
			key = KEY_ENTER;
			break;
		case KEY_MOUSE1 + 1:
			key = KEY_BACKSPACE;
			break;
		case KEY_JOY1:
		case KEY_JOY1 + 2:
			key = KEY_ENTER;
			break;
		case KEY_JOY1 + 3:
			key = 'n';
			break;
		case KEY_JOY1 + 1:
			key = KEY_BACKSPACE;
			break;
		case KEY_HAT1:
			key = KEY_UPARROW;
			break;
		case KEY_HAT1 + 1:
			key = KEY_DOWNARROW;
			break;
		case KEY_HAT1 + 2:
			key = KEY_LEFTARROW;
			break;
		case KEY_HAT1 + 3:
			key = KEY_RIGHTARROW;
			break;
	}

	if (event->type != ev_keydown)
		return false;

	if (key == KEY_DOWNARROW || key == KEY_SPACE)
	{
		if (!timetonext && !finalecount)
			animtimer += 7;
		return false;
	}

	/*if (!(timesBeaten) && !(netgame || multiplayer))
		return false;*/

	if (key != KEY_ESCAPE && key != KEY_ENTER && key != KEY_BACKSPACE)
		return false;

	if (keypressed)
		return true;

	keypressed = true;
	return true;
}

// ============
//  EVALUATION
// ============
#define INTERVAL 50
#define TRANSLEVEL V_80TRANS
static INT32 eemeralds_start;
static boolean drawemblem = false, drawchaosemblem = false;

void F_StartGameEvaluation(void)
{
	// Credits option in secrets menu
	if (cursaveslot == -2)
	{
		F_StartGameEnd();
		return;
	}

	G_SetGamestate(GS_EVALUATION);

	// Just in case they're open ... somehow
	M_ClearMenus(true);

	// Save the second we enter the evaluation
	// We need to do this again!  Remember, it's possible a mod designed skipped
	// the credits sequence!
	if ((!modifiedgame || savemoddata) && !(netgame || multiplayer) && cursaveslot >= 0)
		G_SaveGame((UINT32)cursaveslot);

	gameaction = ga_nothing;
	paused = false;
	CON_ToggleOff();

	finalecount = 0;
}

void F_GameEvaluationDrawer(void)
{
	INT32 x, y, i;
	const fixed_t radius = 48*FRACUNIT;
	angle_t fa;
	INT32 eemeralds_cur;
	char patchname[7] = "CEMGx0";

	V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);

	// Draw all the good crap here.
	if (ALL7EMERALDS(emeralds))
		V_DrawString(114, 16, 0, "GOT THEM ALL!");
	else
		V_DrawString(124, 16, 0, "TRY AGAIN!");

	eemeralds_start++;
	eemeralds_cur = eemeralds_start;

	for (i = 0; i < 7; ++i)
	{
		fa = (FixedAngle(eemeralds_cur*FRACUNIT)>>ANGLETOFINESHIFT) & FINEMASK;
		x = 160 + FixedInt(FixedMul(FINECOSINE(fa),radius));
		y = 100 + FixedInt(FixedMul(FINESINE(fa),radius));

		patchname[4] = 'A'+(char)i;
		if (emeralds & (1<<i))
			V_DrawScaledPatch(x, y, 0, W_CachePatchName(patchname, PU_CACHE));
		else
			V_DrawTranslucentPatch(x, y, TRANSLEVEL, W_CachePatchName(patchname, PU_CACHE));

		eemeralds_cur += INTERVAL;
	}
	if (eemeralds_start >= 360)
		eemeralds_start -= 360;

	if (finalecount == 5*TICRATE)
	{
		if ((!modifiedgame || savemoddata) && !(netgame || multiplayer))
		{
			++timesBeaten;

			if (ALL7EMERALDS(emeralds))
				++timesBeatenWithEmeralds;

			/*if (ultimatemode)
				++timesBeatenUltimate;*/

			if (M_UpdateUnlockablesAndExtraEmblems(false))
				S_StartSound(NULL, sfx_ncitem);

			G_SaveGameData(false);
		}
	}

	if (finalecount >= 5*TICRATE)
	{
		if (drawemblem)
			V_DrawScaledPatch(120, 192, 0, W_CachePatchName("NWNGA0", PU_CACHE));

		if (drawchaosemblem)
			V_DrawScaledPatch(200, 192, 0, W_CachePatchName("NWNGA0", PU_CACHE));

		V_DrawString(8, 16, V_YELLOWMAP, "Unlocked:");

		if (!(netgame) && (!modifiedgame || savemoddata))
		{
			INT32 startcoord = 32;

			for (i = 0; i < MAXUNLOCKABLES; i++)
			{
				if (unlockables[i].conditionset && unlockables[i].conditionset < MAXCONDITIONSETS
					&& unlockables[i].type && !unlockables[i].nocecho)
				{
					if (unlockables[i].unlocked)
						V_DrawString(8, startcoord, 0, unlockables[i].name);
					startcoord += 8;
				}
			}
		}
		else if (netgame)
			V_DrawString(8, 96, V_YELLOWMAP, "Prizes only\nawarded in\nsingle player!");
		else
			V_DrawString(8, 96, V_YELLOWMAP, "Prizes not\nawarded in\nmodified games!");
	}
}

void F_GameEvaluationTicker(void)
{
	finalecount++;

	if (finalecount > 10*TICRATE)
		F_StartGameEnd();
}

// ==========
//  GAME END
// ==========
void F_StartGameEnd(void)
{
	G_SetGamestate(GS_GAMEEND);

	gameaction = ga_nothing;
	paused = false;
	CON_ToggleOff();
	S_StopMusic();

	// In case menus are still up?!!
	M_ClearMenus(true);

	timetonext = TICRATE;
}

//
// F_GameEndDrawer
//
void F_GameEndDrawer(void)
{
	// this function does nothing
}

//
// F_GameEndTicker
//
void F_GameEndTicker(void)
{
	if (timetonext > 0)
		timetonext--;
	else
		D_StartTitle();
}


// ==============
//  TITLE SCREEN
// ==============
void F_StartTitleScreen(void)
{
	if (gamestate != GS_TITLESCREEN && gamestate != GS_WAITINGPLAYERS)
		finalecount = 0;
	else
		wipegamestate = GS_TITLESCREEN;
	G_SetGamestate(GS_TITLESCREEN);
	CON_ClearHUD();

	// IWAD dependent stuff.

	// music is started in the ticker
	if (!demo.fromtitle) // SRB2Kart: Don't reset music if the right track is already playing
		S_StopMusic();
	demo.fromtitle = false;

	animtimer = 0;

	demoDelayLeft = demoDelayTime;
	demoIdleLeft = demoIdleTime;

	ttbanner = W_CachePatchName("TTKBANNR", PU_LEVEL);
	ttkart = W_CachePatchName("TTKART", PU_LEVEL);
	ttcheckers = W_CachePatchName("TTCHECK", PU_LEVEL);
	ttkflash = W_CachePatchName("TTKFLASH", PU_LEVEL);
}

// (no longer) De-Demo'd Title Screen
void F_TitleScreenDrawer(void)
{
	if (modeattacking)
		return; // We likely came here from retrying. Don't do a damn thing.

	// Don't draw outside of the title screen, or if the patch isn't there.
	if (!ttbanner || (gamestate != GS_TITLESCREEN && gamestate != GS_WAITINGPLAYERS))
	{
		F_SkyScroll(titlescrollspeed);
		return;
	}

	if (finalecount < 50)
	{
		V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);

		V_DrawSmallScaledPatch(84, 36, 0, ttbanner);

		if (finalecount >= 20)
			V_DrawSmallScaledPatch(84, 87, 0, ttkart);
		else if (finalecount >= 10)
			V_DrawSciencePatch((84<<FRACBITS) - FixedDiv(180<<FRACBITS, 10<<FRACBITS)*(20-finalecount), (87<<FRACBITS), 0, ttkart, FRACUNIT/2);
	}
	else if (finalecount < 52)
	{
		V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 120);
		V_DrawSmallScaledPatch(84, 36, 0, ttkflash);
	}
	else
	{
		INT32 transval = 0;

		if (finalecount <= (50+(9<<1)))
			transval = (finalecount - 50)>>1;

		F_SkyScroll(titlescrollspeed);

		V_DrawSciencePatch(0, 0 - FixedMul(40<<FRACBITS, FixedDiv(finalecount%70, 70)), V_SNAPTOTOP|V_SNAPTOLEFT, ttcheckers, FRACUNIT);
		V_DrawSciencePatch(280<<FRACBITS, -(40<<FRACBITS) + FixedMul(40<<FRACBITS, FixedDiv(finalecount%70, 70)), V_SNAPTOTOP|V_SNAPTORIGHT, ttcheckers, FRACUNIT);

		if (transval)
			V_DrawFadeScreen(120, 10 - transval);

		V_DrawSmallScaledPatch(84, 36, 0, ttbanner);

		V_DrawSmallScaledPatch(84, 87, 0, ttkart);

		if (!transval)
			return;

		V_DrawSmallScaledPatch(84, 36, transval<<V_ALPHASHIFT, ttkflash);
	}
}

// (no longer) De-Demo'd Title Screen
void F_TitleScreenTicker(boolean run)
{
	if (run)
	{
		finalecount++;

		if (finalecount == 10)
		{
			S_StartSound(NULL, sfx_s23e);
		}
		else if (finalecount == 50)
		{
			// Now start the music
			S_ChangeMusicInternal("titles", looptitle);
			S_StartSound(NULL, sfx_s23c);
		}
	}

	// don't trigger if doing anything besides idling on title
	if (gameaction != ga_nothing || gamestate != GS_TITLESCREEN)
		return;

	// are demos disabled?
	if (!cv_rollingdemos.value)
		return;

	// Wait for a while (for the music to finish, preferably)
	// before starting demos
	if (demoDelayLeft)
	{
		--demoDelayLeft;
		return;
	}

	// Hold up for a bit if menu or console active
	if (menuactive || CON_Ready())
	{
		demoIdleLeft = demoIdleTime;
		return;
	}

	// is it time?
	if (!(--demoIdleLeft))
	{
		//static boolean use_netreplay = false;

		char dname[9];
		lumpnum_t l;
		const char *mapname;
		UINT8 numstaff;

		//@TODO uncomment this when this goes into vanilla
		/*if ((use_netreplay = !use_netreplay))*/
		{
			numstaff = 1;
			while ((l = W_CheckNumForName(va("TDEMO%03u", numstaff))) != LUMPERROR)
				numstaff++;
			numstaff--;

			if (numstaff)
			{
				numstaff = M_RandomKey(numstaff)+1;
				snprintf(dname, 9, "TDEMO%03u", numstaff);
				goto loadreplay;
			}
		}

		// prevent console spam if failed
		demoIdleLeft = demoIdleTime;

		if ((l = W_CheckNumForName("MAP01S01")) == LUMPERROR) // gotta have ONE
		{
			F_StartIntro();
			return;
		}

		// Replay intro when done cycling through demos
		/*if (curDemo == numDemos) -- uuuh... we have a LOT of maps AND a big devteam... probably not gonna see a repeat unless you're super unlucky :V
		{
			curDemo = 0;
			F_StartIntro();
			return;
		}*/

		mapname = G_BuildMapName(G_RandMap(TOL_RACE, -2, false, 0, false, NULL)+1);

		numstaff = 1;
		while (numstaff < 99 && (l = W_CheckNumForName(va("%sS%02u",mapname,numstaff+1))) != LUMPERROR)
			numstaff++;

#if 0 // turns out this isn't how we're gonna organise 'em
		if (numstaff > 1)
		{
			if (laststaff && laststaff <= numstaff) // don't do the same staff member twice in a row, even if they're on different maps
			{
				numstaff = M_RandomKey(numstaff-1)+1;
				if (numstaff >= laststaff)
					numstaff++;
			}
			else
				numstaff = M_RandomKey(numstaff)+1;
		}
		laststaff = numstaff;
#else
		numstaff = M_RandomKey(numstaff)+1;
#endif

		// Setup demo name
		snprintf(dname, 9, "%sS%02u", mapname, numstaff);

		/*if ((l = W_CheckNumForName(dname)) == LUMPERROR) -- we KNOW it exists now
		{
			CONS_Alert(CONS_ERROR, M_GetText("Demo lump \"%s\" doesn't exist\n"), dname);
			F_StartIntro();
			return;
		}*/

loadreplay:
		demo.title = demo.fromtitle = true;
		demo.ignorefiles = true;
		demo.loadfiles = false;
		G_DoPlayDemo(dname);
	}
}

void F_TitleDemoTicker(void)
{
	keypressed = false;
}

// ================
//  WAITINGPLAYERS
// ================

void F_StartWaitingPlayers(void)
{
	INT32 i;
	INT32 randskin;
	spriteframe_t *sprframe;

	wipegamestate = GS_TITLESCREEN; // technically wiping from title screen
	finalecount = 0;

	randskin = M_RandomKey(numskins);

	if (waitcolormap)
		Z_Free(waitcolormap);

	waitcolormap = R_GetTranslationColormap(randskin, skins[randskin].prefcolor, 0);

	for (i = 0; i < 2; i++)
	{
		sprframe = &skins[randskin].spritedef.spriteframes[(6+(i*3)) & FF_FRAMEMASK];
		driver[i] = W_CachePatchNum(sprframe->lumppat[1], PU_LEVEL);
	}
}

void F_WaitingPlayersTicker(void)
{
	if (paused)
		return;

	finalecount++;

	// dumb hack, only start the music on the 1st tick so if you instantly go into the map you aren't hearing a tic of music
	if (finalecount == 2)
		S_ChangeMusicInternal("WAIT2J", true);
}

void F_WaitingPlayersDrawer(void)
{
	UINT32 frame = (finalecount % 8) / 4; // The game only tics every other frame while waitingplayers
	INT32 flags = V_FLIP;
	const char *waittext1 = "You will join";
	const char *waittext2 = "the next race...";
	V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);
	V_DrawCreditString((160 - (V_CreditStringWidth(waittext1)>>1))<<FRACBITS, 48<<FRACBITS, 0, waittext1);
	V_DrawCreditString((160 - (V_CreditStringWidth(waittext2)>>1))<<FRACBITS, 64<<FRACBITS, 0, waittext2);
	V_DrawFixedPatch((160<<FRACBITS) - driver[frame]->width / 2, 150<<FRACBITS, 1<<FRACBITS, flags, driver[frame], waitcolormap);
}

// ==========
//  CONTINUE
// ==========
void F_StartContinue(void)
{
	I_Assert(!netgame && !multiplayer);

	if (players[consoleplayer].continues <= 0)
	{
		Command_ExitGame_f();
		return;
	}

	G_SetGamestate(GS_CONTINUING);
	gameaction = ga_nothing;

	keypressed = false;
	paused = false;
	CON_ToggleOff();

	// In case menus are still up?!!
	M_ClearMenus(true);

	S_ChangeMusicInternal("contsc", false);
	S_StopSounds();

	timetonext = TICRATE*11;
}

//
// F_ContinueDrawer
// Moved continuing out of the HUD (hack removal!!)
//
void F_ContinueDrawer(void)
{
	patch_t *contsonic;
	INT32 i, x = (BASEVIDWIDTH/2) + 4, ncontinues = players[consoleplayer].continues;
	if (ncontinues > 20)
		ncontinues = 20;

	if (imcontinuing)
		contsonic = W_CachePatchName("CONT2", PU_CACHE);
	else
		contsonic = W_CachePatchName("CONT1", PU_CACHE);

	V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);
	V_DrawCenteredString(BASEVIDWIDTH/2, 100, 0, "CONTINUE?");

	// Draw a Sonic!
	V_DrawScaledPatch((BASEVIDWIDTH - SHORT(contsonic->width))/2, 32, 0, contsonic);

	// Draw the continue markers! Show continues minus one.
	x -= ncontinues * 6;
	for (i = 0; i < ncontinues; ++i)
		V_DrawContinueIcon(x + (i*12), 140, 0, players[consoleplayer].skin, players[consoleplayer].skincolor);

	V_DrawCenteredString(BASEVIDWIDTH/2, 168, 0, va("\x82*\x80" " %02d " "\x82*\x80", timetonext/TICRATE));
}

void F_ContinueTicker(void)
{
	if (!imcontinuing)
	{
		// note the setup to prevent 2x reloading
		if (timetonext >= 0)
			timetonext--;
		if (timetonext == 0)
			Command_ExitGame_f();
	}
	else
	{
		// note the setup to prevent 2x reloading
		if (continuetime >= 0)
			continuetime--;
		if (continuetime == 0)
			G_Continue();
	}
}

boolean F_ContinueResponder(event_t *event)
{
	INT32 key = event->data1;

	if (keypressed)
		return true;

	if (timetonext >= 21*TICRATE/2)
		return false;
	if (event->type != ev_keydown)
		return false;

	// remap virtual keys (mouse & joystick buttons)
	switch (key)
	{
		case KEY_ENTER:
		case KEY_SPACE:
		case KEY_MOUSE1:
		case KEY_JOY1:
		case KEY_JOY1 + 2:
			break;
		default:
			return false;
	}

	keypressed = true;
	imcontinuing = true;
	continuetime = TICRATE;
	S_StartSound(0, sfx_itemup);
	return true;
}

// ==================
//  CUSTOM CUTSCENES
// ==================
static INT32 scenenum, cutnum;
static INT32 picxpos, picypos, picnum, pictime;
static INT32 textxpos, textypos;
static boolean dofadenow = false, cutsceneover = false;
static boolean runningprecutscene = false, precutresetplayer = false;

static void F_AdvanceToNextScene(void)
{
	if (rendermode != render_none)
	{
		F_WipeStartScreen();

		// Fade to any palette color you want.
		if (cutscenes[cutnum]->scene[scenenum].fadecolor)
		{
			V_DrawFill(0,0,BASEVIDWIDTH,BASEVIDHEIGHT,cutscenes[cutnum]->scene[scenenum].fadecolor);

			F_WipeEndScreen();
			F_RunWipe(cutscenes[cutnum]->scene[scenenum].fadeinid, true);

			F_WipeStartScreen();
		}
	}

	// Don't increment until after endcutscene check
	// (possible overflow / bad patch names from the one tic drawn before the fade)
	if (scenenum+1 >= cutscenes[cutnum]->numscenes)
	{
		F_EndCutScene();
		return;
	}

	++scenenum;

	timetonext = 0;
	stoptimer = 0;
	picnum = 0;
	picxpos = cutscenes[cutnum]->scene[scenenum].xcoord[picnum];
	picypos = cutscenes[cutnum]->scene[scenenum].ycoord[picnum];

	if (cutscenes[cutnum]->scene[scenenum].musswitch[0])
		S_ChangeMusicEx(cutscenes[cutnum]->scene[scenenum].musswitch,
			cutscenes[cutnum]->scene[scenenum].musswitchflags,
			cutscenes[cutnum]->scene[scenenum].musicloop,
			cutscenes[cutnum]->scene[scenenum].musswitchposition, 0, 0);

	// Fade to the next
	F_NewCutscene(cutscenes[cutnum]->scene[scenenum].text);

	picnum = 0;
	picxpos = cutscenes[cutnum]->scene[scenenum].xcoord[picnum];
	picypos = cutscenes[cutnum]->scene[scenenum].ycoord[picnum];
	textxpos = cutscenes[cutnum]->scene[scenenum].textxpos;
	textypos = cutscenes[cutnum]->scene[scenenum].textypos;

	animtimer = pictime = cutscenes[cutnum]->scene[scenenum].picduration[picnum];

	if (rendermode != render_none)
	{
		F_CutsceneDrawer();

		F_WipeEndScreen();
		F_RunWipe(cutscenes[cutnum]->scene[scenenum].fadeoutid, true);
	}
}

void F_EndCutScene(void)
{
	cutsceneover = true; // do this first, just in case G_EndGame or something wants to turn it back false later
	if (runningprecutscene)
	{
		if (server)
			D_MapChange(gamemap, gametype, false, precutresetplayer, 0, true, false);
	}
	else
	{
		if (cutnum == creditscutscene-1)
			F_StartGameEvaluation();
		else if (cutnum == introtoplay-1)
			D_StartTitle();
		else if (nextmap < 1100-1)
			G_NextLevel();
		else
			G_EndGame();
	}
}

void F_StartCustomCutscene(INT32 cutscenenum, boolean precutscene, boolean resetplayer)
{
	if (!cutscenes[cutscenenum])
		return;

	G_SetGamestate(GS_CUTSCENE);

	gameaction = ga_nothing;
	paused = false;
	CON_ToggleOff();

	F_NewCutscene(cutscenes[cutscenenum]->scene[0].text);

	cutsceneover = false;
	runningprecutscene = precutscene;
	precutresetplayer = resetplayer;

	scenenum = picnum = 0;
	cutnum = cutscenenum;
	picxpos = cutscenes[cutnum]->scene[0].xcoord[0];
	picypos = cutscenes[cutnum]->scene[0].ycoord[0];
	textxpos = cutscenes[cutnum]->scene[0].textxpos;
	textypos = cutscenes[cutnum]->scene[0].textypos;

	pictime = cutscenes[cutnum]->scene[0].picduration[0];

	keypressed = false;
	finalecount = 0;
	timetonext = 0;
	animtimer = cutscenes[cutnum]->scene[0].picduration[0]; // Picture duration
	stoptimer = 0;

	if (cutscenes[cutnum]->scene[0].musswitch[0])
		S_ChangeMusicEx(cutscenes[cutnum]->scene[0].musswitch,
			cutscenes[cutnum]->scene[0].musswitchflags,
			cutscenes[cutnum]->scene[0].musicloop,
			cutscenes[cutnum]->scene[scenenum].musswitchposition, 0, 0);
	else
		S_StopMusic();
}

//
// F_CutsceneDrawer
//
void F_CutsceneDrawer(void)
{
	if (dofadenow && rendermode != render_none)
	{
		F_WipeStartScreen();

		// Fade to any palette color you want.
		if (cutscenes[cutnum]->scene[scenenum].fadecolor)
		{
			V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, cutscenes[cutnum]->scene[scenenum].fadecolor);

			F_WipeEndScreen();
			F_RunWipe(cutscenes[cutnum]->scene[scenenum].fadeinid, true);

			F_WipeStartScreen();
		}
	}
	V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);

	if (cutscenes[cutnum]->scene[scenenum].picname[picnum][0] != '\0')
	{
		if (cutscenes[cutnum]->scene[scenenum].pichires[picnum])
			V_DrawSmallScaledPatch(picxpos, picypos, 0,
				W_CachePatchName(cutscenes[cutnum]->scene[scenenum].picname[picnum], PU_CACHE));
		else
			V_DrawScaledPatch(picxpos,picypos, 0,
				W_CachePatchName(cutscenes[cutnum]->scene[scenenum].picname[picnum], PU_CACHE));
	}

	if (dofadenow && rendermode != render_none)
	{
		F_WipeEndScreen();
		F_RunWipe(cutscenes[cutnum]->scene[scenenum].fadeoutid, true);
	}

	V_DrawString(textxpos, textypos, 0, cutscene_disptext);
}

void F_CutsceneTicker(void)
{
	INT32 i;

	// Online clients tend not to instantly get the map change, so just wait
	// and don't send 30 of them.
	if (cutsceneover)
		return;

	// advance animation
	finalecount++;
	cutscene_boostspeed = 0;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (netgame && i != serverplayer && !IsPlayerAdmin(i))
			continue;

		if (players[i].cmd.buttons & BT_BRAKE || players[i].cmd.buttons & BT_ACCELERATE) // SRB2kart
		{
			keypressed = false;
			cutscene_boostspeed = 1;
			if (timetonext)
				timetonext = 2;
		}
	}

	if (animtimer)
	{
		animtimer--;
		if (animtimer <= 0)
		{
			if (picnum < 7 && cutscenes[cutnum]->scene[scenenum].picname[picnum+1][0] != '\0')
			{
				picnum++;
				picxpos = cutscenes[cutnum]->scene[scenenum].xcoord[picnum];
				picypos = cutscenes[cutnum]->scene[scenenum].ycoord[picnum];
				pictime = cutscenes[cutnum]->scene[scenenum].picduration[picnum];
				animtimer = pictime;
			}
			else
				timetonext = 2;
		}
	}

	if (timetonext)
		--timetonext;

	if (++stoptimer > 2 && timetonext == 1)
		F_AdvanceToNextScene();
	else if (!timetonext && !F_WriteText())
		timetonext = 5*TICRATE + 1;
}

boolean F_CutsceneResponder(event_t *event)
{
	if (cutnum == introtoplay-1)
		return F_IntroResponder(event);

	return false;
}
