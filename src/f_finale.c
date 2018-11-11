// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2016 by Sonic Team Junior.
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
#include "i_video.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"
#include "i_system.h"
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
// F_DrawPatchCol
//
static void F_DrawPatchCol(INT32 x, patch_t *patch, INT32 col)
{
	const column_t *column;
	const UINT8 *source;
	UINT8 *desttop, *dest = NULL;
	const UINT8 *deststop, *destbottom;
	size_t count;

	desttop = screens[0] + x*vid.dupx;
	deststop = screens[0] + vid.rowbytes * vid.height;
	destbottom = desttop + vid.height*vid.width;

	do {
		INT32 topdelta, prevdelta = -1;
		column = (column_t *)((UINT8 *)patch + LONG(patch->columnofs[col]));

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			topdelta = column->topdelta;
			if (topdelta <= prevdelta)
				topdelta += prevdelta;
			prevdelta = topdelta;
			source = (const UINT8 *)column + 3;
			dest = desttop + topdelta*vid.width;
			count = column->length;

			while (count--)
			{
				INT32 dupycount = vid.dupy;

				while (dupycount-- && dest < destbottom)
				{
					INT32 dupxcount = vid.dupx;
					while (dupxcount-- && dest <= deststop)
						*dest++ = *source;

					dest += (vid.width - vid.dupx);
				}
				source++;
			}
			column = (const column_t *)((const UINT8 *)column + column->length + 4);
		}

		desttop += SHORT(patch->height)*vid.dupy*vid.width;
	} while(dest < destbottom);
}

//
// F_SkyScroll
//
static void F_SkyScroll(INT32 scrollspeed)
{
	INT32 scrolled, x, mx, fakedwidth;
	patch_t *pat;

	pat = W_CachePatchName("TITLESKY", PU_CACHE);

	animtimer = ((finalecount*scrollspeed)/16) % SHORT(pat->width);

	fakedwidth = vid.width / vid.dupx;

	if (rendermode == render_soft)
	{ // if only hardware rendering could be this elegant and complete
		scrolled = (SHORT(pat->width) - animtimer) - 1;
		for (x = 0, mx = scrolled; x < fakedwidth; x++, mx = (mx+1)%SHORT(pat->width))
			F_DrawPatchCol(x, pat, mx);
	}
#ifdef HWRENDER
	else if (rendermode != render_none)
	{ // if only software rendering could be this simple and retarded
		INT32 dupz = (vid.dupx < vid.dupy ? vid.dupx : vid.dupy);
		INT32 y, pw = SHORT(pat->width) * dupz, ph = SHORT(pat->height) * dupz;
		scrolled = animtimer * dupz;
		for (x = 0; x < vid.width; x += pw)
		{
			for (y = 0; y < vid.height; y += ph)
			{
				if (scrolled > 0)
					V_DrawScaledPatch(scrolled - pw, y, V_NOSCALESTART, pat);

				V_DrawScaledPatch(x + scrolled, y, V_NOSCALESTART, pat);
			}
		}
	}
#endif

	W_UnlockCachedPatch(pat);
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
	CON_ClearHUD();
	F_NewCutscene(introtext[0]);

	intro_scenenum = 0;
	finalecount = animtimer = stoptimer = 0;
	roidtics = BASEVIDWIDTH - 64;
	timetonext = introscenetime[intro_scenenum];
	S_StopMusic();
}

//
// F_IntroDrawScene
//
static void F_IntroDrawScene(void)
{
	boolean highres = false;
	INT32 cx = 8, cy = 128;
	patch_t *background = NULL;
	INT32 bgxoffs = 0;

	// DRAW A FULL PIC INSTEAD OF FLAT!
	if (intro_scenenum == 0)
	{
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

	if (animtimer)
		animtimer--;

	V_DrawString(cx, cy, 0, cutscene_disptext);
}

//
// F_IntroDrawer
//
void F_IntroDrawer(void)
{
	if (timetonext <= 0)
	{
		if (intro_scenenum == 0)
		{
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
					I_OsPolling();
					I_UpdateNoBlit();
					M_Drawer(); // menu is drawn even on top of wipes
					I_FinishUpdate(); // Update the screen with the image Tails 06-19-2001
				}
			}

			D_StartTitle();
			// Yes, this is a weird hack, we need to force a wipe for this because the game state has changed in the middle of where it would normally wipe
			// Need to set the wipe start and then draw the first frame of the title screen to get it working
			F_WipeStartScreen();
			F_TitleScreenDrawer();
			wipegamestate = -1; // force a wipe
			return;
		}

		F_NewCutscene(introtext[++intro_scenenum]);
		timetonext = introscenetime[intro_scenenum];

		F_WipeStartScreen();
		wipegamestate = -1;
		animtimer = stoptimer = 0;
	}

	intro_curtime = introscenetime[intro_scenenum] - timetonext;

	F_IntroDrawScene();
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
	"\1SRB2 Kart",
	"\1Credits",
	"",
	"\1Game Design",
	"\"Chaos Zero 64\"",
	"\"Iceman404\" aka \"VelocitOni\"",
	"\"ZarroTsu\"",
	"",
	"\1Programming",
	"\"Chaos Zero 64\"",
	"Sally \"TehRealSalt\" Cochenour",
	"Vivian \"toaster\" Grannell",
	"\"Lat\'\"",
	"\"Monster Iestyn\"",
	"Sean \"Sryder\" Ryder",
	"Ehab \"wolfs\" Saeed",
	"\"ZarroTsu\"",
	"",
	"\1Artists",
	"\"Chaos Zero 64\"",
	"Sally \"TehRealSalt\" Cochenour",
	"Desmond \"Blade\" DesJardins",
	"Sherman \"CoatRack\" DesJardin",
	"Wesley \"Charyb\" Gillebaard",
	"James \"SeventhSentinel\" Hall",
	"\"Iceman404\"",
	"\"MotorRoach\"",
	"\"VAdaPEGA\"",
	"\"ZarroTsu\"",
	"",
	"\1Music and Sound",
	"Karl Brueggemann",
	"Wesley \"Charyb\" Gillebaard",
	"James \"SeventhSentinel\" Hall",
	"\"MaxieDaMan\"",
	"",
	"\1Level Design",
	"\"Blitz-T\"",
	"\"D00D64-X\"",
	"\"Chaos Zero 64\"",
	"Paul \"Boinciel\" Clempson",
	"Sally \"TehRealSalt\" Cochenour",
	"Desmond \"Blade\" DesJardins",
	"Sherman \"CoatRack\" DesJardin",
	"James \"SeventhSentinel\" Hall",
	"Sean \"Sryder\" Ryder",
	"\"Ryuspark\"",
	"Jeffery \"Chromatian\" Scott",
	"\"Simsmagic\"",
	"\"Tyrannosaur Chao\" aka \"Chaotic Chao\"",
	"\"ZarroTsu\"",
	"",
	"\1Testing",
	"\"CyberIF\"",
	"\"Dani\"",
	"Karol \"Fooruman\" D""\x1E""browski", // Dąbrowski, <Sryder> accents in srb2 :ytho:
	"Jesse \"Jeck Jims\" Emerick",
	"\"VirtAnderson\"",
	"",
	"\1Special Thanks",
	"Sonic Team Jr. & SRB2 (www.srb2.org)",
	"Bandit \"Bobby\" Cochenour", // i <3 my dog
	"Bear", // i <3 MY dog too
	"\"Chrispy\"",
	"\"DirkTheHusky\"",
	"\"fickle\"", // and my sharki
	"\"Nev3r\"",
	"\"Ritz\"",
	"\"Spherallic\"",
	"",
	"\1Produced By",
	"Kart Krew",
	"",
	"\1In Memory of",
	"\"Tyler52\"",
	"",
	"\1Thank you",
	"\1for playing!",
	NULL
};

static struct {
	UINT32 x, y;
	const char *patch;
} credits_pics[] = {
	/*{  8, 80+200* 1, "CREDIT01"},
	{  4, 80+200* 2, "CREDIT13"},
	{250, 80+200* 3, "CREDIT12"},
	{  8, 80+200* 4, "CREDIT03"},
	{248, 80+200* 5, "CREDIT11"},
	{  8, 80+200* 6, "CREDIT04"},
	{112, 80+200* 7, "CREDIT10"},
	{240, 80+200* 8, "CREDIT05"},
	{120, 80+200* 9, "CREDIT06"},*/
	{112, 80+100+200*10, "TYLER52"},
	{0, 0, NULL}
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
	CON_ClearHUD();
	S_StopMusic();

	S_ChangeMusicInternal("credit", false);

	finalecount = 0;
	animtimer = 0;
	timetonext = 2*TICRATE;
}

void F_CreditDrawer(void)
{
	UINT16 i;
	fixed_t y = (80<<FRACBITS) - 5*(animtimer<<FRACBITS)/8;

	//V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);

	// Draw background
	V_DrawSciencePatch(0, 0 - FixedMul(32<<FRACBITS, FixedDiv(credbgtimer%TICRATE, TICRATE)), V_SNAPTOTOP, W_CachePatchName("CREDTILE", PU_CACHE), FRACUNIT);

	V_DrawSciencePatch(0, 0 - FixedMul(40<<FRACBITS, FixedDiv(credbgtimer%(TICRATE/2), (TICRATE/2))), V_SNAPTOTOP, W_CachePatchName("CREDZIGZ", PU_CACHE), FRACUNIT);
	V_DrawSciencePatch(320<<FRACBITS, 0 - FixedMul(40<<FRACBITS, FixedDiv(credbgtimer%(TICRATE/2), (TICRATE/2))), V_SNAPTOTOP|V_FLIP, W_CachePatchName("CREDZIGZ", PU_CACHE), FRACUNIT);

	// Draw pictures
	for (i = 0; credits_pics[i].patch; i++)
		V_DrawSciencePatch(credits_pics[i].x<<FRACBITS, (credits_pics[i].y<<FRACBITS) - 4*(animtimer<<FRACBITS)/5, 0, W_CachePatchName(credits_pics[i].patch, PU_CACHE), FRACUNIT>>1);

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

	if (!credits[i] && y <= 120<<FRACBITS && !finalecount)
	{
		timetonext = 5*TICRATE+1;
		finalecount = 5*TICRATE;
	}
}

void F_CreditTicker(void)
{
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
	CON_ClearHUD();

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
	CON_ClearHUD();
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
	S_StopMusic();

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
			S_StartSound(NULL, sfx_spin);
		}
		else if (finalecount == 50)
		{
			// Now start the music
			S_ChangeMusicInternal("titles", looptitle);
			S_StartSound(NULL, sfx_zoom);
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
		char dname[9];
		lumpnum_t l;
		const char *mapname;
		UINT8 numstaff;

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

		mapname = G_BuildMapName(G_RandMap(TOL_RACE, -2, false, false, 0, false)+1);

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

		titledemo = true;
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
	waitcolormap = R_GetTranslationColormap(randskin, skins[randskin].prefcolor, 0);

	for (i = 0; i < 2; i++)
	{
		sprframe = &skins[randskin].spritedef.spriteframes[(6+i) & FF_FRAMEMASK];
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
	CON_ClearHUD();

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
		S_ChangeMusic(cutscenes[cutnum]->scene[scenenum].musswitch,
			cutscenes[cutnum]->scene[scenenum].musswitchflags,
			cutscenes[cutnum]->scene[scenenum].musicloop);

	// Fade to the next
	dofadenow = true;
	F_NewCutscene(cutscenes[cutnum]->scene[scenenum].text);

	picnum = 0;
	picxpos = cutscenes[cutnum]->scene[scenenum].xcoord[picnum];
	picypos = cutscenes[cutnum]->scene[scenenum].ycoord[picnum];
	textxpos = cutscenes[cutnum]->scene[scenenum].textxpos;
	textypos = cutscenes[cutnum]->scene[scenenum].textypos;

	animtimer = pictime = cutscenes[cutnum]->scene[scenenum].picduration[picnum];
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

	CON_ClearHUD();

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
		S_ChangeMusic(cutscenes[cutnum]->scene[0].musswitch,
			cutscenes[cutnum]->scene[0].musswitchflags,
			cutscenes[cutnum]->scene[0].musicloop);
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

	dofadenow = false;

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
