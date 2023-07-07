// SONIC ROBO BLAST 2 KART ~ ZarroTsu
//-----------------------------------------------------------------------------
/// \file  k_kart.h
/// \brief SRB2kart stuff.

#ifndef __K_KART__
#define __K_KART__

#include "doomdef.h"
#include "d_player.h" // Need for player_t

#define KART_FULLTURN 800

extern UINT8 colortranslations[MAXTRANSLATIONS][16];
extern const char *KartColor_Names[MAXSKINCOLORS];
extern const UINT8 KartColor_Opposite[MAXSKINCOLORS*2];
void K_RainbowColormap(UINT8 *dest_colormap, UINT8 skincolor);
void K_GenerateKartColormap(UINT8 *dest_colormap, INT32 skinnum, UINT8 color);
UINT8 K_GetKartColorByName(const char *name);

void K_RegisterKartStuff(void);

boolean K_IsPlayerLosing(player_t *player);
boolean K_IsPlayerWanted(player_t *player);
void K_KartBouncing(mobj_t *mobj1, mobj_t *mobj2, boolean bounce, boolean solid);
void K_FlipFromObject(mobj_t *mo, mobj_t *master);
void K_MatchGenericExtraFlags(mobj_t *mo, mobj_t *master);
void K_RespawnChecker(player_t *player);
void K_KartMoveAnimation(player_t *player);
void K_KartPlayerHUDUpdate(player_t *player);
void K_KartPlayerThink(player_t *player, ticcmd_t *cmd);
void K_KartPlayerAfterThink(player_t *player);
void K_DoInstashield(player_t *player);
void K_SpawnBattlePoints(player_t *source, player_t *victim, UINT8 amount);
void K_SpinPlayer(player_t *player, mobj_t *source, INT32 type, mobj_t *inflictor, boolean trapitem);
void K_SquishPlayer(player_t *player, mobj_t *source, mobj_t *inflictor);
void K_ExplodePlayer(player_t *player, mobj_t *source, mobj_t *inflictor);
void K_StealBumper(player_t *player, player_t *victim, boolean force);
void K_SpawnKartExplosion(fixed_t x, fixed_t y, fixed_t z, fixed_t radius, INT32 number, mobjtype_t type, angle_t rotangle, boolean spawncenter, boolean ghostit, mobj_t *source);
void K_SpawnMineExplosion(mobj_t *source, UINT8 color);
void K_SpawnBoostTrail(player_t *player);
void K_SpawnSparkleTrail(mobj_t *mo);
void K_SpawnWipeoutTrail(mobj_t *mo, boolean translucent);
void K_DriftDustHandling(mobj_t *spawner);
void K_PuntMine(mobj_t *mine, mobj_t *punter);
void K_DoSneaker(player_t *player, INT32 type);
void K_DoPogoSpring(mobj_t *mo, fixed_t vertispeed, UINT8 sound);
void K_KillBananaChain(mobj_t *banana, mobj_t *inflictor, mobj_t *source);
void K_UpdateHnextList(player_t *player, boolean clean);
void K_DropHnextList(player_t *player);
void K_RepairOrbitChain(mobj_t *orbit);
player_t *K_FindJawzTarget(mobj_t *actor, player_t *source);
boolean K_CheckPlayersRespawnColliding(INT32 playernum, fixed_t x, fixed_t y);
INT16 K_GetKartTurnValue(player_t *player, INT16 turnvalue);
INT32 K_GetKartDriftSparkValue(player_t *player);
void K_KartUpdatePosition(player_t *player);
void K_DropItems(player_t *player);
void K_DropRocketSneaker(player_t *player);
void K_DropKitchenSink(player_t *player);
void K_StripItems(player_t *player);
void K_StripOther(player_t *player);
void K_MomentumToFacing(player_t *player);
fixed_t K_GetKartSpeed(player_t *player, boolean doboostpower);
fixed_t K_GetKartAccel(player_t *player);
UINT16 K_GetKartFlashing(player_t *player);
fixed_t K_3dKartMovement(player_t *player, boolean onground, fixed_t forwardmove);
void K_MoveKartPlayer(player_t *player, boolean onground);
void K_CalculateBattleWanted(void);
void K_CheckBumpers(void);
void K_CheckSpectateStatus(void);
void K_UpdateSpectateGrief(void);

// sound stuff for lua
void K_PlayAttackTaunt(mobj_t *source);
void K_PlayBoostTaunt(mobj_t *source);
void K_PlayOvertakeSound(mobj_t *source);
void K_PlayHitEmSound(mobj_t *source);
void K_PlayPowerGloatSound(mobj_t *source);

const char *K_GetItemPatch(UINT8 item, boolean tiny);
INT32 K_calcSplitFlags(INT32 snapflags);
void K_LoadKartHUDGraphics(void);
void K_drawKartHUD(void);
void K_drawKartFreePlay(UINT32 flashtime);
void K_drawKartTimestamp(tic_t drawtime, INT32 TX, INT32 TY, INT16 emblemmap, UINT8 mode);

// =========================================================================
#endif  // __K_KART__
