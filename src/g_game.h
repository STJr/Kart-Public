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
/// \file  g_game.h
/// \brief Game loop, events handling.

#ifndef __G_GAME__
#define __G_GAME__

#include "doomdef.h"
#include "doomstat.h"
#include "d_event.h"

extern char gamedatafilename[64];
extern char timeattackfolder[64];
extern char customversionstring[32];
#define GAMEDATASIZE (4*8192)

#ifdef SEENAMES
extern player_t *seenplayer;
#endif
extern char  player_names[MAXPLAYERS][MAXPLAYERNAME+1];
extern INT32 player_name_changes[MAXPLAYERS];

extern player_t players[MAXPLAYERS];
extern boolean playeringame[MAXPLAYERS];

extern UINT8 *demo_p;

// ======================================
// DEMO playback/recording related stuff.
// ======================================

// demoplaying back and demo recording
extern consvar_t cv_recordmultiplayerdemos, cv_netdemosyncquality;

// Publicly-accessible demo vars
struct demovars_s {
	char titlename[65];
	boolean recording, playback, timing;
	UINT16 version; // Current file format of the demo being played
	boolean title; // Title Screen demo can be cancelled by any key
	boolean rewinding; // Rewind in progress

	boolean loadfiles, ignorefiles; // Demo file loading options
	boolean fromtitle; // SRB2Kart: Don't stop the music
	boolean inreplayhut; // Go back to replayhut after demos
	boolean quitafterplaying; // quit after playing a demo from cmdline
	boolean deferstart; // don't start playing demo right away

	tic_t savebutton; // Used to determine when the local player can choose to save the replay while the race is still going
	enum {
		DSM_NOTSAVING,
		DSM_WILLAUTOSAVE,
		DSM_TITLEENTRY,
		DSM_WILLSAVE,
		DSM_SAVED
	} savemode;

	boolean freecam;

};

extern struct demovars_s demo;

typedef enum {
	MD_NOTLOADED,
	MD_LOADED,
	MD_SUBDIR,
	MD_OUTDATED,
	MD_INVALID
} menudemotype_e;

typedef struct menudemo_s {
	char filepath[256];
	menudemotype_e type;

	char title[65]; // Null-terminated for string prints
	UINT16 map;
	UINT8 addonstatus; // What do we need to do addon-wise to play this demo?
	UINT8 gametype;
	UINT8 kartspeed; // Add OR DF_ENCORE for encore mode, idk
	UINT8 numlaps;

	struct {
		UINT8 ranking;
		char name[17];
		UINT8 skin, color;
		UINT32 timeorscore;
	} standings[MAXPLAYERS];
} menudemo_t;


extern mobj_t *metalplayback;

// gametic at level start
extern tic_t levelstarttic;

// for modding?
extern INT16 prevmap, nextmap;
extern INT32 gameovertics;
extern tic_t timeinmap; // Ticker for time spent in level (used for levelcard display)
extern INT16 rw_maximums[NUM_WEAPONS];

// used in game menu
extern consvar_t cv_chatwidth, cv_chatnotifications, cv_chatheight, cv_chattime, cv_consolechat, cv_chatbacktint, cv_chatspamprotection/*, cv_compactscoreboard*/;
extern consvar_t cv_songcredits;
extern consvar_t cv_pauseifunfocused;
//extern consvar_t cv_crosshair, cv_crosshair2, cv_crosshair3, cv_crosshair4;
extern consvar_t cv_invertmouse/*, cv_alwaysfreelook, cv_chasefreelook, cv_mousemove*/;
extern consvar_t cv_invertmouse2/*, cv_alwaysfreelook2, cv_chasefreelook2, cv_mousemove2*/;
extern consvar_t cv_useranalog, cv_useranalog2, cv_useranalog3, cv_useranalog4;
extern consvar_t cv_analog, cv_analog2, cv_analog3, cv_analog4;
extern consvar_t cv_turnaxis,cv_moveaxis,cv_brakeaxis,cv_aimaxis,cv_lookaxis,cv_fireaxis,cv_driftaxis,cv_deadzone;
extern consvar_t cv_turnaxis2,cv_moveaxis2,cv_brakeaxis2,cv_aimaxis2,cv_lookaxis2,cv_fireaxis2,cv_driftaxis2,cv_deadzone2;
extern consvar_t cv_turnaxis3,cv_moveaxis3,cv_brakeaxis3,cv_aimaxis3,cv_lookaxis3,cv_fireaxis3,cv_driftaxis3,cv_deadzone3;
extern consvar_t cv_turnaxis4,cv_moveaxis4,cv_brakeaxis4,cv_aimaxis4,cv_lookaxis4,cv_fireaxis4,cv_driftaxis4,cv_deadzone4;
extern consvar_t cv_ghost_besttime, cv_ghost_bestlap, cv_ghost_last, cv_ghost_guest, cv_ghost_staff;

typedef enum
{
	AXISNONE = 0,
	AXISTURN,
	AXISMOVE,
	AXISBRAKE,
	AXISAIM,
	AXISLOOK,
	AXISDEAD, //Axises that don't want deadzones
	AXISFIRE,
	AXISDRIFT,
} axis_input_e;

// mouseaiming (looking up/down with the mouse or keyboard)
#define KB_LOOKSPEED (1<<25)
#define MAXPLMOVE (50)
#define SLOWTURNTICS (6)

// build an internal map name MAPxx from map number
const char *G_BuildMapName(INT32 map);
void G_BuildTiccmd(ticcmd_t *cmd, INT32 realtics, UINT8 ssplayer);

// copy ticcmd_t to and fro the normal way
ticcmd_t *G_CopyTiccmd(ticcmd_t* dest, const ticcmd_t* src, const size_t n);
// copy ticcmd_t to and fro network packets
ticcmd_t *G_MoveTiccmd(ticcmd_t* dest, const ticcmd_t* src, const size_t n);

// clip the console player aiming to the view
INT16 G_ClipAimingPitch(INT32 *aiming);
INT16 G_SoftwareClipAimingPitch(INT32 *aiming);

boolean InputDown(INT32 gc, UINT8 p);
INT32 JoyAxis(axis_input_e axissel, UINT8 p);

extern angle_t localangle[MAXSPLITSCREENPLAYERS];
extern INT32 localaiming[MAXSPLITSCREENPLAYERS]; // should be an angle_t but signed
extern boolean camspin[MAXSPLITSCREENPLAYERS]; // SRB2Kart

//
// GAME
//
void G_ChangePlayerReferences(mobj_t *oldmo, mobj_t *newmo);
void G_DoReborn(INT32 playernum);
void G_PlayerReborn(INT32 player);
void G_InitNew(UINT8 pencoremode, const char *mapname, boolean resetplayer,
	boolean skipprecutscene);
char *G_BuildMapTitle(INT32 mapnum);

// XMOD spawning
mapthing_t *G_FindCTFStart(INT32 playernum);
mapthing_t *G_FindMatchStart(INT32 playernum);
mapthing_t *G_FindRaceStart(INT32 playernum);
void G_SpawnPlayer(INT32 playernum, boolean starpost);

// Can be called by the startup code or M_Responder.
// A normal game starts at map 1, but a warp test can start elsewhere
void G_DeferedInitNew(boolean pencoremode, const char *mapname, INT32 pickedchar,
	UINT8 ssplayers, boolean FLS);
void G_DoLoadLevel(boolean resetplayer);

void G_LoadDemoInfo(menudemo_t *pdemo);
void G_DeferedPlayDemo(const char *demo);

// Can be called by the startup code or M_Responder, calls P_SetupLevel.
void G_LoadGame(UINT32 slot, INT16 mapoverride);

void G_SaveGameData(boolean force);

void G_SaveGame(UINT32 slot);

// Only called by startup code.
void G_RecordDemo(const char *name);
void G_RecordMetal(void);
void G_BeginRecording(void);
void G_BeginMetal(void);

// Only called by shutdown code.
void G_WriteStanding(UINT8 ranking, char *name, INT32 skinnum, UINT8 color, UINT32 val);
void G_SetDemoTime(UINT32 ptime, UINT32 plap);
UINT8 G_CmpDemoTime(char *oldname, char *newname);

typedef enum
{
	GHC_NORMAL = 0,
	GHC_SUPER,
	GHC_FIREFLOWER,
	GHC_INVINCIBLE
} ghostcolor_t;

extern UINT8 demo_extradata[MAXPLAYERS];
extern UINT8 demo_writerng;
#define DXD_RESPAWN 0x01 // "respawn" command in console
#define DXD_SKIN 0x02 // skin changed
#define DXD_NAME 0x04 // name changed
#define DXD_COLOR 0x08 // color changed
#define DXD_PLAYSTATE 0x10 // state changed between playing, spectating, or not in-game

#define DXD_PST_PLAYING 0x01
#define DXD_PST_SPECTATING 0x02
#define DXD_PST_LEFT 0x03

// Record/playback tics
void G_ReadDemoExtraData(void);
void G_WriteDemoExtraData(void);
void G_ReadDemoTiccmd(ticcmd_t *cmd, INT32 playernum);
void G_WriteDemoTiccmd(ticcmd_t *cmd, INT32 playernum);
void G_GhostAddThok(INT32 playernum);
void G_GhostAddSpin(INT32 playernum);
void G_GhostAddRev(INT32 playernum);
void G_GhostAddColor(INT32 playernum, ghostcolor_t color);
void G_GhostAddFlip(INT32 playernum);
void G_GhostAddScale(INT32 playernum, fixed_t scale);
void G_GhostAddHit(INT32 playernum, mobj_t *victim);
void G_WriteAllGhostTics(void);
void G_WriteGhostTic(mobj_t *ghost, INT32 playernum);
void G_ConsAllGhostTics(void);
void G_ConsGhostTic(INT32 playernum);
void G_GhostTicker(void);

void G_InitDemoRewind(void);
void G_StoreRewindInfo(void);
void G_PreviewRewind(tic_t previewtime);
void G_ConfirmRewind(tic_t rewindtime);

void G_ReadMetalTic(mobj_t *metal);
void G_WriteMetalTic(mobj_t *metal);
void G_SaveMetal(UINT8 **buffer);
void G_LoadMetal(UINT8 **buffer);

// Your naming conventions are stupid and useless.
// There is no conflict here.
typedef struct demoghost {
	UINT8 checksum[16];
	UINT8 *buffer, *p, color;
	UINT16 version;
	mobj_t oldmo, *mo;
	struct demoghost *next;
} demoghost;
extern demoghost *ghosts;

// G_CheckDemoExtraFiles: checks if our loaded WAD list matches the demo's.
#define DFILE_ERROR_NOTLOADED            0x01 // Files are not loaded, but can be without a restart.
#define DFILE_ERROR_OUTOFORDER           0x02 // Files are loaded, but out of order.
#define DFILE_ERROR_INCOMPLETEOUTOFORDER 0x03 // Some files are loaded out of order, but others are not.
#define DFILE_ERROR_CANNOTLOAD           0x04 // Files are missing and cannot be loaded.
#define DFILE_ERROR_EXTRAFILES           0x05 // Extra files outside of the replay's file list are loaded.

void G_DoPlayDemo(char *defdemoname);
void G_TimeDemo(const char *name);
void G_AddGhost(char *defdemoname);
void G_UpdateStaffGhostName(lumpnum_t l);
void G_DoPlayMetal(void);
void G_DoneLevelLoad(void);
void G_StopMetalDemo(void);
ATTRNORETURN void FUNCNORETURN G_StopMetalRecording(void);
void G_StopDemo(void);
boolean G_CheckDemoStatus(void);
void G_SaveDemo(void);
boolean G_DemoTitleResponder(event_t *ev);

INT32 G_GetGametypeByName(const char *gametypestr);
boolean G_IsSpecialStage(INT32 mapnum);
boolean G_GametypeUsesLives(void);
boolean G_GametypeHasTeams(void);
boolean G_GametypeHasSpectators(void);
boolean G_BattleGametype(void);
INT16 G_SometimesGetDifferentGametype(void);
UINT8 G_GetGametypeColor(INT16 gt);
boolean G_RaceGametype(void);
boolean G_TagGametype(void);
void G_ExitLevel(void);
void G_NextLevel(void);
void G_Continue(void);
void G_UseContinue(void);
void G_AfterIntermission(void);
void G_EndGame(void); // moved from y_inter.c/h and renamed

void G_Ticker(boolean run);
boolean G_Responder(event_t *ev);

boolean G_CouldView(INT32 playernum);
boolean G_CanView(INT32 playernum, UINT8 viewnum, boolean onlyactive);

INT32 G_FindView(INT32 startview, UINT8 viewnum, boolean onlyactive, boolean reverse);
INT32 G_CountPlayersPotentiallyViewable(boolean active);

void G_ResetViews(void);
void G_ResetView(UINT8 viewnum, INT32 playernum, boolean onlyactive);
void G_AdjustView(UINT8 viewnum, INT32 offset, boolean onlyactive);

void G_AddPlayer(INT32 playernum);

void G_SetExitGameFlag(void);
void G_ClearExitGameFlag(void);
boolean G_GetExitGameFlag(void);
void G_SetRetryFlag(void);
void G_ClearRetryFlag(void);
boolean G_GetRetryFlag(void);


void G_LoadGameData(void);
void G_LoadGameSettings(void);

void G_SetGameModified(boolean silent, boolean major);

void G_SetGamestate(gamestate_t newstate);

// Gamedata record shit
void G_AllocMainRecordData(INT16 i);
//void G_AllocNightsRecordData(INT16 i);
void G_ClearRecords(void);

//UINT32 G_GetBestScore(INT16 map);
tic_t G_GetBestTime(INT16 map);
//tic_t G_GetBestLap(INT16 map);
//UINT16 G_GetBestRings(INT16 map);
//UINT32 G_GetBestNightsScore(INT16 map, UINT8 mare);
//tic_t G_GetBestNightsTime(INT16 map, UINT8 mare);
//UINT8 G_GetBestNightsGrade(INT16 map, UINT8 mare);

//void G_AddTempNightsRecords(UINT32 pscore, tic_t ptime, UINT8 mare);
//void G_SetNightsRecords(void);

FUNCMATH INT32 G_TicsToHours(tic_t tics);
FUNCMATH INT32 G_TicsToMinutes(tic_t tics, boolean full);
FUNCMATH INT32 G_TicsToSeconds(tic_t tics);
FUNCMATH INT32 G_TicsToCentiseconds(tic_t tics);
FUNCMATH INT32 G_TicsToMilliseconds(tic_t tics);

// Don't split up TOL handling
INT16 G_TOLFlag(INT32 pgametype);

INT16 G_RandMap(INT16 tolflags, INT16 pprevmap, boolean ignorebuffer, UINT8 maphell, boolean callagainsoon, INT16 *extbuffer);
void G_AddMapToBuffer(INT16 map);

#endif
