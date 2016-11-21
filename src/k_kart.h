// SONIC ROBO BLAST 2 KART ~ ZarroTsu
//-----------------------------------------------------------------------------
/// \file  k_kart.h
/// \brief SRB2kart stuff.

#ifndef __K_KART__
#define __K_KART__

#include "doomdef.h"
#include "d_player.h" // Need for player_t

extern const char *KartColor_Names[MAXSKINCOLORS];
void K_GenerateKartColormap(UINT8 *dest_colormap, INT32 skinnum, UINT8 color);
UINT8 K_GetKartColorByName(const char *name);

void K_RegisterKartStuff(void);

void K_KartPlayerThink(player_t *player, ticcmd_t *cmd);
boolean K_SpinPlayer(player_t *player, mobj_t *source);
boolean K_SquishPlayer(player_t *player, mobj_t *source);
boolean K_ExplodePlayer(player_t *player, mobj_t *source);
void K_SpawnKartExplosion(fixed_t x, fixed_t y, fixed_t z, fixed_t radius, INT32 number, mobjtype_t type, angle_t rotangle, boolean spawncenter, boolean ghostit);
void K_SpawnDriftTrail(player_t *player);
void K_MoveKartPlayer(player_t *player, ticcmd_t *cmd, boolean onground);

void K_LoadKartHUDGraphics(void);
void K_drawKartHUD(void);

// =========================================================================
#endif  // __K_KART__
