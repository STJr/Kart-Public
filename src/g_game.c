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
/// \file  g_game.c
/// \brief game loop functions, events handling

#include "doomdef.h"
#include "console.h"
#include "d_main.h"
#include "d_player.h"
#include "f_finale.h"
#include "p_setup.h"
#include "p_saveg.h"
#include "i_system.h"
#include "am_map.h"
#include "m_random.h"
#include "p_local.h"
#include "r_draw.h"
#include "r_main.h"
#include "s_sound.h"
#include "g_game.h"
#include "m_cheat.h"
#include "m_misc.h"
#include "m_menu.h"
#include "m_argv.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "z_zone.h"
#include "i_video.h"
#include "byteptr.h"
#include "i_joy.h"
#include "r_local.h"
#include "r_things.h"
#include "y_inter.h"
#include "v_video.h"
#include "dehacked.h" // get_number (for ghost thok)
#include "lua_hook.h"
#include "b_bot.h"
#include "m_cond.h" // condition sets
#include "md5.h" // demo checksums
#include "k_kart.h" // SRB2kart

gameaction_t gameaction;
gamestate_t gamestate = GS_NULL;
UINT8 ultimatemode = false;

boolean botingame;
UINT8 botskin;
UINT8 botcolor;

JoyType_t Joystick;
JoyType_t Joystick2;
JoyType_t Joystick3;
JoyType_t Joystick4;

// 1024 bytes is plenty for a savegame
#define SAVEGAMESIZE (1024)

// SRB2kart
char gamedatafilename[64] = "kartdata.dat";
char timeattackfolder[64] = "kart";
char customversionstring[32] = "\0";

static void G_DoCompleted(void);
static void G_DoStartContinue(void);
static void G_DoContinued(void);
static void G_DoWorldDone(void);
static void G_DoStartVote(void);

char   mapmusname[7]; // Music name
UINT16 mapmusflags; // Track and reset bit

INT16 gamemap = 1;
INT16 maptol;
UINT8 globalweather = 0;
INT32 curWeather = PRECIP_NONE;
INT32 cursaveslot = -1; // Auto-save 1p savegame slot
INT16 lastmapsaved = 0; // Last map we auto-saved at
boolean gamecomplete = false;

UINT16 mainwads = 0;
boolean modifiedgame; // Set if homebrew PWAD stuff has been added.
boolean savemoddata = false;
UINT8 paused;
UINT8 modeattacking = ATTACKING_NONE;
boolean disableSpeedAdjust = true;
boolean imcontinuing = false;
boolean runemeraldmanager = false;

// menu demo things
UINT8  numDemos      = 0; //3; -- i'm FED UP of losing my skincolour to a broken demo. change this back when we make new ones
UINT32 demoDelayTime = 15*TICRATE;
UINT32 demoIdleTime  = 3*TICRATE;

boolean timingdemo; // if true, exit with report on completion
boolean nodrawers; // for comparative timing purposes
boolean noblit; // for comparative timing purposes
static tic_t demostarttime; // for comparative timing purposes

boolean netgame; // only true if packets are broadcast
boolean multiplayer;
boolean playeringame[MAXPLAYERS];
boolean addedtogame;
player_t players[MAXPLAYERS];

INT32 consoleplayer; // player taking events and displaying
INT32 displayplayer; // view being displayed
INT32 secondarydisplayplayer; // for splitscreen
INT32 thirddisplayplayer;
INT32 fourthdisplayplayer;

tic_t gametic;
tic_t levelstarttic; // gametic at level start
UINT32 totalrings; // for intermission
INT16 lastmap; // last level you were at (returning from special stages)
tic_t timeinmap; // Ticker for time spent in level (used for levelcard display)

INT16 spstage_start;
INT16 sstage_start;
INT16 sstage_end;

boolean looptitle = true;
boolean useNightsSS = false;

UINT8 skincolor_redteam = SKINCOLOR_RED;
UINT8 skincolor_blueteam = SKINCOLOR_BLUE;
UINT8 skincolor_redring = SKINCOLOR_RED;
UINT8 skincolor_bluering = SKINCOLOR_STEEL;

tic_t countdowntimer = 0;
boolean countdowntimeup = false;

cutscene_t *cutscenes[128];

INT16 nextmapoverride;
boolean skipstats;

// Pointers to each CTF flag
mobj_t *redflag;
mobj_t *blueflag;
// Pointers to CTF spawn location
mapthing_t *rflagpoint;
mapthing_t *bflagpoint;

struct quake quake;

// Map Header Information
mapheader_t* mapheaderinfo[NUMMAPS] = {NULL};

static boolean exitgame = false;
static boolean retrying = false;

UINT8 stagefailed; // Used for GEMS BONUS? Also to see if you beat the stage.

UINT16 emeralds;
UINT32 token; // Number of tokens collected in a level
UINT32 tokenlist; // List of tokens collected
INT32 tokenbits; // Used for setting token bits

// Old Special Stage
INT32 sstimer; // Time allotted in the special stage

tic_t totalplaytime;
UINT32 matchesplayed; // SRB2Kart
boolean gamedataloaded = false;

// Time attack data for levels
// These are dynamically allocated for space reasons now
recorddata_t *mainrecords[NUMMAPS]   = {NULL};
//nightsdata_t *nightsrecords[NUMMAPS] = {NULL};
UINT8 mapvisited[NUMMAPS];

// Temporary holding place for nights data for the current map
//nightsdata_t ntemprecords;

UINT32 bluescore, redscore; // CTF and Team Match team scores

// ring count... for PERFECT!
INT32 nummaprings = 0;

// box respawning in battle mode
INT32 nummapboxes = 0;
INT32 numgotboxes = 0;

// Elminates unnecessary searching.
boolean CheckForBustableBlocks;
boolean CheckForBouncySector;
boolean CheckForQuicksand;
boolean CheckForMarioBlocks;
boolean CheckForFloatBob;
boolean CheckForReverseGravity;

// Powerup durations
UINT16 invulntics = 20*TICRATE;
UINT16 sneakertics = 20*TICRATE;
UINT16 flashingtics = 3*TICRATE/2; // SRB2kart
UINT16 tailsflytics = 8*TICRATE;
UINT16 underwatertics = 30*TICRATE;
UINT16 spacetimetics = 11*TICRATE + (TICRATE/2);
UINT16 extralifetics = 4*TICRATE;

// SRB2kart
tic_t introtime = 108+5; // plus 5 for white fade
tic_t starttime = 6*TICRATE + (3*TICRATE/4);
tic_t raceexittime = 5*TICRATE + (2*TICRATE/3);
tic_t battleexittime = 8*TICRATE;
INT32 hyudorotime = 7*TICRATE;
INT32 stealtime = TICRATE/2;
INT32 sneakertime = TICRATE + (TICRATE/3);
INT32 itemtime = 8*TICRATE;
INT32 comebacktime = 10*TICRATE;
INT32 bumptime = 6;
INT32 wipeoutslowtime = 20;
INT32 wantedreduce = 5*TICRATE;
INT32 wantedfrequency = 10*TICRATE;

INT32 gameovertics = 15*TICRATE;

UINT8 use1upSound = 0;
UINT8 maxXtraLife = 2; // Max extra lives from rings

UINT8 introtoplay;
UINT8 creditscutscene;

// Emerald locations
mobj_t *hunt1;
mobj_t *hunt2;
mobj_t *hunt3;

UINT32 countdown, countdown2; // for racing

fixed_t gravity;

INT16 autobalance; //for CTF team balance
INT16 teamscramble; //for CTF team scramble
INT16 scrambleplayers[MAXPLAYERS]; //for CTF team scramble
INT16 scrambleteams[MAXPLAYERS]; //for CTF team scramble
INT16 scrambletotal; //for CTF team scramble
INT16 scramblecount; //for CTF team scramble

INT32 cheats; //for multiplayer cheat commands

// SRB2Kart
// Cvars that we don't want changed mid-game
UINT8 gamespeed; // Game's current speed (or difficulty, or cc, or etc); 0 for easy, 1 for normal, 2 for hard
boolean encoremode = false; // Encore Mode currently enabled?
boolean prevencoremode;
boolean franticitems; // Frantic items currently enabled?
boolean comeback; // Battle Mode's karma comeback is on/off

// Voting system
INT16 votelevels[5][2]; // Levels that were rolled by the host
SINT8 votes[MAXPLAYERS]; // Each player's vote
SINT8 pickedvote; // What vote the host rolls

// Server-sided, synched variables
SINT8 battlewanted[4]; // WANTED players in battle, worth x2 points
tic_t wantedcalcdelay; // Time before it recalculates WANTED
tic_t indirectitemcooldown; // Cooldown before any more Shrink, SPB, or any other item that works indirectly is awarded
tic_t spbincoming; // Timer before SPB hits, can switch targets at this point
UINT8 spbplayer; // Player num that used the last SPB
tic_t mapreset; // Map reset delay when enough players have joined an empty game

// Client-sided, unsynched variables (NEVER use in anything that needs to be synced with other players)
boolean legitimateexit; // Did this client actually finish the match?
boolean comebackshowninfo; // Have you already seen the "ATTACK OR PROTECT" message?
tic_t curlap; // Current lap time
tic_t bestlap; // Best lap time
static INT16 randmapbuffer[NUMMAPS+1]; // Buffer for maps RandMap is allowed to roll

tic_t hidetime;

// Grading
UINT32 timesBeaten;
UINT32 timesBeatenWithEmeralds;
//UINT32 timesBeatenUltimate;

static char demoname[64];
boolean demorecording;
boolean demoplayback;
boolean titledemo; // Title Screen demo can be cancelled by any key
static UINT8 *demobuffer = NULL;
static UINT8 *demo_p, *demotime_p;
static UINT8 *demoend;
static UINT8 demoflags;
static UINT16 demoversion;
boolean singledemo; // quit after playing a demo from cmdline
boolean demo_start; // don't start playing demo right away
static boolean demosynced = true; // console warning message

boolean metalrecording; // recording as metal sonic
mobj_t *metalplayback;
static UINT8 *metalbuffer = NULL;
static UINT8 *metal_p;
static UINT16 metalversion;

// extra data stuff (events registered this frame while recording)
static struct {
	UINT8 flags; // EZT flags

	// EZT_COLOR
	UINT8 color, lastcolor;

	// EZT_SCALE
	fixed_t scale, lastscale;

	// EZT_HIT
	UINT16 hits;
	mobj_t **hitlist;
} ghostext;

// Your naming conventions are stupid and useless.
// There is no conflict here.
demoghost *ghosts = NULL;

boolean precache = true; // if true, load all graphics at start

INT16 prevmap, nextmap;

static UINT8 *savebuffer;

// Analog Control
static void UserAnalog_OnChange(void);
static void UserAnalog2_OnChange(void);
static void UserAnalog3_OnChange(void);
static void UserAnalog4_OnChange(void);
static void Analog_OnChange(void);
static void Analog2_OnChange(void);
static void Analog3_OnChange(void);
static void Analog4_OnChange(void);
void SendWeaponPref(void);
void SendWeaponPref2(void);
void SendWeaponPref3(void);
void SendWeaponPref4(void);

static CV_PossibleValue_t crosshair_cons_t[] = {{0, "Off"}, {1, "Cross"}, {2, "Angle"}, {3, "Point"}, {0, NULL}};
static CV_PossibleValue_t joyaxis_cons_t[] = {{0, "None"},
#ifdef _WII
{1, "LStick.X"}, {2, "LStick.Y"}, {-1, "LStick.X-"}, {-2, "LStick.Y-"},
#if JOYAXISSET > 1
{3, "RStick.X"}, {4, "RStick.Y"}, {-3, "RStick.X-"}, {-4, "RStick.Y-"},
#endif
#if JOYAXISSET > 2
{5, "RTrigger"}, {6, "LTrigger"}, {-5, "RTrigger-"}, {-6, "LTrigger-"},
#endif
#if JOYAXISSET > 3
{7, "Pitch"}, {8, "Roll"}, {-7, "Pitch-"}, {-8, "Roll-"},
#endif
#if JOYAXISSET > 4
{7, "Yaw"}, {8, "Dummy"}, {-7, "Yaw-"}, {-8, "Dummy-"},
#endif
#if JOYAXISSET > 4
{9, "LAnalog"}, {10, "RAnalog"}, {-9, "LAnalog-"}, {-10, "RAnalog-"},
#endif
#elif defined (WMINPUT)
{1, "LStick.X"}, {2, "LStick.Y"}, {-1, "LStick.X-"}, {-2, "LStick.Y-"},
#if JOYAXISSET > 1
{3, "RStick.X"}, {4, "RStick.Y"}, {-3, "RStick.X-"}, {-4, "RStick.Y-"},
#endif
#if JOYAXISSET > 2
{5, "NStick.X"}, {6, "NStick.Y"}, {-5, "NStick.X-"}, {-6, "NStick.Y-"},
#endif
#if JOYAXISSET > 3
{7, "LAnalog"}, {8, "RAnalog"}, {-7, "LAnalog-"}, {-8, "RAnalog-"},
#endif
#else
{1, "X-Axis"}, {2, "Y-Axis"}, {-1, "X-Axis-"}, {-2, "Y-Axis-"},
#ifdef _arch_dreamcast
{3, "R-Trig"}, {4, "L-Trig"}, {-3, "R-Trig-"}, {-4, "L-Trig-"},
{5, "Alt X-Axis"}, {6, "Alt Y-Axis"}, {-5, "Alt X-Axis-"}, {-6, "Alt Y-Axis-"},
{7, "Triggers"}, {-7,"Triggers-"},
#elif defined (_XBOX)
{3, "Alt X-Axis"}, {4, "Alt Y-Axis"}, {-3, "Alt X-Axis-"}, {-4, "Alt Y-Axis-"},
#else
#if JOYAXISSET > 1
{3, "Z-Axis"}, {4, "X-Rudder"}, {-3, "Z-Axis-"}, {-4, "X-Rudder-"},
#endif
#if JOYAXISSET > 2
{5, "Y-Rudder"}, {6, "Z-Rudder"}, {-5, "Y-Rudder-"}, {-6, "Z-Rudder-"},
#endif
#if JOYAXISSET > 3
{7, "U-Axis"}, {8, "V-Axis"}, {-7, "U-Axis-"}, {-8, "V-Axis-"},
#endif
#endif
#endif
 {0, NULL}};
#ifdef _WII
#if JOYAXISSET > 5
"More Axis Sets"
#endif
#else
#if JOYAXISSET > 4
"More Axis Sets"
#endif
#endif

// don't mind me putting these here, I was lazy to figure out where else I could put those without blowing up the compiler.

// chat timer thingy
static CV_PossibleValue_t chattime_cons_t[] = {{5, "MIN"}, {999, "MAX"}, {0, NULL}};
consvar_t cv_chattime = {"chattime", "8", CV_SAVE, chattime_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

// chatwidth
static CV_PossibleValue_t chatwidth_cons_t[] = {{64, "MIN"}, {150, "MAX"}, {0, NULL}};
consvar_t cv_chatwidth = {"chatwidth", "150", CV_SAVE, chatwidth_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

// chatheight
static CV_PossibleValue_t chatheight_cons_t[] = {{6, "MIN"}, {22, "MAX"}, {0, NULL}};
consvar_t cv_chatheight = {"chatheight", "8", CV_SAVE, chatheight_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

// chat notifications (do you want to hear beeps? I'd understand if you didn't.)
consvar_t cv_chatnotifications = {"chatnotifications", "On", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

// chat spam protection (why would you want to disable that???)
consvar_t cv_chatspamprotection = {"chatspamprotection", "On", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

// minichat text background
consvar_t cv_chatbacktint = {"chatbacktint", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

// old shit console chat. (mostly exists for stuff like terminal, not because I cared if anyone liked the old chat.)
static CV_PossibleValue_t consolechat_cons_t[] = {{0, "Window"}, {1, "Console"}, {0, NULL}};
consvar_t cv_consolechat = {"chatmode", "Window", CV_SAVE, consolechat_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_crosshair = {"crosshair", "Cross", CV_SAVE, crosshair_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_crosshair2 = {"crosshair2", "Cross", CV_SAVE, crosshair_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_crosshair3 = {"crosshair3", "Cross", CV_SAVE, crosshair_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_crosshair4 = {"crosshair4", "Cross", CV_SAVE, crosshair_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_invertmouse = {"invertmouse", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_alwaysfreelook = {"alwaysmlook", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_invertmouse2 = {"invertmouse2", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_alwaysfreelook2 = {"alwaysmlook2", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_mousemove = {"mousemove", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_mousemove2 = {"mousemove2", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_analog = {"analog", "Off", CV_CALL, CV_OnOff, Analog_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_analog2 = {"analog2", "Off", CV_CALL, CV_OnOff, Analog2_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_analog3 = {"analog3", "Off", CV_CALL, CV_OnOff, Analog3_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_analog4 = {"analog4", "Off", CV_CALL, CV_OnOff, Analog4_OnChange, 0, NULL, NULL, 0, 0, NULL};
#ifdef DC
consvar_t cv_useranalog = {"useranalog", "On", CV_SAVE|CV_CALL, CV_OnOff, UserAnalog_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_useranalog2 = {"useranalog2", "On", CV_SAVE|CV_CALL, CV_OnOff, UserAnalog2_OnChange, 0, NULL, NULL, 0, 0, NULL};
#else
consvar_t cv_useranalog = {"useranalog", "Off", CV_SAVE|CV_CALL, CV_OnOff, UserAnalog_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_useranalog2 = {"useranalog2", "Off", CV_SAVE|CV_CALL, CV_OnOff, UserAnalog2_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_useranalog3 = {"useranalog3", "Off", CV_SAVE|CV_CALL, CV_OnOff, UserAnalog3_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_useranalog4 = {"useranalog4", "Off", CV_SAVE|CV_CALL, CV_OnOff, UserAnalog4_OnChange, 0, NULL, NULL, 0, 0, NULL};
#endif

consvar_t cv_turnaxis = {"joyaxis_turn", "X-Axis", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_moveaxis = {"joyaxis_move", "None", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_brakeaxis = {"joyaxis_brake", "None", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_aimaxis = {"joyaxis_aim", "Y-Axis", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_lookaxis = {"joyaxis_look", "None", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_fireaxis = {"joyaxis_fire", "Z-Axis", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_driftaxis = {"joyaxis_drift", "Z-Axis-", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_turnaxis2 = {"joyaxis2_turn", "X-Axis", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_moveaxis2 = {"joyaxis2_move", "None", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_brakeaxis2 = {"joyaxis2_brake", "None", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_aimaxis2 = {"joyaxis2_aim", "Y-Axis", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_lookaxis2 = {"joyaxis2_look", "None", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_fireaxis2 = {"joyaxis2_fire", "Z-Axis", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_driftaxis2 = {"joyaxis2_drift", "Z-Axis-", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_turnaxis3 = {"joyaxis3_turn", "X-Axis", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_moveaxis3 = {"joyaxis3_move", "None", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_brakeaxis3 = {"joyaxis3_brake", "None", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_aimaxis3 = {"joyaxis3_aim", "Y-Axis", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_lookaxis3 = {"joyaxis3_look", "None", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_fireaxis3 = {"joyaxis3_fire", "Z-Axis", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_driftaxis3 = {"joyaxis3_drift", "Z-Axis-", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_turnaxis4 = {"joyaxis4_turn", "X-Axis", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_moveaxis4 = {"joyaxis4_move", "None", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_brakeaxis4 = {"joyaxis4_brake", "None", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_aimaxis4 = {"joyaxis4_aim", "Y-Axis", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_lookaxis4 = {"joyaxis4_look", "None", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_fireaxis4 = {"joyaxis4_fire", "Z-Axis", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_driftaxis4 = {"joyaxis4_drift", "Z-Axis-", CV_SAVE, joyaxis_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};


#if MAXPLAYERS > 16
#error "please update player_name table using the new value for MAXPLAYERS"
#endif

#ifdef SEENAMES
player_t *seenplayer; // player we're aiming at right now
#endif

char player_names[MAXPLAYERS][MAXPLAYERNAME+1] =
{
	"Player 1",
	"Player 2",
	"Player 3",
	"Player 4",
	"Player 5",
	"Player 6",
	"Player 7",
	"Player 8",
	"Player 9",
	"Player 10",
	"Player 11",
	"Player 12",
	"Player 13",
	"Player 14",
	"Player 15",
	"Player 16"
}; // SRB2kart - removed Players 17 through 32

INT16 rw_maximums[NUM_WEAPONS] =
{
	800, // MAX_INFINITY
	400, // MAX_AUTOMATIC
	100, // MAX_BOUNCE
	50,  // MAX_SCATTER
	100, // MAX_GRENADE
	50,  // MAX_EXPLOSION
	50   // MAX_RAIL
};

// Allocation for time and nights data
void G_AllocMainRecordData(INT16 i)
{
	if (!mainrecords[i])
		mainrecords[i] = Z_Malloc(sizeof(recorddata_t), PU_STATIC, NULL);
	memset(mainrecords[i], 0, sizeof(recorddata_t));
}

/*void G_AllocNightsRecordData(INT16 i)
{
	if (!nightsrecords[i])
		nightsrecords[i] = Z_Malloc(sizeof(nightsdata_t), PU_STATIC, NULL);
	memset(nightsrecords[i], 0, sizeof(nightsdata_t));
}*/

// MAKE SURE YOU SAVE DATA BEFORE CALLING THIS
void G_ClearRecords(void)
{
	INT16 i;
	for (i = 0; i < NUMMAPS; ++i)
	{
		if (mainrecords[i])
		{
			Z_Free(mainrecords[i]);
			mainrecords[i] = NULL;
		}
		/*if (nightsrecords[i])
		{
			Z_Free(nightsrecords[i]);
			nightsrecords[i] = NULL;
		}*/
	}
}

// For easy retrieval of records
/*UINT32 G_GetBestScore(INT16 map)
{
	if (!mainrecords[map-1])
		return 0;

	return mainrecords[map-1]->score;
}*/

tic_t G_GetBestTime(INT16 map)
{
	if (!mainrecords[map-1] || mainrecords[map-1]->time <= 0)
		return (tic_t)UINT32_MAX;

	return mainrecords[map-1]->time;
}

// Not needed
/*tic_t G_GetBestLap(INT16 map)
{
	if (!mainrecords[map-1] || mainrecords[map-1]->lap <= 0)
		return (tic_t)UINT32_MAX;

	return mainrecords[map-1]->lap;
}*/

/*UINT16 G_GetBestRings(INT16 map)
{
	if (!mainrecords[map-1])
		return 0;

	return mainrecords[map-1]->rings;
}*/

// No NiGHTS records for SRB2Kart
/*UINT32 G_GetBestNightsScore(INT16 map, UINT8 mare)
{
	if (!nightsrecords[map-1])
		return 0;

	return nightsrecords[map-1]->score[mare];
}

tic_t G_GetBestNightsTime(INT16 map, UINT8 mare)
{
	if (!nightsrecords[map-1] || nightsrecords[map-1]->time[mare] <= 0)
		return (tic_t)UINT32_MAX;

	return nightsrecords[map-1]->time[mare];
}

UINT8 G_GetBestNightsGrade(INT16 map, UINT8 mare)
{
	if (!nightsrecords[map-1])
		return 0;

	return nightsrecords[map-1]->grade[mare];
}

// For easy adding of NiGHTS records
void G_AddTempNightsRecords(UINT32 pscore, tic_t ptime, UINT8 mare)
{
	ntemprecords.score[mare] = pscore;
	ntemprecords.grade[mare] = P_GetGrade(pscore, gamemap, mare - 1);
	ntemprecords.time[mare] = ptime;

	// Update nummares
	// Note that mare "0" is overall, mare "1" is the first real mare
	if (ntemprecords.nummares < mare)
		ntemprecords.nummares = mare;
}

void G_SetNightsRecords(void)
{
	INT32 i;
	UINT32 totalscore = 0;
	tic_t totaltime = 0;

	const size_t glen = strlen(srb2home)+1+strlen("replay")+1+strlen(timeattackfolder)+1+strlen("MAPXX")+1;
	char *gpath;
	char lastdemo[256], bestdemo[256];

	if (!ntemprecords.nummares)
		return;

	// Set overall
	{
		UINT8 totalrank = 0, realrank = 0;

		for (i = 1; i <= ntemprecords.nummares; ++i)
		{
			totalscore += ntemprecords.score[i];
			totalrank += ntemprecords.grade[i];
			totaltime += ntemprecords.time[i];
		}

		// Determine overall grade
		realrank = (UINT8)((FixedDiv((fixed_t)totalrank << FRACBITS, ntemprecords.nummares << FRACBITS) + (FRACUNIT/2)) >> FRACBITS);

		// You need ALL rainbow As to get a rainbow A overall
		if (realrank == GRADE_S && (totalrank / ntemprecords.nummares) != GRADE_S)
			realrank = GRADE_A;

		ntemprecords.score[0] = totalscore;
		ntemprecords.grade[0] = realrank;
		ntemprecords.time[0] = totaltime;
	}

	// Now take all temp records and put them in the actual records
	{
		nightsdata_t *maprecords;

		if (!nightsrecords[gamemap-1])
			G_AllocNightsRecordData(gamemap-1);
		maprecords = nightsrecords[gamemap-1];

		if (maprecords->nummares != ntemprecords.nummares)
			maprecords->nummares = ntemprecords.nummares;

		for (i = 0; i < ntemprecords.nummares + 1; ++i)
		{
			if (maprecords->score[i] < ntemprecords.score[i])
				maprecords->score[i] = ntemprecords.score[i];
			if (maprecords->grade[i] < ntemprecords.grade[i])
				maprecords->grade[i] = ntemprecords.grade[i];
			if (!maprecords->time[i] || maprecords->time[i] > ntemprecords.time[i])
				maprecords->time[i] = ntemprecords.time[i];
		}
	}

	memset(&ntemprecords, 0, sizeof(nightsdata_t));

	// Save demo!
	bestdemo[255] = '\0';
	lastdemo[255] = '\0';
	G_SetDemoTime(totaltime, totalscore);
	G_CheckDemoStatus();

	I_mkdir(va("%s"PATHSEP"replay", srb2home), 0755);
	I_mkdir(va("%s"PATHSEP"replay"PATHSEP"%s", srb2home, timeattackfolder), 0755);

	if ((gpath = malloc(glen)) == NULL)
		I_Error("Out of memory for replay filepath\n");

	sprintf(gpath,"%s"PATHSEP"replay"PATHSEP"%s"PATHSEP"%s", srb2home, timeattackfolder, G_BuildMapName(gamemap));
	snprintf(lastdemo, 255, "%s-last.lmp", gpath);

	if (FIL_FileExists(lastdemo))
	{
		UINT8 *buf;
		size_t len = FIL_ReadFile(lastdemo, &buf);

		snprintf(bestdemo, 255, "%s-time-best.lmp", gpath);
		if (!FIL_FileExists(bestdemo) || G_CmpDemoTime(bestdemo, lastdemo) & 1)
		{ // Better time, save this demo.
			if (FIL_FileExists(bestdemo))
				remove(bestdemo);
			FIL_WriteFile(bestdemo, buf, len);
			CONS_Printf("\x83%s\x80 %s '%s'\n", M_GetText("NEW RECORD TIME!"), M_GetText("Saved replay as"), bestdemo);
		}

		snprintf(bestdemo, 255, "%s-score-best.lmp", gpath);
		if (!FIL_FileExists(bestdemo) || (G_CmpDemoTime(bestdemo, lastdemo) & (1<<1)))
		{ // Better score, save this demo.
			if (FIL_FileExists(bestdemo))
				remove(bestdemo);
			FIL_WriteFile(bestdemo, buf, len);
			CONS_Printf("\x83%s\x80 %s '%s'\n", M_GetText("NEW HIGH SCORE!"), M_GetText("Saved replay as"), bestdemo);
		}

		//CONS_Printf("%s '%s'\n", M_GetText("Saved replay as"), lastdemo);

		Z_Free(buf);
	}
	free(gpath);

	// If the mare count changed, this will update the score display
	CV_AddValue(&cv_nextmap, 1);
	CV_AddValue(&cv_nextmap, -1);
}*/

// for consistency among messages: this modifies the game and removes savemoddata.
void G_SetGameModified(boolean silent)
{
	if (modifiedgame && !savemoddata)
		return;

	modifiedgame = true;
	savemoddata = false;

	if (!silent)
		CONS_Alert(CONS_NOTICE, M_GetText("Game must be restarted to record statistics.\n"));

	// If in record attack recording, cancel it.
	if (modeattacking)
		M_EndModeAttackRun();
}

/** Builds an original game map name from a map number.
  * The complexity is due to MAPA0-MAPZZ.
  *
  * \param map Map number.
  * \return Pointer to a static buffer containing the desired map name.
  * \sa M_MapNumber
  */
const char *G_BuildMapName(INT32 map)
{
	static char mapname[10] = "MAPXX"; // internal map name (wad resource name)

	I_Assert(map >= 0);
	I_Assert(map <= NUMMAPS);

	if (map == 0) // hack???
	{
		if (gamestate == GS_TITLESCREEN)
			map = -1;
		else if (gamestate == GS_LEVEL)
			map = gamemap-1;
		else
			map = prevmap;
		map = G_RandMap(G_TOLFlag(cv_newgametype.value), map, false, false, 0, false)+1;
	}

	if (map < 100)
		sprintf(&mapname[3], "%.2d", map);
	else
	{
		mapname[3] = (char)('A' + (char)((map - 100) / 36));
		if ((map - 100) % 36 < 10)
			mapname[4] = (char)('0' + (char)((map - 100) % 36));
		else
			mapname[4] = (char)('A' + (char)((map - 100) % 36) - 10);
		mapname[5] = '\0';
	}

	return mapname;
}

/** Clips the console player's mouse aiming to the current view.
  * Used whenever the player view is changed manually.
  *
  * \param aiming Pointer to the vertical angle to clip.
  * \return Short version of the clipped angle for building a ticcmd.
  */
INT16 G_ClipAimingPitch(INT32 *aiming)
{
	INT32 limitangle;

	limitangle = ANGLE_90 - 1;

	if (*aiming > limitangle)
		*aiming = limitangle;
	else if (*aiming < -limitangle)
		*aiming = -limitangle;

	return (INT16)((*aiming)>>16);
}

INT16 G_SoftwareClipAimingPitch(INT32 *aiming)
{
	INT32 limitangle;

	// note: the current software mode implementation doesn't have true perspective
	limitangle = ANGLE_90 - ANG10; // Some viewing fun, but not too far down...

	if (*aiming > limitangle)
		*aiming = limitangle;
	else if (*aiming < -limitangle)
		*aiming = -limitangle;

	return (INT16)((*aiming)>>16);
}

static INT32 Joy1Axis(axis_input_e axissel)
{
	INT32 retaxis;
	INT32 axisval;
	boolean flp = false;

	//find what axis to get
	switch (axissel)
	{
		case AXISTURN:
			axisval = cv_turnaxis.value;
			break;
		case AXISMOVE:
			axisval = cv_moveaxis.value;
			break;
		case AXISBRAKE:
			axisval = cv_brakeaxis.value;
			break;
		case AXISAIM:
			axisval = cv_aimaxis.value;
			break;
		case AXISLOOK:
			axisval = cv_lookaxis.value;
			break;
		case AXISFIRE:
			axisval = cv_fireaxis.value;
			break;
		case AXISDRIFT:
			axisval = cv_driftaxis.value;
			break;
		default:
			return 0;
	}

	if (axisval < 0) //odd -axises
	{
		axisval = -axisval;
		flp = true;
	}
#ifdef _arch_dreamcast
	if (axisval == 7) // special case
	{
		retaxis = joyxmove[1] - joyymove[1];
		goto skipDC;
	}
	else
#endif
	if (axisval > JOYAXISSET*2 || axisval == 0) //not there in array or None
		return 0;

	if (axisval%2)
	{
		axisval /= 2;
		retaxis = joyxmove[axisval];
	}
	else
	{
		axisval--;
		axisval /= 2;
		retaxis = joyymove[axisval];
	}

#ifdef _arch_dreamcast
	skipDC:
#endif

	if (retaxis < (-JOYAXISRANGE))
		retaxis = -JOYAXISRANGE;
	if (retaxis > (+JOYAXISRANGE))
		retaxis = +JOYAXISRANGE;
	if (!Joystick.bGamepadStyle && axissel < AXISDEAD)
	{
		const INT32 jdeadzone = JOYAXISRANGE/4;
		if (-jdeadzone < retaxis && retaxis < jdeadzone)
			return 0;
	}
	if (flp) retaxis = -retaxis; //flip it around
	return retaxis;
}

static INT32 Joy2Axis(axis_input_e axissel)
{
	INT32 retaxis;
	INT32 axisval;
	boolean flp = false;

	//find what axis to get
	switch (axissel)
	{
		case AXISTURN:
			axisval = cv_turnaxis2.value;
			break;
		case AXISMOVE:
			axisval = cv_moveaxis2.value;
			break;
		case AXISBRAKE:
			axisval = cv_brakeaxis2.value;
			break;
		case AXISAIM:
			axisval = cv_aimaxis2.value;
			break;
		case AXISLOOK:
			axisval = cv_lookaxis2.value;
			break;
		case AXISFIRE:
			axisval = cv_fireaxis2.value;
			break;
		case AXISDRIFT:
			axisval = cv_driftaxis2.value;
			break;
		default:
			return 0;
	}


	if (axisval < 0) //odd -axises
	{
		axisval = -axisval;
		flp = true;
	}
#ifdef _arch_dreamcast
	if (axisval == 7) // special case
	{
		retaxis = joy2xmove[1] - joy2ymove[1];
		goto skipDC;
	}
	else
#endif
	if (axisval > JOYAXISSET*2 || axisval == 0) //not there in array or None
		return 0;

	if (axisval%2)
	{
		axisval /= 2;
		retaxis = joy2xmove[axisval];
	}
	else
	{
		axisval--;
		axisval /= 2;
		retaxis = joy2ymove[axisval];
	}

#ifdef _arch_dreamcast
	skipDC:
#endif

	if (retaxis < (-JOYAXISRANGE))
		retaxis = -JOYAXISRANGE;
	if (retaxis > (+JOYAXISRANGE))
		retaxis = +JOYAXISRANGE;
	if (!Joystick2.bGamepadStyle && axissel < AXISDEAD)
	{
		const INT32 jdeadzone = JOYAXISRANGE/4;
		if (-jdeadzone < retaxis && retaxis < jdeadzone)
			return 0;
	}
	if (flp) retaxis = -retaxis; //flip it around
	return retaxis;
}

static INT32 Joy3Axis(axis_input_e axissel)
{
	INT32 retaxis;
	INT32 axisval;
	boolean flp = false;

	//find what axis to get
	switch (axissel)
	{
		case AXISTURN:
			axisval = cv_turnaxis3.value;
			break;
		case AXISMOVE:
			axisval = cv_moveaxis3.value;
			break;
		case AXISBRAKE:
			axisval = cv_brakeaxis3.value;
			break;
		case AXISAIM:
			axisval = cv_aimaxis3.value;
			break;
		case AXISLOOK:
			axisval = cv_lookaxis3.value;
			break;
		case AXISFIRE:
			axisval = cv_fireaxis3.value;
			break;
		case AXISDRIFT:
			axisval = cv_driftaxis3.value;
			break;
		default:
			return 0;
	}


	if (axisval < 0) //odd -axises
	{
		axisval = -axisval;
		flp = true;
	}
#ifdef _arch_dreamcast
	if (axisval == 7) // special case
	{
		retaxis = joy3xmove[1] - joy3ymove[1];
		goto skipDC;
	}
	else
#endif
	if (axisval > JOYAXISSET*2 || axisval == 0) //not there in array or None
		return 0;

	if (axisval%2)
	{
		axisval /= 2;
		retaxis = joy3xmove[axisval];
	}
	else
	{
		axisval--;
		axisval /= 2;
		retaxis = joy3ymove[axisval];
	}

#ifdef _arch_dreamcast
	skipDC:
#endif

	if (retaxis < (-JOYAXISRANGE))
		retaxis = -JOYAXISRANGE;
	if (retaxis > (+JOYAXISRANGE))
		retaxis = +JOYAXISRANGE;
	if (!Joystick3.bGamepadStyle && axissel < AXISDEAD)
	{
		const INT32 jdeadzone = JOYAXISRANGE/4;
		if (-jdeadzone < retaxis && retaxis < jdeadzone)
			return 0;
	}
	if (flp) retaxis = -retaxis; //flip it around
	return retaxis;
}

static INT32 Joy4Axis(axis_input_e axissel)
{
	INT32 retaxis;
	INT32 axisval;
	boolean flp = false;

	//find what axis to get
	switch (axissel)
	{
		case AXISTURN:
			axisval = cv_turnaxis4.value;
			break;
		case AXISMOVE:
			axisval = cv_moveaxis4.value;
			break;
		case AXISBRAKE:
			axisval = cv_brakeaxis4.value;
			break;
		case AXISAIM:
			axisval = cv_aimaxis4.value;
			break;
		case AXISLOOK:
			axisval = cv_lookaxis4.value;
			break;
		case AXISFIRE:
			axisval = cv_fireaxis4.value;
			break;
		case AXISDRIFT:
			axisval = cv_driftaxis4.value;
			break;
		default:
			return 0;
	}


	if (axisval < 0) //odd -axises
	{
		axisval = -axisval;
		flp = true;
	}
#ifdef _arch_dreamcast
	if (axisval == 7) // special case
	{
		retaxis = joy4xmove[1] - joy4ymove[1];
		goto skipDC;
	}
	else
#endif
	if (axisval > JOYAXISSET*2 || axisval == 0) //not there in array or None
		return 0;

	if (axisval%2)
	{
		axisval /= 2;
		retaxis = joy4xmove[axisval];
	}
	else
	{
		axisval--;
		axisval /= 2;
		retaxis = joy4ymove[axisval];
	}

#ifdef _arch_dreamcast
	skipDC:
#endif

	if (retaxis < (-JOYAXISRANGE))
		retaxis = -JOYAXISRANGE;
	if (retaxis > (+JOYAXISRANGE))
		retaxis = +JOYAXISRANGE;
	if (!Joystick4.bGamepadStyle && axissel < AXISDEAD)
	{
		const INT32 jdeadzone = JOYAXISRANGE/4;
		if (-jdeadzone < retaxis && retaxis < jdeadzone)
			return 0;
	}
	if (flp) retaxis = -retaxis; //flip it around
	return retaxis;
}

boolean InputDown(INT32 gc, UINT8 p)
{
	switch (p)
	{
		case 2:
			return PLAYER2INPUTDOWN(gc);
		case 3:
			return PLAYER3INPUTDOWN(gc);
		case 4:
			return PLAYER4INPUTDOWN(gc);
		default:
			return PLAYER1INPUTDOWN(gc);
	}
}

INT32 JoyAxis(axis_input_e axissel, UINT8 p)
{
	switch (p)
	{
		case 2:
			return Joy2Axis(axissel);
		case 3:
			return Joy3Axis(axissel);
		case 4:
			return Joy4Axis(axissel);
		default:
			return Joy1Axis(axissel);
	}
}

//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs
// or reads it from the demo buffer.
// If recording a demo, write it out
//
// set secondaryplayer true to build player 2's ticcmd in splitscreen mode
//
INT32 localaiming, localaiming2, localaiming3, localaiming4;
angle_t localangle, localangle2, localangle3, localangle4;
boolean camspin, camspin2, camspin3, camspin4;

static fixed_t forwardmove[2] = {25<<FRACBITS>>16, 50<<FRACBITS>>16};
static fixed_t sidemove[2] = {2<<FRACBITS>>16, 4<<FRACBITS>>16};
static fixed_t angleturn[3] = {400, 800, 200}; // + slow turn

void G_BuildTiccmd(ticcmd_t *cmd, INT32 realtics, UINT8 ssplayer)
{
	INT32 laim, th, tspeed, forward, side, axis; //i
	const INT32 speed = 1;
	// these ones used for multiple conditions
	boolean turnleft, turnright, invertmouse, mouseaiming, lookaxis, usejoystick, analogjoystickmove, gamepadjoystickmove, kbl, rd;
	player_t *player;
	camera_t *thiscam;
	angle_t lang;

	static INT32 turnheld, turnheld2, turnheld3, turnheld4; // for accelerative turning
	static boolean keyboard_look, keyboard_look2, keyboard_look3, keyboard_look4; // true if lookup/down using keyboard
	static boolean resetdown, resetdown2, resetdown3, resetdown4; // don't cam reset every frame

	switch (ssplayer)
	{
		case 2:
			player = &players[secondarydisplayplayer];
			thiscam = (player->bot == 2 ? &camera : &camera2);
			lang = localangle2;
			laim = localaiming2;
			th = turnheld2;
			kbl = keyboard_look2;
			rd = resetdown2;
			G_CopyTiccmd(cmd, I_BaseTiccmd2(), 1);
			break;
		case 3:
			player = &players[thirddisplayplayer];
			thiscam = &camera3;
			lang = localangle3;
			laim = localaiming3;
			th = turnheld3;
			kbl = keyboard_look3;
			rd = resetdown3;
			G_CopyTiccmd(cmd, I_BaseTiccmd3(), 1);
			break;
		case 4:
			player = &players[fourthdisplayplayer];
			thiscam = &camera4;
			lang = localangle4;
			laim = localaiming4;
			th = turnheld4;
			kbl = keyboard_look4;
			rd = resetdown4;
			G_CopyTiccmd(cmd, I_BaseTiccmd4(), 1);
			break;
		case 1:
		default:
			player = &players[consoleplayer];
			thiscam = &camera;
			lang = localangle;
			laim = localaiming;
			th = turnheld;
			kbl = keyboard_look;
			rd = resetdown;
			G_CopyTiccmd(cmd, I_BaseTiccmd(), 1); // empty, or external driver
			break;
	}

	// why build a ticcmd if we're paused?
	// Or, for that matter, if we're being reborn.
	// Kart, don't build a ticcmd if someone is resynching or the server is stopped too so we don't fly off course in bad conditions
	if (paused || P_AutoPause() || (gamestate == GS_LEVEL && player->playerstate == PST_REBORN) || hu_resynching)
	{
		cmd->angleturn = (INT16)(lang >> 16);
		cmd->aiming = G_ClipAimingPitch(&laim);
		return;
	}

	switch (ssplayer)
	{
		case 2:
			mouseaiming = player->spectator; //(PLAYER2INPUTDOWN(gc_mouseaiming)) ^ cv_alwaysfreelook2.value;
			invertmouse = cv_invertmouse2.value;
			lookaxis = cv_lookaxis2.value;
			analogjoystickmove = cv_usejoystick2.value && !Joystick2.bGamepadStyle;
			gamepadjoystickmove = cv_usejoystick2.value && Joystick2.bGamepadStyle;
			break;
		case 3:
			mouseaiming = false;
			invertmouse = false;
			lookaxis = cv_lookaxis3.value;
			analogjoystickmove = cv_usejoystick3.value && !Joystick3.bGamepadStyle;
			gamepadjoystickmove = cv_usejoystick3.value && Joystick3.bGamepadStyle;
			break;
		case 4:
			mouseaiming = false;
			invertmouse = false;
			lookaxis = cv_lookaxis4.value;
			analogjoystickmove = cv_usejoystick4.value && !Joystick4.bGamepadStyle;
			gamepadjoystickmove = cv_usejoystick4.value && Joystick4.bGamepadStyle;
			break;
		case 1:
		default:
			mouseaiming = player->spectator; //(PLAYER1INPUTDOWN(gc_mouseaiming)) ^ cv_alwaysfreelook.value;
			invertmouse = cv_invertmouse.value;
			lookaxis = cv_lookaxis.value;
			analogjoystickmove = cv_usejoystick.value && !Joystick.bGamepadStyle;
			gamepadjoystickmove = cv_usejoystick.value && Joystick.bGamepadStyle;
			break;
	}

	usejoystick = (analogjoystickmove || gamepadjoystickmove);
	turnright = InputDown(gc_turnright, ssplayer);
	turnleft = InputDown(gc_turnleft, ssplayer);

	axis = JoyAxis(AXISTURN, ssplayer);

	if (encoremode)
	{
		turnright ^= turnleft; // swap these using three XORs
		turnleft ^= turnright;
		turnright ^= turnleft;
		axis = -axis;
	}

	if (gamepadjoystickmove && axis != 0)
	{
		turnright = turnright || (axis > 0);
		turnleft = turnleft || (axis < 0);
	}
	forward = side = 0;

	// use two stage accelerative turning
	// on the keyboard and joystick
	if (turnleft || turnright)
		th += realtics;
	else
		th = 0;

	if (th < SLOWTURNTICS)
		tspeed = 2; // slow turn
	else
		tspeed = speed;

	cmd->driftturn = 0;

	// let movement keys cancel each other out
	if (turnright && !(turnleft))
	{
		cmd->angleturn = (INT16)(cmd->angleturn - angleturn[tspeed]);
		cmd->driftturn = (INT16)(cmd->driftturn - angleturn[tspeed]);
	}
	else if (turnleft && !(turnright))
	{
		cmd->angleturn = (INT16)(cmd->angleturn + angleturn[tspeed]);
		cmd->driftturn = (INT16)(cmd->driftturn + angleturn[tspeed]);
	}

	if (analogjoystickmove && axis != 0)
	{
		// JOYAXISRANGE should be 1023 (divide by 1024)
		cmd->angleturn = (INT16)(cmd->angleturn - ((axis * angleturn[1]) >> 10)); // ANALOG!
		cmd->driftturn = (INT16)(cmd->driftturn - ((axis * angleturn[1]) >> 10));
	}

	// Specator mouse turning
	if (player->spectator)
	{
		cmd->angleturn = (INT16)(cmd->angleturn - (mousex*(encoremode ? -1 : 1)*8));
		cmd->driftturn = (INT16)(cmd->driftturn - (mousex*(encoremode ? -1 : 1)*8));
	}

	// Speed bump strafing
	if (!demoplayback && ((player->pflags & PF_FORCESTRAFE) || (player->kartstuff[k_pogospring])))
	{
		if (turnright)
			side += sidemove[1];
		if (turnleft)
			side -= sidemove[1];
		if (analogjoystickmove && axis != 0)
		{
			// JOYAXISRANGE is supposed to be 1023 (divide by 1024)
			side += ((axis * sidemove[0]) >> 10);
		}
	}

	if (player->spectator || objectplacing) // SRB2Kart: spectators need special controls
	{
		if (InputDown(gc_accelerate, ssplayer) || (usejoystick && axis > 0))
			cmd->buttons |= BT_ACCELERATE;
		if (InputDown(gc_brake, ssplayer) || (usejoystick && axis > 0))
			cmd->buttons |= BT_BRAKE;
		axis = JoyAxis(AXISAIM, ssplayer);
		if (InputDown(gc_aimforward, ssplayer) || (usejoystick && axis < 0))
			forward += forwardmove[1];
		if (InputDown(gc_aimbackward, ssplayer) || (usejoystick && axis > 0))
			forward -= forwardmove[1];
	}
	else
	{
		// forward with key or button // SRB2kart - we use an accel/brake instead of forward/backward.
		axis = JoyAxis(AXISMOVE, ssplayer);
		if (InputDown(gc_accelerate, ssplayer) || (gamepadjoystickmove && axis > 0) || player->kartstuff[k_sneakertimer])
		{
			cmd->buttons |= BT_ACCELERATE;
			forward = forwardmove[1];	// 50
		}
		else if (analogjoystickmove && axis > 0)
		{
			cmd->buttons |= BT_ACCELERATE;
			// JOYAXISRANGE is supposed to be 1023 (divide by 1024)
			forward += ((axis * forwardmove[1]) >> 10)*2;
		}

		axis = JoyAxis(AXISBRAKE, ssplayer);
		if (InputDown(gc_brake, ssplayer) || (gamepadjoystickmove && axis > 0))
		{
			cmd->buttons |= BT_BRAKE;
			if (cmd->buttons & BT_ACCELERATE || cmd->forwardmove <= 0)
				forward -= forwardmove[0];	// 25 - Halved value so clutching is possible
		}
		else if (analogjoystickmove && axis > 0)
		{
			cmd->buttons |= BT_BRAKE;
			// JOYAXISRANGE is supposed to be 1023 (divide by 1024)
			if (cmd->buttons & BT_ACCELERATE || cmd->forwardmove <= 0)
				forward -= ((axis * forwardmove[0]) >> 10);
		}

		// But forward/backward IS used for aiming.
		axis = JoyAxis(AXISAIM, ssplayer);
		if (InputDown(gc_aimforward, ssplayer) || (usejoystick && axis < 0))
			cmd->buttons |= BT_FORWARD;
		if (InputDown(gc_aimbackward, ssplayer) || (usejoystick && axis > 0))
			cmd->buttons |= BT_BACKWARD;
	}

	// fire with any button/key
	axis = JoyAxis(AXISFIRE, ssplayer);
	if (InputDown(gc_fire, ssplayer) || (usejoystick && axis > 0))
		cmd->buttons |= BT_ATTACK;

	// drift with any button/key
	axis = JoyAxis(AXISDRIFT, ssplayer);
	if (InputDown(gc_drift, ssplayer) || (usejoystick && axis > 0))
		cmd->buttons |= BT_DRIFT;

	// Lua scriptable buttons
	if (InputDown(gc_custom1, ssplayer))
		cmd->buttons |= BT_CUSTOM1;
	if (InputDown(gc_custom2, ssplayer))
		cmd->buttons |= BT_CUSTOM2;
	if (InputDown(gc_custom3, ssplayer))
		cmd->buttons |= BT_CUSTOM3;

	// Reset camera
	if (InputDown(gc_camreset, ssplayer))
	{
		if (thiscam->chase && !rd)
			P_ResetCamera(player, thiscam);
		rd = true;
	}
	else
		rd = false;

	// spectator aiming shit, ahhhh...
	{
		INT32 player_invert = invertmouse ? -1 : 1;
		INT32 screen_invert =
			(player->mo && (player->mo->eflags & MFE_VERTICALFLIP)
			 && (!thiscam->chase || player->pflags & PF_FLIPCAM)) //because chasecam's not inverted
			 ? -1 : 1; // set to -1 or 1 to multiply

		// mouse look stuff (mouse look is not the same as mouse aim)
		if (mouseaiming && player->spectator)
		{
			kbl = false;

			// looking up/down
			laim += (mlooky<<19)*player_invert*screen_invert;
		}

		axis = JoyAxis(AXISLOOK, ssplayer);
		if (analogjoystickmove && axis != 0 && lookaxis && player->spectator)
			laim += (axis<<16) * screen_invert;

		// spring back if not using keyboard neither mouselookin'
		if (!kbl && !lookaxis && !mouseaiming)
			laim = 0;

		if (player->spectator)
		{
			if (InputDown(gc_lookup, ssplayer) || (gamepadjoystickmove && axis < 0))
			{
				laim += KB_LOOKSPEED * screen_invert;
				kbl = true;
			}
			else if (InputDown(gc_lookdown, ssplayer) || (gamepadjoystickmove && axis > 0))
			{
				laim -= KB_LOOKSPEED * screen_invert;
				kbl = true;
			}
		}

		if (InputDown(gc_centerview, ssplayer)) // No need to put a spectator limit on this one though :V
			laim = 0;

		// accept no mlook for network games
		if (!cv_allowmlook.value)
			laim = 0;

		cmd->aiming = G_ClipAimingPitch(&laim);
	}

	mousex = mousey = mlooky = 0;

	if (forward > MAXPLMOVE)
		forward = MAXPLMOVE;
	else if (forward < -MAXPLMOVE)
		forward = -MAXPLMOVE;

	if (side > MAXPLMOVE)
		side = MAXPLMOVE;
	else if (side < -MAXPLMOVE)
		side = -MAXPLMOVE;

	// No additional acceleration when moving forward/backward and strafing simultaneously.
	// do this AFTER we cap to MAXPLMOVE so people can't find ways to cheese around this.
	// SRB2Kart: We don't need this; we WANT bounce strafing to plain stack on top of normal movement.
	/*if (!bouncestrafe && forward && side)
	{
		forward = FixedMul(forward, 3*FRACUNIT/4);
		side = FixedMul(side, 3*FRACUNIT/4);
	}*/

	if (forward || side)
	{
		cmd->forwardmove = (SINT8)(cmd->forwardmove + forward);
		cmd->sidemove = (SINT8)(cmd->sidemove + side);
	}

	//{ SRB2kart - Drift support
	// Not grouped with the rest of turn stuff because it needs to know what buttons you're pressing for rubber-burn turn
	// limit turning to angleturn[1] to stop mouselook letting you look too fast
	if (cmd->angleturn > angleturn[1])
		cmd->angleturn = angleturn[1];
	else if (cmd->angleturn < -angleturn[1])
		cmd->angleturn = -angleturn[1];

	if (cmd->driftturn > angleturn[1])
		cmd->driftturn = angleturn[1];
	else if (cmd->driftturn < -angleturn[1])
		cmd->driftturn = -angleturn[1];

	if (player->mo)
		cmd->angleturn = K_GetKartTurnValue(player, cmd->angleturn);

	// SRB2kart - no additional angle if not moving
	if (((player->mo && player->speed > 0) // Moving
		|| (leveltime > starttime && (cmd->buttons & BT_ACCELERATE && cmd->buttons & BT_BRAKE)) // Rubber-burn turn
		|| (player->spectator || objectplacing)) // Not a physical player
		&& !(player->kartstuff[k_spinouttimer] && player->kartstuff[k_sneakertimer])) // Spinning and boosting cancels out spinout
		lang += (cmd->angleturn<<16);

	cmd->angleturn = (INT16)(lang >> 16);

	if (!hu_stopped)
	{
		switch (ssplayer)
		{
		case 2:
			localangle2 = lang;
			localaiming2 = laim;
			keyboard_look2 = kbl;
			turnheld2 = th;
			resetdown2 = rd;
			camspin2 = InputDown(gc_lookback, ssplayer);
			break;
		case 3:
			localangle3 = lang;
			localaiming3 = laim;
			keyboard_look3 = kbl;
			turnheld3 = th;
			resetdown3 = rd;
			camspin3 = InputDown(gc_lookback, ssplayer);
			break;
		case 4:
			localangle4 = lang;
			localaiming4 = laim;
			keyboard_look4 = kbl;
			turnheld4 = th;
			resetdown4 = rd;
			camspin4 = InputDown(gc_lookback, ssplayer);
			break;
		case 1:
		default:
			localangle = lang;
			localaiming = laim;
			keyboard_look = kbl;
			turnheld = th;
			resetdown = rd;
			camspin = InputDown(gc_lookback, ssplayer);
			break;
		}
	}

	//Reset away view if a command is given.
	if ((cmd->forwardmove || cmd->sidemove || cmd->buttons)
		&& displayplayer != consoleplayer && ssplayer == 1)
		displayplayer = consoleplayer;
}

// User has designated that they want
// analog ON, so tell the game to stop
// fudging with it.
static void UserAnalog_OnChange(void)
{
	/*if (cv_useranalog.value)
		CV_SetValue(&cv_analog, 1);
	else
		CV_SetValue(&cv_analog, 0);*/
}

static void UserAnalog2_OnChange(void)
{
	if (botingame)
		return;
	/*if (cv_useranalog2.value)
		CV_SetValue(&cv_analog2, 1);
	else
		CV_SetValue(&cv_analog2, 0);*/
}

static void UserAnalog3_OnChange(void)
{
	if (botingame)
		return;
	/*if (cv_useranalog3.value)
		CV_SetValue(&cv_analog3, 1);
	else
		CV_SetValue(&cv_analog3, 0);*/
}

static void UserAnalog4_OnChange(void)
{
	if (botingame)
		return;
	/*if (cv_useranalog4.value)
		CV_SetValue(&cv_analog4, 1);
	else
		CV_SetValue(&cv_analog4, 0);*/
}

static void Analog_OnChange(void)
{
	if (!cv_cam_dist.string)
		return;

	// cameras are not initialized at this point

	/*if (!cv_chasecam.value && cv_analog.value) {
		CV_SetValue(&cv_analog, 0);
		return;
	}*/

	if (cv_analog.value)
		players[consoleplayer].pflags |= PF_ANALOGMODE;
	else
		players[consoleplayer].pflags &= ~PF_ANALOGMODE;

	SendWeaponPref();
}

static void Analog2_OnChange(void)
{
	if (!(splitscreen || botingame) || !cv_cam2_dist.string)
		return;

	// cameras are not initialized at this point

	/*if (!cv_chasecam2.value && cv_analog2.value) {
		CV_SetValue(&cv_analog2, 0);
		return;
	}*/

	if (cv_analog2.value)
		players[secondarydisplayplayer].pflags |= PF_ANALOGMODE;
	else
		players[secondarydisplayplayer].pflags &= ~PF_ANALOGMODE;

	SendWeaponPref2();
}

static void Analog3_OnChange(void)
{
	if (splitscreen < 2 || !cv_cam3_dist.string)
		return;

	// cameras are not initialized at this point

	/*if (!cv_chasecam3.value && cv_analog3.value) {
		CV_SetValue(&cv_analog3, 0);
		return;
	}*/

	if (cv_analog3.value)
		players[thirddisplayplayer].pflags |= PF_ANALOGMODE;
	else
		players[thirddisplayplayer].pflags &= ~PF_ANALOGMODE;

	SendWeaponPref3();
}

static void Analog4_OnChange(void)
{
	if (splitscreen < 3 || !cv_cam4_dist.string)
		return;

	// cameras are not initialized at this point

	/*if (!cv_chasecam4.value && cv_analog4.value) {
		CV_SetValue(&cv_analog4, 0);
		return;
	}*/

	if (cv_analog4.value)
		players[fourthdisplayplayer].pflags |= PF_ANALOGMODE;
	else
		players[fourthdisplayplayer].pflags &= ~PF_ANALOGMODE;

	SendWeaponPref4();
}

//
// G_DoLoadLevel
//
void G_DoLoadLevel(boolean resetplayer)
{
	INT32 i;

	// Make sure objectplace is OFF when you first start the level!
	OP_ResetObjectplace();

	levelstarttic = gametic; // for time calculation

	if (wipegamestate == GS_LEVEL)
		wipegamestate = -1; // force a wipe

	if (gamestate == GS_INTERMISSION)
		Y_EndIntermission();
	if (gamestate == GS_VOTING)
		Y_EndVote();

	G_SetGamestate(GS_LEVEL);

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (resetplayer || (playeringame[i] && players[i].playerstate == PST_DEAD))
			players[i].playerstate = PST_REBORN;
	}

	// Setup the level.
	if (!P_SetupLevel(false))
	{
		// fail so reset game stuff
		Command_ExitGame_f();
		return;
	}

	if (!resetplayer)
		P_FindEmerald();

	displayplayer = consoleplayer; // view the guy you are playing
	if (!splitscreen && !botingame)
		secondarydisplayplayer = consoleplayer;
	if (splitscreen < 2)
		thirddisplayplayer = consoleplayer;
	if (splitscreen < 3)
		fourthdisplayplayer = consoleplayer;

	gameaction = ga_nothing;
#ifdef PARANOIA
	Z_CheckHeap(-2);
#endif

	if (camera.chase)
		P_ResetCamera(&players[displayplayer], &camera);
	if (camera2.chase && splitscreen)
		P_ResetCamera(&players[secondarydisplayplayer], &camera2);
	if (camera3.chase && splitscreen > 1)
		P_ResetCamera(&players[thirddisplayplayer], &camera3);
	if (camera4.chase && splitscreen > 2)
		P_ResetCamera(&players[fourthdisplayplayer], &camera4);

	// clear cmd building stuff
	memset(gamekeydown, 0, sizeof (gamekeydown));
	for (i = 0;i < JOYAXISSET; i++)
	{
		joyxmove[i] = joyymove[i] = 0;
		joy2xmove[i] = joy2ymove[i] = 0;
		joy3xmove[i] = joy3ymove[i] = 0;
		joy4xmove[i] = joy4ymove[i] = 0;
	}
	mousex = mousey = 0;
	mouse2x = mouse2y = 0;

	// clear hud messages remains (usually from game startup)
	CON_ClearHUD();
}

static INT32 pausedelay = 0;
static INT32 camtoggledelay, camtoggledelay2, camtoggledelay3, camtoggledelay4 = 0;
static INT32 spectatedelay, spectatedelay2, spectatedelay3, spectatedelay4 = 0;

//
// G_Responder
// Get info needed to make ticcmd_ts for the players.
//
boolean G_Responder(event_t *ev)
{
	// allow spy mode changes even during the demo
	if (gamestate == GS_LEVEL && ev->type == ev_keydown && ev->data1 == KEY_F12)
	{
		if (splitscreen || !netgame)
			displayplayer = consoleplayer;
		else
		{
			// spy mode
			do
			{
				displayplayer++;
				if (displayplayer == MAXPLAYERS)
					displayplayer = 0;

				if (!playeringame[displayplayer])
					continue;

				if (players[displayplayer].spectator)
					continue;

				// SRB2Kart: we have no team-based modes, YET...
				/*if (G_GametypeHasTeams())
				{
					if (players[consoleplayer].ctfteam
					 && players[displayplayer].ctfteam != players[consoleplayer].ctfteam)
						continue;
				}
				else if (gametype == GT_HIDEANDSEEK)
				{
					if (players[consoleplayer].pflags & PF_TAGIT)
						continue;
				}
				// Other Tag-based gametypes?
				else if (G_TagGametype())
				{
					if (!players[consoleplayer].spectator
					 && (players[consoleplayer].pflags & PF_TAGIT) != (players[displayplayer].pflags & PF_TAGIT))
						continue;
				}
				else if (G_GametypeHasSpectators() && G_BattleGametype())
				{
					if (!players[consoleplayer].spectator)
						continue;
				}*/

				// SRB2Kart: Only go through players who are actually playing
				if (players[displayplayer].exiting)
					continue;

				// I don't know if we want this actually, but I'll humor the suggestion anyway
				if (G_BattleGametype())
				{
					if (players[displayplayer].kartstuff[k_bumper] <= 0)
						continue;
				}

				break;
			} while (displayplayer != consoleplayer);

			// change statusbar also if playing back demo
			if (singledemo)
				ST_changeDemoView();

			return true;
		}
	}

	// any other key pops up menu if in demos
	if (gameaction == ga_nothing && !singledemo &&
		((demoplayback && !modeattacking && !titledemo) || gamestate == GS_TITLESCREEN))
	{
		if (ev->type == ev_keydown && ev->data1 != 301)
		{
			M_StartControlPanel();
			return true;
		}
		return false;
	}
	else if (demoplayback && titledemo)
	{
		// Title demo uses intro responder
		if (F_IntroResponder(ev))
		{
			// stop the title demo
			G_CheckDemoStatus();
			return true;
		}
		return false;
	}

	if (gamestate == GS_LEVEL)
	{
		if (HU_Responder(ev))
			return true; // chat ate the event
		if (AM_Responder(ev))
			return true; // automap ate it
		// map the event (key/mouse/joy) to a gamecontrol
	}
	// Intro
	else if (gamestate == GS_INTRO)
	{
		if (F_IntroResponder(ev))
		{
			D_StartTitle();
			return true;
		}
	}
	else if (gamestate == GS_CUTSCENE)
	{
		if (HU_Responder(ev))
			return true; // chat ate the event

		if (F_CutsceneResponder(ev))
		{
			D_StartTitle();
			return true;
		}
	}

	else if (gamestate == GS_CREDITS)
	{
		if (HU_Responder(ev))
			return true; // chat ate the event

		if (F_CreditResponder(ev))
		{
			F_StartGameEvaluation();
			return true;
		}
	}

	else if (gamestate == GS_CONTINUING)
	{
		if (F_ContinueResponder(ev))
			return true;
	}
	// Demo End
	else if (gamestate == GS_GAMEEND || gamestate == GS_EVALUATION || gamestate == GS_CREDITS)
		return true;

	else if (gamestate == GS_INTERMISSION || gamestate == GS_VOTING || gamestate == GS_WAITINGPLAYERS)
		if (HU_Responder(ev))
			return true; // chat ate the event

	// update keys current state
	G_MapEventsToControls(ev);

	switch (ev->type)
	{
		case ev_keydown:
			if (ev->data1 == gamecontrol[gc_pause][0]
				|| ev->data1 == gamecontrol[gc_pause][1])
			{
				if (!pausedelay)
				{
					// don't let busy scripts prevent pausing
					pausedelay = NEWTICRATE/7;

					// command will handle all the checks for us
					COM_ImmedExecute("pause");
					return true;
				}
				else
					pausedelay = NEWTICRATE/7;
			}
			if (ev->data1 == gamecontrol[gc_camtoggle][0]
				|| ev->data1 == gamecontrol[gc_camtoggle][1])
			{
				if (!camtoggledelay)
				{
					camtoggledelay = NEWTICRATE / 7;
					CV_SetValue(&cv_chasecam, cv_chasecam.value ? 0 : 1);
				}
			}
			if (ev->data1 == gamecontrolbis[gc_camtoggle][0]
				|| ev->data1 == gamecontrolbis[gc_camtoggle][1])
			{
				if (!camtoggledelay2)
				{
					camtoggledelay2 = NEWTICRATE / 7;
					CV_SetValue(&cv_chasecam2, cv_chasecam2.value ? 0 : 1);
				}
			}
			if (ev->data1 == gamecontrol3[gc_camtoggle][0]
				|| ev->data1 == gamecontrol3[gc_camtoggle][1])
			{
				if (!camtoggledelay3)
				{
					camtoggledelay3 = NEWTICRATE / 7;
					CV_SetValue(&cv_chasecam3, cv_chasecam3.value ? 0 : 1);
				}
			}
			if (ev->data1 == gamecontrol4[gc_camtoggle][0]
				|| ev->data1 == gamecontrol4[gc_camtoggle][1])
			{
				if (!camtoggledelay4)
				{
					camtoggledelay4 = NEWTICRATE / 7;
					CV_SetValue(&cv_chasecam4, cv_chasecam4.value ? 0 : 1);
				}
			}
			if (ev->data1 == gamecontrol[gc_spectate][0]
				|| ev->data1 == gamecontrol[gc_spectate][1])
			{
				if (!spectatedelay)
				{
					spectatedelay = NEWTICRATE / 7;
					COM_ImmedExecute("changeteam spectator");
				}
			}
			if (ev->data1 == gamecontrolbis[gc_spectate][0]
				|| ev->data1 == gamecontrolbis[gc_spectate][1])
			{
				if (!spectatedelay2)
				{
					spectatedelay2 = NEWTICRATE / 7;
					COM_ImmedExecute("changeteam2 spectator");
				}
			}
			if (ev->data1 == gamecontrol3[gc_spectate][0]
				|| ev->data1 == gamecontrol3[gc_spectate][1])
			{
				if (!spectatedelay3)
				{
					spectatedelay3 = NEWTICRATE / 7;
					COM_ImmedExecute("changeteam3 spectator");
				}
			}
			if (ev->data1 == gamecontrol4[gc_spectate][0]
				|| ev->data1 == gamecontrol4[gc_spectate][1])
			{
				if (!spectatedelay4)
				{
					spectatedelay4 = NEWTICRATE / 7;
					COM_ImmedExecute("changeteam4 spectator");
				}
			}

			return true;

		case ev_keyup:
			return false; // always let key up events filter down

		case ev_mouse:
			return true; // eat events

		case ev_joystick:
			return true; // eat events

		case ev_joystick2:
			return true; // eat events

		case ev_joystick3:
			return true; // eat events

		case ev_joystick4:
			return true; // eat events

		default:
			break;
	}

	return false;
}

//
// G_Ticker
// Make ticcmd_ts for the players.
//
void G_Ticker(boolean run)
{
	UINT32 i;
	INT32 buf;
	ticcmd_t *cmd;

	P_MapStart();
	// do player reborns if needed
	if (gamestate == GS_LEVEL)
	{
		// Or, alternatively, retry.
		if (!(netgame || multiplayer) && G_GetRetryFlag())
		{
			G_ClearRetryFlag();

			// Costs a life to retry ... unless the player in question is dead already.
			if (G_GametypeUsesLives() && players[consoleplayer].playerstate == PST_LIVE)
				players[consoleplayer].lives -= 1;

			G_DoReborn(consoleplayer);
		}

		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i] && players[i].playerstate == PST_REBORN)
				G_DoReborn(i);
	}
	P_MapEnd();

	// do things to change the game state
	while (gameaction != ga_nothing)
		switch (gameaction)
		{
			case ga_completed: G_DoCompleted(); break;
			case ga_startcont: G_DoStartContinue(); break;
			case ga_continued: G_DoContinued(); break;
			case ga_worlddone: G_DoWorldDone(); break;
			case ga_startvote: G_DoStartVote(); break;
			case ga_nothing: break;
			default: I_Error("gameaction = %d\n", gameaction);
		}

	buf = gametic % BACKUPTICS;

	// read/write demo and check turbo cheat
	for (i = 0; i < MAXPLAYERS; i++)
	{
		cmd = &players[i].cmd;

		if (playeringame[i])
		{
			// SRB2kart
			// Save the dir the player is holding
			//  to allow items to be thrown forward or backward.
			if (cmd->buttons & BT_FORWARD)
					players[i].kartstuff[k_throwdir] = 1;
			else if (cmd->buttons & BT_BACKWARD)
					players[i].kartstuff[k_throwdir] = -1;
			else
					players[i].kartstuff[k_throwdir] = 0;

			G_CopyTiccmd(cmd, &netcmds[buf][i], 1);
		}
	}

	// do main actions
	switch (gamestate)
	{
		case GS_LEVEL:
			if (titledemo)
				F_TitleDemoTicker();
			P_Ticker(run); // tic the game
			ST_Ticker();
			AM_Ticker();
			HU_Ticker();
			break;

		case GS_INTERMISSION:
			if (run)
				Y_Ticker();
			HU_Ticker();
			break;

		case GS_VOTING:
			if (run)
				Y_VoteTicker();
			HU_Ticker();
			break;

		case GS_TIMEATTACK:
			break;

		case GS_INTRO:
			if (run)
				F_IntroTicker();
			break;

		case GS_CUTSCENE:
			if (run)
				F_CutsceneTicker();
			HU_Ticker();
			break;

		case GS_GAMEEND:
			if (run)
				F_GameEndTicker();
			break;

		case GS_EVALUATION:
			if (run)
				F_GameEvaluationTicker();
			break;

		case GS_CONTINUING:
			if (run)
				F_ContinueTicker();
			break;

		case GS_CREDITS:
			if (run)
				F_CreditTicker();
			HU_Ticker();
			break;

		case GS_TITLESCREEN:
			F_TitleScreenTicker(run);
			break;
		case GS_WAITINGPLAYERS:
			if (netgame)
				F_WaitingPlayersTicker();
			HU_Ticker();
			break;

		case GS_DEDICATEDSERVER:
		case GS_NULL:
			break; // do nothing
	}

	if (run)
	{
		if (G_GametypeHasSpectators()
			&& (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION || gamestate == GS_VOTING // definitely good
			|| gamestate == GS_WAITINGPLAYERS)) // definitely a problem if we don't do it at all in this gamestate, but might need more protection?
			K_CheckSpectateStatus();

		if (pausedelay)
			pausedelay--;

		if (camtoggledelay)
			camtoggledelay--;
		if (camtoggledelay2)
			camtoggledelay2--;
		if (camtoggledelay3)
			camtoggledelay3--;
		if (camtoggledelay4)
			camtoggledelay4--;

		if (spectatedelay)
			spectatedelay--;
		if (spectatedelay2)
			spectatedelay2--;
		if (spectatedelay3)
			spectatedelay3--;
		if (spectatedelay4)
			spectatedelay4--;
	}
}

//
// PLAYER STRUCTURE FUNCTIONS
// also see P_SpawnPlayer in P_Things
//

//
// G_PlayerFinishLevel
// Called when a player completes a level.
//
static inline void G_PlayerFinishLevel(INT32 player)
{
	player_t *p;

	p = &players[player];

	memset(p->powers, 0, sizeof (p->powers));
	memset(p->kartstuff, 0, sizeof (p->kartstuff)); // SRB2kart
	p->ringweapons = 0;

	p->mo->flags2 &= ~MF2_SHADOW; // cancel invisibility
	P_FlashPal(p, 0, 0); // Resets
	p->starpostangle = 0;
	p->starposttime = 0;
	p->starpostx = 0;
	p->starposty = 0;
	p->starpostz = 0;
	p->starpostnum = 0;
	p->starpostcount = 0;

	// SRB2kart: Increment the "matches played" counter.
	if (player == consoleplayer)
	{
		if (legitimateexit && !demoplayback && !mapreset) // (yes you're allowed to unlock stuff this way when the game is modified)
		{
			UINT8 i = 0;

			if (netgame)
			{
				// check to see if there's anyone else at all
				for (; i < MAXPLAYERS; i++)
				{
					if (i == consoleplayer)
						continue;
					if (playeringame[i] && !stplyr->spectator)
						break;
				}
			}

			if (i != MAXPLAYERS) // Not FREE PLAY
			{
				matchesplayed++;
				if (M_UpdateUnlockablesAndExtraEmblems(true))
				{
					S_StartSound(NULL, sfx_ncitem);
					G_SaveGameData(true); // only save if unlocked something
				}
			}
		}

		legitimateexit = false;
	}
}

//
// G_PlayerReborn
// Called after a player dies. Almost everything is cleared and initialized.
//
void G_PlayerReborn(INT32 player)
{
	player_t *p;
	INT32 score, marescore;
	INT32 lives;
	INT32 continues;
	UINT8 charability;
	UINT8 charability2;
	// SRB2kart
	UINT8 kartspeed;
	UINT8 kartweight;
	//
	fixed_t normalspeed;
	fixed_t runspeed;
	UINT8 thrustfactor;
	UINT8 accelstart;
	UINT8 acceleration;
	INT32 charflags;
	INT32 pflags;
	UINT32 thokitem;
	UINT32 spinitem;
	UINT32 revitem;
	fixed_t actionspd;
	fixed_t mindash;
	fixed_t maxdash;
	INT32 ctfteam;
	INT32 starposttime;
	INT16 starpostx;
	INT16 starposty;
	INT16 starpostz;
	INT32 starpostnum;
	INT32 starpostcount;
	INT32 starpostangle;
	fixed_t jumpfactor;
	INT32 exiting;
	INT16 numboxes;
	INT16 totalring;
	UINT8 laps;
	UINT8 mare;
	UINT8 skincolor;
	INT32 skin;
	tic_t jointime;
	boolean spectator;
	INT16 bot;
	SINT8 pity;

	// SRB2kart
	INT32 starpostwp;
	INT32 itemtype;
	INT32 itemamount;
	INT32 itemroulette;
	INT32 roulettetype;
	INT32 bumper;
	INT32 comebackpoints;
	INT32 wanted;

	score = players[player].score;
	marescore = players[player].marescore;
	lives = players[player].lives;
	continues = players[player].continues;
	ctfteam = players[player].ctfteam;
	exiting = players[player].exiting;
	jointime = players[player].jointime;
	spectator = players[player].spectator;
	pflags = (players[player].pflags & (PF_TIMEOVER|PF_FLIPCAM|PF_TAGIT|PF_TAGGED|PF_ANALOGMODE));

	// As long as we're not in multiplayer, carry over cheatcodes from map to map
	if (!(netgame || multiplayer))
		pflags |= (players[player].pflags & (PF_GODMODE|PF_NOCLIP|PF_INVIS));

	numboxes = players[player].numboxes;
	laps = players[player].laps;
	totalring = players[player].totalring;

	skincolor = players[player].skincolor;
	skin = players[player].skin;
	charability = players[player].charability;
	charability2 = players[player].charability2;
	// SRB2kart
	kartspeed = players[player].kartspeed;
	kartweight = players[player].kartweight;
	//
	normalspeed = players[player].normalspeed;
	runspeed = players[player].runspeed;
	thrustfactor = players[player].thrustfactor;
	accelstart = players[player].accelstart;
	acceleration = players[player].acceleration;
	charflags = players[player].charflags;

	starposttime = players[player].starposttime;
	starpostx = players[player].starpostx;
	starposty = players[player].starposty;
	starpostz = players[player].starpostz;
	starpostnum = players[player].starpostnum;
	starpostcount = players[player].starpostcount;
	starpostangle = players[player].starpostangle;
	jumpfactor = players[player].jumpfactor;
	thokitem = players[player].thokitem;
	spinitem = players[player].spinitem;
	revitem = players[player].revitem;
	actionspd = players[player].actionspd;
	mindash = players[player].mindash;
	maxdash = players[player].maxdash;

	mare = players[player].mare;
	bot = players[player].bot;
	pity = players[player].pity;

	// SRB2kart
	if (leveltime <= starttime)
	{
		itemroulette = 0;
		roulettetype = 0;
		itemtype = 0;
		itemamount = 0;
		bumper = (G_BattleGametype() ? cv_kartbumpers.value : 0);
		comebackpoints = 0;
		wanted = 0;
		starpostwp = 0;
	}
	else
	{
		starpostwp = players[player].kartstuff[k_starpostwp];

		itemroulette = (players[player].kartstuff[k_itemroulette] > 0 ? 1 : 0);
		roulettetype = players[player].kartstuff[k_roulettetype];

		if (players[player].kartstuff[k_itemheld])
		{
			itemtype = 0;
			itemamount = 0;
		}
		else
		{
			itemtype = players[player].kartstuff[k_itemtype];
			itemamount = players[player].kartstuff[k_itemamount];
		}

		bumper = players[player].kartstuff[k_bumper];
		comebackpoints = players[player].kartstuff[k_comebackpoints];
		wanted = players[player].kartstuff[k_wanted];
	}

	p = &players[player];
	memset(p, 0, sizeof (*p));

	p->score = score;
	p->marescore = marescore;
	p->lives = lives;
	p->continues = continues;
	p->pflags = pflags;
	p->ctfteam = ctfteam;
	p->jointime = jointime;
	p->spectator = spectator;

	// save player config truth reborn
	p->skincolor = skincolor;
	p->skin = skin;
	p->charability = charability;
	p->charability2 = charability2;
	// SRB2kart
	p->kartspeed = kartspeed;
	p->kartweight = kartweight;
	//
	p->normalspeed = normalspeed;
	p->runspeed = runspeed;
	p->thrustfactor = thrustfactor;
	p->accelstart = accelstart;
	p->acceleration = acceleration;
	p->charflags = charflags;
	p->thokitem = thokitem;
	p->spinitem = spinitem;
	p->revitem = revitem;
	p->actionspd = actionspd;
	p->mindash = mindash;
	p->maxdash = maxdash;

	p->starposttime = starposttime;
	p->starpostx = starpostx;
	p->starposty = starposty;
	p->starpostz = starpostz;
	p->starpostnum = starpostnum;
	p->starpostcount = starpostcount;
	p->starpostangle = starpostangle;
	p->jumpfactor = jumpfactor;
	p->exiting = exiting;

	p->numboxes = numboxes;
	p->laps = laps;
	p->totalring = totalring;

	p->mare = mare;
	if (bot)
		p->bot = 1; // reset to AI-controlled
	p->pity = pity;

	// SRB2kart
	p->kartstuff[k_starpostwp] = starpostwp; // TODO: get these out of kartstuff, it causes desync
	p->kartstuff[k_itemroulette] = itemroulette;
	p->kartstuff[k_roulettetype] = roulettetype;
	p->kartstuff[k_itemtype] = itemtype;
	p->kartstuff[k_itemamount] = itemamount;
	p->kartstuff[k_bumper] = bumper;
	p->kartstuff[k_comebackpoints] = comebackpoints;
	p->kartstuff[k_comebacktimer] = comebacktime;
	p->kartstuff[k_wanted] = wanted;

	// Don't do anything immediately
	p->pflags |= PF_USEDOWN;
	p->pflags |= PF_ATTACKDOWN;
	p->pflags |= PF_JUMPDOWN;

	p->playerstate = PST_LIVE;
	p->health = 1; // 0 rings
	p->panim = PA_IDLE; // standing animation

	if ((netgame || multiplayer) && !p->spectator)
		p->powers[pw_flashing] = K_GetKartFlashing(p)-1; // Babysitting deterrent

	if (p-players == consoleplayer)
	{
		if (mapmusflags & MUSIC_RELOADRESET)
		{
			strncpy(mapmusname, mapheaderinfo[gamemap-1]->musname, 7);
			mapmusname[6] = 0;
			mapmusflags = mapheaderinfo[gamemap-1]->mustrack & MUSIC_TRACKMASK;
		}
	}

	P_RestoreMusic(p);

	if (leveltime > (starttime + (TICRATE/2)) && !p->spectator)
		p->kartstuff[k_respawn] = 48; // Respawn effect

	if (gametype == GT_COOP)
		P_FindEmerald(); // scan for emeralds to hunt for

	// Reset Nights score and max link to 0 on death
	p->maxlink = 0;

	// If NiGHTS, find lowest mare to start with.
	p->mare = P_FindLowestMare();

	CONS_Debug(DBG_NIGHTS, M_GetText("Current mare is %d\n"), p->mare);

	if (p->mare == 255)
		p->mare = 0;

	// Check to make sure their color didn't change somehow...
	if (G_GametypeHasTeams())
	{
		if (p->ctfteam == 1 && p->skincolor != skincolor_redteam)
		{
			if (p == &players[consoleplayer])
				CV_SetValue(&cv_playercolor, skincolor_redteam);
			else if (p == &players[secondarydisplayplayer])
				CV_SetValue(&cv_playercolor2, skincolor_redteam);
			else if (p == &players[thirddisplayplayer])
				CV_SetValue(&cv_playercolor3, skincolor_redteam);
			else if (p == &players[fourthdisplayplayer])
				CV_SetValue(&cv_playercolor4, skincolor_redteam);
		}
		else if (p->ctfteam == 2 && p->skincolor != skincolor_blueteam)
		{
			if (p == &players[consoleplayer])
				CV_SetValue(&cv_playercolor, skincolor_blueteam);
			else if (p == &players[secondarydisplayplayer])
				CV_SetValue(&cv_playercolor2, skincolor_blueteam);
			else if (p == &players[thirddisplayplayer])
				CV_SetValue(&cv_playercolor3, skincolor_blueteam);
			else if (p == &players[fourthdisplayplayer])
				CV_SetValue(&cv_playercolor4, skincolor_blueteam);
		}
	}
}

//
// G_CheckSpot
// Returns false if the player cannot be respawned
// at the given mapthing_t spot
// because something is occupying it
//
static boolean G_CheckSpot(INT32 playernum, mapthing_t *mthing)
{
	fixed_t x;
	fixed_t y;
	INT32 i;

	// maybe there is no player start
	if (!mthing)
		return false;

	if (!players[playernum].mo)
	{
		// first spawn of level
		for (i = 0; i < playernum; i++)
			if (playeringame[i] && players[i].mo
				&& players[i].mo->x == mthing->x << FRACBITS
				&& players[i].mo->y == mthing->y << FRACBITS)
			{
				return false;
			}
		return true;
	}

	x = mthing->x << FRACBITS;
	y = mthing->y << FRACBITS;

	if (!P_CheckPosition(players[playernum].mo, x, y))
		return false;

	if (!K_CheckPlayersRespawnColliding(playernum, x, y))
		return false;

	return true;
}

//
// G_SpawnPlayer
// Spawn a player in a spot appropriate for the gametype --
// or a not-so-appropriate spot, if it initially fails
// due to a lack of starts open or something.
//
void G_SpawnPlayer(INT32 playernum, boolean starpost)
{
	mapthing_t *spawnpoint;

	if (!playeringame[playernum])
		return;

	P_SpawnPlayer(playernum);

	if (starpost) //Don't even bother with looking for a place to spawn.
	{
		P_MovePlayerToStarpost(playernum);
#ifdef HAVE_BLUA
		LUAh_PlayerSpawn(&players[playernum]); // Lua hook for player spawning :)
#endif
		return;
	}

	// -- CTF --
	// Order: CTF->DM->Coop
	if (gametype == GT_CTF && players[playernum].ctfteam)
	{
		if (!(spawnpoint = G_FindCTFStart(playernum)) // find a CTF start
		&& !(spawnpoint = G_FindMatchStart(playernum))) // find a DM start
			spawnpoint = G_FindRaceStart(playernum); // fallback
	}

	// -- DM/Tag/CTF-spectator/etc --
	// Order: DM->CTF->Coop
	else if (gametype == GT_MATCH || gametype == GT_TEAMMATCH || gametype == GT_CTF
	 || ((gametype == GT_TAG || gametype == GT_HIDEANDSEEK) && !(players[playernum].pflags & PF_TAGIT)))
	{
		if (!(spawnpoint = G_FindMatchStart(playernum)) // find a DM start
		&& !(spawnpoint = G_FindCTFStart(playernum))) // find a CTF start
			spawnpoint = G_FindRaceStart(playernum); // fallback
	}

	// -- Other game modes --
	// Order: Coop->DM->CTF
	else
	{
		if (!(spawnpoint = G_FindRaceStart(playernum)) // find a Race start
		&& !(spawnpoint = G_FindMatchStart(playernum))) // find a DM start
			spawnpoint = G_FindCTFStart(playernum); // fallback
	}

	//No spawns found.  ANYWHERE.
	if (!spawnpoint)
	{
		if (nummapthings)
		{
			if (playernum == consoleplayer
				|| (splitscreen && playernum == secondarydisplayplayer)
				|| (splitscreen > 1 && playernum == thirddisplayplayer)
				|| (splitscreen > 2 && playernum == fourthdisplayplayer))
				CONS_Alert(CONS_ERROR, M_GetText("No player spawns found, spawning at the first mapthing!\n"));
			spawnpoint = &mapthings[0];
		}
		else
		{
			if (playernum == consoleplayer
			|| (splitscreen && playernum == secondarydisplayplayer)
			|| (splitscreen > 1 && playernum == thirddisplayplayer)
			|| (splitscreen > 2 && playernum == fourthdisplayplayer))
				CONS_Alert(CONS_ERROR, M_GetText("No player spawns found, spawning at the origin!\n"));
			//P_MovePlayerToSpawn handles this fine if the spawnpoint is NULL.
		}
	}
	P_MovePlayerToSpawn(playernum, spawnpoint);

#ifdef HAVE_BLUA
	LUAh_PlayerSpawn(&players[playernum]); // Lua hook for player spawning :)
#endif

}

mapthing_t *G_FindCTFStart(INT32 playernum)
{
	INT32 i,j;

	if (!numredctfstarts && !numbluectfstarts) //why even bother, eh?
	{
		if (playernum == consoleplayer
			|| (splitscreen && playernum == secondarydisplayplayer)
			|| (splitscreen > 1 && playernum == thirddisplayplayer)
			|| (splitscreen > 2 && playernum == fourthdisplayplayer))
			CONS_Alert(CONS_WARNING, M_GetText("No CTF starts in this map!\n"));
		return NULL;
	}

	if ((!players[playernum].ctfteam && numredctfstarts && (!numbluectfstarts || P_RandomChance(FRACUNIT/2))) || players[playernum].ctfteam == 1) //red
	{
		if (!numredctfstarts)
		{
			if (playernum == consoleplayer
				|| (splitscreen && playernum == secondarydisplayplayer)
				|| (splitscreen > 1 && playernum == thirddisplayplayer)
				|| (splitscreen > 2 && playernum == fourthdisplayplayer))
				CONS_Alert(CONS_WARNING, M_GetText("No Red Team starts in this map!\n"));
			return NULL;
		}

		for (j = 0; j < 32; j++)
		{
			i = P_RandomKey(numredctfstarts);
			if (G_CheckSpot(playernum, redctfstarts[i]))
				return redctfstarts[i];
		}

		if (playernum == consoleplayer
			|| (splitscreen && playernum == secondarydisplayplayer)
			|| (splitscreen > 1 && playernum == thirddisplayplayer)
			|| (splitscreen > 2 && playernum == fourthdisplayplayer))
			CONS_Alert(CONS_WARNING, M_GetText("Could not spawn at any Red Team starts!\n"));
		return NULL;
	}
	else if (!players[playernum].ctfteam || players[playernum].ctfteam == 2) //blue
	{
		if (!numbluectfstarts)
		{
			if (playernum == consoleplayer
				|| (splitscreen && playernum == secondarydisplayplayer)
				|| (splitscreen > 1 && playernum == thirddisplayplayer)
				|| (splitscreen > 2 && playernum == fourthdisplayplayer))
				CONS_Alert(CONS_WARNING, M_GetText("No Blue Team starts in this map!\n"));
			return NULL;
		}

		for (j = 0; j < 32; j++)
		{
			i = P_RandomKey(numbluectfstarts);
			if (G_CheckSpot(playernum, bluectfstarts[i]))
				return bluectfstarts[i];
		}
		if (playernum == consoleplayer
			|| (splitscreen && playernum == secondarydisplayplayer)
			|| (splitscreen > 1 && playernum == thirddisplayplayer)
			|| (splitscreen > 2 && playernum == fourthdisplayplayer))
			CONS_Alert(CONS_WARNING, M_GetText("Could not spawn at any Blue Team starts!\n"));
		return NULL;
	}
	//should never be reached but it gets stuff to shut up
	return NULL;
}

mapthing_t *G_FindMatchStart(INT32 playernum)
{
	INT32 i, j;

	if (numdmstarts)
	{
		for (j = 0; j < 64; j++)
		{
			i = P_RandomKey(numdmstarts);
			if (G_CheckSpot(playernum, deathmatchstarts[i]))
				return deathmatchstarts[i];
		}
		if (playernum == consoleplayer
			|| (splitscreen && playernum == secondarydisplayplayer)
			|| (splitscreen > 1 && playernum == thirddisplayplayer)
			|| (splitscreen > 2 && playernum == fourthdisplayplayer))
			CONS_Alert(CONS_WARNING, M_GetText("Could not spawn at any Deathmatch starts!\n"));
		return NULL;
	}

	if (playernum == consoleplayer
		|| (splitscreen && playernum == secondarydisplayplayer)
		|| (splitscreen > 1 && playernum == thirddisplayplayer)
		|| (splitscreen > 2 && playernum == fourthdisplayplayer))
		CONS_Alert(CONS_WARNING, M_GetText("No Deathmatch starts in this map!\n"));
	return NULL;
}

mapthing_t *G_FindRaceStart(INT32 playernum)
{
	if (numcoopstarts)
	{
		UINT8 i;
		UINT8 pos = 0;

		// SRB2Kart: figure out player spawn pos from points
		if (!playeringame[playernum] || players[playernum].spectator)
			return playerstarts[0]; // go to first spot if you're a spectator

		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!playeringame[i] || players[i].spectator)
				continue;
			if (i == playernum)
				continue;

			if (players[i].score < players[playernum].score)
			{
				UINT8 j;
				UINT8 num = 0;

				for (j = 0; j < MAXPLAYERS; j++) // I hate similar loops inside loops... :<
				{
					if (!playeringame[j] || players[j].spectator)
						continue;
					if (j == playernum)
						continue;
					if (j == i)
						continue;
					if (players[j].score == players[i].score)
						num++;
				}

				if (num > 1) // found dupes
					pos++;
			}
			else
			{
				if (players[i].score > players[playernum].score || i < playernum)
					pos++;
			}
		}

		if (G_CheckSpot(playernum, playerstarts[pos % numcoopstarts]))
			return playerstarts[pos % numcoopstarts];

		// Your spot isn't available? Find whatever you can get first.
		for (i = 0; i < numcoopstarts; i++)
		{
			if (G_CheckSpot(playernum, playerstarts[i]))
				return playerstarts[i];
		}

		// SRB2Kart: We have solid players, so this behavior is less ideal.
		// Don't bother checking to see if the player 1 start is open.
		// Just spawn there.
		//return playerstarts[0];

		if (playernum == consoleplayer
			|| (splitscreen && playernum == secondarydisplayplayer)
			|| (splitscreen > 1 && playernum == thirddisplayplayer)
			|| (splitscreen > 2 && playernum == fourthdisplayplayer))
			CONS_Alert(CONS_WARNING, M_GetText("Could not spawn at any Race starts!\n"));
		return NULL;
	}

	if (playernum == consoleplayer
		|| (splitscreen && playernum == secondarydisplayplayer)
		|| (splitscreen > 1 && playernum == thirddisplayplayer)
		|| (splitscreen > 2 && playernum == fourthdisplayplayer))
		CONS_Alert(CONS_WARNING, M_GetText("No Race starts in this map!\n"));
	return NULL;
}

// Go back through all the projectiles and remove all references to the old
// player mobj, replacing them with the new one.
void G_ChangePlayerReferences(mobj_t *oldmo, mobj_t *newmo)
{
	thinker_t *th;
	mobj_t *mo2;

	I_Assert((oldmo != NULL) && (newmo != NULL));

	// scan all thinkers
	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if (th->function.acp1 != (actionf_p1)P_MobjThinker)
			continue;

		mo2 = (mobj_t *)th;

		if (!(mo2->flags & MF_MISSILE))
			continue;

		if (mo2->target == oldmo)
		{
			P_SetTarget(&mo2->target, newmo);
			mo2->flags2 |= MF2_BEYONDTHEGRAVE; // this mobj belongs to a player who has reborn
		}
	}
}

//
// G_DoReborn
//
void G_DoReborn(INT32 playernum)
{
	player_t *player = &players[playernum];
	boolean starpost = false;

	/*if (modeattacking) // Not needed for SRB2Kart.
	{
		M_EndModeAttackRun();
		return;
	}*/

	// Make sure objectplace is OFF when you first start the level!
	OP_ResetObjectplace();

	if (player->bot && playernum != consoleplayer)
	{ // Bots respawn next to their master.
		mobj_t *oldmo = NULL;

		// first dissasociate the corpse
		if (player->mo)
		{
			oldmo = player->mo;
			// Don't leave your carcass stuck 10-billion feet in the ground!
			P_RemoveMobj(player->mo);
		}

		B_RespawnBot(playernum);
		if (oldmo)
			G_ChangePlayerReferences(oldmo, players[playernum].mo);
	}
	else if (countdowntimeup || (!multiplayer && gametype == GT_COOP))
	{
		// reload the level from scratch
		if (countdowntimeup)
		{
			player->starpostangle = 0;
			player->starposttime = 0;
			player->starpostx = 0;
			player->starposty = 0;
			player->starpostz = 0;
			player->starpostnum = 0;
			player->starpostcount = 0;
		}
		if (!countdowntimeup && (mapheaderinfo[gamemap-1]->levelflags & LF_NORELOAD))
		{
			INT32 i;

			player->playerstate = PST_REBORN;

			P_LoadThingsOnly();

			P_ClearStarPost(player->starpostnum);

			// Do a wipe
			wipegamestate = -1;

			if (player->starpostnum) // SRB2kart
				starpost = true;

			if (camera.chase)
				P_ResetCamera(&players[displayplayer], &camera);
			if (camera2.chase && splitscreen > 0)
				P_ResetCamera(&players[secondarydisplayplayer], &camera2);
			if (camera3.chase && splitscreen > 1)
				P_ResetCamera(&players[thirddisplayplayer], &camera3);
			if (camera4.chase && splitscreen > 2)
				P_ResetCamera(&players[fourthdisplayplayer], &camera4);

			// clear cmd building stuff
			memset(gamekeydown, 0, sizeof (gamekeydown));
			for (i = 0;i < JOYAXISSET; i++)
			{
				joyxmove[i] = joyymove[i] = 0;
				joy2xmove[i] = joy2ymove[i] = 0;
				joy3xmove[i] = joy3ymove[i] = 0;
				joy4xmove[i] = joy4ymove[i] = 0;
			}
			mousex = mousey = 0;
			mouse2x = mouse2y = 0;

			// clear hud messages remains (usually from game startup)
			CON_ClearHUD();

			// Starpost support
			G_SpawnPlayer(playernum, starpost);

			if (botingame)
			{ // Bots respawn next to their master.
				players[secondarydisplayplayer].playerstate = PST_REBORN;
				G_SpawnPlayer(secondarydisplayplayer, false);
			}
		}
		else
#ifdef HAVE_BLUA
		{
			LUAh_MapChange();
#endif
			G_DoLoadLevel(true);
#ifdef HAVE_BLUA
		}
#endif
	}
	else
	{
		// respawn at the start
		mobj_t *oldmo = NULL;

		if (player->starpostnum) // SRB2kart
			starpost = true;

		// first dissasociate the corpse
		if (player->mo)
		{
			oldmo = player->mo;
			// Don't leave your carcass stuck 10-billion feet in the ground!
			P_RemoveMobj(player->mo);
		}

		G_SpawnPlayer(playernum, starpost);
		if (oldmo)
			G_ChangePlayerReferences(oldmo, players[playernum].mo);
	}
}

void G_AddPlayer(INT32 playernum)
{
	player_t *p = &players[playernum];

	p->jointime = 0;
	p->playerstate = PST_REBORN;
}

void G_ExitLevel(void)
{
	if (gamestate == GS_LEVEL)
	{
		gameaction = ga_completed;
		lastdraw = true;

		// If you want your teams scrambled on map change, start the process now.
		// The teams will scramble at the start of the next round.
		if (cv_scrambleonchange.value && G_GametypeHasTeams())
		{
			if (server)
				CV_SetValue(&cv_teamscramble, cv_scrambleonchange.value);
		}

		if (netgame || multiplayer)
			CONS_Printf(M_GetText("The round has ended.\n"));

		// Remove CEcho text on round end.
		HU_DoCEcho("");
	}
}

//
// G_IsSpecialStage
//
// Returns TRUE if
// the given map is a special stage.
//
boolean G_IsSpecialStage(INT32 mapnum)
{
#if 0
	return (gametype == GT_COOP && modeattacking != ATTACKING_RECORD && mapnum >= sstage_start && mapnum <= sstage_end);
#else
	(void)mapnum;
	return false;
#endif
}

//
// G_GametypeUsesLives
//
// Returns true if the current gametype uses
// the lives system.  False otherwise.
//
boolean G_GametypeUsesLives(void)
{
	// SRB2kart NEEDS no lives
#if 0
	// Coop, Competitive
	if ((gametype == GT_COOP || gametype == GT_COMPETITION)
	 && !modeattacking // No lives in Time Attack
	 //&& !G_IsSpecialStage(gamemap)
	 && !(maptol & TOL_NIGHTS)) // No lives in NiGHTS
		return true;
	return false;
#else
	return false;
#endif
}

//
// G_GametypeHasTeams
//
// Returns true if the current gametype uses
// Red/Blue teams.  False otherwise.
//
boolean G_GametypeHasTeams(void)
{
	return (gametype == GT_TEAMMATCH || gametype == GT_CTF);
}

//
// G_GametypeHasSpectators
//
// Returns true if the current gametype supports
// spectators.  False otherwise.
//
boolean G_GametypeHasSpectators(void)
{
	// SRB2Kart: We don't have any exceptions to not being able to spectate yet. Maybe when SP & bots roll around.
#if 0
	return (gametype != GT_COOP && gametype != GT_COMPETITION && gametype != GT_RACE);
#else
	return (!splitscreen);//true;
#endif
}

//
// G_BattleGametype
//
// Returns true in Battle gamemodes, previously was G_RingSlingerGametype.
//
boolean G_BattleGametype(void)
{
	return (gametype == GT_MATCH);
}

//
// G_SometimesGetDifferentGametype
//
// Oh, yeah, and we sometimes flip encore mode on here too.
//
INT16 G_SometimesGetDifferentGametype(void)
{
	boolean encorepossible = (M_SecretUnlocked(SECRET_ENCORE) && G_RaceGametype());

	if (!cv_kartvoterulechanges.value) // never
		return gametype;

	if (randmapbuffer[NUMMAPS] > 0 && (encorepossible || cv_kartvoterulechanges.value != 3))
	{
		if (cv_kartvoterulechanges.value != 1)
			randmapbuffer[NUMMAPS]--;
		if (encorepossible)
		{
			switch (cv_kartvoterulechanges.value)
			{
				case 3: // always
					randmapbuffer[NUMMAPS] = 0; // gotta prep this in case it isn't already set
					break;
				case 2: // frequent
					encorepossible = M_RandomChance(FRACUNIT>>1);
					break;
				case 1: // sometimes
				default:
					encorepossible = M_RandomChance(FRACUNIT>>3);
					break;
			}
			if (encorepossible != (boolean)cv_kartencore.value)
				return (gametype|0x80);
		}
		return gametype;
	}

	switch (cv_kartvoterulechanges.value) // okay, we're having a gametype change! when's the next one, luv?
	{
		case 3: // always
			randmapbuffer[NUMMAPS] = 1; // every other vote (or always if !encorepossible)
			break;
		case 1: // sometimes
		default:
			// fallthrough - happens when clearing buffer, but needs a reasonable countdown if cvar is modified
		case 2: // frequent
			randmapbuffer[NUMMAPS] = 5; // per "cup"
			break;
	}

	if (gametype == GT_MATCH)
		return GT_RACE;
	return GT_MATCH;
}

//
// G_GetGametypeColor
//
// Pretty and consistent ^u^
// See also M_GetGametypeColor.
//
UINT8 G_GetGametypeColor(INT16 gt)
{
	if (modeattacking // == ATTACKING_RECORD
	|| gamestate == GS_TIMEATTACK)
		return orangemap[120];
	if (gt == GT_MATCH)
		return redmap[120];
	if (gt == GT_RACE)
		return skymap[120];
	return 247; // FALLBACK
}

//
// G_RaceGametype
//
// Returns true in Race gamemodes, previously was G_PlatformGametype.
//
boolean G_RaceGametype(void)
{
	return (gametype == GT_RACE);
}

//
// G_TagGametype
//
// For Jazz's Tag/HnS modes that have a lot of special cases...
// SRB2Kart: do we actually want to add Kart tag later? :V
//
boolean G_TagGametype(void)
{
	return (gametype == GT_TAG || gametype == GT_HIDEANDSEEK);
}

/** Get the typeoflevel flag needed to indicate support of a gametype.
  * In single-player, this always returns TOL_SP.
  * \param gametype The gametype for which support is desired.
  * \return The typeoflevel flag to check for that gametype.
  * \author Graue <graue@oceanbase.org>
  */
INT16 G_TOLFlag(INT32 pgametype)
{
	if (!multiplayer)                 return TOL_SP;
	if (pgametype == GT_COOP)         return TOL_RACE; // SRB2kart
	if (pgametype == GT_COMPETITION)  return TOL_COMPETITION;
	if (pgametype == GT_RACE)         return TOL_RACE;
	if (pgametype == GT_MATCH)        return TOL_MATCH;
	if (pgametype == GT_TEAMMATCH)    return TOL_MATCH;
	if (pgametype == GT_TAG)          return TOL_TAG;
	if (pgametype == GT_HIDEANDSEEK)  return TOL_TAG;
	if (pgametype == GT_CTF)          return TOL_CTF;

	CONS_Alert(CONS_ERROR, M_GetText("Unknown gametype! %d\n"), pgametype);
	return INT16_MAX;
}

#ifdef FLUSHMAPBUFFEREARLY
static INT32 TOLMaps(INT16 tolflags)
{
	INT32 num = 0;
	INT16 i;

	// Find all the maps that are ok and and put them in an array.
	for (i = 0; i < NUMMAPS; i++)
	{
		if (!mapheaderinfo[i])
			continue;

		if ((mapheaderinfo[i]->typeoflevel & tolflags) == tolflags)
			num++;
	}

	return num;
}
#endif

/** Select a random map with the given typeoflevel flags.
  * If no map has those flags, this arbitrarily gives you map 1.
  * \param tolflags The typeoflevel flags to insist on. Other bits may
  *                 be on too, but all of these must be on.
  * \return A random map with those flags, 1-based, or 1 if no map
  *         has those flags.
  * \author Graue <graue@oceanbase.org>
  */
static INT16 *okmaps = NULL;
INT16 G_RandMap(INT16 tolflags, INT16 pprevmap, boolean dontadd, boolean ignorebuffer, UINT8 maphell, boolean callagainsoon)
{
	INT32 numokmaps = 0;
	INT16 ix, bufx;

	if (!okmaps)
		okmaps = Z_Malloc(NUMMAPS * sizeof(INT16), PU_STATIC, NULL);

tryagain:

	// Find all the maps that are ok and and put them in an array.
	for (ix = 0; ix < NUMMAPS; ix++)
	{
		boolean isokmap = true;

		if (!mapheaderinfo[ix])
			continue;

		if ((mapheaderinfo[ix]->typeoflevel & tolflags) != tolflags
			|| ix == pprevmap
			|| (!dedicated && M_MapLocked(ix+1))
			|| (!maphell && (mapheaderinfo[ix]->menuflags & LF2_HIDEINMENU)) // this is bad
			|| ((maphell == 2) && !(mapheaderinfo[ix]->menuflags & LF2_HIDEINMENU))) // gasp
			isokmap = false;

		if (!ignorebuffer)
		{
			for (bufx = 0; bufx < (maphell ? 3 : NUMMAPS); bufx++)
			{
				if (randmapbuffer[bufx] == -1) // Rest of buffer SHOULD be empty
					break;
				if (ix == randmapbuffer[bufx])
				{
					isokmap = false;
					break;
				}
			}
		}

		if (isokmap)
			okmaps[numokmaps++] = ix;
	}

	if (numokmaps == 0)  // If there's no matches... (Goodbye, incredibly silly function chains :V)
	{
		if (!ignorebuffer)
		{
			if (randmapbuffer[3] == -1) // Is the buffer basically empty?
			{
				ignorebuffer = true; // This will probably only help in situations where there's very few maps, but it's folly not to at least try it
				goto tryagain; //return G_RandMap(tolflags, pprevmap, dontadd, true, maphell, callagainsoon);
			}

			for (bufx = 3; bufx < NUMMAPS; bufx++) // Let's clear all but the three most recent maps...
				randmapbuffer[bufx] = -1;
			if (cv_kartvoterulechanges.value == 1) // sometimes
				randmapbuffer[NUMMAPS] = 0;
			goto tryagain; //return G_RandMap(tolflags, pprevmap, dontadd, ignorebuffer, maphell, callagainsoon);
		}

		if (maphell) // Any wiggle room to loosen our restrictions here?
		{
			maphell--;
			goto tryagain; //return G_RandMap(tolflags, pprevmap, dontadd, true, maphell-1, callagainsoon);
		}

		ix = 0; // Sorry, none match. You get MAP01.
		for (bufx = 0; bufx < NUMMAPS+1; bufx++)
			randmapbuffer[bufx] = -1; // if we're having trouble finding a map we should probably clear it
	}
	else
	{
		ix = okmaps[M_RandomKey(numokmaps)];
		if (!dontadd)
		{
			for (bufx = NUMMAPS-1; bufx > 0; bufx--)
				randmapbuffer[bufx] = randmapbuffer[bufx-1];
			randmapbuffer[0] = ix;
		}
	}

	if (!callagainsoon)
	{
		Z_Free(okmaps);
		okmaps = NULL;
	}

	return ix;
}

//
// G_DoCompleted
//
static void G_DoCompleted(void)
{
	INT32 i, j = 0;
	boolean gottoken = false;

	tokenlist = 0; // Reset the list

	gameaction = ga_nothing;

	if (metalplayback)
		G_StopMetalDemo();
	if (metalrecording)
		G_StopMetalRecording();

	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
		{
			// SRB2Kart: exitlevel shouldn't get you the points
			if (!players[i].exiting && !(players[i].pflags & PF_TIMEOVER))
			{
				players[i].pflags |= PF_TIMEOVER;
				if (P_IsLocalPlayer(&players[i]))
					j++;
			}
			G_PlayerFinishLevel(i); // take away cards and stuff
		}

	// play some generic music if there's no win/cool/lose music going on (for exitlevel commands)
	if (G_RaceGametype() && j == splitscreen+1 && (cv_inttime.value > 0))
		S_ChangeMusicInternal("racent", true);

	if (automapactive)
		AM_Stop();

	S_StopSounds();

	prevmap = (INT16)(gamemap-1);

	// go to next level
	// nextmap is 0-based, unlike gamemap
	if (nextmapoverride != 0)
		nextmap = (INT16)(nextmapoverride-1);
	else
		nextmap = (INT16)(mapheaderinfo[gamemap-1]->nextlevel-1);

	// Remember last map for when you come out of the special stage.
	if (!G_IsSpecialStage(gamemap))
		lastmap = nextmap;

	// If nextmap is actually going to get used, make sure it points to
	// a map of the proper gametype -- skip levels that don't support
	// the current gametype. (Helps avoid playing boss levels in Race,
	// for instance).
	if (!token && !G_IsSpecialStage(gamemap) && !modeattacking
		&& (nextmap >= 0 && nextmap < NUMMAPS))
	{
		register INT16 cm = nextmap;
		INT16 tolflag = G_TOLFlag(gametype);
		UINT8 visitedmap[(NUMMAPS+7)/8];

		memset(visitedmap, 0, sizeof (visitedmap));

		while (!mapheaderinfo[cm] || !(mapheaderinfo[cm]->typeoflevel & tolflag))
		{
			visitedmap[cm/8] |= (1<<(cm&7));
			if (!mapheaderinfo[cm])
				cm = -1; // guarantee error execution
			else
				cm = (INT16)(mapheaderinfo[cm]->nextlevel-1);

			if (cm == 1100-1) // !!! WHENEVER WE GET GRAND PRIX, GO TO AWARDS MAP INSTEAD !!!
				cm = cm+1;

			if (cm >= NUMMAPS || cm < 0) // out of range (either 1100-1102 or error)
			{
				cm = nextmap; //Start the loop again so that the error checking below is executed.

				//Make sure the map actually exists before you try to go to it!
				if ((W_CheckNumForName(G_BuildMapName(cm + 1)) == LUMPERROR))
				{
					CONS_Alert(CONS_ERROR, M_GetText("Next map given (MAP %d) doesn't exist! Reverting to MAP01.\n"), cm+1);
					cm = 0;
					break;
				}
			}

			if (visitedmap[cm/8] & (1<<(cm&7))) // smells familiar
			{
				// We got stuck in a loop, came back to the map we started on
				// without finding one supporting the current gametype.
				// Thus, print a warning, and just use this map anyways.
				CONS_Alert(CONS_WARNING, M_GetText("Can't find a compatible map after map %d; using map %d anyway\n"), prevmap+1, cm+1);
				break;
			}
		}
		nextmap = cm;
	}

	if (nextmap < 0 || (nextmap >= NUMMAPS && nextmap < 1100-1) || nextmap > 1102-1)
		I_Error("Followed map %d to invalid map %d\n", prevmap + 1, nextmap + 1);

	// wrap around in race
	if (nextmap >= 1100-1 && nextmap <= 1102-1 && G_RaceGametype())
		nextmap = (INT16)(spstage_start-1);

	if (gametype == GT_COOP && token)
	{
		token--;
		gottoken = true;

		if (!(emeralds & EMERALD1))
			nextmap = (INT16)(sstage_start - 1); // Special Stage 1
		else if (!(emeralds & EMERALD2))
			nextmap = (INT16)(sstage_start); // Special Stage 2
		else if (!(emeralds & EMERALD3))
			nextmap = (INT16)(sstage_start + 1); // Special Stage 3
		else if (!(emeralds & EMERALD4))
			nextmap = (INT16)(sstage_start + 2); // Special Stage 4
		else if (!(emeralds & EMERALD5))
			nextmap = (INT16)(sstage_start + 3); // Special Stage 5
		else if (!(emeralds & EMERALD6))
			nextmap = (INT16)(sstage_start + 4); // Special Stage 6
		else if (!(emeralds & EMERALD7))
			nextmap = (INT16)(sstage_start + 5); // Special Stage 7
		else
			gottoken = false;
	}

	if (G_IsSpecialStage(gamemap) && !gottoken)
		nextmap = lastmap; // Exiting from a special stage? Go back to the game. Tails 08-11-2001

	automapactive = false;

#ifdef FLUSHMAPBUFFEREARLY
	if (randmapbuffer[TOLMaps(G_TOLFlag(gametype))-5] != -1) // We're getting pretty full, so! -- no need for this, handled in G_RandMap
	{
		for (i = 3; i < NUMMAPS; i++) // Let's clear all but the three most recent maps...
			randmapbuffer[i] = -1;
		if (cv_kartvoterulechanges.value == 1) // sometimes
			randmapbuffer[NUMMAPS] = 0;
	}
#endif

	if (gametype != GT_COOP)
	{
		if (cv_advancemap.value == 0) // Stay on same map.
			nextmap = prevmap;
		else if (cv_advancemap.value == 2) // Go to random map.
			nextmap = G_RandMap(G_TOLFlag(gametype), prevmap, false, false, 0, false);
	}

	// We are committed to this map now.
	// We may as well allocate its header if it doesn't exist
	// (That is, if it's a real map)
	if (nextmap < NUMMAPS && !mapheaderinfo[nextmap])
		P_AllocMapHeader(nextmap);

	if (skipstats && !modeattacking) // Don't skip stats if we're in record attack
		G_AfterIntermission();
	else
	{
		G_SetGamestate(GS_INTERMISSION);
		Y_StartIntermission();
	}
}

void G_AfterIntermission(void)
{
	HU_ClearCEcho();
	//G_NextLevel();

	if (mapheaderinfo[gamemap-1]->cutscenenum && !modeattacking) // Start a custom cutscene.
		F_StartCustomCutscene(mapheaderinfo[gamemap-1]->cutscenenum-1, false, false);
	else
	{
		if (nextmap < 1100-1)
			G_NextLevel();
		else
			Y_EndGame();
	}
}

//
// G_NextLevel (WorldDone)
//
// init next level or go to the final scene
// called by end of intermission screen (y_inter)
//
void G_NextLevel(void)
{
	if (gamestate != GS_VOTING)
	{
		if ((cv_advancemap.value == 3) && !modeattacking && !skipstats && (multiplayer || netgame))
		{
			UINT8 i;
			for (i = 0; i < MAXPLAYERS; i++)
			{
				if (playeringame[i] && !players[i].spectator)
				{
					gameaction = ga_startvote;
					return;
				}
			}
		}

		forceresetplayers = false;
		deferencoremode = (boolean)cv_kartencore.value;
	}
	
	gameaction = ga_worlddone;
}

static void G_DoWorldDone(void)
{
	if (server)
	{
		// SRB2Kart
		D_MapChange(nextmap+1,
			gametype,
			deferencoremode,
			forceresetplayers,
			0,
			false,
			false);
	}

	gameaction = ga_nothing;
}

//
// G_DoStartVote
//
static void G_DoStartVote(void)
{
	if (server)
		D_SetupVote();
	gameaction = ga_nothing;
}

//
// G_UseContinue
//
void G_UseContinue(void)
{
	if (gamestate == GS_LEVEL && !netgame && !multiplayer)
	{
		gameaction = ga_startcont;
		lastdraw = true;
	}
}

static void G_DoStartContinue(void)
{
	I_Assert(!netgame && !multiplayer);

	legitimateexit = false;
	G_PlayerFinishLevel(consoleplayer); // take away cards and stuff

	F_StartContinue();
	gameaction = ga_nothing;
}

//
// G_Continue
//
// re-init level, used by continue and possibly countdowntimeup
//
void G_Continue(void)
{
	if (!netgame && !multiplayer)
		gameaction = ga_continued;
}

static void G_DoContinued(void)
{
	player_t *pl = &players[consoleplayer];
	I_Assert(!netgame && !multiplayer);
	I_Assert(pl->continues > 0);

	pl->continues--;

	// Reset score
	pl->score = 0;

	// Allow tokens to come back
	tokenlist = 0;
	token = 0;

	// Reset # of lives
	pl->lives = (ultimatemode) ? 1 : 3;

	D_MapChange(gamemap, gametype, false, false, 0, false, false);

	gameaction = ga_nothing;
}

//
// G_LoadGameSettings
//
// Sets a tad of default info we need.
void G_LoadGameSettings(void)
{
	// defaults
	spstage_start = 1;
	sstage_start = 50;
	sstage_end = 57; // 8 special stages in vanilla SRB2
	useNightsSS = false; //true;

	// initialize free sfx slots for skin sounds
	S_InitRuntimeSounds();
}

// G_LoadGameData
// Loads the main data file, which stores information such as emblems found, etc.
void G_LoadGameData(void)
{
	size_t length;
	INT32 i, j;
	UINT8 modded = false;
	UINT8 rtemp;

	//For records
	tic_t rectime;
	tic_t reclap;
	//UINT32 recscore;
	//UINT16 recrings;

	//UINT8 recmares;
	//INT32 curmare;

	// Clear things so previously read gamedata doesn't transfer
	// to new gamedata
	G_ClearRecords(); // main and nights records
	M_ClearSecrets(); // emblems, unlocks, maps visited, etc
	totalplaytime = 0; // total play time (separate from all)
	matchesplayed = 0; // SRB2Kart: matches played & finished

	if (M_CheckParm("-nodata"))
		return; // Don't load.

	// Allow saving of gamedata beyond this point
	gamedataloaded = true;

	if (M_CheckParm("-resetdata"))
		return; // Don't load (essentially, reset).

	length = FIL_ReadFile(va(pandf, srb2home, gamedatafilename), &savebuffer);
	if (!length) // Aw, no game data. Their loss!
		return;

	save_p = savebuffer;

	// Version check
	if (READUINT32(save_p) != 0xFCAFE211)
	{
		const char *gdfolder = "the SRB2Kart folder";
		if (strcmp(srb2home,"."))
			gdfolder = srb2home;

		Z_Free(savebuffer);
		save_p = NULL;
		I_Error("Game data is from another version of SRB2.\nDelete %s(maybe in %s) and try again.", gamedatafilename, gdfolder);
	}

	totalplaytime = READUINT32(save_p);
	matchesplayed = READUINT32(save_p);

	modded = READUINT8(save_p);

	// Aha! Someone's been screwing with the save file!
	if ((modded && !savemoddata))
		goto datacorrupt;
	else if (modded != true && modded != false)
		goto datacorrupt;

	// TODO put another cipher on these things? meh, I don't care...
	for (i = 0; i < NUMMAPS; i++)
		if ((mapvisited[i] = READUINT8(save_p)) > MV_MAX)
			goto datacorrupt;

	// To save space, use one bit per collected/achieved/unlocked flag
	for (i = 0; i < MAXEMBLEMS;)
	{
		rtemp = READUINT8(save_p);
		for (j = 0; j < 8 && j+i < MAXEMBLEMS; ++j)
			emblemlocations[j+i].collected = ((rtemp >> j) & 1);
		i += j;
	}
	for (i = 0; i < MAXEXTRAEMBLEMS;)
	{
		rtemp = READUINT8(save_p);
		for (j = 0; j < 8 && j+i < MAXEXTRAEMBLEMS; ++j)
			extraemblems[j+i].collected = ((rtemp >> j) & 1);
		i += j;
	}
	for (i = 0; i < MAXUNLOCKABLES;)
	{
		rtemp = READUINT8(save_p);
		for (j = 0; j < 8 && j+i < MAXUNLOCKABLES; ++j)
			unlockables[j+i].unlocked = ((rtemp >> j) & 1);
		i += j;
	}
	for (i = 0; i < MAXCONDITIONSETS;)
	{
		rtemp = READUINT8(save_p);
		for (j = 0; j < 8 && j+i < MAXCONDITIONSETS; ++j)
			conditionSets[j+i].achieved = ((rtemp >> j) & 1);
		i += j;
	}

	timesBeaten = READUINT32(save_p);
	timesBeatenWithEmeralds = READUINT32(save_p);
	//timesBeatenUltimate = READUINT32(save_p);

	// Main records
	for (i = 0; i < NUMMAPS; ++i)
	{
		rectime = (tic_t)READUINT32(save_p);
		reclap  = (tic_t)READUINT32(save_p);
		//recscore = READUINT32(save_p);
		//recrings = READUINT16(save_p);

		/*if (recrings > 10000 || recscore > MAXSCORE)
			goto datacorrupt;*/

		if (rectime || reclap)
		{
			G_AllocMainRecordData((INT16)i);
			mainrecords[i]->time = rectime;
			mainrecords[i]->lap = reclap;
			//mainrecords[i]->score = recscore;
			//mainrecords[i]->rings = recrings;
		}
	}

	// Nights records
	/*for (i = 0; i < NUMMAPS; ++i)
	{
		if ((recmares = READUINT8(save_p)) == 0)
			continue;

		G_AllocNightsRecordData((INT16)i);

		for (curmare = 0; curmare < (recmares+1); ++curmare)
		{
			nightsrecords[i]->score[curmare] = READUINT32(save_p);
			nightsrecords[i]->grade[curmare] = READUINT8(save_p);
			nightsrecords[i]->time[curmare] = (tic_t)READUINT32(save_p);

			if (nightsrecords[i]->grade[curmare] > GRADE_S)
				goto datacorrupt;
		}

		nightsrecords[i]->nummares = recmares;
	}*/

	// done
	Z_Free(savebuffer);
	save_p = NULL;

	// Silent update unlockables in case they're out of sync with conditions
	M_SilentUpdateUnlockablesAndEmblems();

	return;

	// Landing point for corrupt gamedata
	datacorrupt:
	{
		const char *gdfolder = "the SRB2Kart folder";
		if (strcmp(srb2home,"."))
			gdfolder = srb2home;

		Z_Free(savebuffer);
		save_p = NULL;

		I_Error("Corrupt game data file.\nDelete %s(maybe in %s) and try again.", gamedatafilename, gdfolder);
	}
}

// G_SaveGameData
// Saves the main data file, which stores information such as emblems found, etc.
void G_SaveGameData(boolean force)
{
	const boolean wasmodified = modifiedgame;
	size_t length;
	INT32 i, j;
	UINT8 btemp;

	//INT32 curmare;

	if (!gamedataloaded)
		return; // If never loaded (-nodata), don't save

	save_p = savebuffer = (UINT8 *)malloc(GAMEDATASIZE);
	if (!save_p)
	{
		CONS_Alert(CONS_ERROR, M_GetText("No more free memory for saving game data\n"));
		return;
	}

	if (force) // SRB2Kart: for enabling unlocks online, even if the game is modified
		modifiedgame = savemoddata; // L-let's just sort of... hack around the cheat protection, because I'm too worried about just removing it @@;
	else if (modifiedgame && !savemoddata)
	{
		free(savebuffer);
		save_p = savebuffer = NULL;
		return;
	}

	// Version test
	WRITEUINT32(save_p, 0xFCAFE211);

	WRITEUINT32(save_p, totalplaytime);
	WRITEUINT32(save_p, matchesplayed);

	btemp = (UINT8)(savemoddata || modifiedgame);
	WRITEUINT8(save_p, btemp);

	// TODO put another cipher on these things? meh, I don't care...
	for (i = 0; i < NUMMAPS; i++)
		WRITEUINT8(save_p, mapvisited[i]);

	// To save space, use one bit per collected/achieved/unlocked flag
	for (i = 0; i < MAXEMBLEMS;)
	{
		btemp = 0;
		for (j = 0; j < 8 && j+i < MAXEMBLEMS; ++j)
			btemp |= (emblemlocations[j+i].collected << j);
		WRITEUINT8(save_p, btemp);
		i += j;
	}
	for (i = 0; i < MAXEXTRAEMBLEMS;)
	{
		btemp = 0;
		for (j = 0; j < 8 && j+i < MAXEXTRAEMBLEMS; ++j)
			btemp |= (extraemblems[j+i].collected << j);
		WRITEUINT8(save_p, btemp);
		i += j;
	}
	for (i = 0; i < MAXUNLOCKABLES;)
	{
		btemp = 0;
		for (j = 0; j < 8 && j+i < MAXUNLOCKABLES; ++j)
			btemp |= (unlockables[j+i].unlocked << j);
		WRITEUINT8(save_p, btemp);
		i += j;
	}
	for (i = 0; i < MAXCONDITIONSETS;)
	{
		btemp = 0;
		for (j = 0; j < 8 && j+i < MAXCONDITIONSETS; ++j)
			btemp |= (conditionSets[j+i].achieved << j);
		WRITEUINT8(save_p, btemp);
		i += j;
	}

	WRITEUINT32(save_p, timesBeaten);
	WRITEUINT32(save_p, timesBeatenWithEmeralds);
	//WRITEUINT32(save_p, timesBeatenUltimate);

	// Main records
	for (i = 0; i < NUMMAPS; i++)
	{
		if (mainrecords[i])
		{
			WRITEUINT32(save_p, mainrecords[i]->time);
			WRITEUINT32(save_p, mainrecords[i]->lap);
			//WRITEUINT32(save_p, mainrecords[i]->score);
			//WRITEUINT16(save_p, mainrecords[i]->rings);
		}
		else
		{
			WRITEUINT32(save_p, 0);
			WRITEUINT32(save_p, 0);
		}
	}

	// NiGHTS records
	/*for (i = 0; i < NUMMAPS; i++)
	{
		if (!nightsrecords[i] || !nightsrecords[i]->nummares)
		{
			WRITEUINT8(save_p, 0);
			continue;
		}

		WRITEUINT8(save_p, nightsrecords[i]->nummares);

		for (curmare = 0; curmare < (nightsrecords[i]->nummares + 1); ++curmare)
		{
			WRITEUINT32(save_p, nightsrecords[i]->score[curmare]);
			WRITEUINT8(save_p, nightsrecords[i]->grade[curmare]);
			WRITEUINT32(save_p, nightsrecords[i]->time[curmare]);
		}
	}*/

	length = save_p - savebuffer;

	FIL_WriteFile(va(pandf, srb2home, gamedatafilename), savebuffer, length);
	free(savebuffer);
	save_p = savebuffer = NULL;

	if (force) // Eeeek, I'm sorry for my sins!
		modifiedgame = wasmodified;
}

#define VERSIONSIZE 16

#ifdef SAVEGAMES_OTHERVERSIONS
static INT16 startonmapnum = 0;

//
// User wants to load a savegame from a different version?
//
static void M_ForceLoadGameResponse(INT32 ch)
{
	if (ch != 'y' && ch != KEY_ENTER)
	{
		//refused
		Z_Free(savebuffer);
		save_p = savebuffer = NULL;
		startonmapnum = 0;
		M_SetupNextMenu(&SP_LoadDef);
		return;
	}

	// pick up where we left off.
	save_p += VERSIONSIZE;
	if (!P_LoadGame(startonmapnum))
	{
		M_ClearMenus(true); // so ESC backs out to title
		M_StartMessage(M_GetText("Savegame file corrupted\n\nPress ESC\n"), NULL, MM_NOTHING);
		Command_ExitGame_f();
		Z_Free(savebuffer);
		save_p = savebuffer = NULL;
		startonmapnum = 0;

		// no cheating!
		memset(&savedata, 0, sizeof(savedata));
		return;
	}

	// done
	Z_Free(savebuffer);
	save_p = savebuffer = NULL;
	startonmapnum = 0;

	//set cursaveslot to -1 so nothing gets saved.
	cursaveslot = -1;

	displayplayer = consoleplayer;
	multiplayer = false;
	splitscreen = 0;

	if (setsizeneeded)
		R_ExecuteSetViewSize();

	M_ClearMenus(true);
	CON_ToggleOff();
}
#endif

//
// G_InitFromSavegame
// Can be called by the startup code or the menu task.
//
void G_LoadGame(UINT32 slot, INT16 mapoverride)
{
	size_t length;
	char vcheck[VERSIONSIZE];
	char savename[255];

	// memset savedata to all 0, fixes calling perfectly valid saves corrupt because of bots
	memset(&savedata, 0, sizeof(savedata));

#ifdef SAVEGAME_OTHERVERSIONS
	//Oh christ.  The force load response needs access to mapoverride too...
	startonmapnum = mapoverride;
#endif

	sprintf(savename, savegamename, slot);

	length = FIL_ReadFile(savename, &savebuffer);
	if (!length)
	{
		CONS_Printf(M_GetText("Couldn't read file %s\n"), savename);
		return;
	}

	save_p = savebuffer;

	memset(vcheck, 0, sizeof (vcheck));
	sprintf(vcheck, "version %d", VERSION);
	if (strcmp((const char *)save_p, (const char *)vcheck))
	{
#ifdef SAVEGAME_OTHERVERSIONS
		M_StartMessage(M_GetText("Save game from different version.\nYou can load this savegame, but\nsaving afterwards will be disabled.\n\nDo you want to continue anyway?\n\n(Press 'Y' to confirm)\n"),
		               M_ForceLoadGameResponse, MM_YESNO);
		//Freeing done by the callback function of the above message
#else
		M_ClearMenus(true); // so ESC backs out to title
		M_StartMessage(M_GetText("Save game from different version\n\nPress ESC\n"), NULL, MM_NOTHING);
		Command_ExitGame_f();
		Z_Free(savebuffer);
		save_p = savebuffer = NULL;

		// no cheating!
		memset(&savedata, 0, sizeof(savedata));
#endif
		return; // bad version
	}
	save_p += VERSIONSIZE;

//	if (demoplayback) // reset game engine
//		G_StopDemo();

//	paused = false;
//	automapactive = false;

	// dearchive all the modifications
	if (!P_LoadGame(mapoverride))
	{
		M_ClearMenus(true); // so ESC backs out to title
		M_StartMessage(M_GetText("Savegame file corrupted\n\nPress ESC\n"), NULL, MM_NOTHING);
		Command_ExitGame_f();
		Z_Free(savebuffer);
		save_p = savebuffer = NULL;

		// no cheating!
		memset(&savedata, 0, sizeof(savedata));
		return;
	}

	// done
	Z_Free(savebuffer);
	save_p = savebuffer = NULL;

//	gameaction = ga_nothing;
//	G_SetGamestate(GS_LEVEL);
	displayplayer = consoleplayer;
	multiplayer = false;
	splitscreen = 0;

//	G_DeferedInitNew(sk_medium, G_BuildMapName(1), 0, 0, 1);
	if (setsizeneeded)
		R_ExecuteSetViewSize();

	M_ClearMenus(true);
	CON_ToggleOff();
}

//
// G_SaveGame
// Saves your game.
//
void G_SaveGame(UINT32 savegameslot)
{
	boolean saved;
	char savename[256] = "";
	const char *backup;

	sprintf(savename, savegamename, savegameslot);
	backup = va("%s",savename);

	// save during evaluation or credits? game's over, folks!
	if (gamestate == GS_CREDITS || gamestate == GS_EVALUATION)
		gamecomplete = true;

	gameaction = ga_nothing;
	{
		char name[VERSIONSIZE];
		size_t length;

		save_p = savebuffer = (UINT8 *)malloc(SAVEGAMESIZE);
		if (!save_p)
		{
			CONS_Alert(CONS_ERROR, M_GetText("No more free memory for saving game data\n"));
			return;
		}

		memset(name, 0, sizeof (name));
		sprintf(name, "version %d", VERSION);
		WRITEMEM(save_p, name, VERSIONSIZE);

		P_SaveGame();

		length = save_p - savebuffer;
		saved = FIL_WriteFile(backup, savebuffer, length);
		free(savebuffer);
		save_p = savebuffer = NULL;
	}

	gameaction = ga_nothing;

	if (cv_debug && saved)
		CONS_Printf(M_GetText("Game saved.\n"));
	else if (!saved)
		CONS_Alert(CONS_ERROR, M_GetText("Error while writing to %s for save slot %u, base: %s\n"), backup, savegameslot, savegamename);
}

//
// G_DeferedInitNew
// Can be called by the startup code or the menu task,
// consoleplayer, displayplayer, playeringame[] should be set.
//
void G_DeferedInitNew(boolean pencoremode, const char *mapname, INT32 pickedchar, UINT8 ssplayers, boolean FLS)
{
	INT32 i;
	UINT8 color = 0;
	paused = false;

	if (demoplayback)
		COM_BufAddText("stopdemo\n");

	while (ghosts)
	{
		demoghost *next = ghosts->next;
		Z_Free(ghosts);
		ghosts = next;
	}
	ghosts = NULL;

	for (i = 0; i < NUMMAPS+1; i++)
		randmapbuffer[i] = -1;

	// this leave the actual game if needed
	SV_StartSinglePlayerServer();

	if (savedata.lives > 0)
	{
		color = savedata.skincolor;
		botskin = savedata.botskin;
		botcolor = savedata.botcolor;
		botingame = (botskin != 0);
	}
	else if (splitscreen != ssplayers)
	{
		splitscreen = ssplayers;
		SplitScreen_OnChange();
	}

	if (!color && !modeattacking)
		color = skins[pickedchar].prefcolor;
	SetPlayerSkinByNum(consoleplayer, pickedchar);
	CV_StealthSet(&cv_skin, skins[pickedchar].name);

	if (color)
		CV_StealthSetValue(&cv_playercolor, color);

	if (mapname)
		D_MapChange(M_MapNumber(mapname[3], mapname[4]), gametype, pencoremode, true, 1, false, FLS);
}

//
// This is the map command interpretation something like Command_Map_f
//
// called at: map cmd execution, doloadgame, doplaydemo
void G_InitNew(UINT8 pencoremode, const char *mapname, boolean resetplayer, boolean skipprecutscene)
{
	INT32 i;

	if (paused)
	{
		paused = false;
		S_ResumeAudio();
	}

	prevencoremode = ((gamestate == GS_TITLESCREEN) ? false : encoremode);
	encoremode = pencoremode;

	legitimateexit = false; // SRB2Kart
	comebackshowninfo = false;

	if (!demoplayback && !netgame) // Netgame sets random seed elsewhere, demo playback sets seed just before us!
		P_SetRandSeed(M_RandomizedSeed()); // Use a more "Random" random seed

	//SRB2Kart - Score is literally the only thing you SHOULDN'T reset at all times
	//if (resetplayer)
	{
		// Clear a bunch of variables
		tokenlist = token = sstimer = redscore = bluescore = lastmap = 0;
		countdown = countdown2 = mapreset = 0;

		for (i = 0; i < MAXPLAYERS; i++)
		{
			players[i].playerstate = PST_REBORN;
			players[i].starpostangle = players[i].starpostnum = players[i].starposttime = 0;
			players[i].starpostx = players[i].starposty = players[i].starpostz = 0;
			players[i].starpostcount = 0; // srb2kart

#if 0
			if (netgame || multiplayer)
			{
				players[i].lives = cv_startinglives.value;
				players[i].continues = 0;
			}
			else if (pultmode)
			{
				players[i].lives = 1;
				players[i].continues = 0;
			}
			else
			{
				players[i].lives = 3;
				players[i].continues = 1;
			}

			players[i].xtralife = 0;
#else
			players[i].lives = 1; // SRB2Kart
#endif

			// The latter two should clear by themselves, but just in case
			players[i].pflags &= ~(PF_TAGIT|PF_TAGGED|PF_FULLSTASIS);

			// Clear cheatcodes too, just in case.
			players[i].pflags &= ~(PF_GODMODE|PF_NOCLIP|PF_INVIS);

			players[i].marescore = 0;

			if (resetplayer) // SRB2Kart
			{
				players[i].score = 0;
			}
		}

		// Reset unlockable triggers
		unlocktriggers = 0;

		// clear itemfinder, just in case
		if (!dedicated) // except in dedicated servers, where it is not registered and can actually I_Error debug builds
			CV_StealthSetValue(&cv_itemfinder, 0);
	}

	// internal game map
	// well this check is useless because it is done before (d_netcmd.c::command_map_f)
	// but in case of for demos....
	if (W_CheckNumForName(mapname) == LUMPERROR)
	{
		I_Error("Internal game map '%s' not found\n", mapname);
		Command_ExitGame_f();
		return;
	}

	gamemap = (INT16)M_MapNumber(mapname[3], mapname[4]); // get xx out of MAPxx

	// gamemap changed; we assume that its map header is always valid,
	// so make it so
	if(!mapheaderinfo[gamemap-1])
		P_AllocMapHeader(gamemap-1);

	maptol = mapheaderinfo[gamemap-1]->typeoflevel;
	globalweather = mapheaderinfo[gamemap-1]->weather;

	// Don't carry over custom music change to another map.
	mapmusflags |= MUSIC_RELOADRESET;

	playerdeadview = false;
	automapactive = false;
	imcontinuing = false;

	if (!skipprecutscene && mapheaderinfo[gamemap-1]->precutscenenum && !modeattacking) // Start a custom cutscene.
		F_StartCustomCutscene(mapheaderinfo[gamemap-1]->precutscenenum-1, true, resetplayer);
	else
		G_DoLoadLevel(resetplayer);

	if (netgame)
	{
		char *title = G_BuildMapTitle(gamemap);

		CONS_Printf(M_GetText("Map is now \"%s"), G_BuildMapName(gamemap));
		if (title)
		{
			CONS_Printf(": %s", title);
			Z_Free(title);
		}
		CONS_Printf("\"\n");
	}
}


char *G_BuildMapTitle(INT32 mapnum)
{
	char *title = NULL;

	if (mapnum == 0)
		return Z_StrDup("Random");

	if (strcmp(mapheaderinfo[mapnum-1]->lvlttl, ""))
	{
		size_t len = 1;
		const char *zonetext = NULL;
		const char *actnum = NULL;

		len += strlen(mapheaderinfo[mapnum-1]->lvlttl);
		if (strlen(mapheaderinfo[mapnum-1]->zonttl) > 0)
		{
			zonetext = M_GetText(mapheaderinfo[mapnum-1]->zonttl);
			len += strlen(zonetext) + 1;	// ' ' + zonetext
		}
		else if (!(mapheaderinfo[mapnum-1]->levelflags & LF_NOZONE))
		{
			zonetext = M_GetText("ZONE");
			len += strlen(zonetext) + 1;	// ' ' + zonetext
		}
		if (strlen(mapheaderinfo[mapnum-1]->actnum) > 0)
		{
			actnum = M_GetText(mapheaderinfo[mapnum-1]->actnum);
			len += strlen(actnum) + 1;	// ' ' + actnum
		}

		title = Z_Malloc(len, PU_STATIC, NULL);

		sprintf(title, "%s", mapheaderinfo[mapnum-1]->lvlttl);
		if (zonetext) sprintf(title + strlen(title), " %s", zonetext);
		if (actnum) sprintf(title + strlen(title), " %s", actnum);
	}

	return title;
}

//
// DEMO RECORDING
//

#define DEMOVERSION 0x0001
#define DEMOHEADER  "\xF0" "KartReplay" "\x0F"

#define DF_GHOST        0x01 // This demo contains ghost data too!
#define DF_RECORDATTACK 0x02 // This demo is from record attack and contains its final completion time!
#define DF_NIGHTSATTACK 0x04 // This demo is from NiGHTS attack and contains its time left, score, and mares!
#define DF_ATTACKMASK   0x06 // This demo is from ??? attack and contains ???
#define DF_ATTACKSHIFT  1

// For demos
#define ZT_FWD     0x01
#define ZT_SIDE    0x02
#define ZT_ANGLE   0x04
#define ZT_BUTTONS 0x08
#define ZT_AIMING  0x10
#define ZT_DRIFT   0x20
#define DEMOMARKER 0x80 // demoend

static ticcmd_t oldcmd;

// For Metal Sonic and time attack ghosts
#define GZT_XYZ    0x01
#define GZT_MOMXY  0x02
#define GZT_MOMZ   0x04
#define GZT_ANGLE  0x08
// Not used for Metal Sonic
#define GZT_SPRITE 0x10 // Animation frame
#define GZT_EXTRA  0x20
#define GZT_NIGHTS 0x40 // NiGHTS Mode stuff!

// GZT_EXTRA flags
#define EZT_THOK   0x01 // Spawned a thok object
#define EZT_SPIN   0x02 // Because one type of thok object apparently wasn't enough
#define EZT_REV    0x03 // And two types wasn't enough either yet
#define EZT_THOKMASK 0x03
#define EZT_COLOR  0x04 // Changed color (Super transformation, Mario fireflowers/invulnerability, etc.)
#define EZT_FLIP   0x08 // Reversed gravity
#define EZT_SCALE  0x10 // Changed size
#define EZT_HIT    0x20 // Damaged a mobj
#define EZT_SPRITE 0x40 // Changed sprite set completely out of PLAY (NiGHTS, SOCs, whatever)

static mobj_t oldmetal, oldghost;

void G_SaveMetal(UINT8 **buffer)
{
	I_Assert(buffer != NULL && *buffer != NULL);

	WRITEUINT32(*buffer, metal_p - metalbuffer);
}

void G_LoadMetal(UINT8 **buffer)
{
	I_Assert(buffer != NULL && *buffer != NULL);

	G_DoPlayMetal();
	metal_p = metalbuffer + READUINT32(*buffer);
}

ticcmd_t *G_CopyTiccmd(ticcmd_t* dest, const ticcmd_t* src, const size_t n)
{
	return M_Memcpy(dest, src, n*sizeof(*src));
}

ticcmd_t *G_MoveTiccmd(ticcmd_t* dest, const ticcmd_t* src, const size_t n)
{
	size_t i;
	for (i = 0; i < n; i++)
	{
		dest[i].forwardmove = src[i].forwardmove;
		dest[i].sidemove = src[i].sidemove;
		dest[i].angleturn = SHORT(src[i].angleturn);
		dest[i].aiming = (INT16)SHORT(src[i].aiming);
		dest[i].buttons = (UINT16)SHORT(src[i].buttons);
		dest[i].driftturn = (INT16)SHORT(src[i].driftturn);
	}
	return dest;
}

void G_ReadDemoTiccmd(ticcmd_t *cmd, INT32 playernum)
{
	UINT8 ziptic;
	(void)playernum;

	if (!demo_p || !demo_start)
		return;
	ziptic = READUINT8(demo_p);

	if (ziptic & ZT_FWD)
		oldcmd.forwardmove = READSINT8(demo_p);
	if (ziptic & ZT_SIDE)
		oldcmd.sidemove = READSINT8(demo_p);
	if (ziptic & ZT_ANGLE)
		oldcmd.angleturn = READINT16(demo_p);
	if (ziptic & ZT_BUTTONS)
		oldcmd.buttons = (oldcmd.buttons & (BT_FORWARD|BT_BACKWARD)) | (READUINT16(demo_p) & ~(BT_FORWARD|BT_BACKWARD));
	if (ziptic & ZT_AIMING)
		oldcmd.aiming = READINT16(demo_p);
	if (ziptic & ZT_DRIFT)
		oldcmd.driftturn = READINT16(demo_p);

	G_CopyTiccmd(cmd, &oldcmd, 1);

	if (!(demoflags & DF_GHOST) && *demo_p == DEMOMARKER)
	{
		// end of demo data stream
		G_CheckDemoStatus();
		return;
	}
}

void G_WriteDemoTiccmd(ticcmd_t *cmd, INT32 playernum)
{
	char ziptic = 0;
	UINT8 *ziptic_p;
	(void)playernum;

	if (!demo_p)
		return;
	ziptic_p = demo_p++; // the ziptic, written at the end of this function

	if (cmd->forwardmove != oldcmd.forwardmove)
	{
		WRITEUINT8(demo_p,cmd->forwardmove);
		oldcmd.forwardmove = cmd->forwardmove;
		ziptic |= ZT_FWD;
	}

	if (cmd->sidemove != oldcmd.sidemove)
	{
		WRITEUINT8(demo_p,cmd->sidemove);
		oldcmd.sidemove = cmd->sidemove;
		ziptic |= ZT_SIDE;
	}

	if (cmd->angleturn != oldcmd.angleturn)
	{
		WRITEINT16(demo_p,cmd->angleturn);
		oldcmd.angleturn = cmd->angleturn;
		ziptic |= ZT_ANGLE;
	}

	if (cmd->buttons != oldcmd.buttons)
	{
		WRITEUINT16(demo_p,cmd->buttons);
		oldcmd.buttons = cmd->buttons;
		ziptic |= ZT_BUTTONS;
	}

	if (cmd->aiming != oldcmd.aiming)
	{
		WRITEINT16(demo_p,cmd->aiming);
		oldcmd.aiming = cmd->aiming;
		ziptic |= ZT_AIMING;
	}

	if (cmd->driftturn != oldcmd.driftturn)
	{
		WRITEINT16(demo_p,cmd->driftturn);
		oldcmd.driftturn = cmd->driftturn;
		ziptic |= ZT_DRIFT;
	}

	*ziptic_p = ziptic;

	// attention here for the ticcmd size!
	// latest demos with mouse aiming byte in ticcmd
	if (!(demoflags & DF_GHOST) && ziptic_p > demoend - 9)
	{
		G_CheckDemoStatus(); // no more space
		return;
	}
}

void G_GhostAddThok(void)
{
	if (!demorecording || !(demoflags & DF_GHOST))
		return;
	ghostext.flags = (ghostext.flags & ~EZT_THOKMASK) | EZT_THOK;
}

void G_GhostAddSpin(void)
{
	if (!demorecording || !(demoflags & DF_GHOST))
		return;
	ghostext.flags = (ghostext.flags & ~EZT_THOKMASK) | EZT_SPIN;
}

void G_GhostAddRev(void)
{
	if (!demorecording || !(demoflags & DF_GHOST))
		return;
	ghostext.flags = (ghostext.flags & ~EZT_THOKMASK) | EZT_REV;
}

void G_GhostAddFlip(void)
{
	if (!demorecording || !(demoflags & DF_GHOST))
		return;
	ghostext.flags |= EZT_FLIP;
}

void G_GhostAddColor(ghostcolor_t color)
{
	if (!demorecording || !(demoflags & DF_GHOST))
		return;
	if (ghostext.lastcolor == (UINT8)color)
	{
		ghostext.flags &= ~EZT_COLOR;
		return;
	}
	ghostext.flags |= EZT_COLOR;
	ghostext.color = (UINT8)color;
}

void G_GhostAddScale(fixed_t scale)
{
	if (!demorecording || !(demoflags & DF_GHOST))
		return;
	if (ghostext.lastscale == scale)
	{
		ghostext.flags &= ~EZT_SCALE;
		return;
	}
	ghostext.flags |= EZT_SCALE;
	ghostext.scale = scale;
}

void G_GhostAddHit(mobj_t *victim)
{
	if (!demorecording || !(demoflags & DF_GHOST))
		return;
	ghostext.flags |= EZT_HIT;
	ghostext.hits++;
	ghostext.hitlist = Z_Realloc(ghostext.hitlist, ghostext.hits * sizeof(mobj_t *), PU_LEVEL, NULL);
	ghostext.hitlist[ghostext.hits-1] = victim;
}

void G_WriteGhostTic(mobj_t *ghost)
{
	char ziptic = 0;
	UINT8 *ziptic_p;
	UINT32 i;
	UINT8 sprite;
	UINT8 frame;

	if (!demo_p)
		return;
	if (!(demoflags & DF_GHOST))
		return; // No ghost data to write.

	if (ghost->player && ghost->player->pflags & PF_NIGHTSMODE && ghost->tracer)
	{
		// We're talking about the NiGHTS thing, not the normal platforming thing!
		ziptic |= GZT_NIGHTS;
		ghost = ghost->tracer;
	}

	ziptic_p = demo_p++; // the ziptic, written at the end of this function

	#define MAXMOM (0xFFFF<<8)

	// GZT_XYZ is only useful if you've moved 256 FRACUNITS or more in a single tic.
	if (abs(ghost->x-oldghost.x) > MAXMOM
	|| abs(ghost->y-oldghost.y) > MAXMOM
	|| abs(ghost->z-oldghost.z) > MAXMOM)
	{
		oldghost.x = ghost->x;
		oldghost.y = ghost->y;
		oldghost.z = ghost->z;
		ziptic |= GZT_XYZ;
		WRITEFIXED(demo_p,oldghost.x);
		WRITEFIXED(demo_p,oldghost.y);
		WRITEFIXED(demo_p,oldghost.z);
	}
	else
	{
		// For moving normally:
		// Store one full byte of movement, plus one byte of fractional movement.
		INT16 momx = (INT16)((ghost->x-oldghost.x)>>8);
		INT16 momy = (INT16)((ghost->y-oldghost.y)>>8);
		if (momx != oldghost.momx
		|| momy != oldghost.momy)
		{
			oldghost.momx = momx;
			oldghost.momy = momy;
			ziptic |= GZT_MOMXY;
			WRITEINT16(demo_p,momx);
			WRITEINT16(demo_p,momy);
		}
		momx = (INT16)((ghost->z-oldghost.z)>>8);
		if (momx != oldghost.momz)
		{
			oldghost.momz = momx;
			ziptic |= GZT_MOMZ;
			WRITEINT16(demo_p,momx);
		}

		// This SHOULD set oldghost.x/y/z to match ghost->x/y/z
		// but it keeps the fractional loss of one byte,
		// so it will hopefully be made up for in future tics.
		oldghost.x += oldghost.momx<<8;
		oldghost.y += oldghost.momy<<8;
		oldghost.z += oldghost.momz<<8;
	}

	#undef MAXMOM

	// Only store the 8 most relevant bits of angle
	// because exact values aren't too easy to discern to begin with when only 8 angles have different sprites
	// and it does not affect this mode of movement at all anyway.
	if (ghost->angle>>24 != oldghost.angle)
	{
		oldghost.angle = ghost->angle>>24;
		ziptic |= GZT_ANGLE;
		WRITEUINT8(demo_p,oldghost.angle);
	}

	// Store the sprite frame.
	frame = ghost->frame & 0xFF;
	if (frame != oldghost.frame)
	{
		oldghost.frame = frame;
		ziptic |= GZT_SPRITE;
		WRITEUINT8(demo_p,oldghost.frame);
	}

	// Check for sprite set changes
	sprite = ghost->sprite;
	if (sprite != oldghost.sprite)
	{
		oldghost.sprite = sprite;
		ghostext.flags |= EZT_SPRITE;
	}

	if (ghostext.flags)
	{
		ziptic |= GZT_EXTRA;

		if (ghostext.color == ghostext.lastcolor)
			ghostext.flags &= ~EZT_COLOR;
		if (ghostext.scale == ghostext.lastscale)
			ghostext.flags &= ~EZT_SCALE;

		WRITEUINT8(demo_p,ghostext.flags);
		if (ghostext.flags & EZT_COLOR)
		{
			WRITEUINT8(demo_p,ghostext.color);
			ghostext.lastcolor = ghostext.color;
		}
		if (ghostext.flags & EZT_SCALE)
		{
			WRITEFIXED(demo_p,ghostext.scale);
			ghostext.lastscale = ghostext.scale;
		}
		if (ghostext.flags & EZT_HIT)
		{
			WRITEUINT16(demo_p,ghostext.hits);
			for (i = 0; i < ghostext.hits; i++)
			{
				mobj_t *mo = ghostext.hitlist[i];
				WRITEUINT32(demo_p,UINT32_MAX); // reserved for some method of determining exactly which mobj this is. (mobjnum doesn't work here.)
				WRITEUINT32(demo_p,mo->type);
				WRITEUINT16(demo_p,(UINT16)mo->health);
				WRITEFIXED(demo_p,mo->x);
				WRITEFIXED(demo_p,mo->y);
				WRITEFIXED(demo_p,mo->z);
				WRITEANGLE(demo_p,mo->angle);
			}
			Z_Free(ghostext.hitlist);
			ghostext.hits = 0;
			ghostext.hitlist = NULL;
		}
		if (ghostext.flags & EZT_SPRITE)
			WRITEUINT8(demo_p,sprite);
		ghostext.flags = 0;
	}

	*ziptic_p = ziptic;

	// attention here for the ticcmd size!
	// latest demos with mouse aiming byte in ticcmd
	if (demo_p >= demoend - (13 + 9))
	{
		G_CheckDemoStatus(); // no more space
		return;
	}
}

// Uses ghost data to do consistency checks on your position.
// This fixes desynchronising demos when fighting eggman.
void G_ConsGhostTic(void)
{
	UINT8 ziptic;
	UINT16 px,py,pz,gx,gy,gz;
	mobj_t *testmo;
	boolean nightsfail = false;

	if (!demo_p || !demo_start)
		return;
	if (!(demoflags & DF_GHOST))
		return; // No ghost data to use.

	testmo = players[0].mo;

	// Grab ghost data.
	ziptic = READUINT8(demo_p);
	if (ziptic & GZT_XYZ)
	{
		oldghost.x = READFIXED(demo_p);
		oldghost.y = READFIXED(demo_p);
		oldghost.z = READFIXED(demo_p);
	}
	else
	{
		if (ziptic & GZT_MOMXY)
		{
			oldghost.momx = READINT16(demo_p)<<8;
			oldghost.momy = READINT16(demo_p)<<8;
		}
		if (ziptic & GZT_MOMZ)
			oldghost.momz = READINT16(demo_p)<<8;
		oldghost.x += oldghost.momx;
		oldghost.y += oldghost.momy;
		oldghost.z += oldghost.momz;
	}
	if (ziptic & GZT_ANGLE)
		demo_p++;
	if (ziptic & GZT_SPRITE)
		demo_p++;
	if(ziptic & GZT_NIGHTS) {
		if (!testmo->player || !(testmo->player->pflags & PF_NIGHTSMODE) || !testmo->tracer)
			nightsfail = true;
		else
			testmo = testmo->tracer;
	}

	if (ziptic & GZT_EXTRA)
	{ // But wait, there's more!
		ziptic = READUINT8(demo_p);
		if (ziptic & EZT_COLOR)
			demo_p++;
		if (ziptic & EZT_SCALE)
			demo_p += sizeof(fixed_t);
		if (ziptic & EZT_HIT)
		{ // Resync mob damage.
			UINT16 i, count = READUINT16(demo_p);
			thinker_t *th;
			mobj_t *mobj;

			UINT32 type;
			UINT16 health;
			fixed_t x;
			fixed_t y;
			fixed_t z;

			for (i = 0; i < count; i++)
			{
				demo_p += 4; // reserved.
				type = READUINT32(demo_p);
				health = READUINT16(demo_p);
				x = READFIXED(demo_p);
				y = READFIXED(demo_p);
				z = READFIXED(demo_p);
				demo_p += sizeof(angle_t); // angle, unnecessary for cons.

				mobj = NULL;
				for (th = thinkercap.next; th != &thinkercap; th = th->next)
				{
					if (th->function.acp1 != (actionf_p1)P_MobjThinker)
						continue;
					mobj = (mobj_t *)th;
					if (mobj->type == (mobjtype_t)type && mobj->x == x && mobj->y == y && mobj->z == z)
						break;
					mobj = NULL; // wasn't this one, keep searching.
				}
				if (mobj && mobj->health != health) // Wasn't damaged?! This is desync! Fix it!
				{
					if (demosynced)
						CONS_Alert(CONS_WARNING, M_GetText("Demo playback has desynced!\n"));
					demosynced = false;
					P_DamageMobj(mobj, players[0].mo, players[0].mo, 1);
				}
			}
		}
		if (ziptic & EZT_SPRITE)
			demo_p++;
	}

	// Re-synchronise
	px = testmo->x>>FRACBITS;
	py = testmo->y>>FRACBITS;
	pz = testmo->z>>FRACBITS;
	gx = oldghost.x>>FRACBITS;
	gy = oldghost.y>>FRACBITS;
	gz = oldghost.z>>FRACBITS;

	if (nightsfail || px != gx || py != gy || pz != gz)
	{
		if (demosynced)
			CONS_Alert(CONS_WARNING, M_GetText("Demo playback has desynced!\n"));
		demosynced = false;

		P_UnsetThingPosition(testmo);
		testmo->x = oldghost.x;
		testmo->y = oldghost.y;
		P_SetThingPosition(testmo);
		testmo->z = oldghost.z;
	}

	if (*demo_p == DEMOMARKER)
	{
		// end of demo data stream
		G_CheckDemoStatus();
		return;
	}
}

void G_GhostTicker(void)
{
	demoghost *g,*p;
	for(g = ghosts, p = NULL; g; g = g->next)
	{
		// Skip normal demo data.
		UINT8 ziptic = READUINT8(g->p);
		if (ziptic & ZT_FWD)
			g->p++;
		if (ziptic & ZT_SIDE)
			g->p++;
		if (ziptic & ZT_ANGLE)
			g->p += 2;
		if (ziptic & ZT_BUTTONS)
			g->p += 2;
		if (ziptic & ZT_AIMING)
			g->p += 2;
		if (ziptic & ZT_DRIFT)
			g->p += 2;

		// Grab ghost data.
		ziptic = READUINT8(g->p);
		if (ziptic & GZT_XYZ)
		{
			g->oldmo.x = READFIXED(g->p);
			g->oldmo.y = READFIXED(g->p);
			g->oldmo.z = READFIXED(g->p);
		}
		else
		{
			if (ziptic & GZT_MOMXY)
			{
				g->oldmo.momx = READINT16(g->p)<<8;
				g->oldmo.momy = READINT16(g->p)<<8;
			}
			if (ziptic & GZT_MOMZ)
				g->oldmo.momz = READINT16(g->p)<<8;
			g->oldmo.x += g->oldmo.momx;
			g->oldmo.y += g->oldmo.momy;
			g->oldmo.z += g->oldmo.momz;
		}
		if (ziptic & GZT_ANGLE)
			g->oldmo.angle = READUINT8(g->p)<<24;
		if (ziptic & GZT_SPRITE)
			g->oldmo.frame = READUINT8(g->p);

		// Update ghost
		P_UnsetThingPosition(g->mo);
		g->mo->x = g->oldmo.x;
		g->mo->y = g->oldmo.y;
		g->mo->z = g->oldmo.z;
		P_SetThingPosition(g->mo);
		g->mo->angle = g->oldmo.angle;
		g->mo->frame = g->oldmo.frame | tr_trans30<<FF_TRANSSHIFT;

		if (ziptic & GZT_EXTRA)
		{ // But wait, there's more!
			ziptic = READUINT8(g->p);
			if (ziptic & EZT_COLOR)
			{
				g->color = READUINT8(g->p);
				switch(g->color)
				{
				default:
				case GHC_NORMAL: // Go back to skin color
					g->mo->color = g->oldmo.color;
					break;
				// Handled below
				case GHC_SUPER:
				case GHC_INVINCIBLE:
					break;
				case GHC_FIREFLOWER: // Fireflower
					g->mo->color = SKINCOLOR_WHITE;
					break;
				}
			}
			if (ziptic & EZT_FLIP)
				g->mo->eflags ^= MFE_VERTICALFLIP;
			if (ziptic & EZT_SCALE)
			{
				g->mo->destscale = READFIXED(g->p);
				if (g->mo->destscale != g->mo->scale)
					P_SetScale(g->mo, g->mo->destscale);
			}
			if (ziptic & EZT_THOKMASK)
			{ // Let's only spawn ONE of these per frame, thanks.
				mobj_t *mobj;
				INT32 type = -1;
				if (g->mo->skin)
				{
					skin_t *skin = (skin_t *)g->mo->skin;
					switch (ziptic & EZT_THOKMASK)
					{
					case EZT_THOK:
						type = skin->thokitem < 0 ? (UINT32)mobjinfo[MT_PLAYER].painchance : (UINT32)skin->thokitem;
						break;
					case EZT_SPIN:
						type = skin->spinitem < 0 ? (UINT32)mobjinfo[MT_PLAYER].damage : (UINT32)skin->spinitem;
						break;
					case EZT_REV:
						type = skin->revitem < 0 ? (UINT32)mobjinfo[MT_PLAYER].raisestate : (UINT32)skin->revitem;
						break;
					}
				}
				if (type == MT_GHOST)
				{
					mobj = P_SpawnGhostMobj(g->mo); // does a large portion of the work for us
					mobj->frame = (mobj->frame & ~FF_FRAMEMASK)|tr_trans60<<FF_TRANSSHIFT; // P_SpawnGhostMobj sets trans50, we want trans60
				}
				else
				{
					mobj = P_SpawnMobj(g->mo->x, g->mo->y, g->mo->z - FixedDiv(FixedMul(g->mo->info->height, g->mo->scale) - g->mo->height,3*FRACUNIT), MT_THOK);
					mobj->sprite = states[mobjinfo[type].spawnstate].sprite;
					mobj->frame = (states[mobjinfo[type].spawnstate].frame & FF_FRAMEMASK) | tr_trans60<<FF_TRANSSHIFT;
					mobj->tics = -1; // nope.
					mobj->color = g->mo->color;
					if (g->mo->eflags & MFE_VERTICALFLIP)
					{
						mobj->flags2 |= MF2_OBJECTFLIP;
						mobj->eflags |= MFE_VERTICALFLIP;
					}
					P_SetScale(mobj, g->mo->scale);
					mobj->destscale = g->mo->scale;
				}
				mobj->floorz = mobj->z;
				mobj->ceilingz = mobj->z+mobj->height;
				P_UnsetThingPosition(mobj);
				mobj->flags = MF_NOBLOCKMAP|MF_NOCLIP|MF_NOCLIPHEIGHT|MF_NOGRAVITY; // make an ATTEMPT to curb crazy SOCs fucking stuff up...
				P_SetThingPosition(mobj);
				mobj->fuse = 8;
				P_SetTarget(&mobj->target, g->mo);
			}
			if (ziptic & EZT_HIT)
			{ // Spawn hit poofs for killing things!
				UINT16 i, count = READUINT16(g->p), health;
				UINT32 type;
				fixed_t x,y,z;
				angle_t angle;
				mobj_t *poof;
				for (i = 0; i < count; i++)
				{
					g->p += 4; // reserved
					type = READUINT32(g->p);
					health = READUINT16(g->p);
					x = READFIXED(g->p);
					y = READFIXED(g->p);
					z = READFIXED(g->p);
					angle = READANGLE(g->p);
					if (!(mobjinfo[type].flags & MF_SHOOTABLE)
					|| !(mobjinfo[type].flags & (MF_ENEMY|MF_MONITOR))
					|| health != 0 || i >= 4) // only spawn for the first 4 hits per frame, to prevent ghosts from splode-spamming too bad.
						continue;
					poof = P_SpawnMobj(x, y, z, MT_GHOST);
					poof->angle = angle;
					poof->flags = MF_NOBLOCKMAP|MF_NOCLIP|MF_NOCLIPHEIGHT|MF_NOGRAVITY; // make an ATTEMPT to curb crazy SOCs fucking stuff up...
					poof->health = 0;
					P_SetMobjStateNF(poof, S_XPLD1);
				}
			}
			if (ziptic & EZT_SPRITE)
				g->mo->sprite = READUINT8(g->p);
		}

		// Tick ghost colors (Super and Mario Invincibility flashing)
		switch(g->color)
		{
		case GHC_SUPER: // Super Sonic (P_DoSuperStuff)
			g->mo->color = SKINCOLOR_SUPER1;
			g->mo->color += abs( ( (signed)( (unsigned)leveltime >> 1 ) % 9) - 4);
			break;
		case GHC_INVINCIBLE: // Mario invincibility (P_CheckInvincibilityTimer)
			g->mo->color = (UINT8)(leveltime % MAXSKINCOLORS);
			break;
		default:
			break;
		}

		// Demo ends after ghost data.
		if (*g->p == DEMOMARKER)
		{
			g->mo->momx = g->mo->momy = g->mo->momz = 0;
			if (p)
				p->next = g->next;
			else
				ghosts = g->next;
			Z_Free(g);
			continue;
		}
		p = g;
	}
}

void G_ReadMetalTic(mobj_t *metal)
{
	UINT8 ziptic;
	UINT16 speed;
	UINT8 statetype;

	if (!metal_p)
		return;
	ziptic = READUINT8(metal_p);

	// Read changes from the tic
	if (ziptic & GZT_XYZ)
	{
		P_TeleportMove(metal, READFIXED(metal_p), READFIXED(metal_p), READFIXED(metal_p));
		oldmetal.x = metal->x;
		oldmetal.y = metal->y;
		oldmetal.z = metal->z;
	}
	else
	{
		if (ziptic & GZT_MOMXY)
		{
			oldmetal.momx = READINT16(metal_p)<<8;
			oldmetal.momy = READINT16(metal_p)<<8;
		}
		if (ziptic & GZT_MOMZ)
			oldmetal.momz = READINT16(metal_p)<<8;
		oldmetal.x += oldmetal.momx;
		oldmetal.y += oldmetal.momy;
		oldmetal.z += oldmetal.momz;
	}
	if (ziptic & GZT_ANGLE)
		oldmetal.angle = READUINT8(metal_p)<<24;
	if (ziptic & GZT_SPRITE)
		metal_p++; // Currently unused. (Metal Sonic figures out what he's doing his own damn self.)

	// Set movement, position, and angle
	// oldmetal contains where you're supposed to be.
	metal->momx = oldmetal.momx;
	metal->momy = oldmetal.momy;
	metal->momz = oldmetal.momz;
	P_UnsetThingPosition(metal);
	metal->x = oldmetal.x;
	metal->y = oldmetal.y;
	metal->z = oldmetal.z;
	P_SetThingPosition(metal);
	metal->angle = oldmetal.angle;

	if (ziptic & GZT_EXTRA)
	{ // But wait, there's more!
		ziptic = READUINT8(metal_p);
		if (ziptic & EZT_FLIP)
			metal->eflags ^= MFE_VERTICALFLIP;
		if (ziptic & EZT_SCALE)
		{
			metal->destscale = READFIXED(metal_p);
			if (metal->destscale != metal->scale)
				P_SetScale(metal, metal->destscale);
		}
	}

	// Calculates player's speed based on distance-of-a-line formula
	speed = FixedDiv(P_AproxDistance(oldmetal.momx, oldmetal.momy), metal->scale)>>FRACBITS;

	// Use speed to decide an appropriate state
	if (speed > 28) // default skin runspeed
		statetype = 2;
	else if (speed > 1) // stopspeed
		statetype = 1;
	else
		statetype = 0;

	// Set state
	if (statetype != metal->threshold)
	{
		switch (statetype)
		{
		case 2: // run
			P_SetMobjState(metal,metal->info->meleestate);
			break;
		case 1: // walk
			P_SetMobjState(metal,metal->info->seestate);
			break;
		default: // stand
			P_SetMobjState(metal,metal->info->spawnstate);
			break;
		}
		metal->threshold = statetype;
	}

	// TODO: Modify state durations based on movement speed, similar to players?

	if (*metal_p == DEMOMARKER)
	{
		// end of demo data stream
		G_StopMetalDemo();
		return;
	}
}

void G_WriteMetalTic(mobj_t *metal)
{
	UINT8 ziptic = 0;
	UINT8 *ziptic_p;

	if (!demo_p) // demo_p will be NULL until the race start linedef executor is triggered!
		return;

	ziptic_p = demo_p++; // the ziptic, written at the end of this function

	#define MAXMOM (0xFFFF<<8)

	// GZT_XYZ is only useful if you've moved 256 FRACUNITS or more in a single tic.
	if (abs(metal->x-oldmetal.x) > MAXMOM
	|| abs(metal->y-oldmetal.y) > MAXMOM
	|| abs(metal->z-oldmetal.z) > MAXMOM)
	{
		oldmetal.x = metal->x;
		oldmetal.y = metal->y;
		oldmetal.z = metal->z;
		WRITEFIXED(demo_p,oldmetal.x);
		WRITEFIXED(demo_p,oldmetal.y);
		WRITEFIXED(demo_p,oldmetal.z);
		ziptic |= GZT_XYZ;
	}
	else
	{
		// For moving normally:
		// Store one full byte of movement, plus one byte of fractional movement.
		INT16 momx = (INT16)((metal->x-oldmetal.x)>>8);
		INT16 momy = (INT16)((metal->y-oldmetal.y)>>8);
		if (momx != oldmetal.momx
		|| momy != oldmetal.momy)
		{
			oldmetal.momx = momx;
			oldmetal.momy = momy;
			WRITEINT16(demo_p,momx);
			WRITEINT16(demo_p,momy);
			ziptic |= GZT_MOMXY;
		}
		momx = (INT16)((metal->z-oldmetal.z)>>8);
		if (momx != oldmetal.momz)
		{
			oldmetal.momz = momx;
			WRITEINT16(demo_p,momx);
			ziptic |= GZT_MOMZ;
		}

		// This SHOULD set oldmetal.x/y/z to match metal->x/y/z
		// but it keeps the fractional loss of one byte,
		// so it will hopefully be made up for in future tics.
		oldmetal.x += oldmetal.momx<<8;
		oldmetal.y += oldmetal.momy<<8;
		oldmetal.z += oldmetal.momz<<8;
	}

	#undef MAXMOM

	// Only store the 8 most relevant bits of angle
	// because exact values aren't too easy to discern to begin with when only 8 angles have different sprites
	// and it does not affect movement at all anyway.
	if (metal->angle>>24 != oldmetal.angle)
	{
		oldmetal.angle = metal->angle>>24;
		WRITEUINT8(demo_p,oldmetal.angle);
		ziptic |= GZT_ANGLE;
	}

	// Metal Sonic does not need our state changes.
	// ... currently.

	{
		UINT8 *exttic_p = NULL;
		UINT8 exttic = 0;
		if ((metal->eflags & MFE_VERTICALFLIP) != (oldmetal.eflags & MFE_VERTICALFLIP))
		{
			if (!exttic_p)
				exttic_p = demo_p++;
			exttic |= EZT_FLIP;
			oldmetal.eflags ^= MFE_VERTICALFLIP;
		}
		if (metal->scale != oldmetal.scale)
		{
			if (!exttic_p)
				exttic_p = demo_p++;
			exttic |= EZT_SCALE;
			WRITEFIXED(demo_p,metal->scale);
			oldmetal.scale = metal->scale;
		}
		if (exttic_p)
		{
			*exttic_p = exttic;
			ziptic |= GZT_EXTRA;
		}
	}

	*ziptic_p = ziptic;

	// attention here for the ticcmd size!
	// latest demos with mouse aiming byte in ticcmd
	if (demo_p >= demoend - 32)
	{
		G_StopMetalRecording(); // no more space
		return;
	}
}

//
// G_RecordDemo
//
void G_RecordDemo(const char *name)
{
	INT32 maxsize;

	strcpy(demoname, name);
	strcat(demoname, ".lmp");
	maxsize = 1024*1024;
	if (M_CheckParm("-maxdemo") && M_IsNextParm())
		maxsize = atoi(M_GetNextParm()) * 1024;
//	if (demobuffer)
//		free(demobuffer);
	demo_p = NULL;
	demobuffer = malloc(maxsize);
	demoend = demobuffer + maxsize;

	demorecording = true;
}

void G_RecordMetal(void)
{
	INT32 maxsize;
	maxsize = 1024*1024;
	if (M_CheckParm("-maxdemo") && M_IsNextParm())
		maxsize = atoi(M_GetNextParm()) * 1024;
	demo_p = NULL;
	demobuffer = malloc(maxsize);
	demoend = demobuffer + maxsize;
	metalrecording = true;
}

void G_BeginRecording(void)
{
	UINT8 i;
	char name[16];
	player_t *player = &players[consoleplayer];

	if (demo_p)
		return;
	memset(name,0,sizeof(name));

	demo_p = demobuffer;
	demoflags = DF_GHOST|(modeattacking<<DF_ATTACKSHIFT);

	// Setup header.
	M_Memcpy(demo_p, DEMOHEADER, 12); demo_p += 12;
	WRITEUINT8(demo_p,VERSION);
	WRITEUINT8(demo_p,SUBVERSION);
	WRITEUINT16(demo_p,DEMOVERSION);

	// demo checksum
	demo_p += 16;

	// game data
	M_Memcpy(demo_p, "PLAY", 4); demo_p += 4;
	WRITEINT16(demo_p,gamemap);
	M_Memcpy(demo_p, mapmd5, 16); demo_p += 16;

	WRITEUINT8(demo_p,demoflags);
	switch ((demoflags & DF_ATTACKMASK)>>DF_ATTACKSHIFT)
	{
	case ATTACKING_NONE: // 0
		break;
	case ATTACKING_RECORD: // 1
		demotime_p = demo_p;
		WRITEUINT32(demo_p,UINT32_MAX); // time
		WRITEUINT32(demo_p,UINT32_MAX); // lap
		break;
	/*case ATTACKING_NIGHTS: // 2
		demotime_p = demo_p;
		WRITEUINT32(demo_p,UINT32_MAX); // time
		WRITEUINT32(demo_p,0); // score
		break;*/
	default: // 3
		break;
	}

	WRITEUINT32(demo_p,P_GetInitSeed());

	// Name
	for (i = 0; i < 16 && cv_playername.string[i]; i++)
		name[i] = cv_playername.string[i];
	for (; i < 16; i++)
		name[i] = '\0';
	M_Memcpy(demo_p,name,16);
	demo_p += 16;

	// Skin
	for (i = 0; i < 16 && cv_skin.string[i]; i++)
		name[i] = cv_skin.string[i];
	for (; i < 16; i++)
		name[i] = '\0';
	M_Memcpy(demo_p,name,16);
	demo_p += 16;

	// Color
	for (i = 0; i < 16 && cv_playercolor.string[i]; i++)
		name[i] = cv_playercolor.string[i];
	for (; i < 16; i++)
		name[i] = '\0';
	M_Memcpy(demo_p,name,16);
	demo_p += 16;

	// Stats
	WRITEUINT8(demo_p,player->charability);
	WRITEUINT8(demo_p,player->charability2);
	WRITEUINT8(demo_p,player->actionspd>>FRACBITS);
	WRITEUINT8(demo_p,player->mindash>>FRACBITS);
	WRITEUINT8(demo_p,player->maxdash>>FRACBITS);
	// SRB2kart
	WRITEUINT8(demo_p,player->kartspeed);
	WRITEUINT8(demo_p,player->kartweight);
	//
	WRITEUINT8(demo_p,player->normalspeed>>FRACBITS);
	WRITEUINT8(demo_p,player->runspeed>>FRACBITS);
	WRITEUINT8(demo_p,player->thrustfactor);
	WRITEUINT8(demo_p,player->accelstart);
	WRITEUINT8(demo_p,player->acceleration);

	// Trying to convert it back to % causes demo desync due to precision loss.
	// Don't do it.
	WRITEFIXED(demo_p, player->jumpfactor);

	// Save netvar data (SONICCD, etc)
	CV_SaveNetVars(&demo_p);

	memset(&oldcmd,0,sizeof(oldcmd));
	memset(&oldghost,0,sizeof(oldghost));
	memset(&ghostext,0,sizeof(ghostext));
	ghostext.lastcolor = ghostext.color = GHC_NORMAL;
	ghostext.lastscale = ghostext.scale = FRACUNIT;

	if (player->mo)
	{
		oldghost.x = player->mo->x;
		oldghost.y = player->mo->y;
		oldghost.z = player->mo->z;
		oldghost.angle = player->mo->angle;

		// preticker started us gravity flipped
		if (player->mo->eflags & MFE_VERTICALFLIP)
			ghostext.flags |= EZT_FLIP;
	}
}

void G_BeginMetal(void)
{
	mobj_t *mo = players[consoleplayer].mo;

	if (demo_p)
		return;

	demo_p = demobuffer;

	// Write header.
	M_Memcpy(demo_p, DEMOHEADER, 12); demo_p += 12;
	WRITEUINT8(demo_p,VERSION);
	WRITEUINT8(demo_p,SUBVERSION);
	WRITEUINT16(demo_p,DEMOVERSION);

	// demo checksum
	demo_p += 16;

	M_Memcpy(demo_p, "METL", 4); demo_p += 4;

	// Set up our memory.
	memset(&oldmetal,0,sizeof(oldmetal));
	oldmetal.x = mo->x;
	oldmetal.y = mo->y;
	oldmetal.z = mo->z;
	oldmetal.angle = mo->angle;
}

void G_SetDemoTime(UINT32 ptime, UINT32 plap)
{
	if (!demorecording || !demotime_p)
		return;
	if (demoflags & DF_RECORDATTACK)
	{
		WRITEUINT32(demotime_p, ptime);
		WRITEUINT32(demotime_p, plap);
		demotime_p = NULL;
	}
	/*else if (demoflags & DF_NIGHTSATTACK)
	{
		WRITEUINT32(demotime_p, ptime);
		WRITEUINT32(demotime_p, pscore);
		demotime_p = NULL;
	}*/
}

// Returns bitfield:
// 1 == new demo has lower time
// 2 == new demo has higher score
// 4 == new demo has higher rings
UINT8 G_CmpDemoTime(char *oldname, char *newname)
{
	UINT8 *buffer,*p;
	UINT8 flags;
	UINT32 oldtime, newtime, oldlap, newlap;
	UINT16 oldversion;
	size_t bufsize ATTRUNUSED;
	UINT8 c;
	UINT16 s ATTRUNUSED;
	UINT8 aflags = 0;

	// load the new file
	FIL_DefaultExtension(newname, ".lmp");
	bufsize = FIL_ReadFile(newname, &buffer);
	I_Assert(bufsize != 0);
	p = buffer;

	// read demo header
	I_Assert(!memcmp(p, DEMOHEADER, 12));
	p += 12; // DEMOHEADER
	c = READUINT8(p); // VERSION
	I_Assert(c == VERSION);
	c = READUINT8(p); // SUBVERSION
	I_Assert(c == SUBVERSION);
	s = READUINT16(p);
	I_Assert(s == DEMOVERSION);
	p += 16; // demo checksum
	I_Assert(!memcmp(p, "PLAY", 4));
	p += 4; // PLAY
	p += 2; // gamemap
	p += 16; // map md5
	flags = READUINT8(p); // demoflags

	aflags = flags & (DF_RECORDATTACK|DF_NIGHTSATTACK);
	I_Assert(aflags);
	if (flags & DF_RECORDATTACK)
	{
		newtime = READUINT32(p);
		newlap = READUINT32(p);
	}
	/*else if (flags & DF_NIGHTSATTACK)
	{
		newtime = READUINT32(p);
		newscore = READUINT32(p);
	}*/
	else // appease compiler
		return 0;

	Z_Free(buffer);

	// load old file
	FIL_DefaultExtension(oldname, ".lmp");
	if (!FIL_ReadFile(oldname, &buffer))
	{
		CONS_Alert(CONS_ERROR, M_GetText("Failed to read file '%s'.\n"), oldname);
		return UINT8_MAX;
	}
	p = buffer;

	// read demo header
	if (memcmp(p, DEMOHEADER, 12))
	{
		CONS_Alert(CONS_NOTICE, M_GetText("File '%s' invalid format. It will be overwritten.\n"), oldname);
		Z_Free(buffer);
		return UINT8_MAX;
	} p += 12; // DEMOHEADER
	p++; // VERSION
	p++; // SUBVERSION
	oldversion = READUINT16(p);
	switch(oldversion) // demoversion
	{
	case DEMOVERSION: // latest always supported
		break;
	// too old, cannot support.
	default:
		CONS_Alert(CONS_NOTICE, M_GetText("File '%s' invalid format. It will be overwritten.\n"), oldname);
		Z_Free(buffer);
		return UINT8_MAX;
	}
	p += 16; // demo checksum
	if (memcmp(p, "PLAY", 4))
	{
		CONS_Alert(CONS_NOTICE, M_GetText("File '%s' invalid format. It will be overwritten.\n"), oldname);
		Z_Free(buffer);
		return UINT8_MAX;
	} p += 4; // "PLAY"
	p += 2; // gamemap
	p += 16; // mapmd5
	flags = READUINT8(p);
	if (!(flags & aflags))
	{
		CONS_Alert(CONS_NOTICE, M_GetText("File '%s' not from same game mode. It will be overwritten.\n"), oldname);
		Z_Free(buffer);
		return UINT8_MAX;
	}
	if (flags & DF_RECORDATTACK)
	{
		oldtime = READUINT32(p);
		oldlap = READUINT32(p);
	}
	/*else if (flags & DF_NIGHTSATTACK)
	{
		oldtime = READUINT32(p);
		oldscore = READUINT32(p);
	}*/
	else // appease compiler
		return UINT8_MAX;

	Z_Free(buffer);

	c = 0;
	if (newtime < oldtime
	|| (newtime == oldtime && (newlap < oldlap)))
		c |= 1; // Better time
	if (newlap < oldlap
	|| (newlap == oldlap && newtime < oldtime))
		c |= 1<<1; // Better lap time
	return c;
}

//
// G_PlayDemo
//
void G_DeferedPlayDemo(const char *name)
{
	COM_BufAddText("playdemo \"");
	COM_BufAddText(name);
	COM_BufAddText("\"\n");
}

//
// Start a demo from a .LMP file or from a wad resource
//
void G_DoPlayDemo(char *defdemoname)
{
	UINT8 i;
	lumpnum_t l;
	char skin[17],color[17],*n,*pdemoname;
	UINT8 version,subversion,charability,charability2,kartspeed,kartweight,thrustfactor,accelstart,acceleration;
	UINT32 randseed;
	fixed_t actionspd,mindash,maxdash,normalspeed,runspeed,jumpfactor;
	char msg[1024];

	skin[16] = '\0';
	color[16] = '\0';

	n = defdemoname+strlen(defdemoname);
	while (*n != '/' && *n != '\\' && n != defdemoname)
		n--;
	if (n != defdemoname)
		n++;
	pdemoname = ZZ_Alloc(strlen(n)+1);
	strcpy(pdemoname,n);

	// Internal if no extension, external if one exists
	if (FIL_CheckExtension(defdemoname))
	{
		//FIL_DefaultExtension(defdemoname, ".lmp");
		if (!FIL_ReadFile(defdemoname, &demobuffer))
		{
			snprintf(msg, 1024, M_GetText("Failed to read file '%s'.\n"), defdemoname);
			CONS_Alert(CONS_ERROR, "%s", msg);
			gameaction = ga_nothing;
			M_StartMessage(msg, NULL, MM_NOTHING);
			return;
		}
		demo_p = demobuffer;
	}
	// load demo resource from WAD
	else if ((l = W_CheckNumForName(defdemoname)) == LUMPERROR)
	{
		snprintf(msg, 1024, M_GetText("Failed to read lump '%s'.\n"), defdemoname);
		CONS_Alert(CONS_ERROR, "%s", msg);
		gameaction = ga_nothing;
		M_StartMessage(msg, NULL, MM_NOTHING);
		return;
	}
	else // it's an internal demo
		demobuffer = demo_p = W_CacheLumpNum(l, PU_STATIC);

	// read demo header
	gameaction = ga_nothing;
	demoplayback = true;
	if (memcmp(demo_p, DEMOHEADER, 12))
	{
		snprintf(msg, 1024, M_GetText("%s is not a SRB2Kart replay file.\n"), pdemoname);
		CONS_Alert(CONS_ERROR, "%s", msg);
		M_StartMessage(msg, NULL, MM_NOTHING);
		Z_Free(pdemoname);
		Z_Free(demobuffer);
		demoplayback = false;
		titledemo = false;
		return;
	}
	demo_p += 12; // DEMOHEADER

	version = READUINT8(demo_p);
	subversion = READUINT8(demo_p);
	demoversion = READUINT16(demo_p);
	switch(demoversion)
	{
	case DEMOVERSION: // latest always supported
		break;
	// too old, cannot support.
	default:
		snprintf(msg, 1024, M_GetText("%s is an incompatible replay format and cannot be played.\n"), pdemoname);
		CONS_Alert(CONS_ERROR, "%s", msg);
		M_StartMessage(msg, NULL, MM_NOTHING);
		Z_Free(pdemoname);
		Z_Free(demobuffer);
		demoplayback = false;
		titledemo = false;
		return;
	}
	demo_p += 16; // demo checksum
	if (memcmp(demo_p, "PLAY", 4))
	{
		snprintf(msg, 1024, M_GetText("%s is the wrong type of recording and cannot be played.\n"), pdemoname);
		CONS_Alert(CONS_ERROR, "%s", msg);
		M_StartMessage(msg, NULL, MM_NOTHING);
		Z_Free(pdemoname);
		Z_Free(demobuffer);
		demoplayback = false;
		titledemo = false;
		return;
	}
	demo_p += 4; // "PLAY"
	gamemap = READINT16(demo_p);
	demo_p += 16; // mapmd5

	demoflags = READUINT8(demo_p);
	modeattacking = (demoflags & DF_ATTACKMASK)>>DF_ATTACKSHIFT;
	CON_ToggleOff();

	hu_demotime = UINT32_MAX;
	hu_demolap = UINT32_MAX;

	switch (modeattacking)
	{
	case ATTACKING_NONE: // 0
		break;
	case ATTACKING_RECORD: // 1
		hu_demotime  = READUINT32(demo_p);
		hu_demolap  = READUINT32(demo_p);
		break;
	/*case ATTACKING_NIGHTS: // 2
		hu_demotime  = READUINT32(demo_p);
		hu_demoscore = READUINT32(demo_p);
		break;*/
	default: // 3
		modeattacking = ATTACKING_NONE;
		break;
	}

	// Random seed
	randseed = READUINT32(demo_p);

	// Player name
	M_Memcpy(player_names[0],demo_p,16);
	demo_p += 16;

	// Skin
	M_Memcpy(skin,demo_p,16);
	demo_p += 16;

	// Color
	M_Memcpy(color,demo_p,16);
	demo_p += 16;

	charability = READUINT8(demo_p);
	charability2 = READUINT8(demo_p);
	actionspd = (fixed_t)READUINT8(demo_p)<<FRACBITS;
	mindash = (fixed_t)READUINT8(demo_p)<<FRACBITS;
	maxdash = (fixed_t)READUINT8(demo_p)<<FRACBITS;
	// SRB2kart
	kartspeed = READUINT8(demo_p);
	kartweight = READUINT8(demo_p);
	//
	normalspeed = (fixed_t)READUINT8(demo_p)<<FRACBITS;
	runspeed = (fixed_t)READUINT8(demo_p)<<FRACBITS;
	thrustfactor = READUINT8(demo_p);
	accelstart = READUINT8(demo_p);
	acceleration = READUINT8(demo_p);
	jumpfactor = READFIXED(demo_p);

	// net var data
	CV_LoadNetVars(&demo_p);

	// Sigh ... it's an empty demo.
	if (*demo_p == DEMOMARKER)
	{
		snprintf(msg, 1024, M_GetText("%s contains no data to be played.\n"), pdemoname);
		CONS_Alert(CONS_ERROR, "%s", msg);
		M_StartMessage(msg, NULL, MM_NOTHING);
		Z_Free(pdemoname);
		Z_Free(demobuffer);
		demoplayback = false;
		titledemo = false;
		return;
	}

	Z_Free(pdemoname);

	memset(&oldcmd,0,sizeof(oldcmd));
	memset(&oldghost,0,sizeof(oldghost));

	if (VERSION != version || SUBVERSION != subversion)
		CONS_Alert(CONS_WARNING, M_GetText("Demo version does not match game version. Desyncs may occur.\n"));

	// console warning messages
	demosynced = true;

	// didn't start recording right away.
	demo_start = false;

#ifdef HAVE_BLUA
	LUAh_MapChange();
#endif
	displayplayer = consoleplayer = 0;
	memset(playeringame,0,sizeof(playeringame));
	playeringame[0] = true;
	P_SetRandSeed(randseed);
	G_InitNew(false, G_BuildMapName(gamemap), true, true); // Doesn't matter whether you reset or not here, given changes to resetplayer.

	// Set skin
	SetPlayerSkin(0, skin);

	// Set color
	for (i = 0; i < MAXSKINCOLORS; i++)
		if (!stricmp(KartColor_Names[i],color))				// SRB2kart
		{
			players[0].skincolor = i;
			break;
		}
	CV_StealthSetValue(&cv_playercolor, players[0].skincolor);
	if (players[0].mo)
	{
		players[0].mo->color = players[0].skincolor;
		oldghost.x = players[0].mo->x;
		oldghost.y = players[0].mo->y;
		oldghost.z = players[0].mo->z;
	}

	// Set saved attribute values
	// No cheat checking here, because even if they ARE wrong...
	// it would only break the replay if we clipped them.
	players[0].charability = charability;
	players[0].charability2 = charability2;
	players[0].actionspd = actionspd;
	players[0].mindash = mindash;
	players[0].maxdash = maxdash;
	// SRB2kart
	players[0].kartspeed = kartspeed;
	players[0].kartweight = kartweight;
	//
	players[0].normalspeed = normalspeed;
	players[0].runspeed = runspeed;
	players[0].thrustfactor = thrustfactor;
	players[0].accelstart = accelstart;
	players[0].acceleration = acceleration;
	players[0].jumpfactor = jumpfactor;

	demo_start = true;
}

void G_AddGhost(char *defdemoname)
{
	INT32 i;
	lumpnum_t l;
	char name[17],skin[17],color[17],*n,*pdemoname,md5[16];
	demoghost *gh;
	UINT8 flags;
	UINT8 *buffer,*p;
	mapthing_t *mthing;
	UINT16 count, ghostversion;

	name[16] = '\0';
	skin[16] = '\0';
	color[16] = '\0';

	n = defdemoname+strlen(defdemoname);
	while (*n != '/' && *n != '\\' && n != defdemoname)
		n--;
	if (n != defdemoname)
		n++;
	pdemoname = ZZ_Alloc(strlen(n)+1);
	strcpy(pdemoname,n);

	// Internal if no extension, external if one exists
	if (FIL_CheckExtension(defdemoname))
	{
		//FIL_DefaultExtension(defdemoname, ".lmp");
		if (!FIL_ReadFileTag(defdemoname, &buffer, PU_LEVEL))
		{
			CONS_Alert(CONS_ERROR, M_GetText("Failed to read file '%s'.\n"), defdemoname);
			Z_Free(pdemoname);
			return;
		}
		p = buffer;
	}
	// load demo resource from WAD
	else if ((l = W_CheckNumForName(defdemoname)) == LUMPERROR)
	{
		CONS_Alert(CONS_ERROR, M_GetText("Failed to read lump '%s'.\n"), defdemoname);
		Z_Free(pdemoname);
		return;
	}
	else // it's an internal demo
		buffer = p = W_CacheLumpNum(l, PU_LEVEL);

	// read demo header
	if (memcmp(p, DEMOHEADER, 12))
	{
		CONS_Alert(CONS_NOTICE, M_GetText("Ghost %s: Not a SRB2Kart replay.\n"), pdemoname);
		Z_Free(pdemoname);
		Z_Free(buffer);
		return;
	} p += 12; // DEMOHEADER
	p++; // VERSION
	p++; // SUBVERSION
	ghostversion = READUINT16(p);
	switch(ghostversion)
	{
	case DEMOVERSION: // latest always supported
		break;
	// too old, cannot support.
	default:
		CONS_Alert(CONS_NOTICE, M_GetText("Ghost %s: Demo version incompatible.\n"), pdemoname);
		Z_Free(pdemoname);
		Z_Free(buffer);
		return;
	}
	M_Memcpy(md5, p, 16); p += 16; // demo checksum
	for (gh = ghosts; gh; gh = gh->next)
		if (!memcmp(md5, gh->checksum, 16)) // another ghost in the game already has this checksum?
		{ // Don't add another one, then!
			CONS_Debug(DBG_SETUP, "Rejecting duplicate ghost %s (MD5 was matched)\n", pdemoname);
			Z_Free(pdemoname);
			Z_Free(buffer);
			return;
		}
	if (memcmp(p, "PLAY", 4))
	{
		CONS_Alert(CONS_NOTICE, M_GetText("Ghost %s: Demo format unacceptable.\n"), pdemoname);
		Z_Free(pdemoname);
		Z_Free(buffer);
		return;
	} p += 4; // "PLAY"
	p += 2; // gamemap
	p += 16; // mapmd5 (possibly check for consistency?)
	flags = READUINT8(p);
	if (!(flags & DF_GHOST))
	{
		CONS_Alert(CONS_NOTICE, M_GetText("Ghost %s: No ghost data in this demo.\n"), pdemoname);
		Z_Free(pdemoname);
		Z_Free(buffer);
		return;
	}
	switch ((flags & DF_ATTACKMASK)>>DF_ATTACKSHIFT)
	{
	case ATTACKING_NONE: // 0
		break;
	case ATTACKING_RECORD: // 1
		p += 8; // demo time, lap
		break;
	/*case ATTACKING_NIGHTS: // 2
		p += 8; // demo time left, score
		break;*/
	default: // 3
		break;
	}

	p += 4; // random seed

	// Player name (TODO: Display this somehow if it doesn't match cv_playername!)
	M_Memcpy(name, p,16);
	p += 16;

	// Skin
	M_Memcpy(skin, p,16);
	p += 16;

	// Color
	M_Memcpy(color, p,16);
	p += 16;

	// Ghosts do not have a player structure to put this in.
	p++; // charability
	p++; // charability2
	p++; // actionspd
	p++; // mindash
	p++; // maxdash
	// SRB2kart
	p++; // kartspeed
	p++; // kartweight
	//
	p++; // normalspeed
	p++; // runspeed
	p++; // thrustfactor
	p++; // accelstart
	p++; // acceleration
	p += 4; // jumpfactor

	// net var data
	count = READUINT16(p);
	while (count--)
	{
		p += 2;
		SKIPSTRING(p);
		p++;
	}

	if (*p == DEMOMARKER)
	{
		CONS_Alert(CONS_NOTICE, M_GetText("Failed to add ghost %s: Replay is empty.\n"), pdemoname);
		Z_Free(pdemoname);
		Z_Free(buffer);
		return;
	}

	gh = Z_Calloc(sizeof(demoghost), PU_LEVEL, NULL);
	gh->next = ghosts;
	gh->buffer = buffer;
	M_Memcpy(gh->checksum, md5, 16);
	gh->p = p;

	ghosts = gh;

	gh->version = ghostversion;
	mthing = playerstarts[0];
	I_Assert(mthing);
	{ // A bit more complex than P_SpawnPlayer because ghosts aren't solid and won't just push themselves out of the ceiling.
		fixed_t z,f,c;
		gh->mo = P_SpawnMobj(mthing->x << FRACBITS, mthing->y << FRACBITS, 0, MT_GHOST);
		gh->mo->angle = FixedAngle(mthing->angle*FRACUNIT);
		f = gh->mo->floorz;
		c = gh->mo->ceilingz - mobjinfo[MT_PLAYER].height;
		if (!!(mthing->options & MTF_AMBUSH) ^ !!(mthing->options & MTF_OBJECTFLIP))
		{
			z = c;
			if (mthing->options >> ZSHIFT)
				z -= ((mthing->options >> ZSHIFT) << FRACBITS);
			if (z < f)
				z = f;
		}
		else
		{
			z = f;
			if (mthing->options >> ZSHIFT)
				z += ((mthing->options >> ZSHIFT) << FRACBITS);
			if (z > c)
				z = c;
		}
		gh->mo->z = z;
	}
	gh->mo->state = states+S_KART_STND1; // SRB2kart - was S_PLAY_STND
	gh->mo->sprite = gh->mo->state->sprite;
	gh->mo->frame = (gh->mo->state->frame & FF_FRAMEMASK) | tr_trans20<<FF_TRANSSHIFT;
	gh->mo->tics = -1;

	gh->oldmo.x = gh->mo->x;
	gh->oldmo.y = gh->mo->y;
	gh->oldmo.z = gh->mo->z;

	// Set skin
	gh->mo->skin = &skins[0];
	for (i = 0; i < numskins; i++)
		if (!stricmp(skins[i].name,skin))
		{
			gh->mo->skin = &skins[i];
			break;
		}
	gh->oldmo.skin = gh->mo->skin;

	// Set color
	gh->mo->color = ((skin_t*)gh->mo->skin)->prefcolor;
	for (i = 0; i < MAXSKINCOLORS; i++)
		if (!stricmp(KartColor_Names[i],color))				// SRB2kart
		{
			gh->mo->color = (UINT8)i;
			break;
		}
	gh->oldmo.color = gh->mo->color;

	CONS_Printf(M_GetText("Added ghost %s from %s\n"), name, pdemoname);
	Z_Free(pdemoname);
}

//
// G_TimeDemo
// NOTE: name is a full filename for external demos
//
static INT32 restorecv_vidwait;

void G_TimeDemo(const char *name)
{
	nodrawers = M_CheckParm("-nodraw");
	noblit = M_CheckParm("-noblit");
	restorecv_vidwait = cv_vidwait.value;
	if (cv_vidwait.value)
		CV_Set(&cv_vidwait, "0");
	timingdemo = true;
	singletics = true;
	framecount = 0;
	demostarttime = I_GetTime();
	G_DeferedPlayDemo(name);
}

void G_DoPlayMetal(void)
{
	lumpnum_t l;
	mobj_t *mo = NULL;
	thinker_t *th;

	// it's an internal demo
	if ((l = W_CheckNumForName(va("%sMS",G_BuildMapName(gamemap)))) == LUMPERROR)
	{
		CONS_Alert(CONS_WARNING, M_GetText("No bot recording for this map.\n"));
		return;
	}
	else
		metalbuffer = metal_p = W_CacheLumpNum(l, PU_STATIC);

	// find metal sonic
	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if (th->function.acp1 != (actionf_p1)P_MobjThinker)
			continue;

		mo = (mobj_t *)th;
		if (mo->type == MT_METALSONIC_RACE)
			break;
	}
	if (!mo)
	{
		CONS_Alert(CONS_ERROR, M_GetText("Failed to find bot entity.\n"));
		Z_Free(metalbuffer);
		return;
	}

	// read demo header
    metal_p += 12; // DEMOHEADER
	metal_p++; // VERSION
	metal_p++; // SUBVERSION
	metalversion = READUINT16(metal_p);
	switch(metalversion)
	{
	case DEMOVERSION: // latest always supported
		break;
	// too old, cannot support.
	default:
		CONS_Alert(CONS_WARNING, M_GetText("Failed to load bot recording for this map, format version incompatible.\n"));
		Z_Free(metalbuffer);
		return;
	}
	metal_p += 16; // demo checksum
	if (memcmp(metal_p, "METL", 4))
	{
		CONS_Alert(CONS_WARNING, M_GetText("Failed to load bot recording for this map, wasn't recorded in Metal format.\n"));
		Z_Free(metalbuffer);
		return;
	} metal_p += 4; // "METL"

	// read initial tic
	memset(&oldmetal,0,sizeof(oldmetal));
	oldmetal.x = mo->x;
	oldmetal.y = mo->y;
	oldmetal.z = mo->z;
	oldmetal.angle = mo->angle;
	metalplayback = mo;
}

void G_DoneLevelLoad(void)
{
	CONS_Printf(M_GetText("Loaded level in %f sec\n"), (double)(I_GetTime() - demostarttime) / TICRATE);
	framecount = 0;
	demostarttime = I_GetTime();
}

/*
===================
=
= G_CheckDemoStatus
=
= Called after a death or level completion to allow demos to be cleaned up
= Returns true if a new demo loop action will take place
===================
*/

// Stops metal sonic's demo. Separate from other functions because metal + replays can coexist
void G_StopMetalDemo(void)
{

	// Metal Sonic finishing doesn't end the game, dammit.
	Z_Free(metalbuffer);
	metalbuffer = NULL;
	metalplayback = NULL;
	metal_p = NULL;
}

// Stops metal sonic recording.
ATTRNORETURN void FUNCNORETURN G_StopMetalRecording(void)
{
	boolean saved = false;
	if (demo_p)
	{
		UINT8 *p = demobuffer+16; // checksum position
#ifdef NOMD5
		UINT8 i;
		WRITEUINT8(demo_p, DEMOMARKER); // add the demo end marker
		for (i = 0; i < 16; i++, p++)
			*p = P_RandomByte(); // This MD5 was chosen by fair dice roll and most likely < 50% correct.
#else
		WRITEUINT8(demo_p, DEMOMARKER); // add the demo end marker
		md5_buffer((char *)p+16, demo_p - (p+16), (void *)p); // make a checksum of everything after the checksum in the file.
#endif
		saved = FIL_WriteFile(va("%sMS.LMP", G_BuildMapName(gamemap)), demobuffer, demo_p - demobuffer); // finally output the file.
	}
	free(demobuffer);
	metalrecording = false;
	if (saved)
		I_Error("Saved to %sMS.LMP", G_BuildMapName(gamemap));
	I_Error("Failed to save demo!");
}

// reset engine variable set for the demos
// called from stopdemo command, map command, and g_checkdemoStatus.
void G_StopDemo(void)
{
	Z_Free(demobuffer);
	demobuffer = NULL;
	demoplayback = false;
	titledemo = false;
	timingdemo = false;
	singletics = false;

	if (gamestate == GS_INTERMISSION)
		Y_EndIntermission(); // cleanup
	if (gamestate == GS_VOTING)
		Y_EndVote();

	G_SetGamestate(GS_NULL);
	wipegamestate = GS_NULL;
	SV_StopServer();
	SV_ResetServer();
}

boolean G_CheckDemoStatus(void)
{
	boolean saved;

	while (ghosts)
	{
		demoghost *next = ghosts->next;
		Z_Free(ghosts);
		ghosts = next;
	}
	ghosts = NULL;

	// DO NOT end metal sonic demos here

	if (timingdemo)
	{
		INT32 demotime;
		double f1, f2;
		demotime = I_GetTime() - demostarttime;
		if (!demotime)
			return true;
		G_StopDemo();
		timingdemo = false;
		f1 = (double)demotime;
		f2 = (double)framecount*TICRATE;
		CONS_Printf(M_GetText("timed %u gametics in %d realtics\n%f seconds, %f avg fps\n"), leveltime,demotime,f1/TICRATE,f2/f1);
		if (restorecv_vidwait != cv_vidwait.value)
			CV_SetValue(&cv_vidwait, restorecv_vidwait);
		D_AdvanceDemo();
		return true;
	}

	if (demoplayback)
	{
		if (singledemo)
			I_Quit();
		G_StopDemo();

		if (modeattacking)
			M_EndModeAttackRun();
		else
			D_AdvanceDemo();

		return true;
	}

	if (demorecording)
	{
		UINT8 *p = demobuffer+16; // checksum position
#ifdef NOMD5
		UINT8 i;
		WRITEUINT8(demo_p, DEMOMARKER); // add the demo end marker
		for (i = 0; i < 16; i++, p++)
			*p = P_RandomByte(); // This MD5 was chosen by fair dice roll and most likely < 50% correct.
#else
		WRITEUINT8(demo_p, DEMOMARKER); // add the demo end marker
		md5_buffer((char *)p+16, demo_p - (p+16), p); // make a checksum of everything after the checksum in the file.
#endif
		saved = FIL_WriteFile(va(pandf, srb2home, demoname), demobuffer, demo_p - demobuffer); // finally output the file.
		free(demobuffer);
		demorecording = false;

		if (modeattacking != ATTACKING_RECORD)
		{
			if (saved)
				CONS_Printf(M_GetText("Demo %s recorded\n"), demoname);
			else
				CONS_Alert(CONS_WARNING, M_GetText("Demo %s not saved\n"), demoname);
		}
		return true;
	}

	return false;
}

//
// G_SetGamestate
//
// Use this to set the gamestate, please.
//
void G_SetGamestate(gamestate_t newstate)
{
	gamestate = newstate;
}

/* These functions handle the exitgame flag. Before, when the user
   chose to end a game, it happened immediately, which could cause
   crashes if the game was in the middle of something. Now, a flag
   is set, and the game can then be stopped when it's safe to do
   so.
*/

// Used as a callback function.
void G_SetExitGameFlag(void)
{
	exitgame = true;
}

void G_ClearExitGameFlag(void)
{
	exitgame = false;
}

boolean G_GetExitGameFlag(void)
{
	return exitgame;
}

// Same deal with retrying.
void G_SetRetryFlag(void)
{
	retrying = true;
}

void G_ClearRetryFlag(void)
{
	retrying = false;
}

boolean G_GetRetryFlag(void)
{
	return retrying;
}

// Time utility functions
INT32 G_TicsToHours(tic_t tics)
{
	return tics/(3600*TICRATE);
}

INT32 G_TicsToMinutes(tic_t tics, boolean full)
{
	if (full)
		return tics/(60*TICRATE);
	else
		return tics/(60*TICRATE)%60;
}

INT32 G_TicsToSeconds(tic_t tics)
{
	return (tics/TICRATE)%60;
}

INT32 G_TicsToCentiseconds(tic_t tics)
{
	return (INT32)((tics%TICRATE) * (100.00f/TICRATE));
}

INT32 G_TicsToMilliseconds(tic_t tics)
{
	return (INT32)((tics%TICRATE) * (1000.00f/TICRATE));
}

