// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2011-2016 by Matthew "Inuyasha" Walsh.
// Copyright (C) 1999-2018 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  m_menu.c
/// \brief XMOD's extremely revamped menu system.

#ifdef __GNUC__
#include <unistd.h>
#endif

#include "m_menu.h"

#include "doomdef.h"
#include "d_main.h"
#include "d_netcmd.h"
#include "console.h"
#include "r_local.h"
#include "hu_stuff.h"
#include "g_game.h"
#include "g_input.h"
#include "m_argv.h"

// Data.
#include "sounds.h"
#include "s_sound.h"
#include "i_system.h"
#include "i_threads.h"

// Addfile
#include "filesrch.h"

#include "v_video.h"
#include "i_video.h"
#include "keys.h"
#include "z_zone.h"
#include "w_wad.h"
#include "p_local.h"
#include "p_setup.h"
#include "f_finale.h"

#ifdef HWRENDER
#include "hardware/hw_main.h"
#endif

#include "d_net.h"
#include "mserv.h"
#include "m_misc.h"
#include "m_anigif.h"
#include "byteptr.h"
#include "st_stuff.h"
#include "i_sound.h"
#include "k_kart.h" // SRB2kart
#include "d_player.h" // KITEM_ constants

#include "i_joy.h" // for joystick menu controls

// Condition Sets
#include "m_cond.h"

// And just some randomness for the exits.
#include "m_random.h"

#if defined(HAVE_SDL)
#include "SDL.h"
#if SDL_VERSION_ATLEAST(2,0,0)
#include "sdl/sdlmain.h" // JOYSTICK_HOTPLUG
#endif
#endif

#ifdef PC_DOS
#include <stdio.h> // for snprintf
int	snprintf(char *str, size_t n, const char *fmt, ...);
//int	vsnprintf(char *str, size_t n, const char *fmt, va_list ap);
#endif

#ifdef HAVE_DISCORDRPC
//#include "discord_rpc.h"
#include "discord.h"
#endif

#define SKULLXOFF -32
#define LINEHEIGHT 16
#define STRINGHEIGHT 8
#define FONTBHEIGHT 20
#define SMALLLINEHEIGHT 8
#define SLIDER_RANGE 10
#define SLIDER_WIDTH (8*SLIDER_RANGE+6)
#define SERVERS_PER_PAGE 11

typedef enum
{
	QUITMSG = 0,
	QUITMSG1,
	QUITMSG2,
	QUITMSG3,
	QUITMSG4,
	QUITMSG5,
	QUITMSG6,
	QUITMSG7,

	QUIT2MSG,
	QUIT2MSG1,
	QUIT2MSG2,
	QUIT2MSG3,
	QUIT2MSG4,
	QUIT2MSG5,
	QUIT2MSG6,

	QUIT3MSG,
	QUIT3MSG1,
	QUIT3MSG2,
	QUIT3MSG3,
	QUIT3MSG4,
	QUIT3MSG5,
	QUIT3MSG6,
	NUM_QUITMESSAGES
} text_enum;

#ifdef HAVE_THREADS
I_mutex m_menu_mutex;
#endif

M_waiting_mode_t m_waiting_mode = M_NOT_WAITING;

const char *quitmsg[NUM_QUITMESSAGES];

// Stuff for customizing the player select screen Tails 09-22-2003
description_t description[MAXSKINS];

//static char *char_notes = NULL;
//static fixed_t char_scroll = 0;

boolean menuactive = false;
boolean fromlevelselect = false;

typedef enum
{
	LLM_CREATESERVER,
	LLM_LEVELSELECT,
	LLM_RECORDATTACK,
	LLM_NIGHTSATTACK
} levellist_mode_t;

levellist_mode_t levellistmode = LLM_CREATESERVER;
UINT8 maplistoption = 0;

static char joystickInfo[8][29];
#ifndef NONET
static UINT32 serverlistpage;
#endif

//static saveinfo_t savegameinfo[MAXSAVEGAMES]; // Extra info about the save games.

INT16 startmap; // Mario, NiGHTS, or just a plain old normal game?

static INT16 itemOn = 1; // menu item skull is on, Hack by Tails 09-18-2002
static INT16 skullAnimCounter = 10; // skull animation counter

static  UINT8 setupcontrolplayer;
static  INT32   (*setupcontrols)[2];  // pointer to the gamecontrols of the player being edited

// shhh... what am I doing... nooooo!
static INT32 vidm_testingmode = 0;
static INT32 vidm_previousmode;
static INT32 vidm_selected = 0;
static INT32 vidm_nummodes;
static INT32 vidm_column_size;

//
// PROTOTYPES
//

static void M_StopMessage(INT32 choice);

#ifndef NONET
static void M_HandleServerPage(INT32 choice);
#endif

// Prototyping is fun, innit?
// ==========================================================================
// NEEDED FUNCTION PROTOTYPES GO HERE
// ==========================================================================

void M_SetWaitingMode(int mode);
int  M_GetWaitingMode(void);

// the haxor message menu
menu_t MessageDef;

#ifdef HAVE_DISCORDRPC
menu_t MISC_DiscordRequestsDef;
static void M_HandleDiscordRequests(INT32 choice);
static void M_DrawDiscordRequests(void);
#endif

menu_t SPauseDef;

#define lsheadingheight 16

// Sky Room
//static void M_CustomLevelSelect(INT32 choice);
//static void M_CustomWarp(INT32 choice);
FUNCNORETURN static ATTRNORETURN void M_UltimateCheat(INT32 choice);
//static void M_LoadGameLevelSelect(INT32 choice);
static void M_GetAllEmeralds(INT32 choice);
static void M_DestroyRobots(INT32 choice);
//static void M_LevelSelectWarp(INT32 choice);
static void M_Credits(INT32 choice);
static void M_PandorasBox(INT32 choice);
static void M_EmblemHints(INT32 choice);
static char *M_GetConditionString(condition_t cond);
menu_t SR_MainDef, SR_UnlockChecklistDef;

// Misc. Main Menu
#if 0 // Bring this back when we have actual single-player
static void M_SinglePlayerMenu(INT32 choice);
#endif
static void M_Options(INT32 choice);
static void M_Manual(INT32 choice);
static void M_SelectableClearMenus(INT32 choice);
static void M_Retry(INT32 choice);
static void M_EndGame(INT32 choice);
static void M_MapChange(INT32 choice);
static void M_ChangeLevel(INT32 choice);
static void M_ConfirmSpectate(INT32 choice);
static void M_ConfirmEnterGame(INT32 choice);
static void M_ConfirmTeamScramble(INT32 choice);
static void M_ConfirmTeamChange(INT32 choice);
static void M_ConfirmSpectateChange(INT32 choice);
//static void M_SecretsMenu(INT32 choice);
//static void M_SetupChoosePlayer(INT32 choice);
static void M_QuitSRB2(INT32 choice);
menu_t SP_MainDef, MP_MainDef, OP_MainDef;
menu_t MISC_ScrambleTeamDef, MISC_ChangeTeamDef, MISC_ChangeSpectateDef;

// Single Player
//static void M_LoadGame(INT32 choice);
static void M_TimeAttack(INT32 choice);
static boolean M_QuitTimeAttackMenu(void);
//static void M_NightsAttack(INT32 choice);
static void M_Statistics(INT32 choice);
static void M_HandleStaffReplay(INT32 choice);
static void M_ReplayTimeAttack(INT32 choice);
static void M_ChooseTimeAttack(INT32 choice);
//static void M_ChooseNightsAttack(INT32 choice);
static void M_ModeAttackEndGame(INT32 choice);
static void M_SetGuestReplay(INT32 choice);
//static void M_ChoosePlayer(INT32 choice);
menu_t SP_LevelStatsDef;
static menu_t SP_TimeAttackDef, SP_ReplayDef, SP_GuestReplayDef, SP_GhostDef;
//static menu_t SP_NightsAttackDef, SP_NightsReplayDef, SP_NightsGuestReplayDef, SP_NightsGhostDef;

// Multiplayer
#ifndef NONET
static void M_StartServerMenu(INT32 choice);
static void M_ConnectMenu(INT32 choice);
static void M_ConnectMenuModChecks(INT32 choice);
static void M_Refresh(INT32 choice);
static void M_Connect(INT32 choice);
#endif
static void M_StartOfflineServerMenu(INT32 choice);
static void M_StartServer(INT32 choice);
static void M_SetupMultiPlayer(INT32 choice);
static void M_SetupMultiPlayer2(INT32 choice);
static void M_SetupMultiPlayer3(INT32 choice);
static void M_SetupMultiPlayer4(INT32 choice);
static void M_SetupMultiHandler(INT32 choice);

// Options
// Split into multiple parts due to size
// Controls
menu_t OP_ControlsDef, OP_AllControlsDef;
menu_t OP_MouseOptionsDef, OP_Mouse2OptionsDef;
menu_t OP_Joystick1Def, OP_Joystick2Def, OP_Joystick3Def, OP_Joystick4Def;
static void M_VideoModeMenu(INT32 choice);
static void M_Setup1PControlsMenu(INT32 choice);
static void M_Setup2PControlsMenu(INT32 choice);
static void M_Setup3PControlsMenu(INT32 choice);
static void M_Setup4PControlsMenu(INT32 choice);

static void M_Setup1PJoystickMenu(INT32 choice);
static void M_Setup2PJoystickMenu(INT32 choice);
static void M_Setup3PJoystickMenu(INT32 choice);
static void M_Setup4PJoystickMenu(INT32 choice);

static void M_AssignJoystick(INT32 choice);
static void M_ChangeControl(INT32 choice);
static void M_ResetControls(INT32 choice);

// Video & Sound
menu_t OP_VideoOptionsDef, OP_VideoModeDef;
#ifdef HWRENDER
menu_t OP_OpenGLOptionsDef, OP_OpenGLColorDef;
#endif
menu_t OP_SoundOptionsDef;
//static void M_RestartAudio(void);

//Misc
menu_t OP_DataOptionsDef, OP_ScreenshotOptionsDef, OP_EraseDataDef;
#ifdef HAVE_DISCORDRPC
menu_t OP_DiscordOptionsDef;
#endif
menu_t OP_HUDOptionsDef, OP_ChatOptionsDef;
menu_t OP_GameOptionsDef, OP_ServerOptionsDef;
#ifndef NONET
menu_t OP_AdvServerOptionsDef;
#endif
//menu_t OP_NetgameOptionsDef, OP_GametypeOptionsDef;
menu_t OP_MonitorToggleDef;
static void M_ScreenshotOptions(INT32 choice);
static void M_EraseData(INT32 choice);

static void M_Addons(INT32 choice);
static void M_AddonsOptions(INT32 choice);
static patch_t *addonsp[NUM_EXT+5];

#define numaddonsshown 4

// Replay hut
menu_t MISC_ReplayHutDef;
menu_t MISC_ReplayOptionsDef;
static void M_HandleReplayHutList(INT32 choice);
static void M_DrawReplayHut(void);
static void M_DrawReplayStartMenu(void);
static boolean M_QuitReplayHut(void);
static void M_HutStartReplay(INT32 choice);

static void M_DrawPlaybackMenu(void);
static void M_PlaybackRewind(INT32 choice);
static void M_PlaybackPause(INT32 choice);
static void M_PlaybackFastForward(INT32 choice);
static void M_PlaybackAdvance(INT32 choice);
static void M_PlaybackSetViews(INT32 choice);
static void M_PlaybackAdjustView(INT32 choice);
static void M_PlaybackToggleFreecam(INT32 choice);
static void M_PlaybackQuit(INT32 choice);

static UINT8 playback_enterheld = 0; // horrid hack to prevent holding the button from being extremely fucked

// Drawing functions
static void M_DrawGenericMenu(void);
static void M_DrawGenericBackgroundMenu(void);
static void M_DrawCenteredMenu(void);
static void M_DrawAddons(void);
static void M_DrawSkyRoom(void);
static void M_DrawChecklist(void);
static void M_DrawEmblemHints(void);
static void M_DrawPauseMenu(void);
static void M_DrawLevelSelectOnly(boolean leftfade, boolean rightfade);
static void M_DrawServerMenu(void);
static void M_DrawImageDef(void);
//static void M_DrawLoad(void);
static void M_DrawLevelStats(void);
static void M_DrawTimeAttackMenu(void);
//static void M_DrawNightsAttackMenu(void);
//static void M_DrawSetupChoosePlayerMenu(void);
static void M_DrawControl(void);
static void M_DrawVideoMenu(void);
static void M_DrawHUDOptions(void);
static void M_DrawVideoMode(void);
static void M_DrawMonitorToggles(void);
#ifdef HWRENDER
static void M_OGL_DrawColorMenu(void);
#endif
static void M_DrawMPMainMenu(void);
#ifndef NONET
static void M_DrawConnectMenu(void);
#endif
static void M_DrawJoystick(void);
static void M_DrawSetupMultiPlayerMenu(void);

// Handling functions
#ifndef NONET
static boolean M_CancelConnect(void);
#endif
static boolean M_ExitPandorasBox(void);
static boolean M_QuitMultiPlayerMenu(void);
static void M_HandleAddons(INT32 choice);
static void M_HandleSoundTest(INT32 choice);
static void M_HandleImageDef(INT32 choice);
//static void M_HandleLoadSave(INT32 choice);
static void M_HandleLevelStats(INT32 choice);
#ifndef NONET
static void M_HandleConnectIP(INT32 choice);
#endif
static void M_HandleSetupMultiPlayer(INT32 choice);
static void M_HandleVideoMode(INT32 choice);
static void M_HandleMonitorToggles(INT32 choice);

// Consvar onchange functions
static void Nextmap_OnChange(void);
static void Newgametype_OnChange(void);
static void Dummymenuplayer_OnChange(void);
//static void Dummymares_OnChange(void);
static void Dummystaff_OnChange(void);

// ==========================================================================
// CONSOLE VARIABLES AND THEIR POSSIBLE VALUES GO HERE.
// ==========================================================================

consvar_t cv_showfocuslost = {"showfocuslost", "Yes", CV_SAVE, CV_YesNo, NULL, 0, NULL, NULL, 0, 0, NULL };

static CV_PossibleValue_t map_cons_t[] = {
	{0,"MIN"},
	{NUMMAPS, "MAX"},
	{0, NULL}
};
consvar_t cv_nextmap = {"nextmap", "1", CV_HIDEN|CV_CALL, map_cons_t, Nextmap_OnChange, 0, NULL, NULL, 0, 0, NULL};

static CV_PossibleValue_t skins_cons_t[MAXSKINS+1] = {{1, DEFAULTSKIN}};
consvar_t cv_chooseskin = {"chooseskin", DEFAULTSKIN, CV_HIDEN|CV_CALL, skins_cons_t, Nextmap_OnChange, 0, NULL, NULL, 0, 0, NULL};

// This gametype list is integral for many different reasons.
// When you add gametypes here, don't forget to update them in dehacked.c and doomstat.h!
CV_PossibleValue_t gametype_cons_t[NUMGAMETYPES+1];

consvar_t cv_newgametype = {"newgametype", "Race", CV_HIDEN|CV_CALL, gametype_cons_t, Newgametype_OnChange, 0, NULL, NULL, 0, 0, NULL};

static CV_PossibleValue_t serversort_cons_t[] = {
	{0,"Ping"},
	{1,"Modified State"},
	{2,"Most Players"},
	{3,"Least Players"},
	{4,"Max Player Slots"},
	{5,"Gametype"},
	{0,NULL}
};
consvar_t cv_serversort = {"serversort", "Ping", CV_CALL, serversort_cons_t, M_SortServerList, 0, NULL, NULL, 0, 0, NULL};

// autorecord demos for time attack
static consvar_t cv_autorecord = {"autorecord", "Yes", 0, CV_YesNo, NULL, 0, NULL, NULL, 0, 0, NULL};

CV_PossibleValue_t ghost_cons_t[] = {{0, "Hide"}, {1, "Show Character"}, {2, "Show All"}, {0, NULL}};
CV_PossibleValue_t ghost2_cons_t[] = {{0, "Hide"}, {1, "Show"}, {0, NULL}};

consvar_t cv_ghost_besttime  = {"ghost_besttime",  "Show All", CV_SAVE, ghost_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_ghost_bestlap   = {"ghost_bestlap",   "Show All", CV_SAVE, ghost_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_ghost_last      = {"ghost_last",      "Show All", CV_SAVE, ghost_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_ghost_guest     = {"ghost_guest",     "Show", CV_SAVE, ghost2_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_ghost_staff     = {"ghost_staff",     "Show", CV_SAVE, ghost2_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

//Console variables used solely in the menu system.
//todo: add a way to use non-console variables in the menu
//      or make these consvars legitimate like color or skin.
static void Splitplayers_OnChange(void);
CV_PossibleValue_t splitplayers_cons_t[] = {{1, "One"}, {2, "Two"}, {3, "Three"}, {4, "Four"}, {0, NULL}};
consvar_t cv_splitplayers = {"splitplayers", "One", CV_CALL, splitplayers_cons_t, Splitplayers_OnChange, 0, NULL, NULL, 0, 0, NULL};

static CV_PossibleValue_t dummymenuplayer_cons_t[] = {{0, "NOPE"}, {1, "P1"}, {2, "P2"}, {3, "P3"}, {4, "P4"}, {0, NULL}};
static CV_PossibleValue_t dummyteam_cons_t[] = {{0, "Spectator"}, {1, "Red"}, {2, "Blue"}, {0, NULL}};
static CV_PossibleValue_t dummyspectate_cons_t[] = {{0, "Spectator"}, {1, "Playing"}, {0, NULL}};
static CV_PossibleValue_t dummyscramble_cons_t[] = {{0, "Random"}, {1, "Points"}, {0, NULL}};
static CV_PossibleValue_t ringlimit_cons_t[] = {{0, "MIN"}, {9999, "MAX"}, {0, NULL}};
static CV_PossibleValue_t liveslimit_cons_t[] = {{0, "MIN"}, {99, "MAX"}, {0, NULL}};
/*static CV_PossibleValue_t dummymares_cons_t[] = {
	{-1, "END"}, {0,"Overall"}, {1,"Mare 1"}, {2,"Mare 2"}, {3,"Mare 3"}, {4,"Mare 4"}, {5,"Mare 5"}, {6,"Mare 6"}, {7,"Mare 7"}, {8,"Mare 8"}, {0,NULL}
};*/
static CV_PossibleValue_t dummystaff_cons_t[] = {{0, "MIN"}, {100, "MAX"}, {0, NULL}};

static consvar_t cv_dummymenuplayer = {"dummymenuplayer", "P1", CV_HIDEN|CV_CALL, dummymenuplayer_cons_t, Dummymenuplayer_OnChange, 0, NULL, NULL, 0, 0, NULL};
static consvar_t cv_dummyteam = {"dummyteam", "Spectator", CV_HIDEN, dummyteam_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
static consvar_t cv_dummyspectate = {"dummyspectate", "Spectator", CV_HIDEN, dummyspectate_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
static consvar_t cv_dummyscramble = {"dummyscramble", "Random", CV_HIDEN, dummyscramble_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
static consvar_t cv_dummyrings = {"dummyrings", "0", CV_HIDEN, ringlimit_cons_t,	NULL, 0, NULL, NULL, 0, 0, NULL};
static consvar_t cv_dummylives = {"dummylives", "0", CV_HIDEN, liveslimit_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
static consvar_t cv_dummycontinues = {"dummycontinues", "0", CV_HIDEN, liveslimit_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
//static consvar_t cv_dummymares = {"dummymares", "Overall", CV_HIDEN|CV_CALL, dummymares_cons_t, Dummymares_OnChange, 0, NULL, NULL, 0, 0, NULL};
static consvar_t cv_dummystaff = {"dummystaff", "0", CV_HIDEN|CV_CALL, dummystaff_cons_t, Dummystaff_OnChange, 0, NULL, NULL, 0, 0, NULL};

// ==========================================================================
// ORGANIZATION START.
// ==========================================================================
// Note: Never should we be jumping from one category of menu options to another
//       without first going to the Main Menu.
// Note: Ignore the above if you're working with the Pause menu.
// Note: (Prefix)_MainMenu should be the target of all Main Menu options that
//       point to submenus.

// ---------
// Main Menu
// ---------
static menuitem_t MainMenu[] =
{
	{IT_SUBMENU|IT_STRING, NULL, "Extras",      &SR_MainDef,        76},
	//{IT_CALL   |IT_STRING, NULL, "1 Player",    M_SinglePlayerMenu, 84},
	{IT_CALL   |IT_STRING, NULL, "Time Attack", M_TimeAttack,       84},
	{IT_SUBMENU|IT_STRING, NULL, "Multiplayer", &MP_MainDef,        92},
	{IT_CALL   |IT_STRING, NULL, "Options",     M_Options,          100},
	{IT_CALL   |IT_STRING, NULL, "Addons",      M_Addons,           108},
	{IT_CALL   |IT_STRING, NULL, "Quit  Game",  M_QuitSRB2,         116},
};

typedef enum
{
	secrets = 0,
	singleplr,
	multiplr,
	options,
	addons,
	quitdoom
} main_e;

static menuitem_t MISC_AddonsMenu[] =
{
	{IT_KEYHANDLER | IT_NOTHING, NULL, "", M_HandleAddons, 0},     // dummy menuitem for the control func
};

static menuitem_t MISC_ReplayHutMenu[] =
{
	{IT_KEYHANDLER|IT_NOTHING, NULL, "", M_HandleReplayHutList, 0}, // Dummy menuitem for the replay list
	{IT_NOTHING,               NULL, "", NULL,                  0}, // Dummy for handling wrapping to the top of the menu..
};

static menuitem_t MISC_ReplayStartMenu[] =
{
	{IT_CALL      |IT_STRING,  NULL, "Load Addons and Watch", M_HutStartReplay,   0},
	{IT_CALL      |IT_STRING,  NULL, "Watch Without Addons",  M_HutStartReplay,   10},
	{IT_CALL      |IT_STRING,  NULL, "Watch Replay",          M_HutStartReplay,   10},
	{IT_SUBMENU   |IT_STRING,  NULL, "Back",                  &MISC_ReplayHutDef, 30},
};

static menuitem_t MISC_ReplayOptionsMenu[] =
{
	{IT_CVAR|IT_STRING, NULL, "Record Replays",      &cv_recordmultiplayerdemos, 0},
	{IT_CVAR|IT_STRING, NULL, "Sync Check Interval", &cv_netdemosyncquality,     10},
};

static tic_t playback_last_menu_interaction_leveltime = 0;
static menuitem_t PlaybackMenu[] =
{
	{IT_CALL   | IT_STRING, "M_PHIDE",  "Hide Menu (Esc)", M_SelectableClearMenus, 0},

	{IT_CALL   | IT_STRING, "M_PREW",   "Rewind ([)",        M_PlaybackRewind,      20},
	{IT_CALL   | IT_STRING, "M_PPAUSE", "Pause (\\)",         M_PlaybackPause,       36},
	{IT_CALL   | IT_STRING, "M_PFFWD",  "Fast-Forward (])",  M_PlaybackFastForward, 52},
	{IT_CALL   | IT_STRING, "M_PSTEPB", "Backup Frame ([)",  M_PlaybackRewind,      20},
	{IT_CALL   | IT_STRING, "M_PRESUM", "Resume",        M_PlaybackPause,       36},
	{IT_CALL   | IT_STRING, "M_PFADV",  "Advance Frame (])", M_PlaybackAdvance,     52},

	{IT_ARROWS | IT_STRING, "M_PVIEWS", "View Count (- and =)",  M_PlaybackSetViews, 72},
	{IT_ARROWS | IT_STRING, "M_PNVIEW", "Viewpoint (1)",   M_PlaybackAdjustView, 88},
	{IT_ARROWS | IT_STRING, "M_PNVIEW", "Viewpoint 2 (2)", M_PlaybackAdjustView, 104},
	{IT_ARROWS | IT_STRING, "M_PNVIEW", "Viewpoint 3 (3)", M_PlaybackAdjustView, 120},
	{IT_ARROWS | IT_STRING, "M_PNVIEW", "Viewpoint 4 (4)", M_PlaybackAdjustView, 136},

	{IT_CALL   | IT_STRING, "M_PVIEWS", "Toggle Free Camera (')",	M_PlaybackToggleFreecam, 156},
	{IT_CALL   | IT_STRING, "M_PEXIT",  "Stop Playback",   M_PlaybackQuit, 172},
};
typedef enum
{
	playback_hide,
	playback_rewind,
	playback_pause,
	playback_fastforward,
	playback_backframe,
	playback_resume,
	playback_advanceframe,
	playback_viewcount,
	playback_view1,
	playback_view2,
	playback_view3,
	playback_view4,
	playback_freecamera,
	//playback_moreoptions,
	playback_quit
} playback_e;

// ---------------------------------
// Pause Menu Mode Attacking Edition
// ---------------------------------
static menuitem_t MAPauseMenu[] =
{
	{IT_CALL | IT_STRING,    NULL, "Continue",             M_SelectableClearMenus,48},
	{IT_CALL | IT_STRING,    NULL, "Retry",                M_ModeAttackRetry,     56},
	{IT_CALL | IT_STRING,    NULL, "Abort",                M_ModeAttackEndGame,   64},
};

typedef enum
{
	mapause_continue,
	mapause_retry,
	mapause_abort
} mapause_e;

// ---------------------
// Pause Menu MP Edition
// ---------------------
static menuitem_t MPauseMenu[] =
{
	{IT_STRING | IT_CALL,     NULL, "Addons...",        M_Addons,                8},
	{IT_STRING | IT_SUBMENU,  NULL, "Scramble Teams...", &MISC_ScrambleTeamDef,  16},
	{IT_STRING | IT_CALL,     NULL, "Switch Map..."    , M_MapChange,            24},

#ifdef HAVE_DISCORDRPC
	{IT_STRING | IT_SUBMENU,  NULL, "Ask To Join Requests...", &MISC_DiscordRequestsDef, 24},
#endif

	{IT_CALL | IT_STRING,    NULL, "Continue",           M_SelectableClearMenus, 40},
	{IT_CALL | IT_STRING,    NULL, "P1 Setup...",        M_SetupMultiPlayer,     48}, // splitscreen
	{IT_CALL | IT_STRING,    NULL, "P2 Setup...",        M_SetupMultiPlayer2,    56}, // splitscreen
	{IT_CALL | IT_STRING,    NULL, "P3 Setup...",        M_SetupMultiPlayer3,    64}, // splitscreen
	{IT_CALL | IT_STRING,    NULL, "P4 Setup...",        M_SetupMultiPlayer4,    72}, // splitscreen

	{IT_STRING | IT_CALL,    NULL, "Spectate",           M_ConfirmSpectate,      48}, // alone
	{IT_STRING | IT_CALL,    NULL, "Enter Game",         M_ConfirmEnterGame,     48}, // alone
	{IT_STRING | IT_CALL,    NULL, "Cancel Join",        M_ConfirmSpectate,      48}, // alone
	{IT_STRING | IT_SUBMENU, NULL, "Switch Team...",     &MISC_ChangeTeamDef,    48},
	{IT_STRING | IT_SUBMENU, NULL, "Enter/Spectate...",  &MISC_ChangeSpectateDef,48},
	{IT_CALL | IT_STRING,    NULL, "Player Setup...",    M_SetupMultiPlayer,     56}, // alone
	{IT_CALL | IT_STRING,    NULL, "Options",            M_Options,              64},

	{IT_CALL | IT_STRING,    NULL, "Return to Title",    M_EndGame,              80},
	{IT_CALL | IT_STRING,    NULL, "Quit Game",          M_QuitSRB2,             88},
};

typedef enum
{
	mpause_addons = 0,
	mpause_scramble,
	mpause_switchmap,
#ifdef HAVE_DISCORDRPC
	mpause_discordrequests,
#endif

	mpause_continue,
	mpause_psetupsplit,
	mpause_psetupsplit2,
	mpause_psetupsplit3,
	mpause_psetupsplit4,

	mpause_spectate,
	mpause_entergame,
	mpause_canceljoin,
	mpause_switchteam,
	mpause_switchspectate,
	mpause_psetup,
	mpause_options,

	mpause_title,
	mpause_quit
} mpause_e;

// ---------------------
// Pause Menu SP Edition
// ---------------------
static menuitem_t SPauseMenu[] =
{
	// Pandora's Box will be shifted up if both options are available
	{IT_CALL | IT_STRING,    NULL, "Pandora's Box...",     M_PandorasBox,         16},
	{IT_CALL | IT_STRING,    NULL, "Medal Hints...",       M_EmblemHints,         24},
	//{IT_CALL | IT_STRING,    NULL, "Level Select...",      M_LoadGameLevelSelect, 32},

	{IT_CALL | IT_STRING,    NULL, "Continue",             M_SelectableClearMenus,48},
	{IT_CALL | IT_STRING,    NULL, "Retry",                M_Retry,               56},
	{IT_CALL | IT_STRING,    NULL, "Options",              M_Options,             64},

	{IT_CALL | IT_STRING,    NULL, "Return to Title",      M_EndGame,             80},
	{IT_CALL | IT_STRING,    NULL, "Quit Game",            M_QuitSRB2,            88},
};

typedef enum
{
	spause_pandora = 0,
	spause_hints,
	//spause_levelselect,

	spause_continue,
	spause_retry,
	spause_options,
	spause_title,
	spause_quit
} spause_e;

#ifdef HAVE_DISCORDRPC
static menuitem_t MISC_DiscordRequestsMenu[] =
{
	{IT_KEYHANDLER|IT_NOTHING, NULL, "", M_HandleDiscordRequests, 0},
};
#endif

// -----------------
// Misc menu options
// -----------------
// Prefix: MISC_
static menuitem_t MISC_ScrambleTeamMenu[] =
{
	{IT_STRING|IT_CVAR,      NULL, "Scramble Method", &cv_dummyscramble,     30},
	{IT_WHITESTRING|IT_CALL, NULL, "Confirm",         M_ConfirmTeamScramble, 90},
};

static menuitem_t MISC_ChangeTeamMenu[] =
{
	{IT_STRING|IT_CVAR,              NULL, "Player",            &cv_dummymenuplayer,    30},
	{IT_STRING|IT_CVAR,              NULL, "Team",              &cv_dummyteam,          40},
	{IT_WHITESTRING|IT_CALL,         NULL, "Confirm",           M_ConfirmTeamChange,    90},
};

static menuitem_t MISC_ChangeSpectateMenu[] =
{
	{IT_STRING|IT_CVAR,              NULL, "Player",        &cv_dummymenuplayer,        30},
	{IT_STRING|IT_CVAR,              NULL, "Status",        &cv_dummyspectate,          40},
	{IT_WHITESTRING|IT_CALL,         NULL, "Confirm",       M_ConfirmSpectateChange,    90},
};

static menuitem_t MISC_ChangeLevelMenu[] =
{
	{IT_STRING|IT_CVAR,              NULL, "Game Type",             &cv_newgametype,    68},
	{IT_STRING|IT_CVAR,              NULL, "Level",                 &cv_nextmap,        78},
	{IT_WHITESTRING|IT_CALL,         NULL, "Change Level",          M_ChangeLevel,     130},
};

static menuitem_t MISC_HelpMenu[] =
{
	{IT_KEYHANDLER | IT_NOTHING, NULL, "MANUAL00", M_HandleImageDef, 0},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "MANUAL01", M_HandleImageDef, 1},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "MANUAL02", M_HandleImageDef, 1},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "MANUAL03", M_HandleImageDef, 1},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "MANUAL04", M_HandleImageDef, 1},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "MANUAL05", M_HandleImageDef, 1},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "MANUAL06", M_HandleImageDef, 1},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "MANUAL07", M_HandleImageDef, 1},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "MANUAL08", M_HandleImageDef, 1},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "MANUAL09", M_HandleImageDef, 1},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "MANUAL10", M_HandleImageDef, 1},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "MANUAL11", M_HandleImageDef, 1},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "MANUAL12", M_HandleImageDef, 1},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "MANUAL99", M_HandleImageDef, 0},
};

// --------------------------------
// Sky Room and all of its submenus
// --------------------------------
// Prefix: SR_

// Pause Menu Pandora's Box Options
static menuitem_t SR_PandorasBox[] =
{
	{IT_STRING | IT_CVAR, NULL, "Rings",              &cv_dummyrings,      20},
	{IT_STRING | IT_CVAR, NULL, "Lives",              &cv_dummylives,      30},
	{IT_STRING | IT_CVAR, NULL, "Continues",          &cv_dummycontinues,  40},

	{IT_STRING | IT_CVAR, NULL, "Gravity",            &cv_gravity,         60},
	{IT_STRING | IT_CVAR, NULL, "Throw Rings",        &cv_ringslinger,     70},

	{IT_STRING | IT_CALL, NULL, "Get All Emeralds",   M_GetAllEmeralds,    90},
	{IT_STRING | IT_CALL, NULL, "Destroy All Robots", M_DestroyRobots,    100},

	{IT_STRING | IT_CALL, NULL, "Ultimate Cheat",     M_UltimateCheat,    130},
};

// Sky Room Custom Unlocks
static menuitem_t SR_MainMenu[] =
{
	{IT_STRING|IT_SUBMENU,                  NULL, "Unlockables", &SR_UnlockChecklistDef, 100},
	{IT_CALL|IT_STRING|IT_CALL_NOTMODIFIED, NULL, "Statistics",  M_Statistics,           108},
	{IT_CALL|IT_STRING,                     NULL, "Replay Hut",  M_ReplayHut,            116},
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom1
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom2
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom3
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom4
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom5
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom6
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom7
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom8
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom9
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom10
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom11
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom12
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom13
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom14
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom15
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom16
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom17
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom18
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom19
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom20
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom21
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom22
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom23
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom24
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom25
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom26
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom27
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom28
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom29
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom30
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom31
	{IT_DISABLED,         NULL, "",   NULL,                 0}, // Custom32

};

/*static menuitem_t SR_LevelSelectMenu[] =
{
	{IT_STRING|IT_CVAR,              NULL, "Level",                 &cv_nextmap,            78},
	{IT_WHITESTRING|IT_CALL,         NULL, "Start",                 M_LevelSelectWarp,     130},
};*/

static menuitem_t SR_UnlockChecklistMenu[] =
{
	{IT_SUBMENU | IT_STRING,         NULL, "NEXT", &MainDef, 192},
};

static menuitem_t SR_EmblemHintMenu[] =
{
	{IT_STRING|IT_CVAR,         NULL, "Medal Radar",  &cv_itemfinder, 10},
	{IT_WHITESTRING|IT_SUBMENU, NULL, "Back",         &SPauseDef,     20}
};

// --------------------------------
// 1 Player and all of its submenus
// --------------------------------
// Prefix: SP_

// Single Player Main
static menuitem_t SP_MainMenu[] =
{
	//{IT_CALL | IT_STRING,                       NULL, "Start Game",    M_LoadGame,        92},
	{IT_SECRET,                                 NULL, "Record Attack", M_TimeAttack,     100},
	//{IT_SECRET,                                 NULL, "NiGHTS Mode",   M_NightsAttack,   108},
	{IT_CALL | IT_STRING | IT_CALL_NOTMODIFIED, NULL, "Statistics",    M_Statistics,     108},
};

enum
{
	//sploadgame,
	sprecordattack,
	//spnightsmode,
	spstatistics
};

// Single Player Load Game
/*static menuitem_t SP_LoadGameMenu[] =
{
	{IT_KEYHANDLER | IT_NOTHING, NULL, "", M_HandleLoadSave, '\0'},     // dummy menuitem for the control func
};

// Single Player Level Select
static menuitem_t SP_LevelSelectMenu[] =
{
	{IT_STRING|IT_CVAR,              NULL, "Level",                 &cv_nextmap,            78},
	{IT_WHITESTRING|IT_CALL,         NULL, "Start",                 M_LevelSelectWarp,     130},
};*/

// Single Player Time Attack
static menuitem_t SP_TimeAttackMenu[] =
{
	{IT_STRING|IT_CVAR|IT_CV_STRING, NULL, "Name",       &cv_playername,        0},
	{IT_STRING|IT_CVAR,              NULL, "Character",  &cv_chooseskin,       13},
	{IT_STRING|IT_CVAR,              NULL, "Color",      &cv_playercolor,      26},
	{IT_STRING|IT_CVAR,              NULL, "Level",      &cv_nextmap,          78},

	{IT_DISABLED,                                NULL, "Guest...",      &SP_GuestReplayDef,    98},
	{IT_DISABLED,                                NULL, "Replay...",     &SP_ReplayDef,        108},
	{IT_WHITESTRING|IT_SUBMENU,                  NULL, "Ghosts...",     &SP_GhostDef,         118},
	{IT_WHITESTRING|IT_CALL|IT_CALL_NOTMODIFIED, NULL, "Start",         M_ChooseTimeAttack,   130},
};

enum
{
	taname,
	taplayer,
	tacolor,
	talevel,

	taguest,
	tareplay,
	taghost,
	tastart
};

static menuitem_t SP_ReplayMenu[] =
{
	{IT_WHITESTRING|IT_CALL, NULL, "Replay Best Time",  M_ReplayTimeAttack,  90},
	{IT_WHITESTRING|IT_CALL, NULL, "Replay Best Lap",   M_ReplayTimeAttack,  98},

	{IT_WHITESTRING|IT_CALL, NULL, "Replay Last",       M_ReplayTimeAttack, 106},
	{IT_WHITESTRING|IT_CALL, NULL, "Replay Guest",      M_ReplayTimeAttack, 114},
	{IT_WHITESTRING|IT_KEYHANDLER, NULL, "Replay Staff",M_HandleStaffReplay,122},

	{IT_WHITESTRING|IT_SUBMENU, NULL, "Back",           &SP_TimeAttackDef,  130}
};

/*static menuitem_t SP_NightsReplayMenu[] =
{
	{IT_WHITESTRING|IT_CALL, NULL, "Replay Best Score", M_ReplayTimeAttack, 0},
	{IT_WHITESTRING|IT_CALL, NULL, "Replay Best Time",  M_ReplayTimeAttack,16},

	{IT_WHITESTRING|IT_CALL, NULL, "Replay Last",       M_ReplayTimeAttack,21},
	{IT_WHITESTRING|IT_CALL, NULL, "Replay Guest",      M_ReplayTimeAttack,29},
	{IT_WHITESTRING|IT_KEYHANDLER, NULL, "Replay Staff",M_HandleStaffReplay,37},

	{IT_WHITESTRING|IT_SUBMENU, NULL, "Back",           &SP_NightsAttackDef, 50}
};*/

static menuitem_t SP_GuestReplayMenu[] =
{
	{IT_WHITESTRING|IT_CALL, NULL, "Save Best Time as Guest",  M_SetGuestReplay, 94},
	{IT_WHITESTRING|IT_CALL, NULL, "Save Best Lap as Guest",   M_SetGuestReplay,102},
	{IT_WHITESTRING|IT_CALL, NULL, "Save Last as Guest",       M_SetGuestReplay,110},

	{IT_WHITESTRING|IT_CALL, NULL, "Delete Guest Replay",      M_SetGuestReplay,120},

	{IT_WHITESTRING|IT_SUBMENU, NULL, "Back",                &SP_TimeAttackDef, 130}
};

/*static menuitem_t SP_NightsGuestReplayMenu[] =
{
	{IT_WHITESTRING|IT_CALL, NULL, "Save Best Score as Guest", M_SetGuestReplay, 8},
	{IT_WHITESTRING|IT_CALL, NULL, "Save Best Time as Guest",  M_SetGuestReplay,16},
	{IT_WHITESTRING|IT_CALL, NULL, "Save Last as Guest",       M_SetGuestReplay,24},

	{IT_WHITESTRING|IT_CALL, NULL, "Delete Guest Replay",      M_SetGuestReplay,37},

	{IT_WHITESTRING|IT_SUBMENU, NULL, "Back",                &SP_NightsAttackDef, 50}
};*/

static menuitem_t SP_GhostMenu[] =
{
	{IT_STRING|IT_CVAR,         NULL, "Best Time",   &cv_ghost_besttime, 88},
	{IT_STRING|IT_CVAR,         NULL, "Best Lap",    &cv_ghost_bestlap,  96},
	{IT_STRING|IT_CVAR,         NULL, "Last",        &cv_ghost_last,    104},
	{IT_DISABLED,               NULL, "Guest",       &cv_ghost_guest,   112},
	{IT_DISABLED,               NULL, "Staff Attack",&cv_ghost_staff,   120},

	{IT_WHITESTRING|IT_SUBMENU, NULL, "Back",        &SP_TimeAttackDef, 130}
};

/*static menuitem_t SP_NightsGhostMenu[] =
{
	{IT_STRING|IT_CVAR,         NULL, "Best Score",  &cv_ghost_bestscore, 0},
	{IT_STRING|IT_CVAR,         NULL, "Best Time",   &cv_ghost_besttime,  8},
	{IT_STRING|IT_CVAR,         NULL, "Last",        &cv_ghost_last,     16},

	{IT_STRING|IT_CVAR,         NULL, "Guest",       &cv_ghost_guest,    29},
	{IT_STRING|IT_CVAR,         NULL, "Staff Attack",&cv_ghost_staff,    37},

	{IT_WHITESTRING|IT_SUBMENU, NULL, "Back",        &SP_NightsAttackDef,  50}
};*/

// Single Player Nights Attack
/*static menuitem_t SP_NightsAttackMenu[] =
{
	{IT_STRING|IT_CVAR,        NULL, "Level",            &cv_nextmap,          44},
	{IT_STRING|IT_CVAR,        NULL, "Show Records For", &cv_dummymares,       54},

	{IT_DISABLED,              NULL, "Guest Option...",  &SP_NightsGuestReplayDef,   108},
	{IT_DISABLED,              NULL, "Replay...",        &SP_NightsReplayDef,        118},
	{IT_DISABLED,              NULL, "Ghosts...",        &SP_NightsGhostDef,         128},
	{IT_WHITESTRING|IT_CALL|IT_CALL_NOTMODIFIED,   NULL, "Start",            M_ChooseNightsAttack, 138},
};*/

enum
{
	nalevel,
	narecords,

	naguest,
	nareplay,
	naghost,
	nastart
};

// Statistics
static menuitem_t SP_LevelStatsMenu[] =
{
	{IT_KEYHANDLER | IT_NOTHING, NULL, "", M_HandleLevelStats, '\0'},     // dummy menuitem for the control func
};

// A rare case.
// External files modify this menu, so we can't call it static.
// And I'm too lazy to go through and rename it everywhere. ARRGH!
#define M_ChoosePlayer NULL
menuitem_t PlayerMenu[MAXSKINS];

// -----------------------------------
// Multiplayer and all of its submenus
// -----------------------------------
// Prefix: MP_

static menuitem_t MP_MainMenu[] =
{
	{IT_HEADER, NULL, "Players", NULL, 0},
	{IT_STRING|IT_CVAR,      NULL, "Number of local players",     &cv_splitplayers, 10},

	{IT_STRING|IT_KEYHANDLER,NULL, "Player setup...",     M_SetupMultiHandler,18},

	{IT_HEADER, NULL, "Host a game", NULL, 100-24},
#ifndef NONET
	{IT_STRING|IT_CALL,       NULL, "Internet/LAN...",           M_StartServerMenu,        110-24},
#else
	{IT_GRAYEDOUT,            NULL, "Internet/LAN...",           NULL,                     110-24},
#endif
	{IT_STRING|IT_CALL,       NULL, "Offline...",                M_StartOfflineServerMenu, 118-24},

	{IT_HEADER, NULL, "Join a game", NULL, 132-24},
#ifndef NONET
	{IT_STRING|IT_CALL,       NULL, "Internet server browser...",M_ConnectMenuModChecks,   142-24},
	{IT_STRING|IT_KEYHANDLER, NULL, "Specify IPv4 address:",     M_HandleConnectIP,        150-24},
#else
	{IT_GRAYEDOUT,            NULL, "Internet server browser...",NULL,                     142-24},
	{IT_GRAYEDOUT,            NULL, "Specify IPv4 address:",     NULL,                     150-24},
#endif
	//{IT_HEADER, NULL, "Player setup", NULL, 80},
	//{IT_STRING|IT_CALL,       NULL, "Name, character, color...", M_SetupMultiPlayer,       90},
};

#ifndef NONET

static menuitem_t MP_ServerMenu[] =
{
	{IT_STRING|IT_CVAR,                NULL, "Max. Player Count",     &cv_maxplayers,        10},
	{IT_STRING|IT_CVAR,                NULL, "Advertise",             &cv_advertise,         20},
	{IT_STRING|IT_CVAR|IT_CV_STRING,   NULL, "Server Name",           &cv_servername,        30},

	{IT_STRING|IT_CVAR,                NULL, "Game Type",             &cv_newgametype,       68},
	{IT_STRING|IT_CVAR,                NULL, "Level",                 &cv_nextmap,           78},

	{IT_WHITESTRING|IT_CALL,           NULL, "Start",                 M_StartServer,        130},
};

#endif

// Separated offline and normal servers.
static menuitem_t MP_OfflineServerMenu[] =
{
	{IT_STRING|IT_CVAR,      NULL, "Game Type",             &cv_newgametype,       68},
	{IT_STRING|IT_CVAR,      NULL, "Level",                 &cv_nextmap,           78},

	{IT_WHITESTRING|IT_CALL, NULL, "Start",                 M_StartServer,        130},
};

static menuitem_t MP_PlayerSetupMenu[] =
{
	{IT_KEYHANDLER | IT_STRING,   NULL, "Name",      M_HandleSetupMultiPlayer,   0},
	{IT_KEYHANDLER | IT_STRING,   NULL, "Character", M_HandleSetupMultiPlayer,  16}, // Tails 01-18-2001
	{IT_KEYHANDLER | IT_STRING,   NULL, "Color",     M_HandleSetupMultiPlayer, 152},
};

#ifndef NONET
static menuitem_t MP_ConnectMenu[] =
{
	{IT_STRING | IT_CVAR,       NULL, "Sort By",  &cv_serversort,      4},
	{IT_STRING | IT_KEYHANDLER, NULL, "Page",     M_HandleServerPage, 12},
	{IT_STRING | IT_CALL,       NULL, "Refresh",  M_Refresh,          20},

	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,          36},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,          48},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,          60},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,          72},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,          84},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,          96},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         108},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         120},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         132},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         144},
	{IT_STRING | IT_SPACE, NULL, "",              M_Connect,         156},
};

enum
{
	mp_connect_sort,
	mp_connect_page,
	mp_connect_refresh,
	FIRSTSERVERLINE
};
#endif

// ------------------------------------
// Options and most (?) of its submenus
// ------------------------------------
// Prefix: OP_
static menuitem_t OP_MainMenu[] =
{
	{IT_SUBMENU|IT_STRING,		NULL, "Control Setup...",		&OP_ControlsDef,			 10},

	{IT_SUBMENU|IT_STRING,		NULL, "Video Options...",		&OP_VideoOptionsDef,		 30},
	{IT_SUBMENU|IT_STRING,		NULL, "Sound Options...",		&OP_SoundOptionsDef,		 40},

	{IT_SUBMENU|IT_STRING,		NULL, "HUD Options...",			&OP_HUDOptionsDef,			 60},
	{IT_SUBMENU|IT_STRING,		NULL, "Gameplay Options...",	&OP_GameOptionsDef,			 70},
	{IT_SUBMENU|IT_STRING,		NULL, "Server Options...",		&OP_ServerOptionsDef,		 80},

	{IT_SUBMENU|IT_STRING,		NULL, "Data Options...",		&OP_DataOptionsDef,			100},

	{IT_CALL|IT_STRING,			NULL, "Tricks & Secrets (F1)",	M_Manual,					120},
	{IT_CALL|IT_STRING,			NULL, "Play Credits",			M_Credits,					130},
};

static menuitem_t OP_ControlsMenu[] =
{
	{IT_CALL | IT_STRING, NULL, "Player 1 Controls...", M_Setup1PControlsMenu,  10},
	{IT_CALL | IT_STRING, NULL, "Player 2 Controls...", M_Setup2PControlsMenu,  20},

	{IT_CALL | IT_STRING, NULL, "Player 3 Controls...", &M_Setup3PControlsMenu,  30},
	{IT_CALL | IT_STRING, NULL, "Player 4 Controls...", &M_Setup4PControlsMenu,  40},

	{IT_STRING | IT_CVAR, NULL, "Controls per key", &cv_controlperkey, 60},
};

static menuitem_t OP_AllControlsMenu[] =
{
	{IT_SUBMENU|IT_STRING, NULL, "Gamepad Options...", &OP_Joystick1Def, 0},
	{IT_CALL|IT_STRING, NULL, "Reset to defaults", M_ResetControls, 8},
	//{IT_SPACE, NULL, NULL, NULL, 0},
	{IT_HEADER, NULL, "Gameplay Controls", NULL, 0},
	{IT_SPACE, NULL, NULL, NULL, 0},
	{IT_CONTROL, NULL, "Accelerate",            M_ChangeControl, gc_accelerate },
	{IT_CONTROL, NULL, "Turn Left",             M_ChangeControl, gc_turnleft   },
	{IT_CONTROL, NULL, "Turn Right",            M_ChangeControl, gc_turnright  },
	{IT_CONTROL, NULL, "Drift",                 M_ChangeControl, gc_drift      },
	{IT_CONTROL, NULL, "Brake",                 M_ChangeControl, gc_brake      },
	{IT_CONTROL, NULL, "Use/Throw Item",        M_ChangeControl, gc_fire       },
	{IT_CONTROL, NULL, "Aim Forward",           M_ChangeControl, gc_aimforward },
	{IT_CONTROL, NULL, "Aim Backward",          M_ChangeControl, gc_aimbackward},
	{IT_CONTROL, NULL, "Look Backward",         M_ChangeControl, gc_lookback   },
	{IT_HEADER, NULL, "Miscelleanous Controls", NULL, 0},
	{IT_SPACE, NULL, NULL, NULL, 0},
	{IT_CONTROL, NULL, "Chat",                  M_ChangeControl, gc_talkkey    },
	//{IT_CONTROL, NULL, "Team Chat",             M_ChangeControl, gc_teamkey    },
	{IT_CONTROL, NULL, "Show Rankings",         M_ChangeControl, gc_scores     },
	{IT_CONTROL, NULL, "Change Viewpoint",      M_ChangeControl, gc_viewpoint  },
	{IT_CONTROL, NULL, "Reset Camera",          M_ChangeControl, gc_camreset   },
	{IT_CONTROL, NULL, "Toggle First-Person",   M_ChangeControl, gc_camtoggle  },
	{IT_CONTROL, NULL, "Pause",                 M_ChangeControl, gc_pause      },
	{IT_CONTROL, NULL, "Screenshot",            M_ChangeControl, gc_screenshot },
	{IT_CONTROL, NULL, "Toggle GIF Recording",  M_ChangeControl, gc_recordgif  },
	{IT_CONTROL, NULL, "Open/Close Menu (ESC)", M_ChangeControl, gc_systemmenu },
	{IT_CONTROL, NULL, "Developer Console",     M_ChangeControl, gc_console    },
	{IT_HEADER, NULL, "Spectator Controls", NULL, 0},
	{IT_SPACE, NULL, NULL, NULL, 0},
	{IT_CONTROL, NULL, "Become Spectator",      M_ChangeControl, gc_spectate   },
	{IT_CONTROL, NULL, "Look Up",               M_ChangeControl, gc_lookup     },
	{IT_CONTROL, NULL, "Look Down",             M_ChangeControl, gc_lookdown   },
	{IT_CONTROL, NULL, "Center View",           M_ChangeControl, gc_centerview },
	{IT_HEADER, NULL, "Custom Lua Actions", NULL, 0},
	{IT_SPACE, NULL, NULL, NULL, 0},
	{IT_CONTROL, NULL, "Custom Action 1",       M_ChangeControl, gc_custom1    },
	{IT_CONTROL, NULL, "Custom Action 2",       M_ChangeControl, gc_custom2    },
	{IT_CONTROL, NULL, "Custom Action 3",       M_ChangeControl, gc_custom3    },
};

static menuitem_t OP_Joystick1Menu[] =
{
	{IT_STRING | IT_CALL,  NULL, "Select Gamepad..."  , M_Setup1PJoystickMenu, 10},
	{IT_STRING | IT_CVAR,  NULL, "Aim Forward/Back"   , &cv_aimaxis          , 30},
	{IT_STRING | IT_CVAR,  NULL, "Turn Left/Right"    , &cv_turnaxis         , 40},
	{IT_STRING | IT_CVAR,  NULL, "Accelerate"         , &cv_moveaxis         , 50},
	{IT_STRING | IT_CVAR,  NULL, "Brake"              , &cv_brakeaxis        , 60},
	{IT_STRING | IT_CVAR,  NULL, "Drift"              , &cv_driftaxis        , 70},
	{IT_STRING | IT_CVAR,  NULL, "Use Item"           , &cv_fireaxis         , 80},
	{IT_STRING | IT_CVAR,  NULL, "Look Up/Down"       , &cv_lookaxis         , 90},
};

static menuitem_t OP_Joystick2Menu[] =
{
	{IT_STRING | IT_CALL,  NULL, "Select Gamepad..."  , M_Setup2PJoystickMenu, 10},
	{IT_STRING | IT_CVAR,  NULL, "Aim Forward/Back"   , &cv_aimaxis2         , 30},
	{IT_STRING | IT_CVAR,  NULL, "Turn Left/Right"    , &cv_turnaxis2        , 40},
	{IT_STRING | IT_CVAR,  NULL, "Accelerate"         , &cv_moveaxis2        , 50},
	{IT_STRING | IT_CVAR,  NULL, "Brake"              , &cv_brakeaxis2       , 60},
	{IT_STRING | IT_CVAR,  NULL, "Drift"              , &cv_driftaxis2       , 70},
	{IT_STRING | IT_CVAR,  NULL, "Use Item"           , &cv_fireaxis2        , 80},
	{IT_STRING | IT_CVAR,  NULL, "Look Up/Down"       , &cv_lookaxis2        , 90},
};

static menuitem_t OP_Joystick3Menu[] =
{
	{IT_STRING | IT_CALL,  NULL, "Select Gamepad..."  , M_Setup3PJoystickMenu, 10},
	{IT_STRING | IT_CVAR,  NULL, "Aim Forward/Back"   , &cv_aimaxis3         , 30},
	{IT_STRING | IT_CVAR,  NULL, "Turn Left/Right"    , &cv_turnaxis3        , 40},
	{IT_STRING | IT_CVAR,  NULL, "Accelerate"         , &cv_moveaxis3        , 50},
	{IT_STRING | IT_CVAR,  NULL, "Brake"              , &cv_brakeaxis3       , 60},
	{IT_STRING | IT_CVAR,  NULL, "Drift"              , &cv_driftaxis3       , 70},
	{IT_STRING | IT_CVAR,  NULL, "Use Item"           , &cv_fireaxis3        , 80},
	{IT_STRING | IT_CVAR,  NULL, "Look Up/Down"       , &cv_lookaxis3        , 90},
};

static menuitem_t OP_Joystick4Menu[] =
{
	{IT_STRING | IT_CALL,  NULL, "Select Gamepad..."  , M_Setup4PJoystickMenu, 10},
	{IT_STRING | IT_CVAR,  NULL, "Aim Forward/Back"   , &cv_aimaxis4         , 30},
	{IT_STRING | IT_CVAR,  NULL, "Turn Left/Right"    , &cv_turnaxis4        , 40},
	{IT_STRING | IT_CVAR,  NULL, "Accelerate"         , &cv_moveaxis4        , 50},
	{IT_STRING | IT_CVAR,  NULL, "Brake"              , &cv_brakeaxis4       , 60},
	{IT_STRING | IT_CVAR,  NULL, "Drift"              , &cv_driftaxis4       , 70},
	{IT_STRING | IT_CVAR,  NULL, "Use Item"           , &cv_fireaxis4        , 80},
	{IT_STRING | IT_CVAR,  NULL, "Look Up/Down"       , &cv_lookaxis4        , 90},
};

static menuitem_t OP_JoystickSetMenu[] =
{
	{IT_CALL | IT_NOTHING, "None", NULL, M_AssignJoystick, LINEHEIGHT+5},
	{IT_CALL | IT_NOTHING, "", NULL, M_AssignJoystick, (LINEHEIGHT*2)+5},
	{IT_CALL | IT_NOTHING, "", NULL, M_AssignJoystick, (LINEHEIGHT*3)+5},
	{IT_CALL | IT_NOTHING, "", NULL, M_AssignJoystick, (LINEHEIGHT*4)+5},
	{IT_CALL | IT_NOTHING, "", NULL, M_AssignJoystick, (LINEHEIGHT*5)+5},
	{IT_CALL | IT_NOTHING, "", NULL, M_AssignJoystick, (LINEHEIGHT*6)+5},
	{IT_CALL | IT_NOTHING, "", NULL, M_AssignJoystick, (LINEHEIGHT*7)+5},
	{IT_CALL | IT_NOTHING, "", NULL, M_AssignJoystick, (LINEHEIGHT*8)+5},
};

/*static menuitem_t OP_MouseOptionsMenu[] =
{
	{IT_STRING | IT_CVAR, NULL, "Use Mouse",        &cv_usemouse,         10},


	{IT_STRING | IT_CVAR, NULL, "First-Person MouseLook", &cv_alwaysfreelook,   30},
	{IT_STRING | IT_CVAR, NULL, "Third-Person MouseLook", &cv_chasefreelook,   40},
	{IT_STRING | IT_CVAR, NULL, "Mouse Move",       &cv_mousemove,        50},
	{IT_STRING | IT_CVAR, NULL, "Invert Mouse",     &cv_invertmouse,      60},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER,
	                      NULL, "Mouse X Speed",    &cv_mousesens,        70},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER,
	                      NULL, "Mouse Y Speed",    &cv_mouseysens,        80},
};

static menuitem_t OP_Mouse2OptionsMenu[] =
{
	{IT_STRING | IT_CVAR, NULL, "Use Mouse 2",      &cv_usemouse2,        10},
	{IT_STRING | IT_CVAR, NULL, "Second Mouse Serial Port",
	                                                &cv_mouse2port,       20},
	{IT_STRING | IT_CVAR, NULL, "First-Person MouseLook", &cv_alwaysfreelook2,  30},
	{IT_STRING | IT_CVAR, NULL, "Third-Person MouseLook", &cv_chasefreelook2,  40},
	{IT_STRING | IT_CVAR, NULL, "Mouse Move",       &cv_mousemove2,       50},
	{IT_STRING | IT_CVAR, NULL, "Invert Mouse",     &cv_invertmouse2,     60},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER,
	                      NULL, "Mouse X Speed",    &cv_mousesens2,       70},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER,
	                      NULL, "Mouse Y Speed",    &cv_mouseysens2,      80},
};*/

static menuitem_t OP_VideoOptionsMenu[] =
{
	{IT_STRING | IT_CALL,	NULL,	"Set Resolution...",	M_VideoModeMenu,		 10},
#if (defined (__unix__) && !defined (MSDOS)) || defined (UNIXCOMMON) || defined (HAVE_SDL)
	{IT_STRING|IT_CVAR,		NULL,	"Fullscreen",			&cv_fullscreen,			 20},
#endif
	{IT_STRING | IT_CVAR | IT_CV_SLIDER,
							NULL,	"Gamma",				&cv_usegamma,			 30},

	{IT_STRING | IT_CVAR,	NULL,	"Draw Distance",		&cv_drawdist,			 45},
	//{IT_STRING | IT_CVAR,	NULL,	"NiGHTS Draw Dist",		&cv_drawdist_nights,	 55},
	{IT_STRING | IT_CVAR,	NULL,	"Weather Draw Distance",&cv_drawdist_precip,	 55},
	//{IT_STRING | IT_CVAR,	NULL,	"Weather Density",		&cv_precipdensity,		 65},
	{IT_STRING | IT_CVAR,	NULL,	"Skyboxes",				&cv_skybox,				 65},
	{IT_STRING | IT_CVAR,	NULL,	"Field of View",		&cv_fov,				 75},

	{IT_STRING | IT_CVAR,	NULL,	"Show FPS",				&cv_ticrate,			 90},
	{IT_STRING | IT_CVAR,	NULL,	"Vertical Sync",		&cv_vidwait,			100},

#ifdef HWRENDER
	{IT_SUBMENU|IT_STRING,	NULL,	"OpenGL Options...",	&OP_OpenGLOptionsDef,	120},
#endif
};

enum
{
	op_video_res = 0,
#if (defined (__unix__) && !defined (MSDOS)) || defined (UNIXCOMMON) || defined (HAVE_SDL)
	op_video_fullscreen,
#endif
	op_video_gamma,
	op_video_dd,
	op_video_wdd,
	//op_video_wd,
	op_video_skybox,
	op_video_fov,
	op_video_fps,
	op_video_vsync,
#ifdef HWRENDER
	op_video_ogl,
#endif
};

static menuitem_t OP_VideoModeMenu[] =
{
	{IT_KEYHANDLER | IT_NOTHING, NULL, "", M_HandleVideoMode, '\0'},     // dummy menuitem for the control func
};

#ifdef HWRENDER
static menuitem_t OP_OpenGLOptionsMenu[] =
{
	{IT_STRING | IT_CVAR,	NULL, "3D Models",					&cv_grmdls,					 10},
	{IT_STRING | IT_CVAR,	NULL, "Fallback Player 3D Model",	&cv_grfallbackplayermodel,	 20},
	{IT_STRING|IT_CVAR,		NULL, "Shaders",					&cv_grshaders,				 30},

	{IT_STRING|IT_CVAR,		NULL, "Texture Quality",			&cv_scr_depth,				 50},
	{IT_STRING|IT_CVAR,		NULL, "Texture Filter",				&cv_grfiltermode,			 60},
	{IT_STRING|IT_CVAR,		NULL, "Anisotropic",				&cv_granisotropicmode,		 70},

	{IT_STRING|IT_CVAR,		NULL, "Wall Contrast Style",		&cv_grfakecontrast,			 90},
	{IT_STRING|IT_CVAR,		NULL, "Sprite Billboarding",		&cv_grspritebillboarding,	100},
	{IT_STRING|IT_CVAR,		NULL, "Software Perspective",		&cv_grshearing,				110},

	{IT_SUBMENU|IT_STRING,	NULL, "Gamma...",					&OP_OpenGLColorDef,			130},
};

static menuitem_t OP_OpenGLColorMenu[] =
{
	{IT_STRING|IT_CVAR|IT_CV_SLIDER, NULL, "Red",   &cv_grgammared,   10},
	{IT_STRING|IT_CVAR|IT_CV_SLIDER, NULL, "Green", &cv_grgammagreen, 20},
	{IT_STRING|IT_CVAR|IT_CV_SLIDER, NULL, "Blue",  &cv_grgammablue,  30},
};
#endif

static menuitem_t OP_SoundOptionsMenu[] =
{
	{IT_STRING|IT_CVAR,			NULL, "SFX",					&cv_gamesounds,			 10},
	{IT_STRING|IT_CVAR|IT_CV_SLIDER,
								NULL, "SFX Volume",				&cv_soundvolume,		 18},

	{IT_STRING|IT_CVAR,			NULL, "Music",					&cv_gamedigimusic,		 30},
	{IT_STRING|IT_CVAR|IT_CV_SLIDER,
								NULL, "Music Volume",			&cv_digmusicvolume,		 38},

/* -- :nonnathisshit:
	{IT_STRING|IT_CVAR,			NULL, "MIDI",					&cv_gamemidimusic,		 50},
	{IT_STRING|IT_CVAR|IT_CV_SLIDER,
								NULL, "MIDI Volume",			&cv_midimusicvolume,	 58},
#ifdef PC_DOS
	{IT_STRING|IT_CVAR|IT_CV_SLIDER,
								NULL, "CD Volume",				&cd_volume,				 40},
#endif*/

	//{IT_STRING|IT_CALL,			NULL, "Restart Audio System",	M_RestartAudio,			 50},

	{IT_STRING|IT_CVAR,			NULL, "Reverse L/R Channels",	&stereoreverse,			 50},
	{IT_STRING|IT_CVAR,			NULL, "Surround Sound",			&surround,			 60},

	{IT_STRING|IT_CVAR,			NULL, "Chat Notifications",		&cv_chatnotifications,	 75},
	{IT_STRING|IT_CVAR,			NULL, "Character voices",		&cv_kartvoices,			 85},
	{IT_STRING|IT_CVAR,			NULL, "Powerup Warning",		&cv_kartinvinsfx,		 95},

	{IT_KEYHANDLER|IT_STRING,	NULL, "Sound Test",				M_HandleSoundTest,		110},

	{IT_STRING|IT_CVAR,        NULL, "Play Music While Unfocused", &cv_playmusicifunfocused, 125},
	{IT_STRING|IT_CVAR,        NULL, "Play SFX While Unfocused", &cv_playsoundifunfocused, 135},
};

static menuitem_t OP_DataOptionsMenu[] =
{

	{IT_STRING | IT_CALL,		NULL, "Screenshot Options...",	M_ScreenshotOptions,	 10},
	{IT_STRING | IT_CALL,		NULL, "Addon Options...",		M_AddonsOptions,		 20},
	{IT_STRING | IT_SUBMENU,	NULL, "Replay Options...",		&MISC_ReplayOptionsDef,	 30},
#ifdef HAVE_DISCORDRPC
	{IT_STRING | IT_SUBMENU,	NULL, "Discord Options...",		&OP_DiscordOptionsDef,	 40},

	{IT_STRING | IT_SUBMENU,	NULL, "Erase Data...",			&OP_EraseDataDef,		 60},
#else
	{IT_STRING | IT_SUBMENU,	NULL, "Erase Data...",			&OP_EraseDataDef,		 50},
#endif
};

static menuitem_t OP_ScreenshotOptionsMenu[] =
{
	{IT_STRING|IT_CVAR, NULL, "Storage Location", &cv_screenshot_option, 10},
	{IT_STRING|IT_CVAR|IT_CV_STRING, NULL, "Custom Folder", &cv_screenshot_folder, 20},

	{IT_HEADER, NULL, "Screenshots (F8)", NULL, 50},
	{IT_STRING|IT_CVAR, NULL, "Memory Level",      &cv_zlib_memory,      60},
	{IT_STRING|IT_CVAR, NULL, "Compression Level", &cv_zlib_level,       70},
	{IT_STRING|IT_CVAR, NULL, "Strategy",          &cv_zlib_strategy,    80},
	{IT_STRING|IT_CVAR, NULL, "Window Size",       &cv_zlib_window_bits, 90},

	{IT_HEADER, NULL, "Movie Mode (F9)", NULL, 105},
	{IT_STRING|IT_CVAR, NULL, "Capture Mode", &cv_moviemode, 115},

	{IT_STRING|IT_CVAR, NULL, "Region Optimizing", &cv_gif_optimize,  125},
	{IT_STRING|IT_CVAR, NULL, "Downscaling",       &cv_gif_downscale, 135},

	{IT_STRING|IT_CVAR, NULL, "Memory Level",      &cv_zlib_memorya,      125},
	{IT_STRING|IT_CVAR, NULL, "Compression Level", &cv_zlib_levela,       135},
	{IT_STRING|IT_CVAR, NULL, "Strategy",          &cv_zlib_strategya,    145},
	{IT_STRING|IT_CVAR, NULL, "Window Size",       &cv_zlib_window_bitsa, 155},
};

enum
{
	op_screenshot_folder = 1,
	op_screenshot_capture = 8,
	op_screenshot_gif_start = 9,
	op_screenshot_gif_end = 10,
	op_screenshot_apng_start = 11,
	op_screenshot_apng_end = 14,
};

static menuitem_t OP_EraseDataMenu[] =
{
	{IT_STRING | IT_CALL, NULL, "Erase Record Data", M_EraseData, 10},
	{IT_STRING | IT_CALL, NULL, "Erase Unlockable Data", M_EraseData, 20},

	{IT_STRING | IT_CALL, NULL, "\x85" "Erase ALL Data", M_EraseData, 40},
};

static menuitem_t OP_AddonsOptionsMenu[] =
{
	{IT_HEADER,                      NULL, "Menu",                        NULL,                    0},
	{IT_STRING|IT_CVAR,              NULL, "Location",                    &cv_addons_option,      10},
	{IT_STRING|IT_CVAR|IT_CV_STRING, NULL, "Custom Folder",               &cv_addons_folder,      20},
	{IT_STRING|IT_CVAR,              NULL, "Identify addons via",         &cv_addons_md5,         48},
	{IT_STRING|IT_CVAR,              NULL, "Show unsupported file types", &cv_addons_showall,     58},

	{IT_HEADER,                      NULL, "Search",                      NULL,                   76},
	{IT_STRING|IT_CVAR,              NULL, "Matching",                    &cv_addons_search_type, 86},
	{IT_STRING|IT_CVAR,              NULL, "Case-sensitive",              &cv_addons_search_case, 96},
};

enum
{
	op_addons_folder = 2,
};

#ifdef HAVE_DISCORDRPC
static menuitem_t OP_DiscordOptionsMenu[] =
{
	{IT_STRING | IT_CVAR,		NULL, "Rich Presence",			&cv_discordrp,			 10},

	{IT_HEADER,					NULL, "Rich Presence Settings",	NULL,					 30},
	{IT_STRING | IT_CVAR,		NULL, "Streamer Mode",			&cv_discordstreamer,	 40},

	{IT_STRING | IT_CVAR,		NULL, "Allow Ask To Join",		&cv_discordasks,		 60},
	{IT_STRING | IT_CVAR,		NULL, "Allow Invites",			&cv_discordinvites,		 70},
};
#endif

static menuitem_t OP_HUDOptionsMenu[] =
{
	{IT_STRING | IT_CVAR, NULL, "Show HUD (F3)",			&cv_showhud,			 10},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER,
	                      NULL, "HUD Visibility",			&cv_translucenthud,		 20},

	{IT_STRING | IT_SUBMENU, NULL, "Online HUD options...",&OP_ChatOptionsDef, 	 35},
	{IT_STRING | IT_CVAR, NULL, "Background Glass",			&cons_backcolor,		 45},

	{IT_STRING | IT_CVAR | IT_CV_SLIDER,
						  NULL, "Minimap Visibility",		&cv_kartminimap,		 60},
	{IT_STRING | IT_CVAR, NULL, "Speedometer Display",		&cv_kartspeedometer,	 70},
	{IT_STRING | IT_CVAR, NULL, "Show \"CHECK\"",			&cv_kartcheck,			 80},

	{IT_STRING | IT_CVAR, NULL,	"Menu Highlights",			&cons_menuhighlight,     95},
	// highlight info - (GOOD HIGHLIGHT, WARNING HIGHLIGHT) - 105 (see M_DrawHUDOptions)

	{IT_STRING | IT_CVAR, NULL,	"Console Text Size",		&cv_constextsize,		120},

	{IT_STRING | IT_CVAR, NULL,   "Show \"FOCUS LOST\"",  &cv_showfocuslost,   135},
};

// Ok it's still called chatoptions but we'll put ping display in here to be clean
static menuitem_t OP_ChatOptionsMenu[] =
{
	// will ANYONE who doesn't know how to use the console want to touch this one?
	{IT_STRING | IT_CVAR, NULL, "Chat Mode",				&cv_consolechat,		10}, // nonetheless...

	{IT_STRING | IT_CVAR | IT_CV_SLIDER,
	                      NULL, "Chat Box Width",			&cv_chatwidth,			25},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER,
	                      NULL, "Chat Box Height",			&cv_chatheight,			35},

	{IT_STRING | IT_CVAR, NULL, "Chat Background Tint",		&cv_chatbacktint,		50},
	{IT_STRING | IT_CVAR, NULL, "Message Fadeout Time",		&cv_chattime,			60},
	{IT_STRING | IT_CVAR, NULL, "Spam Protection",			&cv_chatspamprotection,	70},

	{IT_STRING | IT_CVAR, NULL, "Local ping display",		&cv_showping,			90},	// shows ping next to framerate if we want to.
};

static menuitem_t OP_GameOptionsMenu[] =
{
	{IT_STRING | IT_SUBMENU, NULL, "Random Item Toggles...",	&OP_MonitorToggleDef,	 10},

	{IT_STRING | IT_CVAR, NULL, "Game Speed",					&cv_kartspeed,			 30},
	{IT_STRING | IT_CVAR, NULL, "Frantic Items",				&cv_kartfrantic,		 40},
	{IT_SECRET,           NULL, "Encore Mode",					&cv_kartencore,			 50},

	{IT_STRING | IT_CVAR, NULL, "Number of Laps",				&cv_basenumlaps,		 70},
	{IT_STRING | IT_CVAR, NULL, "Exit Countdown Timer",			&cv_countdowntime,		 80},

	{IT_STRING | IT_CVAR, NULL, "Time Limit",					&cv_timelimit,			100},
	{IT_STRING | IT_CVAR, NULL, "Starting Bumpers",				&cv_kartbumpers,		110},
	{IT_STRING | IT_CVAR, NULL, "Karma Comeback",				&cv_kartcomeback,		120},

	{IT_STRING | IT_CVAR, NULL, "Force Character",				&cv_forceskin,          140},
	{IT_STRING | IT_CVAR, NULL, "Restrict Character Changes",	&cv_restrictskinchange, 150},
};

static menuitem_t OP_ServerOptionsMenu[] =
{
#ifndef NONET
	{IT_STRING | IT_CVAR | IT_CV_STRING,
	                         NULL, "Server Name",					&cv_servername,			 10},
#endif

	{IT_STRING | IT_CVAR,    NULL, "Intermission Timer",			&cv_inttime,			 40},
	{IT_STRING | IT_CVAR,    NULL, "Map Progression",				&cv_advancemap,			 50},
	{IT_STRING | IT_CVAR,    NULL, "Voting Timer",					&cv_votetime,			 60},
	{IT_STRING | IT_CVAR,    NULL, "Voting Rule Changes",			&cv_kartvoterulechanges, 70},

#ifndef NONET
	{IT_STRING | IT_CVAR,    NULL, "Max. Player Count",				&cv_maxplayers,			 90},
	{IT_STRING | IT_CVAR,    NULL, "Allow Players to Join",			&cv_allownewplayer,		100},
	{IT_STRING | IT_CVAR,    NULL, "Allow Addon Downloading",		&cv_downloading,		110},
	{IT_STRING | IT_CVAR,    NULL, "Pause Permission",				&cv_pause,				120},
	{IT_STRING | IT_CVAR,    NULL, "Mute All Chat",					&cv_mute,				130},

	{IT_SUBMENU|IT_STRING,   NULL, "Advanced Options...",			&OP_AdvServerOptionsDef,150},
#endif
};

#ifndef NONET
static menuitem_t OP_AdvServerOptionsMenu[] =
{
	{IT_STRING | IT_CVAR | IT_CV_STRING,
	                         NULL, "Server Browser Address",		&cv_masterserver,		 10},

	{IT_STRING | IT_CVAR,    NULL, "Attempts to resynchronise",		&cv_resynchattempts,	 40},
	{IT_STRING | IT_CVAR,    NULL, "Ping limit (ms)",				&cv_maxping,			 50},
	{IT_STRING | IT_CVAR,    NULL, "Ping timeout (s)",				&cv_pingtimeout,		 60},
	{IT_STRING | IT_CVAR,    NULL, "Connection timeout (tics)",		&cv_nettimeout,			 70},
	{IT_STRING | IT_CVAR,    NULL, "Join timeout (tics)",			&cv_jointimeout,		 80},

	{IT_STRING | IT_CVAR,    NULL, "Max. file transfer send (KB)",	&cv_maxsend,			100},
	{IT_STRING | IT_CVAR,    NULL, "File transfer packet rate",		&cv_downloadspeed,		110},

	{IT_STRING | IT_CVAR,    NULL, "Log join addresses",			&cv_showjoinaddress,	130},
	{IT_STRING | IT_CVAR,    NULL, "Log resyncs",					&cv_blamecfail,			140},
	{IT_STRING | IT_CVAR,    NULL, "Log file transfers",			&cv_noticedownload,		150},
};
#endif

/*static menuitem_t OP_NetgameOptionsMenu[] =
{
	{IT_STRING | IT_CVAR, NULL, "Time Limit",            &cv_timelimit,        10},
	{IT_STRING | IT_CVAR, NULL, "Point Limit",           &cv_pointlimit,       18},

	{IT_STRING | IT_CVAR, NULL, "Frantic Items",         &cv_kartfrantic,      34},

	{IT_STRING | IT_CVAR, NULL, "Item Respawn",          &cv_itemrespawn,      50},
	{IT_STRING | IT_CVAR, NULL, "Item Respawn Delay",     &cv_itemrespawntime,  58},

	{IT_STRING | IT_CVAR, NULL, "Player Respawn Delay",  &cv_respawntime,      74},

	{IT_STRING | IT_CVAR, NULL, "Force Skin #",          &cv_forceskin,          90},
	{IT_STRING | IT_CVAR, NULL, "Restrict Skin Changes", &cv_restrictskinchange, 98},

	//{IT_STRING | IT_CVAR, NULL, "Autobalance Teams",            &cv_autobalance,      114},
	//{IT_STRING | IT_CVAR, NULL, "Scramble Teams on Map Change", &cv_scrambleonchange, 122},
};*/

/*static menuitem_t OP_GametypeOptionsMenu[] =
{
	{IT_HEADER,           NULL, "RACE",                  NULL,                 2},
	{IT_STRING | IT_CVAR, NULL, "Game Speed",    		  &cv_kartspeed,    	10},
	{IT_STRING | IT_CVAR, NULL, "Encore Mode",    		  &cv_kartencore,    	18},
	{IT_STRING | IT_CVAR, NULL, "Number of Laps",        &cv_numlaps,          26},
	{IT_STRING | IT_CVAR, NULL, "Use Map Lap Counts",    &cv_usemapnumlaps,    34},

	{IT_HEADER,           NULL, "BATTLE",                NULL,                 50},
	{IT_STRING | IT_CVAR, NULL, "Starting Bumpers",     &cv_kartbumpers,     58},
	{IT_STRING | IT_CVAR, NULL, "Karma Comeback",        &cv_kartcomeback,     66},
};*/

#define ITEMTOGGLEBOTTOMRIGHT

static menuitem_t OP_MonitorToggleMenu[] =
{
	// Mostly handled by the drawing function.
	// Instead of using this for dumb monitors, lets use the new item bools we have :V
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Sneakers",				M_HandleMonitorToggles, KITEM_SNEAKER},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Sneakers x3",			M_HandleMonitorToggles, KRITEM_TRIPLESNEAKER},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Rocket Sneakers",		M_HandleMonitorToggles, KITEM_ROCKETSNEAKER},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Toggle All",			M_HandleMonitorToggles, 0},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Bananas",				M_HandleMonitorToggles, KITEM_BANANA},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Bananas x3",			M_HandleMonitorToggles, KRITEM_TRIPLEBANANA},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Bananas x10",			M_HandleMonitorToggles, KRITEM_TENFOLDBANANA},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Eggman Monitors",		M_HandleMonitorToggles, KITEM_EGGMAN},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Orbinauts",				M_HandleMonitorToggles, KITEM_ORBINAUT},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Orbinauts x3",			M_HandleMonitorToggles, KRITEM_TRIPLEORBINAUT},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Orbinauts x4",			M_HandleMonitorToggles, KRITEM_QUADORBINAUT},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Mines",					M_HandleMonitorToggles, KITEM_MINE},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Jawz",					M_HandleMonitorToggles, KITEM_JAWZ},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Jawz x2",				M_HandleMonitorToggles, KRITEM_DUALJAWZ},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Ballhogs",				M_HandleMonitorToggles, KITEM_BALLHOG},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Self-Propelled Bombs",	M_HandleMonitorToggles, KITEM_SPB},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Invinciblity",			M_HandleMonitorToggles, KITEM_INVINCIBILITY},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Grow",					M_HandleMonitorToggles, KITEM_GROW},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Shrink",				M_HandleMonitorToggles, KITEM_SHRINK},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Thunder Shields",		M_HandleMonitorToggles, KITEM_THUNDERSHIELD},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Hyudoros",				M_HandleMonitorToggles, KITEM_HYUDORO},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Pogo Springs",		 	M_HandleMonitorToggles, KITEM_POGOSPRING},
	{IT_KEYHANDLER | IT_NOTHING, NULL, "Kitchen Sinks",			M_HandleMonitorToggles, KITEM_KITCHENSINK},
#ifdef ITEMTOGGLEBOTTOMRIGHT
	{IT_KEYHANDLER | IT_NOTHING, NULL, "---",					M_HandleMonitorToggles, 255},
#endif
};

// ==========================================================================
// ALL MENU DEFINITIONS GO HERE
// ==========================================================================

// Main Menu and related
menu_t MainDef = CENTERMENUSTYLE(NULL, MainMenu, NULL, 72);

menu_t MISC_AddonsDef =
{
	NULL,
	sizeof (MISC_AddonsMenu)/sizeof (menuitem_t),
	&OP_DataOptionsDef,
	MISC_AddonsMenu,
	M_DrawAddons,
	50, 28,
	0,
	NULL
};

menu_t MISC_ReplayHutDef =
{
	NULL,
	sizeof (MISC_ReplayHutMenu)/sizeof (menuitem_t),
	NULL,
	MISC_ReplayHutMenu,
	M_DrawReplayHut,
	30, 80,
	0,
	M_QuitReplayHut
};

menu_t MISC_ReplayOptionsDef =
{
	"M_REPOPT",
	sizeof (MISC_ReplayOptionsMenu)/sizeof (menuitem_t),
	&OP_DataOptionsDef,
	MISC_ReplayOptionsMenu,
	M_DrawGenericMenu,
	27, 40,
	0,
	NULL
};

menu_t MISC_ReplayStartDef =
{
	NULL,
	sizeof (MISC_ReplayStartMenu)/sizeof (menuitem_t),
	&MISC_ReplayHutDef,
	MISC_ReplayStartMenu,
	M_DrawReplayStartMenu,
	30, 90,
	0,
	NULL
};

menu_t PlaybackMenuDef = {
	NULL,
	sizeof (PlaybackMenu)/sizeof (menuitem_t),
	NULL,
	PlaybackMenu,
	M_DrawPlaybackMenu,
	//BASEVIDWIDTH/2 - 94, 2,
	BASEVIDWIDTH/2 - 88, 2,
	0,
	NULL
};

menu_t MAPauseDef = PAUSEMENUSTYLE(MAPauseMenu, 40, 72);
menu_t SPauseDef = PAUSEMENUSTYLE(SPauseMenu, 40, 72);
menu_t MPauseDef = PAUSEMENUSTYLE(MPauseMenu, 40, 72);

#ifdef HAVE_DISCORDRPC
menu_t MISC_DiscordRequestsDef = {
	NULL,
	sizeof (MISC_DiscordRequestsMenu)/sizeof (menuitem_t),
	&MPauseDef,
	MISC_DiscordRequestsMenu,
	M_DrawDiscordRequests,
	0, 0,
	0,
	NULL
};
#endif

// Misc Main Menu
menu_t MISC_ScrambleTeamDef = DEFAULTMENUSTYLE(NULL, MISC_ScrambleTeamMenu, &MPauseDef, 27, 40);
menu_t MISC_ChangeTeamDef = DEFAULTMENUSTYLE(NULL, MISC_ChangeTeamMenu, &MPauseDef, 27, 40);
menu_t MISC_ChangeSpectateDef = DEFAULTMENUSTYLE(NULL, MISC_ChangeSpectateMenu, &MPauseDef, 27, 40);
menu_t MISC_ChangeLevelDef = MAPICONMENUSTYLE(NULL, MISC_ChangeLevelMenu, &MPauseDef);
menu_t MISC_HelpDef = IMAGEDEF(MISC_HelpMenu);

//
// M_GetGametypeColor
//
// Pretty and consistent ^u^
// See also G_GetGametypeColor.
//

static INT32 highlightflags, recommendedflags, warningflags;

inline static void M_GetGametypeColor(void)
{
	INT16 gt;

	warningflags = V_REDMAP;
	recommendedflags = V_GREENMAP;

	if (cons_menuhighlight.value)
	{
		highlightflags = cons_menuhighlight.value;
		if (highlightflags == V_REDMAP)
		{
			warningflags = V_ORANGEMAP;
			return;
		}
		if (highlightflags == V_GREENMAP)
		{
			recommendedflags = V_SKYMAP;
			return;
		}
		return;
	}

	warningflags = V_REDMAP;
	recommendedflags = V_GREENMAP;

	if (modeattacking // == ATTACKING_RECORD
		|| gamestate == GS_TIMEATTACK)
	{
		highlightflags = V_ORANGEMAP;
		return;
	}

	if (currentMenu->drawroutine == M_DrawServerMenu)
		gt = cv_newgametype.value;
	else if (!Playing())
	{
		highlightflags = V_YELLOWMAP;
		return;
	}
	else
		gt = gametype;

	if (gt == GT_MATCH)
	{
		highlightflags = V_REDMAP;
		warningflags = V_ORANGEMAP;
		return;
	}
	if (gt == GT_RACE)
	{
		highlightflags = V_SKYMAP;
		return;
	}

	highlightflags = V_YELLOWMAP; // FALLBACK
}

// excuse me but I'm extremely lazy:
INT32 HU_GetHighlightColor(void)
{
	M_GetGametypeColor();	// update flag colour reguardless of the menu being opened or not.
	return highlightflags;
}

// Sky Room
menu_t SR_PandoraDef =
{
	"M_PANDRA",
	sizeof (SR_PandorasBox)/sizeof (menuitem_t),
	&SPauseDef,
	SR_PandorasBox,
	M_DrawGenericMenu,
	60, 40,
	0,
	M_ExitPandorasBox
};
menu_t SR_MainDef = CENTERMENUSTYLE(NULL, SR_MainMenu, &MainDef, 72);

//menu_t SR_LevelSelectDef = MAPICONMENUSTYLE(NULL, SR_LevelSelectMenu, &SR_MainDef);

menu_t SR_UnlockChecklistDef =
{
	NULL,
	1,
	&SR_MainDef,
	SR_UnlockChecklistMenu,
	M_DrawChecklist,
	280, 185,
	0,
	NULL
};
menu_t SR_EmblemHintDef =
{
	NULL,
	sizeof (SR_EmblemHintMenu)/sizeof (menuitem_t),
	&SPauseDef,
	SR_EmblemHintMenu,
	M_DrawEmblemHints,
	60, 150,
	0,
	NULL
};

// Single Player
menu_t SP_MainDef = CENTERMENUSTYLE(NULL, SP_MainMenu, &MainDef, 72);
/*menu_t SP_LoadDef =
{
	"M_PICKG",
	1,
	&SP_MainDef,
	SP_LoadGameMenu,
	M_DrawLoad,
	68, 46,
	0,
	NULL
};
menu_t SP_LevelSelectDef = MAPICONMENUSTYLE(NULL, SP_LevelSelectMenu, &SP_LoadDef);*/

menu_t SP_LevelStatsDef =
{
	"M_STATS",
	1,
	&SR_MainDef,
	SP_LevelStatsMenu,
	M_DrawLevelStats,
	280, 185,
	0,
	NULL
};

static menu_t SP_TimeAttackDef =
{
	"M_ATTACK",
	sizeof (SP_TimeAttackMenu)/sizeof (menuitem_t),
	&MainDef,  // Doesn't matter.
	SP_TimeAttackMenu,
	M_DrawTimeAttackMenu,
	34, 40,
	0,
	M_QuitTimeAttackMenu
};
static menu_t SP_ReplayDef =
{
	"M_ATTACK",
	sizeof(SP_ReplayMenu)/sizeof(menuitem_t),
	&SP_TimeAttackDef,
	SP_ReplayMenu,
	M_DrawTimeAttackMenu,
	34, 40,
	0,
	NULL
};
static menu_t SP_GuestReplayDef =
{
	"M_ATTACK",
	sizeof(SP_GuestReplayMenu)/sizeof(menuitem_t),
	&SP_TimeAttackDef,
	SP_GuestReplayMenu,
	M_DrawTimeAttackMenu,
	34, 40,
	0,
	NULL
};
static menu_t SP_GhostDef =
{
	"M_ATTACK",
	sizeof(SP_GhostMenu)/sizeof(menuitem_t),
	&SP_TimeAttackDef,
	SP_GhostMenu,
	M_DrawTimeAttackMenu,
	34, 40,
	0,
	NULL
};

/*static menu_t SP_NightsAttackDef =
{
	"M_NIGHTS",
	sizeof (SP_NightsAttackMenu)/sizeof (menuitem_t),
	&MainDef,  // Doesn't matter.
	SP_NightsAttackMenu,
	M_DrawNightsAttackMenu,
	32, 40,
	0,
	NULL
};
static menu_t SP_NightsReplayDef =
{
	"M_NIGHTS",
	sizeof(SP_NightsReplayMenu)/sizeof(menuitem_t),
	&SP_NightsAttackDef,
	SP_NightsReplayMenu,
	M_DrawNightsAttackMenu,
	32, 120,
	0,
	NULL
};
static menu_t SP_NightsGuestReplayDef =
{
	"M_NIGHTS",
	sizeof(SP_NightsGuestReplayMenu)/sizeof(menuitem_t),
	&SP_NightsAttackDef,
	SP_NightsGuestReplayMenu,
	M_DrawNightsAttackMenu,
	32, 120,
	0,
	NULL
};
static menu_t SP_NightsGhostDef =
{
	"M_NIGHTS",
	sizeof(SP_NightsGhostMenu)/sizeof(menuitem_t),
	&SP_NightsAttackDef,
	SP_NightsGhostMenu,
	M_DrawNightsAttackMenu,
	32, 120,
	0,
	NULL
};*/


/*menu_t SP_PlayerDef =
{
	"M_PICKP",
	sizeof (PlayerMenu)/sizeof (menuitem_t),//player_end,
	&SP_MainDef,
	PlayerMenu,
	M_DrawSetupChoosePlayerMenu,
	24, 32,
	0,
	NULL
};*/

// Multiplayer
menu_t MP_MainDef =
{
	"M_MULTI",
	sizeof (MP_MainMenu)/sizeof (menuitem_t),
	&MainDef,
	MP_MainMenu,
	M_DrawMPMainMenu,
	42, 30,
	0,
#ifndef NONET
	M_CancelConnect
#else
	NULL
#endif
};

menu_t MP_OfflineServerDef = MAPICONMENUSTYLE("M_MULTI", MP_OfflineServerMenu, &MP_MainDef);

#ifndef NONET
menu_t MP_ServerDef = MAPICONMENUSTYLE("M_MULTI", MP_ServerMenu, &MP_MainDef);

menu_t MP_ConnectDef =
{
	"M_MULTI",
	sizeof (MP_ConnectMenu)/sizeof (menuitem_t),
	&MP_MainDef,
	MP_ConnectMenu,
	M_DrawConnectMenu,
	27,24,
	0,
	M_CancelConnect
};
#endif
menu_t MP_PlayerSetupDef =
{
	NULL, //"M_SPLAYR"
	sizeof (MP_PlayerSetupMenu)/sizeof (menuitem_t),
	&MP_MainDef,
	MP_PlayerSetupMenu,
	M_DrawSetupMultiPlayerMenu,
	36, 14,
	0,
	M_QuitMultiPlayerMenu
};

// Options
menu_t OP_MainDef =
{
	"M_OPTTTL",
	sizeof (OP_MainMenu)/sizeof (menuitem_t),
	&MainDef,
	OP_MainMenu,
	M_DrawGenericMenu,
	60, 30,
	0,
	NULL
};

menu_t OP_ControlsDef = DEFAULTMENUSTYLE("M_CONTRO", OP_ControlsMenu, &OP_MainDef, 60, 30);
menu_t OP_AllControlsDef = CONTROLMENUSTYLE(OP_AllControlsMenu, &OP_ControlsDef);
menu_t OP_Joystick1Def = DEFAULTMENUSTYLE("M_CONTRO", OP_Joystick1Menu, &OP_AllControlsDef, 60, 30);
menu_t OP_Joystick2Def = DEFAULTMENUSTYLE("M_CONTRO", OP_Joystick2Menu, &OP_AllControlsDef, 60, 30);
menu_t OP_Joystick3Def = DEFAULTMENUSTYLE("M_CONTRO", OP_Joystick3Menu, &OP_AllControlsDef, 60, 30);
menu_t OP_Joystick4Def = DEFAULTMENUSTYLE("M_CONTRO", OP_Joystick4Menu, &OP_AllControlsDef, 60, 30);
menu_t OP_JoystickSetDef =
{
	"M_CONTRO",
	sizeof (OP_JoystickSetMenu)/sizeof (menuitem_t),
	&OP_Joystick1Def,
	OP_JoystickSetMenu,
	M_DrawJoystick,
	50, 40,
	0,
	NULL
};

menu_t OP_VideoOptionsDef =
{
	"M_VIDEO",
	sizeof(OP_VideoOptionsMenu)/sizeof(menuitem_t),
	&OP_MainDef,
	OP_VideoOptionsMenu,
	M_DrawVideoMenu,
	30, 30,
	0,
	NULL
};

menu_t OP_VideoModeDef =
{
	"M_VIDEO",
	1,
	&OP_VideoOptionsDef,
	OP_VideoModeMenu,
	M_DrawVideoMode,
	48, 26,
	0,
	NULL
};

menu_t OP_SoundOptionsDef =
{
	"M_SOUND",
	sizeof (OP_SoundOptionsMenu)/sizeof (menuitem_t),
	&OP_MainDef,
	OP_SoundOptionsMenu,
	M_DrawSkyRoom,
	30, 30,
	0,
	NULL
};

menu_t OP_HUDOptionsDef =
{
	"M_HUD",
	sizeof (OP_HUDOptionsMenu)/sizeof (menuitem_t),
	&OP_MainDef,
	OP_HUDOptionsMenu,
	M_DrawHUDOptions,
	30, 30,
	0,
	NULL
};

menu_t OP_ChatOptionsDef = DEFAULTMENUSTYLE("M_HUD", OP_ChatOptionsMenu, &OP_HUDOptionsDef, 30, 30);

menu_t OP_GameOptionsDef = DEFAULTMENUSTYLE("M_GAME", OP_GameOptionsMenu, &OP_MainDef, 30, 30);
menu_t OP_ServerOptionsDef = DEFAULTMENUSTYLE("M_SERVER", OP_ServerOptionsMenu, &OP_MainDef, 24, 30);
#ifndef NONET
menu_t OP_AdvServerOptionsDef = DEFAULTMENUSTYLE("M_SERVER", OP_AdvServerOptionsMenu, &OP_ServerOptionsDef, 24, 30);
#endif

//menu_t OP_NetgameOptionsDef = DEFAULTMENUSTYLE("M_SERVER", OP_NetgameOptionsMenu, &OP_ServerOptionsDef, 30, 30);
//menu_t OP_GametypeOptionsDef = DEFAULTMENUSTYLE("M_SERVER", OP_GametypeOptionsMenu, &OP_ServerOptionsDef, 30, 30);
//menu_t OP_ChatOptionsDef = DEFAULTMENUSTYLE("M_GAME", OP_ChatOptionsMenu, &OP_GameOptionsDef, 30, 30);
menu_t OP_MonitorToggleDef =
{
	"M_GAME",
	sizeof (OP_MonitorToggleMenu)/sizeof (menuitem_t),
	&OP_GameOptionsDef,
	OP_MonitorToggleMenu,
	M_DrawMonitorToggles,
	47, 30,
	0,
	NULL
};

#ifdef HWRENDER
menu_t OP_OpenGLOptionsDef = DEFAULTMENUSTYLE("M_VIDEO", OP_OpenGLOptionsMenu, &OP_VideoOptionsDef, 30, 30);
menu_t OP_OpenGLColorDef =
{
	"M_VIDEO",
	sizeof (OP_OpenGLColorMenu)/sizeof (menuitem_t),
	&OP_OpenGLOptionsDef,
	OP_OpenGLColorMenu,
	M_OGL_DrawColorMenu,
	60, 40,
	0,
	NULL
};
#endif
menu_t OP_DataOptionsDef = DEFAULTMENUSTYLE("M_DATA", OP_DataOptionsMenu, &OP_MainDef, 60, 30);
menu_t OP_ScreenshotOptionsDef = DEFAULTMENUSTYLE("M_SCSHOT", OP_ScreenshotOptionsMenu, &OP_DataOptionsDef, 30, 30);
menu_t OP_AddonsOptionsDef = DEFAULTMENUSTYLE("M_ADDONS", OP_AddonsOptionsMenu, &OP_DataOptionsDef, 30, 30);
#ifdef HAVE_DISCORDRPC
menu_t OP_DiscordOptionsDef = DEFAULTMENUSTYLE(NULL, OP_DiscordOptionsMenu, &OP_DataOptionsDef, 30, 30);
#endif
menu_t OP_EraseDataDef = DEFAULTMENUSTYLE("M_DATA", OP_EraseDataMenu, &OP_DataOptionsDef, 30, 30);

// ==========================================================================
// CVAR ONCHANGE EVENTS GO HERE
// ==========================================================================
// (there's only a couple anyway)

// Prototypes
static INT32 M_FindFirstMap(INT32 gtype);
static INT32 M_GetFirstLevelInList(void);

// Nextmap.  Used for Time Attack.
static void Nextmap_OnChange(void)
{
	char *leveltitle;
	UINT8 active;

	// Update the string in the consvar.
	Z_Free(cv_nextmap.zstring);
	leveltitle = G_BuildMapTitle(cv_nextmap.value);
	cv_nextmap.string = cv_nextmap.zstring = leveltitle ? leveltitle : Z_StrDup(G_BuildMapName(cv_nextmap.value));

	if (currentMenu == &SP_TimeAttackDef)
	{
		// see also p_setup.c's P_LoadRecordGhosts
		const size_t glen = strlen(srb2home)+1+strlen("replay")+1+strlen(timeattackfolder)+1+strlen("MAPXX")+1;
		char *gpath = malloc(glen);
		INT32 i;

		if (!gpath)
			return;

		sprintf(gpath,"%s"PATHSEP"replay"PATHSEP"%s"PATHSEP"%s", srb2home, timeattackfolder, G_BuildMapName(cv_nextmap.value));

		CV_StealthSetValue(&cv_dummystaff, 0);

		active = false;
		SP_TimeAttackMenu[taguest].status = IT_DISABLED;
		SP_TimeAttackMenu[tareplay].status = IT_DISABLED;
		//SP_TimeAttackMenu[taghost].status = IT_DISABLED;

		// Check if file exists, if not, disable REPLAY option
		for (i = 0; i < 4; i++)
		{
			SP_ReplayMenu[i].status = IT_DISABLED;
			SP_GuestReplayMenu[i].status = IT_DISABLED;
		}
		SP_ReplayMenu[4].status = IT_DISABLED;

		SP_GhostMenu[3].status = IT_DISABLED;
		SP_GhostMenu[4].status = IT_DISABLED;

		if (FIL_FileExists(va("%s-%s-time-best.lmp", gpath, cv_chooseskin.string))) {
			SP_ReplayMenu[0].status = IT_WHITESTRING|IT_CALL;
			SP_GuestReplayMenu[0].status = IT_WHITESTRING|IT_CALL;
			active |= 3;
		}
		if (FIL_FileExists(va("%s-%s-lap-best.lmp", gpath, cv_chooseskin.string))) {
			SP_ReplayMenu[1].status = IT_WHITESTRING|IT_CALL;
			SP_GuestReplayMenu[1].status = IT_WHITESTRING|IT_CALL;
			active |= 3;
		}
		if (FIL_FileExists(va("%s-%s-last.lmp", gpath, cv_chooseskin.string))) {
			SP_ReplayMenu[2].status = IT_WHITESTRING|IT_CALL;
			SP_GuestReplayMenu[2].status = IT_WHITESTRING|IT_CALL;
			active |= 3;
		}

		if (FIL_FileExists(va("%s-guest.lmp", gpath)))
		{
			SP_ReplayMenu[3].status = IT_WHITESTRING|IT_CALL;
			SP_GuestReplayMenu[3].status = IT_WHITESTRING|IT_CALL;
			SP_GhostMenu[3].status = IT_STRING|IT_CVAR;
			active |= 3;
		}

		CV_SetValue(&cv_dummystaff, 1);
		if (cv_dummystaff.value)
		{
			SP_ReplayMenu[4].status = IT_WHITESTRING|IT_KEYHANDLER;
			SP_GhostMenu[4].status = IT_STRING|IT_CVAR;
			CV_StealthSetValue(&cv_dummystaff, 1);
			active |= 1;
		}

		if (active) {
			if (active & 1)
				SP_TimeAttackMenu[tareplay].status = IT_WHITESTRING|IT_SUBMENU;
			if (active & 2)
				SP_TimeAttackMenu[taguest].status = IT_WHITESTRING|IT_SUBMENU;
		}
		else if (itemOn == tareplay) // Reset lastOn so replay isn't still selected when not available.
		{
			currentMenu->lastOn = itemOn;
			itemOn = tastart;
		}

		if (mapheaderinfo[cv_nextmap.value-1] && mapheaderinfo[cv_nextmap.value-1]->forcecharacter[0] != '\0')
			CV_Set(&cv_chooseskin, mapheaderinfo[cv_nextmap.value-1]->forcecharacter);

		free(gpath);
	}
}

static void Dummymenuplayer_OnChange(void)
{
	if (cv_dummymenuplayer.value < 1)
		CV_StealthSetValue(&cv_dummymenuplayer, splitscreen+1);
	else if (cv_dummymenuplayer.value > splitscreen+1)
		CV_StealthSetValue(&cv_dummymenuplayer, 1);
}

/*static void Dummymares_OnChange(void)
{
	if (!nightsrecords[cv_nextmap.value-1])
	{
		CV_StealthSetValue(&cv_dummymares, 0);
		return;
	}
	else
	{
		UINT8 mares = nightsrecords[cv_nextmap.value-1]->nummares;

		if (cv_dummymares.value < 0)
			CV_StealthSetValue(&cv_dummymares, mares);
		else if (cv_dummymares.value > mares)
			CV_StealthSetValue(&cv_dummymares, 0);
	}
}*/

char dummystaffname[22];

static void Dummystaff_OnChange(void)
{
	lumpnum_t l;

	dummystaffname[0] = '\0';

	if ((l = W_CheckNumForName(va("%sS01",G_BuildMapName(cv_nextmap.value)))) == LUMPERROR)
	{
		CV_StealthSetValue(&cv_dummystaff, 0);
		return;
	}
	else
	{
		char *temp = dummystaffname;
		UINT8 numstaff = 1;
		while (numstaff < 99 && (l = W_CheckNumForName(va("%sS%02u",G_BuildMapName(cv_nextmap.value),numstaff+1))) != LUMPERROR)
			numstaff++;

		if (cv_dummystaff.value < 1)
			CV_StealthSetValue(&cv_dummystaff, numstaff);
		else if (cv_dummystaff.value > numstaff)
			CV_StealthSetValue(&cv_dummystaff, 1);

		if ((l = W_CheckNumForName(va("%sS%02u",G_BuildMapName(cv_nextmap.value), cv_dummystaff.value))) == LUMPERROR)
			return; // shouldn't happen but might as well check...

		G_UpdateStaffGhostName(l);

		while (*temp)
			temp++;

		sprintf(temp, " - %d", cv_dummystaff.value);
	}
}

// Newgametype.  Used for gametype changes.
static void Newgametype_OnChange(void)
{
	if (cv_nextmap.value && menuactive)
	{
		if (!mapheaderinfo[cv_nextmap.value-1])
			P_AllocMapHeader((INT16)(cv_nextmap.value-1));

		if ((cv_newgametype.value == GT_RACE && !(mapheaderinfo[cv_nextmap.value-1]->typeoflevel & TOL_RACE)) || // SRB2kart
			//(cv_newgametype.value == GT_COMPETITION && !(mapheaderinfo[cv_nextmap.value-1]->typeoflevel & TOL_COMPETITION)) ||
			//(cv_newgametype.value == GT_RACE && !(mapheaderinfo[cv_nextmap.value-1]->typeoflevel & TOL_RACE)) ||
			((cv_newgametype.value == GT_MATCH || cv_newgametype.value == GT_TEAMMATCH) && !(mapheaderinfo[cv_nextmap.value-1]->typeoflevel & TOL_MATCH))) // ||
			//((cv_newgametype.value == GT_TAG || cv_newgametype.value == GT_HIDEANDSEEK) && !(mapheaderinfo[cv_nextmap.value-1]->typeoflevel & TOL_TAG)) ||
			//(cv_newgametype.value == GT_CTF && !(mapheaderinfo[cv_nextmap.value-1]->typeoflevel & TOL_CTF)))
		{
			INT32 value = 0;

			switch (cv_newgametype.value)
			{
				case GT_COOP:
					value = TOL_RACE; // SRB2kart
					break;
				case GT_COMPETITION:
					value = TOL_COMPETITION;
					break;
				case GT_RACE:
					value = TOL_RACE;
					break;
				case GT_MATCH:
				case GT_TEAMMATCH:
					value = TOL_MATCH;
					break;
				case GT_TAG:
				case GT_HIDEANDSEEK:
					value = TOL_TAG;
					break;
				case GT_CTF:
					value = TOL_CTF;
					break;
			}

			CV_SetValue(&cv_nextmap, M_FindFirstMap(value));
			//CV_AddValue(&cv_nextmap, -1);
			//CV_AddValue(&cv_nextmap, 1);
		}
	}
}

void Screenshot_option_Onchange(void)
{
	OP_ScreenshotOptionsMenu[op_screenshot_folder].status =
		(cv_screenshot_option.value == 3 ? IT_CVAR|IT_STRING|IT_CV_STRING : IT_DISABLED);
}

void Moviemode_mode_Onchange(void)
{
	INT32 i, cstart, cend;
	for (i = op_screenshot_gif_start; i <= op_screenshot_apng_end; ++i)
		OP_ScreenshotOptionsMenu[i].status = IT_DISABLED;

	switch (cv_moviemode.value)
	{
		case MM_GIF:
			cstart = op_screenshot_gif_start;
			cend = op_screenshot_gif_end;
			break;
		case MM_APNG:
			cstart = op_screenshot_apng_start;
			cend = op_screenshot_apng_end;
			break;
		default:
			return;
	}
	for (i = cstart; i <= cend; ++i)
		OP_ScreenshotOptionsMenu[i].status = IT_STRING|IT_CVAR;
}

void Addons_option_Onchange(void)
{
	OP_AddonsOptionsMenu[op_addons_folder].status =
		(cv_addons_option.value == 3 ? IT_CVAR|IT_STRING|IT_CV_STRING : IT_DISABLED);
}

// ==========================================================================
// END ORGANIZATION STUFF.
// ==========================================================================

// current menudef
menu_t *currentMenu = &MainDef;

// =========================================================================
// BASIC MENU HANDLING
// =========================================================================

static void M_ChangeCvar(INT32 choice)
{
	consvar_t *cv = (consvar_t *)currentMenu->menuitems[itemOn].itemaction;

	if (choice == -1)
	{
		if (cv == &cv_playercolor)
		{
			SINT8 skinno = R_SkinAvailable(cv_chooseskin.string);
			if (skinno != -1)
				CV_SetValue(cv,skins[skinno].prefcolor);
			return;
		}
		CV_Set(cv,cv->defaultvalue);
		return;
	}

	choice = (choice<<1) - 1;

	if (((currentMenu->menuitems[itemOn].status & IT_CVARTYPE) == IT_CV_SLIDER)
	    ||((currentMenu->menuitems[itemOn].status & IT_CVARTYPE) == IT_CV_INVISSLIDER)
	    ||((currentMenu->menuitems[itemOn].status & IT_CVARTYPE) == IT_CV_NOMOD))
	{
		CV_SetValue(cv,cv->value+choice);
	}
	else if (cv->flags & CV_FLOAT)
	{
		char s[20];
		sprintf(s,"%f",FIXED_TO_FLOAT(cv->value)+(choice)*(1.0f/16.0f));
		CV_Set(cv,s);
	}
	else
	{
#ifndef NONET
		if (cv == &cv_nettimeout || cv == &cv_jointimeout)
			choice *= (TICRATE/7);
		else if (cv == &cv_maxsend)
			choice *= 512;
		else if (cv == &cv_maxping)
			choice *= 50;
#endif
		CV_AddValue(cv,choice);
	}
}

static boolean M_ChangeStringCvar(INT32 choice)
{
	consvar_t *cv = (consvar_t *)currentMenu->menuitems[itemOn].itemaction;
	char buf[MAXSTRINGLENGTH];
	size_t len;

	if (shiftdown && choice >= 32 && choice <= 127)
		choice = shiftxform[choice];

	switch (choice)
	{
		case KEY_BACKSPACE:
			len = strlen(cv->string);
			if (len > 0)
			{
				S_StartSound(NULL,sfx_menu1); // Tails
				M_Memcpy(buf, cv->string, len);
				buf[len-1] = 0;
				CV_Set(cv, buf);
			}
			return true;
		case KEY_DEL:
			if (cv->string[0])
			{
				S_StartSound(NULL,sfx_menu1); // Tails
				CV_Set(cv, "");
			}
			return true;
		default:
			if (choice >= 32 && choice <= 127)
			{
				len = strlen(cv->string);
				if (len < MAXSTRINGLENGTH - 1)
				{
					S_StartSound(NULL,sfx_menu1); // Tails
					M_Memcpy(buf, cv->string, len);
					buf[len++] = (char)choice;
					buf[len] = 0;
					CV_Set(cv, buf);
				}
				return true;
			}
			break;
	}
	return false;
}

static void M_NextOpt(void)
{
	INT16 oldItemOn = itemOn; // prevent infinite loop

	do
	{
		if (itemOn + 1 > currentMenu->numitems - 1)
			itemOn = 0;
		else
			itemOn++;
	} while (oldItemOn != itemOn && (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_SPACE);
}

static void M_PrevOpt(void)
{
	INT16 oldItemOn = itemOn; // prevent infinite loop

	do
	{
		if (!itemOn)
			itemOn = currentMenu->numitems - 1;
		else
			itemOn--;
	} while (oldItemOn != itemOn && (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_SPACE);
}

// lock out further input in a tic when important buttons are pressed
// (in other words -- stop bullshit happening by mashing buttons in fades)
static boolean noFurtherInput = false;

static void Command_Manual_f(void)
{
	if (modeattacking)
		return;
	M_StartControlPanel();
	M_Manual(INT32_MAX);
	itemOn = 0;
}

//
// M_Responder
//
boolean M_Responder(event_t *ev)
{
	INT32 ch = -1;
//	INT32 i;
	static tic_t joywait = 0, mousewait = 0;
	static INT32 pjoyx = 0, pjoyy = 0;
	static INT32 pmousex = 0, pmousey = 0;
	static INT32 lastx = 0, lasty = 0;
	void (*routine)(INT32 choice); // for some casting problem

	if (dedicated || (demo.playback && demo.title)
	|| gamestate == GS_INTRO || gamestate == GS_CUTSCENE || gamestate == GS_GAMEEND
	|| gamestate == GS_CREDITS || gamestate == GS_EVALUATION)
		return false;

	if (noFurtherInput)
	{
		// Ignore input after enter/escape/other buttons
		// (but still allow shift keyup so caps doesn't get stuck)
		return false;
	}
	else if (ev->type == ev_keydown)
	{
		ch = ev->data1;

		// added 5-2-98 remap virtual keys (mouse & joystick buttons)
		switch (ch)
		{
			case KEY_MOUSE1:
				//case KEY_JOY1:
				//case KEY_JOY1 + 2:
				ch = KEY_ENTER;
				break;
				/*case KEY_JOY1 + 3: // Brake can function as 'n' for message boxes now.
					ch = 'n';
					break;*/
			case KEY_MOUSE1 + 1:
				//case KEY_JOY1 + 1:
				ch = KEY_BACKSPACE;
				break;
			case KEY_HAT1:
				ch = KEY_UPARROW;
				break;
			case KEY_HAT1 + 1:
				ch = KEY_DOWNARROW;
				break;
			case KEY_HAT1 + 2:
				ch = KEY_LEFTARROW;
				break;
			case KEY_HAT1 + 3:
				ch = KEY_RIGHTARROW;
				break;
		}
	}
	else if (menuactive)
	{
		if (ev->type == ev_joystick  && ev->data1 == 0 && joywait < I_GetTime())
		{
			const INT32 jdeadzone = ((JOYAXISRANGE-1) * cv_deadzone.value) >> FRACBITS;
			if (ev->data3 != INT32_MAX)
			{
				if (Joystick.bGamepadStyle || abs(ev->data3) > jdeadzone)
				{
					if (ev->data3 < 0 && pjoyy >= 0)
					{
						ch = KEY_UPARROW;
						joywait = I_GetTime() + NEWTICRATE/7;
					}
					else if (ev->data3 > 0 && pjoyy <= 0)
					{
						ch = KEY_DOWNARROW;
						joywait = I_GetTime() + NEWTICRATE/7;
					}
					pjoyy = ev->data3;
				}
				else
					pjoyy = 0;
			}

			if (ev->data2 != INT32_MAX)
			{
				if (Joystick.bGamepadStyle || abs(ev->data2) > jdeadzone)
				{
					if (ev->data2 < 0 && pjoyx >= 0)
					{
						ch = KEY_LEFTARROW;
						joywait = I_GetTime() + NEWTICRATE/17;
					}
					else if (ev->data2 > 0 && pjoyx <= 0)
					{
						ch = KEY_RIGHTARROW;
						joywait = I_GetTime() + NEWTICRATE/17;
					}
					pjoyx = ev->data2;
				}
				else
					pjoyx = 0;
			}
		}
		else if (ev->type == ev_mouse && mousewait < I_GetTime())
		{
			pmousey += ev->data3;
			if (pmousey < lasty-30)
			{
				ch = KEY_DOWNARROW;
				mousewait = I_GetTime() + NEWTICRATE/7;
				pmousey = lasty -= 30;
			}
			else if (pmousey > lasty + 30)
			{
				ch = KEY_UPARROW;
				mousewait = I_GetTime() + NEWTICRATE/7;
				pmousey = lasty += 30;
			}

			pmousex += ev->data2;
			if (pmousex < lastx - 30)
			{
				ch = KEY_LEFTARROW;
				mousewait = I_GetTime() + NEWTICRATE/7;
				pmousex = lastx -= 30;
			}
			else if (pmousex > lastx+30)
			{
				ch = KEY_RIGHTARROW;
				mousewait = I_GetTime() + NEWTICRATE/7;
				pmousex = lastx += 30;
			}
		}
	}

	if (ch == -1)
		return false;
	else if (ch == gamecontrol[gc_systemmenu][0] || ch == gamecontrol[gc_systemmenu][1]) // allow remappable ESC key
		ch = KEY_ESCAPE;
	else if ((ch == gamecontrol[gc_accelerate][0] || ch == gamecontrol[gc_accelerate][1])  && ch >= KEY_MOUSE1)
		ch = KEY_ENTER;

	// F-Keys
	if (!menuactive)
	{
		noFurtherInput = true;

		switch (ch)
		{
			case KEY_F1: // Help key
				Command_Manual_f();
				return true;

			case KEY_F2: // Empty
				return true;

			case KEY_F3: // Toggle HUD
				CV_SetValue(&cv_showhud, !cv_showhud.value);
				return true;

			case KEY_F4: // Sound Volume
				if (modeattacking)
					return true;
				M_StartControlPanel();
				M_Options(0);
				currentMenu = &OP_SoundOptionsDef;
				itemOn = 0;
				return true;

#ifndef DC
			case KEY_F5: // Video Mode
				if (modeattacking)
					return true;
				M_StartControlPanel();
				M_Options(0);
				M_VideoModeMenu(0);
				return true;
#endif

			case KEY_F6: // Empty
				return true;

			case KEY_F7: // Options
				if (modeattacking)
					return true;
				M_StartControlPanel();
				M_Options(0);
				M_SetupNextMenu(&OP_MainDef);
				return true;

			// Screenshots on F8 now handled elsewhere
			// Same with Moviemode on F9

			case KEY_F10: // Quit SRB2
				M_QuitSRB2(0);
				return true;

			case KEY_F11: // Gamma Level
				CV_AddValue(&cv_usegamma, 1);
				return true;

			// Spymode on F12 handled in game logic

			case KEY_ESCAPE: // Pop up menu
				if (chat_on)
				{
					HU_clearChatChars();
					chat_on = false;
				}
				else
					M_StartControlPanel();
				return true;
		}
		noFurtherInput = false; // turns out we didn't care
		return false;
	}

	if ((ch == gamecontrol[gc_brake][0] || ch == gamecontrol[gc_brake][1]) && ch >= KEY_MOUSE1) // do this here, otherwise brake opens the menu mid-game
		ch = KEY_ESCAPE;

	routine = currentMenu->menuitems[itemOn].itemaction;

	// Handle menuitems which need a specific key handling
	if (routine && (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_KEYHANDLER)
	{
		if (shiftdown && ch >= 32 && ch <= 127)
			ch = shiftxform[ch];
		routine(ch);
		return true;
	}

	if (currentMenu->menuitems[itemOn].status == IT_MSGHANDLER)
	{
		if (currentMenu->menuitems[itemOn].alphaKey != MM_EVENTHANDLER)
		{
			if (ch == ' ' || ch == 'n' || ch == 'y' || ch == KEY_ESCAPE || ch == KEY_ENTER)
			{
				if (routine)
					routine(ch);
				M_StopMessage(0);
				noFurtherInput = true;
				return true;
			}
			return true;
		}
		else
		{
			// dirty hack: for customising controls, I want only buttons/keys, not moves
			if (ev->type == ev_mouse || ev->type == ev_mouse2 || ev->type == ev_joystick
				|| ev->type == ev_joystick2 || ev->type == ev_joystick3 || ev->type == ev_joystick4)
				return true;
			if (routine)
			{
				void (*otherroutine)(event_t *sev) = currentMenu->menuitems[itemOn].itemaction;
				otherroutine(ev); //Alam: what a hack
			}
			return true;
		}
	}

	// BP: one of the more big hack i have never made
	if (routine && (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_CVAR)
	{
		if ((currentMenu->menuitems[itemOn].status & IT_CVARTYPE) == IT_CV_STRING)
		{

			if (shiftdown && ch >= 32 && ch <= 127)
				ch = shiftxform[ch];
			if (M_ChangeStringCvar(ch))
				return true;
			else
				routine = NULL;
		}
		else
			routine = M_ChangeCvar;
	}

	if (currentMenu == &PlaybackMenuDef && !con_destlines)
	{
		playback_last_menu_interaction_leveltime = leveltime;
		// Flip left/right with up/down for the playback menu, since it's a horizontal icon row.
		switch (ch)
		{
			case KEY_LEFTARROW: ch = KEY_UPARROW; break;
			case KEY_UPARROW: ch = KEY_RIGHTARROW; break;
			case KEY_RIGHTARROW: ch = KEY_DOWNARROW; break;
			case KEY_DOWNARROW: ch = KEY_LEFTARROW; break;

			// arbitrary keyboard shortcuts because fuck you

			case '\'':	// toggle freecam
				M_PlaybackToggleFreecam(0);
				break;

			case ']':	// ffw / advance frame (depends on if paused or not)
				if (paused)
					M_PlaybackAdvance(0);
				else
					M_PlaybackFastForward(0);
				break;

			case '[':	// rewind /backupframe, uses the same function
				M_PlaybackRewind(0);
				break;

			case '\\':	// pause
				M_PlaybackPause(0);
				break;

			// viewpoints, an annoyance (tm)
			case '-':	// viewpoint minus
				M_PlaybackSetViews(-1);	// yeah lol.
				break;

			case '=':	// viewpoint plus
				M_PlaybackSetViews(1);	// yeah lol.
				break;

			// switch viewpoints:
			case '1':	// viewpoint for p1 (also f12)
				// maximum laziness:
				if (!demo.freecam)
					G_AdjustView(1, 1, true);
				break;
			case '2':	// viewpoint for p2
				if (!demo.freecam)
					G_AdjustView(2, 1, true);
				break;
			case '3':	// viewpoint for p3
				if (!demo.freecam)
					G_AdjustView(3, 1, true);
				break;
			case '4':	// viewpoint for p4
				if (!demo.freecam)
					G_AdjustView(4, 1, true);
				break;

			default: break;
		}
	}

	// Keys usable within menu
	switch (ch)
	{
		case KEY_DOWNARROW:
			M_NextOpt();
			S_StartSound(NULL, sfx_menu1);
			/*if (currentMenu == &SP_PlayerDef)
			{
				Z_Free(char_notes);
				char_notes = NULL;
			}*/
			return true;

		case KEY_UPARROW:
			M_PrevOpt();
			S_StartSound(NULL, sfx_menu1);
			/*if (currentMenu == &SP_PlayerDef)
			{
				Z_Free(char_notes);
				char_notes = NULL;
			}*/
			return true;

		case KEY_LEFTARROW:
			if (routine && ((currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_ARROWS
				|| (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_CVAR))
			{
				if (currentMenu != &OP_SoundOptionsDef || itemOn > 3)
					S_StartSound(NULL, sfx_menu1);
				routine(0);
			}
			return true;

		case KEY_RIGHTARROW:
			if (routine && ((currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_ARROWS
				|| (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_CVAR))
			{
				if (currentMenu != &OP_SoundOptionsDef || itemOn > 3)
					S_StartSound(NULL, sfx_menu1);
				routine(1);
			}
			return true;

		case KEY_ENTER:
			noFurtherInput = true;
			currentMenu->lastOn = itemOn;

			if (currentMenu == &PlaybackMenuDef)
			{
				boolean held = (boolean)playback_enterheld;
				if (held)
					return true;
				playback_enterheld = 3;
			}

			if (routine)
			{
				if (((currentMenu->menuitems[itemOn].status & IT_TYPE)==IT_CALL
				 || (currentMenu->menuitems[itemOn].status & IT_TYPE)==IT_SUBMENU)
                 && (currentMenu->menuitems[itemOn].status & IT_CALLTYPE))
				{
					if (((currentMenu->menuitems[itemOn].status & IT_CALLTYPE) & IT_CALL_NOTMODIFIED) && majormods)
					{
						S_StartSound(NULL, sfx_menu1);
						M_StartMessage(M_GetText("This cannot be done with complex addons\nor in a cheated game.\n\n(Press a key)\n"), NULL, MM_NOTHING);
						return true;
					}
				}
				S_StartSound(NULL, sfx_menu1);
				switch (currentMenu->menuitems[itemOn].status & IT_TYPE)
				{
					case IT_CVAR:
					case IT_ARROWS:
						routine(1); // right arrow
						break;
					case IT_CALL:
						routine(itemOn);
						break;
					case IT_SUBMENU:
						currentMenu->lastOn = itemOn;
						M_SetupNextMenu((menu_t *)currentMenu->menuitems[itemOn].itemaction);
						break;
				}
			}
			return true;

		case KEY_ESCAPE:
		//case KEY_JOY1 + 2:
			noFurtherInput = true;
			currentMenu->lastOn = itemOn;
			if (currentMenu->prevMenu)
			{
				//If we entered the game search menu, but didn't enter a game,
				//make sure the game doesn't still think we're in a netgame.
				if (!Playing() && netgame && multiplayer)
				{
					netgame = false;
					multiplayer = false;
				}

				if (currentMenu == &SP_TimeAttackDef) //|| currentMenu == &SP_NightsAttackDef
				{
					// D_StartTitle does its own wipe, since GS_TIMEATTACK is now a complete gamestate.
					menuactive = false;
					D_StartTitle();
				}
				else
					M_SetupNextMenu(currentMenu->prevMenu);
			}
			else
				M_ClearMenus(true);

			return true;

		case KEY_BACKSPACE:
			if ((currentMenu->menuitems[itemOn].status) == IT_CONTROL)
			{
				// detach any keys associated with the game control
				G_ClearControlKeys(setupcontrols, currentMenu->menuitems[itemOn].alphaKey);
				S_StartSound(NULL, sfx_shldls);
				return true;
			}

			if (routine && ((currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_ARROWS
				|| (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_CVAR))
			{
				consvar_t *cv = (consvar_t *)currentMenu->menuitems[itemOn].itemaction;

				if (cv == &cv_chooseskin
					|| cv == &cv_dummystaff
					|| cv == &cv_nextmap
					|| cv == &cv_newgametype)
					return true;

				if (currentMenu != &OP_SoundOptionsDef || itemOn > 3)
					S_StartSound(NULL, sfx_menu1);
				routine(-1);
				return true;
			}

			// Why _does_ backspace go back anyway?
			//currentMenu->lastOn = itemOn;
			//if (currentMenu->prevMenu)
			//	M_SetupNextMenu(currentMenu->prevMenu);
			return false;

		default:
			break;
	}

	return true;
}

// special responder for demos
boolean M_DemoResponder(event_t *ev)
{

	INT32 ch = -1;	// cur event data
	boolean eatinput = false;	// :omnom:

	//should be accounted for beforehand but just to be safe...
	if (!demo.playback || demo.title)
		return false;

	if (noFurtherInput)
	{
		// Ignore input after enter/escape/other buttons
		// (but still allow shift keyup so caps doesn't get stuck)
		return false;
	}
	else if (ev->type == ev_keydown && !con_destlines)	// not while the console is on please
	{
		ch = ev->data1;
		// since this is ONLY for demos, there isn't MUCH for us to do.
		// mirrored from m_responder

		switch (ch)
		{
			// arbitrary keyboard shortcuts because fuck you

			case '\'':	// toggle freecam
				M_PlaybackToggleFreecam(0);
				eatinput = true;
				break;

			case ']':	// ffw / advance frame (depends on if paused or not)
				if (paused)
					M_PlaybackAdvance(0);
				else
					M_PlaybackFastForward(0);
				eatinput = true;
				break;

			case '[':	// rewind /backupframe, uses the same function
				M_PlaybackRewind(0);
				break;

			case '\\':	// pause
				M_PlaybackPause(0);
				eatinput = true;
				break;

			// viewpoints, an annoyance (tm)
			case '-':	// viewpoint minus
				M_PlaybackSetViews(-1);	// yeah lol.
				eatinput = true;
				break;

			case '=':	// viewpoint plus
				M_PlaybackSetViews(1);	// yeah lol.
				eatinput = true;
				break;

			// switch viewpoints:
			case '1':	// viewpoint for p1 (also f12)
				// maximum laziness:
				if (!demo.freecam)
					G_AdjustView(1, 1, true);
				break;
			case '2':	// viewpoint for p2
				if (!demo.freecam)
					G_AdjustView(2, 1, true);
				break;
			case '3':	// viewpoint for p3
				if (!demo.freecam)
					G_AdjustView(3, 1, true);
				break;
			case '4':	// viewpoint for p4
				if (!demo.freecam)
					G_AdjustView(4, 1, true);
				break;

			default: break;
		}

	}
	return eatinput;
}


//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer(void)
{
	if (currentMenu == &MessageDef)
		menuactive = true;

	if (menuactive)
	{
		// now that's more readable with a faded background (yeah like Quake...)
		if (!WipeInAction && currentMenu != &PlaybackMenuDef) // Replay playback has its own background
			V_DrawFadeScreen(0xFF00, 16);

		if (currentMenu->drawroutine)
		{
			M_GetGametypeColor();
			currentMenu->drawroutine(); // call current menu Draw routine
		}

		// Draw version down in corner
		// ... but only in the MAIN MENU.  I'm a picky bastard.
		if (currentMenu == &MainDef)
		{
			if (customversionstring[0] != '\0')
			{
				V_DrawThinString(vid.dupx, vid.height - 20*vid.dupy, V_NOSCALESTART|V_TRANSLUCENT, "Mod version:");
				V_DrawThinString(vid.dupx, vid.height - 10*vid.dupy, V_NOSCALESTART|V_TRANSLUCENT|V_ALLOWLOWERCASE, customversionstring);
			}
			else
			{
#ifdef DEVELOP // Development -- show revision / branch info
				V_DrawThinString(vid.dupx, vid.height - 20*vid.dupy, V_NOSCALESTART|V_TRANSLUCENT|V_ALLOWLOWERCASE, compbranch);
				V_DrawThinString(vid.dupx, vid.height - 10*vid.dupy, V_NOSCALESTART|V_TRANSLUCENT|V_ALLOWLOWERCASE, comprevision);
#else // Regular build
				V_DrawThinString(vid.dupx, vid.height - 10*vid.dupy, V_NOSCALESTART|V_TRANSLUCENT|V_ALLOWLOWERCASE, va("%s", VERSIONSTRING));
#endif
			}
		}
	}

	// focus lost notification goes on top of everything, even the former everything
	if (window_notinfocus && cv_showfocuslost.value)
	{
		M_DrawTextBox((BASEVIDWIDTH/2) - (60), (BASEVIDHEIGHT/2) - (16), 13, 2);
		if (gamestate == GS_LEVEL && (P_AutoPause() || paused))
			V_DrawCenteredString(BASEVIDWIDTH/2, (BASEVIDHEIGHT/2) - (4), highlightflags, "Game Paused");
		else
			V_DrawCenteredString(BASEVIDWIDTH/2, (BASEVIDHEIGHT/2) - (4), highlightflags, "Focus Lost");
	}
}

//
// M_StartControlPanel
//
void M_StartControlPanel(void)
{
	// intro might call this repeatedly
	if (menuactive)
	{
		CON_ToggleOff(); // move away console
		return;
	}

	menuactive = true;

	if (demo.playback)
	{
		currentMenu = &PlaybackMenuDef;
		playback_last_menu_interaction_leveltime = leveltime;
	}
	else if (!Playing())
	{
		// Secret menu!
		//MainMenu[secrets].status = (M_AnySecretUnlocked()) ? (IT_STRING | IT_CALL) : (IT_DISABLED);

		currentMenu = &MainDef;
		itemOn = singleplr;
	}
	else if (modeattacking)
	{
		currentMenu = &MAPauseDef;
		itemOn = mapause_continue;
	}
	else if (!(netgame || multiplayer)) // Single Player
	{
		if (gamestate != GS_LEVEL /*|| ultimatemode*/) // intermission, so gray out stuff.
		{
			SPauseMenu[spause_pandora].status = (M_SecretUnlocked(SECRET_PANDORA)) ? (IT_GRAYEDOUT) : (IT_DISABLED);
			SPauseMenu[spause_retry].status = IT_GRAYEDOUT;
		}
		else
		{
			//INT32 numlives = 2;

			SPauseMenu[spause_pandora].status = (M_SecretUnlocked(SECRET_PANDORA)) ? (IT_STRING | IT_CALL) : (IT_DISABLED);

			/*if (&players[consoleplayer])
			{
				numlives = players[consoleplayer].lives;
				if (players[consoleplayer].playerstate != PST_LIVE)
					++numlives;
			}

			// The list of things that can disable retrying is (was?) a little too complex
			// for me to want to use the short if statement syntax
			if (numlives <= 1 || G_IsSpecialStage(gamemap))
				SPauseMenu[spause_retry].status = (IT_GRAYEDOUT);
			else*/
				SPauseMenu[spause_retry].status = (IT_STRING | IT_CALL);
		}

		// We can always use level select though. :33
		//SPauseMenu[spause_levelselect].status = (gamecomplete) ? (IT_STRING | IT_CALL) : (IT_DISABLED);

		// And emblem hints.
		SPauseMenu[spause_hints].status = (M_SecretUnlocked(SECRET_EMBLEMHINTS)) ? (IT_STRING | IT_CALL) : (IT_DISABLED);

		// Shift up Pandora's Box if both pandora and levelselect are active
		/*if (SPauseMenu[spause_pandora].status != (IT_DISABLED)
		 && SPauseMenu[spause_levelselect].status != (IT_DISABLED))
			SPauseMenu[spause_pandora].alphaKey = 24;
		else
			SPauseMenu[spause_pandora].alphaKey = 32;*/

		currentMenu = &SPauseDef;
		itemOn = spause_continue;
	}
	else // multiplayer
	{
		MPauseMenu[mpause_switchmap].status = IT_DISABLED;
		MPauseMenu[mpause_addons].status = IT_DISABLED;
		MPauseMenu[mpause_scramble].status = IT_DISABLED;
		MPauseMenu[mpause_psetupsplit].status = IT_DISABLED;
		MPauseMenu[mpause_psetupsplit2].status = IT_DISABLED;
		MPauseMenu[mpause_psetupsplit3].status = IT_DISABLED;
		MPauseMenu[mpause_psetupsplit4].status = IT_DISABLED;
		MPauseMenu[mpause_spectate].status = IT_DISABLED;
		MPauseMenu[mpause_entergame].status = IT_DISABLED;
		MPauseMenu[mpause_canceljoin].status = IT_DISABLED;
		MPauseMenu[mpause_switchteam].status = IT_DISABLED;
		MPauseMenu[mpause_switchspectate].status = IT_DISABLED;
		MPauseMenu[mpause_psetup].status = IT_DISABLED;
		MISC_ChangeTeamMenu[0].status = IT_DISABLED;
		MISC_ChangeSpectateMenu[0].status = IT_DISABLED;

		// Reset these in case splitscreen messes things up
		MPauseMenu[mpause_addons].alphaKey = 8;
		MPauseMenu[mpause_scramble].alphaKey = 8;
		MPauseMenu[mpause_switchmap].alphaKey = 24;

		MPauseMenu[mpause_switchteam].alphaKey = 48;
		MPauseMenu[mpause_switchspectate].alphaKey = 48;
		MPauseMenu[mpause_options].alphaKey = 64;
		MPauseMenu[mpause_title].alphaKey = 80;
		MPauseMenu[mpause_quit].alphaKey = 88;

		Dummymenuplayer_OnChange();

		if ((server || IsPlayerAdmin(consoleplayer)))
		{
			MPauseMenu[mpause_switchmap].status = IT_STRING | IT_CALL;
			MPauseMenu[mpause_addons].status = IT_STRING | IT_CALL;
			if (G_GametypeHasTeams())
				MPauseMenu[mpause_scramble].status = IT_STRING | IT_SUBMENU;
		}

		if (splitscreen)
		{
			MPauseMenu[mpause_psetupsplit].status = MPauseMenu[mpause_psetupsplit2].status = IT_STRING | IT_CALL;
			MISC_ChangeTeamMenu[0].status = MISC_ChangeSpectateMenu[0].status = IT_STRING|IT_CVAR;

			if (netgame)
			{
				if (G_GametypeHasTeams())
				{
					MPauseMenu[mpause_switchteam].status = IT_STRING | IT_SUBMENU;
					MPauseMenu[mpause_switchteam].alphaKey += ((splitscreen+1) * 8);
					MPauseMenu[mpause_options].alphaKey += 8;
					MPauseMenu[mpause_title].alphaKey += 8;
					MPauseMenu[mpause_quit].alphaKey += 8;
				}
				else if (G_GametypeHasSpectators())
				{
					MPauseMenu[mpause_switchspectate].status = IT_STRING | IT_SUBMENU;
					MPauseMenu[mpause_switchspectate].alphaKey += ((splitscreen+1) * 8);
					MPauseMenu[mpause_options].alphaKey += 8;
					MPauseMenu[mpause_title].alphaKey += 8;
					MPauseMenu[mpause_quit].alphaKey += 8;
				}
			}

			if (splitscreen > 1)
			{
				MPauseMenu[mpause_psetupsplit3].status = IT_STRING | IT_CALL;

				MPauseMenu[mpause_options].alphaKey += 8;
				MPauseMenu[mpause_title].alphaKey += 8;
				MPauseMenu[mpause_quit].alphaKey += 8;

				if (splitscreen > 2)
				{
					MPauseMenu[mpause_psetupsplit4].status = IT_STRING | IT_CALL;
					MPauseMenu[mpause_options].alphaKey += 8;
					MPauseMenu[mpause_title].alphaKey += 8;
					MPauseMenu[mpause_quit].alphaKey += 8;
				}
			}
		}
		else
		{
			MPauseMenu[mpause_psetup].status = IT_STRING | IT_CALL;

			if (G_GametypeHasTeams())
				MPauseMenu[mpause_switchteam].status = IT_STRING | IT_SUBMENU;
			else if (G_GametypeHasSpectators())
			{
				if (!players[consoleplayer].spectator)
					MPauseMenu[mpause_spectate].status = IT_STRING | IT_CALL;
				else if (players[consoleplayer].pflags & PF_WANTSTOJOIN)
					MPauseMenu[mpause_canceljoin].status = IT_STRING | IT_CALL;
				else
					MPauseMenu[mpause_entergame].status = IT_STRING | IT_CALL;
			}
			else // in this odd case, we still want something to be on the menu even if it's useless
				MPauseMenu[mpause_spectate].status = IT_GRAYEDOUT;
		}

#ifdef HAVE_DISCORDRPC
		{
			UINT8 i;

			for (i = 0; i < mpause_discordrequests; i++)
				MPauseMenu[i].alphaKey -= 8;

			MPauseMenu[mpause_discordrequests].alphaKey = MPauseMenu[i].alphaKey;

			M_RefreshPauseMenu();
		}
#endif

		currentMenu = &MPauseDef;
		itemOn = mpause_continue;
	}

	CON_ToggleOff(); // move away console
}

void M_EndModeAttackRun(void)
{
	M_ModeAttackEndGame(0);
}

//
// M_ClearMenus
//
void M_ClearMenus(boolean callexitmenufunc)
{
	if (!menuactive)
		return;

	if (currentMenu->quitroutine && callexitmenufunc && !currentMenu->quitroutine())
		return; // we can't quit this menu (also used to set parameter from the menu)

#ifndef DC // Save the config file. I'm sick of crashing the game later and losing all my changes!
	COM_BufAddText(va("saveconfig \"%s\" -silent\n", configfile));
#endif //Alam: But not on the Dreamcast's VMUs

	if (currentMenu == &MessageDef) // Oh sod off!
		currentMenu = &MainDef; // Not like it matters
	menuactive = false;
}

//
// M_SetupNextMenu
//
void M_SetupNextMenu(menu_t *menudef)
{
	INT16 i;

	if (currentMenu->quitroutine)
	{
		// If you're going from a menu to itself, why are you running the quitroutine? You're not quitting it! -SH
		if (currentMenu != menudef && !currentMenu->quitroutine())
			return; // we can't quit this menu (also used to set parameter from the menu)
	}
	currentMenu = menudef;
	itemOn = currentMenu->lastOn;

	// in case of...
	if (itemOn >= currentMenu->numitems)
		itemOn = currentMenu->numitems - 1;

	// the curent item can be disabled,
	// this code go up until an enabled item found
	if ((currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_SPACE)
	{
		for (i = 0; i < currentMenu->numitems; i++)
		{
			if ((currentMenu->menuitems[i].status & IT_TYPE) != IT_SPACE)
			{
				itemOn = i;
				break;
			}
		}
	}
}

//
// M_Ticker
//
void M_Ticker(void)
{
	// reset input trigger
	noFurtherInput = false;

	if (dedicated)
		return;

	if (--skullAnimCounter <= 0)
		skullAnimCounter = 8;

	if (currentMenu == &PlaybackMenuDef)
	{
		if (playback_enterheld > 0)
			playback_enterheld--;
	}
	else
		playback_enterheld = 0;

	//added : 30-01-98 : test mode for five seconds
	if (vidm_testingmode > 0)
	{
		// restore the previous video mode
		if (--vidm_testingmode == 0)
			setmodeneeded = vidm_previousmode + 1;
	}

#if defined (MASTERSERVER) && defined (HAVE_THREADS)
	I_lock_mutex(&ms_ServerList_mutex);
	{
		if (ms_ServerList)
		{
			CL_QueryServerList(ms_ServerList);
			free(ms_ServerList);
			ms_ServerList = NULL;
		}
	}
	I_unlock_mutex(ms_ServerList_mutex);
#endif
}

//
// M_Init
//
void M_Init(void)
{
	UINT8 i;

	COM_AddCommand("manual", Command_Manual_f);

	CV_RegisterVar(&cv_nextmap);
	CV_RegisterVar(&cv_newgametype);
	CV_RegisterVar(&cv_chooseskin);
	CV_RegisterVar(&cv_autorecord);

	if (dedicated)
		return;

	// Menu hacks
	CV_RegisterVar(&cv_dummymenuplayer);
	CV_RegisterVar(&cv_dummyteam);
	CV_RegisterVar(&cv_dummyspectate);
	CV_RegisterVar(&cv_dummyscramble);
	CV_RegisterVar(&cv_dummyrings);
	CV_RegisterVar(&cv_dummylives);
	CV_RegisterVar(&cv_dummycontinues);
	//CV_RegisterVar(&cv_dummymares);
	CV_RegisterVar(&cv_dummystaff);

	quitmsg[QUITMSG] = M_GetText("Eggman's tied explosives\nto your girlfriend, and\nwill activate them if\nyou press the 'Y' key!\nPress 'N' to save her!\n\n(Press 'Y' to quit)");
	quitmsg[QUITMSG1] = M_GetText("What would Tails say if\nhe saw you quitting the game?\n\n(Press 'Y' to quit)");
	quitmsg[QUITMSG2] = M_GetText("Hey!\nWhere do ya think you're goin'?\n\n(Press 'Y' to quit)");
	quitmsg[QUITMSG3] = M_GetText("Forget your studies!\nPlay some more!\n\n(Press 'Y' to quit)");
	quitmsg[QUITMSG4] = M_GetText("You're trying to say you\nlike Sonic R better than\nthis, aren't you?\n\n(Press 'Y' to quit)");
	quitmsg[QUITMSG5] = M_GetText("Don't leave yet -- there's a\nsuper emerald around that corner!\n\n(Press 'Y' to quit)");
	quitmsg[QUITMSG6] = M_GetText("You'd rather work than play?\n\n(Press 'Y' to quit)");
	quitmsg[QUITMSG7] = M_GetText("Go ahead and leave. See if I care...\n*sniffle*\n\n(Press 'Y' to quit)");

	quitmsg[QUIT2MSG] = M_GetText("If you leave now,\nEggman will take over the world!\n\n(Press 'Y' to quit)");
	quitmsg[QUIT2MSG1] = M_GetText("On your mark,\nget set,\nhit the 'N' key!\n\n(Press 'Y' to quit)");
	quitmsg[QUIT2MSG2] = M_GetText("Aw c'mon, just\na few more laps!\n\n(Press 'Y' to quit)");
	quitmsg[QUIT2MSG3] = M_GetText("Did you get all those Chaos Emeralds?\n\n(Press 'Y' to quit)");
	quitmsg[QUIT2MSG4] = M_GetText("If you leave, I'll use\nmy Jawz on you!\n\n(Press 'Y' to quit)");
	quitmsg[QUIT2MSG5] = M_GetText("Don't go!\nYou might find the hidden\nlevels!\n\n(Press 'Y' to quit)");
	quitmsg[QUIT2MSG6] = M_GetText("Hit the 'N' key, Sonic!\nThe 'N' key!\n\n(Press 'Y' to quit)");

	quitmsg[QUIT3MSG] = M_GetText("Are you really going to give up?\nWe certainly would never give you up.\n\n(Press 'Y' to quit)");
	quitmsg[QUIT3MSG1] = M_GetText("Come on, just ONE more netgame!\n\n(Press 'Y' to quit)");
	quitmsg[QUIT3MSG2] = M_GetText("Press 'N' to unlock\nthe Golden Kart!\n\n(Press 'Y' to quit)");
	quitmsg[QUIT3MSG3] = M_GetText("Couldn't handle\nthe banana meta?\n\n(Press 'Y' to quit)");
	quitmsg[QUIT3MSG4] = M_GetText("Every time you press 'Y', an\nSRB2Kart Developer cries...\n\n(Press 'Y' to quit)");
	quitmsg[QUIT3MSG5] = M_GetText("You'll be back to play soon, though...\n...right?\n\n(Press 'Y' to quit)");
	quitmsg[QUIT3MSG6] = M_GetText("Aww, is Eggman's Nightclub too\ndifficult for you?\n\n(Press 'Y' to quit)");

	// Setup PlayerMenu table
	for (i = 0; i < MAXSKINS; i++)
	{
		PlayerMenu[i].status = (i == 0 ? IT_CALL : IT_DISABLED);
		PlayerMenu[i].patch = PlayerMenu[i].text = NULL;
		PlayerMenu[i].itemaction = M_ChoosePlayer;
		PlayerMenu[i].alphaKey = 0;
	}

#ifdef HWRENDER
	// Permanently hide some options based on render mode
	if (rendermode == render_soft)
		OP_VideoOptionsMenu[op_video_ogl].status = IT_DISABLED;
#endif

#ifndef NONET
	CV_RegisterVar(&cv_serversort);
#endif

	//todo put this somewhere better...
	CV_RegisterVar(&cv_allcaps);
}

void M_InitCharacterTables(void)
{
	UINT8 i;

	// Setup PlayerMenu table
	for (i = 0; i < MAXSKINS; i++)
	{
		PlayerMenu[i].status = (i < 4 ? IT_CALL : IT_DISABLED);
		PlayerMenu[i].patch = PlayerMenu[i].text = NULL;
		PlayerMenu[i].itemaction = M_ChoosePlayer;
		PlayerMenu[i].alphaKey = 0;
	}

	// Setup description table
	for (i = 0; i < MAXSKINS; i++)
	{
		if (i == 0)
		{
			strcpy(description[i].notes, "\x82Sonic\x80 is the fastest of the three, but also the hardest to control. Beginners beware, but experts will find Sonic very powerful.\n\n\x82""Ability:\x80 Speed Thok\nDouble jump to zoom forward with a huge burst of speed.\n\n\x82Tip:\x80 Simply letting go of forward does not slow down in SRB2. To slow down, hold the opposite direction.");
			strcpy(description[i].picname, "");
			strcpy(description[i].skinname, "sonic");
		}
		else if (i == 1)
		{
			strcpy(description[i].notes, "\x82Tails\x80 is the most mobile of the three, but has the slowest speed. Because of his mobility, he's well-\nsuited to beginners.\n\n\x82""Ability:\x80 Fly\nDouble jump to start flying for a limited time. Repetitively hit the jump button to ascend.\n\n\x82Tip:\x80 To quickly descend while flying, hit the spin button.");
			strcpy(description[i].picname, "");
			strcpy(description[i].skinname, "tails");
		}
		else if (i == 2)
		{
			strcpy(description[i].notes, "\x82Knuckles\x80 is well-\nrounded and can destroy breakable walls simply by touching them, but he can't jump as high as the other two.\n\n\x82""Ability:\x80 Glide & Climb\nDouble jump to glide in the air as long as jump is held. Glide into a wall to climb it.\n\n\x82Tip:\x80 Press spin while climbing to jump off the wall; press jump instead to jump off\nand face away from\nthe wall.");
			strcpy(description[i].picname, "");
			strcpy(description[i].skinname, "knuckles");
		}
		else if (i == 3)
		{
			strcpy(description[i].notes, "\x82Sonic & Tails\x80 team up to take on Dr. Eggman!\nControl Sonic while Tails desperately struggles to keep up.\n\nPlayer 2 can control Tails directly by setting the controls in the options menu.\nTails's directional controls are relative to Player 1's camera.\n\nTails can pick up Sonic while flying and carry him around.");
			strcpy(description[i].picname, "CHRS&T");
			strcpy(description[i].skinname, "sonic&tails");
		}
		else
		{
			strcpy(description[i].notes, "???");
			strcpy(description[i].picname, "");
			strcpy(description[i].skinname, "");
		}
	}
}

// ==========================================================================
// SPECIAL MENU OPTION DRAW ROUTINES GO HERE
// ==========================================================================

// Converts a string into question marks.
// Used for the secrets menu, to hide yet-to-be-unlocked stuff.
static const char *M_CreateSecretMenuOption(const char *str)
{
	static char qbuf[32];
	int i;

	for (i = 0; i < 31; ++i)
	{
		if (!str[i])
		{
			qbuf[i] = '\0';
			return qbuf;
		}
		else if (str[i] != ' ')
			qbuf[i] = '?';
		else
			qbuf[i] = ' ';
	}

	qbuf[31] = '\0';
	return qbuf;
}

static void M_DrawThermo(INT32 x, INT32 y, consvar_t *cv)
{
	INT32 xx = x, i;
	lumpnum_t leftlump, rightlump, centerlump[2], cursorlump;
	patch_t *p;

	leftlump = W_GetNumForName("M_THERML");
	rightlump = W_GetNumForName("M_THERMR");
	centerlump[0] = W_GetNumForName("M_THERMM");
	centerlump[1] = W_GetNumForName("M_THERMM");
	cursorlump = W_GetNumForName("M_THERMO");

	V_DrawScaledPatch(xx, y, 0, p = W_CachePatchNum(leftlump,PU_CACHE));
	xx += SHORT(p->width) - SHORT(p->leftoffset);
	for (i = 0; i < 16; i++)
	{
		V_DrawScaledPatch(xx, y, V_WRAPX, W_CachePatchNum(centerlump[i & 1], PU_CACHE));
		xx += 8;
	}
	V_DrawScaledPatch(xx, y, 0, W_CachePatchNum(rightlump, PU_CACHE));

	xx = (cv->value - cv->PossibleValue[0].value) * (15*8) /
		(cv->PossibleValue[1].value - cv->PossibleValue[0].value);

	V_DrawScaledPatch((x + 8) + xx, y, 0, W_CachePatchNum(cursorlump, PU_CACHE));
}

//  A smaller 'Thermo', with range given as percents (0-100)
static void M_DrawSlider(INT32 x, INT32 y, const consvar_t *cv, boolean ontop)
{
	INT32 i;
	INT32 range;
	patch_t *p;

	for (i = 0; cv->PossibleValue[i+1].strvalue; i++);

	x = BASEVIDWIDTH - x - SLIDER_WIDTH;

	if (ontop)
	{
		V_DrawCharacter(x - 16 - (skullAnimCounter/5), y,
			'\x1C' | highlightflags, false); // left arrow
		V_DrawCharacter(x+(SLIDER_RANGE*8) + 8 + (skullAnimCounter/5), y,
			'\x1D' | highlightflags, false); // right arrow
	}

	if ((range = atoi(cv->defaultvalue)) != cv->value)
	{
		range = ((range - cv->PossibleValue[0].value) * 100 /
		(cv->PossibleValue[1].value - cv->PossibleValue[0].value));

		if (range < 0)
			range = 0;
		if (range > 100)
			range = 100;

		// draw the default
		p = W_CachePatchName("M_SLIDEC", PU_CACHE);
		V_DrawScaledPatch(x - 4 + (((SLIDER_RANGE)*8 + 4)*range)/100, y, 0, p);
	}

	V_DrawScaledPatch(x - 8, y, 0, W_CachePatchName("M_SLIDEL", PU_CACHE));

	p =  W_CachePatchName("M_SLIDEM", PU_CACHE);
	for (i = 0; i < SLIDER_RANGE; i++)
		V_DrawScaledPatch (x+i*8, y, 0,p);

	p = W_CachePatchName("M_SLIDER", PU_CACHE);
	V_DrawScaledPatch(x+SLIDER_RANGE*8, y, 0, p);

	range = ((cv->value - cv->PossibleValue[0].value) * 100 /
	 (cv->PossibleValue[1].value - cv->PossibleValue[0].value));

	if (range < 0)
		range = 0;
	if (range > 100)
		range = 100;

	// draw the slider cursor
	p = W_CachePatchName("M_SLIDEC", PU_CACHE);
	V_DrawScaledPatch(x - 4 + (((SLIDER_RANGE)*8 + 4)*range)/100, y, 0, p);
}

//
//  Draw a textbox, like Quake does, because sometimes it's difficult
//  to read the text with all the stuff in the background...
//
void M_DrawTextBox(INT32 x, INT32 y, INT32 width, INT32 boxlines)
{
	// Solid color textbox.
	V_DrawFill(x+5, y+5, width*8+6, boxlines*8+6, 239);
	//V_DrawFill(x+8, y+8, width*8, boxlines*8, 31);
/*
	patch_t *p;
	INT32 cx, cy, n;
	INT32 step, boff;

	step = 8;
	boff = 8;

	// draw left side
	cx = x;
	cy = y;
	V_DrawScaledPatch(cx, cy, 0, W_CachePatchNum(viewborderlump[BRDR_TL], PU_CACHE));
	cy += boff;
	p = W_CachePatchNum(viewborderlump[BRDR_L], PU_CACHE);
	for (n = 0; n < boxlines; n++)
	{
		V_DrawScaledPatch(cx, cy, V_WRAPY, p);
		cy += step;
	}
	V_DrawScaledPatch(cx, cy, 0, W_CachePatchNum(viewborderlump[BRDR_BL], PU_CACHE));

	// draw middle
	V_DrawFlatFill(x + boff, y + boff, width*step, boxlines*step, st_borderpatchnum);

	cx += boff;
	cy = y;
	while (width > 0)
	{
		V_DrawScaledPatch(cx, cy, V_WRAPX, W_CachePatchNum(viewborderlump[BRDR_T], PU_CACHE));
		V_DrawScaledPatch(cx, y + boff + boxlines*step, V_WRAPX, W_CachePatchNum(viewborderlump[BRDR_B], PU_CACHE));
		width--;
		cx += step;
	}

	// draw right side
	cy = y;
	V_DrawScaledPatch(cx, cy, 0, W_CachePatchNum(viewborderlump[BRDR_TR], PU_CACHE));
	cy += boff;
	p = W_CachePatchNum(viewborderlump[BRDR_R], PU_CACHE);
	for (n = 0; n < boxlines; n++)
	{
		V_DrawScaledPatch(cx, cy, V_WRAPY, p);
		cy += step;
	}
	V_DrawScaledPatch(cx, cy, 0, W_CachePatchNum(viewborderlump[BRDR_BR], PU_CACHE));
*/
}

//
// Draw border for the savegame description
//
/*static void M_DrawSaveLoadBorder(INT32 x,INT32 y)
{
	INT32 i;

	V_DrawScaledPatch (x-8,y+7,0,W_CachePatchName("M_LSLEFT",PU_CACHE));

	for (i = 0;i < 24;i++)
	{
		V_DrawScaledPatch (x,y+7,0,W_CachePatchName("M_LSCNTR",PU_CACHE));
		x += 8;
	}

	V_DrawScaledPatch (x,y+7,0,W_CachePatchName("M_LSRGHT",PU_CACHE));
}*/

// horizontally centered text
static void M_CentreText(INT32 y, const char *string)
{
	INT32 x;
	//added : 02-02-98 : centre on 320, because V_DrawString centers on vid.width...
	x = (BASEVIDWIDTH - V_StringWidth(string, V_OLDSPACING))>>1;
	V_DrawString(x,y,V_OLDSPACING,string);
}

//
// M_DrawMapEmblems
//
// used by pause & statistics to draw a row of emblems for a map
//
static void M_DrawMapEmblems(INT32 mapnum, INT32 x, INT32 y)
{
	UINT8 lasttype = UINT8_MAX, curtype;
	emblem_t *emblem = M_GetLevelEmblems(mapnum);

	while (emblem)
	{
		switch (emblem->type)
		{
			case ET_TIME: //case ET_SCORE: case ET_RINGS:
				curtype = 1; break;
			/*case ET_NGRADE: case ET_NTIME:
				curtype = 2; break;*/
			default:
				curtype = 0; break;
		}

		// Shift over if emblem is of a different discipline
		if (lasttype != UINT8_MAX && lasttype != curtype)
			x -= 4;
		lasttype = curtype;

		if (emblem->collected)
			V_DrawSmallMappedPatch(x, y, 0, W_CachePatchName(M_GetEmblemPatch(emblem), PU_CACHE),
			                       R_GetTranslationColormap(TC_DEFAULT, M_GetEmblemColor(emblem), GTC_MENUCACHE));
		else
			V_DrawSmallScaledPatch(x, y, 0, W_CachePatchName("NEEDIT", PU_CACHE));

		emblem = M_GetLevelEmblems(-1);
		x -= 8;
	}
}

static void M_DrawMenuTitle(void)
{
	if (currentMenu->menutitlepic)
	{
		patch_t *p = W_CachePatchName(currentMenu->menutitlepic, PU_CACHE);

		if (p->height > 24) // title is larger than normal
		{
			INT32 xtitle = (BASEVIDWIDTH - (SHORT(p->width)/2))/2;
			INT32 ytitle = (30 - (SHORT(p->height)/2))/2;

			if (xtitle < 0)
				xtitle = 0;
			if (ytitle < 0)
				ytitle = 0;

			V_DrawSmallScaledPatch(xtitle, ytitle, 0, p);
		}
		else
		{
			INT32 xtitle = (BASEVIDWIDTH - SHORT(p->width))/2;
			INT32 ytitle = (30 - SHORT(p->height))/2;

			if (xtitle < 0)
				xtitle = 0;
			if (ytitle < 0)
				ytitle = 0;

			V_DrawScaledPatch(xtitle, ytitle, 0, p);
		}
	}
}

static void M_DrawGenericMenu(void)
{
	INT32 x, y, w, i, cursory = 0;

	// DRAW MENU
	x = currentMenu->x;
	y = currentMenu->y;

	// draw title (or big pic)
	M_DrawMenuTitle();

	for (i = 0; i < currentMenu->numitems; i++)
	{
		if (i == itemOn)
			cursory = y;
		switch (currentMenu->menuitems[i].status & IT_DISPLAY)
		{
			case IT_PATCH:
				if (currentMenu->menuitems[i].patch && currentMenu->menuitems[i].patch[0])
				{
					if (currentMenu->menuitems[i].status & IT_CENTER)
					{
						patch_t *p;
						p = W_CachePatchName(currentMenu->menuitems[i].patch, PU_CACHE);
						V_DrawScaledPatch((BASEVIDWIDTH - SHORT(p->width))/2, y, 0, p);
					}
					else
					{
						V_DrawScaledPatch(x, y, 0,
							W_CachePatchName(currentMenu->menuitems[i].patch, PU_CACHE));
					}
				}
				/* FALLTHRU */
			case IT_NOTHING:
			case IT_DYBIGSPACE:
				y = currentMenu->y+currentMenu->menuitems[i].alphaKey;//+= LINEHEIGHT;
				break;
			case IT_BIGSLIDER:
				M_DrawThermo(x, y, (consvar_t *)currentMenu->menuitems[i].itemaction);
				y += LINEHEIGHT;
				break;
			case IT_STRING:
			case IT_WHITESTRING:
				if (currentMenu->menuitems[i].alphaKey)
					y = currentMenu->y+currentMenu->menuitems[i].alphaKey;
				if (i == itemOn)
					cursory = y;

				if ((currentMenu->menuitems[i].status & IT_DISPLAY)==IT_STRING)
					V_DrawString(x, y, 0, currentMenu->menuitems[i].text);
				else
					V_DrawString(x, y, highlightflags, currentMenu->menuitems[i].text);

				// Cvar specific handling
				switch (currentMenu->menuitems[i].status & IT_TYPE)
					case IT_CVAR:
					{
						consvar_t *cv = (consvar_t *)currentMenu->menuitems[i].itemaction;
						switch (currentMenu->menuitems[i].status & IT_CVARTYPE)
						{
							case IT_CV_SLIDER:
								M_DrawSlider(x, y, cv, (i == itemOn));
							case IT_CV_NOPRINT: // color use this
							case IT_CV_INVISSLIDER: // monitor toggles use this
								break;
							case IT_CV_STRING:
								M_DrawTextBox(x, y + 4, MAXSTRINGLENGTH, 1);
								V_DrawString(x + 8, y + 12, V_ALLOWLOWERCASE, cv->string);
								if (skullAnimCounter < 4 && i == itemOn)
									V_DrawCharacter(x + 8 + V_StringWidth(cv->string, 0), y + 12,
										'_' | 0x80, false);
								y += 16;
								break;
							default:
								w = V_StringWidth(cv->string, 0);
								V_DrawString(BASEVIDWIDTH - x - w, y,
									((cv->flags & CV_CHEAT) && !CV_IsSetToDefault(cv) ? warningflags : highlightflags), cv->string);
								if (i == itemOn)
								{
									V_DrawCharacter(BASEVIDWIDTH - x - 10 - w - (skullAnimCounter/5), y,
											'\x1C' | highlightflags, false); // left arrow
									V_DrawCharacter(BASEVIDWIDTH - x + 2 + (skullAnimCounter/5), y,
											'\x1D' | highlightflags, false); // right arrow
								}
								break;
						}
						break;
					}
					y += STRINGHEIGHT;
					break;
			case IT_STRING2:
				V_DrawString(x, y, 0, currentMenu->menuitems[i].text);
				/* FALLTHRU */
			case IT_DYLITLSPACE:
				y += SMALLLINEHEIGHT;
				break;
			case IT_GRAYPATCH:
				if (currentMenu->menuitems[i].patch && currentMenu->menuitems[i].patch[0])
					V_DrawMappedPatch(x, y, 0,
						W_CachePatchName(currentMenu->menuitems[i].patch,PU_CACHE), graymap);
				y += LINEHEIGHT;
				break;
			case IT_TRANSTEXT:
				if (currentMenu->menuitems[i].alphaKey)
					y = currentMenu->y+currentMenu->menuitems[i].alphaKey;
				/* FALLTHRU */
			case IT_TRANSTEXT2:
				V_DrawString(x, y, V_TRANSLUCENT, currentMenu->menuitems[i].text);
				y += SMALLLINEHEIGHT;
				break;
			case IT_QUESTIONMARKS:
				if (currentMenu->menuitems[i].alphaKey)
					y = currentMenu->y+currentMenu->menuitems[i].alphaKey;

				V_DrawString(x, y, V_TRANSLUCENT|V_OLDSPACING, M_CreateSecretMenuOption(currentMenu->menuitems[i].text));
				y += SMALLLINEHEIGHT;
				break;
			case IT_HEADERTEXT: // draws 16 pixels to the left, in yellow text
				if (currentMenu->menuitems[i].alphaKey)
					y = currentMenu->y+currentMenu->menuitems[i].alphaKey;

				V_DrawString(x-16, y, highlightflags, currentMenu->menuitems[i].text);
				y += SMALLLINEHEIGHT;
				break;
		}
	}

	// DRAW THE SKULL CURSOR
	if (((currentMenu->menuitems[itemOn].status & IT_DISPLAY) == IT_PATCH)
		|| ((currentMenu->menuitems[itemOn].status & IT_DISPLAY) == IT_NOTHING))
	{
		V_DrawScaledPatch(currentMenu->x + SKULLXOFF, cursory - 5, 0,
			W_CachePatchName("M_CURSOR", PU_CACHE));
	}
	else
	{
		V_DrawScaledPatch(currentMenu->x - 24, cursory, 0,
			W_CachePatchName("M_CURSOR", PU_CACHE));
		V_DrawString(currentMenu->x, cursory, highlightflags, currentMenu->menuitems[itemOn].text);
	}
}

static void M_DrawGenericBackgroundMenu(void)
{
	V_DrawPatchFill(W_CachePatchName("SRB2BACK", PU_CACHE));
	M_DrawGenericMenu();
}

static void M_DrawPauseMenu(void)
{
#if 0
	if (!netgame && !multiplayer && (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION || gamestate == GS_VOTING))
	{
		emblem_t *emblem_detail[3] = {NULL, NULL, NULL};
		char emblem_text[3][20];
		INT32 i;

		M_DrawTextBox(27, 16, 32, 6);

		// Draw any and all emblems at the top.
		M_DrawMapEmblems(gamemap, 272, 28);

		if (strlen(mapheaderinfo[gamemap-1]->zonttl) > 0)
		{
			if (strlen(mapheaderinfo[gamemap-1]->actnum) > 0)
				V_DrawString(40, 28, highlightflags, va("%s %s %s", mapheaderinfo[gamemap-1]->lvlttl, mapheaderinfo[gamemap-1]->zonttl, mapheaderinfo[gamemap-1]->actnum));
			else
				V_DrawString(40, 28, highlightflags, va("%s %s", mapheaderinfo[gamemap-1]->lvlttl, mapheaderinfo[gamemap-1]->zonttl));
		}
		else
		{
			if (strlen(mapheaderinfo[gamemap-1]->actnum) > 0)
				V_DrawString(40, 28, highlightflags, va("%s %s", mapheaderinfo[gamemap-1]->lvlttl, mapheaderinfo[gamemap-1]->actnum));
			else
				V_DrawString(40, 28, highlightflags, mapheaderinfo[gamemap-1]->lvlttl);
		}

		// Set up the detail boxes.
		{
			emblem_t *emblem = M_GetLevelEmblems(gamemap);
			while (emblem)
			{
				INT32 emblemslot;
				char targettext[9], currenttext[9];

				switch (emblem->type)
				{
					/*case ET_SCORE:
						snprintf(targettext, 9, "%d", emblem->var);
						snprintf(currenttext, 9, "%u", G_GetBestScore(gamemap));

						targettext[8] = 0;
						currenttext[8] = 0;

						emblemslot = 0;
						break;*/
					case ET_TIME:
						emblemslot = emblem->var; // dumb hack
						snprintf(targettext, 9, "%i:%02i.%02i",
							G_TicsToMinutes((tic_t)emblemslot, false),
							G_TicsToSeconds((tic_t)emblemslot),
							G_TicsToCentiseconds((tic_t)emblemslot));

						emblemslot = (INT32)G_GetBestTime(gamemap); // dumb hack pt ii
						if ((tic_t)emblemslot == UINT32_MAX)
							snprintf(currenttext, 9, "-:--.--");
						else
							snprintf(currenttext, 9, "%i:%02i.%02i",
								G_TicsToMinutes((tic_t)emblemslot, false),
								G_TicsToSeconds((tic_t)emblemslot),
								G_TicsToCentiseconds((tic_t)emblemslot));

						targettext[8] = 0;
						currenttext[8] = 0;

						emblemslot = 1;
						break;
					/*case ET_RINGS:
						snprintf(targettext, 9, "%d", emblem->var);
						snprintf(currenttext, 9, "%u", G_GetBestRings(gamemap));

						targettext[8] = 0;
						currenttext[8] = 0;

						emblemslot = 2;
						break;
					case ET_NGRADE:
						snprintf(targettext, 9, "%u", P_GetScoreForGrade(gamemap, 0, emblem->var));
						snprintf(currenttext, 9, "%u", G_GetBestNightsScore(gamemap, 0));

						targettext[8] = 0;
						currenttext[8] = 0;

						emblemslot = 1;
						break;
					case ET_NTIME:
						emblemslot = emblem->var; // dumb hack pt iii
						snprintf(targettext, 9, "%i:%02i.%02i",
							G_TicsToMinutes((tic_t)emblemslot, false),
							G_TicsToSeconds((tic_t)emblemslot),
							G_TicsToCentiseconds((tic_t)emblemslot));

						emblemslot = (INT32)G_GetBestNightsTime(gamemap, 0); // dumb hack pt iv
						if ((tic_t)emblemslot == UINT32_MAX)
							snprintf(currenttext, 9, "-:--.--");
						else
							snprintf(currenttext, 9, "%i:%02i.%02i",
								G_TicsToMinutes((tic_t)emblemslot, false),
								G_TicsToSeconds((tic_t)emblemslot),
								G_TicsToCentiseconds((tic_t)emblemslot));

						targettext[8] = 0;
						currenttext[8] = 0;

						emblemslot = 2;
						break;*/
					default:
						goto bademblem;
				}
				if (emblem_detail[emblemslot])
					goto bademblem;

				emblem_detail[emblemslot] = emblem;
				snprintf(emblem_text[emblemslot], 20, "%8s /%8s", currenttext, targettext);
				emblem_text[emblemslot][19] = 0;

				bademblem:
				emblem = M_GetLevelEmblems(-1);
			}
		}
		for (i = 0; i < 3; ++i)
		{
			emblem_t *emblem = emblem_detail[i];
			if (!emblem)
				continue;

			if (emblem->collected)
				V_DrawSmallMappedPatch(40, 44 + (i*8), 0, W_CachePatchName(M_GetEmblemPatch(emblem), PU_CACHE),
				                       R_GetTranslationColormap(TC_DEFAULT, M_GetEmblemColor(emblem), GTC_MENUCACHE));
			else
				V_DrawSmallScaledPatch(40, 44 + (i*8), 0, W_CachePatchName("NEEDIT", PU_CACHE));

			switch (emblem->type)
			{
				/*case ET_SCORE:
				case ET_NGRADE:
					V_DrawString(56, 44 + (i*8), highlightflags, "SCORE:");
					break;*/
				case ET_TIME:
				//case ET_NTIME:
					V_DrawString(56, 44 + (i*8), highlightflags, "TIME:");
					break;
				/*case ET_RINGS:
					V_DrawString(56, 44 + (i*8), highlightflags, "RINGS:");
					break;*/
			}
			V_DrawRightAlignedString(284, 44 + (i*8), V_MONOSPACE, emblem_text[i]);
		}
	}
#endif

#ifdef HAVE_DISCORDRPC
	// kind of hackily baked in here
	if (currentMenu == &MPauseDef && discordRequestList != NULL)
	{
		const tic_t freq = TICRATE/2;

		if ((leveltime % freq) >= freq/2)
		{
			V_DrawFixedPatch(204 * FRACUNIT,
				(currentMenu->y + MPauseMenu[mpause_discordrequests].alphaKey - 1) * FRACUNIT,
				FRACUNIT,
				0,
				W_CachePatchName("K_REQUE2", PU_CACHE),
				NULL
			);
		}
	}
#endif

	M_DrawGenericMenu();
}

static void M_DrawCenteredMenu(void)
{
	INT32 x, y, i, cursory = 0;

	// DRAW MENU
	x = currentMenu->x;
	y = currentMenu->y;

	// draw title (or big pic)
	M_DrawMenuTitle();

	for (i = 0; i < currentMenu->numitems; i++)
	{
		if (i == itemOn)
			cursory = y;
		switch (currentMenu->menuitems[i].status & IT_DISPLAY)
		{
			case IT_PATCH:
				if (currentMenu->menuitems[i].patch && currentMenu->menuitems[i].patch[0])
				{
					if (currentMenu->menuitems[i].status & IT_CENTER)
					{
						patch_t *p;
						p = W_CachePatchName(currentMenu->menuitems[i].patch, PU_CACHE);
						V_DrawScaledPatch((BASEVIDWIDTH - SHORT(p->width))/2, y, 0, p);
					}
					else
					{
						V_DrawScaledPatch(x, y, 0,
							W_CachePatchName(currentMenu->menuitems[i].patch, PU_CACHE));
					}
				}
				/* FALLTHRU */
			case IT_NOTHING:
			case IT_DYBIGSPACE:
				y += LINEHEIGHT;
				break;
			case IT_BIGSLIDER:
				M_DrawThermo(x, y, (consvar_t *)currentMenu->menuitems[i].itemaction);
				y += LINEHEIGHT;
				break;
			case IT_STRING:
			case IT_WHITESTRING:
				if (currentMenu->menuitems[i].alphaKey)
					y = currentMenu->y+currentMenu->menuitems[i].alphaKey;
				if (i == itemOn)
					cursory = y;

				if ((currentMenu->menuitems[i].status & IT_DISPLAY)==IT_STRING)
					V_DrawCenteredString(x, y, 0, currentMenu->menuitems[i].text);
				else
					V_DrawCenteredString(x, y, highlightflags, currentMenu->menuitems[i].text);

				// Cvar specific handling
				switch(currentMenu->menuitems[i].status & IT_TYPE)
					case IT_CVAR:
					{
						consvar_t *cv = (consvar_t *)currentMenu->menuitems[i].itemaction;
						switch(currentMenu->menuitems[i].status & IT_CVARTYPE)
						{
							case IT_CV_SLIDER:
								M_DrawSlider(x, y, cv, (i == itemOn));
							case IT_CV_NOPRINT: // color use this
								break;
							case IT_CV_STRING:
								M_DrawTextBox(x, y + 4, MAXSTRINGLENGTH, 1);
								V_DrawString(x + 8, y + 12, V_ALLOWLOWERCASE, cv->string);
								if (skullAnimCounter < 4 && i == itemOn)
									V_DrawCharacter(x + 8 + V_StringWidth(cv->string, 0), y + 12,
										'_' | 0x80, false);
								y += 16;
								break;
							default:
								V_DrawString(BASEVIDWIDTH - x - V_StringWidth(cv->string, 0), y,
									((cv->flags & CV_CHEAT) && !CV_IsSetToDefault(cv) ? warningflags : highlightflags), cv->string);
								break;
						}
						break;
					}
					y += STRINGHEIGHT;
					break;
			case IT_STRING2:
				V_DrawCenteredString(x, y, 0, currentMenu->menuitems[i].text);
				/* FALLTHRU */
			case IT_DYLITLSPACE:
				y += SMALLLINEHEIGHT;
				break;
			case IT_QUESTIONMARKS:
				if (currentMenu->menuitems[i].alphaKey)
					y = currentMenu->y+currentMenu->menuitems[i].alphaKey;

				V_DrawCenteredString(x, y, V_TRANSLUCENT|V_OLDSPACING, M_CreateSecretMenuOption(currentMenu->menuitems[i].text));
				y += SMALLLINEHEIGHT;
				break;
			case IT_GRAYPATCH:
				if (currentMenu->menuitems[i].patch && currentMenu->menuitems[i].patch[0])
					V_DrawMappedPatch(x, y, 0,
						W_CachePatchName(currentMenu->menuitems[i].patch,PU_CACHE), graymap);
				y += LINEHEIGHT;
				break;
		}
	}

	// DRAW THE SKULL CURSOR
	if (((currentMenu->menuitems[itemOn].status & IT_DISPLAY) == IT_PATCH)
		|| ((currentMenu->menuitems[itemOn].status & IT_DISPLAY) == IT_NOTHING))
	{
		V_DrawScaledPatch(x + SKULLXOFF, cursory - 5, 0,
			W_CachePatchName("M_CURSOR", PU_CACHE));
	}
	else
	{
		V_DrawScaledPatch(x - V_StringWidth(currentMenu->menuitems[itemOn].text, 0)/2 - 24, cursory, 0,
			W_CachePatchName("M_CURSOR", PU_CACHE));
		V_DrawCenteredString(x, cursory, highlightflags, currentMenu->menuitems[itemOn].text);
	}
}

//
// M_StringHeight
//
// Find string height from hu_font chars
//
static inline size_t M_StringHeight(const char *string)
{
	size_t h = 8, i;

	for (i = 0; i < strlen(string); i++)
		if (string[i] == '\n')
			h += 8;

	return h;
}

// ==========================================================================
// Extraneous menu patching functions
// ==========================================================================

//
// M_PatchSkinNameTable
//
// Like M_PatchLevelNameTable, but for cv_chooseskin
//
static void M_PatchSkinNameTable(void)
{
	INT32 j;

	memset(skins_cons_t, 0, sizeof (skins_cons_t));

	for (j = 0; j < MAXSKINS; j++)
	{
		if (skins[j].name[0] != '\0')
		{
			skins_cons_t[j].strvalue = skins[j].name;
			skins_cons_t[j].value = j+1;
		}
		else
		{
			skins_cons_t[j].strvalue = NULL;
			skins_cons_t[j].value = 0;
			break;
		}
	}

	j = R_SkinAvailable(cv_skin.string);
	if (j == -1)
		j = 0;

	CV_SetValue(&cv_chooseskin, j+1); // This causes crash sometimes?!

	return;
}

// Call before showing any level-select menus
static void M_PrepareLevelSelect(void)
{
	if (levellistmode != LLM_CREATESERVER)
		CV_SetValue(&cv_nextmap, M_GetFirstLevelInList());
	else
		Newgametype_OnChange(); // Make sure to start on an appropriate map if wads have been added
}

//
// M_CanShowLevelInList
//
// Determines whether to show a given map in the various level-select lists.
// Set gt = -1 to ignore gametype.
//
boolean M_CanShowLevelInList(INT32 mapnum, INT32 gt)
{
	// Random map!
	if (mapnum == -1)
		return (gamestate != GS_TIMEATTACK && !modeattacking);

	// Does the map exist?
	if (!mapheaderinfo[mapnum])
		return false;

	// Does the map have a name?
	if (!mapheaderinfo[mapnum]->lvlttl[0])
		return false;

	switch (levellistmode)
	{
		case LLM_CREATESERVER:
			// Should the map be hidden?
			if (mapheaderinfo[mapnum]->menuflags & LF2_HIDEINMENU && mapnum+1 != gamemap)
				return false;

			if (M_MapLocked(mapnum+1))
				return false; // not unlocked

			/*if (gt == GT_COOP && (mapheaderinfo[mapnum]->typeoflevel & TOL_COOP))
				return true;

			if (gt == GT_COMPETITION && (mapheaderinfo[mapnum]->typeoflevel & TOL_COMPETITION))
				return true;

			if (gt == GT_CTF && (mapheaderinfo[mapnum]->typeoflevel & TOL_CTF))
				return true;

			if ((gt == GT_TAG || gt == GT_HIDEANDSEEK) && (mapheaderinfo[mapnum]->typeoflevel & TOL_TAG))
				return true;*/

			if ((gt == GT_MATCH || gt == GT_TEAMMATCH) && (mapheaderinfo[mapnum]->typeoflevel & TOL_MATCH))
				return true;

			if (gt == GT_RACE && (mapheaderinfo[mapnum]->typeoflevel & TOL_RACE))
				return true;

			return false;

		/*case LLM_LEVELSELECT:
			if (mapheaderinfo[mapnum]->levelselect != maplistoption)
				return false;

			if (M_MapLocked(mapnum+1))
				return false; // not unlocked

			return true;*/
		case LLM_RECORDATTACK:
			/*if (!(mapheaderinfo[mapnum]->menuflags & LF2_RECORDATTACK))
				return false;*/

			if (!(mapheaderinfo[mapnum]->typeoflevel & TOL_RACE))
				return false;

			if (M_MapLocked(mapnum+1))
				return false; // not unlocked

			if (M_SecretUnlocked(SECRET_HELLATTACK))
				return true; // now you're in hell

			if (mapheaderinfo[mapnum]->menuflags & LF2_HIDEINMENU)
				return false; // map hell

			/*if (mapheaderinfo[mapnum]->menuflags & LF2_NOVISITNEEDED)
				return true;

			if (!mapvisited[mapnum])
				return false;*/

			return true;
		/*case LLM_NIGHTSATTACK:
			if (!(mapheaderinfo[mapnum]->menuflags & LF2_NIGHTSATTACK))
				return false;

			if (M_MapLocked(mapnum+1))
				return false; // not unlocked

			if (mapheaderinfo[mapnum]->menuflags & LF2_NOVISITNEEDED)
				return true;

			if (!mapvisited[mapnum])
				return false;

			return true;*/
		default:
			return false;
	}

	// Hmm? Couldn't decide?
	return false;
}

static INT32 M_CountLevelsToShowInList(void)
{
	INT32 mapnum, count = 0;

	for (mapnum = 0; mapnum < NUMMAPS; mapnum++)
		if (M_CanShowLevelInList(mapnum, -1))
			count++;

	return count;
}

static INT32 M_GetFirstLevelInList(void)
{
	INT32 mapnum;

	for (mapnum = 0; mapnum < NUMMAPS; mapnum++)
		if (M_CanShowLevelInList(mapnum, -1))
			return mapnum + 1;

	return 1;
}

// ==================================================
// MESSAGE BOX (aka: a hacked, cobbled together menu)
// ==================================================
static void M_DrawMessageMenu(void);

// Because this is just a hack-ish 'menu', I'm not putting this with the others
static menuitem_t MessageMenu[] =
{
	// TO HACK
	{0,NULL, NULL, NULL,0}
};

menu_t MessageDef =
{
	NULL,               // title
	1,                  // # of menu items
	NULL,               // previous menu       (TO HACK)
	MessageMenu,        // menuitem_t ->
	M_DrawMessageMenu,  // drawing routine ->
	0, 0,               // x, y                (TO HACK)
	0,                  // lastOn, flags       (TO HACK)
	NULL
};


void M_StartMessage(const char *string, void *routine,
	menumessagetype_t itemtype)
{
	size_t max = 0, start = 0, i, strlines;
	static char *message = NULL;
	Z_Free(message);
	message = Z_StrDup(string);
	DEBFILE(message);

	// Rudementary word wrapping.
	// Simple and effective. Does not handle nonuniform letter sizes, colors, etc. but who cares.
	strlines = 0;
	for (i = 0; message[i]; i++)
	{
		if (message[i] == ' ')
		{
			start = i;
			max += 4;
		}
		else if (message[i] == '\n')
		{
			strlines = i;
			start = 0;
			max = 0;
			continue;
		}
		else
			max += 8;

		// Start trying to wrap if presumed length exceeds the screen width.
		if (max >= BASEVIDWIDTH && start > 0)
		{
			message[start] = '\n';
			max -= (start-strlines)*8;
			strlines = start;
			start = 0;
		}
	}

	start = 0;
	max = 0;

	M_StartControlPanel(); // can't put menuactive to true

	if (currentMenu == &MessageDef) // Prevent recursion
		MessageDef.prevMenu = ((demo.playback) ? &PlaybackMenuDef : &MainDef);
	else
		MessageDef.prevMenu = currentMenu;

	MessageDef.menuitems[0].text     = message;
	MessageDef.menuitems[0].alphaKey = (UINT8)itemtype;
	if (!routine && itemtype != MM_NOTHING) itemtype = MM_NOTHING;
	switch (itemtype)
	{
		case MM_NOTHING:
			MessageDef.menuitems[0].status     = IT_MSGHANDLER;
			MessageDef.menuitems[0].itemaction = M_StopMessage;
			break;
		case MM_YESNO:
			MessageDef.menuitems[0].status     = IT_MSGHANDLER;
			MessageDef.menuitems[0].itemaction = routine;
			break;
		case MM_EVENTHANDLER:
			MessageDef.menuitems[0].status     = IT_MSGHANDLER;
			MessageDef.menuitems[0].itemaction = routine;
			break;
	}
	//added : 06-02-98: now draw a textbox around the message
	// compute lenght max and the numbers of lines
	for (strlines = 0; *(message+start); strlines++)
	{
		for (i = 0;i < strlen(message+start);i++)
		{
			if (*(message+start+i) == '\n')
			{
				if (i > max)
					max = i;
				start += i;
				i = (size_t)-1; //added : 07-02-98 : damned!
				start++;
				break;
			}
		}

		if (i == strlen(message+start))
			start += i;
	}

	MessageDef.x = (INT16)((BASEVIDWIDTH  - 8*max-16)/2);
	MessageDef.y = (INT16)((BASEVIDHEIGHT - M_StringHeight(message))/2);

	MessageDef.lastOn = (INT16)((strlines<<8)+max);

	//M_SetupNextMenu();
	currentMenu = &MessageDef;
	itemOn = 0;
}

#define MAXMSGLINELEN 256

static void M_DrawMessageMenu(void)
{
	INT32 y = currentMenu->y;
	size_t i, start = 0;
	INT16 max;
	char string[MAXMSGLINELEN];
	INT32 mlines;
	const char *msg = currentMenu->menuitems[0].text;

	mlines = currentMenu->lastOn>>8;
	max = (INT16)((UINT8)(currentMenu->lastOn & 0xFF)*8);

	// hack: draw RA background in RA menus
	if (gamestate == GS_TIMEATTACK)
		V_DrawPatchFill(W_CachePatchName("SRB2BACK", PU_CACHE));

	M_DrawTextBox(currentMenu->x, y - 8, (max+7)>>3, mlines);

	while (*(msg+start))
	{
		size_t len = strlen(msg+start);

		for (i = 0; i < len; i++)
		{
			if (*(msg+start+i) == '\n')
			{
				memset(string, 0, MAXMSGLINELEN);
				if (i >= MAXMSGLINELEN)
				{
					CONS_Printf("M_DrawMessageMenu: too long segment in %s\n", msg);
					return;
				}
				else
				{
					strncpy(string,msg+start, i);
					string[i] = '\0';
					start += i;
					i = (size_t)-1; //added : 07-02-98 : damned!
					start++;
				}
				break;
			}
		}

		if (i == strlen(msg+start))
		{
			if (i >= MAXMSGLINELEN)
			{
				CONS_Printf("M_DrawMessageMenu: too long segment in %s\n", msg);
				return;
			}
			else
			{
				strcpy(string, msg + start);
				start += i;
			}
		}

		V_DrawString((BASEVIDWIDTH - V_StringWidth(string, 0))/2,y,V_ALLOWLOWERCASE,string);
		y += 8; //SHORT(hu_font[0]->height);
	}
}

// default message handler
static void M_StopMessage(INT32 choice)
{
	(void)choice;
	if (menuactive)
		M_SetupNextMenu(MessageDef.prevMenu);
}

// =========
// IMAGEDEFS
// =========

// Draw an Image Def.  Aka, Help images.
// Defines what image is used in (menuitem_t)->text.
// You can even put multiple images in one menu!
static void M_DrawImageDef(void)
{
	patch_t *patch = W_CachePatchName(currentMenu->menuitems[itemOn].text,PU_CACHE);
	if (patch->width <= BASEVIDWIDTH)
		V_DrawScaledPatch(0,0,0,patch);
	else
		V_DrawSmallScaledPatch(0,0,0,patch);

	if (currentMenu->menuitems[itemOn].alphaKey)
	{
		V_DrawString(2,BASEVIDHEIGHT-10, V_YELLOWMAP, va("%d", (itemOn<<1)-1)); // intentionally not highlightflags, unlike below
		V_DrawRightAlignedString(BASEVIDWIDTH-2,BASEVIDHEIGHT-10, V_YELLOWMAP, va("%d", itemOn<<1)); // ditto
	}
	else
	{
		INT32 x = BASEVIDWIDTH>>1, y = (BASEVIDHEIGHT>>1) - 4;
		x += (itemOn ? 1 : -1)*((BASEVIDWIDTH>>2) + 10);
		V_DrawCenteredString(x, y-10, highlightflags, "USE ARROW KEYS");
		V_DrawCharacter(x - 10 - (skullAnimCounter/5), y,
			'\x1C' | highlightflags, false); // left arrow
		V_DrawCharacter(x + 2 + (skullAnimCounter/5), y,
			'\x1D' | highlightflags, false); // right arrow
		V_DrawCenteredString(x, y+10, highlightflags, "TO LEAF THROUGH");
	}
}

// Handles the ImageDefs.  Just a specialized function that
// uses left and right movement.
static void M_HandleImageDef(INT32 choice)
{
	boolean exitmenu = false;

	switch (choice)
	{
		case KEY_RIGHTARROW:
			if (itemOn >= (INT16)(currentMenu->numitems-1))
				break;
			S_StartSound(NULL, sfx_menu1);
			itemOn++;
			break;

		case KEY_LEFTARROW:
			if (!itemOn)
				break;

			S_StartSound(NULL, sfx_menu1);
			itemOn--;
			break;

		case KEY_ESCAPE:
		case KEY_ENTER:
			exitmenu = true;
			break;
	}

	if (exitmenu)
	{
		if (currentMenu->prevMenu)
			M_SetupNextMenu(currentMenu->prevMenu);
		else
			M_ClearMenus(true);
	}
}

// ======================
// MISC MAIN MENU OPTIONS
// ======================

static void M_AddonsOptions(INT32 choice)
{
	(void)choice;
	Addons_option_Onchange();

	M_SetupNextMenu(&OP_AddonsOptionsDef);
}

#define LOCATIONSTRING1 "Visit \x83SRB2.ORG/MODS\x80 to get & make addons!"
#define LOCATIONSTRING2 "Visit \x88SRB2.ORG/MODS\x80 to get & make addons!"

static void M_Addons(INT32 choice)
{
	const char *pathname = ".";

	(void)choice;

#if 1
	if (cv_addons_option.value == 0)
		pathname = usehome ? srb2home : srb2path;
	else if (cv_addons_option.value == 1)
		pathname = srb2home;
	else if (cv_addons_option.value == 2)
		pathname = srb2path;
	else
#endif
	if (cv_addons_option.value == 3 && *cv_addons_folder.string != '\0')
		pathname = cv_addons_folder.string;

	strlcpy(menupath, pathname, 1024);
	menupathindex[(menudepthleft = menudepth-1)] = strlen(menupath) + 1;

	if (menupath[menupathindex[menudepthleft]-2] != PATHSEP[0])
	{
		menupath[menupathindex[menudepthleft]-1] = PATHSEP[0];
		menupath[menupathindex[menudepthleft]] = 0;
	}
	else
		--menupathindex[menudepthleft];

	if (!preparefilemenu(false, false))
	{
		M_StartMessage(va("No files/folders found.\n\n%s\n\n(Press a key)\n", (recommendedflags == V_SKYMAP ? LOCATIONSTRING2 : LOCATIONSTRING1)),NULL,MM_NOTHING);
		return;
	}
	else
		dir_on[menudepthleft] = 0;

	if (addonsp[0]) // never going to have some provided but not all, saves individually checking
	{
		size_t i;
		for (i = 0; i < NUM_EXT+5; i++)
			W_UnlockCachedPatch(addonsp[i]);
	}

	addonsp[EXT_FOLDER] = W_CachePatchName("M_FFLDR", PU_STATIC);
	addonsp[EXT_UP] = W_CachePatchName("M_FBACK", PU_STATIC);
	addonsp[EXT_NORESULTS] = W_CachePatchName("M_FNOPE", PU_STATIC);
	addonsp[EXT_TXT] = W_CachePatchName("M_FTXT", PU_STATIC);
	addonsp[EXT_CFG] = W_CachePatchName("M_FCFG", PU_STATIC);
	addonsp[EXT_WAD] = W_CachePatchName("M_FWAD", PU_STATIC);
#ifdef USE_KART
	addonsp[EXT_KART] = W_CachePatchName("M_FKART", PU_STATIC);
#endif
	addonsp[EXT_PK3] = W_CachePatchName("M_FPK3", PU_STATIC);
	addonsp[EXT_SOC] = W_CachePatchName("M_FSOC", PU_STATIC);
	addonsp[EXT_LUA] = W_CachePatchName("M_FLUA", PU_STATIC);
	addonsp[NUM_EXT] = W_CachePatchName("M_FUNKN", PU_STATIC);
	addonsp[NUM_EXT+1] = W_CachePatchName("M_FSEL", PU_STATIC);
	addonsp[NUM_EXT+2] = W_CachePatchName("M_FLOAD", PU_STATIC);
	addonsp[NUM_EXT+3] = W_CachePatchName("M_FSRCH", PU_STATIC);
	addonsp[NUM_EXT+4] = W_CachePatchName("M_FSAVE", PU_STATIC);

	MISC_AddonsDef.prevMenu = currentMenu;
	M_SetupNextMenu(&MISC_AddonsDef);
}

#define width 4
#define vpadding 27
#define h (BASEVIDHEIGHT-(2*vpadding))
#define NUMCOLOURS 8 // when toast's coding it's british english hacker fucker
static void M_DrawTemperature(INT32 x, fixed_t t)
{
	INT32 y;

	// bounds check
	if (t > FRACUNIT)
		t = FRACUNIT;
	/*else if (t < 0) -- not needed
		t = 0;*/

	// scale
	if (t > 1)
		t = (FixedMul(h<<FRACBITS, t)>>FRACBITS);

	// border
	V_DrawFill(x - 1, vpadding, 1, h, 120);
	V_DrawFill(x + width, vpadding, 1, h, 120);
	V_DrawFill(x - 1, vpadding-1, width+2, 1, 120);
	V_DrawFill(x - 1, vpadding+h, width+2, 1, 120);

	// bar itself
	y = h;
	if (t)
		for (t = h - t; y > 0; y--)
		{
			UINT8 colours[NUMCOLOURS] = {135, 133, 92, 77, 114, 178, 161, 162};
			UINT8 c;
			if (y <= t) break;
			if (y+vpadding >= BASEVIDHEIGHT/2)
				c = 185;
			else
				c = colours[(NUMCOLOURS*(y-1))/(h/2)];
			V_DrawFill(x, y-1 + vpadding, width, 1, c);
		}

	// fill the rest of the backing
	if (y)
		V_DrawFill(x, vpadding, width, y, 30);
}
#undef width
#undef vpadding
#undef h
#undef NUMCOLOURS

static char *M_AddonsHeaderPath(void)
{
	UINT32 len;
	static char header[1024];

	strlcpy(header, va("%s folder%s", cv_addons_option.string, menupath+menupathindex[menudepth-1]-1), 1024);
	len = strlen(header);
	if (len > 34)
	{
		len = len-34;
		header[len] = header[len+1] = header[len+2] = '.';
	}
	else
		len = 0;

	return header+len;
}

#define UNEXIST S_StartSound(NULL, sfx_s26d);\
		M_SetupNextMenu(MISC_AddonsDef.prevMenu);\
		M_StartMessage(va("\x82%s\x80\nThis folder no longer exists!\nAborting to main menu.\n\n(Press a key)\n", M_AddonsHeaderPath()),NULL,MM_NOTHING)

#define CLEARNAME Z_Free(refreshdirname);\
					refreshdirname = NULL

static boolean prevmajormods = false;

static void M_AddonsClearName(INT32 choice)
{
	if (!majormods || prevmajormods)
	{
		CLEARNAME;
	}
	M_StopMessage(choice);
}

// returns whether to do message draw
static boolean M_AddonsRefresh(void)
{
	if ((refreshdirmenu & REFRESHDIR_NORMAL) && !preparefilemenu(true, false))
	{
		UNEXIST;
		if (refreshdirname)
		{
			CLEARNAME;
		}
		return true;
	}

	if (!majormods && prevmajormods)
		prevmajormods = false;

	if ((refreshdirmenu & REFRESHDIR_ADDFILE) || (majormods && !prevmajormods))
	{
		char *message = NULL;

		if (refreshdirmenu & REFRESHDIR_NOTLOADED)
		{
			S_StartSound(NULL, sfx_s26d);
			if (refreshdirmenu & REFRESHDIR_MAX)
				message = va("%c%s\x80\nMaximum number of addons reached.\nA file could not be loaded.\nIf you wish to play with this addon, restart the game to clear existing ones.\n\n(Press a key)\n", ('\x80' + (highlightflags>>V_CHARCOLORSHIFT)), refreshdirname);
			else
				message = va("%c%s\x80\nA file was not loaded.\nCheck the console log for more information.\n\n(Press a key)\n", ('\x80' + (highlightflags>>V_CHARCOLORSHIFT)), refreshdirname);
		}
		else if (refreshdirmenu & (REFRESHDIR_WARNING|REFRESHDIR_ERROR))
		{
			S_StartSound(NULL, sfx_s224);
			message = va("%c%s\x80\nA file was loaded with %s.\nCheck the console log for more information.\n\n(Press a key)\n", ('\x80' + (highlightflags>>V_CHARCOLORSHIFT)), refreshdirname, ((refreshdirmenu & REFRESHDIR_ERROR) ? "errors" : "warnings"));
		}
		else if (majormods && !prevmajormods)
		{
			S_StartSound(NULL, sfx_s221);
			message = va("%c%s\x80\nYou've loaded a gameplay-modifying addon.\n\nRecord Attack has been disabled, but you\ncan still play alone in local Multiplayer.\n\nIf you wish to play Record Attack mode, restart the game to disable loaded addons.\n\n(Press a key)\n", ('\x80' + (highlightflags>>V_CHARCOLORSHIFT)), refreshdirname);
			prevmajormods = majormods;
		}

		if (message)
		{
			M_StartMessage(message,M_AddonsClearName,MM_EVENTHANDLER);
			return true;
		}

		S_StartSound(NULL, sfx_s221);
		CLEARNAME;
	}

	return false;
}

static void M_DrawAddons(void)
{
	INT32 x, y;
	ssize_t i, m;
	const UINT8 *flashcol = NULL;
	UINT8 hilicol;

	// hack - need to refresh at end of frame to handle addfile...
	if (refreshdirmenu & M_AddonsRefresh())
	{
		M_DrawMessageMenu();
		return;
	}

	if (Playing())
		V_DrawCenteredString(BASEVIDWIDTH/2, 5, warningflags, "Adding files mid-game may cause problems.");
	else
		V_DrawCenteredString(BASEVIDWIDTH/2, 5, 0, (recommendedflags == V_SKYMAP ? LOCATIONSTRING2 : LOCATIONSTRING1));

	if (numwadfiles <= mainwads+1)
		y = 0;
	else if (numwadfiles >= MAX_WADFILES)
		y = FRACUNIT;
	else
	{
		y = FixedDiv(((ssize_t)(numwadfiles) - (ssize_t)(mainwads+1))<<FRACBITS, ((ssize_t)MAX_WADFILES - (ssize_t)(mainwads+1))<<FRACBITS);
		if (y > FRACUNIT) // happens because of how we're shrinkin' it a little
			y = FRACUNIT;
	}

	M_DrawTemperature(BASEVIDWIDTH - 19 - 5, y);

	// DRAW MENU
	x = currentMenu->x;
	y = currentMenu->y + 1;

	hilicol = V_GetStringColormap(highlightflags)[120];

	V_DrawString(x-21, (y - 16) + (lsheadingheight - 12), highlightflags|V_ALLOWLOWERCASE, M_AddonsHeaderPath());
	V_DrawFill(x-21, (y - 16) + (lsheadingheight - 3), MAXSTRINGLENGTH*8+6, 1, hilicol);
	V_DrawFill(x-21, (y - 16) + (lsheadingheight - 2), MAXSTRINGLENGTH*8+6, 1, 30);

	m = (BASEVIDHEIGHT - currentMenu->y + 2) - (y - 1);
	V_DrawFill(x - 21, y - 1, MAXSTRINGLENGTH*8+6, m, 239);

	// scrollbar!
	if (sizedirmenu <= (2*numaddonsshown + 1))
		i = 0;
	else
	{
		ssize_t q = m;
		m = ((2*numaddonsshown + 1) * m)/sizedirmenu;
		if (dir_on[menudepthleft] <= numaddonsshown) // all the way up
			i = 0;
		else if (sizedirmenu <= (dir_on[menudepthleft] + numaddonsshown + 1)) // all the way down
			i = q-m;
		else
			i = ((dir_on[menudepthleft] - numaddonsshown) * (q-m))/(sizedirmenu - (2*numaddonsshown + 1));
	}

	V_DrawFill(x + MAXSTRINGLENGTH*8+5 - 21, (y - 1) + i, 1, m, hilicol);

	// get bottom...
	m = dir_on[menudepthleft] + numaddonsshown + 1;
	if (m > (ssize_t)sizedirmenu)
		m = sizedirmenu;

	// then compute top and adjust bottom if needed!
	if (m < (2*numaddonsshown + 1))
	{
		m = min(sizedirmenu, 2*numaddonsshown + 1);
		i = 0;
	}
	else
		i = m - (2*numaddonsshown + 1);

	if (i != 0)
		V_DrawString(19, y+4 - (skullAnimCounter/5), highlightflags, "\x1A");

	if (skullAnimCounter < 4)
		flashcol = V_GetStringColormap(highlightflags);

	for (; i < m; i++)
	{
		UINT32 flags = V_ALLOWLOWERCASE;
		if (y > BASEVIDHEIGHT) break;
		if (dirmenu[i])
#define type (UINT8)(dirmenu[i][DIR_TYPE])
		{
			if (type & EXT_LOADED)
			{
				flags |= V_TRANSLUCENT;
				V_DrawSmallScaledPatch(x-(16+4), y, V_TRANSLUCENT, addonsp[(type & ~EXT_LOADED)]);
				V_DrawSmallScaledPatch(x-(16+4), y, 0, addonsp[NUM_EXT+2]);
			}
			else
				V_DrawSmallScaledPatch(x-(16+4), y, 0, addonsp[(type & ~EXT_LOADED)]);

			if ((size_t)i == dir_on[menudepthleft])
			{
				V_DrawFixedPatch((x-(16+4))<<FRACBITS, (y)<<FRACBITS, FRACUNIT/2, 0, addonsp[NUM_EXT+1], flashcol);
				flags = V_ALLOWLOWERCASE|highlightflags;
			}

#define charsonside 14
			if (dirmenu[i][DIR_LEN] > (charsonside*2 + 3))
				V_DrawString(x, y+4, flags, va("%.*s...%s", charsonside, dirmenu[i]+DIR_STRING, dirmenu[i]+DIR_STRING+dirmenu[i][DIR_LEN]-(charsonside+1)));
#undef charsonside
			else
				V_DrawString(x, y+4, flags, dirmenu[i]+DIR_STRING);
		}
#undef type
		y += 16;
	}

	if (m != (ssize_t)sizedirmenu)
		V_DrawString(19, y-12 + (skullAnimCounter/5), highlightflags, "\x1B");

	y = BASEVIDHEIGHT - currentMenu->y + 1;

	M_DrawTextBox(x - (21 + 5), y, MAXSTRINGLENGTH, 1);
	if (menusearch[0])
		V_DrawString(x - 18, y + 8, V_ALLOWLOWERCASE, menusearch+1);
	else
		V_DrawString(x - 18, y + 8, V_ALLOWLOWERCASE|V_TRANSLUCENT, "Type to search...");
	if (skullAnimCounter < 4)
		V_DrawCharacter(x - 18 + V_StringWidth(menusearch+1, 0), y + 8,
			'_' | 0x80, false);

	x -= (21 + 5 + 16);
	V_DrawSmallScaledPatch(x, y + 4, (menusearch[0] ? 0 : V_TRANSLUCENT), addonsp[NUM_EXT+3]);

	x = BASEVIDWIDTH - x - 16;
	V_DrawSmallScaledPatch(x, y + 4, ((!majormods) ? 0 : V_TRANSLUCENT), addonsp[NUM_EXT+4]);

	if (modifiedgame)
		V_DrawSmallScaledPatch(x, y + 4, 0, addonsp[NUM_EXT+2]);
}

static void M_AddonExec(INT32 ch)
{
	if (ch != 'y' && ch != KEY_ENTER)
		return;

	S_StartSound(NULL, sfx_zoom);
	COM_BufAddText(va("exec \"%s%s\"", menupath, dirmenu[dir_on[menudepthleft]]+DIR_STRING));
}

#define len menusearch[0]
static boolean M_ChangeStringAddons(INT32 choice)
{
	if (shiftdown && choice >= 32 && choice <= 127)
		choice = shiftxform[choice];

	switch (choice)
	{
		case KEY_DEL:
			if (len)
			{
				len = menusearch[1] = 0;
				return true;
			}
			break;
		case KEY_BACKSPACE:
			if (len)
			{
				menusearch[1+--len] = 0;
				return true;
			}
			break;
		default:
			if (choice >= 32 && choice <= 127)
			{
				if (len < MAXSTRINGLENGTH - 1)
				{
					menusearch[1+len++] = (char)choice;
					menusearch[1+len] = 0;
					return true;
				}
			}
			break;
	}
	return false;
}
#undef len

static void M_HandleAddons(INT32 choice)
{
	boolean exitmenu = false; // exit to previous menu

	if (M_ChangeStringAddons(choice))
	{
		char *tempname = NULL;
		if (dirmenu && dirmenu[dir_on[menudepthleft]])
			tempname = Z_StrDup(dirmenu[dir_on[menudepthleft]]+DIR_STRING); // don't need to I_Error if can't make - not important, just QoL
#if 0 // much slower
		if (!preparefilemenu(true, false))
		{
			UNEXIST;
			return;
		}
#else // streamlined
		searchfilemenu(tempname);
#endif
	}

	switch (choice)
	{
		case KEY_DOWNARROW:
			if (dir_on[menudepthleft] < sizedirmenu-1)
				dir_on[menudepthleft]++;
			S_StartSound(NULL, sfx_menu1);
			break;
		case KEY_UPARROW:
			if (dir_on[menudepthleft])
				dir_on[menudepthleft]--;
			S_StartSound(NULL, sfx_menu1);
			break;
		case KEY_PGDN:
			{
				UINT8 i;
				for (i = numaddonsshown; i && (dir_on[menudepthleft] < sizedirmenu-1); i--)
					dir_on[menudepthleft]++;
			}
			S_StartSound(NULL, sfx_menu1);
			break;
		case KEY_PGUP:
			{
				UINT8 i;
				for (i = numaddonsshown; i && (dir_on[menudepthleft]); i--)
					dir_on[menudepthleft]--;
			}
			S_StartSound(NULL, sfx_menu1);
			break;
		case KEY_ENTER:
			{
				boolean refresh = true;
				if (!dirmenu[dir_on[menudepthleft]])
					S_StartSound(NULL, sfx_s26d);
				else
				{
					switch (dirmenu[dir_on[menudepthleft]][DIR_TYPE])
					{
						case EXT_FOLDER:
							strcpy(&menupath[menupathindex[menudepthleft]],dirmenu[dir_on[menudepthleft]]+DIR_STRING);
							if (menudepthleft)
							{
								menupathindex[--menudepthleft] = strlen(menupath);
								menupath[menupathindex[menudepthleft]] = 0;

								if (!preparefilemenu(false, false))
								{
									S_StartSound(NULL, sfx_s224);
									M_StartMessage(va("%c%s\x80\nThis folder is empty.\n\n(Press a key)\n", ('\x80' + (highlightflags>>V_CHARCOLORSHIFT)), M_AddonsHeaderPath()),NULL,MM_NOTHING);
									menupath[menupathindex[++menudepthleft]] = 0;

									if (!preparefilemenu(true, false))
									{
										UNEXIST;
										return;
									}
								}
								else
								{
									S_StartSound(NULL, sfx_menu1);
									dir_on[menudepthleft] = 1;
								}
								refresh = false;
							}
							else
							{
								S_StartSound(NULL, sfx_s26d);
								M_StartMessage(va("%c%s\x80\nThis folder is too deep to navigate to!\n\n(Press a key)\n", ('\x80' + (highlightflags>>V_CHARCOLORSHIFT)), M_AddonsHeaderPath()),NULL,MM_NOTHING);
								menupath[menupathindex[menudepthleft]] = 0;
							}
							break;
						case EXT_UP:
							S_StartSound(NULL, sfx_menu1);
							menupath[menupathindex[++menudepthleft]] = 0;
							if (!preparefilemenu(false, false))
							{
								UNEXIST;
								return;
							}
							break;
						case EXT_TXT:
							M_StartMessage(va("%c%s\x80\nThis file may not be a console script.\nAttempt to run anyways? \n\n(Press 'Y' to confirm)\n", ('\x80' + (highlightflags>>V_CHARCOLORSHIFT)), dirmenu[dir_on[menudepthleft]]+DIR_STRING),M_AddonExec,MM_YESNO);
							break;
						case EXT_CFG:
							M_AddonExec(KEY_ENTER);
							break;
						case EXT_LUA:
#ifndef HAVE_BLUA
							S_StartSound(NULL, sfx_s26d);
							M_StartMessage(va("%c%s\x80\nThis version of SRB2Kart does not\nhave support for .lua files.\n\n(Press a key)\n", ('\x80' + (highlightflags>>V_CHARCOLORSHIFT)), dirmenu[dir_on[menudepthleft]]+DIR_STRING),NULL,MM_NOTHING);
							break;
#endif
						// else intentional fallthrough
						case EXT_SOC:
						case EXT_WAD:
#ifdef USE_KART
						case EXT_KART:
#endif
						case EXT_PK3:
							COM_BufAddText(va("addfile \"%s%s\"", menupath, dirmenu[dir_on[menudepthleft]]+DIR_STRING));
							break;
						default:
							S_StartSound(NULL, sfx_s26d);
					}
				}
				if (refresh)
					refreshdirmenu |= REFRESHDIR_NORMAL;
			}
			break;

		case KEY_ESCAPE:
			exitmenu = true;
			break;

		default:
			break;
	}
	if (exitmenu)
	{
		closefilemenu(true);

		// Secret menu!
		//MainMenu[secrets].status = (M_AnySecretUnlocked()) ? (IT_STRING | IT_CALL) : (IT_DISABLED);

		if (currentMenu->prevMenu)
			M_SetupNextMenu(currentMenu->prevMenu);
		else
			M_ClearMenus(true);
	}
}

// ---- REPLAY HUT -----
menudemo_t *demolist;

#define DF_ENCORE       0x40
static INT16 replayScrollTitle = 0;
static SINT8 replayScrollDelay = TICRATE, replayScrollDir = 1;

static void PrepReplayList(void)
{
	size_t i;

	if (demolist)
		Z_Free(demolist);

	demolist = Z_Calloc(sizeof(menudemo_t) * sizedirmenu, PU_STATIC, NULL);

	for (i = 0; i < sizedirmenu; i++)
	{
		if (dirmenu[i][DIR_TYPE] == EXT_UP)
		{
			demolist[i].type = MD_SUBDIR;
			sprintf(demolist[i].title, "UP");
		}
		else if (dirmenu[i][DIR_TYPE] == EXT_FOLDER)
		{
			demolist[i].type = MD_SUBDIR;
			strncpy(demolist[i].title, dirmenu[i] + DIR_STRING, 64);
		}
		else
		{
			demolist[i].type = MD_NOTLOADED;
			snprintf(demolist[i].filepath, 255, "%s%s", menupath, dirmenu[i] + DIR_STRING);
			sprintf(demolist[i].title, ".....");
		}
	}
}

void M_ReplayHut(INT32 choice)
{
	(void)choice;

	if (!demo.inreplayhut)
	{
		snprintf(menupath, 1024, "%s"PATHSEP"replay"PATHSEP"online"PATHSEP, srb2home);
		menupathindex[(menudepthleft = menudepth-1)] = strlen(menupath);
	}
	if (!preparefilemenu(false, true))
	{
		M_StartMessage("No replays found.\n\n(Press a key)\n", NULL, MM_NOTHING);
		return;
	}
	else if (!demo.inreplayhut)
		dir_on[menudepthleft] = 0;
	demo.inreplayhut = true;

	replayScrollTitle = 0; replayScrollDelay = TICRATE; replayScrollDir = 1;

	PrepReplayList();

	menuactive = true;
	M_SetupNextMenu(&MISC_ReplayHutDef);
	G_SetGamestate(GS_TIMEATTACK);

	demo.rewinding = false;
	CL_ClearRewinds();

	S_ChangeMusicInternal("replst", true);
}

static void M_HandleReplayHutList(INT32 choice)
{
	switch (choice)
	{
	case KEY_UPARROW:
		if (dir_on[menudepthleft])
			dir_on[menudepthleft]--;
		else
			return;
			//M_PrevOpt();

		S_StartSound(NULL, sfx_menu1);
		replayScrollTitle = 0; replayScrollDelay = TICRATE; replayScrollDir = 1;
		break;

	case KEY_DOWNARROW:
		if (dir_on[menudepthleft] < sizedirmenu-1)
			dir_on[menudepthleft]++;
		else
			return;
			//itemOn = 0; // Not M_NextOpt because that would take us to the extra dummy item

		S_StartSound(NULL, sfx_menu1);
		replayScrollTitle = 0; replayScrollDelay = TICRATE; replayScrollDir = 1;
		break;

	case KEY_ESCAPE:
		M_QuitReplayHut();
		break;

	case KEY_ENTER:
		switch (dirmenu[dir_on[menudepthleft]][DIR_TYPE])
		{
			case EXT_FOLDER:
				strcpy(&menupath[menupathindex[menudepthleft]],dirmenu[dir_on[menudepthleft]]+DIR_STRING);
				if (menudepthleft)
				{
					menupathindex[--menudepthleft] = strlen(menupath);
					menupath[menupathindex[menudepthleft]] = 0;

					if (!preparefilemenu(false, true))
					{
						S_StartSound(NULL, sfx_s224);
						M_StartMessage(va("%c%s\x80\nThis folder is empty.\n\n(Press a key)\n", ('\x80' + (highlightflags>>V_CHARCOLORSHIFT)), M_AddonsHeaderPath()),NULL,MM_NOTHING);
						menupath[menupathindex[++menudepthleft]] = 0;

						if (!preparefilemenu(true, true))
						{
							M_QuitReplayHut();
							return;
						}
					}
					else
					{
						S_StartSound(NULL, sfx_menu1);
						dir_on[menudepthleft] = 1;
						PrepReplayList();
					}
				}
				else
				{
					S_StartSound(NULL, sfx_s26d);
					M_StartMessage(va("%c%s\x80\nThis folder is too deep to navigate to!\n\n(Press a key)\n", ('\x80' + (highlightflags>>V_CHARCOLORSHIFT)), M_AddonsHeaderPath()),NULL,MM_NOTHING);
					menupath[menupathindex[menudepthleft]] = 0;
				}
				break;
			case EXT_UP:
				S_StartSound(NULL, sfx_menu1);
				menupath[menupathindex[++menudepthleft]] = 0;
				if (!preparefilemenu(false, true))
				{
					M_QuitReplayHut();
					return;
				}
				PrepReplayList();
				break;
			default:
				// We can't just use M_SetupNextMenu because that'll run ReplayDef's quitroutine and boot us back to the title screen!
				currentMenu->lastOn = itemOn;
				currentMenu = &MISC_ReplayStartDef;

				replayScrollTitle = 0; replayScrollDelay = TICRATE; replayScrollDir = 1;

				switch (demolist[dir_on[menudepthleft]].addonstatus)
				{
				case DFILE_ERROR_CANNOTLOAD:
					// Only show "Watch Replay Without Addons"
					MISC_ReplayStartMenu[0].status = IT_DISABLED;
					MISC_ReplayStartMenu[1].status = IT_CALL|IT_STRING;
					//MISC_ReplayStartMenu[1].alphaKey = 0;
					MISC_ReplayStartMenu[2].status = IT_DISABLED;
					itemOn = 1;
					break;

				case DFILE_ERROR_NOTLOADED:
				case DFILE_ERROR_INCOMPLETEOUTOFORDER:
					// Show "Load Addons and Watch Replay" and "Watch Replay Without Addons"
					MISC_ReplayStartMenu[0].status = IT_CALL|IT_STRING;
					MISC_ReplayStartMenu[1].status = IT_CALL|IT_STRING;
					//MISC_ReplayStartMenu[1].alphaKey = 10;
					MISC_ReplayStartMenu[2].status = IT_DISABLED;
					itemOn = 0;
					break;

				case DFILE_ERROR_EXTRAFILES:
				case DFILE_ERROR_OUTOFORDER:
				default:
					// Show "Watch Replay"
					MISC_ReplayStartMenu[0].status = IT_DISABLED;
					MISC_ReplayStartMenu[1].status = IT_DISABLED;
					MISC_ReplayStartMenu[2].status = IT_CALL|IT_STRING;
					//MISC_ReplayStartMenu[2].alphaKey = 0;
					itemOn = 2;
					break;
				}
		}

		break;
	}
}

#define SCALEDVIEWWIDTH (vid.width/vid.dupx)
#define SCALEDVIEWHEIGHT (vid.height/vid.dupy)
static void DrawReplayHutReplayInfo(void)
{
	lumpnum_t lumpnum;
	patch_t *patch;
	UINT8 *colormap;
	INT32 x, y, w, h;

	switch (demolist[dir_on[menudepthleft]].type)
	{
	case MD_NOTLOADED:
		V_DrawCenteredString(160, 40, V_SNAPTOTOP, "Loading replay information...");
		break;

	case MD_INVALID:
		V_DrawCenteredString(160, 40, V_SNAPTOTOP|warningflags, "This replay cannot be played.");
		break;

	case MD_SUBDIR:
		break; // Can't think of anything to draw here right now

	case MD_OUTDATED:
		V_DrawThinString(17, 64, V_SNAPTOTOP|V_ALLOWLOWERCASE|V_TRANSLUCENT|highlightflags, "Recorded on an outdated version.");
		/*fallthru*/
	default:
		// Draw level stuff
		x = 15; y = 15;

		//  A 160x100 image of the level as entry MAPxxP
		//CONS_Printf("%d %s\n", demolist[dir_on[menudepthleft]].map, G_BuildMapName(demolist[dir_on[menudepthleft]].map));
		lumpnum = W_CheckNumForName(va("%sP", G_BuildMapName(demolist[dir_on[menudepthleft]].map)));
		if (lumpnum != LUMPERROR)
			patch = W_CachePatchNum(lumpnum, PU_CACHE);
		else
			patch = W_CachePatchName("M_NOLVL", PU_CACHE);

		if (!(demolist[dir_on[menudepthleft]].kartspeed & DF_ENCORE))
			V_DrawSmallScaledPatch(x, y, V_SNAPTOTOP, patch);
		else
		{
			w = SHORT(patch->width);
			h = SHORT(patch->height);
			V_DrawSmallScaledPatch(x+(w>>1), y, V_SNAPTOTOP|V_FLIP, patch);

			{
				static angle_t rubyfloattime = 0;
				const fixed_t rubyheight = FINESINE(rubyfloattime>>ANGLETOFINESHIFT);
				V_DrawFixedPatch((x+(w>>2))<<FRACBITS, ((y+(h>>2))<<FRACBITS) - (rubyheight<<1), FRACUNIT, V_SNAPTOTOP, W_CachePatchName("RUBYICON", PU_CACHE), NULL);
				rubyfloattime += (ANGLE_MAX/NEWTICRATE);
			}
		}

		x += 85;

		if (mapheaderinfo[demolist[dir_on[menudepthleft]].map-1])
			V_DrawString(x, y, V_SNAPTOTOP, G_BuildMapTitle(demolist[dir_on[menudepthleft]].map));
		else
			V_DrawString(x, y, V_SNAPTOTOP|V_ALLOWLOWERCASE|V_TRANSLUCENT, "Level is not loaded.");

		if (demolist[dir_on[menudepthleft]].numlaps)
			V_DrawThinString(x, y+9, V_SNAPTOTOP|V_ALLOWLOWERCASE, va("(%d laps)", demolist[dir_on[menudepthleft]].numlaps));

		V_DrawString(x, y+20, V_SNAPTOTOP|V_ALLOWLOWERCASE, demolist[dir_on[menudepthleft]].gametype == GT_RACE ?
			va("Race (%s speed)", kartspeed_cons_t[demolist[dir_on[menudepthleft]].kartspeed & ~DF_ENCORE].strvalue) :
			"Battle Mode");

		if (!demolist[dir_on[menudepthleft]].standings[0].ranking)
		{
			// No standings were loaded!
			V_DrawString(x, y+39, V_SNAPTOTOP|V_ALLOWLOWERCASE|V_TRANSLUCENT, "No standings available.");


			break;
		}

		V_DrawThinString(x, y+29, V_SNAPTOTOP|highlightflags, "WINNER");
		V_DrawString(x+38, y+30, V_SNAPTOTOP|V_ALLOWLOWERCASE, demolist[dir_on[menudepthleft]].standings[0].name);

		if (demolist[dir_on[menudepthleft]].gametype == GT_RACE)
		{
			V_DrawThinString(x, y+39, V_SNAPTOTOP|highlightflags, "TIME");
			V_DrawRightAlignedString(x+84, y+40, V_SNAPTOTOP, va("%d'%02d\"%02d",
											G_TicsToMinutes(demolist[dir_on[menudepthleft]].standings[0].timeorscore, true),
											G_TicsToSeconds(demolist[dir_on[menudepthleft]].standings[0].timeorscore),
											G_TicsToCentiseconds(demolist[dir_on[menudepthleft]].standings[0].timeorscore)
			));
		}
		else
		{
			V_DrawThinString(x, y+39, V_SNAPTOTOP|highlightflags, "SCORE");
			V_DrawString(x+32, y+40, V_SNAPTOTOP, va("%d", demolist[dir_on[menudepthleft]].standings[0].timeorscore));
		}

		// Character face!
		if (W_CheckNumForName(skins[demolist[dir_on[menudepthleft]].standings[0].skin].facewant) != LUMPERROR)
		{
			patch = facewantprefix[demolist[dir_on[menudepthleft]].standings[0].skin];
			colormap = R_GetTranslationColormap(
				demolist[dir_on[menudepthleft]].standings[0].skin,
				demolist[dir_on[menudepthleft]].standings[0].color,
				GTC_MENUCACHE);
		}
		else
		{
			patch = W_CachePatchName("M_NOWANT", PU_CACHE);
			colormap = R_GetTranslationColormap(
				TC_RAINBOW,
				demolist[dir_on[menudepthleft]].standings[0].color,
				GTC_MENUCACHE);
		}

		V_DrawMappedPatch(BASEVIDWIDTH-15 - SHORT(patch->width), y+20, V_SNAPTOTOP, patch, colormap);

		break;
	}
}

static void M_DrawReplayHut(void)
{
	INT32 x, y, cursory = 0;
	INT16 i;
	INT16 replaylistitem = currentMenu->numitems-2;
	boolean processed_one_this_frame = false;

	static UINT16 replayhutmenuy = 0;

	V_DrawPatchFill(W_CachePatchName("SRB2BACK", PU_CACHE));

	if (cv_vhseffect.value)
		V_DrawVhsEffect(false);

	// Draw menu choices
	x = currentMenu->x;
	y = currentMenu->y;

	if (itemOn > replaylistitem)
	{
		itemOn = replaylistitem;
		dir_on[menudepthleft] = sizedirmenu-1;
		replayScrollTitle = 0; replayScrollDelay = TICRATE; replayScrollDir = 1;
	}
	else if (itemOn < replaylistitem)
	{
		dir_on[menudepthleft] = 0;
		replayScrollTitle = 0; replayScrollDelay = TICRATE; replayScrollDir = 1;
	}

	if (itemOn == replaylistitem)
	{
		INT32 maxy;
		// Scroll menu items if needed
		cursory = y + currentMenu->menuitems[replaylistitem].alphaKey + dir_on[menudepthleft]*10;
		maxy = y + currentMenu->menuitems[replaylistitem].alphaKey + sizedirmenu*10;

		if (cursory > maxy - 20)
			cursory = maxy - 20;

		if (cursory - replayhutmenuy > SCALEDVIEWHEIGHT-50)
			replayhutmenuy += (cursory-SCALEDVIEWHEIGHT-replayhutmenuy + 51)/2;
		else if (cursory - replayhutmenuy < 110)
			replayhutmenuy += (max(0, cursory-110)-replayhutmenuy - 1)/2;
	}
	else
		replayhutmenuy /= 2;

	y -= replayhutmenuy;

	// Draw static menu items
	for (i = 0; i < replaylistitem; i++)
	{
		INT32 localy = y + currentMenu->menuitems[i].alphaKey;

		if (localy < 65)
			continue;

		if (i == itemOn)
			cursory = localy;

		if ((currentMenu->menuitems[i].status & IT_DISPLAY)==IT_STRING)
			V_DrawString(x, localy, V_SNAPTOTOP|V_SNAPTOLEFT, currentMenu->menuitems[i].text);
		else
			V_DrawString(x, localy, V_SNAPTOTOP|V_SNAPTOLEFT|highlightflags, currentMenu->menuitems[i].text);
	}

	y += currentMenu->menuitems[replaylistitem].alphaKey;

	for (i = 0; i < (INT16)sizedirmenu; i++)
	{
		INT32 localy = y+i*10;
		INT32 localx = x;

		if (localy < 65)
			continue;
		if (localy >= SCALEDVIEWHEIGHT)
			break;

		if (demolist[i].type == MD_NOTLOADED && !processed_one_this_frame)
		{
			processed_one_this_frame = true;
			G_LoadDemoInfo(&demolist[i]);
		}

		if (demolist[i].type == MD_SUBDIR)
		{
			localx += 8;
			V_DrawScaledPatch(x - 4, localy, V_SNAPTOTOP|V_SNAPTOLEFT, W_CachePatchName(dirmenu[i][DIR_TYPE] == EXT_UP ? "M_RBACK" : "M_RFLDR", PU_CACHE));
		}

		if (itemOn == replaylistitem && i == (INT16)dir_on[menudepthleft])
		{
			cursory = localy;

			if (replayScrollDelay)
				replayScrollDelay--;
			else if (replayScrollDir > 0)
			{
				if (replayScrollTitle < (V_StringWidth(demolist[i].title, 0) - (SCALEDVIEWWIDTH - (x<<1)))<<1)
					replayScrollTitle++;
				else
				{
					replayScrollDelay = TICRATE;
					replayScrollDir = -1;
				}
			}
			else
			{
				if (replayScrollTitle > 0)
					replayScrollTitle--;
				else
				{
					replayScrollDelay = TICRATE;
					replayScrollDir = 1;
				}
			}

			V_DrawString(localx - (replayScrollTitle>>1), localy, V_SNAPTOTOP|V_SNAPTOLEFT|highlightflags|V_ALLOWLOWERCASE, demolist[i].title);
		}
		else
			V_DrawString(localx, localy, V_SNAPTOTOP|V_SNAPTOLEFT|V_ALLOWLOWERCASE, demolist[i].title);
	}

	// Draw scrollbar
	y = sizedirmenu*10 + currentMenu->menuitems[replaylistitem].alphaKey + 30;
	if (y > SCALEDVIEWHEIGHT-80)
	{
		V_DrawFill(BASEVIDWIDTH-4, 75, 4, SCALEDVIEWHEIGHT-80, V_SNAPTOTOP|V_SNAPTORIGHT|239);
		V_DrawFill(BASEVIDWIDTH-3, 76 + (SCALEDVIEWHEIGHT-80) * replayhutmenuy / y, 2, (((SCALEDVIEWHEIGHT-80) * (SCALEDVIEWHEIGHT-80))-1) / y - 1, V_SNAPTOTOP|V_SNAPTORIGHT|229);
	}

	// Draw the cursor
	V_DrawScaledPatch(currentMenu->x - 24, cursory, V_SNAPTOTOP|V_SNAPTOLEFT,
		W_CachePatchName("M_CURSOR", PU_CACHE));
	V_DrawString(currentMenu->x, cursory, V_SNAPTOTOP|V_SNAPTOLEFT|highlightflags, currentMenu->menuitems[itemOn].text);

	// Now draw some replay info!
	V_DrawFill(10, 10, 300, 60, V_SNAPTOTOP|239);

	if (itemOn == replaylistitem)
	{
		DrawReplayHutReplayInfo();
	}
}

static void M_DrawReplayStartMenu(void)
{
	const char *warning;
	UINT8 i;

	M_DrawGenericBackgroundMenu();

#define STARTY 62-(replayScrollTitle>>1)
	// Draw rankings beyond first
	for (i = 1; i < MAXPLAYERS && demolist[dir_on[menudepthleft]].standings[i].ranking; i++)
	{
		patch_t *patch;
		UINT8 *colormap;

		V_DrawRightAlignedString(BASEVIDWIDTH-100, STARTY + i*20, V_SNAPTOTOP|highlightflags, va("%2d", demolist[dir_on[menudepthleft]].standings[i].ranking));
		V_DrawThinString(BASEVIDWIDTH-96, STARTY + i*20, V_SNAPTOTOP|V_ALLOWLOWERCASE, demolist[dir_on[menudepthleft]].standings[i].name);

		if (demolist[dir_on[menudepthleft]].standings[i].timeorscore == UINT32_MAX-1)
			V_DrawThinString(BASEVIDWIDTH-92, STARTY + i*20 + 9, V_SNAPTOTOP, "NO CONTEST");
		else if (demolist[dir_on[menudepthleft]].gametype == GT_RACE)
			V_DrawRightAlignedString(BASEVIDWIDTH-40, STARTY + i*20 + 9, V_SNAPTOTOP, va("%d'%02d\"%02d",
											G_TicsToMinutes(demolist[dir_on[menudepthleft]].standings[i].timeorscore, true),
											G_TicsToSeconds(demolist[dir_on[menudepthleft]].standings[i].timeorscore),
											G_TicsToCentiseconds(demolist[dir_on[menudepthleft]].standings[i].timeorscore)
			));
		else
			V_DrawString(BASEVIDWIDTH-92, STARTY + i*20 + 9, V_SNAPTOTOP, va("%d", demolist[dir_on[menudepthleft]].standings[i].timeorscore));

		// Character face!
		if (W_CheckNumForName(skins[demolist[dir_on[menudepthleft]].standings[i].skin].facerank) != LUMPERROR)
		{
			patch = facerankprefix[demolist[dir_on[menudepthleft]].standings[i].skin];
			colormap = R_GetTranslationColormap(
				demolist[dir_on[menudepthleft]].standings[i].skin,
				demolist[dir_on[menudepthleft]].standings[i].color,
				GTC_MENUCACHE);
		}
		else
		{
			patch = W_CachePatchName("M_NORANK", PU_CACHE);
			colormap = R_GetTranslationColormap(
				TC_RAINBOW,
				demolist[dir_on[menudepthleft]].standings[i].color,
				GTC_MENUCACHE);
		}

		V_DrawMappedPatch(BASEVIDWIDTH-5 - SHORT(patch->width), STARTY + i*20, V_SNAPTOTOP, patch, colormap);
	}
#undef STARTY

	// Handle scrolling rankings
	if (replayScrollDelay)
		replayScrollDelay--;
	else if (replayScrollDir > 0)
	{
		if (replayScrollTitle < (i*20 - SCALEDVIEWHEIGHT + 100)<<1)
			replayScrollTitle++;
		else
		{
			replayScrollDelay = TICRATE;
			replayScrollDir = -1;
		}
	}
	else
	{
		if (replayScrollTitle > 0)
			replayScrollTitle--;
		else
		{
			replayScrollDelay = TICRATE;
			replayScrollDir = 1;
		}
	}

	V_DrawFill(10, 10, 300, 60, V_SNAPTOTOP|239);
	DrawReplayHutReplayInfo();

	V_DrawString(10, 72, V_SNAPTOTOP|highlightflags|V_ALLOWLOWERCASE, demolist[dir_on[menudepthleft]].title);

	// Draw a warning prompt if needed
	switch (demolist[dir_on[menudepthleft]].addonstatus)
	{
	case DFILE_ERROR_CANNOTLOAD:
		warning = "Some addons in this replay cannot be loaded.\nYou can watch anyway, but desyncs may occur.";
		break;

	case DFILE_ERROR_NOTLOADED:
	case DFILE_ERROR_INCOMPLETEOUTOFORDER:
		warning = "Loading addons will mark your game as modified, and Record Attack may be unavailable.\nYou can watch without loading addons, but desyncs may occur.";
		break;

	case DFILE_ERROR_EXTRAFILES:
		warning = "You have addons loaded that were not present in this replay.\nYou can watch anyway, but desyncs may occur.";
		break;

	case DFILE_ERROR_OUTOFORDER:
		warning = "You have this replay's addons loaded, but they are out of order.\nYou can watch anyway, but desyncs may occur.";
		break;

	default:
		return;
	}

	V_DrawSmallString(4, BASEVIDHEIGHT-14, V_SNAPTOBOTTOM|V_SNAPTOLEFT|V_ALLOWLOWERCASE, warning);
}

static boolean M_QuitReplayHut(void)
{
	// D_StartTitle does its own wipe, since GS_TIMEATTACK is now a complete gamestate.
	menuactive = false;
	D_StartTitle();

	if (demolist)
		Z_Free(demolist);
	demolist = NULL;

	demo.inreplayhut = false;

	return true;
}

static void M_HutStartReplay(INT32 choice)
{
	(void)choice;

	M_ClearMenus(false);
	demo.loadfiles = (itemOn == 0);
	demo.ignorefiles = (itemOn != 0);

	G_DoPlayDemo(demolist[dir_on[menudepthleft]].filepath);
}

void M_SetPlaybackMenuPointer(void)
{
	itemOn = playback_pause;
}

static void M_DrawPlaybackMenu(void)
{
	INT16 i;
	patch_t *icon;
	UINT8 *activemap = R_GetTranslationColormap(TC_RAINBOW, SKINCOLOR_GOLD, GTC_MENUCACHE);
	UINT32 transmap = max(0, (INT32)(leveltime - playback_last_menu_interaction_leveltime - 4*TICRATE)) / 5;
	transmap = min(8, transmap) << V_ALPHASHIFT;

	if (leveltime - playback_last_menu_interaction_leveltime >= 6*TICRATE)
		playback_last_menu_interaction_leveltime = leveltime - 6*TICRATE;

	// Toggle items
	if (paused && !demo.rewinding)
	{
		PlaybackMenu[playback_pause].status = PlaybackMenu[playback_fastforward].status = PlaybackMenu[playback_rewind].status = IT_DISABLED;
		PlaybackMenu[playback_resume].status = PlaybackMenu[playback_advanceframe].status = PlaybackMenu[playback_backframe].status = IT_CALL|IT_STRING;

		if (itemOn >= playback_rewind && itemOn <= playback_fastforward)
			itemOn += playback_backframe - playback_rewind;
	}
	else
	{
		PlaybackMenu[playback_pause].status = PlaybackMenu[playback_fastforward].status = PlaybackMenu[playback_rewind].status = IT_CALL|IT_STRING;
		PlaybackMenu[playback_resume].status = PlaybackMenu[playback_advanceframe].status = PlaybackMenu[playback_backframe].status = IT_DISABLED;

		if (itemOn >= playback_backframe && itemOn <= playback_advanceframe)
			itemOn -= playback_backframe - playback_rewind;
	}

	if (modeattacking)
	{
		for (i = playback_viewcount; i <= playback_view4; i++)
			PlaybackMenu[i].status = IT_DISABLED;
		PlaybackMenu[playback_freecamera].alphaKey = 72;
		PlaybackMenu[playback_quit].alphaKey = 88;

		currentMenu->x = BASEVIDWIDTH/2 - 52;
	}
	else
	{
		PlaybackMenu[playback_viewcount].status = IT_ARROWS|IT_STRING;

		for (i = 0; i <= splitscreen; i++)
			PlaybackMenu[playback_view1+i].status = IT_ARROWS|IT_STRING;
		for (i = splitscreen+1; i < 4; i++)
			PlaybackMenu[playback_view1+i].status = IT_DISABLED;

		PlaybackMenu[playback_freecamera].alphaKey = 156;
		PlaybackMenu[playback_quit].alphaKey = 172;
		currentMenu->x = BASEVIDWIDTH/2 - 88;
	}

	// wip
	//M_DrawTextBox(currentMenu->x-68, currentMenu->y-7, 15, 15);
	//M_DrawCenteredMenu();

	for (i = 0; i < currentMenu->numitems; i++)
	{
		UINT8 *inactivemap = NULL;

		if (i >= playback_view1 && i <= playback_view4)
		{
			if (modeattacking) continue;

			if (splitscreen >= i - playback_view1)
			{
				INT32 ply = displayplayers[i - playback_view1];

				icon = facerankprefix[players[ply].skin];
				if (i != itemOn)
					inactivemap = R_GetTranslationColormap(players[ply].skin, players[ply].skincolor, GTC_MENUCACHE);
			}
			else if (currentMenu->menuitems[i].patch && W_CheckNumForName(currentMenu->menuitems[i].patch) != LUMPERROR)
				icon = W_CachePatchName(currentMenu->menuitems[i].patch, PU_CACHE);
			else
				icon = W_CachePatchName("PLAYRANK", PU_CACHE); // temp
		}
		else if (currentMenu->menuitems[i].status == IT_DISABLED)
			continue;
		else if (currentMenu->menuitems[i].patch && W_CheckNumForName(currentMenu->menuitems[i].patch) != LUMPERROR)
			icon = W_CachePatchName(currentMenu->menuitems[i].patch, PU_CACHE);
		else
			icon = W_CachePatchName("PLAYRANK", PU_CACHE); // temp

		if ((i == playback_fastforward && cv_playbackspeed.value > 1) || (i == playback_rewind && demo.rewinding))
			V_DrawMappedPatch(currentMenu->x + currentMenu->menuitems[i].alphaKey, currentMenu->y, transmap|V_SNAPTOTOP, icon, R_GetTranslationColormap(TC_RAINBOW, SKINCOLOR_JAWZ, GTC_MENUCACHE));
		else
			V_DrawMappedPatch(currentMenu->x + currentMenu->menuitems[i].alphaKey, currentMenu->y, transmap|V_SNAPTOTOP, icon, (i == itemOn) ? activemap : inactivemap);

		if (i == itemOn)
		{
			V_DrawCharacter(currentMenu->x + currentMenu->menuitems[i].alphaKey + 4, currentMenu->y + 14,
				'\x1A' | transmap|V_SNAPTOTOP|highlightflags, false);

			V_DrawCenteredString(BASEVIDWIDTH/2, currentMenu->y + 18, transmap|V_SNAPTOTOP|V_ALLOWLOWERCASE, currentMenu->menuitems[i].text);

			if ((currentMenu->menuitems[i].status & IT_TYPE) == IT_ARROWS)
			{
				char *str;

				if (!(i == playback_viewcount && splitscreen == 3))
					V_DrawCharacter(BASEVIDWIDTH/2 - 4, currentMenu->y + 28 - (skullAnimCounter/5),
						'\x1A' | transmap|V_SNAPTOTOP|highlightflags, false); // up arrow

				if (!(i == playback_viewcount && splitscreen == 0))
					V_DrawCharacter(BASEVIDWIDTH/2 - 4, currentMenu->y + 48 + (skullAnimCounter/5),
						'\x1B' | transmap|V_SNAPTOTOP|highlightflags, false); // down arrow

				switch (i)
				{
				case playback_viewcount:
					str = va("%d", splitscreen+1);
					break;

				case playback_view1:
				case playback_view2:
				case playback_view3:
				case playback_view4:
					str = player_names[displayplayers[i - playback_view1]]; // 0 to 3
					break;

				default: // shouldn't ever be reached but whatever
					continue;
				}

				V_DrawCenteredString(BASEVIDWIDTH/2, currentMenu->y + 38, transmap|V_SNAPTOTOP|V_ALLOWLOWERCASE|highlightflags, str);
			}
		}
	}
}

static void M_PlaybackRewind(INT32 choice)
{
	static tic_t lastconfirmtime;

	(void)choice;

	if (!demo.rewinding)
	{
		if (paused)
		{
			G_ConfirmRewind(leveltime-1);
			paused = true;
			S_PauseAudio();
		}
		else
			demo.rewinding = paused = true;
	}
	else if (lastconfirmtime + TICRATE/2 < I_GetTime())
	{
		lastconfirmtime = I_GetTime();
		G_ConfirmRewind(leveltime);
	}

	CV_SetValue(&cv_playbackspeed, 1);
}

static void M_PlaybackPause(INT32 choice)
{
	(void)choice;

	paused = !paused;

	if (demo.rewinding)
	{
		G_ConfirmRewind(leveltime);
		paused = true;
		S_PauseAudio();
	}
	else if (paused)
		S_PauseAudio();
	else
		S_ResumeAudio();

	CV_SetValue(&cv_playbackspeed, 1);
}

static void M_PlaybackFastForward(INT32 choice)
{
	(void)choice;

	if (demo.rewinding)
	{
		G_ConfirmRewind(leveltime);
		paused = false;
		S_ResumeAudio();
	}
	CV_SetValue(&cv_playbackspeed, cv_playbackspeed.value == 1 ? 4 : 1);
}

static void M_PlaybackAdvance(INT32 choice)
{
	(void)choice;

	paused = false;
	TryRunTics(1);
	paused = true;
}


static void M_PlaybackSetViews(INT32 choice)
{

	if (demo.freecam)
		return;	// not here.

	if (choice > 0)
	{
		if (splitscreen < 3)
			G_AdjustView(splitscreen + 2, 0, true);
	}
	else if (splitscreen)
	{
		splitscreen--;
		R_ExecuteSetViewSize();
	}
}

static void M_PlaybackAdjustView(INT32 choice)
{
	G_AdjustView(itemOn - playback_viewcount, (choice > 0) ? 1 : -1, true);
}

// this one's rather tricky
static void M_PlaybackToggleFreecam(INT32 choice)
{
	(void)choice;
	M_ClearMenus(true);

	// remove splitscreen:
	splitscreen = 0;
	R_ExecuteSetViewSize();

	P_InitCameraCmd();	// init camera controls
	if (!demo.freecam)	// toggle on
	{
		demo.freecam = true;
		democam.cam = &camera[0];	// this is rather useful
	}
	else	// toggle off
	{
		demo.freecam = false;
		// reset democam vars:
		democam.cam = NULL;
		democam.turnheld = false;
		democam.keyboardlook = false;	// reset only these. localangle / aiming gets set before the cam does anything anyway
	}
}


static void M_PlaybackQuit(INT32 choice)
{
	(void)choice;
	G_StopDemo();

	if (demo.inreplayhut)
		M_ReplayHut(choice);
	else if (modeattacking)
	{
		M_EndModeAttackRun();
		S_ChangeMusicInternal("racent", true);
	}
	else
		D_StartTitle();
}

static void M_PandorasBox(INT32 choice)
{
	(void)choice;
	CV_StealthSetValue(&cv_dummyrings, max(players[consoleplayer].health - 1, 0));
	CV_StealthSetValue(&cv_dummylives, players[consoleplayer].lives);
	CV_StealthSetValue(&cv_dummycontinues, players[consoleplayer].continues);
	M_SetupNextMenu(&SR_PandoraDef);
}

static boolean M_ExitPandorasBox(void)
{
	if (cv_dummyrings.value != max(players[consoleplayer].health - 1, 0))
		COM_ImmedExecute(va("setrings %d", cv_dummyrings.value));
	if (cv_dummylives.value != players[consoleplayer].lives)
		COM_ImmedExecute(va("setlives %d", cv_dummylives.value));
	if (cv_dummycontinues.value != players[consoleplayer].continues)
		COM_ImmedExecute(va("setcontinues %d", cv_dummycontinues.value));
	return true;
}

static void M_ChangeLevel(INT32 choice)
{
	char mapname[6];
	(void)choice;

	strlcpy(mapname, G_BuildMapName(cv_nextmap.value), sizeof (mapname));
	strlwr(mapname);
	mapname[5] = '\0';

	M_ClearMenus(true);
	COM_BufAddText(va("map %s -gametype \"%s\"\n", mapname, cv_newgametype.string));
}

static void M_ConfirmSpectate(INT32 choice)
{
	(void)choice;
	// We allow switching to spectator even if team changing is not allowed
	M_ClearMenus(true);
	COM_ImmedExecute("changeteam spectator");
}

static void M_ConfirmEnterGame(INT32 choice)
{
	(void)choice;
	if (!cv_allowteamchange.value)
	{
		M_StartMessage(M_GetText("The server is not allowing\nteam changes at this time.\nPress a key.\n"), NULL, MM_NOTHING);
		return;
	}
	M_ClearMenus(true);
	COM_ImmedExecute("changeteam playing");
}

static void M_ConfirmTeamScramble(INT32 choice)
{
	(void)choice;
	M_ClearMenus(true);

	COM_ImmedExecute(va("teamscramble %d", cv_dummyscramble.value+1));
}

static void M_ConfirmTeamChange(INT32 choice)
{
	(void)choice;

	if (cv_dummymenuplayer.value > splitscreen+1)
		return;

	if (!cv_allowteamchange.value && cv_dummyteam.value)
	{
		M_StartMessage(M_GetText("The server is not allowing\nteam changes at this time.\nPress a key.\n"), NULL, MM_NOTHING);
		return;
	}

	M_ClearMenus(true);

	switch (cv_dummymenuplayer.value)
	{
		case 1:
		default:
			COM_ImmedExecute(va("changeteam %s", cv_dummyteam.string));
			break;
		case 2:
			COM_ImmedExecute(va("changeteam2 %s", cv_dummyteam.string));
			break;
		case 3:
			COM_ImmedExecute(va("changeteam3 %s", cv_dummyteam.string));
			break;
		case 4:
			COM_ImmedExecute(va("changeteam4 %s", cv_dummyteam.string));
			break;
	}
}

static void M_ConfirmSpectateChange(INT32 choice)
{
	(void)choice;

	if (cv_dummymenuplayer.value > splitscreen+1)
		return;

	if (!cv_allowteamchange.value && cv_dummyspectate.value)
	{
		M_StartMessage(M_GetText("The server is not allowing\nteam changes at this time.\nPress a key.\n"), NULL, MM_NOTHING);
		return;
	}

	M_ClearMenus(true);

	switch (cv_dummymenuplayer.value)
	{
		case 1:
		default:
			COM_ImmedExecute(va("changeteam %s", cv_dummyspectate.string));
			break;
		case 2:
			COM_ImmedExecute(va("changeteam2 %s", cv_dummyspectate.string));
			break;
		case 3:
			COM_ImmedExecute(va("changeteam3 %s", cv_dummyspectate.string));
			break;
		case 4:
			COM_ImmedExecute(va("changeteam4 %s", cv_dummyspectate.string));
			break;
	}
}

static void M_Options(INT32 choice)
{
	(void)choice;

	// if the player is not admin or server, disable gameplay & server options
	OP_MainMenu[4].status = OP_MainMenu[5].status = (Playing() && !(server || IsPlayerAdmin(consoleplayer))) ? (IT_GRAYEDOUT) : (IT_STRING|IT_SUBMENU);

	OP_MainMenu[8].status = (Playing()) ? (IT_GRAYEDOUT) : (IT_STRING|IT_CALL); // Play credits

#ifdef HAVE_DISCORDRPC
	OP_DataOptionsMenu[4].status = (Playing()) ? (IT_GRAYEDOUT) : (IT_STRING|IT_SUBMENU); // Erase data
#else
	OP_DataOptionsMenu[3].status = (Playing()) ? (IT_GRAYEDOUT) : (IT_STRING|IT_SUBMENU); // Erase data
#endif

	OP_GameOptionsMenu[3].status =
		(M_SecretUnlocked(SECRET_ENCORE)) ? (IT_CVAR|IT_STRING) : IT_SECRET; // cv_kartencore

	OP_MainDef.prevMenu = currentMenu;
	M_SetupNextMenu(&OP_MainDef);
}

static void M_Manual(INT32 choice)
{
	(void)choice;

	MISC_HelpDef.prevMenu = (choice == INT32_MAX ? NULL : currentMenu);
	M_SetupNextMenu(&MISC_HelpDef);
}

static void M_RetryResponse(INT32 ch)
{
	if (ch != 'y' && ch != KEY_ENTER)
		return;

	if (!&players[consoleplayer] || netgame || multiplayer) // Should never happen!
		return;

	M_ClearMenus(true);
	G_SetRetryFlag();
}

static void M_Retry(INT32 choice)
{
	(void)choice;
	M_StartMessage(M_GetText("Start this race over?\n\n(Press 'Y' to confirm)\n"),M_RetryResponse,MM_YESNO);
}

static void M_SelectableClearMenus(INT32 choice)
{
	(void)choice;
	M_ClearMenus(true);
}

void M_RefreshPauseMenu(void)
{
#ifdef HAVE_DISCORDRPC
	if (discordRequestList != NULL)
	{
		MPauseMenu[mpause_discordrequests].status = IT_STRING | IT_SUBMENU;
	}
	else
	{
		MPauseMenu[mpause_discordrequests].status = IT_GRAYEDOUT;
	}
#endif
}

// ======
// CHEATS
// ======

static void M_UltimateCheat(INT32 choice)
{
	(void)choice;
	I_Quit();
}

static void M_GetAllEmeralds(INT32 choice)
{
	(void)choice;

	emeralds = ((EMERALD7)*2)-1;
	M_StartMessage(M_GetText("You now have all 7 emeralds.\nUse them wisely.\nWith great power comes great ring drain.\n"),NULL,MM_NOTHING);

	G_SetGameModified(multiplayer, true);
}

static void M_DestroyRobotsResponse(INT32 ch)
{
	if (ch != 'y' && ch != KEY_ENTER)
		return;

	// Destroy all robots
	P_DestroyRobots();

	G_SetGameModified(multiplayer, true);
}

static void M_DestroyRobots(INT32 choice)
{
	(void)choice;

	M_StartMessage(M_GetText("Do you want to destroy all\nrobots in the current level?\n\n(Press 'Y' to confirm)\n"),M_DestroyRobotsResponse,MM_YESNO);
}

/*static void M_LevelSelectWarp(INT32 choice)
{
	boolean fromloadgame = (currentMenu == &SP_LevelSelectDef);

	(void)choice;

	if (W_CheckNumForName(G_BuildMapName(cv_nextmap.value)) == LUMPERROR)
	{
//		CONS_Alert(CONS_WARNING, "Internal game map '%s' not found\n", G_BuildMapName(cv_nextmap.value));
		return;
	}

	startmap = (INT16)(cv_nextmap.value);

	fromlevelselect = true;

	if (fromloadgame)
		G_LoadGame((UINT32)cursaveslot, startmap);
	else
	{
		cursaveslot = -1;
		M_SetupChoosePlayer(0);
	}
}*/

// ========
// SKY ROOM
// ========

UINT8 skyRoomMenuTranslations[MAXUNLOCKABLES];

static char *M_GetConditionString(condition_t cond)
{
	switch(cond.type)
	{
		case UC_PLAYTIME:
			return va("Play for %i:%02i:%02i",
				G_TicsToHours(cond.requirement),
				G_TicsToMinutes(cond.requirement, false),
				G_TicsToSeconds(cond.requirement));
		case UC_MATCHESPLAYED:
			return va("Play %d matches", cond.requirement);
		case UC_GAMECLEAR:
			if (cond.requirement > 1)
				return va("Beat game %d times", cond.requirement);
			else
				return va("Beat the game");
		case UC_ALLEMERALDS:
			if (cond.requirement > 1)
				return va("Beat game w/ all emeralds %d times", cond.requirement);
			else
				return va("Beat game w/ all emeralds");
		case UC_OVERALLTIME:
			return va("Get overall time of %i:%02i:%02i",
				G_TicsToHours(cond.requirement),
				G_TicsToMinutes(cond.requirement, false),
				G_TicsToSeconds(cond.requirement));
		case UC_MAPVISITED:
			return va("Visit %s", G_BuildMapTitle(cond.requirement-1));
		case UC_MAPBEATEN:
			return va("Beat %s", G_BuildMapTitle(cond.requirement-1));
		case UC_MAPALLEMERALDS:
			return va("Beat %s w/ all emeralds", G_BuildMapTitle(cond.requirement-1));
		case UC_MAPTIME:
			return va("Beat %s in %i:%02i.%02i", G_BuildMapTitle(cond.extrainfo1-1),
				G_TicsToMinutes(cond.requirement, true),
				G_TicsToSeconds(cond.requirement),
				G_TicsToCentiseconds(cond.requirement));
		case UC_TOTALEMBLEMS:
			return va("Get %d medals", cond.requirement);
		case UC_EXTRAEMBLEM:
			return va("Get \"%s\" medal", extraemblems[cond.requirement-1].name);
		default:
			return NULL;
	}
}

#define NUMCHECKLIST 23
static void M_DrawChecklist(void)
{
	UINT32 i, line = 0, c;
	INT32 lastid;
	boolean secret = false;

	for (i = 0; i < MAXUNLOCKABLES; i++)
	{
		const char *secretname;

		secret = (!M_Achieved(unlockables[i].showconditionset - 1) && !unlockables[i].unlocked);

		if (unlockables[i].name[0] == 0 || unlockables[i].nochecklist
		|| !unlockables[i].conditionset || unlockables[i].conditionset > MAXCONDITIONSETS
		|| (unlockables[i].type == SECRET_HELLATTACK && secret)) // TODO: turn this into an unlockable setting instead of tying it to Hell Attack
			continue;

		++line;
		secretname = M_CreateSecretMenuOption(unlockables[i].name);

		V_DrawString(8, (line*8), V_RETURN8|(unlockables[i].unlocked ? recommendedflags : warningflags), (secret ? secretname : unlockables[i].name));

		if (conditionSets[unlockables[i].conditionset - 1].numconditions)
		{
			c = 0;
			lastid = -1;

			for (c = 0; c < conditionSets[unlockables[i].conditionset - 1].numconditions; c++)
			{
				condition_t cond = conditionSets[unlockables[i].conditionset - 1].condition[c];
				UINT8 achieved = M_CheckCondition(&cond);
				char *str = M_GetConditionString(cond);
				const char *secretstr = M_CreateSecretMenuOption(str);

				if (!str)
					continue;

				++line;

				if (lastid == -1 || cond.id != (UINT32)lastid)
				{
					V_DrawString(16, (line*8), V_MONOSPACE|V_ALLOWLOWERCASE|(achieved ? highlightflags : 0), "*");
					V_DrawString(32, (line*8), V_MONOSPACE|V_ALLOWLOWERCASE|(achieved ? highlightflags : 0), (secret ? secretstr : str));
				}
				else
				{
					V_DrawString(32, (line*8), V_MONOSPACE|V_ALLOWLOWERCASE|(achieved ? highlightflags : 0), (secret ? "?" : "&"));
					V_DrawString(48, (line*8), V_MONOSPACE|V_ALLOWLOWERCASE|(achieved ? highlightflags : 0), (secret ? secretstr : str));
				}

				lastid = cond.id;
			}
		}

		++line;

		if (line >= NUMCHECKLIST)
			break;
	}
}
#undef NUMCHECKLIST

#define NUMHINTS 5
static void M_EmblemHints(INT32 choice)
{
	(void)choice;
	SR_EmblemHintMenu[0].status = (M_SecretUnlocked(SECRET_ITEMFINDER)) ? (IT_CVAR|IT_STRING) : (IT_SECRET);
	M_SetupNextMenu(&SR_EmblemHintDef);
	itemOn = 1; // always start on back.
}

static void M_DrawEmblemHints(void)
{
	INT32 i, j = 0;
	UINT32 collected = 0;
	emblem_t *emblem;
	const char *hint;

	for (i = 0; i < numemblems; i++)
	{
		emblem = &emblemlocations[i];
		if (emblem->level != gamemap || emblem->type > ET_SKIN)
			continue;

		if (emblem->collected)
		{
			collected = recommendedflags;
			V_DrawMappedPatch(12, 12+(28*j), 0, W_CachePatchName(M_GetEmblemPatch(emblem), PU_CACHE),
				R_GetTranslationColormap(TC_DEFAULT, M_GetEmblemColor(emblem), GTC_MENUCACHE));
		}
		else
		{
			collected = 0;
			V_DrawScaledPatch(12, 12+(28*j), 0, W_CachePatchName("NEEDIT", PU_CACHE));
		}

		if (emblem->hint[0])
			hint = emblem->hint;
		else
			hint = M_GetText("No hints available.");
		hint = V_WordWrap(40, BASEVIDWIDTH-12, 0, hint);
		V_DrawString(40, 8+(28*j), V_RETURN8|V_ALLOWLOWERCASE|collected, hint);

		if (++j >= NUMHINTS)
			break;
	}
	if (!j)
		V_DrawCenteredString(160, 48, highlightflags, "No hidden medals on this map.");

	M_DrawGenericMenu();
}

static void M_DrawSkyRoom(void)
{
	INT32 i, y = 0;
	INT32 lengthstring = 0;

	M_DrawGenericMenu();

	if (currentMenu == &OP_SoundOptionsDef)
	{
		V_DrawRightAlignedString(BASEVIDWIDTH - currentMenu->x,
			currentMenu->y+currentMenu->menuitems[0].alphaKey,
			(sound_disabled ? warningflags : highlightflags),
			(sound_disabled ? "OFF" : "ON"));

		V_DrawRightAlignedString(BASEVIDWIDTH - currentMenu->x,
			currentMenu->y+currentMenu->menuitems[2].alphaKey,
			(digital_disabled ? warningflags : highlightflags),
			(digital_disabled ? "OFF" : "ON"));

		/*V_DrawRightAlignedString(BASEVIDWIDTH - currentMenu->x,
			currentMenu->y+currentMenu->menuitems[5].alphaKey,
			(midi_disabled ? warningflags : highlightflags),
			(midi_disabled ? "OFF" : "ON"));*/

		if (itemOn == 0)
			lengthstring = 8*(sound_disabled ? 3 : 2);
		else if (itemOn == 2)
			lengthstring = 8*(digital_disabled ? 3 : 2);
		/*else if (itemOn == 5)
			lengthstring = 8*(midi_disabled ? 3 : 2);*/
	}

	for (i = 0; i < currentMenu->numitems; ++i)
	{
		if (currentMenu->menuitems[i].itemaction == M_HandleSoundTest)
		{
			y = currentMenu->menuitems[i].alphaKey;
			break;
		}
	}

	if (y)
	{
		y += currentMenu->y;

		V_DrawRightAlignedString(BASEVIDWIDTH - currentMenu->x, y, highlightflags, cv_soundtest.string);
		if (cv_soundtest.value)
			V_DrawRightAlignedString(BASEVIDWIDTH - currentMenu->x, y + 8, highlightflags, S_sfx[cv_soundtest.value].name);

		if (i == itemOn)
			lengthstring = V_StringWidth(cv_soundtest.string, 0);
	}

	if (lengthstring)
	{
		V_DrawCharacter(BASEVIDWIDTH - currentMenu->x - 10 - lengthstring - (skullAnimCounter/5), currentMenu->y+currentMenu->menuitems[itemOn].alphaKey,
			'\x1C' | highlightflags, false); // left arrow
		V_DrawCharacter(BASEVIDWIDTH - currentMenu->x + 2 + (skullAnimCounter/5), currentMenu->y+currentMenu->menuitems[itemOn].alphaKey,
			'\x1D' | highlightflags, false); // right arrow
	}
}

static void M_HandleSoundTest(INT32 choice)
{
	boolean exitmenu = false; // exit to previous menu

	switch (choice)
	{
		case KEY_DOWNARROW:
			M_NextOpt();
			S_StartSound(NULL, sfx_menu1);
			break;
		case KEY_UPARROW:
			M_PrevOpt();
			S_StartSound(NULL, sfx_menu1);
			break;
		case KEY_BACKSPACE:
		case KEY_ESCAPE:
			exitmenu = true;
			break;

		case KEY_RIGHTARROW:
			CV_AddValue(&cv_soundtest, 1);
			break;
		case KEY_LEFTARROW:
			CV_AddValue(&cv_soundtest, -1);
			break;
		case KEY_ENTER:
			S_StopSounds();
			S_StartSound(NULL, cv_soundtest.value);
			break;

		default:
			break;
	}
	if (exitmenu)
	{
		if (currentMenu->prevMenu)
			M_SetupNextMenu(currentMenu->prevMenu);
		else
			M_ClearMenus(true);
	}
}

// Entering secrets menu
/*static void M_SecretsMenu(INT32 choice)
{
	INT32 i, j, ul;
	UINT8 done[MAXUNLOCKABLES];
	UINT16 curheight;

	(void)choice;

	// Clear all before starting
	for (i = 1; i < MAXUNLOCKABLES+1; ++i)
		SR_MainMenu[i].status = IT_DISABLED;

	memset(skyRoomMenuTranslations, 0, sizeof(skyRoomMenuTranslations));
	memset(done, 0, sizeof(done));

	for (i = 1; i < MAXUNLOCKABLES+1; ++i)
	{
		curheight = UINT16_MAX;
		ul = -1;

		// Autosort unlockables
		for (j = 0; j < MAXUNLOCKABLES; ++j)
		{
			if (!unlockables[j].height || done[j] || unlockables[j].type < 0)
				continue;

			if (unlockables[j].height < curheight)
			{
				curheight = unlockables[j].height;
				ul = j;
			}
		}
		if (ul < 0)
			break;

		done[ul] = true;

		skyRoomMenuTranslations[i-1] = (UINT8)ul;
		SR_MainMenu[i].text = unlockables[ul].name;
		SR_MainMenu[i].alphaKey = (UINT8)unlockables[ul].height;

		if (unlockables[ul].type == SECRET_HEADER)
		{
			SR_MainMenu[i].status = IT_HEADER;
			continue;
		}

		SR_MainMenu[i].status = IT_SECRET;

		if (unlockables[ul].unlocked)
		{
			switch (unlockables[ul].type)
			{
				case SECRET_LEVELSELECT:
					SR_MainMenu[i].status = IT_STRING|IT_CALL;
					SR_MainMenu[i].itemaction = M_CustomLevelSelect;
					break;
				case SECRET_WARP:
					SR_MainMenu[i].status = IT_STRING|IT_CALL;
					SR_MainMenu[i].itemaction = M_CustomWarp;
					break;
				case SECRET_CREDITS:
					SR_MainMenu[i].status = IT_STRING|IT_CALL;
					SR_MainMenu[i].itemaction = M_Credits;
					break;
				case SECRET_SOUNDTEST:
					SR_MainMenu[i].status = IT_STRING|IT_KEYHANDLER;
					SR_MainMenu[i].itemaction = M_HandleSoundTest;
				default:
					break;
			}
		}
	}

	M_SetupNextMenu(&SR_MainDef);
}*/

// ==================
// NEW GAME FUNCTIONS
// ==================

/*INT32 ultimate_selectable = false;

static void M_NewGame(void)
{
	fromlevelselect = false;

	startmap = spstage_start;
	CV_SetValue(&cv_newgametype, GT_RACE); // SRB2kart

	M_SetupChoosePlayer(0);
}*/

/*static void M_CustomWarp(INT32 choice)
{
	INT32 ul = skyRoomMenuTranslations[choice-1];

	startmap = (INT16)(unlockables[ul].variable);

	M_SetupChoosePlayer(0);
}*/

static void M_Credits(INT32 choice)
{
	(void)choice;
	cursaveslot = -2;
	M_ClearMenus(true);
	F_StartCredits();
}

/*static void M_CustomLevelSelect(INT32 choice)
{
	INT32 ul = skyRoomMenuTranslations[choice-1];

	SR_LevelSelectDef.prevMenu = currentMenu;
	levellistmode = LLM_LEVELSELECT;
	maplistoption = (UINT8)(unlockables[ul].variable);
	if (M_CountLevelsToShowInList() == 0)
	{
		M_StartMessage(M_GetText("No selectable levels found.\n"),NULL,MM_NOTHING);
		return;
	}

	M_PrepareLevelSelect();
	M_SetupNextMenu(&SR_LevelSelectDef);
}*/

// ==================
// SINGLE PLAYER MENU
// ==================

#if 0 // Bring this back when we have actual single-player
static void M_SinglePlayerMenu(INT32 choice)
{
	(void)choice;
	SP_MainMenu[sprecordattack].status =
		(M_SecretUnlocked(SECRET_RECORDATTACK)) ? IT_CALL|IT_STRING : IT_SECRET;
	/*SP_MainMenu[spnightsmode].status =
		(M_SecretUnlocked(SECRET_NIGHTSMODE)) ? IT_CALL|IT_STRING : IT_SECRET;*/

	M_SetupNextMenu(&SP_MainDef);
}
#endif

/*static void M_LoadGameLevelSelect(INT32 choice)
{
	(void)choice;
	levellistmode = LLM_LEVELSELECT;
	maplistoption = 1;
	if (M_CountLevelsToShowInList() == 0)
	{
		M_StartMessage(M_GetText("No selectable levels found.\n"),NULL,MM_NOTHING);
		return;
	}

	SP_LevelSelectDef.prevMenu = currentMenu;

	M_PrepareLevelSelect();
	M_SetupNextMenu(&SP_LevelSelectDef);
}*/

// ==============
// LOAD GAME MENU
// ==============

/*static INT32 saveSlotSelected = 0;
static short menumovedir = 0;

static void M_DrawLoadGameData(void)
{
	INT32 ecks;
	INT32 i;

	ecks = SP_LoadDef.x + 24;
	M_DrawTextBox(SP_LoadDef.x-12,144, 24, 4);

	if (saveSlotSelected == NOSAVESLOT) // last slot is play without saving
	{
		if (ultimate_selectable)
		{
			V_DrawCenteredString(ecks + 68, 144, V_ORANGEMAP, "ULTIMATE MODE");
			V_DrawCenteredString(ecks + 68, 156, 0, "NO RINGS, NO ONE-UPS,");
			V_DrawCenteredString(ecks + 68, 164, 0, "NO CONTINUES, ONE LIFE,");
			V_DrawCenteredString(ecks + 68, 172, 0, "FINAL DESTINATION.");
		}
		else
		{
			V_DrawCenteredString(ecks + 68, 144, V_ORANGEMAP, "PLAY WITHOUT SAVING");
			V_DrawCenteredString(ecks + 68, 156, 0, "THIS GAME WILL NOT BE");
			V_DrawCenteredString(ecks + 68, 164, 0, "SAVED, BUT YOU CAN STILL");
			V_DrawCenteredString(ecks + 68, 172, 0, "GET MEDALS AND SECRETS.");
		}
		return;
	}

	if (savegameinfo[saveSlotSelected].lives == -42) // Empty
	{
		V_DrawCenteredString(ecks + 68, 160, 0, "NO DATA");
		return;
	}

	if (savegameinfo[saveSlotSelected].lives == -666) // savegame is bad
	{
		V_DrawCenteredString(ecks + 68, 144, warningflags, "CORRUPT SAVE FILE");
		V_DrawCenteredString(ecks + 68, 156, 0, "THIS SAVE FILE");
		V_DrawCenteredString(ecks + 68, 164, 0, "CAN NOT BE LOADED.");
		V_DrawCenteredString(ecks + 68, 172, 0, "DELETE USING BACKSPACE.");
		return;
	}

	// Draw the back sprite, it looks ugly if we don't
	V_DrawScaledPatch(SP_LoadDef.x, 144+8, 0, livesback);
	if (savegameinfo[saveSlotSelected].skincolor == 0)
		V_DrawScaledPatch(SP_LoadDef.x,144+8,0,W_CachePatchName(skins[savegameinfo[saveSlotSelected].skinnum].face, PU_CACHE));
	else
	{
		UINT8 *colormap = R_GetTranslationColormap(savegameinfo[saveSlotSelected].skinnum, savegameinfo[saveSlotSelected].skincolor, GTC_MENUCACHE);
		V_DrawMappedPatch(SP_LoadDef.x,144+8,0,W_CachePatchName(skins[savegameinfo[saveSlotSelected].skinnum].face, PU_CACHE), colormap);
	}

	V_DrawString(ecks + 12, 152, 0, savegameinfo[saveSlotSelected].playername);

#ifdef SAVEGAMES_OTHERVERSIONS
	if (savegameinfo[saveSlotSelected].gamemap & 16384)
		V_DrawCenteredString(ecks + 68, 144, warningflags, "OUTDATED SAVE FILE!");
#endif

	if (savegameinfo[saveSlotSelected].gamemap & 8192)
		V_DrawString(ecks + 12, 160, recommendedflags, "CLEAR!");
	else
		V_DrawString(ecks + 12, 160, 0, va("%s", savegameinfo[saveSlotSelected].levelname));

	// Use the big face pic for lives, duh. :3
	V_DrawScaledPatch(ecks + 12, 175, 0, W_CachePatchName("STLIVEX", PU_HUDGFX));
	V_DrawTallNum(ecks + 40, 172, 0, savegameinfo[saveSlotSelected].lives);

	// Absolute ridiculousness, condensed into another function.
	V_DrawContinueIcon(ecks + 58, 182, 0, savegameinfo[saveSlotSelected].skinnum, savegameinfo[saveSlotSelected].skincolor);
	V_DrawScaledPatch(ecks + 68, 175, 0, W_CachePatchName("STLIVEX", PU_HUDGFX));
	V_DrawTallNum(ecks + 96, 172, 0, savegameinfo[saveSlotSelected].continues);

	for (i = 0; i < 7; ++i)
	{
		if (savegameinfo[saveSlotSelected].numemeralds & (1 << i))
			V_DrawScaledPatch(ecks + 104 + (i * 8), 172, 0, tinyemeraldpics[i]);
	}
}

#define LOADBARHEIGHT SP_LoadDef.y + (LINEHEIGHT * (j+1)) + ymod
#define CURSORHEIGHT  SP_LoadDef.y + (LINEHEIGHT*3) - 1
static void M_DrawLoad(void)
{
	INT32 i, j;
	INT32 ymod = 0, offset = 0;

	M_DrawMenuTitle();

	if (menumovedir != 0) //movement illusion
	{
		ymod = (-(LINEHEIGHT/4))*menumovedir;
		offset = ((menumovedir > 0) ? -1 : 1);
	}

	V_DrawCenteredString(BASEVIDWIDTH/2, 40, 0, "Press backspace to delete a save.");

	for (i = MAXSAVEGAMES + saveSlotSelected - 2 + offset, j = 0;i <= MAXSAVEGAMES + saveSlotSelected + 2 + offset; i++, j++)
	{
		if ((menumovedir < 0 && j == 4) || (menumovedir > 0 && j == 0))
			continue; //this helps give the illusion of movement

		M_DrawSaveLoadBorder(SP_LoadDef.x, LOADBARHEIGHT);

		if ((i%MAXSAVEGAMES) == NOSAVESLOT) // play without saving
		{
			if (ultimate_selectable)
				V_DrawCenteredString(SP_LoadDef.x+92, LOADBARHEIGHT - 1, V_ORANGEMAP, "ULTIMATE MODE");
			else
				V_DrawCenteredString(SP_LoadDef.x+92, LOADBARHEIGHT - 1, V_ORANGEMAP, "PLAY WITHOUT SAVING");
			continue;
		}

		if (savegameinfo[i%MAXSAVEGAMES].lives == -42)
			V_DrawString(SP_LoadDef.x-6, LOADBARHEIGHT - 1, V_TRANSLUCENT, "NO DATA");
		else if (savegameinfo[i%MAXSAVEGAMES].lives == -666)
			V_DrawString(SP_LoadDef.x-6, LOADBARHEIGHT - 1, warningflags, "CORRUPT SAVE FILE");
		else if (savegameinfo[i%MAXSAVEGAMES].gamemap & 8192)
			V_DrawString(SP_LoadDef.x-6, LOADBARHEIGHT - 1, recommendedflags, "CLEAR!");
		else
			V_DrawString(SP_LoadDef.x-6, LOADBARHEIGHT - 1, 0, va("%s", savegameinfo[i%MAXSAVEGAMES].levelname));

		//Draw the save slot number on the right side
		V_DrawRightAlignedString(SP_LoadDef.x+192, LOADBARHEIGHT - 1, 0, va("%d",(i%MAXSAVEGAMES) + 1));
	}

	//Draw cursors on both sides.
	V_DrawScaledPatch( 32, CURSORHEIGHT, 0, W_CachePatchName("M_CURSOR", PU_CACHE));
	V_DrawScaledPatch(274, CURSORHEIGHT, 0, W_CachePatchName("M_CURSOR", PU_CACHE));

	M_DrawLoadGameData();

	//finishing the movement illusion
	if (menumovedir)
		menumovedir += ((menumovedir > 0) ? 1 : -1);
	if (abs(menumovedir) > 3)
		menumovedir = 0;
}
#undef LOADBARHEIGHT
#undef CURSORHEIGHT

//
// User wants to load this game
//
static void M_LoadSelect(INT32 choice)
{
	(void)choice;

	if (saveSlotSelected == NOSAVESLOT) //last slot is play without saving
	{
		M_NewGame();
		cursaveslot = -1;
		return;
	}

	if (!FIL_ReadFileOK(va(savegamename, saveSlotSelected)))
	{
		// This slot is empty, so start a new game here.
		M_NewGame();
	}
	else if (savegameinfo[saveSlotSelected].gamemap & 8192) // Completed
		M_LoadGameLevelSelect(saveSlotSelected + 1);
	else
		G_LoadGame((UINT32)saveSlotSelected, 0);

	cursaveslot = saveSlotSelected;
}

#define VERSIONSIZE 16
#define BADSAVE { savegameinfo[slot].lives = -666; Z_Free(savebuffer); return; }
#define CHECKPOS if (save_p >= end_p) BADSAVE
// Reads the save file to list lives, level, player, etc.
// Tails 05-29-2003
static void M_ReadSavegameInfo(UINT32 slot)
{
	size_t length;
	char savename[255];
	UINT8 *savebuffer;
	UINT8 *end_p; // buffer end point, don't read past here
	UINT8 *save_p;
	INT32 fake; // Dummy variable
	char temp[sizeof(timeattackfolder)];
	char vcheck[VERSIONSIZE];
#ifdef SAVEGAMES_OTHERVERSIONS
	boolean oldversion = false;
#endif

	sprintf(savename, savegamename, slot);

	length = FIL_ReadFile(savename, &savebuffer);
	if (length == 0)
	{
		savegameinfo[slot].lives = -42;
		return;
	}

	end_p = savebuffer + length;

	// skip the description field
	save_p = savebuffer;

	// Version check
	memset(vcheck, 0, sizeof (vcheck));
	sprintf(vcheck, "version %d", VERSION);
	if (strcmp((const char *)save_p, (const char *)vcheck))
	{
#ifdef SAVEGAMES_OTHERVERSIONS
		oldversion = true;
#else
		BADSAVE // Incompatible versions?
#endif
	}
	save_p += VERSIONSIZE;

	// dearchive all the modifications
	// P_UnArchiveMisc()

	CHECKPOS
	fake = READINT16(save_p);

	if (((fake-1) & 8191) >= NUMMAPS) BADSAVE

	if(!mapheaderinfo[(fake-1) & 8191])
	{
		savegameinfo[slot].levelname[0] = '\0';
		savegameinfo[slot].actnum = 0;
	}
	else
	{
		strcpy(savegameinfo[slot].levelname, mapheaderinfo[(fake-1) & 8191]->lvlttl);
		savegameinfo[slot].actnum = 0; //mapheaderinfo[(fake-1) & 8191]->actnum
	}

#ifdef SAVEGAMES_OTHERVERSIONS
	if (oldversion)
	{
		if (fake == 24) //meh, let's count old Clear! saves too
			fake |= 8192;
		fake |= 16384; // marker for outdated version
	}
#endif
	savegameinfo[slot].gamemap = fake;

	CHECKPOS
	fake = READUINT16(save_p)-357; // emeralds

	savegameinfo[slot].numemeralds = (UINT8)fake;

	CHECKPOS
	READSTRINGN(save_p, temp, sizeof(temp)); // mod it belongs to

	if (strcmp(temp, timeattackfolder)) BADSAVE

	// P_UnArchivePlayer()
	CHECKPOS
	savegameinfo[slot].skincolor = READUINT8(save_p);
	CHECKPOS
	savegameinfo[slot].skinnum = READUINT8(save_p);

	CHECKPOS
	(void)READINT32(save_p); // Score

	CHECKPOS
	savegameinfo[slot].lives = READINT32(save_p); // lives
	CHECKPOS
	savegameinfo[slot].continues = READINT32(save_p); // continues

	if (fake & (1<<10))
	{
		CHECKPOS
		savegameinfo[slot].botskin = READUINT8(save_p);
		if (savegameinfo[slot].botskin-1 >= numskins)
			savegameinfo[slot].botskin = 0;
		CHECKPOS
		savegameinfo[slot].botcolor = READUINT8(save_p); // because why not.
	}
	else
		savegameinfo[slot].botskin = 0;

	if (savegameinfo[slot].botskin)
		snprintf(savegameinfo[slot].playername, 36, "%s & %s",
			skins[savegameinfo[slot].skinnum].realname,
			skins[savegameinfo[slot].botskin-1].realname);
	else
		strcpy(savegameinfo[slot].playername, skins[savegameinfo[slot].skinnum].realname);

	savegameinfo[slot].playername[31] = 0;

	// File end marker check
	CHECKPOS
	if (READUINT8(save_p) != 0x1d) BADSAVE;

	// done
	Z_Free(savebuffer);
}
#undef CHECKPOS
#undef BADSAVE

//
// M_ReadSaveStrings
//  read the strings from the savegame files
//  and put it in savegamestrings global variable
//
static void M_ReadSaveStrings(void)
{
	FILE *handle;
	UINT32 i;
	char name[256];

	for (i = 0; i < MAXSAVEGAMES; i++)
	{
		snprintf(name, sizeof name, savegamename, i);
		name[sizeof name - 1] = '\0';

		handle = fopen(name, "rb");
		if (handle == NULL)
		{
			savegameinfo[i].lives = -42;
			continue;
		}
		fclose(handle);
		M_ReadSavegameInfo(i);
	}
}

//
// User wants to delete this game
//
static void M_SaveGameDeleteResponse(INT32 ch)
{
	char name[256];

	if (ch != 'y' && ch != KEY_ENTER)
		return;

	// delete savegame
	snprintf(name, sizeof name, savegamename, saveSlotSelected);
	name[sizeof name - 1] = '\0';
	remove(name);

	// Refresh savegame menu info
	M_ReadSaveStrings();
}

static void M_HandleLoadSave(INT32 choice)
{
	boolean exitmenu = false; // exit to previous menu

	switch (choice)
	{
		case KEY_DOWNARROW:
			S_StartSound(NULL, sfx_menu1);
			++saveSlotSelected;
			if (saveSlotSelected >= MAXSAVEGAMES)
				saveSlotSelected -= MAXSAVEGAMES;
			menumovedir = 1;
			break;

		case KEY_UPARROW:
			S_StartSound(NULL, sfx_menu1);
			--saveSlotSelected;
			if (saveSlotSelected < 0)
				saveSlotSelected += MAXSAVEGAMES;
			menumovedir = -1;
			break;

		case KEY_ENTER:
			S_StartSound(NULL, sfx_menu1);
			if (savegameinfo[saveSlotSelected].lives != -666) // don't allow loading of "bad saves"
				M_LoadSelect(saveSlotSelected);
			break;

		case KEY_ESCAPE:
			exitmenu = true;
			break;

		case KEY_BACKSPACE:
			S_StartSound(NULL, sfx_menu1);
			// Don't allow people to 'delete' "Play without Saving."
			// Nor allow people to 'delete' slots with no saves in them.
			if (saveSlotSelected != NOSAVESLOT && savegameinfo[saveSlotSelected].lives != -42)
				M_StartMessage(M_GetText("Are you sure you want to delete\nthis save game?\n\n(Press 'Y' to confirm)\n"),M_SaveGameDeleteResponse,MM_YESNO);
			break;
	}
	if (exitmenu)
	{
		if (currentMenu->prevMenu)
			M_SetupNextMenu(currentMenu->prevMenu);
		else
			M_ClearMenus(true);
	}
}

//
// Selected from SRB2 menu
//
static void M_LoadGame(INT32 choice)
{
	(void)choice;

	M_ReadSaveStrings();
	M_SetupNextMenu(&SP_LoadDef);
}

//
// Used by cheats to force the save menu to a specific spot.
//
void M_ForceSaveSlotSelected(INT32 sslot)
{
	// Already there? Out of bounds? Whatever, then!
	if (sslot == saveSlotSelected || sslot >= MAXSAVEGAMES)
		return;

	// Figure out whether to display up movement or down movement
	menumovedir = (saveSlotSelected - sslot) > 0 ? -1 : 1;
	if (abs(saveSlotSelected - sslot) > (MAXSAVEGAMES>>1))
		menumovedir *= -1;

	saveSlotSelected = sslot;
}

// ================
// CHARACTER SELECT
// ================

static void M_SetupChoosePlayer(INT32 choice)
{
	(void)choice;

	if (mapheaderinfo[startmap-1] && mapheaderinfo[startmap-1]->forcecharacter[0] != '\0')
	{
		M_ChoosePlayer(0); //oh for crying out loud just get STARTED, it doesn't matter!
		return;
	}

	if (Playing() == false)
	{
		S_StopMusic();
		S_ChangeMusicInternal("chrsel", true);
	}

	SP_PlayerDef.prevMenu = currentMenu;
	M_SetupNextMenu(&SP_PlayerDef);
	char_scroll = itemOn*128*FRACUNIT; // finish scrolling the menu
	Z_Free(char_notes);
	char_notes = NULL;
}

// Draw the choose player setup menu, had some fun with player anim
static void M_DrawSetupChoosePlayerMenu(void)
{
	const INT32 my = 24;
	patch_t *patch;
	INT32 i, o, j;
	char *picname;

	// Black BG
	V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);
	//V_DrawPatchFill(W_CachePatchName("SRB2BACK", PU_CACHE));

	// Character select profile images!1
	M_DrawTextBox(0, my, 16, 20);

	if (abs(itemOn*128*FRACUNIT - char_scroll) > 256*FRACUNIT)
		char_scroll = itemOn*128*FRACUNIT;
	else if (itemOn*128*FRACUNIT - char_scroll > 128*FRACUNIT)
		char_scroll += 48*FRACUNIT;
	else if (itemOn*128*FRACUNIT - char_scroll < -128*FRACUNIT)
		char_scroll -= 48*FRACUNIT;
	else if (itemOn*128*FRACUNIT > char_scroll+16*FRACUNIT)
		char_scroll += 16*FRACUNIT;
	else if (itemOn*128*FRACUNIT < char_scroll-16*FRACUNIT)
		char_scroll -= 16*FRACUNIT;
	else // close enough.
		char_scroll = itemOn*128*FRACUNIT; // just be exact now.
	i = (char_scroll+16*FRACUNIT)/(128*FRACUNIT);
	o = ((char_scroll/FRACUNIT)+16)%128;

	// prev character
	if (i-1 >= 0 && PlayerMenu[i-1].status != IT_DISABLED
	&& o < 32)
	{
		picname = description[i-1].picname;
		if (picname[0] == '\0')
		{
			picname = strtok(Z_StrDup(description[i-1].skinname), "&");
			for (j = 0; j < numskins; j++)
				if (stricmp(skins[j].name, picname) == 0)
				{
					Z_Free(picname);
					picname = skins[j].charsel;
					break;
				}
			if (j == numskins) // AAAAAAAAAA
				picname = skins[0].charsel;
		}
		patch = W_CachePatchName(picname, PU_CACHE);
		if (SHORT(patch->width) >= 256)
			V_DrawCroppedPatch(8<<FRACBITS, (my + 8)<<FRACBITS, FRACUNIT/2, 0, patch, 0, SHORT(patch->height) - 64 + o*2, SHORT(patch->width), SHORT(patch->height));
		else
			V_DrawCroppedPatch(8<<FRACBITS, (my + 8)<<FRACBITS, FRACUNIT, 0, patch, 0, SHORT(patch->height) - 32 + o, SHORT(patch->width), SHORT(patch->height));
		W_UnlockCachedPatch(patch);
	}

	// next character
	if (i+1 < currentMenu->numitems && PlayerMenu[i+1].status != IT_DISABLED
	&& o < 128)
	{
		picname = description[i+1].picname;
		if (picname[0] == '\0')
		{
			picname = strtok(Z_StrDup(description[i+1].skinname), "&");
			for (j = 0; j < numskins; j++)
				if (stricmp(skins[j].name, picname) == 0)
				{
					Z_Free(picname);
					picname = skins[j].charsel;
					break;
				}
			if (j == numskins) // AAAAAAAAAA
				picname = skins[0].charsel;
		}
		patch = W_CachePatchName(picname, PU_CACHE);
		if (SHORT(patch->width) >= 256)
			V_DrawCroppedPatch(8<<FRACBITS, (my + 168 - o)<<FRACBITS, FRACUNIT/2, 0, patch, 0, 0, SHORT(patch->width), o*2);
		else
			V_DrawCroppedPatch(8<<FRACBITS, (my + 168 - o)<<FRACBITS, FRACUNIT, 0, patch, 0, 0, SHORT(patch->width), o);
		W_UnlockCachedPatch(patch);
	}

	// current character
	if (i < currentMenu->numitems && PlayerMenu[i].status != IT_DISABLED)
	{
		picname = description[i].picname;
		if (picname[0] == '\0')
		{
			picname = strtok(Z_StrDup(description[i].skinname), "&");
			for (j = 0; j < numskins; j++)
				if (stricmp(skins[j].name, picname) == 0)
				{
					Z_Free(picname);
					picname = skins[j].charsel;
					break;
				}
			if (j == numskins) // AAAAAAAAAA
				picname = skins[0].charsel;
		}
		patch = W_CachePatchName(picname, PU_CACHE);
		if (o >= 0 && o <= 32)
		{
			if (SHORT(patch->width) >= 256)
				V_DrawSmallScaledPatch(8, my + 40 - o, 0, patch);
			else
				V_DrawScaledPatch(8, my + 40 - o, 0, patch);
		}
		else
		{
			if (SHORT(patch->width) >= 256)
				V_DrawCroppedPatch(8<<FRACBITS, (my + 8)<<FRACBITS, FRACUNIT/2, 0, patch, 0, (o - 32)*2, SHORT(patch->width), SHORT(patch->height));
			else
				V_DrawCroppedPatch(8<<FRACBITS, (my + 8)<<FRACBITS, FRACUNIT, 0, patch, 0, o - 32, SHORT(patch->width), SHORT(patch->height));
		}
		W_UnlockCachedPatch(patch);
	}

	// draw title (or big pic)
	M_DrawMenuTitle();

	// Character description
	M_DrawTextBox(136, my, 21, 20);
	if (!char_notes)
		char_notes = V_WordWrap(0, 21*8, V_ALLOWLOWERCASE, description[itemOn].notes);
	V_DrawString(146, my + 9, V_RETURN8|V_ALLOWLOWERCASE, char_notes);
}

// Chose the player you want to use Tails 03-02-2002
static void M_ChoosePlayer(INT32 choice)
{
	char *skin1,*skin2;
	INT32 skinnum;
	//boolean ultmode = (ultimate_selectable && SP_PlayerDef.prevMenu == &SP_LoadDef && saveSlotSelected == NOSAVESLOT);

	// skip this if forcecharacter
	if (mapheaderinfo[startmap-1] && mapheaderinfo[startmap-1]->forcecharacter[0] == '\0')
	{
		// M_SetupChoosePlayer didn't call us directly, that means we've been properly set up.
		char_scroll = itemOn*128*FRACUNIT; // finish scrolling the menu
		M_DrawSetupChoosePlayerMenu(); // draw the finally selected character one last time for the fadeout
	}
	M_ClearMenus(true);

	skin1 = strtok(description[choice].skinname, "&");
	skin2 = strtok(NULL, "&");

	if (skin2) {
		// this character has a second skin
		skinnum = R_SkinAvailable(skin1);
		botskin = (UINT8)(R_SkinAvailable(skin2)+1);
		botingame = true;

		botcolor = skins[botskin-1].prefcolor;

		// undo the strtok
		description[choice].skinname[strlen(skin1)] = '&';
	} else {
		skinnum = R_SkinAvailable(description[choice].skinname);
		botingame = false;
		botskin = 0;
		botcolor = 0;
	}

	if (startmap != spstage_start)
		cursaveslot = -1;

	lastmapsaved = 0;
	gamecomplete = false;

	G_DeferedInitNew(false, G_BuildMapName(startmap), (UINT8)skinnum, 0, fromlevelselect);
	COM_BufAddText("dummyconsvar 1\n"); // G_DeferedInitNew doesn't do this
}*/

// ===============
// STATISTICS MENU
// ===============

static INT32 statsLocation;
static INT32 statsMax;
static INT16 statsMapList[NUMMAPS+1];

static void M_Statistics(INT32 choice)
{
	INT16 i, j = 0;

	(void)choice;

	memset(statsMapList, 0, sizeof(statsMapList));

	for (i = 0; i < NUMMAPS; i++)
	{
		if (!mapheaderinfo[i] || mapheaderinfo[i]->lvlttl[0] == '\0')
			continue;

		if (!(mapheaderinfo[i]->typeoflevel & TOL_RACE) // TOL_SP
			|| (mapheaderinfo[i]->menuflags & (LF2_HIDEINSTATS|LF2_HIDEINMENU)))
			continue;

		if (M_MapLocked(i+1)) // !mapvisited[i]
			continue;

		statsMapList[j++] = i;
	}
	statsMapList[j] = -1;
	statsMax = j - 11 + numextraemblems;
	statsLocation = 0;

	if (statsMax < 0)
		statsMax = 0;

	M_SetupNextMenu(&SP_LevelStatsDef);
}

static void M_DrawStatsMaps(int location)
{
	INT32 y = 80, i = -1;
	INT16 mnum;
	extraemblem_t *exemblem;
	boolean dotopname = true, dobottomarrow = (location < statsMax);

	if (location)
		V_DrawCharacter(10, y-(skullAnimCounter/5),
			'\x1A' | highlightflags, false); // up arrow

	while (statsMapList[++i] != -1)
	{
		if (location)
		{
			--location;
			continue;
		}
		else if (dotopname)
		{
			V_DrawString(20,  y, highlightflags, "LEVEL NAME");
			V_DrawString(256, y, highlightflags, "MEDALS");
			y += 8;
			dotopname = false;
		}

		mnum = statsMapList[i];
		M_DrawMapEmblems(mnum+1, 295, y);

		if (mapheaderinfo[mnum]->levelflags & LF_NOZONE)
			V_DrawString(20, y, 0, va("%s %s",
				mapheaderinfo[mnum]->lvlttl,
				mapheaderinfo[mnum]->actnum));
		else
			V_DrawString(20, y, 0, va("%s %s %s",
				mapheaderinfo[mnum]->lvlttl,
				(mapheaderinfo[mnum]->zonttl[0] ? mapheaderinfo[mnum]->zonttl : "Zone"),
				mapheaderinfo[mnum]->actnum));

		y += 8;

		if (y >= BASEVIDHEIGHT-8)
			goto bottomarrow;
	}
	if (dotopname && !location)
	{
		V_DrawString(20,  y, highlightflags, "LEVEL NAME");
		V_DrawString(256, y, highlightflags, "MEDALS");
		y += 8;
	}
	else if (location)
		--location;

	// Extra Emblems
	for (i = -2; i < numextraemblems; ++i)
	{
		if (i == -1)
		{
			V_DrawString(20, y, highlightflags, "EXTRA MEDALS");
			if (location)
			{
				y += 8;
				location++;
			}
		}
		if (location)
		{
			--location;
			continue;
		}

		if (i >= 0)
		{
			exemblem = &extraemblems[i];

			if (exemblem->collected)
				V_DrawSmallMappedPatch(295, y, 0, W_CachePatchName(M_GetExtraEmblemPatch(exemblem), PU_CACHE),
				                       R_GetTranslationColormap(TC_DEFAULT, M_GetExtraEmblemColor(exemblem), GTC_MENUCACHE));
			else
				V_DrawSmallScaledPatch(295, y, 0, W_CachePatchName("NEEDIT", PU_CACHE));

			V_DrawString(20, y, 0, va("%s", exemblem->description));
		}

		y += 8;

		if (y >= BASEVIDHEIGHT-8)
			goto bottomarrow;
	}
bottomarrow:
	if (dobottomarrow)
		V_DrawCharacter(10, y-8 + (skullAnimCounter/5),
			'\x1B' | highlightflags, false); // down arrow
}

static void M_DrawLevelStats(void)
{
	char beststr[40];

	tic_t besttime = 0;

	INT32 i;
	INT32 mapsunfinished = 0;

	M_DrawMenuTitle();

	V_DrawString(20, 24, highlightflags, "Total Play Time:");
	V_DrawCenteredString(BASEVIDWIDTH/2, 32, 0, va("%i hours, %i minutes, %i seconds",
	                         G_TicsToHours(totalplaytime),
	                         G_TicsToMinutes(totalplaytime, false),
	                         G_TicsToSeconds(totalplaytime)));
	V_DrawString(20, 42, highlightflags, "Total Matches:");
	V_DrawRightAlignedString(BASEVIDWIDTH-16, 42, 0, va("%i played", matchesplayed));

	for (i = 0; i < NUMMAPS; i++)
	{
		if (!mapheaderinfo[i] || !(mapheaderinfo[i]->menuflags & LF2_RECORDATTACK))
			continue;

		if (!mainrecords[i] || mainrecords[i]->time <= 0)
		{
			mapsunfinished++;
			continue;
		}

		besttime += mainrecords[i]->time;
	}

	V_DrawString(20, 62, highlightflags, "Combined time records:");

	sprintf(beststr, "%i:%02i:%02i.%02i", G_TicsToHours(besttime), G_TicsToMinutes(besttime, false), G_TicsToSeconds(besttime), G_TicsToCentiseconds(besttime));
	V_DrawRightAlignedString(BASEVIDWIDTH-16, 62, (mapsunfinished ? warningflags : 0), beststr);

	if (mapsunfinished)
		V_DrawRightAlignedString(BASEVIDWIDTH-16, 70, warningflags, va("(%d unfinished)", mapsunfinished));
	else
		V_DrawRightAlignedString(BASEVIDWIDTH-16, 70, recommendedflags, "(complete)");

	V_DrawString(32, 70, 0, va("x %d/%d", M_CountEmblems(), numemblems+numextraemblems));
	V_DrawSmallScaledPatch(20, 70, 0, W_CachePatchName("GOTITA", PU_STATIC));

	M_DrawStatsMaps(statsLocation);
}

// Handle statistics.
static void M_HandleLevelStats(INT32 choice)
{
	boolean exitmenu = false; // exit to previous menu

	switch (choice)
	{
		case KEY_DOWNARROW:
			S_StartSound(NULL, sfx_menu1);
			if (statsLocation < statsMax)
				++statsLocation;
			break;

		case KEY_UPARROW:
			S_StartSound(NULL, sfx_menu1);
			if (statsLocation)
				--statsLocation;
			break;

		case KEY_PGDN:
			S_StartSound(NULL, sfx_menu1);
			statsLocation += (statsLocation+13 >= statsMax) ? statsMax-statsLocation : 13;
			break;

		case KEY_PGUP:
			S_StartSound(NULL, sfx_menu1);
			statsLocation -= (statsLocation < 13) ? statsLocation : 13;
			break;

		case KEY_ESCAPE:
			exitmenu = true;
			break;
	}
	if (exitmenu)
	{
		if (currentMenu->prevMenu)
			M_SetupNextMenu(currentMenu->prevMenu);
		else
			M_ClearMenus(true);
	}
}

// ===========
// MODE ATTACK
// ===========

// Drawing function for Time Attack
void M_DrawTimeAttackMenu(void)
{
	INT32 i, x, y, cursory = 0;
	UINT16 dispstatus;

	//S_ChangeMusicInternal("racent", true); // Eww, but needed for when user hits escape during demo playback

	V_DrawPatchFill(W_CachePatchName("SRB2BACK", PU_CACHE));

	M_DrawMenuTitle();
	if (currentMenu == &SP_TimeAttackDef)
		M_DrawLevelSelectOnly(true, false);

	// draw menu (everything else goes on top of it)
	// Sadly we can't just use generic mode menus because we need some extra hacks
	x = currentMenu->x;
	y = currentMenu->y;

	// Character face!
	if (W_CheckNumForName(skins[cv_chooseskin.value-1].facewant) != LUMPERROR)
	{
		UINT8 *colormap = R_GetTranslationColormap(cv_chooseskin.value-1, cv_playercolor.value, GTC_MENUCACHE);
		V_DrawMappedPatch(BASEVIDWIDTH-x - SHORT(facewantprefix[cv_chooseskin.value-1]->width), y, 0, facewantprefix[cv_chooseskin.value-1], colormap);
	}

	for (i = 0; i < currentMenu->numitems; ++i)
	{
		dispstatus = (currentMenu->menuitems[i].status & IT_DISPLAY);
		if (dispstatus != IT_STRING && dispstatus != IT_WHITESTRING)
			continue;

		y = currentMenu->y+currentMenu->menuitems[i].alphaKey;
		if (i == itemOn)
			cursory = y;

		V_DrawString(x, y, (dispstatus == IT_WHITESTRING) ? highlightflags : 0 , currentMenu->menuitems[i].text);

		// Cvar specific handling
		if ((currentMenu->menuitems[i].status & IT_TYPE) == IT_CVAR)
		{
			consvar_t *cv = (consvar_t *)currentMenu->menuitems[i].itemaction;
			if (currentMenu->menuitems[i].status & IT_CV_STRING)
			{
				M_DrawTextBox(x + 32, y - 8, MAXPLAYERNAME, 1);
				V_DrawString(x + 40, y, V_ALLOWLOWERCASE, cv->string);
				if (itemOn == i && skullAnimCounter < 4) // blink cursor
					V_DrawCharacter(x + 40 + V_StringWidth(cv->string, V_ALLOWLOWERCASE), y, '_',false);
			}
			else
			{
				const char *str = ((cv == &cv_chooseskin) ? skins[cv_chooseskin.value-1].realname : cv->string);
				INT32 soffset = 40, strw = V_StringWidth(str, 0);

				// hack to keep the menu from overlapping the level icon
				if (currentMenu != &SP_TimeAttackDef || cv == &cv_nextmap)
					soffset = 0;

				// Should see nothing but strings
				V_DrawString(BASEVIDWIDTH - x - soffset - strw, y, highlightflags, str);

				if (i == itemOn)
				{
					V_DrawCharacter(BASEVIDWIDTH - x - soffset - 10 - strw - (skullAnimCounter/5), y,
						'\x1C' | highlightflags, false); // left arrow
					V_DrawCharacter(BASEVIDWIDTH - x - soffset + 2 + (skullAnimCounter/5), y,
						'\x1D' | highlightflags, false); // right arrow
				}
			}
		}
		else if ((currentMenu->menuitems[i].status & IT_TYPE) == IT_KEYHANDLER && cv_dummystaff.value) // bad hacky assumption: IT_KEYHANDLER is assumed to be staff ghost selector
		{
			INT32 strw = V_StringWidth(dummystaffname, V_ALLOWLOWERCASE);
			V_DrawString(BASEVIDWIDTH - x - strw, y, highlightflags|V_ALLOWLOWERCASE, dummystaffname);
			if (i == itemOn)
			{
				V_DrawCharacter(BASEVIDWIDTH - x - 10 - strw - (skullAnimCounter/5), y,
					'\x1C' | highlightflags, false); // left arrow
				V_DrawCharacter(BASEVIDWIDTH - x + 2 + (skullAnimCounter/5), y,
					'\x1D' | highlightflags, false); // right arrow
			}
		}
	}

	x = currentMenu->x;
	y = currentMenu->y;

	// DRAW THE SKULL CURSOR
	V_DrawScaledPatch(x - 24, cursory, 0, W_CachePatchName("M_CURSOR", PU_CACHE));
	V_DrawString(x, cursory, highlightflags, currentMenu->menuitems[itemOn].text);

	// Level record list
	if (cv_nextmap.value)
	{
		INT32 dupadjust = (vid.width/vid.dupx);
		tic_t lap = 0, time = 0;
		if (mainrecords[cv_nextmap.value-1])
		{
			lap = mainrecords[cv_nextmap.value-1]->lap;
			time = mainrecords[cv_nextmap.value-1]->time;
		}

		V_DrawFill((BASEVIDWIDTH - dupadjust)>>1, 78, dupadjust, 36, 239);

		V_DrawRightAlignedString(149, 80, highlightflags, "BEST LAP:");
		K_drawKartTimestamp(lap, 19, 86, 0, 2);

		V_DrawRightAlignedString(292, 80, highlightflags, "BEST TIME:");
		K_drawKartTimestamp(time, 162, 86, cv_nextmap.value, 1);
	}
	/*{
		char beststr[40];
		emblem_t *em;

		if (!mainrecords[cv_nextmap.value-1] || !mainrecords[cv_nextmap.value-1]->time)
			sprintf(beststr, "(none)");
		else
			sprintf(beststr, "%i:%02i.%02i", G_TicsToMinutes(mainrecords[cv_nextmap.value-1]->time, true),
			                                 G_TicsToSeconds(mainrecords[cv_nextmap.value-1]->time),
			                                 G_TicsToCentiseconds(mainrecords[cv_nextmap.value-1]->time));

		V_DrawString(64, y+48, highlightflags, "BEST TIME:");
		V_DrawRightAlignedString(BASEVIDWIDTH - 64 - 24 - 8, y+48, V_ALLOWLOWERCASE, beststr);

		if (!mainrecords[cv_nextmap.value-1] || !mainrecords[cv_nextmap.value-1]->lap)
			sprintf(beststr, "(none)");
		else
			sprintf(beststr, "%i:%02i.%02i", G_TicsToMinutes(mainrecords[cv_nextmap.value-1]->lap, true),
			                                 G_TicsToSeconds(mainrecords[cv_nextmap.value-1]->lap),
			                                 G_TicsToCentiseconds(mainrecords[cv_nextmap.value-1]->lap));

		V_DrawString(64, y+56, highlightflags, "BEST LAP:");
		V_DrawRightAlignedString(BASEVIDWIDTH - 64 - 24 - 8, y+56, V_ALLOWLOWERCASE, beststr);

		// Draw record emblems.
		em = M_GetLevelEmblems(cv_nextmap.value);
		while (em)
		{
			switch (em->type)
			{
				case ET_TIME: break;
				default:
					goto skipThisOne;
			}

			if (em->collected)
				V_DrawMappedPatch(BASEVIDWIDTH - 64 - 24, y+48, 0, W_CachePatchName(M_GetEmblemPatch(em), PU_CACHE),
				                       R_GetTranslationColormap(TC_DEFAULT, M_GetEmblemColor(em), GTC_MENUCACHE));
			else
				V_DrawScaledPatch(BASEVIDWIDTH - 64 - 24, y+48, 0, W_CachePatchName("NEEDIT", PU_CACHE));

			skipThisOne:
			em = M_GetLevelEmblems(-1);
		}
	}*/

	// ALWAYS DRAW player name, level name, skin and color even when not on this menu!
	if (currentMenu != &SP_TimeAttackDef)
	{
		consvar_t *ncv;

		for (i = 0; i < 4; ++i)
		{
			y = currentMenu->y+SP_TimeAttackMenu[i].alphaKey;
			V_DrawString(x, y, V_TRANSLUCENT, SP_TimeAttackMenu[i].text);
			ncv = (consvar_t *)SP_TimeAttackMenu[i].itemaction;
			if (SP_TimeAttackMenu[i].status & IT_CV_STRING)
			{
				M_DrawTextBox(x + 32, y - 8, MAXPLAYERNAME, 1);
				V_DrawString(x + 40, y, V_TRANSLUCENT|V_ALLOWLOWERCASE, ncv->string);
			}
			else
			{
				const char *str = ((ncv == &cv_chooseskin) ? skins[cv_chooseskin.value-1].realname : ncv->string);
				INT32 soffset = 40, strw = V_StringWidth(str, 0);

				// hack to keep the menu from overlapping the level icon
				if (ncv == &cv_nextmap)
					soffset = 0;

				// Should see nothing but strings
				V_DrawString(BASEVIDWIDTH - x - soffset - strw, y, highlightflags|V_TRANSLUCENT, str);
			}
		}
	}
}

// Going to Time Attack menu...
static void M_TimeAttack(INT32 choice)
{
	(void)choice;

	memset(skins_cons_t, 0, sizeof (skins_cons_t));

	levellistmode = LLM_RECORDATTACK; // Don't be dependent on cv_newgametype

	if (M_CountLevelsToShowInList() == 0)
	{
		M_StartMessage(M_GetText("No record-attackable levels found.\n"),NULL,MM_NOTHING);
		return;
	}

	M_PatchSkinNameTable();

	M_PrepareLevelSelect();
	M_SetupNextMenu(&SP_TimeAttackDef);

	G_SetGamestate(GS_TIMEATTACK);

	if (cv_nextmap.value)
		Nextmap_OnChange();
	else
		CV_AddValue(&cv_nextmap, 1);

	itemOn = tastart; // "Start" is selected.

	S_ChangeMusicInternal("racent", true);
}

static boolean M_QuitTimeAttackMenu(void)
{
	// you know what? always putting these in the buffer won't hurt anything.
	COM_BufAddText(va("skin \"%s\"\n", cv_chooseskin.string));
	return true;
}

// Drawing function for Nights Attack
/*void M_DrawNightsAttackMenu(void)
{
	patch_t *PictureOfLevel;
	lumpnum_t lumpnum;
	char beststr[40];

	S_ChangeMusicInternal("racent", true); // Eww, but needed for when user hits escape during demo playback

	V_DrawPatchFill(W_CachePatchName("SRB2BACK", PU_CACHE));

	// draw menu (everything else goes on top of it)
	M_DrawGenericMenu();

	//  A 160x100 image of the level as entry MAPxxP
	lumpnum = W_CheckNumForName(va("%sP", G_BuildMapName(cv_nextmap.value)));

	if (lumpnum != LUMPERROR)
		PictureOfLevel = W_CachePatchNum(lumpnum, PU_CACHE);
	else
		PictureOfLevel = W_CachePatchName("BLANKLVL", PU_CACHE);

	V_DrawSmallScaledPatch(90, 28, 0, PictureOfLevel);

	// Level record list
	if (cv_nextmap.value)
	{
		emblem_t *em;
		INT32 yHeight;

		UINT8 bestoverall	= G_GetBestNightsGrade(cv_nextmap.value, 0);
		UINT8 bestgrade		= G_GetBestNightsGrade(cv_nextmap.value, cv_dummymares.value);
		UINT32 bestscore	= G_GetBestNightsScore(cv_nextmap.value, cv_dummymares.value);
		tic_t besttime		= G_GetBestNightsTime(cv_nextmap.value, cv_dummymares.value);

		if (P_HasGrades(cv_nextmap.value, 0))
			V_DrawScaledPatch(200, 28 + 8, 0, ngradeletters[bestoverall]);

		if (currentMenu == &SP_NightsAttackDef)
		{
			if (P_HasGrades(cv_nextmap.value, cv_dummymares.value))
			{
				V_DrawString(160-88, 112, highlightflags, "BEST GRADE:");
				V_DrawSmallScaledPatch(160 + 86 - (ngradeletters[bestgrade]->width/2),
					112 + 8 - (ngradeletters[bestgrade]->height/2),
					0, ngradeletters[bestgrade]);
			}

			if (!bestscore)
				sprintf(beststr, "(none)");
			else
				sprintf(beststr, "%u", bestscore);

			V_DrawString(160 - 88, 122, highlightflags, "BEST SCORE:");
			V_DrawRightAlignedString(160 + 88, 122, V_ALLOWLOWERCASE, beststr);

			if (besttime == UINT32_MAX)
				sprintf(beststr, "(none)");
			else
				sprintf(beststr, "%i:%02i.%02i", G_TicsToMinutes(besttime, true),
																				 G_TicsToSeconds(besttime),
																				 G_TicsToCentiseconds(besttime));

			V_DrawString(160-88, 132, highlightflags, "BEST TIME:");
			V_DrawRightAlignedString(160+88, 132, V_ALLOWLOWERCASE, beststr);

			if (cv_dummymares.value == 0) {
				// Draw record emblems.
				em = M_GetLevelEmblems(cv_nextmap.value);
				while (em)
				{
					switch (em->type)
					{
						case ET_NGRADE: yHeight = 112; break;
						case ET_NTIME:  yHeight = 132; break;
						default:
							goto skipThisOne;
					}

					if (em->collected)
						V_DrawSmallMappedPatch(160+88, yHeight, 0, W_CachePatchName(M_GetEmblemPatch(em), PU_CACHE),
																	 R_GetTranslationColormap(TC_DEFAULT, M_GetEmblemColor(em), GTC_MENUCACHE));
					else
						V_DrawSmallScaledPatch(160+88, yHeight, 0, W_CachePatchName("NEEDIT", PU_CACHE));

					skipThisOne:
					em = M_GetLevelEmblems(-1);
				}
			}
		}
		// ALWAYS DRAW level name even when not on this menu!
		else
		{
			consvar_t *ncv;
			INT32 x = SP_NightsAttackDef.x;
			INT32 y = SP_NightsAttackDef.y;

			ncv = (consvar_t *)SP_NightsAttackMenu[0].itemaction;
			V_DrawString(x, y + SP_NightsAttackMenu[0].alphaKey, V_TRANSLUCENT, SP_NightsAttackMenu[0].text);
			V_DrawString(BASEVIDWIDTH - x - V_StringWidth(ncv->string, 0),
									 y + SP_NightsAttackMenu[0].alphaKey, highlightflags|V_TRANSLUCENT, ncv->string);
		}
	}
}*/

// Going to Nights Attack menu...
/*static void M_NightsAttack(INT32 choice)
{
	(void)choice;

	memset(skins_cons_t, 0, sizeof (skins_cons_t));

	levellistmode = LLM_NIGHTSATTACK; // Don't be dependent on cv_newgametype

	if (M_CountLevelsToShowInList() == 0)
	{
		M_StartMessage(M_GetText("No NiGHTS-attackable levels found.\n"),NULL,MM_NOTHING);
		return;
	}

	// This is really just to make sure Sonic is the played character, just in case
	M_PatchSkinNameTable();

	M_PrepareLevelSelect();
	M_SetupNextMenu(&SP_NightsAttackDef);
	Nextmap_OnChange();

	itemOn = nastart; // "Start" is selected.

	G_SetGamestate(GS_TIMEATTACK);
	S_ChangeMusicInternal("racent", true);
}*/

// Player has selected the "START" from the nights attack screen
/*static void M_ChooseNightsAttack(INT32 choice)
{
	char nameofdemo[256];
	(void)choice;
	emeralds = 0;
	M_ClearMenus(true);
	modeattacking = ATTACKING_NIGHTS;

	I_mkdir(va("%s"PATHSEP"replay", srb2home), 0755);
	I_mkdir(va("%s"PATHSEP"replay"PATHSEP"%s", srb2home, timeattackfolder), 0755);

	snprintf(nameofdemo, sizeof nameofdemo, "replay"PATHSEP"%s"PATHSEP"%s-last", timeattackfolder, G_BuildMapName(cv_nextmap.value));

	if (!cv_autorecord.value)
		remove(va("%s"PATHSEP"%s.lmp", srb2home, nameofdemo));
	else
		G_RecordDemo(nameofdemo);

	G_DeferedInitNew(false, G_BuildMapName(cv_nextmap.value), 0, 0, false);
}*/

// Player has selected the "START" from the time attack screen
static void M_ChooseTimeAttack(INT32 choice)
{
	char *gpath;
	const size_t glen = strlen("replay")+1+strlen(timeattackfolder)+1+strlen("MAPXX")+1;
	char nameofdemo[256];
	(void)choice;
	emeralds = 0;
	M_ClearMenus(true);
	modeattacking = ATTACKING_RECORD;

	I_mkdir(va("%s"PATHSEP"replay", srb2home), 0755);
	I_mkdir(va("%s"PATHSEP"replay"PATHSEP"%s", srb2home, timeattackfolder), 0755);

	if ((gpath = malloc(glen)) == NULL)
		I_Error("Out of memory for replay filepath\n");

	sprintf(gpath,"replay"PATHSEP"%s"PATHSEP"%s", timeattackfolder, G_BuildMapName(cv_nextmap.value));
	snprintf(nameofdemo, sizeof nameofdemo, "%s-%s-last", gpath, cv_chooseskin.string);

	if (!cv_autorecord.value)
		remove(va("%s"PATHSEP"%s.lmp", srb2home, nameofdemo));
	else
		G_RecordDemo(nameofdemo);

	G_DeferedInitNew(false, G_BuildMapName(cv_nextmap.value), (UINT8)(cv_chooseskin.value-1), 0, false);
}

static void M_HandleStaffReplay(INT32 choice)
{
	boolean exitmenu = false; // exit to previous menu
	lumpnum_t l = W_CheckNumForName(va("%sS%02u",G_BuildMapName(cv_nextmap.value),cv_dummystaff.value));

	switch (choice)
	{
		case KEY_DOWNARROW:
			M_NextOpt();
			S_StartSound(NULL, sfx_menu1);
			break;
		case KEY_UPARROW:
			M_PrevOpt();
			S_StartSound(NULL, sfx_menu1);
			break;
		case KEY_BACKSPACE:
		case KEY_ESCAPE:
			exitmenu = true;
			break;
		case KEY_RIGHTARROW:
			CV_AddValue(&cv_dummystaff, 1);
			S_StartSound(NULL, sfx_menu1);
			break;
		case KEY_LEFTARROW:
			CV_AddValue(&cv_dummystaff, -1);
			S_StartSound(NULL, sfx_menu1);
			break;
		case KEY_ENTER:
			if (l == LUMPERROR)
				break;
			M_ClearMenus(true);
			modeattacking = ATTACKING_RECORD;
			demo.loadfiles = false; demo.ignorefiles = true; // Just assume that record attack replays have the files needed
			G_DoPlayDemo(va("%sS%02u",G_BuildMapName(cv_nextmap.value),cv_dummystaff.value));
			break;
		default:
			break;
	}
	if (exitmenu)
	{
		if (currentMenu->prevMenu)
			M_SetupNextMenu(currentMenu->prevMenu);
		else
			M_ClearMenus(true);
	}
}

// Player has selected the "REPLAY" from the time attack screen
static void M_ReplayTimeAttack(INT32 choice)
{
	const char *which;
	M_ClearMenus(true);
	modeattacking = ATTACKING_RECORD; // set modeattacking before G_DoPlayDemo so the map loader knows
	demo.loadfiles = false; demo.ignorefiles = true; // Just assume that record attack replays have the files needed

	if (currentMenu == &SP_ReplayDef)
	{
		switch(choice) {
		default:
		case 0: // best time
			which = "time-best";
			break;
		case 1: // best lap
			which = "lap-best";
			break;
		case 2: // last
			which = "last";
			break;
		case 3: // guest
			// srb2/replay/main/map01-guest.lmp
			G_DoPlayDemo(va("%s"PATHSEP"replay"PATHSEP"%s"PATHSEP"%s-guest.lmp", srb2home, timeattackfolder, G_BuildMapName(cv_nextmap.value)));
			return;
		}
		// srb2/replay/main/map01-sonic-time-best.lmp
		G_DoPlayDemo(va("%s"PATHSEP"replay"PATHSEP"%s"PATHSEP"%s-%s-%s.lmp", srb2home, timeattackfolder, G_BuildMapName(cv_nextmap.value), cv_chooseskin.string, which));
	}
	/*else if (currentMenu == &SP_NightsReplayDef)
	{
		switch(choice) {
		default:
		case 0: // best score
			which = "score-best";
			break;
		case 1: // best time
			which = "time-best";
			break;
		case 2: // last
			which = "last";
			break;
		case 3: // staff
			return; // M_HandleStaffReplay
		case 4: // guest
			which = "guest";
			break;
		}
		// srb2/replay/main/map01-score-best.lmp
		G_DoPlayDemo(va("%s"PATHSEP"replay"PATHSEP"%s"PATHSEP"%s-%s.lmp", srb2home, timeattackfolder, G_BuildMapName(cv_nextmap.value), which));
	}*/
}

static void M_EraseGuest(INT32 choice)
{
	const char *rguest = va("%s"PATHSEP"replay"PATHSEP"%s"PATHSEP"%s-guest.lmp", srb2home, timeattackfolder, G_BuildMapName(cv_nextmap.value));
	(void)choice;
	if (FIL_FileExists(rguest))
		remove(rguest);
	/*if (currentMenu == &SP_NightsGuestReplayDef)
		M_SetupNextMenu(&SP_NightsAttackDef);
	else*/
		M_SetupNextMenu(&SP_TimeAttackDef);
	CV_AddValue(&cv_nextmap, -1);
	CV_AddValue(&cv_nextmap, 1);
	M_StartMessage(M_GetText("Guest replay data erased.\n"),NULL,MM_NOTHING);
}

static void M_OverwriteGuest(const char *which)
{
	char *rguest = Z_StrDup(va("%s"PATHSEP"replay"PATHSEP"%s"PATHSEP"%s-guest.lmp", srb2home, timeattackfolder, G_BuildMapName(cv_nextmap.value)));
	UINT8 *buf;
	size_t len;
	len = FIL_ReadFile(va("%s"PATHSEP"replay"PATHSEP"%s"PATHSEP"%s-%s-%s.lmp", srb2home, timeattackfolder, G_BuildMapName(cv_nextmap.value), cv_chooseskin.string, which), &buf);
	if (!len) {
		return;
	}
	if (FIL_FileExists(rguest)) {
		M_StopMessage(0);
		remove(rguest);
	}
	FIL_WriteFile(rguest, buf, len);
	Z_Free(rguest);
	/*if (currentMenu == &SP_NightsGuestReplayDef)
		M_SetupNextMenu(&SP_NightsAttackDef);
	else*/
		M_SetupNextMenu(&SP_TimeAttackDef);
	CV_AddValue(&cv_nextmap, -1);
	CV_AddValue(&cv_nextmap, 1);
	M_StartMessage(M_GetText("Guest replay data saved.\n"),NULL,MM_NOTHING);
}

static void M_OverwriteGuest_Time(INT32 choice)
{
	(void)choice;
	M_OverwriteGuest("time-best");
}

static void M_OverwriteGuest_Lap(INT32 choice)
{
	(void)choice;
	M_OverwriteGuest("lap-best");
}

/* SRB2Kart
static void M_OverwriteGuest_Score(INT32 choice)
{
	(void)choice;
	M_OverwriteGuest("score-best");
}

static void M_OverwriteGuest_Rings(INT32 choice)
{
	(void)choice;
	M_OverwriteGuest("rings-best");
}*/

static void M_OverwriteGuest_Last(INT32 choice)
{
	(void)choice;
	M_OverwriteGuest("last");
}

static void M_SetGuestReplay(INT32 choice)
{
	void (*which)(INT32);
	switch(choice)
	{
	case 0: // best time
		which = M_OverwriteGuest_Time;
		break;
	case 1: // best lap
		which = M_OverwriteGuest_Lap;
		break;
	case 2: // last
		which = M_OverwriteGuest_Last;
		break;
	case 3: // guest
	default:
		M_StartMessage(M_GetText("Are you sure you want to\ndelete the guest replay data?\n\n(Press 'Y' to confirm)\n"),M_EraseGuest,MM_YESNO);
		return;
	}
	if (FIL_FileExists(va("%s"PATHSEP"replay"PATHSEP"%s"PATHSEP"%s-guest.lmp", srb2home, timeattackfolder, G_BuildMapName(cv_nextmap.value))))
		M_StartMessage(M_GetText("Are you sure you want to\noverwrite the guest replay data?\n\n(Press 'Y' to confirm)\n"),which,MM_YESNO);
	else
		which(0);
}

void M_ModeAttackRetry(INT32 choice)
{
	(void)choice;
	G_CheckDemoStatus(); // Cancel recording
	if (modeattacking == ATTACKING_RECORD)
		M_ChooseTimeAttack(0);
	/*else if (modeattacking == ATTACKING_NIGHTS)
		M_ChooseNightsAttack(0);*/
}

static void M_ModeAttackEndGame(INT32 choice)
{
	(void)choice;
	G_CheckDemoStatus(); // Cancel recording

	if (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION || gamestate == GS_VOTING)
		Command_ExitGame_f();

	M_StartControlPanel();
	switch(modeattacking)
	{
	default:
	case ATTACKING_RECORD:
		currentMenu = &SP_TimeAttackDef;
		break;
	/*case ATTACKING_NIGHTS:
		currentMenu = &SP_NightsAttackDef;
		break;*/
	}
	itemOn = currentMenu->lastOn;
	G_SetGamestate(GS_TIMEATTACK);
	modeattacking = ATTACKING_NONE;
	S_ChangeMusicInternal("racent", true);
	// Update replay availability.
	CV_AddValue(&cv_nextmap, 1);
	CV_AddValue(&cv_nextmap, -1);
}

// ========
// END GAME
// ========

static void M_ExitGameResponse(INT32 ch)
{
	if (ch != 'y' && ch != KEY_ENTER)
		return;

	//Command_ExitGame_f();
	G_SetExitGameFlag();
	M_ClearMenus(true);
}

static void M_EndGame(INT32 choice)
{
	(void)choice;
	if (demo.playback)
		return;

	if (!Playing())
		return;

	M_StartMessage(M_GetText("Are you sure you want to end the game?\n\n(Press 'Y' to confirm)\n"), M_ExitGameResponse, MM_YESNO);
}

//===========================================================================
// Connect Menu
//===========================================================================

void
M_SetWaitingMode (int mode)
{
#ifdef HAVE_THREADS
	I_lock_mutex(&m_menu_mutex);
#endif
	{
		m_waiting_mode = mode;
	}
#ifdef HAVE_THREADS
	I_unlock_mutex(m_menu_mutex);
#endif
}

int
M_GetWaitingMode (void)
{
	int mode;

#ifdef HAVE_THREADS
	I_lock_mutex(&m_menu_mutex);
#endif
	{
		mode = m_waiting_mode;
	}
#ifdef HAVE_THREADS
	I_unlock_mutex(m_menu_mutex);
#endif

	return mode;
}

#ifdef MASTERSERVER
#ifdef HAVE_THREADS
static void
Spawn_masterserver_thread (const char *name, void (*thread)(int*))
{
	int *id = malloc(sizeof *id);

	I_lock_mutex(&ms_QueryId_mutex);
	{
		*id = ms_QueryId;
	}
	I_unlock_mutex(ms_QueryId_mutex);

	I_spawn_thread(name, (I_thread_fn)thread, id);
}

static int
Same_instance (int id)
{
	int okay;

	I_lock_mutex(&ms_QueryId_mutex);
	{
		okay = ( id == ms_QueryId );
	}
	I_unlock_mutex(ms_QueryId_mutex);

	return okay;
}
#endif/*HAVE_THREADS*/

static void
Fetch_servers_thread (int *id)
{
	msg_server_t * server_list;

	(void)id;

	M_SetWaitingMode(M_WAITING_SERVERS);

#ifdef HAVE_THREADS
	server_list = GetShortServersList(*id);
#else
	server_list = GetShortServersList(0);
#endif

	if (server_list)
	{
#ifdef HAVE_THREADS
		if (Same_instance(*id))
#endif
		{
			M_SetWaitingMode(M_NOT_WAITING);

#ifdef HAVE_THREADS
			I_lock_mutex(&ms_ServerList_mutex);
			{
				ms_ServerList = server_list;
			}
			I_unlock_mutex(ms_ServerList_mutex);
#else
			CL_QueryServerList(server_list);
			free(server_list);
#endif
		}
#ifdef HAVE_THREADS
		else
		{
			free(server_list);
		}
#endif
	}

#ifdef HAVE_THREADS
	free(id);
#endif
}
#endif/*MASTERSERVER*/

#define SERVERHEADERHEIGHT 36
#define SERVERLINEHEIGHT 12

#define S_LINEY(n) currentMenu->y + SERVERHEADERHEIGHT + (n * SERVERLINEHEIGHT)

#ifndef NONET
static UINT32 localservercount;

static void M_HandleServerPage(INT32 choice)
{
	boolean exitmenu = false; // exit to previous menu

	switch (choice)
	{
		case KEY_DOWNARROW:
			M_NextOpt();
			S_StartSound(NULL, sfx_menu1);
			break;
		case KEY_UPARROW:
			M_PrevOpt();
			S_StartSound(NULL, sfx_menu1);
			break;
		case KEY_BACKSPACE:
		case KEY_ESCAPE:
			exitmenu = true;
			break;

		case KEY_ENTER:
		case KEY_RIGHTARROW:
			S_StartSound(NULL, sfx_menu1);
			if ((serverlistpage + 1) * SERVERS_PER_PAGE < serverlistcount)
				serverlistpage++;
			break;
		case KEY_LEFTARROW:
			S_StartSound(NULL, sfx_menu1);
			if (serverlistpage > 0)
				serverlistpage--;
			break;

		default:
			break;
	}
	if (exitmenu)
	{
		if (currentMenu->prevMenu)
			M_SetupNextMenu(currentMenu->prevMenu);
		else
			M_ClearMenus(true);
	}
}

static void M_Connect(INT32 choice)
{
	// do not call menuexitfunc
	M_ClearMenus(false);

	COM_BufAddText(va("connect node %d\n", serverlist[choice-FIRSTSERVERLINE + serverlistpage * SERVERS_PER_PAGE].node));
}

static void M_Refresh(INT32 choice)
{
	(void)choice;

	// Display a little "please wait" message.
	M_DrawTextBox(52, BASEVIDHEIGHT/2-10, 25, 3);
	V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT/2, 0, "Searching for servers...");
	V_DrawCenteredString(BASEVIDWIDTH/2, (BASEVIDHEIGHT/2)+12, 0, "Please wait.");
	I_OsPolling();
	I_UpdateNoBlit();
	if (rendermode == render_soft)
		I_FinishUpdate(); // page flip or blit buffer

	// first page of servers
	serverlistpage = 0;

#ifdef MASTERSERVER
#ifdef HAVE_THREADS
	Spawn_masterserver_thread("fetch-servers", Fetch_servers_thread);
#else/*HAVE_THREADS*/
	Fetch_servers_thread(NULL);
#endif/*HAVE_THREADS*/
#else/*MASTERSERVER*/
	CL_UpdateServerList();
#endif/*MASTERSERVER*/
}

static void M_DrawConnectMenu(void)
{
	UINT16 i;
	const char *gt = "Unknown";
	const char *spd = "";
	INT32 numPages = (serverlistcount+(SERVERS_PER_PAGE-1))/SERVERS_PER_PAGE;
	int waiting;

	for (i = FIRSTSERVERLINE; i < min(localservercount, SERVERS_PER_PAGE)+FIRSTSERVERLINE; i++)
		MP_ConnectMenu[i].status = IT_STRING | IT_SPACE;

	if (!numPages)
		numPages = 1;

	// Page num
	V_DrawRightAlignedString(BASEVIDWIDTH - currentMenu->x, currentMenu->y + MP_ConnectMenu[mp_connect_page].alphaKey,
	                         highlightflags, va("%u of %d", serverlistpage+1, numPages));

	// Horizontal line!
	V_DrawFill(1, currentMenu->y+32, 318, 1, 0);

	if (serverlistcount <= 0)
		V_DrawString(currentMenu->x,currentMenu->y+SERVERHEADERHEIGHT, 0, "No servers found");
	else
	for (i = 0; i < min(serverlistcount - serverlistpage * SERVERS_PER_PAGE, SERVERS_PER_PAGE); i++)
	{
		INT32 slindex = i + serverlistpage * SERVERS_PER_PAGE;
		UINT32 globalflags = ((serverlist[slindex].info.numberofplayer >= serverlist[slindex].info.maxplayer) ? V_TRANSLUCENT : 0)
			|((itemOn == FIRSTSERVERLINE+i) ? highlightflags : 0)|V_ALLOWLOWERCASE;

		V_DrawString(currentMenu->x, S_LINEY(i), globalflags, serverlist[slindex].info.servername);

		// Don't use color flags intentionally, the global yellow color will auto override the text color code
		if (serverlist[slindex].info.modifiedgame)
			V_DrawSmallString(currentMenu->x+202, S_LINEY(i)+8, globalflags, "\x85" "Mod");
		if (serverlist[slindex].info.cheatsenabled)
			V_DrawSmallString(currentMenu->x+222, S_LINEY(i)+8, globalflags, "\x83" "Cheats");

		V_DrawSmallString(currentMenu->x, S_LINEY(i)+8, globalflags,
		                     va("Ping: %u", (UINT32)LONG(serverlist[slindex].info.time)));

		gt = "Unknown";
		if (serverlist[slindex].info.gametype < NUMGAMETYPES)
			gt = Gametype_Names[serverlist[slindex].info.gametype];

		V_DrawSmallString(currentMenu->x+46,S_LINEY(i)+8, globalflags,
		                         va("Players: %02d/%02d", serverlist[slindex].info.numberofplayer, serverlist[slindex].info.maxplayer));

		V_DrawSmallString(currentMenu->x+112, S_LINEY(i)+8, globalflags, gt);

		// display game speed for race gametypes
		if (serverlist[slindex].info.gametype == GT_RACE)
		{
			spd = kartspeed_cons_t[serverlist[slindex].info.kartvars & SV_SPEEDMASK].strvalue;

			V_DrawSmallString(currentMenu->x+132, S_LINEY(i)+8, globalflags, va("(%s Speed)", spd));
		}

		MP_ConnectMenu[i+FIRSTSERVERLINE].status = IT_STRING | IT_CALL;
	}

	localservercount = serverlistcount;

	M_DrawGenericMenu();

	waiting = M_GetWaitingMode();

	if (waiting)
	{
		const char *message;

		if (waiting == M_WAITING_VERSION)
			message = "Checking for updates...";
		else
			message = "Searching for servers...";

		// Display a little "please wait" message.
		M_DrawTextBox(52, BASEVIDHEIGHT/2-10, 25, 3);
		V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT/2, 0, message);
		V_DrawCenteredString(BASEVIDWIDTH/2, (BASEVIDHEIGHT/2)+12, 0, "Please wait.");
	}
}

static boolean M_CancelConnect(void)
{
	D_CloseConnection();
	return true;
}

// Ascending order, not descending.
// The casts are safe as long as the caller doesn't do anything stupid.
#define SERVER_LIST_ENTRY_COMPARATOR(key) \
static int ServerListEntryComparator_##key(const void *entry1, const void *entry2) \
{ \
	const serverelem_t *sa = (const serverelem_t*)entry1, *sb = (const serverelem_t*)entry2; \
	if (sa->info.key != sb->info.key) \
		return sa->info.key - sb->info.key; \
	return strcmp(sa->info.servername, sb->info.servername); \
}

// This does descending instead of ascending.
#define SERVER_LIST_ENTRY_COMPARATOR_REVERSE(key) \
static int ServerListEntryComparator_##key##_reverse(const void *entry1, const void *entry2) \
{ \
	const serverelem_t *sa = (const serverelem_t*)entry1, *sb = (const serverelem_t*)entry2; \
	if (sb->info.key != sa->info.key) \
		return sb->info.key - sa->info.key; \
	return strcmp(sb->info.servername, sa->info.servername); \
}

SERVER_LIST_ENTRY_COMPARATOR(time)
SERVER_LIST_ENTRY_COMPARATOR(numberofplayer)
SERVER_LIST_ENTRY_COMPARATOR_REVERSE(numberofplayer)
SERVER_LIST_ENTRY_COMPARATOR_REVERSE(maxplayer)
SERVER_LIST_ENTRY_COMPARATOR(gametype)

// Special one for modified state.
static int ServerListEntryComparator_modified(const void *entry1, const void *entry2)
{
	const serverelem_t *sa = (const serverelem_t*)entry1, *sb = (const serverelem_t*)entry2;

	// Modified acts as 2 points, cheats act as one point.
	int modstate_a = (sa->info.cheatsenabled ? 1 : 0) | (sa->info.modifiedgame ? 2 : 0);
	int modstate_b = (sb->info.cheatsenabled ? 1 : 0) | (sb->info.modifiedgame ? 2 : 0);

	if (modstate_a != modstate_b)
		return modstate_a - modstate_b;

	// Default to strcmp.
	return strcmp(sa->info.servername, sb->info.servername);
}
#endif

void M_SortServerList(void)
{
#ifndef NONET
	switch(cv_serversort.value)
	{
	case 0:		// Ping.
		qsort(serverlist, serverlistcount, sizeof(serverelem_t), ServerListEntryComparator_time);
		break;
	case 1:		// Modified state.
		qsort(serverlist, serverlistcount, sizeof(serverelem_t), ServerListEntryComparator_modified);
		break;
	case 2:		// Most players.
		qsort(serverlist, serverlistcount, sizeof(serverelem_t), ServerListEntryComparator_numberofplayer_reverse);
		break;
	case 3:		// Least players.
		qsort(serverlist, serverlistcount, sizeof(serverelem_t), ServerListEntryComparator_numberofplayer);
		break;
	case 4:		// Max players.
		qsort(serverlist, serverlistcount, sizeof(serverelem_t), ServerListEntryComparator_maxplayer_reverse);
		break;
	case 5:		// Gametype.
		qsort(serverlist, serverlistcount, sizeof(serverelem_t), ServerListEntryComparator_gametype);
		break;
	}
#endif
}

#ifndef NONET
#ifdef UPDATE_ALERT
static void M_CheckMODVersion(int id)
{
	char updatestring[500];
	const char *updatecheck = GetMODVersion(id);
	if(updatecheck)
	{
		sprintf(updatestring, UPDATE_ALERT_STRING, VERSIONSTRING, updatecheck);
#ifdef HAVE_THREADS
		I_lock_mutex(&m_menu_mutex);
#endif
		M_StartMessage(updatestring, NULL, MM_NOTHING);
#ifdef HAVE_THREADS
		I_unlock_mutex(m_menu_mutex);
#endif
	}
}
#endif/*UPDATE_ALERT*/

#if defined (UPDATE_ALERT) && defined (HAVE_THREADS)
static void
Check_new_version_thread (int *id)
{
	M_SetWaitingMode(M_WAITING_VERSION);

	M_CheckMODVersion(*id);

	if (Same_instance(*id))
	{
		Fetch_servers_thread(id);
	}
	else
	{
		free(id);
	}
}
#endif/*defined (UPDATE_ALERT) && defined (HAVE_THREADS)*/

static void M_ConnectMenu(INT32 choice)
{
	(void)choice;
	// modified game check: no longer handled
	// we don't request a restart unless the filelist differs

	// first page of servers
	serverlistpage = 0;
	M_SetupNextMenu(&MP_ConnectDef);
	itemOn = 0;

#if defined (MASTERSERVER) && defined (HAVE_THREADS)
	I_lock_mutex(&ms_QueryId_mutex);
	{
		ms_QueryId++;
	}
	I_unlock_mutex(ms_QueryId_mutex);

	I_lock_mutex(&ms_ServerList_mutex);
	{
		if (ms_ServerList)
		{
			free(ms_ServerList);
			ms_ServerList = NULL;
		}
	}
	I_unlock_mutex(ms_ServerList_mutex);

#ifdef UPDATE_ALERT
	Spawn_masterserver_thread("check-new-version", Check_new_version_thread);
#else/*UPDATE_ALERT*/
	Spawn_masterserver_thread("fetch-servers", Fetch_servers_thread);
#endif/*UPDATE_ALERT*/
#else/*defined (MASTERSERVER) && defined (HAVE_THREADS)*/
#ifdef UPDATE_ALERT
	M_CheckMODVersion(0);
#endif/*UPDATE_ALERT*/
	M_Refresh(0);
#endif/*defined (MASTERSERVER) && defined (HAVE_THREADS)*/
}

static void M_ConnectMenuModChecks(INT32 choice)
{
	(void)choice;
	// okay never mind we want to COMMUNICATE to the player pre-emptively instead of letting them try and then get confused when it doesn't work

	if (modifiedgame)
	{
		M_StartMessage(M_GetText("You have addons loaded.\nYou won't be able to join netgames!\n\nTo play online, restart the game\nand don't load any addons.\nSRB2Kart will automatically add\neverything you need when you join.\n\n(Press a key)\n"),M_ConnectMenu,MM_EVENTHANDLER);
		return;
	}

	M_ConnectMenu(-1);
}
#endif //NONET

//===========================================================================
// Start Server Menu
//===========================================================================

//
// FindFirstMap
//
// Finds the first map of a particular gametype (or returns the current map)
// Defaults to 1 if nothing found.
//
static INT32 M_FindFirstMap(INT32 gtype)
{
	INT32 i;

	if (mapheaderinfo[gamemap] && (mapheaderinfo[gamemap]->typeoflevel & gtype))
		return gamemap;

	for (i = 0; i < NUMMAPS; i++)
	{
		if (!mapheaderinfo[i])
			continue;
		if (!(mapheaderinfo[i]->typeoflevel & gtype))
			continue;
		return i + 1;
	}

	return 1;
}

static void M_StartServer(INT32 choice)
{
	UINT8 ssplayers = cv_splitplayers.value-1;

	(void)choice;

	if (currentMenu == &MP_OfflineServerDef)
		netgame = false;
	else
		netgame = true;

	multiplayer = true;

	strncpy(connectedservername, cv_servername.string, MAXSERVERNAME);

	// Still need to reset devmode
	cv_debug = 0;

	if (demo.playback)
		G_StopDemo();
	if (metalrecording)
		G_StopMetalDemo();

	if (!cv_nextmap.value)
		CV_SetValue(&cv_nextmap, G_RandMap(G_TOLFlag(cv_newgametype.value), -1, false, 0, false, NULL)+1);

	if (cv_maxplayers.value < ssplayers+1)
		CV_SetValue(&cv_maxplayers, ssplayers+1);

	if (splitscreen != ssplayers)
	{
		splitscreen = ssplayers;
		SplitScreen_OnChange();
	}

	if (currentMenu == &MP_OfflineServerDef) // offline server
	{
		paused = false;
		SV_StartSinglePlayerServer();
		multiplayer = true; // yeah, SV_StartSinglePlayerServer clobbers this...
		D_MapChange(cv_nextmap.value, cv_newgametype.value, (boolean)cv_kartencore.value, 1, 1, false, false);
	}
	else
	{
		D_MapChange(cv_nextmap.value, cv_newgametype.value, (boolean)cv_kartencore.value, 1, 1, false, false);
		COM_BufAddText("dummyconsvar 1\n");
	}

	M_ClearMenus(true);
}

static void M_DrawLevelSelectOnly(boolean leftfade, boolean rightfade)
{
	lumpnum_t lumpnum;
	patch_t *PictureOfLevel;
	INT32 x, y, w, i, oldval, trans, dupadjust = ((vid.width/vid.dupx) - BASEVIDWIDTH)>>1;

	//  A 160x100 image of the level as entry MAPxxP
	if (cv_nextmap.value)
	{
		lumpnum = W_CheckNumForName(va("%sP", G_BuildMapName(cv_nextmap.value)));
		if (lumpnum != LUMPERROR)
			PictureOfLevel = W_CachePatchNum(lumpnum, PU_CACHE);
		else
			PictureOfLevel = W_CachePatchName("BLANKLVL", PU_CACHE);
	}
	else
		PictureOfLevel = W_CachePatchName("RANDOMLV", PU_CACHE);

	w = SHORT(PictureOfLevel->width)/2;
	i = SHORT(PictureOfLevel->height)/2;
	x = BASEVIDWIDTH/2 - w/2;
	y = currentMenu->y + 130 + 8 - i;

	if (currentMenu->menuitems[itemOn].itemaction == &cv_nextmap && skullAnimCounter < 4)
		trans = 120;
	else
		trans = G_GetGametypeColor(cv_newgametype.value);

	V_DrawFill(x-1, y-1, w+2, i+2, trans); // variable reuse...

	if (!cv_kartencore.value || gamestate == GS_TIMEATTACK || cv_newgametype.value != GT_RACE)
		V_DrawSmallScaledPatch(x, y, 0, PictureOfLevel);
	else
	{
		/*UINT8 *mappingforencore = NULL;
		if ((lumpnum = W_CheckNumForName(va("%sE", mapname))) != LUMPERROR)
			mappingforencore = W_CachePatchNum(lumpnum, PU_CACHE);*/

		V_DrawFixedPatch((x+w)<<FRACBITS, (y)<<FRACBITS, FRACUNIT/2, V_FLIP, PictureOfLevel, 0);

		{
			static angle_t rubyfloattime = 0;
			const fixed_t rubyheight = FINESINE(rubyfloattime>>ANGLETOFINESHIFT);
			V_DrawFixedPatch((x+w/2)<<FRACBITS, ((y+i/2)<<FRACBITS) - (rubyheight<<1), FRACUNIT, 0, W_CachePatchName("RUBYICON", PU_CACHE), NULL);
			rubyfloattime += (ANGLE_MAX/NEWTICRATE);
		}
	}
	/*V_DrawDiag(x, y, 12, 31);
	V_DrawDiag(x, y, 10, G_GetGametypeColor(cv_newgametype.value));*/

	y += i/4;
	i = cv_nextmap.value - 1;
	trans = (leftfade ? V_TRANSLUCENT : 0);

#define horizspac 2
	do
	{
		oldval = i;
		do
		{
			i--;
			if (i == -2)
				i = NUMMAPS-1;

			if (i == oldval)
				return;

			if(!mapheaderinfo[i])
				continue; // Don't allocate the header.  That just makes memory usage skyrocket.

		} while (!M_CanShowLevelInList(i, cv_newgametype.value));

		//  A 160x100 image of the level as entry MAPxxP
		if (i+1)
		{
			lumpnum = W_CheckNumForName(va("%sP", G_BuildMapName(i+1)));
			if (lumpnum != LUMPERROR)
				PictureOfLevel = W_CachePatchNum(lumpnum, PU_CACHE);
			else
				PictureOfLevel = W_CachePatchName("BLANKLVL", PU_CACHE);
		}
		else
			PictureOfLevel = W_CachePatchName("RANDOMLV", PU_CACHE);

		x -= horizspac + w/2;

		V_DrawTinyScaledPatch(x, y, trans, PictureOfLevel);
	} while (x > horizspac-dupadjust);

	x = (BASEVIDWIDTH + w)/2 + horizspac;
	i = cv_nextmap.value - 1;
	trans = (rightfade ? V_TRANSLUCENT : 0);

	while (x < BASEVIDWIDTH+dupadjust-horizspac)
	{
		oldval = i;
		do
		{
			i++;
			if (i == NUMMAPS)
				i = -1;

			if (i == oldval)
				return;

			if(!mapheaderinfo[i])
				continue; // Don't allocate the header.  That just makes memory usage skyrocket.

		} while (!M_CanShowLevelInList(i, cv_newgametype.value));

		//  A 160x100 image of the level as entry MAPxxP
		if (i+1)
		{
			lumpnum = W_CheckNumForName(va("%sP", G_BuildMapName(i+1)));
			if (lumpnum != LUMPERROR)
				PictureOfLevel = W_CachePatchNum(lumpnum, PU_CACHE);
			else
				PictureOfLevel = W_CachePatchName("BLANKLVL", PU_CACHE);
		}
		else
			PictureOfLevel = W_CachePatchName("RANDOMLV", PU_CACHE);

		V_DrawTinyScaledPatch(x, y, trans, PictureOfLevel);

		x += horizspac + w/2;
	}
#undef horizspac
}

static void M_DrawServerMenu(void)
{
	M_DrawLevelSelectOnly(false, false);
	M_DrawGenericMenu();
}

static void M_MapChange(INT32 choice)
{
	(void)choice;

	levellistmode = LLM_CREATESERVER;

	CV_SetValue(&cv_newgametype, gametype);
	CV_SetValue(&cv_nextmap, gamemap);

	M_PrepareLevelSelect();
	M_SetupNextMenu(&MISC_ChangeLevelDef);
}

static void M_StartOfflineServerMenu(INT32 choice)
{
	(void)choice;
	levellistmode = LLM_CREATESERVER;
	M_PrepareLevelSelect();
	M_SetupNextMenu(&MP_OfflineServerDef);
}

#ifndef NONET
static void M_StartServerMenu(INT32 choice)
{
	(void)choice;
	levellistmode = LLM_CREATESERVER;
	M_PrepareLevelSelect();
	M_SetupNextMenu(&MP_ServerDef);

}

// ==============
// CONNECT VIA IP
// ==============

static char setupm_ip[28];
#endif
static UINT8 setupm_pselect = 1;

// Draw the funky Connect IP menu. Tails 11-19-2002
// So much work for such a little thing!
static void M_DrawMPMainMenu(void)
{
	INT32 x = currentMenu->x;
	INT32 y = currentMenu->y;

	// use generic drawer for cursor, items and title
	M_DrawGenericMenu();

#ifndef NONET
#if MAXPLAYERS != 16
Update the maxplayers label...
#endif
	V_DrawRightAlignedString(BASEVIDWIDTH-x, y+MP_MainMenu[4].alphaKey,
		((itemOn == 4) ? highlightflags : 0), "(2-16 players)");
#endif

	V_DrawRightAlignedString(BASEVIDWIDTH-x, y+MP_MainMenu[5].alphaKey,
		((itemOn == 5) ? highlightflags : 0),
		"(2-4 players)"
		);

#ifndef NONET
	y += MP_MainMenu[8].alphaKey;

	V_DrawFill(x+5, y+4+5, /*16*8 + 6,*/ BASEVIDWIDTH - 2*(x+5), 8+6, 239);

	// draw name string
	V_DrawString(x+8,y+12, V_ALLOWLOWERCASE, setupm_ip);

	// draw text cursor for name
	if (itemOn == 8
	    && skullAnimCounter < 4)   //blink cursor
		V_DrawCharacter(x+8+V_StringWidth(setupm_ip, V_ALLOWLOWERCASE),y+12,'_',false);
#endif

	// character bar, ripped off the color bar :V
	{
#define iconwidth 32
#define spacingwidth 32
#define incrwidth (iconwidth + spacingwidth)
		UINT8 i = 0, pskin, pcol;
		// player arrangement width, but there's also a chance i'm a furry, shhhhhh
		const INT32 paw = iconwidth + 3*incrwidth;
		INT32 trans = 0;
		UINT8 *colmap;
		x = BASEVIDWIDTH/2 - paw/2;
		y = currentMenu->y + 32;

		while (++i <= 4)
		{
			switch (i)
			{
				default:
					pskin = R_SkinAvailable(cv_skin.string);
					pcol = cv_playercolor.value;
					break;
				case 2:
					pskin = R_SkinAvailable(cv_skin2.string);
					pcol = cv_playercolor2.value;
					break;
				case 3:
					pskin = R_SkinAvailable(cv_skin3.string);
					pcol = cv_playercolor3.value;
					break;
				case 4:
					pskin = R_SkinAvailable(cv_skin4.string);
					pcol = cv_playercolor4.value;
					break;
			}

			if (pskin >= MAXSKINS)
				pskin = 0;

			if (!trans && i > cv_splitplayers.value)
				trans = V_TRANSLUCENT;

			colmap = R_GetTranslationColormap(pskin, pcol, GTC_MENUCACHE);

			V_DrawFixedPatch(x<<FRACBITS, y<<FRACBITS, FRACUNIT, trans, facewantprefix[pskin], colmap);

			if (itemOn == 2 && i == setupm_pselect)
			{
				static UINT8 cursorframe = 0;
				if (skullAnimCounter % 4 == 0)
					cursorframe++;
				if (cursorframe > 7)
					cursorframe = 0;
				V_DrawFixedPatch(x<<FRACBITS, y<<FRACBITS, FRACUNIT, 0, W_CachePatchName(va("K_BHILI%d", cursorframe+1), PU_CACHE), NULL);
			}

			x += incrwidth;
		}
#undef incrwidth
#undef spacingwidth
#undef iconwidth
	}
}

static void Splitplayers_OnChange(void)
{
	if (cv_splitplayers.value < setupm_pselect)
		setupm_pselect = 1;
}

static void M_SetupMultiHandler(INT32 choice)
{
	boolean exitmenu = false;  // exit to previous menu and send name change

	switch (choice)
	{
		case KEY_LEFTARROW:
			if (cv_splitplayers.value > 1)
			{
				if (--setupm_pselect < 1)
					setupm_pselect = cv_splitplayers.value;
				S_StartSound(NULL,sfx_menu1); // Tails
			}
			break;

		case KEY_RIGHTARROW:
			if (cv_splitplayers.value > 1)
			{
				if (++setupm_pselect > cv_splitplayers.value)
					setupm_pselect = 1;
				S_StartSound(NULL,sfx_menu1); // Tails
			}
			break;

		case KEY_DOWNARROW:
			M_NextOpt();
			S_StartSound(NULL,sfx_menu1); // Tails
			break;

		case KEY_UPARROW:
			M_PrevOpt();
			S_StartSound(NULL,sfx_menu1); // Tails
			break;

		case KEY_ENTER:
		{
			S_StartSound(NULL,sfx_menu1); // Tails
			currentMenu->lastOn = itemOn;
			switch (setupm_pselect)
			{
				case 2:
					M_SetupMultiPlayer2(0);
					return;
				case 3:
					M_SetupMultiPlayer3(0);
					return;
				case 4:
					M_SetupMultiPlayer4(0);
					return;
				default:
					M_SetupMultiPlayer(0);
					return;
			}
			break;
		}

		case KEY_ESCAPE:
			exitmenu = true;
			break;
	}

	if (exitmenu)
	{
		if (currentMenu->prevMenu)
			M_SetupNextMenu (currentMenu->prevMenu);
		else
			M_ClearMenus(true);
	}
}

#ifndef NONET

// Tails 11-19-2002
static void M_ConnectIP(INT32 choice)
{
	(void)choice;

	if (*setupm_ip == 0)
	{
		M_StartMessage("You must specify an IP address.\n", NULL, MM_NOTHING);
		return;
	}

	M_ClearMenus(true);

	COM_BufAddText(va("connect \"%s\"\n", setupm_ip));

	// A little "please wait" message.
	M_DrawTextBox(56, BASEVIDHEIGHT/2-12, 24, 2);
	V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT/2, 0, "Connecting to server...");
	I_OsPolling();
	I_UpdateNoBlit();
	if (rendermode == render_soft)
		I_FinishUpdate(); // page flip or blit buffer
}

// Tails 11-19-2002
static void M_HandleConnectIP(INT32 choice)
{
	size_t l;
	boolean exitmenu = false;  // exit to previous menu and send name change

	switch (choice)
	{
		case KEY_DOWNARROW:
			M_NextOpt();
			S_StartSound(NULL,sfx_menu1); // Tails
			break;

		case KEY_UPARROW:
			M_PrevOpt();
			S_StartSound(NULL,sfx_menu1); // Tails
			break;

		case KEY_ENTER:
			S_StartSound(NULL,sfx_menu1); // Tails
			currentMenu->lastOn = itemOn;
			M_ConnectIP(1);
			break;

		case KEY_ESCAPE:
			exitmenu = true;
			break;

		case KEY_BACKSPACE:
			if ((l = strlen(setupm_ip)) != 0)
			{
				S_StartSound(NULL,sfx_menu1); // Tails
				setupm_ip[l-1] = 0;
			}
			break;

		case KEY_DEL:
			if (setupm_ip[0])
			{
				S_StartSound(NULL,sfx_menu1); // Tails
				setupm_ip[0] = 0;
			}
			break;

		default:
			l = strlen(setupm_ip);
			if (l >= 28-1)
				break;

			// Rudimentary number and period enforcing - also allows letters so hostnames can be used instead
			if ((choice >= '-' && choice <= ':') || (choice >= 'A' && choice <= 'Z') || (choice >= 'a' && choice <= 'z'))
			{
				S_StartSound(NULL,sfx_menu1); // Tails
				setupm_ip[l] = (char)choice;
				setupm_ip[l+1] = 0;
			}
			else if (choice >= 199 && choice <= 211 && choice != 202 && choice != 206) //numpad too!
			{
				char keypad_translation[] = {'7','8','9','-','4','5','6','+','1','2','3','0','.'};
				choice = keypad_translation[choice - 199];
				S_StartSound(NULL,sfx_menu1); // Tails
				setupm_ip[l] = (char)choice;
				setupm_ip[l+1] = 0;
			}
			break;
	}

	if (exitmenu)
	{
		if (currentMenu->prevMenu)
			M_SetupNextMenu (currentMenu->prevMenu);
		else
			M_ClearMenus(true);
	}
}
#endif //!NONET

// ========================
// MULTIPLAYER PLAYER SETUP
// ========================
// Tails 03-02-2002

static INT32      multi_tics;
static state_t   *multi_state;

// this is set before entering the MultiPlayer setup menu,
// for either player 1 or 2
static char       setupm_name[MAXPLAYERNAME+1];
static player_t  *setupm_player;
static consvar_t *setupm_cvskin;
static consvar_t *setupm_cvcolor;
static consvar_t *setupm_cvname;
static INT32      setupm_fakeskin;
static INT32      setupm_fakecolor;

static void M_DrawSetupMultiPlayerMenu(void)
{
	INT32 mx, my, st, flags = 0;
	spritedef_t *sprdef;
	spriteframe_t *sprframe;
	patch_t *statbg = W_CachePatchName("K_STATBG", PU_CACHE);
	patch_t *statlr = W_CachePatchName("K_STATLR", PU_CACHE);
	patch_t *statud = W_CachePatchName("K_STATUD", PU_CACHE);
	patch_t *statdot = W_CachePatchName("K_SDOT0", PU_CACHE);
	patch_t *patch;
	UINT8 frame;
	UINT8 speed;
	UINT8 weight;
	UINT8 i;
	const UINT8 *flashcol = V_GetStringColormap(highlightflags);
	INT32 statx, staty;

	mx = MP_PlayerSetupDef.x;
	my = MP_PlayerSetupDef.y;

	statx = (BASEVIDWIDTH - mx - 118);
	staty = (my+62);

	// use generic drawer for cursor, items and title
	M_DrawGenericMenu();

	// draw name string
	M_DrawTextBox(mx + 32, my - 8, MAXPLAYERNAME, 1);
	V_DrawString(mx + 40, my, V_ALLOWLOWERCASE, setupm_name);

	// draw text cursor for name
	if (!itemOn && skullAnimCounter < 4) // blink cursor
		V_DrawCharacter(mx + 40 + V_StringWidth(setupm_name, V_ALLOWLOWERCASE), my, '_',false);

	// draw skin string
	st = V_StringWidth(skins[setupm_fakeskin].realname, 0);
	V_DrawString(BASEVIDWIDTH - mx - st, my + 16,
	             ((MP_PlayerSetupMenu[2].status & IT_TYPE) == IT_SPACE ? V_TRANSLUCENT : 0)|highlightflags|V_ALLOWLOWERCASE,
	             skins[setupm_fakeskin].realname);
	if (itemOn == 1)
	{
		V_DrawCharacter(BASEVIDWIDTH - mx - 10 - st - (skullAnimCounter/5), my + 16,
			'\x1C' | highlightflags, false); // left arrow
		V_DrawCharacter(BASEVIDWIDTH - mx + 2 + (skullAnimCounter/5), my + 16,
			'\x1D' | highlightflags, false); // right arrow
	}

	// draw the name of the color you have chosen
	// Just so people don't go thinking that "Default" is Green.
	st = V_StringWidth(KartColor_Names[setupm_fakecolor], 0);
	V_DrawString(BASEVIDWIDTH - mx - st, my + 152, highlightflags|V_ALLOWLOWERCASE, KartColor_Names[setupm_fakecolor]);	// SRB2kart
	if (itemOn == 2)
	{
		V_DrawCharacter(BASEVIDWIDTH - mx - 10 - st - (skullAnimCounter/5), my + 152,
			'\x1C' | highlightflags, false); // left arrow
		V_DrawCharacter(BASEVIDWIDTH - mx + 2 + (skullAnimCounter/5), my + 152,
			'\x1D' | highlightflags, false); // right arrow
	}

	// SRB2Kart: draw the stat backer
	// labels
	V_DrawThinString(statx+16, staty, V_6WIDTHSPACE|highlightflags, "Acceleration");
	V_DrawThinString(statx+91, staty, V_6WIDTHSPACE|highlightflags, "Max Speed");
	V_DrawThinString(statx, staty+12, V_6WIDTHSPACE|highlightflags, "Handling");
	V_DrawThinString(statx+7, staty+77, V_6WIDTHSPACE|highlightflags, "Weight");
	// label arrows
	V_DrawFixedPatch((statx+64)<<FRACBITS, staty<<FRACBITS, FRACUNIT, 0, statlr, flashcol);
	V_DrawFixedPatch((statx+24)<<FRACBITS, (staty+22)<<FRACBITS, FRACUNIT, 0, statud, flashcol);
	// bg
	V_DrawFixedPatch((statx+34)<<FRACBITS, (staty+10)<<FRACBITS, FRACUNIT, 0, statbg, 0);

	for (i = 0; i < numskins; i++) // draw the stat dots
	{
		if (i != setupm_fakeskin && R_SkinAvailable(skins[i].name) != -1)
		{
			speed = skins[i].kartspeed;
			weight = skins[i].kartweight;
			V_DrawFixedPatch(((BASEVIDWIDTH - mx - 80) + ((speed-1)*8))<<FRACBITS, ((my+76) + ((weight-1)*8))<<FRACBITS, FRACUNIT, 0, statdot, NULL);
		}
	}

	speed = skins[setupm_fakeskin].kartspeed;
	weight = skins[setupm_fakeskin].kartweight;

	statdot = W_CachePatchName("K_SDOT1", PU_CACHE);
	if (skullAnimCounter < 4) // SRB2Kart: we draw this dot later so that it's not covered if there's multiple skins with the same stats
		V_DrawFixedPatch(((BASEVIDWIDTH - mx - 80) + ((speed-1)*8))<<FRACBITS, ((my+76) + ((weight-1)*8))<<FRACBITS, FRACUNIT, 0, statdot, flashcol);
	else
		V_DrawFixedPatch(((BASEVIDWIDTH - mx - 80) + ((speed-1)*8))<<FRACBITS, ((my+76) + ((weight-1)*8))<<FRACBITS, FRACUNIT, 0, statdot, NULL);

	statdot = W_CachePatchName("K_SDOT2", PU_CACHE); // coloured center
	if (setupm_fakecolor)
		V_DrawFixedPatch(((BASEVIDWIDTH - mx - 80) + ((speed-1)*8))<<FRACBITS, ((my+76) + ((weight-1)*8))<<FRACBITS, FRACUNIT, 0, statdot, R_GetTranslationColormap(0, setupm_fakecolor, GTC_MENUCACHE));

	// 2.2 color bar backported with permission
#define charw 72
#define indexwidth 8
	{
		const INT32 colwidth = ((BASEVIDWIDTH-(2*mx))-charw)/(2*indexwidth);
		INT32 j = -colwidth;
		INT16 col = setupm_fakecolor - colwidth;
		INT32 x = mx;
		INT32 w = indexwidth;
		UINT8 h;

		while (col < 1)
			col += MAXSKINCOLORS-1;
		while (j <= colwidth)
		{
			if (!(j++))
				w = charw;
			else
				w = indexwidth;
			for (h = 0; h < 16; h++)
				V_DrawFill(x, my+162+h, w, 1, colortranslations[col][h]);
			if (++col >= MAXSKINCOLORS)
				col -= MAXSKINCOLORS-1;
			x += w;
		}
	}
#undef indexwidth

	// character bar, ripped off the color bar :V
	if (setupm_fakecolor) // inverse should never happen
#define iconwidth 32
	{
		const INT32 icons = 4;
		INT32 k = -icons;
		INT16 col = setupm_fakeskin - icons;
		INT32 x = BASEVIDWIDTH/2 - ((icons+1)*24) - 4;
		fixed_t scale = FRACUNIT/2;
		INT32 offx = 8, offy = 8;
		patch_t *cursor;
		static UINT8 cursorframe = 0;
		patch_t *face;
		UINT8 *colmap;

		if (skullAnimCounter % 4 == 0)
			cursorframe++;
		if (cursorframe > 7)
			cursorframe = 0;

		cursor = W_CachePatchName(va("K_BHILI%d", cursorframe+1), PU_CACHE);

		if (col < 0)
			col += numskins;
		while (k <= icons)
		{
			if (!(k++))
			{
				scale = FRACUNIT;
				face = facewantprefix[col];
				offx = 12;
				offy = 0;
			}
			else
			{
				scale = FRACUNIT/2;
				face = facerankprefix[col];
				offx = 8;
				offy = 8;
			}
			colmap =  R_GetTranslationColormap(col, setupm_fakecolor, GTC_MENUCACHE);
			V_DrawFixedPatch((x+offx)<<FRACBITS, (my+28+offy)<<FRACBITS, FRACUNIT, 0, face, colmap);
			if (scale == FRACUNIT) // bit of a hack
				V_DrawFixedPatch((x+offx)<<FRACBITS, (my+28+offy)<<FRACBITS, FRACUNIT, 0, cursor, colmap);
			if (++col >= numskins)
				col -= numskins;
			x += FixedMul(iconwidth<<FRACBITS, 3*scale/2)/FRACUNIT;
		}
	}
#undef iconwidth

	// anim the player in the box
	if (--multi_tics <= 0)
	{
		st = multi_state->nextstate;
		if (st != S_NULL)
			multi_state = &states[st];
		multi_tics = multi_state->tics;
		if (multi_tics == -1)
			multi_tics = 15;
	}

	// skin 0 is default player sprite
	if (R_SkinAvailable(skins[setupm_fakeskin].name) != -1)
		sprdef = &skins[R_SkinAvailable(skins[setupm_fakeskin].name)].spritedef;
	else
		sprdef = &skins[0].spritedef;

	if (!sprdef->numframes) // No frames ??
		return; // Can't render!

	frame = multi_state->frame & FF_FRAMEMASK;
	if (frame >= sprdef->numframes) // Walking animation missing
		frame = 0; // Try to use standing frame

	sprframe = &sprdef->spriteframes[frame];
	patch = W_CachePatchNum(sprframe->lumppat[1], PU_CACHE);
	if (sprframe->flip & 1) // Only for first sprite
		flags |= V_FLIP; // This sprite is left/right flipped!

	// draw box around guy
	V_DrawFill(mx + 43 - (charw/2), my+65, charw, 84, 239);

	// draw player sprite
	if (setupm_fakecolor) // inverse should never happen
	{
		UINT8 *colormap = R_GetTranslationColormap(setupm_fakeskin, setupm_fakecolor, GTC_MENUCACHE);

		if (skins[setupm_fakeskin].flags & SF_HIRES)
		{
			V_DrawFixedPatch((mx+43)<<FRACBITS,
						(my+131)<<FRACBITS,
						skins[setupm_fakeskin].highresscale,
						flags, patch, colormap);
		}
		else
			V_DrawMappedPatch(mx+43, my+131, flags, patch, colormap);
	}
#undef charw
}

// Handle 1P/2P MP Setup
static void M_HandleSetupMultiPlayer(INT32 choice)
{
	size_t   l;
	boolean  exitmenu = false;  // exit to previous menu and send name change

	if ((choice == gamecontrol[gc_fire][0] || choice == gamecontrol[gc_fire][1]) && itemOn == 2)
		choice = KEY_BACKSPACE; // Hack to allow resetting prefcolor on controllers

	switch (choice)
	{
		case KEY_DOWNARROW:
			M_NextOpt();
			S_StartSound(NULL,sfx_menu1); // Tails
			break;

		case KEY_UPARROW:
			M_PrevOpt();
			S_StartSound(NULL,sfx_menu1); // Tails
			break;

		case KEY_LEFTARROW:
			if (itemOn == 1)       //player skin
			{
				S_StartSound(NULL,sfx_menu1); // Tails
				setupm_fakeskin--;
			}
			else if (itemOn == 2) // player color
			{
				S_StartSound(NULL,sfx_menu1); // Tails
				setupm_fakecolor--;
			}
			break;

		case KEY_RIGHTARROW:
			if (itemOn == 1)       //player skin
			{
				S_StartSound(NULL,sfx_menu1); // Tails
				setupm_fakeskin++;
			}
			else if (itemOn == 2) // player color
			{
				S_StartSound(NULL,sfx_menu1); // Tails
				setupm_fakecolor++;
			}
			break;

		case KEY_ESCAPE:
			exitmenu = true;
			break;

		case KEY_BACKSPACE:
			if (itemOn == 0)
			{
				if ((l = strlen(setupm_name))!=0)
				{
					S_StartSound(NULL,sfx_menu1); // Tails
					setupm_name[l-1] =0;
				}
			}
			else if (itemOn == 2)
			{
				UINT8 col = skins[setupm_fakeskin].prefcolor;
				if (setupm_fakecolor != col)
				{
					S_StartSound(NULL,sfx_menu1); // Tails
					setupm_fakecolor = col;
				}
			}
			break;

		case KEY_DEL:
			if (itemOn == 0 && (l = strlen(setupm_name))!=0)
			{
				S_StartSound(NULL,sfx_menu1); // Tails
				setupm_name[0] = 0;
			}
			break;

		default:
			if (choice < 32 || choice > 127 || itemOn != 0)
				break;
			l = strlen(setupm_name);
			if (l < MAXPLAYERNAME)
			{
				S_StartSound(NULL,sfx_menu1); // Tails
				setupm_name[l] =(char)choice;
				setupm_name[l+1] =0;
			}
			break;
	}

	// check skin
	if (setupm_fakeskin < 0)
		setupm_fakeskin = numskins-1;
	if (setupm_fakeskin > numskins-1)
		setupm_fakeskin = 0;

	// check color
	if (setupm_fakecolor < 1)
		setupm_fakecolor = MAXSKINCOLORS-1;
	if (setupm_fakecolor > MAXSKINCOLORS-1)
		setupm_fakecolor = 1;

	if (exitmenu)
	{
		if (currentMenu->prevMenu)
			M_SetupNextMenu (currentMenu->prevMenu);
		else
			M_ClearMenus(true);
	}
}

// start the multiplayer setup menu
static void M_SetupMultiPlayer(INT32 choice)
{
	(void)choice;

	multi_state = &states[mobjinfo[MT_PLAYER].seestate];
	multi_tics = multi_state->tics;
	strcpy(setupm_name, cv_playername.string);

	// set for player 1
	setupm_player = &players[consoleplayer];
	setupm_cvskin = &cv_skin;
	setupm_cvcolor = &cv_playercolor;
	setupm_cvname = &cv_playername;

	// For whatever reason this doesn't work right if you just use ->value
	setupm_fakeskin = R_SkinAvailable(setupm_cvskin->string);
	if (setupm_fakeskin == -1)
		setupm_fakeskin = 0;
	setupm_fakecolor = setupm_cvcolor->value;

	// disable skin changes if we can't actually change skins
	if (!CanChangeSkin(consoleplayer))
		MP_PlayerSetupMenu[2].status = (IT_GRAYEDOUT);
	else
		MP_PlayerSetupMenu[2].status = (IT_KEYHANDLER|IT_STRING);

	MP_PlayerSetupDef.prevMenu = currentMenu;
	M_SetupNextMenu(&MP_PlayerSetupDef);
}

// start the multiplayer setup menu, for secondary player (splitscreen mode)
static void M_SetupMultiPlayer2(INT32 choice)
{
	(void)choice;

	multi_state = &states[mobjinfo[MT_PLAYER].seestate];
	multi_tics = multi_state->tics;
	strcpy (setupm_name, cv_playername2.string);

	// set for splitscreen secondary player
	setupm_player = &players[displayplayers[1]];
	setupm_cvskin = &cv_skin2;
	setupm_cvcolor = &cv_playercolor2;
	setupm_cvname = &cv_playername2;

	// For whatever reason this doesn't work right if you just use ->value
	setupm_fakeskin = R_SkinAvailable(setupm_cvskin->string);
	if (setupm_fakeskin == -1)
		setupm_fakeskin = 0;
	setupm_fakecolor = setupm_cvcolor->value;

	// disable skin changes if we can't actually change skins
	if (splitscreen && !CanChangeSkin(displayplayers[1]))
		MP_PlayerSetupMenu[2].status = (IT_GRAYEDOUT);
	else
		MP_PlayerSetupMenu[2].status = (IT_KEYHANDLER | IT_STRING);

	MP_PlayerSetupDef.prevMenu = currentMenu;
	M_SetupNextMenu(&MP_PlayerSetupDef);
}

// start the multiplayer setup menu, for third player (splitscreen mode)
static void M_SetupMultiPlayer3(INT32 choice)
{
	(void)choice;

	multi_state = &states[mobjinfo[MT_PLAYER].seestate];
	multi_tics = multi_state->tics;
	strcpy(setupm_name, cv_playername3.string);

	// set for splitscreen third player
	setupm_player = &players[displayplayers[2]];
	setupm_cvskin = &cv_skin3;
	setupm_cvcolor = &cv_playercolor3;
	setupm_cvname = &cv_playername3;

	// For whatever reason this doesn't work right if you just use ->value
	setupm_fakeskin = R_SkinAvailable(setupm_cvskin->string);
	if (setupm_fakeskin == -1)
		setupm_fakeskin = 0;
	setupm_fakecolor = setupm_cvcolor->value;

	// disable skin changes if we can't actually change skins
	if (splitscreen > 1 && !CanChangeSkin(displayplayers[2]))
		MP_PlayerSetupMenu[2].status = (IT_GRAYEDOUT);
	else
		MP_PlayerSetupMenu[2].status = (IT_KEYHANDLER | IT_STRING);

	MP_PlayerSetupDef.prevMenu = currentMenu;
	M_SetupNextMenu(&MP_PlayerSetupDef);
}

// start the multiplayer setup menu, for third player (splitscreen mode)
static void M_SetupMultiPlayer4(INT32 choice)
{
	(void)choice;

	multi_state = &states[mobjinfo[MT_PLAYER].seestate];
	multi_tics = multi_state->tics;
	strcpy(setupm_name, cv_playername4.string);

	// set for splitscreen fourth player
	setupm_player = &players[displayplayers[3]];
	setupm_cvskin = &cv_skin4;
	setupm_cvcolor = &cv_playercolor4;
	setupm_cvname = &cv_playername4;

	// For whatever reason this doesn't work right if you just use ->value
	setupm_fakeskin = R_SkinAvailable(setupm_cvskin->string);
	if (setupm_fakeskin == -1)
		setupm_fakeskin = 0;
	setupm_fakecolor = setupm_cvcolor->value;

	// disable skin changes if we can't actually change skins
	if (splitscreen > 2 && !CanChangeSkin(displayplayers[3]))
		MP_PlayerSetupMenu[2].status = (IT_GRAYEDOUT);
	else
		MP_PlayerSetupMenu[2].status = (IT_KEYHANDLER | IT_STRING);

	MP_PlayerSetupDef.prevMenu = currentMenu;
	M_SetupNextMenu(&MP_PlayerSetupDef);
}

static boolean M_QuitMultiPlayerMenu(void)
{
	size_t l;
	// send name if changed
	if (strcmp(setupm_name, setupm_cvname->string))
	{
		// remove trailing whitespaces
		for (l= strlen(setupm_name)-1;
		    (signed)l >= 0 && setupm_name[l] ==' '; l--)
			setupm_name[l] =0;
		COM_BufAddText (va("%s \"%s\"\n",setupm_cvname->name,setupm_name));
	}
	// you know what? always putting these in the buffer won't hurt anything.
	COM_BufAddText (va("%s \"%s\"\n",setupm_cvskin->name,skins[setupm_fakeskin].name));
	COM_BufAddText (va("%s %d\n",setupm_cvcolor->name,setupm_fakecolor));
	return true;
}

// =================
// DATA OPTIONS MENU
// =================
static UINT8 erasecontext = 0;

static void M_EraseDataResponse(INT32 ch)
{
	if (ch != 'y' && ch != KEY_ENTER)
		return;

	S_StartSound(NULL, sfx_itrole); // bweh heh heh

	// Delete the data
	if (erasecontext == 2)
	{
		// SRB2Kart: This actually needs to be done FIRST, so that you don't immediately regain playtime/matches secrets
		totalplaytime = 0;
		matchesplayed = 0;
		F_StartIntro();
	}
	if (erasecontext != 1)
		G_ClearRecords();
	if (erasecontext != 0)
		M_ClearSecrets();
	M_ClearMenus(true);
}

static void M_EraseData(INT32 choice)
{
	const char *eschoice, *esstr = M_GetText("Are you sure you want to erase\n%s?\n\n(Press 'Y' to confirm)\n");

	erasecontext = (UINT8)choice;

	if (choice == 0)
		eschoice = M_GetText("Record Attack data");
	else if (choice == 1)
		eschoice = M_GetText("Secrets data");
	else
		eschoice = M_GetText("ALL game data");

	M_StartMessage(va(esstr, eschoice),M_EraseDataResponse,MM_YESNO);
}

static void M_ScreenshotOptions(INT32 choice)
{
	(void)choice;
	Screenshot_option_Onchange();
	Moviemode_mode_Onchange();

	M_SetupNextMenu(&OP_ScreenshotOptionsDef);
}

// =============
// JOYSTICK MENU
// =============

// Start the controls menu, setting it up for either the console player,
// or the secondary splitscreen player

static void M_DrawJoystick(void)
{
	INT32 i, compareval4, compareval3, compareval2, compareval;

	M_DrawGenericMenu();

	for (i = 0; i < 8; i++)
	{
		M_DrawTextBox(OP_JoystickSetDef.x-8, OP_JoystickSetDef.y+LINEHEIGHT*i-12, 28, 1);
		//M_DrawSaveLoadBorder(OP_JoystickSetDef.x, OP_JoystickSetDef.y+LINEHEIGHT*i);

#ifdef JOYSTICK_HOTPLUG
		if (atoi(cv_usejoystick4.string) > I_NumJoys())
			compareval4 = atoi(cv_usejoystick4.string);
		else
			compareval4 = cv_usejoystick4.value;

		if (atoi(cv_usejoystick3.string) > I_NumJoys())
			compareval3 = atoi(cv_usejoystick3.string);
		else
			compareval3 = cv_usejoystick3.value;

		if (atoi(cv_usejoystick2.string) > I_NumJoys())
			compareval2 = atoi(cv_usejoystick2.string);
		else
			compareval2 = cv_usejoystick2.value;

		if (atoi(cv_usejoystick.string) > I_NumJoys())
			compareval = atoi(cv_usejoystick.string);
		else
			compareval = cv_usejoystick.value;
#else
		compareval4 = cv_usejoystick4.value;
		compareval3 = cv_usejoystick3.value;
		compareval2 = cv_usejoystick2.value;
		compareval = cv_usejoystick.value
#endif

		if ((setupcontrolplayer == 4 && (i == compareval4))
			|| (setupcontrolplayer == 3 && (i == compareval3))
			|| (setupcontrolplayer == 2 && (i == compareval2))
			|| (setupcontrolplayer == 1 && (i == compareval)))
			V_DrawString(OP_JoystickSetDef.x, OP_JoystickSetDef.y+LINEHEIGHT*i-4,V_GREENMAP,joystickInfo[i]);
		else
			V_DrawString(OP_JoystickSetDef.x, OP_JoystickSetDef.y+LINEHEIGHT*i-4,0,joystickInfo[i]);
	}
}

void M_SetupJoystickMenu(INT32 choice)
{
	INT32 i = 0;
	const char *joyNA = "Unavailable";
	INT32 n = I_NumJoys();
	(void)choice;

	strcpy(joystickInfo[i], "None");

	for (i = 1; i < 8; i++)
	{
		if (i <= n && (I_GetJoyName(i)) != NULL)
			strncpy(joystickInfo[i], I_GetJoyName(i), 28);
		else
			strcpy(joystickInfo[i], joyNA);

#ifdef JOYSTICK_HOTPLUG
		// We use cv_usejoystick.string as the USER-SET var
		// and cv_usejoystick.value as the INTERNAL var
		//
		// In practice, if cv_usejoystick.string == 0, this overrides
		// cv_usejoystick.value and always disables
		//
		// Update cv_usejoystick.string here so that the user can
		// properly change this value.
		if (i == cv_usejoystick.value)
			CV_SetValue(&cv_usejoystick, i);
		if (i == cv_usejoystick2.value)
			CV_SetValue(&cv_usejoystick2, i);
		if (i == cv_usejoystick3.value)
			CV_SetValue(&cv_usejoystick3, i);
		if (i == cv_usejoystick4.value)
			CV_SetValue(&cv_usejoystick4, i);
#endif
	}

	M_SetupNextMenu(&OP_JoystickSetDef);
}

static void M_Setup1PJoystickMenu(INT32 choice)
{
	setupcontrolplayer = 1;
	OP_JoystickSetDef.prevMenu = &OP_Joystick1Def;
	M_SetupJoystickMenu(choice);
}

static void M_Setup2PJoystickMenu(INT32 choice)
{
	setupcontrolplayer = 2;
	OP_JoystickSetDef.prevMenu = &OP_Joystick2Def;
	M_SetupJoystickMenu(choice);
}

static void M_Setup3PJoystickMenu(INT32 choice)
{
	setupcontrolplayer = 3;
	OP_JoystickSetDef.prevMenu = &OP_Joystick3Def;
	M_SetupJoystickMenu(choice);
}

static void M_Setup4PJoystickMenu(INT32 choice)
{
	setupcontrolplayer = 4;
	OP_JoystickSetDef.prevMenu = &OP_Joystick4Def;
	M_SetupJoystickMenu(choice);
}

static void M_AssignJoystick(INT32 choice)
{
#ifdef JOYSTICK_HOTPLUG
	INT32 oldchoice, oldstringchoice;
	INT32 numjoys = I_NumJoys();

	if (setupcontrolplayer == 4)
	{
		oldchoice = oldstringchoice = atoi(cv_usejoystick4.string) > numjoys ? atoi(cv_usejoystick4.string) : cv_usejoystick4.value;
		CV_SetValue(&cv_usejoystick4, choice);

		// Just in case last-minute changes were made to cv_usejoystick.value,
		// update the string too
		// But don't do this if we're intentionally setting higher than numjoys
		if (choice <= numjoys)
		{
			CV_SetValue(&cv_usejoystick4, cv_usejoystick4.value);

			// reset this so the comparison is valid
			if (oldchoice > numjoys)
				oldchoice = cv_usejoystick4.value;

			if (oldchoice != choice)
			{
				if (choice && oldstringchoice > numjoys) // if we did not select "None", we likely selected a used device
					CV_SetValue(&cv_usejoystick4, (oldstringchoice > numjoys ? oldstringchoice : oldchoice));

				if (oldstringchoice ==
					(atoi(cv_usejoystick4.string) > numjoys ? atoi(cv_usejoystick4.string) : cv_usejoystick4.value))
					M_StartMessage("This joystick is used by another\n"
								   "player. Reset the joystick\n"
								   "for that player first.\n\n"
								   "(Press a key)\n", NULL, MM_NOTHING);
			}
		}
	}
	else if (setupcontrolplayer == 3)
	{
		oldchoice = oldstringchoice = atoi(cv_usejoystick3.string) > numjoys ? atoi(cv_usejoystick3.string) : cv_usejoystick3.value;
		CV_SetValue(&cv_usejoystick3, choice);

		// Just in case last-minute changes were made to cv_usejoystick.value,
		// update the string too
		// But don't do this if we're intentionally setting higher than numjoys
		if (choice <= numjoys)
		{
			CV_SetValue(&cv_usejoystick3, cv_usejoystick3.value);

			// reset this so the comparison is valid
			if (oldchoice > numjoys)
				oldchoice = cv_usejoystick3.value;

			if (oldchoice != choice)
			{
				if (choice && oldstringchoice > numjoys) // if we did not select "None", we likely selected a used device
					CV_SetValue(&cv_usejoystick3, (oldstringchoice > numjoys ? oldstringchoice : oldchoice));

				if (oldstringchoice ==
					(atoi(cv_usejoystick3.string) > numjoys ? atoi(cv_usejoystick3.string) : cv_usejoystick3.value))
					M_StartMessage("This joystick is used by another\n"
								   "player. Reset the joystick\n"
								   "for that player first.\n\n"
								   "(Press a key)\n", NULL, MM_NOTHING);
			}
		}
	}
	else if (setupcontrolplayer == 2)
	{
		oldchoice = oldstringchoice = atoi(cv_usejoystick2.string) > numjoys ? atoi(cv_usejoystick2.string) : cv_usejoystick2.value;
		CV_SetValue(&cv_usejoystick2, choice);

		// Just in case last-minute changes were made to cv_usejoystick.value,
		// update the string too
		// But don't do this if we're intentionally setting higher than numjoys
		if (choice <= numjoys)
		{
			CV_SetValue(&cv_usejoystick2, cv_usejoystick2.value);

			// reset this so the comparison is valid
			if (oldchoice > numjoys)
				oldchoice = cv_usejoystick2.value;

			if (oldchoice != choice)
			{
				if (choice && oldstringchoice > numjoys) // if we did not select "None", we likely selected a used device
					CV_SetValue(&cv_usejoystick2, (oldstringchoice > numjoys ? oldstringchoice : oldchoice));

				if (oldstringchoice ==
					(atoi(cv_usejoystick2.string) > numjoys ? atoi(cv_usejoystick2.string) : cv_usejoystick2.value))
					M_StartMessage("This joystick is used by another\n"
					               "player. Reset the joystick\n"
					               "for that player first.\n\n"
					               "(Press a key)\n", NULL, MM_NOTHING);
			}
		}
	}
	else if (setupcontrolplayer == 1)
	{
		oldchoice = oldstringchoice = atoi(cv_usejoystick.string) > numjoys ? atoi(cv_usejoystick.string) : cv_usejoystick.value;
		CV_SetValue(&cv_usejoystick, choice);

		// Just in case last-minute changes were made to cv_usejoystick.value,
		// update the string too
		// But don't do this if we're intentionally setting higher than numjoys
		if (choice <= numjoys)
		{
			CV_SetValue(&cv_usejoystick, cv_usejoystick.value);

			// reset this so the comparison is valid
			if (oldchoice > numjoys)
				oldchoice = cv_usejoystick.value;

			if (oldchoice != choice)
			{
				if (choice && oldstringchoice > numjoys) // if we did not select "None", we likely selected a used device
					CV_SetValue(&cv_usejoystick, (oldstringchoice > numjoys ? oldstringchoice : oldchoice));

				if (oldstringchoice ==
					(atoi(cv_usejoystick.string) > numjoys ? atoi(cv_usejoystick.string) : cv_usejoystick.value))
					M_StartMessage("This joystick is used by another\n"
					               "player. Reset the joystick\n"
					               "for that player first.\n\n"
					               "(Press a key)\n", NULL, MM_NOTHING);
			}
		}
	}
#else
	if (setupcontrolplayer == 4)
		CV_SetValue(&cv_usejoystick4, choice);
	else if (setupcontrolplayer == 3)
		CV_SetValue(&cv_usejoystick3, choice);
	else if (setupcontrolplayer == 2)
		CV_SetValue(&cv_usejoystick2, choice);
	else if (setupcontrolplayer == 1)
		CV_SetValue(&cv_usejoystick, choice);
#endif
}

// =============
// CONTROLS MENU
// =============

static void M_Setup1PControlsMenu(INT32 choice)
{
	(void)choice;
	setupcontrolplayer = 1;
	setupcontrols = gamecontrol;        // was called from main Options (for console player, then)
	currentMenu->lastOn = itemOn;

	// Set proper gamepad options
	OP_AllControlsMenu[0].itemaction = &OP_Joystick1Def;

	// Unhide P1-only controls
	OP_AllControlsMenu[15].status = IT_CONTROL; // Chat
	//OP_AllControlsMenu[16].status = IT_CONTROL; // Team-chat
	OP_AllControlsMenu[16].status = IT_CONTROL; // Rankings
	//OP_AllControlsMenu[17].status = IT_CONTROL; // Viewpoint
	// 18 is Reset Camera, 19 is Toggle Chasecam
	OP_AllControlsMenu[20].status = IT_CONTROL; // Pause
	OP_AllControlsMenu[21].status = IT_CONTROL; // Screenshot
	OP_AllControlsMenu[22].status = IT_CONTROL; // GIF
	OP_AllControlsMenu[23].status = IT_CONTROL; // System Menu
	OP_AllControlsMenu[24].status = IT_CONTROL; // Console
	/*OP_AllControlsMenu[25].status = IT_HEADER; // Spectator Controls header
	OP_AllControlsMenu[26].status = IT_SPACE; // Spectator Controls space
	OP_AllControlsMenu[27].status = IT_CONTROL; // Spectate
	OP_AllControlsMenu[28].status = IT_CONTROL; // Look Up
	OP_AllControlsMenu[29].status = IT_CONTROL; // Look Down
	OP_AllControlsMenu[30].status = IT_CONTROL; // Center View
	*/

	M_SetupNextMenu(&OP_AllControlsDef);
}

static void M_Setup2PControlsMenu(INT32 choice)
{
	(void)choice;
	setupcontrolplayer = 2;
	setupcontrols = gamecontrolbis;
	currentMenu->lastOn = itemOn;

	// Set proper gamepad options
	OP_AllControlsMenu[0].itemaction = &OP_Joystick2Def;

	// Hide P1-only controls
	OP_AllControlsMenu[15].status = IT_GRAYEDOUT2; // Chat
	//OP_AllControlsMenu[16].status = IT_GRAYEDOUT2; // Team-chat
	OP_AllControlsMenu[16].status = IT_GRAYEDOUT2; // Rankings
	//OP_AllControlsMenu[17].status = IT_GRAYEDOUT2; // Viewpoint
	// 18 is Reset Camera, 19 is Toggle Chasecam
	OP_AllControlsMenu[20].status = IT_GRAYEDOUT2; // Pause
	OP_AllControlsMenu[21].status = IT_GRAYEDOUT2; // Screenshot
	OP_AllControlsMenu[22].status = IT_GRAYEDOUT2; // GIF
	OP_AllControlsMenu[23].status = IT_GRAYEDOUT2; // System Menu
	OP_AllControlsMenu[24].status = IT_GRAYEDOUT2; // Console
	/*OP_AllControlsMenu[25].status = IT_GRAYEDOUT2; // Spectator Controls header
	OP_AllControlsMenu[26].status = IT_GRAYEDOUT2; // Spectator Controls space
	OP_AllControlsMenu[27].status = IT_GRAYEDOUT2; // Spectate
	OP_AllControlsMenu[28].status = IT_GRAYEDOUT2; // Look Up
	OP_AllControlsMenu[29].status = IT_GRAYEDOUT2; // Look Down
	OP_AllControlsMenu[30].status = IT_GRAYEDOUT2; // Center View
	*/

	M_SetupNextMenu(&OP_AllControlsDef);
}

static void M_Setup3PControlsMenu(INT32 choice)
{
	(void)choice;
	setupcontrolplayer = 3;
	setupcontrols = gamecontrol3;
	currentMenu->lastOn = itemOn;

	// Set proper gamepad options
	OP_AllControlsMenu[0].itemaction = &OP_Joystick3Def;

	// Hide P1-only controls
	OP_AllControlsMenu[15].status = IT_GRAYEDOUT2; // Chat
	//OP_AllControlsMenu[16].status = IT_GRAYEDOUT2; // Team-chat
	OP_AllControlsMenu[16].status = IT_GRAYEDOUT2; // Rankings
	//OP_AllControlsMenu[17].status = IT_GRAYEDOUT2; // Viewpoint
	// 18 is Reset Camera, 19 is Toggle Chasecam
	OP_AllControlsMenu[20].status = IT_GRAYEDOUT2; // Pause
	OP_AllControlsMenu[21].status = IT_GRAYEDOUT2; // Screenshot
	OP_AllControlsMenu[22].status = IT_GRAYEDOUT2; // GIF
	OP_AllControlsMenu[23].status = IT_GRAYEDOUT2; // System Menu
	OP_AllControlsMenu[24].status = IT_GRAYEDOUT2; // Console
	/*OP_AllControlsMenu[25].status = IT_GRAYEDOUT2; // Spectator Controls header
	OP_AllControlsMenu[26].status = IT_GRAYEDOUT2; // Spectator Controls space
	OP_AllControlsMenu[27].status = IT_GRAYEDOUT2; // Spectate
	OP_AllControlsMenu[28].status = IT_GRAYEDOUT2; // Look Up
	OP_AllControlsMenu[29].status = IT_GRAYEDOUT2; // Look Down
	OP_AllControlsMenu[30].status = IT_GRAYEDOUT2; // Center View
	*/

	M_SetupNextMenu(&OP_AllControlsDef);
}

static void M_Setup4PControlsMenu(INT32 choice)
{
	(void)choice;
	setupcontrolplayer = 4;
	setupcontrols = gamecontrol4;
	currentMenu->lastOn = itemOn;

	// Set proper gamepad options
	OP_AllControlsMenu[0].itemaction = &OP_Joystick4Def;

	// Hide P1-only controls
	OP_AllControlsMenu[15].status = IT_GRAYEDOUT2; // Chat
	//OP_AllControlsMenu[16].status = IT_GRAYEDOUT2; // Team-chat
	OP_AllControlsMenu[16].status = IT_GRAYEDOUT2; // Rankings
	//OP_AllControlsMenu[17].status = IT_GRAYEDOUT2; // Viewpoint
	// 18 is Reset Camera, 19 is Toggle Chasecam
	OP_AllControlsMenu[20].status = IT_GRAYEDOUT2; // Pause
	OP_AllControlsMenu[21].status = IT_GRAYEDOUT2; // Screenshot
	OP_AllControlsMenu[22].status = IT_GRAYEDOUT2; // GIF
	OP_AllControlsMenu[23].status = IT_GRAYEDOUT2; // System Menu
	OP_AllControlsMenu[24].status = IT_GRAYEDOUT2; // Console
	/*OP_AllControlsMenu[25].status = IT_GRAYEDOUT2; // Spectator Controls header
	OP_AllControlsMenu[26].status = IT_GRAYEDOUT2; // Spectator Controls space
	OP_AllControlsMenu[27].status = IT_GRAYEDOUT2; // Spectate
	OP_AllControlsMenu[28].status = IT_GRAYEDOUT2; // Look Up
	OP_AllControlsMenu[29].status = IT_GRAYEDOUT2; // Look Down
	OP_AllControlsMenu[30].status = IT_GRAYEDOUT2; // Center View
	*/

	M_SetupNextMenu(&OP_AllControlsDef);
}

#define controlheight 18

// Draws the Customise Controls menu
static void M_DrawControl(void)
{
	char tmp[50];
	INT32 x, y, i, max, cursory = 0, iter;
	INT32 keys[2];

	x = currentMenu->x;
	y = currentMenu->y;

	/*i = itemOn - (controlheight/2);
	if (i < 0)
		i = 0;
	*/

	iter = (controlheight/2);
	for (i = itemOn; ((iter || currentMenu->menuitems[i].status == IT_GRAYEDOUT2) && i > 0); i--)
	{
		if (currentMenu->menuitems[i].status != IT_GRAYEDOUT2)
			iter--;
	}
	if (currentMenu->menuitems[i].status == IT_GRAYEDOUT2)
		i--;

	iter += (controlheight/2);
	for (max = itemOn; (iter && max < currentMenu->numitems); max++)
	{
		if (currentMenu->menuitems[max].status != IT_GRAYEDOUT2)
			iter--;
	}

	if (iter)
	{
		iter += (controlheight/2);
		for (i = itemOn; ((iter || currentMenu->menuitems[i].status == IT_GRAYEDOUT2) && i > 0); i--)
		{
			if (currentMenu->menuitems[i].status != IT_GRAYEDOUT2)
				iter--;
		}
	}

	/*max = i + controlheight;
	if (max > currentMenu->numitems)
	{
		max = currentMenu->numitems;
		if (max < controlheight)
			i = 0;
		else
			i = max - controlheight;
	}*/

	// draw title (or big pic)
	M_DrawMenuTitle();

	M_CentreText(28,
		(setupcontrolplayer > 1 ? va("\x86""Set controls for ""\x82""Player %d", setupcontrolplayer) :
		                          "\x86""Press ""\x82""ENTER""\x86"" to change, ""\x82""BACKSPACE""\x86"" to clear"));

	if (i)
		V_DrawCharacter(currentMenu->x - 16, y-(skullAnimCounter/5),
			'\x1A' | highlightflags, false); // up arrow
	if (max != currentMenu->numitems)
		V_DrawCharacter(currentMenu->x - 16, y+(SMALLLINEHEIGHT*(controlheight-1))+(skullAnimCounter/5) + (skullAnimCounter/5),
			'\x1B' | highlightflags, false); // down arrow

	for (; i < max; i++)
	{
		if (currentMenu->menuitems[i].status == IT_GRAYEDOUT2)
			continue;

		if (i == itemOn)
			cursory = y;

		if (currentMenu->menuitems[i].status == IT_CONTROL)
		{
			V_DrawString(x, y, ((i == itemOn) ? highlightflags : 0), currentMenu->menuitems[i].text);
			keys[0] = setupcontrols[currentMenu->menuitems[i].alphaKey][0];
			keys[1] = setupcontrols[currentMenu->menuitems[i].alphaKey][1];

			tmp[0] ='\0';
			if (keys[0] == KEY_NULL && keys[1] == KEY_NULL)
			{
				strcpy(tmp, "---");
			}
			else
			{
				if (keys[0] != KEY_NULL)
					strcat (tmp, G_KeynumToString (keys[0]));

				if (keys[0] != KEY_NULL && keys[1] != KEY_NULL)
					strcat(tmp,", ");

				if (keys[1] != KEY_NULL)
					strcat (tmp, G_KeynumToString (keys[1]));

			}
			V_DrawRightAlignedString(BASEVIDWIDTH-currentMenu->x, y, highlightflags, tmp);
		}
		/*else if (currentMenu->menuitems[i].status == IT_GRAYEDOUT2)
			V_DrawString(x, y, V_TRANSLUCENT, currentMenu->menuitems[i].text);*/
		else if ((currentMenu->menuitems[i].status == IT_HEADER) && (i != max-1))
			V_DrawString(19, y+6, highlightflags, currentMenu->menuitems[i].text);
		else if (currentMenu->menuitems[i].status & IT_STRING)
			V_DrawString(x, y, ((i == itemOn) ? highlightflags : 0), currentMenu->menuitems[i].text);

		y += SMALLLINEHEIGHT;
	}

	V_DrawScaledPatch(currentMenu->x - 20, cursory, 0,
		W_CachePatchName("M_CURSOR", PU_CACHE));
}

#undef controlheight

static INT32 controltochange;
static char controltochangetext[33];

static void M_ChangecontrolResponse(event_t *ev)
{
	INT32        control;
	INT32        found;
	INT32        ch = ev->data1;

	// ESCAPE cancels; dummy out PAUSE
	if (ch != KEY_ESCAPE && ch != KEY_PAUSE)
	{

		switch (ev->type)
		{
			// ignore mouse/joy movements, just get buttons
			case ev_mouse:
			case ev_mouse2:
			case ev_joystick:
			case ev_joystick2:
			case ev_joystick3:
			case ev_joystick4:
				ch = KEY_NULL;      // no key
			break;

			// keypad arrows are converted for the menu in cursor arrows
			// so use the event instead of ch
			case ev_keydown:
				ch = ev->data1;
			break;

			default:
			break;
		}

		control = controltochange;

		// check if we already entered this key
		found = -1;
		if (setupcontrols[control][0] ==ch)
			found = 0;
		else if (setupcontrols[control][1] ==ch)
			found = 1;
		if (found >= 0)
		{
			// replace mouse and joy clicks by double clicks
			if (ch >= KEY_MOUSE1 && ch <= KEY_MOUSE1+MOUSEBUTTONS)
				setupcontrols[control][found] = ch-KEY_MOUSE1+KEY_DBLMOUSE1;
			else if (ch >= KEY_JOY1 && ch <= KEY_JOY1+JOYBUTTONS)
				setupcontrols[control][found] = ch-KEY_JOY1+KEY_DBLJOY1;
			else if (ch >= KEY_2MOUSE1 && ch <= KEY_2MOUSE1+MOUSEBUTTONS)
				setupcontrols[control][found] = ch-KEY_2MOUSE1+KEY_DBL2MOUSE1;
			else if (ch >= KEY_2JOY1 && ch <= KEY_2JOY1+JOYBUTTONS)
				setupcontrols[control][found] = ch-KEY_2JOY1+KEY_DBL2JOY1;
			else if (ch >= KEY_3JOY1 && ch <= KEY_3JOY1+JOYBUTTONS)
				setupcontrols[control][found] = ch-KEY_3JOY1+KEY_DBL3JOY1;
			else if (ch >= KEY_4JOY1 && ch <= KEY_4JOY1+JOYBUTTONS)
				setupcontrols[control][found] = ch-KEY_4JOY1+KEY_DBL4JOY1;
		}
		else
		{
			// check if change key1 or key2, or replace the two by the new
			found = 0;
			if (setupcontrols[control][0] == KEY_NULL)
				found++;
			if (setupcontrols[control][1] == KEY_NULL)
				found++;
			if (found == 2)
			{
				found = 0;
				setupcontrols[control][1] = KEY_NULL;  //replace key 1,clear key2
			}
			(void)G_CheckDoubleUsage(ch, true);
			setupcontrols[control][found] = ch;
		}
		S_StartSound(NULL, sfx_s221);
	}
	else if (ch == KEY_PAUSE)
	{
		// This buffer assumes a 125-character message plus a 32-character control name (per controltochangetext buffer size)
		static char tmp[158];
		menu_t *prev = currentMenu->prevMenu;

		if (controltochange == gc_pause)
			sprintf(tmp, M_GetText("The \x82Pause Key \x80is enabled, but \nyou may select another key. \n\nHit another key for\n%s\nESC for Cancel"),
				controltochangetext);
		else
			sprintf(tmp, M_GetText("The \x82Pause Key \x80is enabled, but \nit is not configurable. \n\nHit another key for\n%s\nESC for Cancel"),
				controltochangetext);

		M_StartMessage(tmp, M_ChangecontrolResponse, MM_EVENTHANDLER);
		currentMenu->prevMenu = prev;

		S_StartSound(NULL, sfx_s3k42);
		return;
	}
	else
		S_StartSound(NULL, sfx_s224);

	M_StopMessage(0);
}

static void M_ChangeControl(INT32 choice)
{
	// This buffer assumes a 35-character message (per below) plus a max control name limit of 32 chars (per controltochangetext)
	// If you change the below message, then change the size of this buffer!
	static char tmp[68];

	controltochange = currentMenu->menuitems[choice].alphaKey;
	sprintf(tmp, M_GetText("Hit the new key for\n%s\nESC for Cancel"),
		currentMenu->menuitems[choice].text);
	strlcpy(controltochangetext, currentMenu->menuitems[choice].text, 33);

	M_StartMessage(tmp, M_ChangecontrolResponse, MM_EVENTHANDLER);
}

static void M_ResetControlsResponse(INT32 ch)
{
	INT32 i;

	if (ch != 'y' && ch != KEY_ENTER)
		return;

	// clear all controls
	for (i = 0; i < num_gamecontrols; i++)
	{
		switch (setupcontrolplayer)
		{
			case 4:
				G_ClearControlKeys(gamecontrol4, i);
				break;
			case 3:
				G_ClearControlKeys(gamecontrol3, i);
				break;
			case 2:
				G_ClearControlKeys(gamecontrolbis, i);
				break;
			case 1:
			default:
				G_ClearControlKeys(gamecontrol, i);
				break;
		}
	}

	// Setup original defaults
	G_Controldefault(setupcontrolplayer);

	// Setup gamepad option defaults (yucky)
	switch (setupcontrolplayer)
	{
		case 4:
			CV_StealthSet(&cv_usejoystick4, cv_usejoystick4.defaultvalue);
			CV_StealthSet(&cv_turnaxis4, cv_turnaxis4.defaultvalue);
			CV_StealthSet(&cv_moveaxis4, cv_moveaxis4.defaultvalue);
			CV_StealthSet(&cv_brakeaxis4, cv_brakeaxis4.defaultvalue);
			CV_StealthSet(&cv_aimaxis4, cv_aimaxis4.defaultvalue);
			CV_StealthSet(&cv_lookaxis4, cv_lookaxis4.defaultvalue);
			CV_StealthSet(&cv_fireaxis4, cv_fireaxis4.defaultvalue);
			CV_StealthSet(&cv_driftaxis4, cv_driftaxis4.defaultvalue);
			break;
		case 3:
			CV_StealthSet(&cv_usejoystick3, cv_usejoystick3.defaultvalue);
			CV_StealthSet(&cv_turnaxis3, cv_turnaxis3.defaultvalue);
			CV_StealthSet(&cv_moveaxis3, cv_moveaxis3.defaultvalue);
			CV_StealthSet(&cv_brakeaxis3, cv_brakeaxis3.defaultvalue);
			CV_StealthSet(&cv_aimaxis3, cv_aimaxis3.defaultvalue);
			CV_StealthSet(&cv_lookaxis3, cv_lookaxis3.defaultvalue);
			CV_StealthSet(&cv_fireaxis3, cv_fireaxis3.defaultvalue);
			CV_StealthSet(&cv_driftaxis3, cv_driftaxis3.defaultvalue);
			break;
		case 2:
			CV_StealthSet(&cv_usejoystick2, cv_usejoystick2.defaultvalue);
			CV_StealthSet(&cv_turnaxis2, cv_turnaxis2.defaultvalue);
			CV_StealthSet(&cv_moveaxis2, cv_moveaxis2.defaultvalue);
			CV_StealthSet(&cv_brakeaxis2, cv_brakeaxis2.defaultvalue);
			CV_StealthSet(&cv_aimaxis2, cv_aimaxis2.defaultvalue);
			CV_StealthSet(&cv_lookaxis2, cv_lookaxis2.defaultvalue);
			CV_StealthSet(&cv_fireaxis2, cv_fireaxis2.defaultvalue);
			CV_StealthSet(&cv_driftaxis2, cv_driftaxis2.defaultvalue);
			break;
		case 1:
		default:
			CV_StealthSet(&cv_usejoystick, cv_usejoystick.defaultvalue);
			CV_StealthSet(&cv_turnaxis, cv_turnaxis.defaultvalue);
			CV_StealthSet(&cv_moveaxis, cv_moveaxis.defaultvalue);
			CV_StealthSet(&cv_brakeaxis, cv_brakeaxis.defaultvalue);
			CV_StealthSet(&cv_aimaxis, cv_aimaxis.defaultvalue);
			CV_StealthSet(&cv_lookaxis, cv_lookaxis.defaultvalue);
			CV_StealthSet(&cv_fireaxis, cv_fireaxis.defaultvalue);
			CV_StealthSet(&cv_driftaxis, cv_driftaxis.defaultvalue);
			break;
	}

	S_StartSound(NULL, sfx_s224);
}

static void M_ResetControls(INT32 choice)
{
	(void)choice;
	M_StartMessage(va(M_GetText("Reset Player %d's controls to defaults?\n\n(Press 'Y' to confirm)\n"), setupcontrolplayer), M_ResetControlsResponse, MM_YESNO);
}

// =====
// SOUND
// =====

/*static void M_RestartAudio(void)
{
	COM_ImmedExecute("restartaudio");
}*/

// ===============
// VIDEO MODE MENU
// ===============

//added : 30-01-98:
#define MAXCOLUMNMODES   12     //max modes displayed in one column
#define MAXMODEDESCS     (MAXCOLUMNMODES*3)

static modedesc_t modedescs[MAXMODEDESCS];

static void M_VideoModeMenu(INT32 choice)
{
	INT32 i, j, vdup, nummodes, width, height;
	const char *desc;

	(void)choice;

	memset(modedescs, 0, sizeof(modedescs));

#if (defined (__unix__) && !defined (MSDOS)) || defined (UNIXCOMMON) || defined (HAVE_SDL)
	VID_PrepareModeList(); // FIXME: hack
#endif
	vidm_nummodes = 0;
	vidm_selected = 0;
	nummodes = VID_NumModes();

#ifdef _WINDOWS
	// clean that later: skip windowed mode 0, video modes menu only shows FULL SCREEN modes
	if (nummodes <= NUMSPECIALMODES)
		i = 0; // unless we have nothing
	else
		i = NUMSPECIALMODES;
#else
	// DOS does not skip mode 0, because mode 0 is ALWAYS present
	i = 0;
#endif
	for (; i < nummodes && vidm_nummodes < MAXMODEDESCS; i++)
	{
		desc = VID_GetModeName(i);
		if (desc)
		{
			vdup = 0;

			// when a resolution exists both under VGA and VESA, keep the
			// VESA mode, which is always a higher modenum
			for (j = 0; j < vidm_nummodes; j++)
			{
				if (!strcmp(modedescs[j].desc, desc))
				{
					// mode(0): 320x200 is always standard VGA, not vesa
					if (modedescs[j].modenum)
					{
						modedescs[j].modenum = i;
						vdup = 1;

						if (i == vid.modenum)
							vidm_selected = j;
					}
					else
						vdup = 1;

					break;
				}
			}

			if (!vdup)
			{
				modedescs[vidm_nummodes].modenum = i;
				modedescs[vidm_nummodes].desc = desc;

				if (i == vid.modenum)
					vidm_selected = vidm_nummodes;

				// Pull out the width and height
				sscanf(desc, "%u%*c%u", &width, &height);

				// Show multiples of 320x200 as green.
				if (SCR_IsAspectCorrect(width, height))
					modedescs[vidm_nummodes].goodratio = 1;

				vidm_nummodes++;
			}
		}
	}

	vidm_column_size = (vidm_nummodes+2) / 3;

	M_SetupNextMenu(&OP_VideoModeDef);
}

static void M_DrawVideoMenu(void)
{
	M_DrawGenericMenu();

	V_DrawRightAlignedString(BASEVIDWIDTH - currentMenu->x, currentMenu->y + OP_VideoOptionsMenu[0].alphaKey,
		(SCR_IsAspectCorrect(vid.width, vid.height) ? recommendedflags : highlightflags),
			va("%dx%d", vid.width, vid.height));
}

static void M_DrawHUDOptions(void)
{
	const char *str0 = ")";
	const char *str1 = " Warning highlight";
	const char *str2 = ",";
	const char *str3 = "Good highlight";
	INT32 x = BASEVIDWIDTH - currentMenu->x + 2, y = currentMenu->y + 105;
	INT32 w0 = V_StringWidth(str0, 0), w1 = V_StringWidth(str1, 0), w2 = V_StringWidth(str2, 0), w3 = V_StringWidth(str3, 0);

	M_DrawGenericMenu();

	x -= w0;
	V_DrawString(x, y, highlightflags, str0);
	x -= w1;
	V_DrawString(x, y, warningflags, str1);
	x -= w2;
	V_DrawString(x, y, highlightflags, str2);
	x -= w3;
	V_DrawString(x, y, recommendedflags, str3);
	V_DrawRightAlignedString(x, y, highlightflags, "(");
}

// Draw the video modes list, a-la-Quake
static void M_DrawVideoMode(void)
{
	INT32 i, j, row, col;

	// draw title
	M_DrawMenuTitle();

	V_DrawCenteredString(BASEVIDWIDTH/2, OP_VideoModeDef.y,
		highlightflags, "Choose mode, reselect to change default");

	row = 41;
	col = OP_VideoModeDef.y + 14;
	for (i = 0; i < vidm_nummodes; i++)
	{
		if (i == vidm_selected)
			V_DrawString(row, col, highlightflags, modedescs[i].desc);
		// Show multiples of 320x200 as green.
		else
			V_DrawString(row, col, (modedescs[i].goodratio) ? recommendedflags : 0, modedescs[i].desc);

		col += 8;
		if ((i % vidm_column_size) == (vidm_column_size-1))
		{
			row += 7*13;
			col = OP_VideoModeDef.y + 14;
		}
	}

	if (vidm_testingmode > 0)
	{
		INT32 testtime = (vidm_testingmode/TICRATE) + 1;

		M_CentreText(OP_VideoModeDef.y + 116,
			va("Previewing mode %c%dx%d",
				(SCR_IsAspectCorrect(vid.width, vid.height)) ? 0x83 : 0x80,
				vid.width, vid.height));
		M_CentreText(OP_VideoModeDef.y + 138,
			"Press ENTER again to keep this mode");
		M_CentreText(OP_VideoModeDef.y + 150,
			va("Wait %d second%s", testtime, (testtime > 1) ? "s" : ""));
		M_CentreText(OP_VideoModeDef.y + 158,
			"or press ESC to return");

	}
	else
	{
		M_CentreText(OP_VideoModeDef.y + 116,
			va("Current mode is %c%dx%d",
				(SCR_IsAspectCorrect(vid.width, vid.height)) ? 0x83 : 0x80,
				vid.width, vid.height));
		M_CentreText(OP_VideoModeDef.y + 124,
			va("Default mode is %c%dx%d",
				(SCR_IsAspectCorrect(cv_scr_width.value, cv_scr_height.value)) ? 0x83 : 0x80,
				cv_scr_width.value, cv_scr_height.value));

		V_DrawCenteredString(BASEVIDWIDTH/2, OP_VideoModeDef.y + 138,
			recommendedflags, "Marked modes are recommended.");
		V_DrawCenteredString(BASEVIDWIDTH/2, OP_VideoModeDef.y + 146,
			highlightflags, "Other modes may have visual errors.");
		V_DrawCenteredString(BASEVIDWIDTH/2, OP_VideoModeDef.y + 158,
			highlightflags, "Larger modes may have performance issues.");
	}

	// Draw the cursor for the VidMode menu
	i = 41 - 10 + ((vidm_selected / vidm_column_size)*7*13);
	j = OP_VideoModeDef.y + 14 + ((vidm_selected % vidm_column_size)*8);

	V_DrawScaledPatch(i - 8, j, 0,
		W_CachePatchName("M_CURSOR", PU_CACHE));
}

// special menuitem key handler for video mode list
static void M_HandleVideoMode(INT32 ch)
{
	if (vidm_testingmode > 0) switch (ch)
	{
		// change back to the previous mode quickly
		case KEY_ESCAPE:
			setmodeneeded = vidm_previousmode + 1;
			vidm_testingmode = 0;
			break;

		case KEY_ENTER:
			S_StartSound(NULL, sfx_menu1);
			vidm_testingmode = 0; // stop testing
	}

	else switch (ch)
	{
		case KEY_DOWNARROW:
			S_StartSound(NULL, sfx_menu1);
			if (++vidm_selected >= vidm_nummodes)
				vidm_selected = 0;
			break;

		case KEY_UPARROW:
			S_StartSound(NULL, sfx_menu1);
			if (--vidm_selected < 0)
				vidm_selected = vidm_nummodes - 1;
			break;

		case KEY_LEFTARROW:
			S_StartSound(NULL, sfx_menu1);
			vidm_selected -= vidm_column_size;
			if (vidm_selected < 0)
				vidm_selected = (vidm_column_size*3) + vidm_selected;
			if (vidm_selected >= vidm_nummodes)
				vidm_selected = vidm_nummodes - 1;
			break;

		case KEY_RIGHTARROW:
			S_StartSound(NULL, sfx_menu1);
			vidm_selected += vidm_column_size;
			if (vidm_selected >= (vidm_column_size*3))
				vidm_selected %= vidm_column_size;
			if (vidm_selected >= vidm_nummodes)
				vidm_selected = vidm_nummodes - 1;
			break;

		case KEY_ENTER:
			S_StartSound(NULL, sfx_menu1);
			if (vid.modenum == modedescs[vidm_selected].modenum)
				SCR_SetDefaultMode();
			else
			{
				vidm_testingmode = 15*TICRATE;
				vidm_previousmode = vid.modenum;
				if (!setmodeneeded) // in case the previous setmode was not finished
					setmodeneeded = modedescs[vidm_selected].modenum + 1;
			}
			break;

		case KEY_ESCAPE: // this one same as M_Responder
			if (currentMenu->prevMenu)
				M_SetupNextMenu(currentMenu->prevMenu);
			else
				M_ClearMenus(true);
			break;

		default:
			break;
	}
}

// ===============
// Monitor Toggles
// ===============
static consvar_t *kartitemcvs[NUMKARTRESULTS-1] = {
	&cv_sneaker,
	&cv_rocketsneaker,
	&cv_invincibility,
	&cv_banana,
	&cv_eggmanmonitor,
	&cv_orbinaut,
	&cv_jawz,
	&cv_mine,
	&cv_ballhog,
	&cv_selfpropelledbomb,
	&cv_grow,
	&cv_shrink,
	&cv_thundershield,
	&cv_hyudoro,
	&cv_pogospring,
	&cv_kitchensink,
	&cv_triplesneaker,
	&cv_triplebanana,
	&cv_decabanana,
	&cv_tripleorbinaut,
	&cv_quadorbinaut,
	&cv_dualjawz
};

static tic_t shitsfree = 0;

static void M_DrawMonitorToggles(void)
{
	const INT32 edges = 4;
	const INT32 height = 4;
	const INT32 spacing = 35;
	const INT32 column = itemOn/height;
	//const INT32 row = itemOn%height;
	INT32 leftdraw, rightdraw, totaldraw;
	INT32 x = currentMenu->x, y = currentMenu->y+(spacing/4);
	INT32 onx = 0, ony = 0;
	consvar_t *cv;
	INT32 i, translucent, drawnum;

	M_DrawMenuTitle();

	// Find the available space around column
	leftdraw = rightdraw = column;
	totaldraw = 0;
	for (i = 0; (totaldraw < edges*2 && i < edges*4); i++)
	{
		if (rightdraw+1 < (currentMenu->numitems/height)+1)
		{
			rightdraw++;
			totaldraw++;
		}
		if (leftdraw-1 >= 0)
		{
			leftdraw--;
			totaldraw++;
		}
	}

	for (i = leftdraw; i <= rightdraw; i++)
	{
		INT32 j;

		for (j = 0; j < height; j++)
		{
			const INT32 thisitem = (i*height)+j;

			if (thisitem >= currentMenu->numitems)
				continue;

			if (thisitem == itemOn)
			{
				onx = x;
				ony = y;
				y += spacing;
				continue;
			}

#ifdef ITEMTOGGLEBOTTOMRIGHT
			if (currentMenu->menuitems[thisitem].alphaKey == 255)
			{
				V_DrawScaledPatch(x, y, V_TRANSLUCENT, W_CachePatchName("K_ISBG", PU_CACHE));
				continue;
			}
#endif
			if (currentMenu->menuitems[thisitem].alphaKey == 0)
			{
				V_DrawScaledPatch(x, y, 0, W_CachePatchName("K_ISBG", PU_CACHE));
				V_DrawScaledPatch(x, y, 0, W_CachePatchName("K_ISTOGL", PU_CACHE));
				continue;
			}

			cv = kartitemcvs[currentMenu->menuitems[thisitem].alphaKey-1];
			translucent = (cv->value ? 0 : V_TRANSLUCENT);

			switch (currentMenu->menuitems[thisitem].alphaKey)
			{
				case KRITEM_DUALJAWZ:
					drawnum = 2;
					break;
				case KRITEM_TRIPLESNEAKER:
				case KRITEM_TRIPLEBANANA:
				case KRITEM_TRIPLEORBINAUT:
					drawnum = 3;
					break;
				case KRITEM_QUADORBINAUT:
					drawnum = 4;
					break;
				case KRITEM_TENFOLDBANANA:
					drawnum = 10;
					break;
				default:
					drawnum = 0;
					break;
			}

			if (cv->value)
				V_DrawScaledPatch(x, y, 0, W_CachePatchName("K_ISBG", PU_CACHE));
			else
				V_DrawScaledPatch(x, y, 0, W_CachePatchName("K_ISBGD", PU_CACHE));

			if (drawnum != 0)
			{
				V_DrawScaledPatch(x, y, 0, W_CachePatchName("K_ISMUL", PU_CACHE));
				V_DrawScaledPatch(x, y, translucent, W_CachePatchName(K_GetItemPatch(currentMenu->menuitems[thisitem].alphaKey, true), PU_CACHE));
				V_DrawString(x+24, y+31, V_ALLOWLOWERCASE|translucent, va("x%d", drawnum));
			}
			else
				V_DrawScaledPatch(x, y, translucent, W_CachePatchName(K_GetItemPatch(currentMenu->menuitems[thisitem].alphaKey, true), PU_CACHE));

			y += spacing;
		}

		x += spacing;
		y = currentMenu->y+(spacing/4);
	}

	{
#ifdef ITEMTOGGLEBOTTOMRIGHT
		if (currentMenu->menuitems[itemOn].alphaKey == 255)
		{
			V_DrawScaledPatch(onx-1, ony-2, V_TRANSLUCENT, W_CachePatchName("K_ITBG", PU_CACHE));
			if (shitsfree)
			{
				INT32 trans = V_TRANSLUCENT;
				if (shitsfree-1 > TICRATE-5)
					trans = ((10-TICRATE)+shitsfree-1)<<V_ALPHASHIFT;
				else if (shitsfree < 5)
					trans = (10-shitsfree)<<V_ALPHASHIFT;
				V_DrawScaledPatch(onx-1, ony-2, trans, W_CachePatchName("K_ITFREE", PU_CACHE));
			}
		}
		else
#endif
		if (currentMenu->menuitems[itemOn].alphaKey == 0)
		{
			V_DrawScaledPatch(onx-1, ony-2, 0, W_CachePatchName("K_ITBG", PU_CACHE));
			V_DrawScaledPatch(onx-1, ony-2, 0, W_CachePatchName("K_ITTOGL", PU_CACHE));
		}
		else
		{
			cv = kartitemcvs[currentMenu->menuitems[itemOn].alphaKey-1];
			translucent = (cv->value ? 0 : V_TRANSLUCENT);

			switch (currentMenu->menuitems[itemOn].alphaKey)
			{
				case KRITEM_DUALJAWZ:
					drawnum = 2;
					break;
				case KRITEM_TRIPLESNEAKER:
				case KRITEM_TRIPLEBANANA:
					drawnum = 3;
					break;
				case KRITEM_TENFOLDBANANA:
					drawnum = 10;
					break;
				default:
					drawnum = 0;
					break;
			}

			if (cv->value)
				V_DrawScaledPatch(onx-1, ony-2, 0, W_CachePatchName("K_ITBG", PU_CACHE));
			else
				V_DrawScaledPatch(onx-1, ony-2, 0, W_CachePatchName("K_ITBGD", PU_CACHE));

			if (drawnum != 0)
			{
				V_DrawScaledPatch(onx-1, ony-2, 0, W_CachePatchName("K_ITMUL", PU_CACHE));
				V_DrawScaledPatch(onx-1, ony-2, translucent, W_CachePatchName(K_GetItemPatch(currentMenu->menuitems[itemOn].alphaKey, false), PU_CACHE));
				V_DrawScaledPatch(onx+27, ony+39, translucent, W_CachePatchName("K_ITX", PU_CACHE));
				V_DrawKartString(onx+37, ony+34, translucent, va("%d", drawnum));
			}
			else
				V_DrawScaledPatch(onx-1, ony-2, translucent, W_CachePatchName(K_GetItemPatch(currentMenu->menuitems[itemOn].alphaKey, false), PU_CACHE));
		}
	}

	if (shitsfree)
		shitsfree--;

	V_DrawCenteredString(BASEVIDWIDTH/2, currentMenu->y, highlightflags, va("* %s *", currentMenu->menuitems[itemOn].text));
}

static void M_HandleMonitorToggles(INT32 choice)
{
	const INT32 width = 6, height = 4;
	INT32 column = itemOn/height, row = itemOn%height;
	INT16 next;
	UINT8 i;
	boolean exitmenu = false;

	switch (choice)
	{
		case KEY_RIGHTARROW:
			S_StartSound(NULL, sfx_menu1);
			column++;
			if (((column*height)+row) >= currentMenu->numitems)
				column = 0;
			next = min(((column*height)+row), currentMenu->numitems-1);
			itemOn = next;
			break;

		case KEY_LEFTARROW:
			S_StartSound(NULL, sfx_menu1);
			column--;
			if (column < 0)
				column = width-1;
			if (((column*height)+row) >= currentMenu->numitems)
				column--;
			next = max(((column*height)+row), 0);
			if (next >= currentMenu->numitems)
				next = currentMenu->numitems-1;
			itemOn = next;
			break;

		case KEY_DOWNARROW:
			S_StartSound(NULL, sfx_menu1);
			row = (row+1) % height;
			if (((column*height)+row) >= currentMenu->numitems)
				row = 0;
			next = min(((column*height)+row), currentMenu->numitems-1);
			itemOn = next;
			break;

		case KEY_UPARROW:
			S_StartSound(NULL, sfx_menu1);
			row = (row-1) % height;
			if (row < 0)
				row = height-1;
			if (((column*height)+row) >= currentMenu->numitems)
				row--;
			next = max(((column*height)+row), 0);
			if (next >= currentMenu->numitems)
				next = currentMenu->numitems-1;
			itemOn = next;
			break;

		case KEY_ENTER:
#ifdef ITEMTOGGLEBOTTOMRIGHT
			if (currentMenu->menuitems[itemOn].alphaKey == 255)
			{
				//S_StartSound(NULL, sfx_s26d);
				if (!shitsfree)
				{
					shitsfree = TICRATE;
					S_StartSound(NULL, sfx_itfree);
				}
			}
			else
#endif
			if (currentMenu->menuitems[itemOn].alphaKey == 0)
			{
				INT32 v = cv_sneaker.value;
				S_StartSound(NULL, sfx_s1b4);
				for (i = 0; i < NUMKARTRESULTS-1; i++)
				{
					if (kartitemcvs[i]->value == v)
						CV_AddValue(kartitemcvs[i], 1);
				}
			}
			else
			{
				S_StartSound(NULL, sfx_s1ba);
				CV_AddValue(kartitemcvs[currentMenu->menuitems[itemOn].alphaKey-1], 1);
			}
			break;

		case KEY_ESCAPE:
			exitmenu = true;
			break;
	}

	if (exitmenu)
	{
		if (currentMenu->prevMenu)
			M_SetupNextMenu(currentMenu->prevMenu);
		else
			M_ClearMenus(true);
	}
}

// =========
// Quit Game
// =========
static INT32 quitsounds[] =
{
	// holy shit we're changing things up!
	// srb2kart: you ain't seen nothing yet
	sfx_kc2e,
	sfx_kc2f,
	sfx_cdfm01,
	sfx_ddash,
	sfx_s3ka2,
	sfx_s3k49,
	sfx_slip,
	sfx_tossed,
	sfx_s3k7b,
	sfx_itrolf,
	sfx_itrole,
	sfx_cdpcm9,
	sfx_s3k4e,
	sfx_s259,
	sfx_3db06,
	sfx_s3k3a,
	sfx_peel,
	sfx_cdfm28,
	sfx_s3k96,
	sfx_s3kc0s,
	sfx_cdfm39,
	sfx_hogbom,
	sfx_kc5a,
	sfx_kc46,
	sfx_s3k92,
	sfx_s3k42,
	sfx_kpogos,
	sfx_screec
};

void M_QuitResponse(INT32 ch)
{
	tic_t ptime;
	INT32 mrand;

	if (ch != 'y' && ch != KEY_ENTER)
		return;
	if (!(netgame || cv_debug))
	{
		mrand = M_RandomKey(sizeof(quitsounds)/sizeof(INT32));
		if (quitsounds[mrand]) S_StartSound(NULL, quitsounds[mrand]);

		//added : 12-02-98: do that instead of I_WaitVbl which does not work
		ptime = I_GetTime() + NEWTICRATE*2; // Shortened the quit time, used to be 2 seconds Tails 03-26-2001
		while (ptime > I_GetTime())
		{
			V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31);
			V_DrawSmallScaledPatch(0, 0, 0, W_CachePatchName("GAMEQUIT", PU_CACHE)); // Demo 3 Quit Screen Tails 06-16-2001
			I_FinishUpdate(); // Update the screen with the image Tails 06-19-2001
			I_Sleep();
		}
	}
	I_Quit();
}

static void M_QuitSRB2(INT32 choice)
{
	// We pick index 0 which is language sensitive, or one at random,
	// between 1 and maximum number.
	(void)choice;
	M_StartMessage(quitmsg[M_RandomKey(NUM_QUITMESSAGES)], M_QuitResponse, MM_YESNO);
}

#ifdef HWRENDER
// =====================================================================
// OpenGL specific options
// =====================================================================

// =====================
// M_OGL_DrawColorMenu()
// =====================
static void M_OGL_DrawColorMenu(void)
{
	INT32 mx, my;

	mx = currentMenu->x;
	my = currentMenu->y;
	M_DrawGenericMenu(); // use generic drawer for cursor, items and title
	V_DrawString(mx, my + currentMenu->menuitems[0].alphaKey - 10,
		highlightflags, "Gamma correction");
}
#endif

#ifdef HAVE_DISCORDRPC
static const tic_t confirmLength = 3*TICRATE/4;
static tic_t confirmDelay = 0;
static boolean confirmAccept = false;

static void M_HandleDiscordRequests(INT32 choice)
{
	if (confirmDelay > 0)
		return;

	switch (choice)
	{
		case KEY_ENTER:
			Discord_Respond(discordRequestList->userID, DISCORD_REPLY_YES);
			confirmAccept = true;
			confirmDelay = confirmLength;
			S_StartSound(NULL, sfx_s3k63);
			break;

		case KEY_ESCAPE:
			Discord_Respond(discordRequestList->userID, DISCORD_REPLY_NO);
			confirmAccept = false;
			confirmDelay = confirmLength;
			S_StartSound(NULL, sfx_s3kb2);
			break;
	}
}

static const char *M_GetDiscordName(discordRequest_t *r)
{
	if (r == NULL)
		return "";

	if (cv_discordstreamer.value)
		return r->username;

	return va("%s#%s", r->username, r->discriminator);
}

// (this goes in k_hud.c when merged into v2)
static void M_DrawSticker(INT32 x, INT32 y, INT32 width, INT32 flags, boolean small)
{
	patch_t *stickerEnd;
	INT32 height;
	
	if (small == true)
	{
		stickerEnd = W_CachePatchName("K_STIKE2", PU_CACHE);
		height = 6;
	}
	else
	{
		stickerEnd = W_CachePatchName("K_STIKEN", PU_CACHE);
		height = 11;
	}

	V_DrawFixedPatch(x*FRACUNIT, y*FRACUNIT, FRACUNIT, flags, stickerEnd, NULL);
	V_DrawFill(x, y, width, height, 24|flags);
	V_DrawFixedPatch((x + width)*FRACUNIT, y*FRACUNIT, FRACUNIT, flags|V_FLIP, stickerEnd, NULL);
}

static void M_DrawDiscordRequests(void)
{
	discordRequest_t *curRequest = discordRequestList;
	UINT8 *colormap;
	patch_t *hand = NULL;
	boolean removeRequest = false;

	const char *wantText = "...would like to join!";
	const char *controlText = "\x82" "ENTER" "\x80" " - Accept    " "\x82" "ESC" "\x80" " - Decline";

	INT32 x = 100;
	INT32 y = 133;

	INT32 slide = 0;
	INT32 maxYSlide = 18;

	if (confirmDelay > 0)
	{
		if (confirmAccept == true)
		{
			colormap = R_GetTranslationColormap(TC_DEFAULT, SKINCOLOR_GREEN, GTC_MENUCACHE);
			hand = W_CachePatchName("K_LAPH02", PU_CACHE);
		}
		else
		{
			colormap = R_GetTranslationColormap(TC_DEFAULT, SKINCOLOR_RED, GTC_MENUCACHE);
			hand = W_CachePatchName("K_LAPH03", PU_CACHE);
		}

		slide = confirmLength - confirmDelay;

		confirmDelay--;

		if (confirmDelay == 0)
			removeRequest = true;
	}
	else
	{
		colormap = R_GetTranslationColormap(TC_DEFAULT, SKINCOLOR_GREY, GTC_MENUCACHE);
	}

	V_DrawFixedPatch(56*FRACUNIT, 150*FRACUNIT, FRACUNIT, 0, W_CachePatchName("K_LAPE01", PU_CACHE), colormap);

	if (hand != NULL)
	{
		fixed_t handoffset = (4 - abs((signed)(skullAnimCounter - 4))) * FRACUNIT;
		V_DrawFixedPatch(56*FRACUNIT, 150*FRACUNIT + handoffset, FRACUNIT, 0, hand, NULL);
	}

	M_DrawSticker(x + (slide * 32), y - 1, V_ThinStringWidth(M_GetDiscordName(curRequest), V_ALLOWLOWERCASE|V_6WIDTHSPACE), 0, false);
	V_DrawThinString(x + (slide * 32), y, V_ALLOWLOWERCASE|V_6WIDTHSPACE|V_YELLOWMAP, M_GetDiscordName(curRequest));

	M_DrawSticker(x, y + 12, V_ThinStringWidth(wantText, V_ALLOWLOWERCASE|V_6WIDTHSPACE), 0, true);
	V_DrawThinString(x, y + 10, V_ALLOWLOWERCASE|V_6WIDTHSPACE, wantText);

	M_DrawSticker(x, y + 26, V_ThinStringWidth(controlText, V_ALLOWLOWERCASE|V_6WIDTHSPACE), 0, true);
	V_DrawThinString(x, y + 24, V_ALLOWLOWERCASE|V_6WIDTHSPACE, controlText);

	y -= 18;

	while (curRequest->next != NULL)
	{
		INT32 ySlide = min(slide * 4, maxYSlide);

		curRequest = curRequest->next;

		M_DrawSticker(x, y - 1 + ySlide, V_ThinStringWidth(M_GetDiscordName(curRequest), V_ALLOWLOWERCASE|V_6WIDTHSPACE), 0, false);
		V_DrawThinString(x, y + ySlide, V_ALLOWLOWERCASE|V_6WIDTHSPACE, M_GetDiscordName(curRequest));

		y -= 12;
		maxYSlide = 12;
	}

	if (removeRequest == true)
	{
		DRPC_RemoveRequest(discordRequestList);

		if (discordRequestList == NULL)
		{
			// No other requests
			MPauseMenu[mpause_discordrequests].status = IT_GRAYEDOUT;

			if (currentMenu->prevMenu)
			{
				M_SetupNextMenu(currentMenu->prevMenu);
				if (currentMenu == &MPauseDef)
					itemOn = mpause_continue;
			}
			else
				M_ClearMenus(true);

			return;
		}
	}
}
#endif
