// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2012-2016 by John "JTE" Muniz.
// Copyright (C) 2012-2018 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  lua_hook.h
/// \brief hooks for Lua scripting

#ifdef HAVE_BLUA

#include "r_defs.h"
#include "d_player.h"

enum hook {
	hook_NetVars=0,
	hook_MapChange,
	hook_MapLoad,
	hook_PlayerJoin,
	hook_PreThinkFrame,
	hook_ThinkFrame,
	hook_PostThinkFrame,
	hook_MobjSpawn,
	hook_MobjCollide,
	hook_MobjMoveCollide,
	hook_TouchSpecial,
	hook_MobjFuse,
	hook_MobjThinker,
	hook_BossThinker,
	hook_ShouldDamage,
	hook_MobjDamage,
	hook_MobjDeath,
	hook_BossDeath,
	hook_MobjRemoved,
	hook_JumpSpecial,
	hook_AbilitySpecial,
	hook_SpinSpecial,
	hook_JumpSpinSpecial,
	hook_BotTiccmd,
	hook_BotAI,
	hook_LinedefExecute,
	hook_PlayerMsg,
	hook_HurtMsg,
	hook_PlayerSpawn,
	hook_PlayerQuit,
	hook_PlayerThink,
	hook_MusicChange,
	hook_ShouldSpin,	//SRB2KART
	hook_ShouldExplode,	//SRB2KART
	hook_ShouldSquish,	//SRB2KART
	hook_PlayerSpin,	//SRB2KART
	hook_PlayerExplode,	//SRB2KART
	hook_PlayerSquish,	//SRB2KART
	hook_PlayerCmd,		//SRB2KART
	hook_IntermissionThinker, //SRB2KART
	hook_VoteThinker, //SRB2KART

	hook_MAX // last hook
};
extern const char *const hookNames[];

extern boolean hook_cmd_running;	// This is used by PlayerCmd and lua_playerlib to prevent anything from being wirtten to player while we run PlayerCmd.
extern int hook_defrosting;

void LUAh_MapChange(INT16 mapnumber); // Hook for map change (before load)
void LUAh_MapLoad(void); // Hook for map load
void LUAh_PlayerJoin(int playernum); // Hook for Got_AddPlayer
void LUAh_PreThinkFrame(void); // Hook for frame (before mobj and player thinkers)
void LUAh_ThinkFrame(void); // Hook for frame (after mobj and player thinkers)
void LUAh_PostThinkFrame(void); // Hook for frame (at end of tick, ie after overlays, precipitation, specials)
boolean LUAh_MobjHook(mobj_t *mo, enum hook which);
boolean LUAh_PlayerHook(player_t *plr, enum hook which);
#define LUAh_MobjSpawn(mo) LUAh_MobjHook(mo, hook_MobjSpawn) // Hook for P_SpawnMobj by mobj type
UINT8 LUAh_MobjCollideHook(mobj_t *thing1, mobj_t *thing2, enum hook which);
#define LUAh_MobjCollide(thing1, thing2) LUAh_MobjCollideHook(thing1, thing2, hook_MobjCollide) // Hook for PIT_CheckThing by (thing) mobj type
#define LUAh_MobjMoveCollide(thing1, thing2) LUAh_MobjCollideHook(thing1, thing2, hook_MobjMoveCollide) // Hook for PIT_CheckThing by (tmthing) mobj type
boolean LUAh_TouchSpecial(mobj_t *special, mobj_t *toucher); // Hook for P_TouchSpecialThing by mobj type
#define LUAh_MobjFuse(mo) LUAh_MobjHook(mo, hook_MobjFuse) // Hook for mobj->fuse == 0 by mobj type
boolean LUAh_MobjThinker(mobj_t *mo); // Hook for P_MobjThinker or P_SceneryThinker by mobj type
#define LUAh_BossThinker(mo) LUAh_MobjHook(mo, hook_BossThinker) // Hook for P_GenericBossThinker by mobj type
UINT8 LUAh_ShouldDamage(mobj_t *target, mobj_t *inflictor, mobj_t *source, INT32 damage); // Hook for P_DamageMobj by mobj type (Should mobj take damage?)
boolean LUAh_MobjDamage(mobj_t *target, mobj_t *inflictor, mobj_t *source, INT32 damage); // Hook for P_DamageMobj by mobj type (Mobj actually takes damage!)
boolean LUAh_MobjDeath(mobj_t *target, mobj_t *inflictor, mobj_t *source); // Hook for P_KillMobj by mobj type
#define LUAh_BossDeath(mo) LUAh_MobjHook(mo, hook_BossDeath) // Hook for A_BossDeath by mobj type
#define LUAh_MobjRemoved(mo) LUAh_MobjHook(mo, hook_MobjRemoved) // Hook for P_RemoveMobj by mobj type
#define LUAh_JumpSpecial(player) LUAh_PlayerHook(player, hook_JumpSpecial) // Hook for P_DoJumpStuff (Any-jumping)
#define LUAh_AbilitySpecial(player) LUAh_PlayerHook(player, hook_AbilitySpecial) // Hook for P_DoJumpStuff (Double-jumping)
#define LUAh_SpinSpecial(player) LUAh_PlayerHook(player, hook_SpinSpecial) // Hook for P_DoSpinDash (Spin button effect)
#define LUAh_JumpSpinSpecial(player) LUAh_PlayerHook(player, hook_JumpSpinSpecial) // Hook for P_DoJumpStuff (Spin button effect (mid-air))
boolean LUAh_BotTiccmd(player_t *bot, ticcmd_t *cmd); // Hook for B_BuildTiccmd
boolean LUAh_BotAI(mobj_t *sonic, mobj_t *tails, ticcmd_t *cmd); // Hook for B_BuildTailsTiccmd by skin name
boolean LUAh_LinedefExecute(line_t *line, mobj_t *mo, sector_t *sector); // Hook for linedef executors
boolean LUAh_PlayerMsg(int source, int target, int flags, char *msg, int mute); // Hook for chat messages
boolean LUAh_HurtMsg(player_t *player, mobj_t *inflictor, mobj_t *source); // Hook for hurt messages
#define LUAh_PlayerSpawn(player) LUAh_PlayerHook(player, hook_PlayerSpawn) // Hook for G_SpawnPlayer
void LUAh_PlayerQuit(player_t *plr, int reason); // Hook for player quitting
boolean LUAh_MusicChange(const char *oldname, char *newname, UINT16 *mflags, boolean *looping,
	UINT32 *position, UINT32 *prefadems, UINT32 *fadeinms); // Hook for music changes

UINT8 LUAh_ShouldSpin(player_t *player, mobj_t *inflictor, mobj_t *source); // SRB2KART: Should player be spun out?
UINT8 LUAh_ShouldExplode(player_t *player, mobj_t *inflictor, mobj_t *source); // SRB2KART: Should player be exploded?
UINT8 LUAh_ShouldSquish(player_t *player, mobj_t *inflictor, mobj_t *source); // SRB2KART: Should player be squished?

boolean LUAh_PlayerSpin(player_t *player, mobj_t *inflictor, mobj_t *source); // SRB2KART: Hook for K_SpinPlayer. Allows Lua to execute code and/or overwrite its behavior.
boolean LUAh_PlayerExplode(player_t *player, mobj_t *inflictor, mobj_t *source); // SRB2KART: Hook for K_ExplodePlayer. Allows Lua to execute code and/or overwrite its behavior.
boolean LUAh_PlayerSquish(player_t *player, mobj_t *inflictor, mobj_t *source); // SRB2KART: Hook for K_SquishPlayer. Allows Lua to execute code and/or overwrite its behavior.

boolean LUAh_PlayerCmd(player_t *player, ticcmd_t *cmd);	// Allows to write to player cmd before the game does anything with them.

void LUAh_IntermissionThinker(void); // Hook for Y_Ticker
void LUAh_VoteThinker(void);	// Hook for Y_VoteTicker
#define LUAh_PlayerThink(player) LUAh_PlayerHook(player, hook_PlayerThink) // Hook for P_PlayerThink

#endif
